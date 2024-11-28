"""
This script compiles or runs all the apps in X-HEEP

FUTURE WORK:
- The current setup only uses the on_chip linker.
- The current setup only allows for the compilation with one compiler for 
    each application.
"""

import argparse
import os
import subprocess
import re


# Timeout for the simulation in seconds
SIM_TIMEOUT_S = 600

# Available simulators
SIMULATORS = ["verilator"]

# Pattern to look for when simulating an app to see if the app finished
# correctly or not
ERROR_PATTERN_DICT = {
    "verilator": r"Program Finished with value (\d+)",
}

# The following whitelists and blacklists allow for pattern matching
# with their corresponding in_*list() functions. That is, if "example"
# is in a list, in_*list("my_example_app") will return True.

# Whitelist of apps. Has priority over the blacklist.
# Useful if you only want to test certain apps
WHITELIST = []

# Blacklist of apps to skip
BLACKLIST = []

# Blacklist of apps to skip with verilator
VERILATOR_BLACKLIST = []


def in_whitelist(name):
    """
    Checks if the given name is in the WHITELIST.
    """
    return any(word in name for word in WHITELIST)


def in_blacklist(name):
    """
    Checks if the given name is in the BLACKLIST.
    """
    return any(word in name for word in BLACKLIST)


def in_verilator_blacklist(name):
    """
    Checks if the given name is in the VERILATOR_BLACKLIST.
    """
    return any(word in name for word in VERILATOR_BLACKLIST)


