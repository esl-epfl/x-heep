#  SystemC model

Supporting SystemC model in `X-HEEP` is still a work-in-progress.
However, a simple example is provided in the SystemC testbench available in `tb/tb_sc_top.cpp`.

When compiling the `X-HEEP` with Verilator using SystemC, the above testbench is used for simulation.
The testbench gets an `X-HEEP` external-memory `obi` master port to communicate with a SystemC memory model.

Such model is very simple as meant to be an example and is provided in `tb/systemc_tb`.
For those who want to extend the functionality of `X-HEEP` with SystemC, such examples can be used as starting point.

The SystemC modules leverages `TLM-2.0` as well as baseline SystemC functionalities.

The `X-HEEP` `obi` port is connected to a `C++` direct-mapped cache who handles `hit` and `miss` with pre-defined latencies.
It uses `TLM-2.0` to communicate with the external SystemC memory on `miss` cache-transactions.
A module in SystemC then communicates with the RTL SystemC model compiled by Verilator to provides read/write data.