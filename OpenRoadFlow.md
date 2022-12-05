# OpenROAD support with SkyWater 130 PDK

## Install OpenROAD

Follow the instructions [here](https://openroad.readthedocs.io/en/latest/user/BuildWithDocker.html).

First, clone clone `OpenROAD-flow-scripts`

```bash
cd flow
git clone --recursive https://github.com/The-OpenROAD-Project/OpenROAD-flow-scripts
cd OpenROAD-flow-scripts
```

If you want to install it with `Docker`,
first make you you have it or install it as explained [here](https://docs.docker.com/engine/install/ubuntu/),

then,

```bash
sudo ./build_openroad.sh --clean
cd ../
mv OpenROAD-flow-scripts OpenROAD
```

Otherwise, if you want to install it [locally](https://openroad.readthedocs.io/en/latest/user/BuildLocally.html) instead,


```bash
tools/OpenROAD/etc/DependencyInstaller.sh
sudo ./build_openroad.sh --local
```

Finally, you need to install `KLayout` v0.27.1


```bash
git clone --depth=1 --branch v0.27.1 https://github.com/KLayout/klayout.git
cd klayout
./build.sh -noruby
```

## Edalize

`x-heep` uses a verion of `edalize` + `fusesoc` that supports `sv2v` to convert SystemVerilog to Verilog so that
`OpenRoad` (`yosys`) can compile it.

You need to install `sv2v` as:

```bash
git clone https://github.com/zachjs/sv2v.git
git checkout 36cff4ab0ff3fc64dddb66ef6f3ff4ed80cbd581
cd sv2v
make
```

Follow the instructions at [sv2v](https://github.com/zachjs/sv2v#installation)
and add `sv2v` to the `PATH` variable.

## Run command

First of all, the binaries of OpenRoad are only available from the Docker container, thus:

```
cd flow/OpenROAD
docker run -it -u $(id -u ${USER}):$(id -g ${USER}) -v $(pwd)/flow/platforms:/OpenROAD-flow-scripts/flow/platforms:ro openroad/flow-scripts
```

From the top folder, execute

```bash
fusesoc --verbose --cores-root . run --target=asic_yosys_synthesis --flag=use_sky130 openhwgroup.org:systems:core-v-mini-mcu
```

## Current status

* [hw.patch](./hw.patch) hw/ folder patch

```bash
git apply hw.patch
```

