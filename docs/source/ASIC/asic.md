# ASIC Implementations

X-HEEP is designed to be a simple yet extensible RISC-V microcontroller, perfect for tightly integrating your own custom accelerators within an ASIC.  This approach lets you concentrate on accelerator design, while inheriting a complete RISC-V system, including peripherals, software stacks, FPGA support, and application examples — significantly simplifying hardware integration and application deployment.

Building your "chip" around a full RISC-V microcontroller, rather than just the accelerator, makes testing and demonstrations much easier.  Having a CPU to manage peripherals and handle parts of the application outside the accelerator's scope enables the deployment of complete, end-to-end applications.

That's why we use X-HEEP for testing our accelerators and other custom hardware, and we highly recommend it to you!

To create an ASIC with X-HEEP and your custom accelerator, you'll need to add a pad ring and pad controller (optionally including a frequency-locked loop, or FLL) and integrate your accelerator with X-HEEP.

X-HEEP is freely available under the Solderpad license, empowering anyone to create their own chips.

Following you find ASIC implementations that uses X-HEEP.

## HEEPocrates

``HEEPocrates`` is the first silicon implementation of X-HEEP made by us, i.e. the [ESL](https://www.epfl.ch/labs/esl/) Lab of EPFL.


<p align="center">
  <img src="https://raw.githubusercontent.com/esl-epfl/x-heep/refs/heads/main/docs/source/images/chips/heepocrates.png" />
</p>

<p  align="center">Figure 1: HEEPocrates Test Chip layout in TSMC 65nm LP </p>


`HEEPocrates`, taped out in November 2022, is based on X-HEEP and features the CV32E2 CPU, 256KB of memory (organized into eight 32KB banks), and peripherals including a DMA, two SPI interfaces, an I2C interface, GPIOs, a UART, and JTAG.
It has been used to integrate the [OpenEdgeCGRA](https://github.com/esl-epfl/OpenEdgeCGRA) and the [Blade](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=8988198) in-memory-computing macro.
Clock generation is handled by a frequency-locked loop (FLL) from the [PULP project](https://pulp-platform.org/index.html).

Implemented in `TSMC 65nm LP CMOS` technology using low-voltage threshold (LVT) cells, HEEPocrates targets ultra-low-power healthcare applications, which often have diverse data size, performance, and energy requirements. To address this, HEEPocrates employs fine-grained power domains, enabling precise control over which components are active and which are powered off.

The CPU itself resides in its own power domain and is typically power-gated during long acquisition periods.
After the SPI-DMA subsystem completes writing data to memory, an interrupt wakes the core. Furthermore, unused memory banks are selectively powered off via dedicated power-gating cells, based on the data and instructions required.
Finally, less frequently used peripherals and the CGRA accelerators also have their own power domains, allowing them to be kept off when not in use.

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

Other resources that leverages `HEEPocrates` (silicon implementation, FPGA, or simulation results) are:

- [HEEPocrates: An Ultra-Low-Power RISC-V Microcontroller for Edge-Computing Healthcare Applications](https://infoscience.epfl.ch/entities/publication/16ef81ae-dfa1-46fc-8059-73d8ba850fd2)

- [FEMU: An Open-Source RISC-V Emulation Platform for the Exploration of Accelerator-based Edge Applications](https://infoscience.epfl.ch/server/api/core/bitstreams/6f71b055-1cde-45b9-8153-456ee6702045/content)

- [Energy-Efficient Frequency Selection Method for Bio-Signal Acquisition in AI/ML Wearables](https://dl.acm.org/doi/pdf/10.1145/3665314.3670815)

- [VersaSens: An Extendable Multimodal Platform for Next-Generation Edge-AI Wearables](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10669834)


## HEEPnosis

``HEEPnosis`` is the second silicon implementation of X-HEEP, made by a collaboration between the ESL lab and the 
[TCL](https://www.epfl.ch/labs/tcl/) lab of EPFL.

<p align="center">
  <img src="https://raw.githubusercontent.com/esl-epfl/x-heep/refs/heads/main/docs/source/images/chips/heepnosis.png" />
</p>

<p  align="center">Figure 2: HEEPnosis Test Chip layout in GF 22nm FDX </p>

`HEEPnosis`, taped out in February 2025, utilizes X-HEEP configured with the CV32E2 CPU, 128KB of memory split into four 32KB banks: two contiguous and two interleaved. Peripherals include two DMAs, an SPI interface, GPIOs, a UART, and JTAG.

This chip integrates the Near-Memory macros [Caesar](https://arxiv.org/pdf/2406.14263) and [Maxwell](https://infoscience.epfl.ch/entities/publication/308a11b1-45dc-4bdf-ba04-b7ca35a03bad), as well as the [Wukong](https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=10682670) peripheral. It also incorporates several test IPs implemented using various `logic-suppression` styles and transistor flavors. An FLL from the [PULP project](https://pulp-platform.org/index.html) provides clock generation.

HEEPnosis is implemented in `GF 22nm FDX` technology using a mix of LVT, HVT, and UHVT cells. Its primary purpose is to validate the functionality and efficiency of full-custom blocks designed with logic suppression, as well as to verify the performance and functionality of the integrated near-memory macros.

This chip has been sponsored by [SwissChips](https://swisschips.ethz.ch/).

There are no references yet about this chip as it hasn't been tested yet.

## X-TRELA

`X-TRELA` is the third sillicon implementation of X-HEEP, developed by the [CEI](https://www.cei.upm.es/) lab of the Universidad Politécnica de Madrid (UPM), Spain.

![X-TRELA](/images/chips/xtrela.png)

<p  align="center">Figure 3: X-TRELA Test Chip layout in TSMC 65nm LP </p>

The chip was taped out in February 2025. The chosen X-HEEP configuration for this SoC includes a CV32E40P CPU, an NtoM bus topology, and 256 KB of memory distributed across four contiguous and four interleaved banks of 32 KB each. Peripherals include a DMA, SPI interfaces, an I2C interface, GPIOs, a UART, and JTAG. `X-TRELA` integrates a [STReaming ELAstic Coarse-Grained Reconfigurable Architecture (CGRA)](https://github.com/des-cei/strela) of 32-bit 4x4 Processing Elements, supporting dataflow data- and control-oriented applications. For more information about this hardware accelerator, check this [preprint paper](https://arxiv.org/pdf/2404.12503).

The design uses LVT cells, slim digital IOs, and CUP bondpads, and memory macros sourced from ARM. This is the first tape out of CEI-UPM, employing a single power domain to simplify the ASIC flow.

This chip has been funded by the [A-IQ Ready](https://www.aiqready.eu/) project, supported by the Spanish Government MICIU/AEI 10.13039/501100011033 (PCI2022-135077-2) and the European Union NextGenerationEU/PRTR (101096658).
