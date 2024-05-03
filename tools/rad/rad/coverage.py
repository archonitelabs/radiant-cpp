#! /usr/bin/env python
#
# Copyright 2023 The Radiant Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Utility functions to generate coverage data for the Radiant project."""

import logging
import os
import pathlib
import subprocess
from xml.etree import ElementTree

from . import bazel, repo


def _load_coverage_xml(path):
    """Load colbertura xml from file"""
    return ElementTree.parse(path)


def _merge_coverage_xml(dst, src):
    """Combine dst and src colbertura xml with result in dst."""
    srccov = src.findall(".")[0]
    dstcov = dst.findall(".")[0]
    lines_covered = int(dstcov.get("lines-covered")) + int(srccov.get("lines-covered"))
    lines_valid = int(dstcov.get("lines-valid")) + int(srccov.get("lines-valid"))
    dstcov.set("lines-covered", str(lines_covered))
    dstcov.set("lines-valid", str(lines_valid))
    if lines_valid == 0:
        dstcov.set("line-rate", "0.0")
    else:
        dstcov.set("line-rate", str(lines_covered / lines_valid))
    packages = dst.findall("packages")
    if not packages or len(packages) > 1:
        return
    packages[0].extend(src.iter("package"))


def _opencppcoverage(
    label, output_xml=None, srcs_glob=None, pdb_prefix=None, pdb_prefix_replace=None
):
    """Execute OpenCppCoverage to generate the given output xml file in cobertura format"""
    bazel.build(label)
    executables = bazel.get_label_executables(label)
    if not executables:
        return

    if output_xml is None:
        coverage_path = repo.ROOT_PATH / "bazel-out" / "coverage.xml"
    else:
        coverage_path = pathlib.Path(output_xml)

    output = ["--export_type", "cobertura:" + str(coverage_path)]

    srcs = []
    if srcs_glob is not None:
        srcs = ["--sources", srcs_glob]

    pdb_substitute = []
    if pdb_prefix is not None and pdb_prefix_replace is not None:
        pdb_substitute = [
            "--substitute_pdb_source_path",
            str(pdb_prefix) + "?" + pdb_prefix_replace,
        ]

    for exe in executables:
        if not pathlib.Path(exe).exists():
            bazel.build(label)
            break

    combined = None
    res = True
    logging.info("starting coverage")
    for exe in executables:
        logging.info("running coverage for %s", exe)
        proc = subprocess.run(
            ["OpenCppCoverage", "--quiet"]
            + srcs
            + pdb_substitute
            + output
            + ["--", exe],
            stdout=subprocess.DEVNULL,
            check=False,
        )
        if proc.returncode != 0:
            res = False
            break

        xml = _load_coverage_xml(coverage_path)
        try:
            os.remove(coverage_path)
        except FileNotFoundError:
            pass
        combined = _merge_coverage_xml(combined, xml) if combined else xml

    logging.log(
        logging.INFO if res else logging.ERROR,
        "coverage finished" if res else "coverage failed!",
    )
    if combined:
        combined.write(
            coverage_path, xml_declaration=True, method="xml", encoding="utf-8"
        )


def generate_coverage(
    label, output_xml=None, filters=None, pdb_prefix=None, pdb_prefix_replace=None
):
    """Generate coverage data for the Radiant project."""
    if os.name == "nt":
        _opencppcoverage(label, output_xml, filters, pdb_prefix, pdb_prefix_replace)
    else:
        bazel.coverage(label, output_xml, filters)
