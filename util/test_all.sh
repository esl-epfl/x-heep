#!/usr/bin/bash -e

#############################################################
#					FUNCTION DEFINITIONS
#############################################################

SIM_SUCCESS(){
	echo -e ${LONG_G}
	echo -e "${GREEN}Successfully simulated $APP using $SIMULATOR${RESET}"
	echo -e ${LONG_G}

}

SIM_FAIL(){
	echo -e ${LONG_R}
	echo -e "${RED}Failed simulating $APP using $SIMULATOR${RESET}"
	echo -e ${LONG_R}
	SIM_FAILURES=$(( SIM_FAILURES + 1 ))
	FAILED="$FAILED($SIMULATOR)\t$APP "
}

SIM_TIMEOUT(){
	echo -e ${LONG_WAR}
	echo -e "${WARNING}Timeout simulating $APP using $SIMULATOR${RESET}"
	echo -e ${LONG_WAR}
	SIM_FAILURES=$(( SIM_FAILURES + 1 ))
	FAILED="$FAILED(Timeout)\t$APP "
}

SIM_SKIP(){
	echo -e ${LONG_WAR}
	echo -e "${WARNING}Will skip simulation for $APP.${RESET}"
	echo -e ${LONG_WAR}
	SIM_SKIPPED=$(( SIM_SKIPPED + 1 ))
	SKIPPED="$SKIPPED($SIMULATOR)\t$APP "
}

SIMULATE(){
	# Simulation is only performed if the last compilation succeeded and the app is not
	# in the black list. The compiler to use was chosen before based on its existance.
	if  [ -n "${COMPILER_TO_USE}" ] ; then
		if ! [[ ${BLACKLIST[*]} =~ "$APP" ]] && [ "$APP_RESULT" == "0" ] ; then
			# The following is done in a very strange way for a reasons:
			# To get the output of the ./Vtestharness, the Makefile cannot be used.
			# To be able to cancel de script and not be inside the simulation directory
			# the parenthesis are needed. However, those parenthesis make the variable
			# out not be accesible from outside. The outmost variable out will contain
			# the output of the execution of the $(sub-script), which happens to be the
			# echo of the variable out. Then only the last character is used, because
			# it contains whether the simulation returned 0 or 1.
			case $SIMULATOR in
				"verilator")
					out=$(cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
						out=$(./Vtestharness +firmware=../../../sw/build/main.hex); \
						cd ../../../ ; \
						echo $out; )
					if [ "${out: -1}" == "0" ] ; then
						return 0;
					else
						return 1;
					fi
						;;
				"questasim")
					case $LINKER in
						"flash_load")
							boot_sel="1"
							flash="0"
							;;
						"flash_exec")
							boot_sel="1"
							flash="1"
							;;
						"on_chip")
							boot_sel="0"
							flash="0"
							;;
						*)
							echo -e "${RED}INVALID LINKER: $LINKER!${RESET}"
							return 2;
							;;
					esac

					out=$(cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-modelsim; \
						out=$(make run PLUSARGS="c firmware=../../../sw/build/main.hex boot_sel=$boot_sel execute_from_flash=$flash"); \
						cd ../../../ ; \
						echo $out; )
					if [[ "${out}" == *"# EXIT FAILURE:"* ]] ; then
						return 1;
					else
						return 0;
					fi
					;;
				*)
					echo -e "${RED}INVALID SIMULATOR: $SIMULATOR!${RESET}"
					return 2;
					;;
			esac

		else
			return 2;
		fi
	fi

}

BUILD (){
	for COMPILER in "${COMPILERS[@]}"
	do
		for LINKER in "${LINKERS[@]}"
		do
			# If a compiler is not installed, that compilation is skipped.
			COMPILER_TO_USE=$COMPILER
			make --no-print-directory -s app-clean
			out=$(make -s --no-print-directory app PROJECT=$APP COMPILER=$COMPILER LINKER=$LINKER; val=$?; echo $val)
			APP_RESULT="${out: -1}"
			# The output of the make command is extracted. This way, the output is quite silent
			# but the error information is still printed.
			if [ "$APP_RESULT" == "0" ]; then
				echo -e ${LONG_G}
				echo -e "${GREEN}Successfully built $APP using $COMPILER for $LINKER${RESET}"
				echo -e ${LONG_G}
			else
				echo -e ${LONG_R}
				echo -e "${RED}Failure building $APP using $COMPILER for $LINKER${RESET}"
				echo -e ${LONG_R}
				BUILD_FAILURES=$(( BUILD_FAILURES + 1 ))
				FAILED="$FAILED($COMPILER,$LINKER)\t\t$APP "
			fi
		done
	done
}

#############################################################
#						FORMATTING
#############################################################

# Some colors are defined to help debugging in the console.
WHITE="\033[37;1m "
RED="\033[31;1m "
GREEN="\033[32;1m "
WARNING="\033[33;1m "
RESET="\033[0m"

