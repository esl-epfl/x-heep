# Setup

There are two ways of setting up X-HEEP. You can either use the provided docker image or install and configure the environment manually.

## Docker setup

A docker image containing all the required software dependencies is available on [github-packages](https://ghcr.io/esl-epfl/x-heep/x-heep-toolchain:latest).

It is only required to [install Docker](https://docs.docker.com/engine/install/) and pull the image.

```bash
docker pull ghcr.io/esl-epfl/x-heep/x-heep-toolchain:latest
```

Assuming that X-HEEP has been cloned to `X-HEEP-DIR=\absolute\path\to\x-HEEP\folder`, it is possible to directly run the docker mounting `X-HEEP-DIR` to the path `\workspace\x-heep` in the docker.

```bash
docker run -it -v ${X-HEEP-DIR}:/workspace/x-heep esl-epfl/x-heep/x-heep-toolchain:latest
```

```{warning}
Take care to indicate the absolute path to the local clone of X-HEEP, otherwise Docker will not be able to properly mount the local folder in the container.
```

The two steps above are also wrapped in a dedicated `makefile`:
```bash
make -C util/docker docker-pull # pull the latest available X-HEEP image
make -C util/docker docker-run #  mount the current X-HEEP clone to '/workspace/x-heep'
```

The Docker setup has certain limitations. For example, the following are not supported:

- Simulation with Questasim and VCS, synthesis with Design Compiler. Licenses are required to use these tools, so they are not installed in the container.

- OpenRoad flow is not installed in the container, so it is not possible to run the related make commands.

- Synthesis with Vivado could be possible, but currently is untested.

## Manual setup

### 1. OS requirements

To use `X-HEEP`, first you will need to install some OS dependencies.

The following command `apt` command should install every required package (tested on an Ubuntu 22.04 distribution):
```bash
sudo apt install autoconf automake autotools-dev curl python3 python3-pip python3-tomli libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev libslirp-dev help2man perl make g++ libfl2 libfl-dev zlibc zlib1g zlib1g-dev ccache mold libgoogle-perftools-dev numactl
```

Errors occurring when installing the following packages may be ignored:
```bash
libfl2 libfl-dev zlibc zlib1g zlib1g-dev
```

Links for the packages relative to each software can also be found under the corresponding section of this guide. In general, make sure to have a look at the [Check system requirements](https://opentitan.org/book/doc/getting_started/index.html) section of the OpenTitan documentation.

### 2. Python

We rely on either (a) `miniconda`, or (b) `virtual environment` environment.

Choose between `2.a` or `2.b` to setup your environment.

#### 2.a Miniconda

Install [Miniconda](https://www.anaconda.com/docs/getting-started/miniconda/install#quickstart-install-instructions) python 3.8 version by downloading an older version and selecting the latest `py38` version [here](https://repo.anaconda.com/miniconda/). Then, create the Conda environment from inside the x-heep folder:

```bash
make conda
```

You need to do it only the first time, then just activate the environment every time you work with `X-HEEP` as

```bash
conda activate core-v-mini-mcu
```
or put the command directly in the `~/.bashrc` file.

#### 2.b Virtual Environment

```{note}
The python environment has only been tested on Python 3.8.
```

Install the python virtual environment just as:

```bash
make venv
```

You need to do it only the first time, then just activate the environment every time you work with `X-HEEP` as

```bash
source .venv/bin/activate
```

### 3. Install the RISC-V Compiler:

```{warning}
The RISC-V toolchain environment variable name has changed. Use `RISCV_XHEEP` instead of `RISCV` to avoid conflicts with other projects. If you previously exported `RISCV` for X-HEEP, update your shell initialization files (e.g., `~/.bashrc`, `~/.zshrc`) or environment modules to export `RISCV_XHEEP` and remove or adjust any old `RISCV` definitions accordingly.
```

The RISC-V compiler requires the [following packages](https://github.com/riscv-collab/riscv-gnu-toolchain) to be installed (Check [OS requirements](#1-os-requirements) for Ubuntu distribution). The GitHub page contains instructions for other linux distributions.

Then the installation can proceed with the following commands :
```
git clone https://github.com/riscv/riscv-gnu-toolchain
cd riscv-gnu-toolchain
git checkout 2023.01.03
./configure --prefix=/home/$USER/tools/riscv --with-abi=ilp32 --with-arch=rv32imc --with-cmodel=medlow
make -j $(nproc)
```
You need to set the `RISCV_XHEEP` environment variable like this:

```
export RISCV_XHEEP=/home/$USER/tools/riscv
```
Also consider adding it to your `~/.bashrc` or equivalent so that it's set automatically in the future. 

Optionally you can also compile with clang/LLVM instead of gcc. For that you must install the clang compiler into the same `RISCV_XHEEP` path. The binaries of gcc and clang do not collide so you can have both residing in the same `RISCV_XHEEP` directory. For this you can set the `-DCMAKE_INSTALL_PREFIX` cmake variable to `$RISCV_XHEEP` when building LLVM. This can be accomplished by doing the following:

```
git clone https://github.com/llvm/llvm-project.git
cd llvm-project
git checkout llvmorg-14.0.0
mkdir build && cd build
cmake -G "Unix Makefiles" -DLLVM_ENABLE_PROJECTS=clang -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$RISCV_XHEEP -DLLVM_TARGETS_TO_BUILD="RISCV" ../llvm
cmake --build . --target install
```

### 4. Install Verilator:

X-HEEP supports Verilator version 5.040, which requires the [following packages](https://verilator.org/guide/latest/install.html) to be installed (Check [OS requirements](#1-os-requirements) for Ubuntu distribution). The [documentation](https://verilator.org/guide/latest/install.html) page contains instructions for other linux distributions. 

> [!Note]
> Backward compatibility with Verilator 4.210 is currently maintained, yet _this is very likely to change_ in future releases, so we strongly suggest against using it. Also, Verilator 4.210 _requires GCC older than 12.0_, so make sure to configure your environment accordingly if you choose to use it anyway.

To proceed with the installation, use the following command:

```bash
export VERILATOR_VERSION=5.040

git clone https://github.com/verilator/verilator.git
cd verilator
git checkout v$VERILATOR_VERSION

autoconf
./configure --prefix=/home/$USER/tools/verilator/$VERILATOR_VERSION
make
make install
```

After installation you need to add `/home/$USER/tools/verilator/$VERILATOR_VERSION/bin` to your `PATH` environment variable. Also consider adding it to your `~/.bashrc` or equivalent environment initialization script so that it's on the `PATH` in the future, like this:

```
export VERILATOR_VERSION=5.040
export PATH=/home/$USER/tools/verilator/$VERILATOR_VERSION/bin:$PATH
```

In general, have a look at the [Install Verilator](https://opentitan.org/book/doc/getting_started/setup_verilator.html) section of the OpenTitan documentation.

If you want to see the waveforms generated by the Verilator simulation (FST format), install GTKWAVE:

```
sudo apt install libcanberra-gtk-module libcanberra-gtk3-module
sudo apt-get install -y gtkwave
```

### 5. Install Verible

Files are formatted with Verible. We use version v0.0-4023-gc1271a00

```
export VERIBLE_VERSION=v0.0-4023-gc1271a00
wget https://github.com/chipsalliance/verible/releases/download/${VERIBLE_VERSION}/verible-${VERIBLE_VERSION}-linux-static-x86_64.tar.gz
tar -xf verible-${VERIBLE_VERSION}-linux-static-x86_64.tar.gz
mkdir -p /home/$USER/tools/verible/${VERIBLE_VERSION}/
mv verible-${VERIBLE_VERSION}/* /home/$USER/tools/verible/${VERIBLE_VERSION}/
rm verible-${VERIBLE_VERSION}-linux-static-x86_64.tar.gz
rm -r verible-${VERIBLE_VERSION}
```

After installation you need to add `/home/$USER/tools/verible/${VERIBLE_VERSION}/bin` to your `PATH` environment variable. Also consider adding it to your `~/.bashrc` or equivalent so that it's on the `PATH` in the future, like this:

```
export VERIBLE_VERSION=v0.0-4023-gc1271a00
export PATH=/home/$USER/tools/verible/${VERIBLE_VERSION}/bin:$PATH
```

In general, have a look at the [Install Verible](https://opentitan.org/book/doc/getting_started/index.html#step-7a-install-verible-optional) section of the OpenTitan documentation (the version referenced there is _not_ the one we use in X-HEEP).
