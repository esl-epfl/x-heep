# Define parameters for the test-apps.py script

import os
import subprocess
import re

from git import Repo


class bcolors:
    HEADER = "\033[95m"
    OKBLUE = "\033[94m"
    OKCYAN = "\033[96m"
    OKGREEN = "\033[92m"
    WARNING = "\033[93m"
    FAIL = "\033[91m"
    ENDC = "\033[0m"
    BOLD = "\033[1m"
    UNDERLINE = "\033[4m"


SIMULATOR = "verilator"
SIM_TIMEOUT_S = 300
LINKER = "on_chip"
COMPILER = "gcc"

# Blacklist of apps to skip
blacklist = [
    "example_spi_read",
    "example_spi_host_dma_power_gate",
    "example_spi_write",
]

app_list = [app for app in os.listdir("sw/applications")]

print(bcolors.OKCYAN + "Apps to test:" + bcolors.ENDC)
for app in app_list:
    if app not in blacklist:
        print(bcolors.OKCYAN + f"    - {app}" + bcolors.ENDC)

apps = {app: {"building": "", "simulation": ""} for app in app_list}

# dirty_o = open("dirty_output.txt", "w")

# Compile the {SIMULATOR} model and suppress the output
print(bcolors.OKBLUE + f"Generating {SIMULATOR} model of X-HEEP..." + bcolors.ENDC)
simulation_build_output = subprocess.run(
    ["make", f"{SIMULATOR}-sim"], capture_output=True, check=False
)
# Check if the {SIMULATOR} model was built successfully

if "ERROR:" in str(simulation_build_output.stdout.decode("utf-8")):
    print(bcolors.FAIL + "=====================================" + bcolors.ENDC)
    print(bcolors.FAIL + "Error building verilated model!" + bcolors.ENDC)
    print(bcolors.FAIL + "=====================================" + bcolors.ENDC)
    print(str(simulation_build_output.stdout.decode("utf-8")))
    exit(1)
print(
    bcolors.OKGREEN
    + f"Generated {SIMULATOR} model of X-HEEP successfully!"
    + bcolors.ENDC
)

# dirty_o.write(str(simulation_build_output.stdout.decode('utf-8')) + "\n")

error_pattern = r"Program Finished with value (\d+)"

# Compile every app and run the simulator
for an_app in apps.keys():
    if an_app not in blacklist:
        apps[an_app] = {"building": "OK", "simulation": "OK"}
        print(bcolors.OKBLUE + f"Compiling {an_app}..." + bcolors.ENDC)
        compile_output = subprocess.run(
            [
                "make",
                "app",
                f"PROJECT={an_app}",
                f"COMPILER={COMPILER}",
                f"LINKER={LINKER}",
            ],
            capture_output=True,
            check=False,
        )
        if "Error" in str(compile_output.stdout.decode("utf-8")):
            print(bcolors.FAIL + f"Error compiling {an_app}!" + bcolors.ENDC)
            print(str(compile_output.stdout.decode("utf-8")))
            apps[an_app] = {"building": "Failed", "simulation": "Skipped"}
        else:
            apps[an_app] = {"building": "OK", "simulation": "Skipped"}
            print(bcolors.OKGREEN + f"Compiled successfully {an_app}!" + bcolors.ENDC)
            print(
                bcolors.OKBLUE
                + f"Running {SIMULATOR} simulation of {an_app}..."
                + bcolors.ENDC
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
                    bcolors.FAIL + f"Simulation of {an_app} timed out!" + bcolors.ENDC
                )
                apps[an_app] = {"building": "OK", "simulation": "Timed out"}
            else:
                match = re.search(error_pattern, str(run_output.stdout.decode("utf-8")))
                if (
                    "Error" in str(run_output.stdout.decode("utf-8"))
                    or match.group(1) != "0"
                ):
                    print(
                        bcolors.FAIL
                        + str(run_output.stdout.decode("utf-8"))
                        + bcolors.ENDC
                    )
                    uart_output = open(
                        "build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator/uart0.log",
                        "r",
                    )
                    print(bcolors.FAIL + "UART output:" + bcolors.ENDC)
                    print(bcolors.FAIL + uart_output.read() + bcolors.ENDC)
                    uart_output.close()
                    apps[an_app] = {"building": "OK", "simulation": "Failed"}
                else:
                    apps[an_app] = {"building": "OK", "simulation": "OK"}
                    print(
                        bcolors.OKGREEN + f"Ran {an_app} successfully!" + bcolors.ENDC
                    )
        print(bcolors.OKBLUE + f"Finished running {an_app}." + bcolors.ENDC)
    else:
        print(bcolors.WARNING + f"Skipping {an_app}..." + bcolors.ENDC)
        apps[an_app] = {"building": "Skipped", "simulation": "Skipped"}
    # dirty_o.write(f"Results for {an_app}:\n")
    # dirty_o.write(str(compile_output.stdout.decode('utf-8')) + "\n")
    # dirty_o.write(str(run_output.stdout.decode('utf-8')) + "\n")

# Print the results
print(bcolors.BOLD + "=================================" + bcolors.ENDC)
print(bcolors.BOLD + "Results:" + bcolors.ENDC)
print(bcolors.BOLD + "=================================" + bcolors.ENDC)

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
    bcolors.OKGREEN
    + f"{len(ok_apps)} out of {len(app_list)} apps compiled and ran successfully!"
    + bcolors.ENDC
)
if len(no_build_apps) > 0:
    print(bcolors.FAIL + f"{len(no_build_apps)} apps failed to build!" + bcolors.ENDC)
    for app in no_build_apps:
        print(bcolors.FAIL + f"    - {app}" + bcolors.ENDC)
if len(no_sim_apps) > 0:
    print(bcolors.FAIL + f"{len(no_sim_apps)} apps failed to run!" + bcolors.ENDC)
    for app in no_sim_apps:
        print(bcolors.FAIL + f"    - {app}" + bcolors.ENDC)
if len(skipped_apps) > 0:
    print(bcolors.WARNING + f"{len(skipped_apps)} apps were skipped!" + bcolors.ENDC)
    for app in skipped_apps:
        print(bcolors.WARNING + f"    - {app}" + bcolors.ENDC)
if len(timed_out_apps) > 0:
    print(bcolors.FAIL + f"{len(timed_out_apps)} apps timed out!" + bcolors.ENDC)
    for app in timed_out_apps:
        print(bcolors.FAIL + f"    - {app}" + bcolors.ENDC)
print(bcolors.BOLD + "=================================" + bcolors.ENDC)

if len(no_build_apps) > 0 or len(no_sim_apps) > 0:
    exit(1)

# dirty_o.close()
