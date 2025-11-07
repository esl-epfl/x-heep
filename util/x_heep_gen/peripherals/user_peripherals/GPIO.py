from ..abstractions import UserPeripheral


class GPIO(UserPeripheral):
    """
    General Purpose Input/Output controller.

    Default configuration file: ./hw/vendor/pulp_platform_gpio/gpio_regs.hjson
    """

    _name = "gpio"

