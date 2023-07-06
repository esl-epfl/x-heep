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
			# The following is done in a very strange way for the following reasons:
			# To get the output of the ./Vtestharness, the Makefile cannot be used.
			# To be able to cancel de script and not be inside the simulation directory
			# the parenthesis are needed. However, those parenthesis make the variable
			# out not be accesible from outside. The outmost variable out will contain
			# the output of the execution of the $(sub-script), which happens to be the
			# echo of the variable out. Then only the last character is used, because
			# it contains whether the simulation returned 0 or 1.
			out=$(cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
				out=$(./Vtestharness +firmware=../../../sw/build/main.hex); \
				cd ../../../ ; \
				echo $out; )
			if [ "${out: -1}" = "0" ] ; then
				echo -e ${LONG_G}
				echo -e "${GREEN}Successfully simulated $APP using $SIMULATOR${RESET}"
				echo -e ${LONG_G}
			else
				echo -e ${LONG_R}
				echo -e "${RED}Failure simulating $APP using $SIMULATOR${RESET}"
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
git status
echo -e "${WHITE}During the execution, some files might have been modified."
echo -e "${WHITE}Do you want to revert all these changes? (Y/n)"
read yn
case $yn in
	[Yy]* ) git stash; git stash drop;;
	[Nn]* ) ;;
	* ) git stash; git stash drop;;
esac

# Present the results
if [ $BUILD_FAILURES -gt 0 ] || [ $SIM_FAILURES -gt 0 ]; then

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
