# Copyright 2022 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1

# Author: Pierre Guillod <pierre.guillod@epfl.ch>, EPFL, STI-SEL

package(default_visibility = ["//visibility:public"])

load("//rules:autogen.bzl", "autogen_hjson_header")

autogen_hjson_header(
    name = "pdm2pcm_regs",
    srcs = [
        "pdm2pcm.hjson",
    ],
)

filegroup(
    name = "all_files",
    srcs = glob(["**"]),
)
