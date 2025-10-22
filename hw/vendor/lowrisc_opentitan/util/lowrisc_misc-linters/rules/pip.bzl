# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

"""Dependencies that linter rules depend on."""

load("@rules_python//python:pip.bzl", "pip_parse")
load("@python3//:defs.bzl", "interpreter")

def lowrisc_misc_linters_pip_dependencies():
    """
    Declares workspaces linting rules depend on.
    Make sure to call this in your WORKSPACE file.

    Make sure to call lowrisc_misc_linters_dependencies() from
    deps.bzl first.
    """
    pip_parse(
        name = "lowrisc_misc_linters_pip",
        python_interpreter_target = interpreter,
        requirements_lock = Label("//:requirements.txt"),
    )
