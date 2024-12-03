# TODO

1. DMA extra parameters in `mcu_cfg.hjson`
2. Interrupts in `mcu_cfg.hjson`
3. In `.github/workflows/simulate.yml` the new DMA added the following:

    ~~~hjson
    sed 's/num_channels:   0x1,/num_channels:   0x4,/' -i mcu_cfg.hjson
    sed 's/num_channels_per_master_port: 0x1,/num_channels_per_master_port: 0x4,/' -i mcu_cfg.hjson
    ~~~

4. Can we remove `dma_default_target` from `util/x_heep_gen/system.py`? Now it's only in `sw/device/lib/drivers/dma/dma.h.tpl`.
5. Regenerate `hw/core-v-mini-mcu/core_v_mini_mcu.sv` with the new template.
6. Regenerate `hw/core-v-mini-mcu/peripheral_subsystem.sv` with the new template.
7. Check if the new DMA signals are broken when removing them from the `hw/core-v-mini-mcu/include/core_v_mini_mcu_pkg.sv.tpl`.
8. Are the changes from this PR needed now? How? https://github.com/esl-epfl/x-heep/pull/577
9. Check changes in `docs/source/Peripherals/DMA.md`
10. Add functionality from this PR: https://github.com/esl-epfl/x-heep/pull/539 to `hw/system/pad_control/data/pad_control.hjson.tpl`
11. Regenerate `sw/device/lib/drivers/fast_intr_ctrl/fast_intr_ctrl.c` with the new template.
12. Test with `make mcu-gen X_HEEP_CFG=configs/example.py  && make verilator-sim`
13. Update this PR with the `mcu-gen` changes from:
    - https://github.com/esl-epfl/x-heep/pull/517
    - https://github.com/esl-epfl/x-heep/pull/588
    - https://github.com/esl-epfl/x-heep/pull/581
    These will ahve to get integrated into the new peripheral mcu-gen system. Check again the docs from the project report.
14. Check the two FIXMEs in `sw/device/lib/runtime/core_v_mini_mcu.h.tpl` regarding PRs:
    - https://github.com/esl-epfl/x-heep/pull/523
    - https://github.com/esl-epfl/x-heep/pull/517
    - https://github.com/esl-epfl/x-heep/pull/581
