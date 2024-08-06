
# DMA

## Introduction

The **Direct Memory Access (DMA)** peripheral allows data transfers with reduced CPU interaction.

It can perform *transactions* of data between peripherals and memory, or memory-to-memory (as a `memcpy` would).

The CPU is required to configure the transaction, but once launched it is free to go to sleep, process the incoming data or do anything else.

This unit is capable of performing complex tasks that can significantly impact on the performance and power consumption of memory-intense applications. 
It can be configured to perform *1D* or *2D* transactions and it can apply **zero padding** and perform **transpositions** on-the-fly, reducing the overhead of matrix operations.

The DMA **Hardware Abstraction Layer (HAL)** facilitates the configuration of transactions from the users application. Furthermore, it adds an additional layer of safety checks to reduce the risk of faulty memory accesses, data override or infinite loops.
<br>

## Structural description

INSERIRE SCHEMATICO DMA

#### DMA channels layout

The DMA subsystem is composed of a parametrized number of control units called *channels*. 
Each channel can be configured, by the CPU or by an external controller, to perform a *transaction*, independently of the state of other channels.

N-channels are connected to a N-to-M bus that exposes M-master ports on the system bus. Multiple channels can thus perform multiple transactions in parallel, a feature that enables memory-intense applications to greatly increase their throuhput.

There are several ways to connect N-channels to the system bus through M-master ports.
e.g.
Let's consider a DMA subsystem with N = 4 channels and M = 2 master ports.

There are two possible solutions: 
- CH0, CH1 connected to port 0 & CH2, CH3 connected to port 1
- CH0, CH1, CH2 connected to port 0 & CH3 connected to port 1

In order to specify one among these configurations, the user has to set the `num_channels_per_master_port` parameter in `mcu_cfg.hjson`, which defines the _maximum_ channels per master port ratio.

The first configuration of the previous example has 2 channels per master port, so a channels per master port ratio of 2.
On the other hand, the second solution has a ratio of 3: the first 3 channels are connected to port 0, while the remaining channel is connected to the remaining port 1.

While the 1st solution is a general purpose, balanced configuration, the 2nd solution might be better suited for applications that need a low latency channel for high priority tasks.

This mechanism guarantees maximum flexibility, enabling the user to adapt the DMA subsystem to its requirements, both in terms of area and performance.
<br>

#### Interrupts

If enabled, a transaction interrupt is raised every time a DMA transaction is completed.
Due to architectural limitations, there is only a single transaction done signal for the whole DMA subsystem. 
In order to allow the user to identify which channel raised the transaction interrupt, an interrupt flag register system has been developed.
It works in this way: when the DMA channel has completed the transaction and the interrupt enable register is correctly set, the transaction IFR is set to 1. This register has been developed to be cleared once read: this mechanism is very handy because it removes the need for an additional register write.
The transaction interrupt handler takes advantage of this mechanism to identify the channel that raised the interrupt: as soon as an IFR is read high, it launches the de-facto handler, which can be redefined by the user [LINK ALLA SEZIONE HAL].
It could happen that a channel raises an interrupt while the CPU is processing a previous interrupt of another channel: this is not an issue, because the IFR will stay set until the CPU reads its content. 
The only issue is that this situation will cause an additional delay to the execution of the application.
For this reason, the handler implemented by the user should be as short as possible.

#### Data FIFOs configuration

Each DMA channel uses a FIFO to buffer the data to be written, which is crucial for mitigating the combined delays from the system bus and the DMA subsystem bus. 
The size of the FIFO is parametric and is, by default, the same across all channels.

Some applications can benefit from a larger FIFO because it allows for more values to be buffered in situations where the bus is heavily utilized or the target peripheral, such as the SPI, is too slow.
On the other hand, other applications might not require such a large FIFO, so area can be saved by reducing its size.
A hybrid system, where some channels have large FIFO sizes and others have smaller ones, could benefit both these types of applications.

It is possible to specify the size of each DMA channel FIFO in `dma_subsystem.sv`. 
These are the steps to follow to take advantage this feature:

- Uncomment `//define EN_SET_FIFO_CH_SIZE;` to enable the mechanism
- Adjust the parameters _L_, _M_ and _S_. They define the size of a large, medium and small FIFO.
- Modify the parameter `typedef enum {L, M, S} fifo_ch_size_t;` to assign individual sizes to the FIFOs. The number of elements must reflect the number of DMA channels.
<br>

#### Triggers

In the case of memory-peripheral operations, it is common for the peripheral to have a reaction time that cannot match the system clock. For example, the SPI trasmits data with a period of circa 30 clock cycles. 

This difference in response times creates the need for a communication channel between DMA subsystem and peripheral allowing the DMA operations to be suspended according to the peripheral state. These signals are called _triggers_. 

They can be used both when the peripheral writes data using the DMA and when the DMA reads data from the peripheral.
The DMA can be configured to respond to triggers by enabling the appropriate _slot_ via software, using the DMA HAL. [INSERIRE LINK A ESEMPIO SPI SLOT DMA MULTICHANNEL]
<br>

#### Tips for DMA-based accelerator developers

The DMA subsystem has been developed with specific features to facilitate the creation of custom accelerators that can leverage it to improve memory-intense applications.

- **Always-On Peripheral Bus** (AOPB): it exposes the register interface of the units in the Always-On subsystem to any Smart Peripheral Controller (SPC). 
In the case of the DMA subsystem, this feature allows the developers to configure the DMA subsystem without any CPU action, reducing power consumption while at the same time increasing the performance and effectiveness of the accelerator. 
Check out the _im2col SPC_ in the `\ip_examples` folder for a detailed example and [LINK DOCUMENTAZIONE AOPB] for a detailed description of the AOPB.
<br>

- **Triggers**: useful to synchronize the data streams to and from the accelerator.
<br>
- **Stop signal**: it can terminate a DMA transaction at any moment. It's particularly useful for accelerators that produce a large quantity of data, but with a variable trasfer size that cannot be known or computed. 
A good example of such an accelerator is a level crossing subsampler, which writes sampled data only when they cross a specific threshold.
<br>

- **VerifHEEP**: it's a python library that has been developed to test computational units and accelerators developed on X-Heep. 
It has been deployed succesfully to validate the _im2col SPC_ as it is expecially useful for data-intense accelerators.
[LINK ALLA DOCUMENTAZIONE DI VERIFHEEP]
It includes methods to: 
  - Generate random inputs and compute the corresponding golden results
  - Launch synthesis & simulations on QuestaSim and Verilator
  - Compile, program & launch applications on FPGA targets for a tenfold reduction in test & verification times
  - Analyze the performance of the tests and plot the results. 
