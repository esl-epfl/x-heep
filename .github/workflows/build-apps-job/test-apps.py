# Define parameters for the test-apps.py script

import os
import subprocess

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

SIMULATOR     = 'verilator'
SIM_TIMEOUT_S = 120 
LINKER        = 'on_chip'
COMPILER      = 'gcc'

# Blacklist of apps to skip
blacklist = ["hello_world"]

# List of apps to test
app_list = [app for app in os.listdir('sw/applications')]

apps = {
    app : {"building" : "", "simulation" : ""} for app in app_list
}

# Compile the verilator model and suppress the output
simulation_build_output = subprocess.check_output(["make", f"{SIMULATOR}-sim"])

# Check if the verilator model was built successfully

if 'ERROR:' in str(simulation_build_output):
    print(bcolors.FAIL + "=====================================" + bcolors.ENDC)
    print(bcolors.FAIL + "Error building verilated model!" + bcolors.ENDC)
    print(bcolors.FAIL + "=====================================" + bcolors.ENDC)
    print(str(simulation_build_output))
    exit(1)

# Compile every app and run the simulator
for an_app in apps.keys():
    if an_app not in blacklist:
        print(bcolors.OKBLUE + f"Compiling {an_app}..." + bcolors.ENDC)
        compile_output = subprocess.check_output(["make", f"PROJECT={an_app} COMPILER={COMPILER} LINKER={LINKER}"])
        if 'Error' in str(compile_output):
            print(bcolors.FAIL + f"Error compiling {an_app}!" + bcolors.ENDC)
            print(compile_output)
            apps[an_app] = {"building" : "Failed", "simulation" : "Skipped"}
        else:
            print(bcolors.OKGREEN + f"Compiled successfully {an_app}!" + bcolors.ENDC)
            print(bcolors.OKBLUE + f"Running {SIMULATOR} simulation of {an_app}..." + bcolors.ENDC)
            run_output = subprocess.run(["./Vtestharness", "+firmware=../../../sw/build/main.hex"], cwd='build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator', capture_output=True)
            print(run_output.stdout)
            
        #print(run_output)
        print(bcolors.OKGREEN + f"Finished running {an_app}." + bcolors.ENDC)
    else:
        print(bcolors.WARNING + f"Skipping {an_app}..." + bcolors.ENDC)
        apps[an_app] = {"building" : "", "simulation" : "Skipped"}