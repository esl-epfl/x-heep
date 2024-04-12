#!/bin/bash
REQUIRED_DIR="/home/ubuntu/Desktop/Xheep/Source/x-heep/scripts"


# The completion function
_completions()
{
  local cur prev opts
  # Get the current working directory
  cur="${COMP_WORDS[COMP_CWORD+1]}"   # The current word being completed
  prev="${COMP_WORDS[COMP_CWORD]}" # The previous word
  preprev="${COMP_WORDS[COMP_CWORD-1]}" # The word before the previous word
  # echo "current: $cur" 
  # echo "previous: $prev"
  # echo "preprevious: $preprev"

  # Options to be completed anywhere
  opts="-make_mcu -make_ver -make_fpga -make_fpga_sw"

  if [[ "$REQUIRED_DIR" == $(pwd) ]]; then

    if [ "$prev" == "" ] && [ "$preprev" != "im2col.sh" ]; then
      COMPREPLY=( $(compgen -W "im2col.sh" -- "$prev") )
      # Check if the previous word is -make_mcu
    elif [[ "$prev" == "-make_mcu" ]]; then
      # Suggestions for after -make_mcu
      COMPREPLY=( $(compgen -W "CPU BUS MEM MEM_IL" -- "$prev") )
    else
      # Default suggestions
      COMPREPLY=( $(compgen -W "$opts" -- "$prev") )
    fi

  fi
}

complete -o bashdefault -F _completions im2col.sh source