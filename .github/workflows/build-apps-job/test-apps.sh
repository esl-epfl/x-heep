#!/usr/bin/bash -e

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


#############################################################
#			CONFIGURATION PARAMETERS
#############################################################

# List of applications that will not be simulated (skipped)
declare -a BLACKLIST=( "example_freertos_blinky" )

# Simulation timeout to prevent apps from running infinitely
SIM_TIMEOUT_S=120 # This time, in seconds, was chosen empirically.

SIMULATOR='verilator'

LINKER='on_chip'

COMPILER='gcc'

#############################################################
#			VARIABLE INITIALIZATION
#############################################################

# Error vars are not defined if there is problem!
declare -i BUILD_FAILURES=0 &&\
declare -i SIM_FAILURES=0 &&\
declare -i SIM_SKIPPED=0 &&\
APPS=$(\ls sw/applications/) &&\
FAILED='' &&\
SKIPPED=''


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
	# in the black list.
	if ! [[ ${BLACKLIST[*]} =~ "$APP" ]] && [ "$APP_RESULT" == "0" ] ; then
		# The following is done in a very strange way for a reasons:
		# To get the output of the ./Vtestharness, the Makefile cannot be used.
		# To be able to cancel de script and not be inside the simulation directory
		# the parenthesis are needed. However, those parenthesis make the variable
		# out not be accesible from outside. The outmost variable out will contain
		# the output of the execution of the $(sub-script), which happens to be the
		# echo of the variable out. Then only the last character is used, because
		# it contains whether the simulation returned 0 or 1.
		out=$(cd ./build/openhwgroup.org_systems_core-v-mini-mcu_0/sim-verilator; \
		# out=$(./Vtestharness +firmware=../../../sw/build/main.hex); \
		./Vtestharness +firmware=../../../sw/build/main.hex; \
		cd ../../../ ; \
		echo $out; )
		if [ "${out: -1}" == "0" ] ; then
			return 0;
		else
			return 1;
		fi
	else
		return 2;
	fi
}

BUILD (){

	# The app is cleaned and built.
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

}


#############################################################
#				SHOW INFORMATION
#############################################################

echo -e ${LONG_W}

echo -e "${WHITE}Will build using:${RESET}"
echo "${COMPILER[@]}"
echo "${LINKER[@]}"

echo -e ${LONG_W}

echo -e "${WHITE}Will test the following apps:${RESET}"
echo -e $APPS | tr " " "\n"

echo -e ${LONG_W}

echo -e "${WHITE}Will ignore the simulation of:${RESET}"
echo -e "${BLACKLIST[*]}"  | tr ' ' '\n'

echo -e ${LONG_W}

#############################################################
#					SET-UP THE TOOLS
#############################################################

SIM_MODEL_CMD=${SIMULATOR}"-sim"

echo -e ${LONG_W}
echo -e "${WHITE}Building simulation model $SIM_MODEL_CMD ${RESET}"
echo -e ${LONG_W}

make $SIM_MODEL_CMD

#############################################################
#			PERFORM THE BUILDING AND SIMULATIONS
#############################################################

for APP in $APPS
do
	echo -e ${LONG_W}
	echo -e "${WHITE}Now testing $APP ${RESET}"
	echo -e ${LONG_W}

	# The app is built with the set compiler.
	BUILD

	# if [ "$SIMULATOR" != "none" ]; then
	# The simulation is done with a timeout. If an application is failing
	# it will return an error after the timeout has finished.
	# Because the simulation will be called as a sub-process, the returns are
	# analyzed externally (unlike BUILD, that can modify the parent's variables).
	local res=$(SIMULATE)
	#( sleep $SIM_TIMEOUT_S && kill -HUP $pid ) 2>/dev/null & watcher=$!
	#wait $pid 2>/dev/null;
	
	if [ $res -eq 0 ]; then
	 	SIM_SUCCESS
	 	#pkill -HUP -P $watcher
	 	#wait $watcher
	elif [ $res -eq 1 ]; then
	 	SIM_FAIL
	 	#pkill -HUP -P $watcher
	 	#wait $watcher
	elif [ $res -eq 2 ]; then
	 	SIM_SKIP
	 	#pkill -HUP -P $watcher
	 	#wait $watcher
	else
	 	echo -e "${WARNING} Timeout!${RESET}"
	 	SIM_TIMEOUT
	fi
done

#############################################################
#						FINISH UP
#############################################################

echo -e ${LONG_W}

#############################################################
#						SHOW RESULTS
#############################################################

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

if [ $SIM_SKIPPED -gt 0  ]; then
	echo -e ${LONG_WAR}
	echo -e "${WARNING}Simulation of $SIM_SKIPPED apps was skipped${RESET}"
	echo -e $SKIPPED | tr " " "\n"
	echo -e ${LONG_WAR}
fi
