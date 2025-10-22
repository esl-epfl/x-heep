# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("//rules:rules.bzl", "licence_check")
load("@aspect_rules_lint//format:defs.bzl", "format_multirun")

package(default_visibility = ["//visibility:private"])

licence_check(
    name = "licence-check",
    exclude_patterns = [".style.yapf"],
    licence = """
    Copyright lowRISC contributors.
    Licensed under the Apache License, Version 2.0, see LICENSE for details.
    SPDX-License-Identifier: Apache-2.0
    """,
)

alias(
    name = "ruff",
    actual = select({
        "@bazel_tools//src/conditions:linux_x86_64": "@ruff_x86_64-unknown-linux-gnu//:ruff",
        "@bazel_tools//src/conditions:linux_aarch64": "@ruff_aarch64-unknown-linux-gnu//:ruff",
        "@bazel_tools//src/conditions:darwin_arm64": "@ruff_aarch64-apple-darwin//:ruff",
        "@bazel_tools//src/conditions:darwin_x86_64": "@ruff_x86_64-apple-darwin//:ruff",
    }),
)

format_multirun(
    name = "format",
    python = ":ruff",
)