# Some delimiters are defined to assist the process of detecting each app in the console.
LONG_G="${GREEN}================================================================================${RESET}"
LONG_R="${RED}================================================================================${RESET}"
LONG_W="${WHITE}================================================================================${RESET}"
LONG_WAR="${WARNING}================================================================================${RESET}"

# Error vars are not defined if there is problem!
declare -i BUILD_FAILURES=0 &&\
declare -i SIM_FAILURES=0 &&\
declare -i SIM_SKIPPED=0 &&\
APPS=$(\ls sw/applications/) &&\
FAILED='' &&\
SKIPPED=''

#############################################################
#			CONFIGURATION PARAMETERS
#############################################################

# List of applications that will not be simulated (skipped)
# This list is a temporary solution. Apps should report by themselves
# whether their simulation should be skipped or not.
declare -a BLACKLIST=( "example_virtual_flash" )

# List of possible compilers. The last compiler will be used for simulation.
declare -a COMPILERS=( )

# Simulator tool.
SIMULATOR=''

# List of possible linkers. The last linker will be used for simulation.
declare -a LINKERS=( )

# Simulation timeout to prevent apps from running infinitely
SIM_TIMEOUT_S=240 # This time, in seconds, was chosen empirically.

# Prevent the re-generation of the mcu and the simualtion model on every
# execution by changing DEBUG to 1
DEBUG=0



#############################################################
#					USER ARGUMENTS
#############################################################

for arg in "$@"
do
	case $arg in

		verilator | VERILATOR)
			SIMULATOR='verilator'
			;;

		questasim | questa | QUESTASIM | QUESTA)
			SIMULATOR='questasim'
			;;

		nosim | NOSIM)
			SIMULATOR='none'
			;;

		on_chip | 'on-chip' | ON_CHIP | 'ON-CHIP' | jtag | JTAG)
			LINKER='on_chip'
			if ! [[ " $LINKERS " =~ .*\ $LINKER\ .* ]]; then
                LINKERS+=($LINKER)
            fi
			;;

		flash_load | "flash-load" | FLASH_LOAD | "FLASH-LOAD")
			LINKER='flash_load'
			if ! [[ " $LINKERS " =~ .*\ $LINKER\ .* ]]; then
				LINKERS+=($LINKER)
            fi
			;;

		flash_exec | "flash-exec" | FLASH_EXEC | "FLASH-EXEC")
			LINKER='flash_exec'
			if ! [[ " $LINKERS " =~ .*\ $LINKER\ .* ]]; then
                LINKERS+=($LINKER)
            fi
			;;

		gcc | GCC )
			COMPILER='gcc'
			if ! [[ " $COMPILERS " =~ .*\ $COMPILER\ .* ]]; then
                COMPILERS+=($COMPILER)
            fi
			;;

		clang | CLANG )
			COMPILER='clang'
			if ! [[ " $COMPILERS " =~ .*\ $COMPILER\ .* ]]; then
                COMPILERS+=($COMPILER)
            fi
			;;

		debug)
			DEBUG=1
			;;

		*)
			re='^[0-9]+$'
			if [[ $arg =~ $re ]] ; then
				SIM_TIMEOUT_S=$arg
			else
				echo -e "${RED}$arg is not a valid argument${RESET}"
			fi
			;;

	esac
done



#############################################################
#			SET DEFAULT VALUES IF NON WERE SUPPLIED
#############################################################

if [ -z "$COMPILERS" ]; then
	COMPILERS+="gcc"
fi

if [ -z "$SIMULATOR" ]; then
	SIMULATOR+="verilator"
fi

if [ -z "$LINKERS" ]; then
	LINKERS+="on_chip"
fi

#############################################################
#					CHECKS
#############################################################

# Check that there are some apps to build and simulate
if [ -z "$APPS" ]; then
		echo -e ${LONG_R}
		echo -e "${RED}No apps found${RESET}"
		echo -e ${LONG_R}
		exit 2
fi


# Increase the timeout to be used for questasim


#############################################################
#				SHOW INFORMATION
#############################################################

echo -e ${LONG_W}

echo -e "${WHITE}Will build using:${RESET}"
echo "${COMPILERS[@]}"
echo "${LINKERS[@]}"

echo -e ${LONG_W}

if [ "$SIMULATOR" != "none" ]; then
	echo -e "${WHITE}Will simulate using:${RESET}"
	echo -e "$SIMULATOR (timeout: $SIM_TIMEOUT_S seconds)"
	echo -e ${LONG_W}
fi

echo -e "${WHITE}Will test the following apps:${RESET}"
echo -e $APPS | tr " " "\n"

echo -e ${LONG_W}

echo -e "${WHITE}Will ignore the simulation of:${RESET}"
echo -e "${BLACKLIST[*]}"  | tr ' ' '\n'

echo -e ${LONG_W}

echo -e "${WHITE}\n\nPlease make sure you have...${RESET}"
echo -e "\t() activated the virtual environment (conda or venv)."
echo -e "\t() GCC or CLANG installed in the RISCV toolchain."
echo -e "\t() Verilator or Questasim installed."
echo -e "\t() some patience."

