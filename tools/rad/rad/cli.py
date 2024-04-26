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

"""Radiant Development Tool command line interface."""

import argparse
import logging
import os
import pathlib
import subprocess
import sys

import rad.bazel
import rad.coverage
import rad.intellisense
import rad.repo


def init(args):  # pylint: disable=unused-argument
    """Initializes the Radiant development environment."""
    print("Initializing Radiant Development Environment...")
    logging.info("Setting up pre-commit hooks...")
    subprocess.run(
        [
            "pre-commit",
            "install",
            "--config",
            rad.repo.ROOT_PATH / ".pre-commit-config.yaml",
        ],
        check=False,
    )
    logging.info("Generating compile_commands.json...")
    rad.bazel.generate_compile_commands()
    # Disabled in favor of compile_commands.json. Required admin on Windows to
    # initialize. Might be removed completely in the future.
    # logging.info("Setting up vscode c_cpp_properties.json...")
    # rad.intellisense.update_vscode_configurations()


def build(args):
    """Builds the Radiant project."""
    if args.clean:
        rad.bazel.clean(False)
    output = {
        logging.ERROR: [],
        logging.WARNING: [],
        logging.INFO: ["--subcommands=pretty_print"],
        logging.DEBUG: ["--subcommands"],
    }
    verbosity = output[logging.root.level]
    mode = ["-c", "opt"] if args.release else ["-c", "dbg"]
    platform = []
    if os.name == "nt":
        if args.win_x86:
            platform = ["--platforms=:windows_x86"]
        elif args.win_x64:
            platform = ["--platforms=:windows_x64"]
        elif args.win_arm:
            platform = ["--platforms=:windows_arm"]
        elif args.win_arm64:
            platform = ["--platforms=:windows_arm64"]
    clang = ["--repo_env=CC=clang"] if os.name != "nt" and args.clang else []
    nostd = ["--copt=-DRAD_NO_STD"] if args.no_std else []
    rad.bazel.build("//...", clang + mode + platform + nostd + verbosity)


def clean(args):
    """Cleans the Radiant project."""
    rad.bazel.clean(args.expunge)


def get_label(args):
    """Returns the label to use for the given arguments."""
    if args.label:
        return args.label
    if not args.select:
        return "//..."
    labels = rad.bazel.get_output_labels()
    labels.insert(0, "//...")
    for n, label in enumerate(labels):
        print(f"{n + 1}) {label}")
    n = int(input("Select: ")) - 1
    if n < 0 or n >= len(labels):
        return "//..."
    return labels[n]


def test(args):
    """Runs the unit tests for the Radiant project."""
    if args.clean:
        rad.bazel.clean(False)
    output = {
        logging.ERROR: ["--test_output=summary", "--test_summary=terse"],
        logging.WARNING: ["--test_output=summary", "--test_summary=short"],
        logging.INFO: ["--test_output=summary", "--test_summary=detailed"],
        logging.DEBUG: ["--test_output=all", "--test_summary=detailed"],
    }
    params = output[logging.root.level]
    if args.no_cache:
        params.append("--nocache_test_results")
    if args.no_std:
        params.append("--copt=-DRAD_NO_STD")
    rad.bazel.test(get_label(args), params)


def coverage(args):
    """Generates coverage data for the Radiant project."""
    if args.clean:
        rad.bazel.clean(False)
    label = get_label(args)
    if os.name == "nt":
        exec_root = rad.bazel.get_execution_root()
        pdb_prefix = pathlib.PurePath(exec_root) / "radiant"
        rad.coverage.generate_coverage(
            label,
            args.output_xml,
            filters="radiant\\*",
            pdb_prefix=pdb_prefix,
            pdb_prefix_replace="radiant",
        )
    else:
        rad.coverage.generate_coverage(label, args.output_xml, filters="radiant")


def lint(args):
    """Runs lint checks for the Radiant project."""
    env = os.environ.copy()
    if args.skip:
        env["SKIP"] = args.skip
    command = [
        "pre-commit",
        "run",
        "--config",
        rad.repo.ROOT_PATH / ".pre-commit-config.yaml",
    ]
    if args.all_files:
        command.append("--all-files")
    subprocess.run(command, env=env, check=False)


