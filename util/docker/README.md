# x-heep-docker
This repository contains the dockerfile to build the [X-HEEP](https://github.com/esl-epfl/x-heep) docker (hosted on https://hub.docker.com/r/luigi2898/x-heep)

# Docker content

- Verilator
- risc-v-gcc-toolchain
- clang
- verible


# Commands to pull and run the docker image

Pull the docker image from docker-hub:

```bash
docker pull luigi2898/x-heep:light
```

Once that the image is pulled you can clone X-HEEP (if needed) and directly run it:

```bash
X_HEEP_PATH=<path/to/x-heep/clone>

git clone https://github.com/esl-epfl/x-heep $X_HEEP_PATH

docker run -it -v $X_HEEP_PATH:/workspace/x-heep luigi2898/x-heep:light
```

Once that the docker is up and running, activate the virtual environment:

```bash
conda activate core-v-mini-mcu
```

Follow the [README](https://github.com/esl-epfl/x-heep) of X-HEEP to build hardware and run applications.

# Build the image (Takes some hours)

To build the docker image you can execute:

```bash
docker build -t luigi2898/x-heep:latest .
```