echo -e "\n\nStart? (y/N)"
read yn
case $yn in
	[Yy]* ) ;;
	[Nn]* ) return;;
	* ) return;;
esac

#############################################################
#					SET-UP THE TOOLS
#############################################################

# All peripherals are included to make sure all apps can be built.
sed 's/is_included: "no",/is_included: "yes",/' -i mcu_cfg.hjson

if [ $DEBUG -eq 0 ];	 then
	# The MCU is generated with several memory banks to avoid example code not fitting.
	make mcu-gen X_HEEP_CFG=configs/testall.hjson EXTERNAL_DOMAINS=1

	if [ "$SIMULATOR" != "none" ]; then
	# Make the simualtion model
	SIM_MODEL_CMD=${SIMULATOR}"-sim"

	echo -e ${LONG_W}
	echo -e "${WHITE}Building simulation model $SIM_MODEL_CMD ${RESET}"
	echo -e ${LONG_W}

	if [ "$OPT" == "1" ] && [ "$SIMULATOR" == "questasim" ]; then
		# Perform optimization
		SIM_MODEL_CMD=${SIM_MODEL_CMD}"-opt"
		echo -e ${LONG_W}
		echo -e "${WHITE}Optimizing simulation model ${RESET}"
		echo -e ${LONG_W}
	fi

	make $SIM_MODEL_CMD
	fi
fi

#############################################################
#			PERFORM THE BUILDING AND SIMULATIONS
#############################################################

for APP in $APPS
do
	echo -e ${LONG_W}
	echo -e "${WHITE}Now testing $APP ${RESET}"
	echo -e ${LONG_W}

	COMPILER_TO_USE=""
	# Will store the build result to determine if the app should be simulated or not.
	# Because only the last compiler option is used for simulation, there is not need
	# to keep track of which compilers succeeded and which not.
	APP_RESULT=""

	# The app is built with all the possible compilers.
	BUILD

	if [ "$SIMULATOR" != "none" ]; then
	# The simulation is done with a timeout. If an application is failing
	# it will return an error after the timeout has finished.
	# Because the simulation will be called as a sub-process, the returns are
	# analyzed externally (unlike BUILD, that can modify the parent's variables).
	( SIMULATE ) & pid=$!
	( sleep $SIM_TIMEOUT_S && kill -HUP $pid ) 2>/dev/null & watcher=$!
	wait $pid 2>/dev/null;
	res=$?
	if [ $res -eq 0 ]; then
		SIM_SUCCESS
		pkill -HUP -P $watcher
		wait $watcher
	elif [ $res -eq 1 ]; then
		SIM_FAIL
		pkill -HUP -P $watcher
		wait $watcher
	elif [ $res -eq 2 ]; then
		SIM_SKIP
		pkill -HUP -P $watcher
		wait $watcher
	else
		echo -e "${WARNING} Timeout!${RESET}"
		SIM_TIMEOUT
		fi
	fi
done

#############################################################
#						FINISH UP
#############################################################

# Because there were changed made to the mcu_cfg.json file, and the mcu was
# re-generated, some files were changed with respect to the original state when
# the script was launched.
# Ideally, the user should have commited their changes before sourcing the script,
# but if they didn't they can opt-out of the stashing of the changes and make the stash
# manually themselves.
echo -e ${LONG_W}

git status -uno

echo -e "${WHITE}During the execution, some files might have been modified."
echo -e "${WHITE}Do you want to revert all these changes? (Y/n)${RESET}"
read yn
case $yn in
	[Yy]* ) git restore .;;
	[Nn]* ) ;;
	* ) git restore .;;
esac

#############################################################
#						SHOW RESULTS
#############################################################

if [ $BUILD_FAILURES -gt 0 ] || [ $SIM_FAILURES -gt 0 ]; then

	echo -e ${LONG_R}
	echo -e ${LONG_R}
	echo -e "${RED}FAIL: $BUILD_FAILURES APPS COULD NOT BE BUILT${RESET}"
	if [ "$SIMULATOR" != "none" ]; then
		echo -e "${RED}FAIL: $SIM_FAILURES APPS FAILED THE SIMULATION${RESET}"
	fi
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

if [ "$SIMULATOR" != "none" ]; then
	if [ $SIM_SKIPPED -gt 0  ]; then
		echo -e ${LONG_WAR}
		echo -e "${WARNING}Simulation of $SIM_SKIPPED apps was skipped${RESET}"
		echo -e $SKIPPED | tr " " "\n"
		echo -e ${LONG_WAR}
	fi
fi

#############################################################
#						FUTURE WORK
#############################################################

# Fix apps that never finish testing on (one) simulator
### example_virtual_flash
### example_freertos_blinky

# Keep a count of apps that return a meaningless execution

# Update the condition over which an app is simulated (check 
# both compilation results and not only the last one)