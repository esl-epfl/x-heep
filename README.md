Minimal configuration of a core-v-mcu

## Prerequisite

1. [optional] If you wish to install the Conda enviroment with python 3.9:

```bash
$ conda update conda
$ conda env create -f environment.yml
```

Activate the environment with

```bash
$ source activate core-v-mini-mcu
```
2. Install the required Python tools:

```
pip3 install --user -r python-requirements.txt
```

Add '--root user_builds' to set your build foders for the pip packages
and add that folder to the PATH variable

## Adding external IPs

This repository relies on [vendor](https://docs.opentitan.org/doc/ug/vendor_hw/) to add new IPs.
In the ./util folder, the vendor.py scripts implements what is describeb above.


## Compiling for Questasim

```
$ fusesoc --cores-root . run --no-export --target=sim --setup --build openhwgroup.org:systems:core-v-mini-mcu 2>&1 | tee buildsim.log
```
First set the env variable MODEL_TECH to your Questasim bin folder.

Questasim version must be >= Questasim 2019.3

## Compiling Software


First set the RISCV env variable to the compiler folder (without the `/bin` included).

Then go to the ./sw folder and type:

```
make applications/hello_world/hello_world.hex
```

This will create the executable file to be loaded in your target system (ASIC, FPGA, Questasim, Verilator, etc).

## Running Software on Questasim tool

Go to your target system built folder, e.g.

```
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim/
```

Then type:

```
make run PLUSARGS="c firmware=../../../sw/applications/hello_world/hello_world.hex"
```

Replace the  `.hex` file with your own application if you want to run another pre-compiled application.