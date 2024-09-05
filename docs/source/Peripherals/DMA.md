
# DMA
The **Direct Memory Access (DMA)** peripheral allows data transfers with reduced CPU interaction.
It can perform *transactions* of data between peripherals and memory, or memory-to-memory (as a `memcpy` would).
The CPU is required to configure the transaction, but once launched it is free to go to sleep, process the incoming data or do anything else.
  
The DMA **Hardware Abstraction Layer (HAL)** facilitates the configuration of transactions from the users application. Furthermore, it adds an additional layer of safety checks to reduce the risk of faulty memory accesses, data override or infinite loops.
  
  
## Previous Definitions
The implementation of this software layer introduced some concepts that need to be understood in order to make proper use of the DMA's functionalities .
  
### Transaction
A transaction is an operation to be performed by the DMA. It implies copying bytes from a source pointer into a destination pointer. The transaction configuration can be loaded into the DMA registers once it has been cross-checked and it only starts when the size along the *first dimension* of the transaction is written in its corresponding register. The transaction is finished once the DMA has sent all its bytes (which not necessarily means they have been received by the final destination).
Transactions cannot be stopped once they were launched.
While a transaction is running, new transactions can be validated, but not launched or loaded into the DMA.
Transactions can be re-launched automatically in `circular mode`.
Once the transaction has finished, a status bit is changed (that can be monitored through polling) and a fast interrupt is triggered.
  
### Source and destination
Sources and destinations are the two pointers that will exchange data. Bytes will be copied from the source and into the destination address.
  
### Data type
The DMA allows transactions in chunks of 1, 2 or 4 Bytes (`Byte`, `Half-Word` and `Word` respectively). The size in bytes of the chosen data type is called _data unit_ (usually abbreviated as `du`).
For example, 16 bytes can be 16 data units if the data type is `Byte`, but 8 data units if the data type is `Half Word`.
Source and destination can have different data types, if the destination type is wider than the source type, data can be sign extended.
### Sign extension
If specified (setting the bit in the corresponding register) and if the destination data type is wider than the source type, sign of the source data is extended to fill the size of the destination data type.

### Dimensionality
The DMA can perform both **1D transactions** and **2D transactions**. 
When set to perform 1D transactions, the DMA copies the defined number of elements from the source pointer to the destination pointer using a 1D increment.
When set to perform 2D transactions, the DMA copies data from the source pointer to the destination pointer using both a 1D increment and an additional 2D increment. 
In this way, two-dimensional data manipulations (e.g. matrix manipulations) can be performed in a single DMA transaction, greatly reducing the CPU time.
### Increment
In the 1D configuration, if source and/or destination data are not to be consecutively read/written, a certain increment can be defined.
For instance, if you have an array of 4-bytes-words, but only want to copy the first 2 bytes of each word, you could define the transaction with a data type of half word, an increment of 2 data units in the source, and 1 data unit in the destination. This way, after each read operation the DMA will increment the read pointer in 4 bytes (2 data units), but the write pointer by only 2 bytes.
In the 2D configuration, a second increment has to be set in order to enable the 2D transaction.
In the context of matrix manipulation, the 2D increment is the number of words that the DMA has to "skip" to move to the next row.
For instance, let's examine the extraction of a continuous 2x2 matrix from a 4x4 matrix, stored in a continuous 2x2 matrix.
```
| 3 | 5 | 7 | 9 |    
| 2 | 4 | 6 | 8 | 	->  | 3 | 5 |
| 1 | 3 | 5 | 7 | 	    | 2 | 4 |
| 0 | 2 | 4 | 6 |
```
The total number of elements copied from the source is 4 elements. The increments are:
 - For the source:
	- 1D increment set to 1 word 
	- 2D increment set to 3 words
 - For the destination:
	- 1D increment set to 1 word
	- 2D increment set to 1 word
By exploiting the 2D increment, it's possible to implement a non-continuous read and/or write by computing the correct 2D increment. Detailed formulas for the computation of 2D increments are reported in the *example_dma_2d* folder.
  
### Zero padding

The DMA is capable of performing zero padding on the extracted data, both in 1D and 2D transactions.
This is done by by setting four padding parameters:
- **T**op				    
- **B**ottom				     				
- **L**eft				
- **R**ight		

e.g.

```		
| T | T | T | T | T |	
| L | x | x | x | R |	
| L | x | x | x | R |	
| B | B | B | B | B |	
```
It's important to highlight the fact that the padding is performed (conceptually) only *after* the matrix has been extracted.
Let's examine the previous 2x2 extraction example, adding a left and top padding of 1 word:

```
| 3 | 5 | 7 | 9 |      | 0 | 0 | 0 |
| 2 | 4 | 6 | 8 |  ->  | 0 | 3 | 5 |
| 1 | 3 | 5 | 7 |      | 0 | 2 | 4 |
| 0 | 2 | 4 | 6 |
```

### Alignment
When doing transactions with bytes, the DMA can read/write from any pointer. However, if the data type is larger, words should be aligned so the DMA can perform a read/write operation and affect only the chosen bytes. If a word or half-word's pointer is not a multiple of 4 or 2 (respectively), the pointer is _misaligned_. In some cases the DMA HAL can overcome this problem reducing the data type (which will reflect on an efficiency loss).
  
### Environment
An environment is a region of memory that can optionally be defined by the user to let the HAL know that it is allowed to read/write on that region. It is useful to make sure the DMA will not affect reserved memory regions.
Right now, read and write permissions are not supported by environments, meaning that if it is defined, the DMA will be able to read AND write on it.
  
