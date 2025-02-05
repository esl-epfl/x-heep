# Linker Section Configuration

The linker script sections can be configured in two ways.
Either a section can be matched with a group of ram banks
or it can be manually defined.

To automatically add a section the entry `auto_section: auto` should be added to the banks.
It will add a section that matches the banks exactly, it can only be used in the non recursive part.
The name of the section is set with the name of the group of banks.

When manually setting the the linker section the name and the start address have to be provided.
Additionally the end is either provided by it's address or by the size of the section.
If no end is provided the end is inferred by the start of the next linker section.
Or if no section follows, the end address of the last ram bank.

Both configuration types can be freely combined as long as no section overlap.
All sections will e sorted by the configuration system.

The first two sections should always be code and data.
The other name can be used in code with a `.xheep_` prefix, like in `example_matadd_interleaved`

```{code} c
int32_t __attribute__((section(".xheep_data_interleaved"))) m_c[16*16];
```

```{code} js
{
    ram_address: 0
    bus_type: "onetoM",
    ram_banks: {
        code_and_data: {
            num: 2
            sizes: [32]
        }
        i_am_a_section_name: {
            auto_section: auto
            sizes: 16
        }
    }

    linker_sections: 
    [
        {
            name: code
            start: 0

            // Alternatively the end tag can be used to provide the end.
            size: 0x00000C800
        },
        {
            name: data
            start: 0x00000C800
            // The end of this section will be at the beginning of the next.
            // In this example the next section is i_am_a_section_name
        }
    ]
}
```
