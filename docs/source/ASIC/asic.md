# ASIC Implementations

X-HEEP has been mainly designed as to act as a simple, extendable RISC-V microcontroller to integrate your own accelerator in ASIC in 
a tightly coupled fashion.

This will allow people to focus on the design of the accelerator itself, and then hinerit a whole RISC-V system with peripherals, 
SW stacks, FPGAs, applications examples, etc that ease HW integration and application deployment.

Doing a `chip` using a whole RISC-V microcontroller instead of the accelerator standalone simplifies testing and DEMOs.
In addition, having a CPU that controls peripherals and part of the applications the accelerator is not designed for allows for deployments of end-to-end applications.

For all these reasons, we uses X-HEEP for testing our accelerators or any other custom hardware.

We suggest you doing the same 😎

To make an ASIC using X-HEEP with your own accelerators, you need to:

Add a pad ring with its pad controller, optionally a frequency-locked loop (FLL), and extend X-HEEP with you accelerator.

X-HEEP is free to use with solderpad license, so eveyone can make a chip with it.

In fact, within this list you can also find implementations made by other universities or companies (that we are aware of).

## HEEPocrates

``HEEPocrates`` is the first silicon implementation of X-HEEP made by us, i.e. the [ESL](https://www.epfl.ch/labs/esl/) Lab of EPFL.

![HEEPocrates](/images/chips/heepocrates.png)

<p  align="center">Figure 1: HEEPocrates Test Chip layout in TSMC 65nm LP </p>


It uses X-HEEP configures with the `cv32e2` CPU, 256kB of memory organized in 8 banks of 32kB, peripherals as one DMA, two SPI, one I2C, 
one UART, and ...

It has been tapeout in November 2022 using the X-HEEP commit ().

It has been used to integrate the CGRA available here() and the in-memory-computing macro described here.
In addition, for the clock generation it leverages the frequency-locked loop (FLL) from the [PULP project](https://pulp-platform.org/index.html).

``HEEPocrates`` is implemented in ``TSMC 65nm LP CMOS`` technology using low-voltage threshold (LVT) cells.

The chip targets ultra-low-power healthcare applications, which are often characterized by a wide range of data size, performance, and 
energy requirements. For this reason, ``HEEPocrates`` leverages fine-grain power domains to fine-control the components that are 
`active` and the ones that are `switched off`.

The CPU itself has its own power-domain, and is kept off (`power-gated`) during the long acquisition time.
While the SPI-DMA subsystem has finished writing the data to the memory, an interrupt is used to wake the core up.

Also, depending on the data and instructions needed, all the unsed memory banks are switched off via a dedicted `power gating` cell.

Finally, all the peripherals used less rarely, and the CGRA accelerators have also their own power domain to keep them off while not used.

For more information about the `HEEPocrates`, check this [paper](https://arxiv.org/pdf/2401.05548), and if you want to cite it, you can use:

```
@misc{machetti2024xheep,
      title={X-HEEP: An Open-Source, Configurable and Extendible RISC-V Microcontroller for the Exploration of Ultra-Low-Power Edge Accelerators},
      author={Simone Machetti and Pasquale Davide Schiavone and Thomas Christoph Müller and Miguel Peón-Quirós and David Atienza},
      year={2024},
      eprint={2401.05548},
      archivePrefix={arXiv},
      primaryClass={cs.AR}
}
```

Other resources using `HEEPocrates` (silicon implementation, FPGA, or simulation results) are:


## HEEPnosis

``HEEPnosis`` is the second silicon implementation of X-HEEP, made by a collaboration between the ESL lab and the 
[TCL](https://www.epfl.ch/labs/tcl/) lab of EPFL.

![HEEPnosis](/images/chips/heepnosis.png)

<p  align="center">Figure 2: HEEPnosis Test Chip layout in GF 22nm FDX </p>

It uses X-HEEP configures with the `cv32e2` CPU, 128kB of memory organized in 2 banks of 32kB in the contiguos memory space, and 2 banks of 32kB in the interleaved memory space. It has peripherals such as two DMAs, one SPI, one I2C, one UART, and ...

It has been tapeout in February 2024 using the X-HEEP commit ().

It has been used to integrate the Near-Memory macros `caesar` described here(), `maxwell` described here (), and the `wukong` peripheral described here.
In addition, it contains several testing IPs implemented in different `logic-suppression` style and transistor flavours.
For the clock generation, it leverages the FLL from the [PULP project](https://pulp-platform.org/index.html) ref.

``HEEPnosis`` is implemented in ``GF 22nm FDX`` technology using a mixed of LVT, HVT, and UHVT cells.

The chip is used mainly to validate the functionality and efficiency of full-custom blocks designed with `logic-suppression` style, 
as well as to validate the efficiency and functionality of near-memory macros.

There are no references yet about this chip.

This chip has been sponsored by [Swisschip](https://swisschips.ethz.ch/).