<br>

### Registers description

This section will describe every register of a DMA channel and their function.
The complete addres of a DMA channel register is the following:
<p style="text-align: center;"><code>DMA_START_ADDRESS + DMA_CH_SIZE * channel + REGISTER_OFFSET</code></p>

The previous parameters, including the register offsets, can be found at `sw/device/lib/runtime/core_v_mini_mcu.h` and `sw/device/lib/drivers/dma/dma_regs.h`

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|-------------- 31 : 0 -------------|
|-------------- PTR_IN -------------|</code></pre>
</div>

- **SRC_PTR_REG**
  - _SW access_: rw
  - _Description_: contains the pointer to the source, which could either be data stored in memory or a peripheral.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|-------------- 31 : 0 -------------|
|------------- PTR_OUT -------------|</code></pre>
</div>

- **DST_PTR_REG**
  - _SW access_: rw
  - _Description_: contains the pointer to the destination, which could either be data stored in memory or a peripheral.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|-------------- 31 : 0 -------------|
|------------- PTR_ADDR ------------|</code></pre>
</div>

- **ADDR_PTR_REG**
  - _SW access_: rw
  - _Description_: Used only with the address mode [LINK ALLE MODALITA']. It contains the pointer to the source, which in this case must data stored in memory.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 14 ----|---- 15 : 0 ----|
|--- Reserved ----|----- SIZE -----|</code></pre>
</div>

- **SIZE_D1**
  - _SW access_: rw
  - _Description_: number of bytes to be copied by the DMA channel along the first dimension, i.e. using the first counter. As soon as this register is written, the **transaction starts**.


<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 14 ----|---- 15 : 0 ----|
|--- Reserved ----|----- SIZE -----|</code></pre>
</div>

- **SIZE_D2**
  - _SW access_: rw
  - _Description_: number of bytes to be copied by the DMA channel along the second dimension, i.e. using the second counter.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 2 ----|--- 1 ---|--- 0 ---|
|--- Reserved ---|- WIN_DN |-- RDY --|</code></pre>
</div>

- **STATUS**
  - _SW access_: ro
  - _Description_: this register is used by the DMA to communicate the status of the transaction. READY is 0 when the DMA is busy performing the transaction, 1 otherwise.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 6 ----|---- 5 : 0 ----|
|--- Reserved ---|----- INC -----|</code></pre>
</div>

- **SRC_PTR_INC_D1**
  - _SW access_: rw
  - _Description_: increment in bytes to apply to the source pointer for every copied element.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|--- 31 : 23 --|---- 22 : 0 ----|
|-- Reserved --|------ INC -----|</code></pre>
</div>

- **SRC_PTR_INC_D2**
  - _SW access_: rw
  - _Description_: increment in bytes to apply to the source pointer every time a "row" is copied, in order to go to the new line. It's necessary only for 2D transactions. 
  From an application perspective, this value has to be computed depending on the size of the input matrix and the 2D stride. Check the functional description paragraph for more information.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 6 ----|---- 5 : 0 ----|
|--- Reserved ---|----- INC -----|</code></pre>
</div>

- **DST_PTR_INC_D1**
  - _SW access_: rw
  - _Description_: increment in bytes to apply to the destination pointer for every copied element.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|--- 31 : 23 --|---- 22 : 0 ----|
|-- Reserved --|------ INC -----|</code></pre>
</div>

- **DST_PTR_INC_D2**
  - _SW access_: rw
  - _Description_: increment in bytes to apply to the destination pointer every time a "row" is copied in order to go to the new line. It's necessary only for 2D transactions. 
  From an application perspective, this value has to be computed depending on the size of the output matrix and the 2D stride. Check the functional description paragraph for more information.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 16 ----|---- 15 : 0 ----|
|----- TX_TRG ----|---- RX_TRG ----|</code></pre>
</div>

- **SLOT**
  - _SW access_: rw
  - _Description_: identifies the triggers for which the DMA channel will stall in writing and/or reading operations.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 2 ----|---- 1 : 0 ----|
|--- Reserved ---|---- DATA_T ---|</code></pre>
</div>

- **SRC_DATA_TYPE**
  - _SW access_: rw
  - _Description_: defines the source data type using this scheme:
    - 0: _word_, i.e. 4 bytes
    - 1: _half word_, i.e. 2 bytes
    - 2 or 3: _byte_

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 2 ----|---- 1 : 0 ----|
|--- Reserved ---|---- DATA_T ---|</code></pre>
</div>

- **DST_DATA_TYPE**
  - _SW access_: rw
  - _Description_: defines the destination data type using this scheme:
    - 0: _word_, i.e. 4 bytes
    - 1: _half word_, i.e. 2 bytes
    - 2 or 3: _byte_
    If wider than the source datatype and signe extension is enable, the output data will be sign extended

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 1 ----|---- 0 : 0 ---|
|--- Reserved ---|---- SGND ----|</code></pre>
</div>

- **SIGN_EXT**
  - _SW access_: rw
  - _Description_: enables the sign extension, which can be performed only if the output datatype is wider that the source datatype

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 2 ----|--- 1 : 0 ---|
|--- Reserved ---|---- MODE ---|</code></pre>
</div>

- **MODE**
  - _SW access_: rw
  - _Description_: defines the operation mode of the DMA channel, following this scheme:
    - 0: _linear mode_
    - 1: _circular mode_
    - 2: _address mode_

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 1 ----|---- 0 : 0 ----|
|--- Reserved ---|--- DIM_CFG ---|</code></pre>
</div>

- **DIM_CONFIG**
  - _SW access_: rw
  - _Description_: defines the dimensionality of the transaction, i.e. if the transaction is 1D (DMA_DIM = 0) or if it is 2D (DMA_DIM = 1).

  <hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 1 ----|--- 0 : 0 ---|
|--- Reserved ---|---- INV ----|</code></pre>
</div>

- **DIM_INV**
  - _SW access_: rw
  - _Description_: enables the transposition of the input source, only with 2D transactions.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 6 ----|--- 5 : 0 ---|
|--- Reserved ---|---- PAD ----|</code></pre>
</div>

- **PAD_TOP/BOTTOM/RIGHT/LEFT**
  - _SW access_: rw
  - _Description_: defines the size of the padding to be applied to the source, in data units.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 13 ---|--- 12 : 0 ---|
|--- Reserved ---|---- W_SZ ----|</code></pre>
</div>

- **WINDOW_SIZE**
  - _SW access_: rw
  - _Description_: defines the size of the window to be copied.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 8 ----|---- 7 : 0 ---|
|--- Reserved ---|---- W_CNT ---|</code></pre>
</div>

- **WINDOW_COUNT**
  - _SW access_: ro
  - _Description_: indicates the number of times the end of the window was reached since the beginning of the transaction.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 2 ----|-- 1 ---|-- 0 ---|
|--- Reserved ---|- W_DN -|- T_DN -|</code></pre>
</div>

- **INTERRUPT_EN**
  - _SW access_: rw
  - _Description_: enables the interrupt for window and/or transaction done.


<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 1 ----|--- 0 : 0 ---|
|--- Reserved ---|--- FLAG ----|</code></pre>
</div>

- **TRANSACTION_IFR**
  - _SW access_: r0
  - _Description_: interrupt flag register for transaction interrupts. It is set to '1' when the transaction interrupts are enabled and the transaction is completed. It is cleared when it's read by the CPU in the IRQ handler. This feature enables the handler to identify which channel raised the interrupt, more on this mechanism in the functional description paragraph.

<hr>

<div style="text-align: center;">
  <pre style="display: inline-block; text-align: left;"><code>|---- 31 : 1 ----|--- 0 : 0 ---|
|--- Reserved ---|--- FLAG ----|</code></pre>
</div>

- **WINDOW_IFR**
  - _SW access_: r0
  - _Description_: interrupt flag register for window interrupts. It is set to '1' when the window interrupts are enabled and the window is done. It is cleared when it's read by the CPU in the IRQ handler. This feature enables the handler to identify which channel raised the interrupt, more on this mechanism in the functional description paragraph.

  
## Functional description

#### Dictionary

The implementation of this software layer introduced some concepts that need to be understood in order to make proper use of the DMA's functionalities .

  

#### Transaction

A transaction is an operation to be performed by the DMA. It implies copying bytes from a source pointer into a destination pointer. 
The transaction configuration can be cross-checked before loading it into the DMA registers to avoid potential issues. 
The transaction starts only when the size of the *first dimension*, i.e. 1D, of the transaction is written in its corresponding register. The transaction is finished once the DMA has sent all its bytes (which not necessarily means they have been received by the final destination) or when the _external stop signal_ is asserted.

While a transaction is running, new transactions can be validated, loaded and launched, provided they are not targeting the same DMA channel.

Transactions can be re-launched automatically in `circular mode`.

Once the transaction has finished, a status bit is changed (that can be monitored through polling) and a fast interrupt is triggered. 
An interrupt flag register system has been developed to overcome architectural limitations described in [LINK TO STRUCTURAL DESCRIPTION]. As explained in the HAL section [LINK TO HAL SECTION], this system enables the interrupt handler to identify which channel has raised the interrupt.

In the event of a transaction interrupt, the ID of the channel that raised the interrupt is passed to the weak implementation of the handler. This weak handler can be redefined by the user, as demonstrated in example_dma_multichannel.
  

#### Source and destination

Sources and destinations are the two pointers that will exchange data. Bytes will be copied from the source and into the destination address. If the destination size is wider that the source one and sign extension is enabled, the channel will sign extend the data.

  

#### Data type

The DMA allows transactions in chunks of 1, 2 or 4 Bytes (`Byte`, `Half-Word` and `Word` respectively). The size in bytes of the chosen data type is called _data unit_ (usually abbreviated as `du`).

For example, 16 bytes can be 16 data units if the data type is `Byte`, but 8 data units if the data type is `Half Word`.
Source and destination can have different data types and if the destination type is larger than the source type, data can be sign extended.

#### Sign extension
It can be enabled by setting the bit in the corresponding register [LINK SEZIONE REGISTRI].
If the destination data type is larger than the source type, the source data is sign extended to fill up the size of the destination data type.


#### Dimensionality

The DMA can perform both **1D transactions** and **2D transactions**. 
In a 1D transaction, the DMA copies a certain number of elements from the source pointer to the destination pointer using a single increment.
On the other hand, in a 2D transaction the DMA copies data from the source pointer to the destination pointer using two separate increments, which can be interpreted as a 1D and a 2D increment. 
In this way, two-dimensional data manipulations (i.e. matrix manipulations) can be performed in a single DMA transaction.

#### Increment

In a 1D transaction, setting the increment appropriately can be leveraged to achieve non-contiguous read and/or write operations.

For example, let's consider an array of 4 word-type elements. 
To copy just the first 2 bytes of each word, follow these steps:

- Set the datatype of the transaction to **half word**.
- Set the source increment to **2 data units**.
- Set the destination increment to **1 data unit**.

After each reading operation, the DMA will increment the read pointer by 4 bytes (2 data units) and the write pointer by 2 bytes.

In the case of 2D transactions, a second increment must be set to perform matrix manipulations. 
The 2D increment can be interpreted as the number of words that the DMA has to "skip" to move to the next row of the matrix.

For example, let's examine the extraction of a contiguous 2x2 matrix from a 4x4 matrix.

		| 3 | 5 | 7 | 9 |               
		| 2 | 4 | 6 | 8 |           ->           | 3 | 5 |
		| 1 | 3 | 5 | 7 |                        | 2 | 4 |
		| 0 | 2 | 4 | 6 |

The total number of elements copied from the source is 4. 
In order to achieve this result, these are the setup of the increments:
 - For the source:
	- 1D increment set to 1 word 
	- 2D increment set to 3 words
 - For the destination:
	- 1D increment set to 1 word
	- 2D increment set to 1 word

Moreover, by exploiting the 2D increment, it is possible to implement a 2D non-continuous read and/or write.

[INSERIRE ESEMPIO GRAFICO PRESO DAI DISEGNI DELL'IM2COL]

Detailed formulas for the computation of 2D increments are reported in the example application in the `\example_dma_2d` folder.
  
#### Zero padding


The DMA is capable of performing zero padding on the extracted data, both in 1D and 2D transactions, within a single transaction.
This is achieved by setting four padding parameters:

- **T**op							    
- **B**ottom				     					
- **L**eft								
- **R**ight							    

e.g. Let's consider a 2x3 matrix:
		
		| T | T | T | T | T |	
		| L | x | x | x | R |	
		| L | x | x | x | R |	
		| B | B | B | B | B |	


It's important to highlight that the padding is performed (conceptually) only *after* the matrix has been extracted.
So the padding parameters refers only to the extracted matrix, not to the entire source matrix.

Let's revisit the previous 2x2 extraction example, this time adding a left and top padding of 1 word:


	| 3 | 5 | 7 | 9 |                        | 0 | 0 | 0 |
	| 2 | 4 | 6 | 8 |           ->           | 0 | 3 | 5 |
	| 1 | 3 | 5 | 7 |                        | 0 | 2 | 4 |
	| 0 | 2 | 4 | 6 |


#### Alignment

When performing transactions with bytes, the DMA can read/write from any pointer. However, if the data type is larger, words should be aligned so the DMA can perform a read/write operation and affect only the chosen bytes. 
If a word or half-word's pointer is not a multiple of 4 or 2 (respectively), the pointer is _misaligned_. In some cases the DMA HAL can overcome this problem by reducing the data type (which will reflect on an efficiency loss).

  

#### Environment

An environment is a user-defined region of memory that informs the HAL of permissible read/write areas. This ensures that the DMA does not interfere with reserved memory regions.

Read and write permissions are not supported by environments. If an environment is defined, the DMA will have both read and write access to it.
  

#### Target

A target is either a memory region or a peripheral that the DMA can read from or write to. When targets point to memory, they can be assigned an environment to ensure compliance with memory restrictions.

Targets include:

- A pointer (either a memory location or the Rx/Tx buffer for peripherals)
- A size to be copied (if used as a source)
- A data type
- An increment

  

#### Configuration flags

During the creation or configuration of environments, targets or transactions, there could be inconsistencies or threatening situations (like writing outside the boundaries of a defined region). To provide the user with information about this potentially harmful situations, configuration flags are set while creating each of these entities. They can be unmasked and checked.

In some cases, when the threat is too risky, a _crucial error_ might be raised and the operation of the configuration is halted.

If senseless configurations are input to functions, assertions may halt the whole program. This is reserved for extreme situations that mean the program was not properly coded (e.g. a slot value is provided and is not among the available ones).

  

#### Transaction modes

There are three different transaction modes:

**Single Mode:** The default mode, where the DMA channel will perform the copy from the source target to the destination, and trigger an interrupt once done.

**Circular mode:** It takes full advantage of the speed and transparency of the DMA. When selected, the DMA will relaunch the exact same transaction upon finishing. This cycle only stops if by the end of a transaction the _transaction mode_ was changed to _single_. The CPU receives a fast interrupt on every transaction finished.

**Address Mode:** Instead of using the destination pointer and increment to decide where to copy information, an _address list_ must be provided, containing addresses for each data unit being copied. It is only carried out in _single_ mode. 
In this mode it's possible to perform only 1D transactions.

  

#### Windows

In order to process information as it arrives, the application can define a _window size_ (smaller than the _transaction size_). Every time the DMA has finished sending that amount of information, it will trigger an interrupt through the PLIC.

> :warning: If the window size is a multiple of the transaction size, upon finishing the transaction there will be first an interrupt for the whole transaction (through the FIC), and then an interrupt for the window (through the PLIC, which is slower).

  
  

#### Checks and Validations

The DMA HAL's interface functions perform two types of checks:

*  **Sanity checks**: Make sure that each individual value passed as an argument is reasonable and belongs to the proper domain. This errors will raise an _assertion_ and, depending on how assertions are managed in the application, may result in the program crashing.

*  **Integrity checks**: Arguments are cross-checked to make sure that they abide by the rules of the DMA. If configurations are incompatible, contradictory, or a risk for the programs integrity, warnings are raised through the _configuration flags_. In some special cases, a _critical error_ can be raised, which blocks the deployment of the configuration into the DMA registers.

  

> :warning: Integrity checks can be disabled to speed up configuration time. Do this only if you have previously checked the configuration and are sure it will not cause any trouble.

  

Checks and validations are performed during the transactions creation, loading and launching.

  

A transaction is validated if it went through the creation-checks without raising critical errors.

  

#### End events

The DMA considers a certain amount of bytes to have been transferred once it has sent them. It does not wait for a confirmation from the recipient, but can be interrupted at any time using the `ext_dma_stop` signal. 
When a transaction/window is finished the DMA performs a series of events. These may include:

* Changing its status register.

* Raising a _transaction done_ interrupt.

* Raising a _window done_ interrupt.

  

The DMA HAL can follow up on these changes or let the application be in charge of them. For this purpose, three different types of _end events_ are defined:

*  **Polling**: The HAL will disable interrupts from the DMA. The application will need to frequently query the status of the DMA to know when a transaction has finished.

*  **Interrupt**: Interrupts will be enabled. The _window done interrupt_ is enabled if a window size is provided.

*  **Interrupt wait**: The DMA HAL will block the program in a `wfi()` state until the _transaction done interrupt_ is triggered.

  
  

## Operation

This section will explain the operation of the DMA through the DMA HAL.

There is a DMA subsystem instance inside X-HEEP, but others can be connected outside through the bus (see the `example_external_peripheral` application in `sw/aplications/example_external_peripheral/main.c`). 
As long as the DMA instance is the same and the registers are memory mapped with the same structure, the DMA HAL can be used.

  

The DMA HAL adds an extra computational overhead to transactions in order to check the consistency of the transaction configurations. By-passing this layer (and the steps here described) is risky. For the efficiency-hungry applications, doing at least one pass with the whole validation process is recommended. The HAL allows to load and launch transactions with minimum overhead afterwards.

  

The following explanation makes use of Figure 1.

  

![DMA HAL-HW + addresses](https://github.com/esl-epfl/x-heep/assets/54960111/3092faa5-c72c-4cd9-a4d4-4c87de63d1c7)

<p  align="center">Figure 1: Example operation of the DMA and its HAL</p>

  

---

  

The use of the DMA starts with the application creating a set of targets (<span  style="color:red">**a**</span>) . In the figure, the source target is a peripheral connected to an SPI and to the DMA. The address of the reception FIFO (_Rx FIFO_) of this SPI is `0x70` (<span  style="color:red">**b**</span>) . The destination target is a region of memory of address `0x16` (<span  style="color:red">**c**</span>). It is located inside an environment that spans from `0x12` to `0x35`. Any number of environments and targets can be created, and not used.

  

Additionally, in the application the transaction is created. The _operation mode_ and _window size_ are selected.

  

The application calls the validation function of the HAL. If the configurations do not raise a critical flag, it then calls the loading function. By doing so, the desired values are written into their corresponding registers (<span  style="color:red">**d**</span>). The only register that is not immediately written is the _transaction size_, as it is the one responsible for launching the transaction when changed from zero to a non-zero value. It is only written once the application calls the launching function (<span  style="color:red">**e**</span>).

  

Note that there could be some changes between the configuration input in the application and the values written in the registers. For example, a _window size_ of 2 refers to _2 data units_ (i.e. 2 half-words, as such is the data type of the source in Figure 1); however, when writing on the register, this is translated to bytes, so 4 is written instead.

  

Once launched, the transaction will execute completely (it cannot be stopped). Upon finishing, it will check the _operation mode_ and _transaction size_ registers. If any of both is non-zero, it will relaunch. Note that the selecting _circular mode_ during the configuration loading does not launch the transaction (despite what step (<span  style="color:red">**f**</span>) might suggest, which is only illustrative).

  
  

Once the transaction is launched, the DMA will take care of copying as many data units as were requested from the source target (<span  style="color:red">**g**</span>), and pasting them into the destination target (<span  style="color:red">**h**</span>). The data width of the transaction is determined by the _data type_ of the source target (<span  style="color:red">**i**</span>). However, this might be changed (for a smaller width) in case of misalignment. It is possible to reject changes by the DMA HAL and raise an error in case of misalignments instead.

  

The selected slot will query the state of a trigger from the peripheral (<span  style="color:red">**j**</span>). In case the peripheral's FIFO is empty/full (for reception/transmission respectively), the transaction is paused until the trigger enables it again.

  

The source and destination increments will determine the amount of steps the pointer should jump after every read and write, respectively. For peripherals it should always be zero (as it should always take the first element of the FIFO).

  

The DMA will consider a data unit was transferred once it is sent (<span  style="color:red">**k**</span>). It does not wait for an acknowledge by the destination target.

  

The _interrupts_ register controls whether the events triggered by the DMA upon finishing a window or a whole transaction should be propagated as interrupts (<span  style="color:red">**l**</span>).

  

Every time _window size_ data units have been transferred the window count is incremented and a PLIC interrupt is triggered (if enabled) (<span  style="color:red">**m**</span>).

  

Every time a transaction is finished a FIC interrupt is triggered and the restart condition is evaluated (<span  style="color:red">**f**</span>).

  

If the window size is a multiple of the transaction size, upon finishing the transaction there will be first an interrupt for the whole transaction (through the FIC), and then an interrupt for the window (through the PLIC, which is slower).

  
  

The DMA HAL has weak implementations to handle each interrupt. It is up to the application to do something useful with this. the HAL will only forward the interrupt (<span  style="color:red">**n**</span>).

  

> :warning: If the window size is too small (i.e. the time the DMA requires to make the copy is smaller than the time required to attend the interrupt handling), data might be lost. The HAL implements a warning in case the transaction-window size ratio is to small. The warning can be re-configured to an appropriate threshold by overriding a weak implementation. The user should do its own testing and choose this threshold accordingly. The same type of issue might be found on circular mode. The user should be sure it can attend an interrupt before the next one occurs. There is no warning from the HAL in this case.

  

The same result from this example could have been achieved by setting the transaction mode to _address_. It requires an array of destination addresses (<span  style="color:red">**p**</span>) that must be provided as the destination target pointer. Instead of copying information to that pointer, the DMA will read from there and copy the information into the addresses stored in each word (<span  style="color:red">**o**</span>).

This use case is very impractical as it doubles the memory usage. It is intended to be used along In-Memory-Computing architectures and algorithms.

## Software stack: HAL and SDK
  
Like every other computational unit in X-Heep, a DMA transaction can be set up and launched using direct register writes. This method achieves minimal overhead and optimal performance, as thoroughly documented in sw/applications/example_dma_2d and sw/applications/example_dma_multichannel. 

However, it carries significant risks. For instance, the transaction starts immediately after writing to the size register, so the order of register writes must be strictly followed. If the code is compiled with aggressive optimization flags, these write operations might be reordered, potentially compromising DMA functionality. Additionally, errors in setting the transaction size or increments can lead to memory corruption.

Given the criticality of DMA operations and the potential for destructive errors, direct register writes should be approached with utmost _caution_. To mitigate these risks, two software stacks have been developed:

- **HAL**: Provides functions for initializing DMA channels, validating and correcting issues within the targets, loading and launching transactions.
- **SDK**: Employs the HAL to offer user-friendly functions for basic but essential memcpy and fill operations.

### DMA HAL

This section will include a brief overview of the functionalities offered by the DMA HAL. For more practical example, please refer to the next chapter, _"Usecases and examples"_.

Let's start with the structures that enable users to define a DMA transaction and its targets, defined in `dma.h`:


#### <i> dma_target_t </i>

The dma_target_t structure represents a target for a DMA transaction, either as a source or a destination. It encapsulates the parameters required to define a memory region or a peripheral for DMA operations. Furthermore, control parameters can be added to prevent the DMA from reading/writing outside the boundaries of the target.

#### <i> dma_trans_t </i>

The dma_trans_t structure defines a DMA transaction, encapsulating all the necessary parameters and configurations required to perform a DMA operation. Each member of the structure is carefully designed to handle specific aspects of the transaction, from source and destination targets to increment sizes, data types, and operational modes.

Once these structures are defined, let's examine the main functions to be called in order to correctly perform a DMA transaction.

#### <i> dma_init() </i>

*Purpose*:
The dma_init function initializes the DMA subsystem by cresetting transaction structures and clearing DMA registers of each channel.

*Parameters*:
- dma *dma_peri: Pointer to the DMA peripheral. If this pointer is provided, it uses the given DMA peripheral; otherwise (NULL), it uses the integrated DMA peripheral.

#### <i> dma_validate_transaction() </i>

_Purpose_:
The dma_validate_transaction function ensures the configuration of a DMA transaction is correct,checking for potential issues that could prevent the transaction from executing properly. It performs sanity checks, verifies target configurations, and addresses any alignment, increment, padding, trigger, and mode inconsistencies.

_Parameters_:
- dma_trans_t *p_trans: Pointer to the DMA transaction structure that contains the transaction configuration.
- dma_en_realign_t p_enRealign: Flag indicating whether realignment is enabled.
- dma_perf_checks_t p_check: Flag indicating whether integrity checks should be performed.

_Return Values_:
- dma_config_flags_t: Configuration flags indicating the status of the transaction validation. They provide information about the validity of the transaction and any detected errors or warnings.

#### <i> dma_load_transaction() </i>

_Purpose_:
The dma_load_transaction function configures and loads a DMA transaction into a DMA channel. It checks for critical errors defined by the validation function, ensures no other transaction is running, and sets various parameters such as interrupts, pointers, increments, padding, and operation modes by writing in the correct registers. The only register that is not written is the 
SIZE_D1 register so it doesn't launch the transaction.

_Parameters_:
- dma_trans_t *p_trans: Pointer to the DMA transaction structure that contains the configuration for the transaction.

_Return Values_:
- DMA_CONFIG_OK: Indicates that the transaction was successfully loaded.
- DMA_CONFIG_CRITICAL_ERROR: Indicates that the transaction contains a critical error and cannot be loaded.
- DMA_CONFIG_TRANS_OVERRIDE: Indicates that another transaction is currently running and cannot be overridden.

#### <i> dma_launch_transaction() </i>

_Purpose_:
The dma_launch function initiates a DMA transaction that has been previously configured and loaded into a DMA channel. It ensures the transaction is valid, checks for any ongoing transactions, and then starts the new transaction. If the end event is set to wait for an interrupt, the function will block until the interrupt is received.

_Parameters_:
- dma_trans_t *p_trans: Pointer to the DMA transaction structure that contains the configuration for the transaction.

_Return Values_:

- DMA_CONFIG_OK: Indicates that the transaction was successfully launched.
- DMA_CONFIG_CRITICAL_ERROR: Indicates that the transaction could not be launched due to a critical error.
- DMA_CONFIG_TRANS_OVERRIDE: Indicates that another transaction is currently running and cannot be overridden.

The interrupt handlers are critical in many applications. As stated in the structural description of the DMA [LINK STRUCTURAL DESCRIPTION], there is a single interrupt for the entire 

## Usacases and examples

This section will examine and explain several use cases in detail to provide users with a comprehensive understanding of the DMA subsystem and how to leverage it to enhance their application's performance.

Here is a brief overview of the examples:

1) Simple mem2mem transaction, i.e., 1D memcpy
2) 2D mem2mem transaction, i.e., 2D memcpy
3) Matrix transposition
4) Matrix padding
5) Multichannel mem2mem transaction, focusing on the IRQ handler
6) Multichannel flash2mem transaction using the SPI SDK
7) BONUS: Minimizing overhead by using register writes instead of HALs

