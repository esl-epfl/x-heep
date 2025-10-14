# Project Architecture

This is the codebase for the X-HEEP SoC platform. The code is organized as follows:

- Hardware Components, written in SystsemVerilog in `/hw/ip`
- Simulation models for RTL simulation in `/hw/simulation`
- IPs from external repositories in `/hw/vendor`
- Microcontroller configuration files in `/configs`
- Software (drivers, runtime, headers, test applications, etc.) in `/sw`
- Testbench files in `/tb`
- Utility scripts in `/util`
- Top-level makefile is `/Makefile`

## Coding Standards

- SystemVerilog formatting is done with Verible (called by FuseSoC), as configured in `/core-v-mini-mcu.core` in the `format` rule. Format code by calling the `Makefile` wrapper `make verible`.
- Follow the existing naming conventions

# Documentation Resources

The documentation describing the hardware and software build process is in `/docs/source`.

It is also available on Read the Docs: https://x-heep.readthedocs.io/en/latest/index.html
