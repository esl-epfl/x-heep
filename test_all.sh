#! /usr/bin/bash -e

# Some colors are defined to help debugging in the GitHub console.
WHITE="\033[37;1m"
RED="\033[31;1m"
GREEN="\033[32;1m"
WARNING="\033[33;1m"
RESET="\033[0m"

# Some delimiters are defined to assist the process of detecting each app in the console.
LONG_G="${GREEN}================================================================================${RESET}"
LONG_R="${RED}================================================================================${RESET}"
LONG_W="${WHITE}================================================================================${RESET}"
LONG_WAR="${WARNING}================================================================================${RESET}"

# Error vars are not defined if there is problem!
APPS=$(\ls sw/applications/) &&\
declare -i BUILD_FAILURES=0 &&\
declare -i SIM_FAILURES=0 &&\
declare -i SIM_SKIPPED=0 &&\
FAILED='' &&\

echo -e ${LONG_W}
echo -e "${WHITE}Will try building and simualting the following apps:${RESET}"
echo -e $APPS | tr " " "\n"

echo -e ${LONG_W}

echo -e "${WHITE}Will ignore the simulation of:${RESET}"
echo -e "${BLACKLIST[*]}"  | tr ' ' '\n'

echo -e ${LONG_W}

echo -e "${WHITE}\n\nPlease make sure you have...${RESET}"
echo -e "\t() activated the virtual environment (conda or venv)."
echo -e "\t() Verilator or Questasim installed."
echo -e "\t() some patience."

echo -e "\n\nStart? (y/N)"
read yn

case $yn in
	[Yy]* ) ;;
	[Nn]* ) return;;
	* ) return;;
esac

echo -e ${LONG_W}

if [ -z "$APPS" ]; then
        echo -e ${LONG_R}
        echo -e "${RED}No apps found${RESET}"
        echo -e ${LONG_R}
        exit 2
fi

declare -a BLACKLIST=("example_freertos_blinky" "example_virtual_flash" )

# All peripherals are included to make sure all apps can be built.
sed 's/is_included: "no",/is_included: "yes",/' -i mcu_cfg.hjson

# The MCU is generated with several memory banks to avoid example code not fitting.
make mcu-gen MEMORY_BANKS=3 EXTERNAL_DOMAINS=1


SIMULATOR='verilator'
SIM_MODEL_CMD=${SIMULATOR}"-sim"
SIM_CMD="sim-app-"${SIMULATOR}


USE_GCC=$(which gcc) &&\
USE_CLANG=$(which clang) &&\

echo $USE_GCC
echo $USE_CLANG

make $SIM_MODEL_CMD

for APP in $APPS
do
	echo -e ${LONG_W}
	echo -e "${WHITE}Now testing $APP ${RESET}"
	echo -e ${LONG_W}

	# Build the app with Clang
	if [ -n "${USE_CLANG}" ] ; then
		make --no-print-directory -s app-clean
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
	fi

	# Build the app with GCC
	if [ -n "${USE_GCC}"  ] ; then
		make --no-print-directory -s app-clean
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
	fi

	# Simulate. The result value is stored but not yet used.
	if  [ -n "${USE_GCC}" ] || [ -n "${USE_CLANG}"   ] ; then
		if ! [[ ${BLACKLIST[*]} =~ "$APP" ]]  ; then
			make --no-print-directory -s $SIM_CMD
			res=$?
			if [ "$res" = "0" ] ; then
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
		else
			echo -e ${LONG_WAR}
			echo -e "${WARNING}Will skip simulation for $APP.${RESET}"
			echo -e ${LONG_WAR}
			SIM_SKIPPED=$(( SIM_SKIPPED + 1 ))
		fi
	fi

done


# Reset changes made to files
git stash push mcu_cfg.hjson
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
# Try different linkers
# let modification of certain parameters
