# Automatic testing

X-HEEP includes two tools to perform automatic tests on your modifications.

## Github CIs

Upon push, tests are run on GitHub runners. These include:
* The generated `.sv` files pushed are equal to those generated in the runner (the code does not depend on the modification of generated files)
* Vendor is up to date (the code does not depend on the modification of vendorized files)
* All applications can be built successfully using both gcc and clang

All tests must be successful before PRs can be merged.

## Simulation script

Additionally, a `test_all.sh` script is provided. Apart from compiling all apps with both gcc and clang, it will simulate them and check the result.

The available parameters are:
* COMPILER: `gcc` (default) or `clang` (can provide more than one)
* SIMULATOR: `verilator` (default), `questasim` or disable simulation with `nosim` (only one, the last provided is used).
* LINKER: `on_chip`(default), `flash_load` or `flash_exec` (can provide more than one)
* TIMEOUT: Integer number of seconds (default 120)

### Usage

#### Comands
You can use two different commands to compile or simulate all the existing APPs:
```
make app-compile-all
```
```
make app-simulate-all
```
Note that both commands allow the previous parameters to specify compiling or simulation options. E.g.:
```
make app-simulate-all LINKER=on_chip SIMULATOR=questasim COMPILER=clang TIMEOUT=150
```

#### Manually
You can also **SOURCE** the script as
```bash
. util/test_all.sh on_chip questasim clang 150
```

*Pay special attention to the first period in the command!*
You will be killing simulations that take too long, if you **EXECUTE** (`./test_all.sh`) this action kills the script.

For both usages (commands or manual), the order of the arguments is irrelevant.

```{warning}
Be sure to commit all your changes before running the script!
```

* Applications that fail being built with gcc will not be simulated (skipped).
* Some applications are skipped by default for not being suitable for simulation.
* If a simulation takes too long (>timeout), it is killed.

* Upon starting, the script will modify the `mcu_cfg.hjson` file to include all peripherals (so the largest number of apps can be run), re-generates the mcu and re-builds the simulation model for the chosen tool.
These changes can be reverted at the end of the execution (default). If changes were not commited, accepting this operation will revert them!

The success of the script is not required for merging a PR.