The complete code for these examples can be found in `sw/applications/example_dma`, `sw/applications/example_dma_2d`, `sw/applications/example_dma_multichannel` and `sw/applications/example_dma_sdk`. These applications offer both verification and performance estimation modes, enabling users to verify the DMA and measure the application's execution time.


### 1. Simple mem2mem transaction

The goal of this example is to develop a function that 


#### Basic application

This example will provide a simplified code for copying data from one region of memory to another. For a real implementation please refer to the `dma_example` application in `sw/applications/dma_example/main.c`.

  

The objective of this app would be to copy the content of an array into another.

  

---

  

Start the application by calling

```C

dma_init( NULL );

```

  

This will reset the DMA registers. The `NULL` parameter tells the HAL that the devices internal DMA is to be used.

  

The most basic implementation requires the creation of **two targets** and a **transaction** relating them both.

  

A target will include the information of the source or destination pointer and the characteristics of the expected transaction.

  

Theoretically, there is no required difference between a _source_ and _destination_ target. They will only be differentiated once the transaction is created. Eventually, they could be interchanged from one transaction to the next. In this example they will be given explicit names and will take different arguments for the sake of clarity.

  

```C
static dma_target_t tgt_src = {
                                .ptr        = copy_buffer,
                                .inc_du     = 1,
                                .size_du    = sizeof(copy_buffer),
                                .type       = DMA_DATA_TYPE_WORD
                                };
```

  

