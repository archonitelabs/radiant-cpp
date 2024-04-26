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

"""Updates the given configurations within .vscode/c_cpp_properties.json to
include the include paths extracted via bazel aquery."""

import os
import json
import shutil
import logging
from . import repo
from . import bazel

_LINUX_C_CPP_PROPERTIES = """{
    "configurations": [
        {
            "name": "debug",
            "includePath": [
                "${workspaceFolder}"
            ],
            "intelliSenseMode": "linux-gcc-x64",
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "c++14"
        },
        {
            "name": "release",
            "includePath": [
                "${workspaceFolder}"
            ],
            "defines": [
                "NDEBUG"
            ],
            "intelliSenseMode": "linux-gcc-x64",
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "c17",
            "cppStandard": "c++14"
        }
    ],
    "version": 4
}
"""


def _maybe_generate_default_c_cpp_properties():
    """If .vscode/c_cpp_properties.json doesn't exist, generate a default."""
    dst = repo.ROOT_PATH / ".vscode" / "c_cpp_properties.json"
    if dst.exists():
        logging.debug("c_cpp_properties.json already exists")
        return
    if os.name == "nt":
        has_ewdkdir = os.environ.get("EWDKDIR", None)
        if has_ewdkdir:
            src = (
                repo.ROOT_PATH
                / "external "
                / "ewdk_cc_configured_toolchain"
                / "c_cpp_properties.json"
            )
            shutil.copy(src=src, dst=dst)
        else:
            pass
    else:
        with open(dst, "w", encoding="utf-8") as file:
            file.write(_LINUX_C_CPP_PROPERTIES)


def _load_c_cpp_properties():
    """Loads the existing .vscode/c_cpp_properties.json"""
    with open(
        repo.ROOT_PATH / ".vscode" / "c_cpp_properties.json", "r", encoding="utf-8"
    ) as file:
        return json.load(file)


def _update_c_cpp_configs(configs, includes):
    """Updates each given configurations's includePath section. This removes
    and replaces each includePath that starts with ${workspaceFolder}, and
    leaves all other paths."""
    includes = ["${workspaceFolder}"] + ["${workspaceFolder}/" + x for x in includes]
    for config in configs["configurations"]:
        paths = list(
            filter(
                lambda x: not x.startswith("${workspaceFolder}"), config["includePath"]
            )
        )
        config["includePath"] = includes + paths


def _write_c_cpp_properties(configs):
    """Writes the given configurations to"""
    with open(
        repo.ROOT_PATH / ".vscode" / "c_cpp_properties.json", "w", encoding="utf-8"
    ) as file:
        return json.dump(configs, file, indent=4)


def update_vscode_configurations():
    """Updates the configurations within c_cpp_properties.json to include the
    include paths extracted via bazel aquery."""
    bazel.maybe_create_external_link()
    includes = bazel.get_includes(bazel.get_compile_commands())
    _maybe_generate_default_c_cpp_properties()
    configs = _load_c_cpp_properties()
    _update_c_cpp_configs(configs, includes)
    _write_c_cpp_properties(configs)
