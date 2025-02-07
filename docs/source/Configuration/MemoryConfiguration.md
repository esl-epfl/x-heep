# Memory Banks Configuration

The system can be configured with banks of different sizes. The sizes should be a power of two in kiB.
All banks are configured in one continuous address region.

To configure interleaved banks the number and the size of the banks have to be provided.
The following restrictions apply: All banks must have the same size and a power of two banks must be configured.

For continuous banks, the default mode, only the `sizes` filed is required.
It can be either the size in kiB of a single bank,
a dictionary of the same format containing more banks, or a list of multiple entries.
If the `num` field is also provided the configuration in the `sizes` field is repeated `num` times.

```{code} js
ram_banks: {
    code: {sizes: 64} // configure just one bank
    
    data: {
        type: continuous // the default, can be omitted
        num: 2
        sizes: 32
    }
    
    alt_data: {sizes: [32, 32]} // the same as data but with a list

    more_complex: {
    // This also works recursively so we can easily have different sizes of banks
    // and in bigger numbers without listing them all one by one.
        sizes: [
            {
                num: 4
                sizes: 8
            },
            {
                num: 16
                sizes: 4
            },
        ]
    }
}
```