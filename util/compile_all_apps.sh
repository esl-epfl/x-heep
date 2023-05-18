#!/bin/sh
# Author : Jose Miranda
# Copyright (c) EPFL
# Script follows here:

# Set the -e option
set -e

path=.
spin='-\|/'

##echo "This is the reviewer script to compile and run $path APPs"
##echo "Do you want to compile<1> or run<2>?"
##read action
action=1

echo "                                                                                    
    .//      .//.         .//      */,   /////////,  */////////  .//////*.   
     (@@&   &@@/          (@@      @@&  *@@&%%%%%%#  @@@%%%%%%%  #@@&%%%@@@& 
       *//%@@#            ./*      @@&  *@@,         @@&         #@@.    ,@@,
        #@@@(    .@@@@@@. (@@@@@@@@@@&  *@@@@@@@@@   @@@@@@@@@*  #@@@@@@@@@# 
      /@@& @@@/           (@@      @@&  *@@.         @@&         #@@.        
    *@@@.   .@@@*         (@@      @@&  *@@@@@@@@@@  @@@@@@@@@@. #@@         
                                                                             
                                                                     "

## Let's see what you want to do ##
compile=0
run=0
if [ $action -eq 1 ]
then
  echo "Let's compile them all"
  compile=1
elif [ $action -eq 2 ]
then
  echo "Let's run them all"
  run=1
else
  echo "Please, type '1' for compiling and '2' for running"
fi

## LINKER array ##
linker_array=( on_chip flash_load flash_exec )

## COMPILER array ##
compiler_array=( gcc clang )

## TARGET array ##
target_array=( sim pynq-z2 )

fail=0
if [ $compile -eq 1 ]
then
  for dir in $path/sw/applications/*/
  do
    for linker in "${linker_array[@]}"
	do 
	  for compiler in "${compiler_array[@]}"
	  do
      	for target in "${target_array[@]}"
	    do
		  echo "compiling app $(basename $dir), ${linker}, ${compiler}, ${target} ..."
	      if make -C $path/ app PROJECT="$(basename $dir)" TARGET="${target}" LINKER="${linker}" COMPILER="${compiler}" &>compilation.out
		  then
		    echo -e "\e[1A\e[Kapp $(basename $dir), ${linker}, ${compiler}, ${target}: Succeeded"
		  else
		    echo -e "\e[1A\e[Kapp $(basename $dir), ${linker}, ${compiler}, ${target}: Failed"
			fail=1
		  fi
	    done	  
	  done
	done    	
  done
elif [ $run -eq 1 ]
then 
  echo "Not implemented yet"
fi

if [ $fail -eq 0 ]
then
  echo "Congrats, you have executed all of them, you are officially the lord of the X-HEEP"
  rm compilation.out
else
  echo "          
                  ___('-&&&-')__
                 '.__./     \__.'
     _     _     _ .-'  6  6 \_
   |   --'( ('--   \         |
  |        ) )      \ \ _   _|
 |        ( (        | (0_._0)
 |         ) )       |/  
 |        ( (        |\_
 |         ) )       |( \,
  \       ((        / )__/
   |     /:))\     |   d
   |    /:((::\    |
   |   |:::):::|   |
   |   |::&&:::|   |
    |  |;U&::U;|   |
    | | | u:u | | |
    | | \     / | |
    | | _|   | _| |
    | |         | |
   | __|       | __|
    '''         '''"
  echo "In case the cow is looking at you, there is something to fix!!!"
  echo "Please, check the 'compilation.out' file and run again the script"
fi