def setup_logging(verbosity):
    """Sets up the logging configuration."""
    levels = [logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG]
    level = levels[min(verbosity, len(levels) - 1)]
    logging.basicConfig(
        level=level,
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )


def main():
    """Main entry point for the Radiant Development Tool."""
    global_parser = argparse.ArgumentParser(add_help=False)
    global_parser.add_argument(
        "-v", "--verbose", action="count", default=0, help="Increase verbosity level"
    )
    parser = argparse.ArgumentParser(
        prog="rad",
        description="Radiant Development Tool",
        parents=[global_parser],
    )
    subparsers = parser.add_subparsers(
        title="commands",
        description="valid commands",
        required=True,
    )

    # rad init

    init_praser = subparsers.add_parser(
        "init",
        help="Development initialization",
        parents=[global_parser],
    )
    init_praser.set_defaults(func=init)

    # rad build

    build_parser = subparsers.add_parser(
        "build",
        help="Build using bazel",
        parents=[global_parser],
    )
    build_parser.add_argument(
        "--clean", action="store_true", help="Clean before building"
    )
    build_parser.add_argument(
        "--release", action="store_true", help="Builds release instead of debug"
    )
    if os.name == "nt":
        plat = build_parser.add_mutually_exclusive_group()
        plat.add_argument(
            "--win_x86", action="store_true", help="Cross compile for x86 target"
        )
        plat.add_argument(
            "--win_x64", action="store_true", help="Cross compile for x64 target"
        )
        plat.add_argument(
            "--win_arm", action="store_true", help="Cross compile for ARM target"
        )
        plat.add_argument(
            "--win_arm64", action="store_true", help="Cross compile for ARM64 target"
        )
    else:
        build_parser.add_argument(
            "--clang", action="store_true", help="Use clang compiler"
        )
    build_parser.add_argument(
        "--no-std",
        action="store_true",
        help="Build with RAD_NO_STD",
    )
    build_parser.set_defaults(func=build)

    # rad clean

    clean_parser = subparsers.add_parser(
        "clean",
        help="Clean using bazel",
        parents=[global_parser],
    )
    clean_parser.add_argument(
        "--expunge",
        action="store_true",
        help="Removes entire working tree for bazel instance and stops the bazel server.",
    )
    clean_parser.set_defaults(func=clean)

    # rad test

    test_parser = subparsers.add_parser(
        "test", help="Build and execute unit tests", parents=[global_parser]
    )
    test_parser.add_argument(
        "--clean", action="store_true", help="Clean before building"
    )
    test_parser.add_argument(
        "--no-cache",
        action="store_true",
        help="Ignore cached test results, run all tests unconditionally.",
    )
    test_parser.add_argument(
        "--label",
        required=False,
        help="Test the given label",
    )
    test_parser.add_argument(
        "--select",
        action="store_true",
        help="Select a test label interactively",
    )
    test_parser.add_argument(
        "--no-std",
        action="store_true",
        help="Tests with RAD_NO_STD",
    )
    test_parser.set_defaults(func=test)

    # rad coverage

    coverage_parser = subparsers.add_parser(
        "coverage",
        help="Generate coverage data",
        parents=[global_parser],
    )
    coverage_parser.add_argument(
        "--clean", action="store_true", help="Clean before building"
    )
    coverage_parser.add_argument(
        "--output-xml",
        default=rad.repo.ROOT_PATH / "bazel-out" / "coverage.xml",
        help="Output coverage xml file (default: bazel-out/coverage.xml)",
    )
    coverage_parser.add_argument(
        "--label",
        required=False,
        help="Generate coverage for the given label",
    )
    coverage_parser.add_argument(
        "--select",
        action="store_true",
        help="Select a coverage label interactively",
    )
    coverage_parser.set_defaults(func=coverage)

    # rad lint

    lint_parser = subparsers.add_parser(
        "lint",
        help="Run lint checks",
        parents=[global_parser],
    )
    lint_parser.add_argument(
        "--all-files",
        action="store_true",
        help="Run lint checks on all files in repo",
    )
    lint_parser.add_argument(
        "--skip",
        type=str,
        required=False,
        help="Skip the specified lint checks",
    )
    lint_parser.set_defaults(func=lint)

    # parse arguments...

    args = parser.parse_args(sys.argv[1:])

    setup_logging(args.verbose)

    args.func(args)
