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
import sys
import subprocess
from concurrent.futures import ThreadPoolExecutor, as_completed
import rad.repo
import rad.bazel

CHUNK_SIZE = 3
TEST_DIR = rad.repo.ROOT_PATH / "test"


def _run_clang_tidy(args, files) -> bool:
    args = args + ["--extra-arg=-v"]
    proc = subprocess.run(
        ["clang-tidy", *args, *files],
        capture_output=True,
        text=True,
        check=False,
        cwd=rad.repo.ROOT_PATH,
    )
    if proc.stderr == "Error: no checks enabled.\n":
        return True
    # print(" ".join(str(a) for a in proc.args))
    # print(proc.stderr)
    print(proc.stdout)
    return proc.returncode == 0


def bootstrap(args) -> bool:
    """Bootstraps clang-tidy."""

    if os.name == "nt":
        # Hack for Windows. Poor interplay with Bazel, compile_commands.json,
        # and clang-tidy causes issues with header files. We need to add the
        # current directory to the include path and disable some warnings. This
        # is a temporary workaround until we can find a better solution.
        clang_tidy_args = [
            "--extra-arg=-I.",
            "--extra-arg=-Wno-pragma-once-outside-header",
            "--extra-arg=-Wno-builtin-macro-redefined",
        ]
    elif sys.platform == "darwin":
        # There are some unfortunate side effects with Bazel compile command
        # generation on macOS. I've tired a few hacks without success. Here is
        # some related reading on work on the topic:
        #
        # https://stackoverflow.com/questions/58466701/clang-tidy-cant-locate-stdlib-headers
        # https://github.com/llvm/llvm-project/issues/52606
        # https://github.com/hedronvision/bazel-compile-commands-extractor/issues/52
        # https://github.com/erenon/bazel_clang_tidy/issues/20
        #
        # N.B. We do not raise or return false here since this is used by the
        # pre-commit hook and we do not want to block commits on macOS.
        print("macOS is not yet supported")
        return True
    else:
        clang_tidy_args = []

    result = True
    args_chunks = [args[i : i + CHUNK_SIZE] for i in range(0, len(args), CHUNK_SIZE)]
    with ThreadPoolExecutor() as executor:
        futures = [
            executor.submit(_run_clang_tidy, clang_tidy_args, chunk)
            for chunk in args_chunks
        ]
        for future in as_completed(futures):
            if future.result() is False:
                result = False
    return result
