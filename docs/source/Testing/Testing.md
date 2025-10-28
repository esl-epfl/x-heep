# Automatic testing

X-HEEP includes a script to perform automatic tests on your modifications. In addition, we also have
CI tests that check the code.

## Github CIs

Upon push, tests are run on GitHub runners. These include:
* The generated files have not been manually modified
* Vendored repositories have not been manually modified
* All applications can be built successfully using both GCC and Clang
* All applications are successfully simulated using verilator

For an exhaustive list, check the `.github/workflows` folder. All tests must be successful before PRs can be merged.

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
