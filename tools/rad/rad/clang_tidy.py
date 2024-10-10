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

"""Bootstrap for clang-tidy."""

import os
import subprocess
import rad.repo

FILE_EXTENSIONS = [".h", ".hh", ".hpp", ".hxx", ".c", ".cc", ".cpp", ".cxx"]
TEST_DIR = rad.repo.ROOT_PATH / "test"


def bootstrap(args) -> bool:
    """Bootstraps clang-tidy."""
    files = []
    for arg in args:
        file = rad.repo.ROOT_PATH / arg
        if TEST_DIR in file.parents:
            continue
        if file.is_file() and file.suffix in FILE_EXTENSIONS:
            files.append(file.relative_to(rad.repo.ROOT_PATH))
    if len(files) == 0:
        return True
    if os.name == "nt":
        clang_tidy_args = [
            "--extra-arg=-I.",
            "--extra-arg=-Wno-pragma-once-outside-header",
            "--extra-arg=-Wno-builtin-macro-redefined",
        ]
    else:
        clang_tidy_args = []
    clang_tidy_args.extend(files)
    proc = subprocess.run(
        ["clang-tidy", *clang_tidy_args],
        capture_output=True,
        text=True,
        check=False,
        cwd=rad.repo.ROOT_PATH,
    )
    # print(" ".join(str(a) for a in proc.args))
    # print(proc.stderr)
    print(proc.stdout)
    return proc.returncode == 0
