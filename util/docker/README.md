# x-heep-docker
This directory contains the dockerfile to build the docker container to run the CI pipeline and to be used as an all-in-one solution to make experiments with X-HEEP.

## Docker content

- Verilator 5.040 (for RTL simulation)
- OpenHW RISC-V baremetal toolchain (for software build)
- Clang (alternative for software build)
- Verible (SystemVerilog source formatting and static analysis)


## Quick Start

To use the Docker container to build X-HEEP and run applications on it, follow these steps:

1. Pull the docker image from docker-hub:

```bash
make docker-pull
```

2. Once that the image is pulled you can mount X-HEEP inside the container and open a shell in it:

```bash
make docker-run
```

3. Follow X-HEEP [documentation](https://x-heep.readthedocs.io/en/latest/index.html) to generate the RTL of the SoC, build the simulation model, and run applications.

## Modify and build the container (for the Maintainers)

To upgrade the tools included in the Docker container, edit the [`dockerfile`](./dockerfile). Once you are happy with your changes, you can build the image with:

```bash
make docker-build
```

The container is automatically built and pushed to the registry by the [`docker-publish.yml`](/.github/workflows/docker-publish.yml) GitHub action, that is executed upon push events to the `main` branch (e.g., when a pull request is merged). In case you need to manually push the updated container, you can do so with:

```
make docker-push
```

> [!Note]
> Manually publishing requires logging in with a [personal access token](https://github.com/settings/tokens) with: `docker login ghcr.io -u USERNAME`.

### Apptainer Version

An Apptainer image (SIF format) can be automatically generated from the Docker layers using the following command:

```bash
apptainer build x-heep-toolchain.sif docker://ghcr.io/esl-epfl/x-heep/x-heep-toolchain:latest
```
