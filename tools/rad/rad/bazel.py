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

"""Utility functions to execute or query bazel"""

import json
import logging
import os
import pathlib
import shutil
import subprocess
import sys
from lcov_cobertura.lcov_cobertura import LcovCobertura
from . import repo


class _Bazel:
    """Class for managing information about and invoking bazel. Handles lazily
    locating the bazel executable when needed. Should be accessed via the BAZEL
    global variable."""

    def __init__(self):
        self._located_filename = None

    def run(self, args) -> bool:
        """Runs bazel with the given arguments."""
        proc = subprocess.run([self.filename()] + args, check=False)
        return proc.returncode == 0

    def capture_output(self, args) -> str:
        """Runs bazel with the given arguments and captures the output."""
        proc = subprocess.run(
            [self.filename()] + args, capture_output=True, text=True, check=False
        )
        return proc.stdout.strip()

    def filename(self) -> str:
        """Returns the bazel filename."""
        if self._located_filename is None:
            self._located_filename = self._locate_bazel()
        return self._located_filename

    def _locate_bazel(self):
        """Locates a suitable bazel executable."""
        files = [
            "bazelisk",
            "bazel",
        ]
        for f in files:
            bazel = shutil.which(f)
            if bazel is not None:
                return bazel
        logging.error("bazel not found!")
        raise FileNotFoundError("bazel not found!")


BAZEL = _Bazel()


def build(label, args=None):
    """Builds using bazel with the given label and arguments."""
    if args is None:
        args = []
    logging.info("starting build")
    res = BAZEL.run(["build"] + args + [label])
    logging.log(
        logging.INFO if res else logging.ERROR,
        "build finished" if res else "build failed!",
    )
    return res


def clean(expunge):
    """Cleans the bazel workspace."""
    logging.info("starting clean")
    res = BAZEL.run(["clean"] + (["--expunge"] if expunge else []))
    logging.log(
        logging.INFO if res else logging.ERROR,
        "clean finished" if res else "clean failed!",
    )
    return res


def test(label, args=None):
    """Runs tests using bazel with the given label and arguments."""
    if args is None:
        args = []
    logging.info("starting test")
    res = BAZEL.run(["test"] + args + [label])
    logging.log(
        logging.INFO if res else logging.ERROR,
        "test finished" if res else "test failed!",
    )
    return res


def refresh_compile_commands():
    """Refreshes compile_commands.json using bazel."""
    logging.info("refreshing compile_commands.json")
    res = BAZEL.run(["run", ":refresh_compile_commands"])
    logging.log(
        logging.INFO if res else logging.ERROR,
        "compile_commands.json refreshed" if res else "compile_commands.json failed!",
    )
    return res


def coverage(label, output_xml=None, filters=None):
    """Runs coverage using bazel."""
    try:
        shutil.rmtree(pathlib.Path("bazel-out") / "_coverage")
    except FileNotFoundError:
        pass

    if filters is None:
        filters = "radiant"

    command = ["coverage"]

    if sys.platform == "darwin":
        # HACK: https://github.com/bazelbuild/bazel/issues/14970#issuecomment-1894565761
        command.extend(
            [
                "--experimental_generate_llvm_lcov",
                # pylint: disable=line-too-long
                "--test_env=COVERAGE_GCOV_PATH=/Library/Developer/CommandLineTools/usr/bin/llvm-profdata",
                "--test_env=LLVM_COV=/Library/Developer/CommandLineTools/usr/bin/llvm-cov",
                "--copt=-ffile-compilation-dir=.",
            ]
        )

    command.extend(
        [
            "--instrumentation_filter=" + filters,
            "--combined_report=lcov",
            label,
        ]
    )

    logging.info("starting coverage")

    if BAZEL.run(command) is False:
        logging.error("coverage failed!")
        return False

    logging.info("generating coverage.xml")

    lcov_path = repo.ROOT_PATH / "bazel-out" / "_coverage" / "_coverage_report.dat"
    if output_xml is None:
        coverage_path = repo.ROOT_PATH / "bazel-out" / "coverage.xml"
    else:
        coverage_path = output_xml
    try:
        with open(lcov_path, "r", encoding="utf-8") as lcov_file:
            lcov_data = lcov_file.read()
            lcov_cobertura = LcovCobertura(
                lcov_data, str(repo.ROOT_PATH), demangle=True
            )
            cobertura_xml = lcov_cobertura.convert()
        with open(coverage_path, mode="wt", encoding="utf-8") as output_file:
            output_file.write(cobertura_xml)
    except IOError:
        logging.error("unable to convert %s to cobertura xml", lcov_path)
        return False

    logging.info("coverage finished")
    return True


def get_execution_root():
    """Get the bazel execution_root value"""
    return BAZEL.capture_output(["info", "execution_root"])


def get_output_executables():
    """Get list of built executables"""
    output = BAZEL.capture_output(
        [
            "aquery",
            'mnemonic("CppLink", ...)',
            "--output=jsonproto",
            "--include_param_files=true",
            "--include_artifacts=false",
        ]
    )
    j = json.loads(output)
    execs = []
    for action in j["actions"]:
        for arg in action["arguments"]:
            if arg.startswith("/OUT:") and arg.endswith(".exe"):
                execs.append(arg[5:])
    return execs


def get_output_labels():
    """Get list of target labels that generate a binary"""
    output = BAZEL.capture_output(
        [
            "aquery",
            'mnemonic("CppLink", ...)',
            "--output=jsonproto",
            "--include_artifacts=true",
        ]
    )
    j = json.loads(output)
    labels = []
    for target in j["targets"]:
        labels.append(target["label"])
    return labels


def _get_cpplink_executable(args):
    """Extract the output executable path from the given CppLink arguments"""
    for arg in args:
        if arg.startswith("/OUT:") and arg.endswith(".exe"):
            return arg[5:]
    return None


def get_label_executables(label, deps=False):
    """Get the result executables from the given label"""
    internal_label = f"deps({label})" if deps else label
    output = BAZEL.capture_output(
        [
            "aquery",
            f'mnemonic("CppLink", (outputs(".*exe", {internal_label})))',
            "--output=jsonproto",
        ]
    )
    exes = []
    j = json.loads(output)
    for target in j["targets"]:
        target_id = target["id"]
        for action in j["actions"]:
            if action["targetId"] == target_id:
                exes.append(_get_cpplink_executable(action["arguments"]))
                break
    return sorted(exes)


def get_compile_commands():
    """Returns a list of commands. Each command being a list of arguments."""
    output = BAZEL.capture_output(
        [
            "aquery",
            'mnemonic("CppCompile", ...)',
            "--output=jsonproto",
            "--include_param_files=true",
            "--include_artifacts=false",
        ]
    )
    j = json.loads(output)
    return [action["arguments"] for action in j["actions"]]


def get_includes(commands):
    """Returns a list of includes extracted from all given commands.
    Filters out "." and paths starting with "bazel-out"."""
    includes = {}
    prev = False
    for cmd in commands:
        for arg in cmd:
            if arg.startswith("/I"):
                includes[arg[2:]] = True
            elif arg in ("-isystem", "-iquote"):
                prev = True
            elif prev:
                includes[arg], prev = True, False
    return list(
        filter(lambda x: x != "." and not x.startswith("bazel-out"), includes.keys())
    )


def maybe_create_external_link():
    """Create ./external (symlink/directory junction) to the bazel output
    external directory if it doesn't already exist."""
    target = pathlib.Path(BAZEL.capture_output(["info", "output_base"])) / "external"
    link = pathlib.Path("external")
    if target.exists() and not link.exists():
        os.symlink(target, link, target_is_directory=True)
