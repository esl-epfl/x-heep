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
APPS_GCC=$(\ls sw/applications/) &&\

# Applications that should only be compiled with GCC
ONLY_GCC="example_cpp"

# Initialize APPS_CLANG with the same content as APPS_GCC
APPS_CLANG="$APPS_GCC"

# Loop through ONLY_GCC to filter out the specified applications from APPS_CLANG
for app in $ONLY_GCC; do
    # Remove the app from APPS_CLANG
    APPS_CLANG=${APPS_CLANG//$app/}
done

declare -i FAILURES=0 &&\
FAILED='' &&\

echo -e ${LONG_W}
echo -e "Will try building the following apps:${RESET}"
echo -e "----> GCC" 
echo -e $APPS_GCC | tr " " "\n" 
echo -e "----> CLANG" 
echo -e $APPS_CLANG | tr " " "\n" 
echo -e ${LONG_W}

if [ -z "$APPS_GCC" ]; then
        echo -e ${LONG_R}
        echo -e "${RED}No apps found${RESET}"
        echo -e ${LONG_R}
        exit 2
fi

for APP in $APPS_GCC
do 
	
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
		FAILURES=$(( FAILURES + 1 ))
		FAILED="$FAILED(gcc)\t$APP "  
	fi
done

for APP in $APPS_CLANG
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
		FAILURES=$(( FAILURES + 1 ))
		FAILED="$FAILED(clang)\t$APP "  
	fi
done


# Present the results

if [ $FAILURES -gt 0 ]; then
	
	echo -e ${LONG_R}
	echo -e ${LONG_R}
	echo -e "${RED}FAIL: $FAILURES APPS COULD NOT BE BUILT${RESET}"
	echo -e $FAILED | tr " " "\n"
	echo -e ${LONG_R}
	echo -e ${LONG_R}
	exit 1
else 
	
	echo -e ${LONG_G}
	echo -e ${LONG_G}
	echo -e "${WHITE}THE OPERATION SUCCEEDED${RESET}"
	echo -e ${LONG_G}
	echo -e ${LONG_G}
	exit 0
fi


