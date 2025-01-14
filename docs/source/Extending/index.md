# eXtending X-HEEP

X-HEEP is meant to be extended with your own custom IPs. X-HEEP itself posseses a hardware-software framework capable of working standalone. If you want to extend it, you will need to merge your hardware and software with X-HEEP's.

For this purpose we support the [CV-X-IF](https://docs.openhwgroup.org/projects/openhw-group-core-v-xif/en/latest/intro.html) interface with the [cv32e40px](https://github.com/esl-epfl/cv32e40px) or [cv32e40x](https://github.com/openhwgroup/cv32e40x) RISC-V CPUs, and we expose master and slave ports to/from the bus.

> We recommend using the cv32e40px for pairing with your CV-X-IF compliant coprocessor. If you choose to use the cv32e40x, X-HEEP currently uses the revision [`0.9.0`](https://github.com/openhwgroup/cv32e40x/commit/f17028f2369373d9443e4636f2826218e8d54e0f). It is recommended to use the same revision in peripheral IPs to prevent conflicts during RTL compilation.

In addition, the X-HEEP testbench has been extended with a DMA, dummy peripherals (including the flash), and a CV-X-IF compatible coprocessor implementing the F RISC-V extension. This has been done to help us maintaining and verifying the extension interface.

If you want to try the FPU-like coprocessor with a CV-X-IF compatible CPU as the cv32e40px, you can do it in the base X-HEEP as follows:

```
make mcu-gen CPU=cv32e40px
make verilator-sim FUSESOC_PARAM="--X_EXT=1"
make app PROJECT=example_matfadd ARCH=rv32imfc
cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator
./Vtestharness +firmware=../../../sw/build/main.hex
```

The program should terminate with value 0.

To learn how to extend X-HEEP you can read the guides in this section.

```{toctree}
:maxdepth: 3
:numbered: 1

eXtendingHW
eXtendingSW
eXamples
