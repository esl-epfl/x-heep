# Automatic testing

X-HEEP includes a script to perform automatic tests on your modifications. In addition, we also have
CI tests that check the code.

## Github CIs

Upon push, tests are run on GitHub runners. These include:
* The generated `.sv` files pushed are equal to those generated in the runner (the code does not depend on the modification of generated files)
* Vendor is up to date (the code does not depend on the modification of vendorized files)
* All applications can be built successfully using both gcc and clang
* All applications are successfully simulated using verilator

All tests must be successful before PRs can be merged.

## Simulation script

Additionally, the testing script (`test/test_apps/test_apps.py`) can be used to perform local tests. For quick
debugging, you can check the global variables in the script such as the `BLACKLIST` and `WHITELIST`.

You can run it with the following command:

```bash
make test
```

This will output the results in the terminal and in the `test/test_apps/test_apps.log` file.

Additionally, you can check only the compilation of the applications with the following command:

```bash
make test TEST_FLAGS=--compile-only
```