### Triggers and Slots
If the source or destination pointer is a peripheral, there are lines connecting the peripheral and the DMA that can be used to control the data flow (they behave as _triggers_). These lines are connected to _slots_ on the DMA and they allow/stop the DMA from reading/writing data.
  
### Target
A target is either a region of memory or a peripheral to which the DMA will be able to read/write. When targets are pointing to memory, they can be assigned an environment to make sure that they will comply with memory restrictions.
Targets include a pointer (a point in the memory, or the Rx/Tx buffer in case of peripherals), a size to be copied (if its going to be used as a source), a data type and an increment.
  
### Configuration flags
During the creation or configuration of environments, targets or transactions, there could be inconsistencies or threatening situations (like writing outside the boundaries of a defined region). To provide the user with information about this potentially harmful situations, configuration flags are set while creating each of these entities. They can be unmasked and checked.
In some cases, when the threat is too risky, a _crucial error_ might be raised and the operation of the configuration is halted.
If senseless configurations are input to functions, assertions may halt the whole program. This is reserved for extreme situations that mean the program was not properly coded (e.g. a slot value is provided and is not among the available ones).
  
### Transaction modes
There are three different transaction modes:
**Single Mode:** The default mode, where the DMA will perform the copy from the source target to the destination, and trigger an interrupt once done.
**Circular mode:** To take full advantage of the speed and transparency of the DMA, a _circular_ mode was implemented. When selected, the DMA will relaunch the exactly same transaction upon finishing. This cycle only stops if by the end of a transaction the _transaction mode_ was changed to _single_. The CPU receives a fast interrupt on every transaction finished.
**Address Mode:** Instead of using the destination pointer and increment to decide where to copy information, an _address list_ must be provided, containing addresses for each data unit being copied. It is only carried out in _single_ mode. 
In this mode it's possible to perform only 1D transactions.
  
### Windows
In order to process information as it arrives, the application can define a _window size_ (smaller than the _transaction size_. Every time the DMA has finished sending that given amount of information will trigger an interrupt through the PLIC.
> :warning: If the window size is a multiple of the transaction size, upon finishing the transaction there will be first an interrupt for the whole transaction (through the FIC), and then an interrupt for the window (through the PLIC, which is slower).
  
  
### Checks and Validations
The DMA HAL's interface functions perform two types of checks:
*  **Sanity checks**: Make sure that each individual value passed as an argument is reasonable and belongs to the proper domain. This errors will raise an _assertion_ and, depending on how assertions are managed in the application, may result in the program crashing.
*  **Integrity checks**: Arguments are cross-checked to make sure that they abide by the rules of the DMA. If configurations are incompatible, contradictory, or a risk for the programs integrity, warnings are raised through the _configuration flags_. In some special cases, a _critical error_ can be raised, which blocks the deployment of the configuration into the DMA registers.
  
> :warning: Integrity checks can be disabled to speed up configuration time. Do this only if you have previously checked the configuration and are sure it will not cause any trouble.
  
Checks and validations are performed during the transactions creation, loading and launching.
  
A transaction is validated if it went through the creation-checks without raising critical errors.
  
### End events
The DMA considers a certain amount of bytes to have been transferred once it has sent them. It does not wait for a confirmation from the recipient. When a transaction/window is finished the DMA performs a series of event. These may include:
* Changing its status register.
* Raising a _transaction done_ interrupt.
* Raising a _window done_ interrupt.
  
The DMA HAL can follow up on these changes or let the application be in charge. For this purpose, three different types of _end events_ are defined:
*  **Polling**: The HAL will disable interrupts from the DMA. The application will need to frequently query the status of the DMA to know when a transaction has finished.
*  **Interrupt**: Interrupts will be enabled. The _window done interrupt_ is enabled if a window size is provided.
*  **Interrupt wait**: The DMA HAL will block the program in a `wfi()` state until the _transaction done interrupt_ is triggered.
  
  
## Operation
This section will explain the operation of the DMA through the DMA HAL.
There is a DMA instance inside X-HEEP, but others can be connected outside through the bus (see the `example_external_peripheral` application in `sw/aplications/example_external_peripheral/main.c`). As long as the DMA instance is the same and the registers are memory mapped with the same structure, the DMA HAL can be used.
  
The DMA HAL adds an extra computational overhead to transactions in order to check the consistency of the transaction configurations. By-passing this layer (and the steps here described) is disadvised. For the efficiency-hungry applications, doing at least one pass with the whole validation process is recommended. The HAL allows to load and launch transactions with minimum overhead afterwards.
  
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
  
## Usage
This section will explain a basic usage of the DMA as a `memcpy`, and a slightly more complex situation involving a peripheral connected via an SPI.
  
### Basic application
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
Both destination and source targets have to contain a data type (they can be different) and size in data units (they should be the same).
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
while( spi_intr_flag == 0 ) {
	wait_for_interrupt();
}
```
  
If something is to be done as soon as the DMA finishes (like preparing a new transaction) it can be triggered by the interrupt attention routine:
```C
void  dma_intr_handler_trans_done() {
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
void  dma_intr_handler_trans_done() {
	transaction_count++;
}
```
  
The search for the `MILESTONE_SYMBOL` can be done inside the _window done_ interrupt handling.
```C
void  dma_intr_handler_window_done() {
	/* The current window is obtained. The count is zero when no windows have yet been written. When it is set to one, the window zero is ready. */
	window_count = dma_get_window_count() -1;
	/* The pointer to the symbol is obtained from the destination pointer + the amount of half-words that have been written. this assumes the symbol is on the first element of each chunk.*/
	address = (uint16_t *)trans.dst->ptr + window_count*window_size_du;
	symbol = *address;
	if( symbol != MILESTONE_SYMBOL ){
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