Here, `ptr = copy_buffer` is a `uint32_t` pointer from where the information will be extracted. `inc_du = 1` is telling the DMA that for each word copied, the pointer should be incremented by 1 unit, therefore words will be copied consecutively without gaps.

  

This configuration is implicitly initializing the rest of the target configurations as zero (as of [C99](https://gcc.gnu.org/onlinedocs/gcc/Designated-Inits.html)). This means that:

* No environment is set.

* Data type is set to _word_ (32-bits).

* The trigger is set to _memory_ (vs. a peripheral).

```C
static dma_target_t tgt_dst = {
                                .ptr        = copy_buffer,
                                .inc_du     = 1,
                                .size_du    = sizeof(copy_buffer),
                                .type       = DMA_DATA_TYPE_WORD
                                };
```

Both destination and source targets has to contain a data type (they can be different) and size in data units (they should be the same).

Finally, a transaction is created to relate both targets:

  

```C

static  dma_trans_t trans = {

.src = &tgt_src,

.dst = &tgt_dst,

};

```

  

This will also imply some configurations set to zero:

* Mode is set to singular (only one transaction will be executed).

* There will be no window interrupts.

* The application will have to do polling to know when the transaction has finished.

  

The transaction can now be created (some extra configurations are computed from the targets), loaded into the DMA and launched.

  

```C

dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );

dma_load_transaction( &trans );

dma_launch( &trans );

  

```

  

When creating the transaction, the two last parameters are allowing the DMA to perform integrity checks and try some fixes (at the expense of efficiency) if misalignments are found.

  

As there will be no interrupts set, the application has to check by itself the status of the transaction.

  

```C

while( ! dma_is_ready() ){}

// The transaction has finished!

```

  
  

### Complete Application

The objective of this example is to show various special cases, precautions and considerations that can be taken. For a real example refer to the `spi_flash_write` application in `sw/applications/spi_flash_write/main.c`.

  

This example will copy every other byte from a buffer in RAM to a FLASH connected via an SPI to the DMA. Then, it will copy information from an SPI peripheral continuously into a small buffer.

  

This explanation assumes you have read the previous example.

  

---

  

Start the application by calling

```C

dma_init( NULL );

```

  

The first source target will be pointing to a `uint16_t` buffer in memory. Data will be copied in chunks of 1 byte. For every half-word (16 bits), the only the second half (second 8 bits) will be copied.

  

```C

static  dma_target_t tgt1= {

.ptr = (uint8_t*)(copy_buffer + 1),

.inc_du = 2,

.size_du = sizeof(copy_buffer),

.type = DMA_DATA_TYPE_BYTE,

};

  

```

  

To start copying information from the second byte of `copy_buffer`, the source pointer is set to the address of `copy_buffer` plus one byte.

In order to copy one byte and skip another, the data type is set to byte, and the increment is set to two data units (i.e. two bytes). As for every half-word of the buffer a data unit will be copied, the transaction size is the same as the size of the `copy_buffer`.

  
  

The second target will point to the address of the SPI FLASH transmission FIFO. On this example, this value was already computed and stored in a constant `ADDRESS_SPI_FLASH_TX_FIFO`.

  
  

```C

static  uint32_t *spi_flash_fifo_tx = ADDRESS_SPI_FLASH_TX_FIFO;

  
  

static  dma_target_t tgt2= {

.inc_du = 0,

.trig = DMA_TRIG_SLOT_SPI_FLASH_TX,

};

  

tgt2.ptr = spi_flash_fifo_tx;

  

```

  

Because structure initializers need to be constants, the pointer value (which is a non-constant variable) needs to be initialized outside the designated initializer.

  
  

There is no need to assign a value of transaction size or data type. By default, the DMA will use the source's values. The data type could eventually be modified by the HAL if there is a misalignment and `DMA_ENABLE_REALIGN` is set when creating the transaction.

  

The increment needs to be set to zero as the pointer should always be set to the FIFO address.

  

Because the SPI FLASH transmission FIFO has a line connected to slot number 4 (codified as a `1` in the fourth bit of the `trig` element) to let the DMA know if the FIFO is full, a trigger is set in that position by passing `.trig = DMA_TRIG_SLOT_SPI_FLASH_TX`.

  
  

The transaction is formed by selecting the source and destination targets

  

```C

static  dma_trans_t trans = {

.src = &tgt1,

.dst = &tgt2,

.end = DMA_TRANS_END_INTR,

};

```

  

To enable interrupts, the end event is set to `DMA_TRANS_END_INTR`.

  
  

To create the transaction allowing the DMA to perform necessary realignments and integrity checks such arguments are passed to the creation function along with a pointer to the transaction.

  

```C

dma_config_flags_t res;

res = dma_validate_transaction( &trans, DMA_ENABLE_REALIGN, DMA_PERFORM_CHECKS_INTEGRITY );

```

  

The result variable `res` contains configuration flags that can be used to check that no errors or warnings were raised. These are codified as bits in the 16-bit `res` variable.

If only one flag was raised, the value of `res` will be equal to it. Otherwise, they can be checked by selecting individual bits, with inequalities, or comparing the result to a bitwise-or of flags.

  

```C

if( res == DMA_CONFIG_OK ) // Everything was ok.

if( res & DMA_CONFIG_CRITICAL_ERROR ) // A critical error was found. It has to be fixed or the transaction will not be loaded.

if( res >= DMA_CONFIG_OVERLAP && res =< DMA_CONFIG_TRANS_OVERRIDE ) // One or more warning flags were raised between those two.

if( res & ( DMA_CONFIG_SRC | DMA_CONFIG_DST) ) // Either one of the flags was raised.

```

  

The transaction can then be loaded into the DMA registers by calling the load function

```C

res = dma_load_transaction( &trans );

```

  

and launched

```C

res = dma_load_transaction( &trans );

```

  

Because the DMA will raise an interrupt as soon as it has sent all of its information, it is recommended to wait for the SPI interrupt.

```C

while( spi_intr_flag == 0 )

{

wait_for_interrupt();

}

```

  

If something is to be done as soon as the DMA finishes (like preparing a new transaction) it can be triggered by the interrupt attention routine:

```C

void  dma_intr_handler_trans_done()

{

// Raise a flag to trigger an action

}

```

This function is a strong implementation defined in the application of the weak one in `dma.c`.

  
  

During this transaction

* The DMA grabbed the first byte of from the pointer (i.e. the second of the buffer, letter `B` in the chart below).

* Sent it to the SPI, where it will be treated as a 32-bit word.

* The flash stored the byte wherever it was configured to.

* The source pointer was increased by two data units (i.e. two bytes) to point letter `D` from the chart.

* The DMA grabbed that byte and sent it to the SPI.

* The process went on until the the last byte (letter `L`) is sent to the SPI.

  
  

<style  type="text/css">

.tg {border-collapse:collapse;border-spacing:0;}

.tg  td{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;

overflow:hidden;padding:10px  5px;word-break:normal;}

.tg  th{border-color:black;border-style:solid;border-width:1px;font-family:Arial, sans-serif;font-size:14px;

font-weight:normal;overflow:hidden;padding:10px  5px;word-break:normal;}

.tg  .tg-wp8o{border-color:#000000;text-align:center;vertical-align:top}

</style>

<table  class="tg">

<thead>

<tr>

<td  class="tg-wp8o"  colspan="4">RAM</td>

<td  class="tg-wp8o"  rowspan="5"></td>

<td  class="tg-wp8o"  colspan="4">FLASH</td>

</tr>

<tr>

<td  class="tg-wp8o">A</td>

<td  class="tg-wp8o">B</td>

<td  class="tg-wp8o">C</td>

<td  class="tg-wp8o">D</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">B</td>

</tr>

<tr>

<td  class="tg-wp8o">E</td>

<td  class="tg-wp8o">F</td>

<td  class="tg-wp8o">G</td>

<td  class="tg-wp8o">H</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">D</td>

</tr>

<tr>

<td  class="tg-wp8o">I</td>

<td  class="tg-wp8o">J</td>

<td  class="tg-wp8o">K</td>

<td  class="tg-wp8o">L</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">0</td>

<td  class="tg-wp8o">F</td>

</tr>

<tr>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

<td  class="tg-wp8o">.</td>

</tr>

</thead>

</table>

  
  

---

  
  

For the second part of this example, the targets will be modified and their role switched.

  

The source of the data will be a peripheral connected to the SPI, which will be continuously sending information in half-word format, in streams of 4096 bytes (2048 half-words). Every 1024 half-words there should be a `MILESTONE-CHARACTER`, if there is not, the data flow should be stopped.

  

```C

tgt2.ptr = spi_peripheral_fifo_rx;

tgt2.size_du = 2048;

tgt2.type = DMA_DATA_TYPE_HALF_WORD;

tgt2.trig = DMA_TRIG_SLOT_SPI_RX;

```

  

The source pointer is set to the reception FIFO of the SPI, and the appropriate slot is chosen.

The increment is kept in zero.

  

The destination pointer will be given by a function.

  

```C

tgt1.ptr = (uint8_t*) myApp_getDestination();

```

  

To guarantee that the result of this function will not cause the DMA to write in undesired regions of memory, an environment can be created.

It requires two pointers to the first and last byte where the DMA will be authorized to make any action.

```C

static  dma_env_t safe_zone = {

.start = ADDRESS_START_OF_SAFE_ZONE,

.end = ADDRESS_END_OF_SAFE_ZONE,

};

```

  

The environment can be assigned to the target.

```C

tgt1.env = &safe_zone;

tgt1.inc_du = 1;

```

  

Size and data type is up to the source, and the rest of the configurations are inherited from the last transaction.

  

The transaction is conformed as:

```C

window_size_du = 1024;

  

trans.src = &tgt2;

trans.dst = &tgt1;

trans.mode = DMA_TRANS_MODE_CIRCULAR;

trans.win_du = window_size_du;

  

```

  

Meaning that the information will start to flow into the target 1 from target 2, and every 1024 half-words transferred the application will get an interrupt to check if the `MILESTONE_SYMBOL` is present. Upon finishing the transaction, it shall start again to refill the buffer as it was set in `CIRCULAR_MODE`.

  

The amount of times the buffer was filled will be updated on every _transaction done_ interrupt.

```C

void  dma_intr_handler_trans_done()

{

transaction_count++;

}

```

  

The search for the `MILESTONE_SYMBOL` can be done inside the _window done_ interrupt handling.

```C

void  dma_intr_handler_window_done()

{

/* The current window is obtained. The count is zero when no windows have yet been written. When it is set to one, the window zero is ready. */

window_count = dma_get_window_count() -1;

/* The pointer to the symbol is obtained from the destination pointer + the amount of half-words that have been written. this assumes the symbol is on the first element of each chunk.*/

address = (uint16_t *)trans.dst->ptr + window_count*window_size_du;

symbol = *address;

if( symbol != MILESTONE_SYMBOL )

{

/* If the symbol was not the expected one, future transactions should not be carried out.*/

dma_stop_circular();

/* The number of the first window with error is saved to analyze it later.*/

error_window = error_window == 0 ? window_count : error_window;

}

}

```

> :warning: Interrupt attention routines are advised to be kept as short as possible. This example is merely illustrative.

  

Once the `dma_stop_circular()` function is called, the DMA will still finish the transaction it is currently executing (which could be a whole transaction if the faulty window was the last one). It is important to save valuable information that might be overridden by the ongoing transaction (like the faulty window number).

  
  

## Testing

This section will describe the available example application in X-HEEP using the DMA, and will outline the testing that needs to be performed in order to validate its proper operation.

  
  

### Available Applications

  

There are 6 applications using the DMA:

*  `dma_example`: Tests memory-to-memory transfer, the blocking of transactions while another one is in progress, and window interrupts.

*  `example_external_peripheral`: Tests the use of the DMA HAL one a DMA instance external to X-HEEP. Only available for simulation.

*  `example_virtual_flash`: Tests the transfer to/from an external flash through the DMA.

*  `spi_flash_write`: Tests the transfer to/from the flash. Tests circular mode. Not available on FPGA if linker is `flash-exec`. Should be used with `mcu gen BUS=NtoM CPU=cv32e40p` to test circular mode.

*  `spi_host_dma_exampe`: Test the transfer of data through the SPI host. Not available on Verilator.

*  `spi_host_dma_power_gate_example`: Test the transfer of data through the SPI host. Not available on Verilator.

  
  

## 😎 X-pert Zone:

  

If you know what you are doing and want to minimize the overhead of using the DMA, you can try by-passing the HAL and writing directly on the configuration registers.

  

```c

  

/* We will copy a set of 25 half-words of 16 bits into a buffer of 32-bit words.

Each word in the destination buffer will have its 16 MSB set to 0, and the 16 LSB with the corresponding value from the source.*/

#define HALF_WORDS_TO_COPY 25

static  uint16_t  src_buffer[HALF_WORDS_TO_COPY]; // The source buffer

static  uint32_t  dst_buffer[HALF_WORDS_TO_COPY]; // The destination buffer

  

/* Set the DMA's control block's peripheral structure to point to the address defined in core_v_mini_mcu.h */

dma_cb.peri = dma_peri;

/* Activate interrupts*/

dma_cb.peri->INTERRUPT_EN |= INTR_EN_TRANS_DONE;

/* Set the source and destination pointers*/

dma_cb.peri->SRC_PTR = (uint16_t*) source_buffer;

dma_cb.peri->DST_PTR = (uint32_t*) dst_buffer;

  

/* Set the source increment as 2 bytes (because the source buffer is uint16_t).

Set the destination increment as 4 bytes (because the destination buffer is uint32_t).

We write 1026 = 0000 0100 0000 0010,

as the first 8 LSB refer to the source, and the next 8 bits for the destination. */

dma_cb.peri->PTR_INC = (uint32_t) 1026;

  

/* Make sure that the DMA will point to memory.*/

dma_cb.peri->SLOT = DMA_TRIG_MEMORY;

  

/* Set the data transfer type as half-words.*/

dma_cb.peri->TYPE = DMA_DATA_TYPE_HALF_WORD;

  

/* Set the transaction size, this will launch the transaction.

If you want to restart the same transaction again, just run from here.*/

dma_cb.peri->SIZE = HALF_WORDS_TO_COPY;

  

/* Go to sleep until the DMA finishes.*/

while( dma_cb.peri->STATUS == 0 ) {

/* Disable the interrupts MSTATUS to avoid going to sleep AFTER the interrupt

was triggered.*/

CSR_CLEAR_BITS(CSR_REG_MSTATUS, 0x8);

/* If the transaction has not yet finished, go to sleep*/

if (dma_cb.peri->STATUS == 0) {

/* If a interrupt happened before, the core would still wake-up,

but will not jump to the interrupt handler MSTATUS is not re-set. */

{ asm  volatile("wfi"); }

}

/* Restore the interrupts. */

CSR_SET_BITS(CSR_REG_MSTATUS, 0x8);

}

  
  

```
