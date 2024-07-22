"""
This script compiles and runs all the apps in the sw/applications directory
"""

import os
import subprocess
import re


class BColors:
    """
    Class to define colors in the terminal output.
    """
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


# Define parameters for the test_apps.py script
SIMULATOR = "verilator"
SIM_TIMEOUT_S = 120
LINKER = "on_chip"
COMPILER = "gcc"

# Blacklist of apps to skip
blacklist = [
    "example_spi_read",
    "example_spidma_powergate",
    "example_spi_write",
]

app_list = [app for app in os.listdir("sw/applications")]

print(BColors.OKCYAN + "Apps to test:" + BColors.ENDC)
for app in app_list:
    if app not in blacklist:
        print(BColors.OKCYAN + f"    - {app}" + BColors.ENDC)

apps = {app: {"building": "", "simulation": ""} for app in app_list}


# Compile the {SIMULATOR} model and suppress the output
print(BColors.OKBLUE + f"Generating {SIMULATOR} model of X-HEEP..." + BColors.ENDC)
try:
    simulation_build_output = subprocess.run(
        ["make", f"{SIMULATOR}-sim"], capture_output=True, check=False
    )
except subprocess.CalledProcessError as exc:
    print(BColors.FAIL + "=====================================" + BColors.ENDC)
    print(BColors.FAIL + "Error building verilated model!" + BColors.ENDC)
    print(BColors.FAIL + "=====================================" + BColors.ENDC)
    print(str(exc.stderr.decode("utf-8")))
    exit(1)
else:
    print(
        BColors.OKGREEN
        + f"Generated {SIMULATOR} model of X-HEEP successfully!"
        + BColors.ENDC
    )

error_pattern = r"Program Finished with value (\d+)"

# Compile every app and run the simulator
for an_app in apps.keys():
    if an_app not in blacklist:
        apps[an_app] = {"building": "OK", "simulation": "OK"}
        print(BColors.OKBLUE + f"Compiling {an_app}..." + BColors.ENDC)
        try:
            compile_output = subprocess.run(
                [
                    "make",
                    "app",
                    f"PROJECT={an_app}",
                    f"COMPILER={COMPILER}",
                    f"LINKER={LINKER}",
                ],
                capture_output=True,
                check=True,
            )
        except subprocess.CalledProcessError as exc:
            print(BColors.FAIL + f"Error compiling {an_app}!" + BColors.ENDC)
            print(exc.stderr.decode("utf-8"))
            apps[an_app] = {"building": "Failed", "simulation": "Skipped"}
        else:
            apps[an_app] = {"building": "OK", "simulation": "Skipped"}
            print(BColors.OKGREEN + f"Compiled successfully {an_app}!" + BColors.ENDC)
            print(
                BColors.OKBLUE
                + f"Running {SIMULATOR} simulation of {an_app}..."
                + BColors.ENDC
            )
            try:
                run_output = subprocess.run(
                    ["./Vtestharness", "+firmware=../../../sw/build/main.hex"],
                    cwd="build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator",
                    capture_output=True,
                    timeout=SIM_TIMEOUT_S,
                    check=False,
                )
            except subprocess.TimeoutExpired:
                print(
                    BColors.FAIL + f"Simulation of {an_app} timed out!" + BColors.ENDC
                )
                apps[an_app] = {"building": "OK", "simulation": "Timed out"}
            else:
                match = re.search(error_pattern, str(run_output.stdout.decode("utf-8")))
                if (
                    "Error" in str(run_output.stdout.decode("utf-8"))
                    or match.group(1) != "0"
                ):
                    print(
                        BColors.FAIL
                        + str(run_output.stdout.decode("utf-8"))
                        + BColors.ENDC
                    )
                    uart_output = open(
                        "build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/uart0.log",
                        "r",
                        encoding="utf-8",
                    )
                    print(BColors.FAIL + "UART output:" + BColors.ENDC)
                    print(BColors.FAIL + uart_output.read() + BColors.ENDC)
                    uart_output.close()
                    apps[an_app] = {"building": "OK", "simulation": "Failed"}
                else:
                    apps[an_app] = {"building": "OK", "simulation": "OK"}
                    print(
                        BColors.OKGREEN + f"Ran {an_app} successfully!" + BColors.ENDC
                    )
        print(BColors.OKBLUE + f"Finished running {an_app}." + BColors.ENDC)
    else:
        print(BColors.WARNING + f"Skipping {an_app}..." + BColors.ENDC)
        apps[an_app] = {"building": "Skipped", "simulation": "Skipped"}

# Print the results
print(BColors.BOLD + "=================================" + BColors.ENDC)
print(BColors.BOLD + "Results:" + BColors.ENDC)
print(BColors.BOLD + "=================================" + BColors.ENDC)

# Filter the dictionary by values
ok_apps = [
    app
    for app, status in apps.items()
    if (status["simulation"] == "OK" and status["building"] == "OK")
]
no_build_apps = [app for app, status in apps.items() if status["building"] == "Failed"]
no_sim_apps = [app for app, status in apps.items() if status["simulation"] == "Failed"]
skipped_apps = [
    app
    for app, status in apps.items()
    if (status["simulation"] == "Skipped" or status["building"] == "Skipped")
]
timed_out_apps = [
    app for app, status in apps.items() if status["simulation"] == "Timed out"
]

# Print the filtered results
print(
    BColors.OKGREEN
    + f"{len(ok_apps)} out of {len(app_list)} apps compiled and ran successfully!"
    + BColors.ENDC
)
if len(no_build_apps) > 0:
    print(BColors.FAIL + f"{len(no_build_apps)} apps failed to build!" + BColors.ENDC)
    for failed_build_app in no_build_apps:
        print(BColors.FAIL + f"    - {failed_build_app}" + BColors.ENDC)
if len(no_sim_apps) > 0:
    print(BColors.FAIL + f"{len(no_sim_apps)} apps failed to run!" + BColors.ENDC)
    for failed_run_app in no_sim_apps:
        print(BColors.FAIL + f"    - {failed_run_app}" + BColors.ENDC)
if len(skipped_apps) > 0:
    print(BColors.WARNING + f"{len(skipped_apps)} apps were skipped!" + BColors.ENDC)
    for skipped_app in skipped_apps:
        print(BColors.WARNING + f"    - {skipped_app}" + BColors.ENDC)
if len(timed_out_apps) > 0:
    print(BColors.FAIL + f"{len(timed_out_apps)} apps timed out!" + BColors.ENDC)
    for timed_out_app in timed_out_apps:
        print(BColors.FAIL + f"    - {timed_out_app}" + BColors.ENDC)
print(BColors.BOLD + "=================================" + BColors.ENDC)

if len(no_build_apps) > 0 or len(no_sim_apps) > 0:
    exit(1)
