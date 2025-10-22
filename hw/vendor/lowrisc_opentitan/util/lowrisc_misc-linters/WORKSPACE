# Copyright lowRISC contributors.
# Licensed under the Apache License, Version 2.0, see LICENSE for details.
# SPDX-License-Identifier: Apache-2.0

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

load("//rules:repos.bzl", "lowrisc_misc_linters_repos")
lowrisc_misc_linters_repos()

load("//rules:deps.bzl", "lowrisc_misc_linters_dependencies")
lowrisc_misc_linters_dependencies()

load("//rules:pip.bzl", "lowrisc_misc_linters_pip_dependencies")
lowrisc_misc_linters_pip_dependencies()

load("@lowrisc_misc_linters_pip//:requirements.bzl", "install_deps")
install_deps()

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
http_archive(
    name = "aspect_rules_lint",
    sha256 = "6da562afb6e328757eb29f3ad044e273317371139d245eac0a74b60173d36025",
    strip_prefix = "rules_lint-1.0.0-rc3",
    url = "https://github.com/aspect-build/rules_lint/releases/download/v1.0.0-rc3/rules_lint-v1.0.0-rc3.tar.gz",
)

# aspect_rules_lint depends on aspect_bazel_lib. Either 1.x or 2.x works.
http_archive(
    name = "aspect_bazel_lib",
    sha256 = "6d758a8f646ecee7a3e294fbe4386daafbe0e5966723009c290d493f227c390b",
    strip_prefix = "bazel-lib-2.7.7",
    url = "https://github.com/aspect-build/bazel-lib/releases/download/v2.7.7/bazel-lib-v2.7.7.tar.gz",
)
load("@aspect_bazel_lib//lib:repositories.bzl", "aspect_bazel_lib_dependencies")

# aspect_bazel_lib depends on bazel_skylib
aspect_bazel_lib_dependencies()

load("@aspect_rules_lint//format:repositories.bzl", "rules_lint_dependencies")
rules_lint_dependencies()

load("@aspect_rules_lint//lint:ruff.bzl", "fetch_ruff")
fetch_ruff()
