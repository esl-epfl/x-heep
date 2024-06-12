# Copyright 2023 EPFL
# Solderpad Hardware License, Version 2.1, see LICENSE.md for details.
# SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1
#
# Author: Embedded Systems Laboratory (EPFL)

import os

project = 'X-HEEP'
copyright = '2023, EPFL'
author = 'ESL'

release = '1.0'
version = '1.0.0'

extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    'sphinx.ext.autodoc',
    'sphinx.ext.autosummary',
    'sphinx.ext.intersphinx',
    'myst_parser',
    'autoapi.extension',
]

html_static_path = ['_static']

source_suffix = ['.rst', '.md']

intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'sphinx': ('https://www.sphinx-doc.org/en/master/', None),
}
intersphinx_disabled_domains = ['std']

templates_path = ['_templates']

html_theme = 'sphinx_rtd_theme'

epub_show_urls = 'footnote'

apidoc_module_dir = '../../util/x_heep_gen/'
apidoc_output_dir = 'Configuration/generated'

autoapi_dirs = [apidoc_module_dir]
autoapi_root = apidoc_output_dir
autoapi_python_use_implicit_namespaces = False
autoapi_python_class_content = "both"
autoapi_options = [
    'members',
    'undoc-members',
    'private-members',
    'show-inheritance',
    'show-module-summary',
    'special-members',
]

os.environ["X_HEEP_PROJECT_ROOT"] = "../"