class BColors:
    """
    Colors in the terminal output.
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


class SimResult:
    """
    Possible simulation results.
    """

    PASSED = "Passed"
    FAILED = "Failed"
    TIMED_OUT = "Timed out"
    SKIPPED = "Skipped"


class Application:
    """
    Represents an application. Contains its compilation and simulation
    results.
    """

    def __init__(self, name: str):
        self.name = name
        self.compilation_success = False
        self.simulation_results = {}

    def set_compilation_status(self, success: bool):
        self.compilation_success = success

    def add_simulation_result(self, simulator: str, result: SimResult):
        self.simulation_results[simulator] = result


def compile_app(an_app, compiler, linker):
    """
    Compile an_app with the compiler and linker. Outputs if
    it finishes with errors or without.

    Returns True if the compilation succeded and False otherwise.
    """
    print(BColors.OKBLUE + f"Compiling {an_app.name}..." + BColors.ENDC, flush=True)
    try:
        compile_command = ["make", "app", f"PROJECT={an_app.name}"]
        if compiler:
            compile_command.append(f"COMPILER={compiler}")
        if linker:
            compile_command.append(f"LINKER={linker}")

        compile_output = subprocess.run(
            compile_command, capture_output=True, check=True
        )
    except subprocess.CalledProcessError as exc:
        print(BColors.FAIL + f"Error compiling {an_app.name}!" + BColors.ENDC)
        print(exc.stderr.decode("utf-8"), flush=True)
        return False
    else:
        print(
            BColors.OKGREEN + f"Compiled {an_app.name} successfully!" + BColors.ENDC,
            flush=True,
        )
        return True


def run_app(an_app, simulator):
    """
    Runs an_app with the simulator. Checks if it times out. Outputs if
    it finishes with errors or without.

    Returns the SimResult for the simulation of an_app.
    """
    print(
        BColors.OKBLUE + f"Running {an_app.name} with {simulator}..." + BColors.ENDC,
        flush=True,
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
            BColors.FAIL
            + f"Simulation of {an_app.name} with {simulator} timed out!"
            + BColors.ENDC,
            flush=True,
        )
        return SimResult.TIMED_OUT
    else:
        match = re.search(
            ERROR_PATTERN_DICT[simulator], str(run_output.stdout.decode("utf-8"))
        )
        if match and match.group(1) == "0":
            print(
                BColors.OKGREEN
                + f"Ran {an_app.name} with {simulator} successfully!"
                + BColors.ENDC,
                flush=True,
            )
            return SimResult.PASSED
        else:
            print(BColors.FAIL + str(run_output.stdout.decode("utf-8")) + BColors.ENDC)

            # For questasim the build folder is sim-modelsim instead of sim-questasim
            simulator_build_name = simulator if simulator != "questasim" else "modelsim"
            uart_output = open(
                f"build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-{simulator_build_name}/uart0.log",
                "r",
                encoding="utf-8",
            )
            print(BColors.FAIL + "UART output:" + BColors.ENDC)
            print(BColors.FAIL + uart_output.read() + BColors.ENDC, flush=True)
            uart_output.close()
            return SimResult.FAILED


def build_simulator(simulator):
    """
    Build the simulator model.
    """
    print(
        BColors.OKBLUE + f"Generating {simulator} model..." + BColors.ENDC,
        flush=True,
    )
    try:
        simulation_build_output = subprocess.run(
            ["make", f"{simulator}-sim"], capture_output=True, check=True
        )
    except subprocess.CalledProcessError as exc:
        print(BColors.FAIL + "=====================================" + BColors.ENDC)
        print(BColors.FAIL + f"Error building {simulator} model!" + BColors.ENDC)
        print(BColors.FAIL + "=====================================" + BColors.ENDC)
        print(str(exc.stderr.decode("utf-8")), flush=True)
        exit(1)
    else:
        print(
            BColors.OKGREEN
            + f"Generated {simulator} model successfully!"
            + BColors.ENDC,
            flush=True,
        )


def get_apps(apps_dir):
    """
    Get all apps from apps_dir. If the WHITELIST contains any elements,
    it only obtains those apps. Skips the BLACKLIST apps.

    Returns the list of apps.
    """
    if not WHITELIST:
        app_list = [Application(app) for app in os.listdir(apps_dir)]
    else:
        app_list = [
            Application(app) for app in os.listdir(apps_dir) if in_whitelist(app)
        ]

    print(BColors.OKCYAN + "Apps to test from " + apps_dir + ":" + BColors.ENDC)
    for app in app_list:
        if not in_blacklist(app.name):
            print(BColors.OKCYAN + f"    - {app.name}" + BColors.ENDC)

    return app_list


def filter_results(app_list):
    """
    Filters the results from compiling or running the apps and divides
    them into different lists.

    Returns the filtered lists. These are:
    - skipped_apps
    - ok_apps
    - compilation_failed_apps
    - simulation_failed_apps
    """

    skipped_apps = []
    ok_apps = []
    compilation_failed_apps = []
    simulation_failed_apps = []

    for app in app_list:
        if in_blacklist(app.name):
            skipped_apps.append(app)
        elif not app.compilation_success:
            compilation_failed_apps.append(app)
        else:
            all_sim_passed = True
            for sim, res in app.simulation_results.items():
                if res != SimResult.PASSED and res != SimResult.SKIPPED:
                    all_sim_passed = False
            if all_sim_passed:
                ok_apps.append(app)
            else:
                simulation_failed_apps.append(app)

    return skipped_apps, ok_apps, compilation_failed_apps, simulation_failed_apps


def print_results(
    app_list, skipped_apps, ok_apps, compilation_failed_apps, simulation_failed_apps
):
    """
    Print the results of the tests.
    """
    print(BColors.BOLD + "=================================" + BColors.ENDC)
    print(BColors.BOLD + "Results:" + BColors.ENDC)
    print(BColors.BOLD + "=================================" + BColors.ENDC)

    print(
        BColors.OKGREEN
        + f"{len(ok_apps)} out of {len(app_list)} apps finished successfully!"
        + BColors.ENDC
    )

    if len(skipped_apps) > 0:
        print(
            BColors.WARNING + f"{len(skipped_apps)} apps were skipped!" + BColors.ENDC
        )
        for app in skipped_apps:
            print(BColors.WARNING + f"    - {app.name}" + BColors.ENDC)

    if len(compilation_failed_apps) > 0:
        print(
            BColors.FAIL
            + f"{len(compilation_failed_apps)} apps failed to compile!"
            + BColors.ENDC
        )
        for app in compilation_failed_apps:
            print(BColors.FAIL + f"    - {app.name}" + BColors.ENDC)

    if len(simulation_failed_apps) > 0:
        print(
            BColors.FAIL
            + f"{len(simulation_failed_apps)} apps failed to run!"
            + BColors.ENDC
        )
        for app in simulation_failed_apps:
            for sim, res in app.simulation_results.items():
                if res != SimResult.PASSED:
                    print(
                        BColors.FAIL
                        + f"    - {app.name} with {sim} {res}"
                        + BColors.ENDC
                    )

    print(BColors.BOLD + "=================================" + BColors.ENDC, flush=True)


def main():
    parser = argparse.ArgumentParser(description="Test script")
    parser.add_argument(
        "--compile-only", action="store_true", help="Only compile the applications"
    )
    args = parser.parse_args()

    # Get a list with all the applications we want to test
    app_list = get_apps("sw/applications")

    if not args.compile_only:
        for simulator in SIMULATORS:
            build_simulator(simulator)

    # Compile every app and run with the simulators
    for an_app in app_list:
        if not in_blacklist(an_app.name):
            compilation_result = compile_app(an_app, "gcc", "on_chip")
            an_app.set_compilation_status(compilation_result)
            if compilation_result and not args.compile_only:
                for simulator in SIMULATORS:
                    if simulator == "verilator" and in_verilator_blacklist(an_app.name):
                        an_app.add_simulation_result(simulator, SimResult.SKIPPED)
                        print(
                            BColors.WARNING
                            + f"Skipping running {an_app.name} with verilator..."
                            + BColors.ENDC,
                            flush=True,
                        )
                    else:
                        simulation_result = run_app(an_app, simulator)
                        an_app.add_simulation_result(simulator, simulation_result)
        else:
            print(
                BColors.WARNING + f"Skipping {an_app.name}..." + BColors.ENDC,
                flush=True,
            )

    # Filter and print the results
    skipped_apps, ok_apps, compilation_failed_apps, simulation_failed_apps = (
        filter_results(app_list)
    )
    print_results(
        app_list, skipped_apps, ok_apps, compilation_failed_apps, simulation_failed_apps
    )

    # Exit with error if any app failed to compile or run
    if len(compilation_failed_apps) > 0 or len(simulation_failed_apps) > 0:
        exit(1)


if __name__ == "__main__":
    main()
