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

"""Utility functions to interact with the Radiant repository."""

import pathlib
import platform


def _norm_machine():
    """Normalizes the machine architecture to a common name."""
    machine = platform.machine().lower()
    if machine in ["x86", "i386", "i686"]:
        return "x86"
    if machine in ["x64", "x86_64", "amd64"]:
        return "x64"
    if machine in ["arm64", "aarch64", "aarch64_be", "armv8b", "armv8l"]:
        return "arm64"
    if machine in ["arm", "armv7l"]:
        return "arm"
    return "unknown"


ROOT_PATH = pathlib.Path(__file__).resolve().parents[3]
NORM_ARCH = _norm_machine()
NORM_ARCHES = ["x64", "x86", "arm64", "arm"]
