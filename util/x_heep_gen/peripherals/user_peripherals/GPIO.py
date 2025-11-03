from ..abstractions import UserPeripheral, DataConfiguration, PeripheralDomain


class GPIO(UserPeripheral, DataConfiguration):
    """
    General Purpose Input/Output controller.

    Default configuration file: ./hw/vendor/pulp_platform_gpio/gpio_regs.hjson
    """

    _name = "gpio"
    _config_path = "./hw/vendor/pulp_platform_gpio/gpio_regs.hjson"
