# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("@lowrisc_misc_linters//rules:rules.bzl", "licence_test")

licence_test(
    name = "licence_check",
    exclude_patterns = ["file_without_licence.txt"],
    licence = """
    Copyright lowRISC contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0
    """,
    workspace = ":MODULE.bazel",
)
