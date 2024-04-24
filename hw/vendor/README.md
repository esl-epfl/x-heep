To generate the serial-link registers, the repository natively uses Bender that fetches regtool and then generates the peripheral's registers.
To avoid that, we used the reggtool.py already present in the vendorized opentitan repository as follow:

```
cd pulp_platform_serial_link
make update-regs REGGEN=../pulp_platform_register_interface/vendor/lowrisc_opentitan/util/regtool.py
```


