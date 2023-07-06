#! /usr/bin/bash

# Some colors are defined to help debugging in the GitHub console.
WHITE="\033[37;1m"
RED="\033[31;1m"
GREEN="\033[32;1m"
RESET="\033[0m"

# Some delimiters are defined to assist the process of detecting each app in the console.
LONG_G="${GREEN}================================================================================${RESET}"
LONG_R="${RED}================================================================================${RESET}"
LONG_W="${WHITE}================================================================================${RESET}"

# Error vars are not defined if there is problem!
APPS=$(\ls sw/applications/) &&\
declare -i BUILD_FAILURES=0 &&\
declare -i SIM_FAILURES=0 &&\
FAILED='' &&\

echo -e ${LONG_W}
echo -e "Will try building and simualting the following apps:${RESET}"
echo -e $APPS | tr " " "\n"
echo -e ${LONG_W}

if [ -z "$APPS" ]; then
        echo -e ${LONG_R}
        echo -e "${RED}No apps found${RESET}"
        echo -e ${LONG_R}
        exit 2
fi


# All peripherals are included to make sure all apps can be built.
sed 's/is_included: "no",/is_included: "yes",/' -i mcu_cfg.hjson
# The MCU is generated with various memory banks to avoid example code not fitting.
make mcu-gen MEMORY_BANKS=3 EXTERNAL_DOMAINS=1

SIMULATOR='verilator'
SIM_MODEL_CMD=${SIMULATOR}"-sim"
SIM_CMD="sim-app-"${SIMULATOR}



make $SIM_MODEL_CMD

for APP in $APPS
do

	# Build the app with Clang
	make app-clean
	if make app PROJECT=$APP COMPILER=clang ; then
		echo -e ${LONG_G}
		echo -e "${GREEN}Successfully built $APP using Clang${RESET}"
		echo -e ${LONG_G}
	else
		echo -e ${LONG_R}
		echo -e "${RED}Failure building $APP using Clang${RESET}"
		echo -e ${LONG_R}
		BUILD_FAILURES=$(( BUILD_FAILURES + 1 ))
		FAILED="$FAILED(clang)\t$APP "
	fi

	# Build the app with GCC
	make app-clean
	if make app PROJECT=$APP ; then
		echo -e ${LONG_G}
		echo -e "${GREEN}Successfully built $APP using GCC${RESET}"
		echo -e ${LONG_G}
	else
		echo -e ${LONG_R}
		echo -e "${RED}Failure building $APP using GCC${RESET}"
		echo -e ${LONG_R}
		BUILD_FAILURES=$(( BUILD_FAILURES + 1 ))
		FAILED="$FAILED(gcc)\t$APP "
	fi

	if  [ "$APP" != "example_freertos_blinky" ] ; then
		if make $SIM_CMD ; then
			echo -e ${LONG_G}
			echo -e "${GREEN}Successfully simulated $APP using $SIMULATOR${RESET}"
			echo -e ${LONG_G}
		else
			echo -e ${LONG_R}
			echo -e "${RED}Failure building $APP using $SIMULATOR${RESET}"
			echo -e ${LONG_R}
			SIM_FAILURES=$(( SIM_FAILURES + 1 ))
			FAILED="$FAILED($SIMULATOR)\t$APP "
		fi
	fi

done


# Reset changes made to files
git stash
git stash drop



# Present the results

if [ $BUILD_FAILURES + $SIM_FAILURES -gt 0 ]; then

	echo -e ${LONG_R}
	echo -e ${LONG_R}
	echo -e "${RED}FAIL: $BUILD_FAILURES APPS COULD NOT BE BUILT${RESET}"
	echo -e "${RED}FAIL: $SIM_FAILURES APPS FAILED THE SIMULATION${RESET}"
	echo -e $FAILED | tr " " "\n"
	echo -e ${LONG_R}
	echo -e ${LONG_R}

else

	echo -e ${LONG_G}
	echo -e ${LONG_G}
	echo -e "${WHITE}THE TEST SUCCEEDED${RESET}"
	echo -e ${LONG_G}
	echo -e ${LONG_G}

fi


# ToDo:
# Make venv or conda
# Select simulator
# Warn if no simulator was chosen
# Fix example_freertos_blinky
# Add watchdog timer
# Fix apps that never finish testing on one simulator
### example_virtual_flash

# Keep a count of apps that return a meaningless execution
# Try different linkers!
# Present what is going to be done and wait for user confirmation

