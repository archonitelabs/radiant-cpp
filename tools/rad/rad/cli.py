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
import rad.clang_tidy
import rad.coverage
import rad.intellisense
import rad.repo


def init(args) -> bool:  # pylint: disable=unused-argument
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
    logging.info("Refreshing compile_commands.json...")
    rad.bazel.refresh_compile_commands()
    # Disabled in favor of compile_commands.json. Required admin on Windows to
    # initialize. Might be removed completely in the future.
    # logging.info("Setting up vscode c_cpp_properties.json...")
    # rad.intellisense.update_vscode_configurations()
    return True


def get_platforms_args(args) -> list[str]:
    """Returns the bazel platforms for the given arguments."""
    if os.name == "nt":
        return [] if args.arch is None else [f"--platforms=//:windows_{args.arch}"]
    return []


def build(args) -> bool:
    """Builds the Radiant project."""
    if args.clean:
        rad.bazel.clean(False)
    output = {
        logging.ERROR: [],
        logging.WARNING: [],
        logging.INFO: ["--subcommands=pretty_print"],
        logging.DEBUG: ["--subcommands", "--toolchain_resolution_debug=.*"],
    }
    verbosity = output[logging.root.level]
    mode = ["-c", "opt"] if args.release else ["-c", "dbg"]
    platforms = get_platforms_args(args)
    clang = ["--repo_env=CC=clang"] if os.name != "nt" and args.clang else []
    nostd = ["--copt=-DRAD_NO_STD"] if args.no_std else []
    return rad.bazel.build("//...", clang + mode + platforms + nostd + verbosity)


def clean(args) -> bool:
    """Cleans the Radiant project."""
    return rad.bazel.clean(args.expunge)


def get_label(args) -> str:
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


def test(args) -> bool:
    """Runs the unit tests for the Radiant project."""
    if args.clean:
        rad.bazel.clean(False)
    output = {
        logging.ERROR: ["--test_output=summary", "--test_summary=terse"],
        logging.WARNING: ["--test_output=summary", "--test_summary=short"],
        logging.INFO: ["--test_output=summary", "--test_summary=detailed"],
        logging.DEBUG: ["--test_output=all", "--test_summary=detailed"],
    }
    verbosity = output[logging.root.level]
    platforms = get_platforms_args(args)
    clang = ["--repo_env=CC=clang"] if os.name != "nt" and args.clang else []
    nostd = ["--copt=-DRAD_NO_STD"] if args.no_std else []
    nocache = ["--nocache_test_results"] if args.no_cache else []
    return rad.bazel.test(
        get_label(args), clang + platforms + nostd + nocache + verbosity
    )


def coverage(args) -> bool:
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
    return True


def lint(args) -> bool:
    """Runs lint checks for the Radiant project."""
    if args.clang_tidy is not None:
        return rad.clang_tidy.bootstrap(args.clang_tidy)
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
    proc = subprocess.run(command, env=env, check=False)
    return proc.returncode == 0


def setup_logging(verbosity) -> None:
    """Sets up the logging configuration."""
    levels = [logging.ERROR, logging.WARNING, logging.INFO, logging.DEBUG]
    level = levels[min(verbosity, len(levels) - 1)]
    logging.basicConfig(
        level=level,
        format="%(asctime)s %(levelname)-8s %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
    )


def main() -> int:
    """Main entry point for the Radiant Development Tool."""
    global_parser = argparse.ArgumentParser(add_help=False)
    global_parser.add_argument(
        "-v", "--verbose", action="count", default=0, help="increase verbosity level"
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
        help="development initialization",
        parents=[global_parser],
    )
    init_praser.set_defaults(func=init)

    # rad build

    build_parser = subparsers.add_parser(
        "build",
        help="build using bazel",
        parents=[global_parser],
    )
    build_parser.add_argument(
        "--clean", action="store_true", help="clean before building"
    )
    if os.name == "nt":
        build_parser.add_argument(
            "--arch",
            default=rad.repo.NORM_ARCH,
            choices=rad.repo.NORM_ARCHES,
            nargs="?",
            help=f"architecture to build (default: {rad.repo.NORM_ARCH})",
        )
    else:
        build_parser.add_argument(
            "--clang", action="store_true", help="use clang compiler"
        )
    build_parser.add_argument(
        "--no-std",
        action="store_true",
        help="build with RAD_NO_STD",
    )
    build_parser.add_argument(
        "--release", action="store_true", help="builds release instead of debug"
    )
    build_parser.set_defaults(func=build)

    # rad clean

    clean_parser = subparsers.add_parser(
        "clean",
        help="clean using bazel",
        parents=[global_parser],
    )
    clean_parser.add_argument(
        "--expunge",
        action="store_true",
        help="removes entire working tree for bazel instance and stops the bazel server",
    )
    clean_parser.set_defaults(func=clean)

    # rad test

    test_parser = subparsers.add_parser(
        "test", help="build and execute unit tests", parents=[global_parser]
    )
    test_parser.add_argument(
        "--clean", action="store_true", help="clean before building"
    )
    if os.name == "nt":
        test_parser.add_argument(
            "--arch",
            default=rad.repo.NORM_ARCH,
            choices=rad.repo.NORM_ARCHES,
            nargs="?",
            help=f"architecture to test (default: {rad.repo.NORM_ARCH})",
        )
    else:
        test_parser.add_argument(
            "--clang", action="store_true", help="use clang compiler"
        )
    test_parser.add_argument(
        "--no-std",
        action="store_true",
        help="tests with RAD_NO_STD",
    )
    test_parser.add_argument(
        "--no-cache",
        action="store_true",
        help="ignore cached test results",
    )
    test_parser.add_argument(
        "--label",
        required=False,
        help="test the given label",
    )
    test_parser.add_argument(
        "--select",
        action="store_true",
        help="select a test label interactively",
    )
    test_parser.set_defaults(func=test)

    # rad coverage

    coverage_parser = subparsers.add_parser(
        "coverage",
        help="generate coverage data",
        parents=[global_parser],
    )
    coverage_parser.add_argument(
        "--clean", action="store_true", help="clean before building"
    )
    coverage_parser.add_argument(
        "--output-xml",
        default=rad.repo.ROOT_PATH / "bazel-out" / "coverage.xml",
        help="output coverage xml file (default: bazel-out/coverage.xml)",
    )
    coverage_parser.add_argument(
        "--label",
        required=False,
        help="generate coverage for the given label",
    )
    coverage_parser.add_argument(
        "--select",
        action="store_true",
        help="select a coverage label interactively",
    )
    coverage_parser.set_defaults(func=coverage)

    # rad lint

    lint_parser = subparsers.add_parser(
        "lint",
        help="run lint checks",
        parents=[global_parser],
    )
    lint_parser.add_argument(
        "--all-files",
        action="store_true",
        help="run lint checks on all files in repo",
    )
    lint_parser.add_argument(
        "--skip",
        type=str,
        required=False,
        help="skip the specified lint checks",
    )
    lint_parser.add_argument(
        "--clang-tidy",
        nargs="*",
        help="bootstraps clang-tidy with the given arguments",
    )
    lint_parser.set_defaults(func=lint)

    # parse arguments...

    args = parser.parse_args(sys.argv[1:])

    setup_logging(args.verbose)

    return 0 if args.func(args) is True else 1
