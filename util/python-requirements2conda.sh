#!/usr/bin/env bash
cat <<< 'name: core-v-mini-mcu
channels:
  - defaults
dependencies:
  - python=3.8
  - pip
  - pip:' > environment.yml

lines=`cat python-requirements.txt |sed /^$/d | sed /^#/d`;
while read -r i
do
	echo "    - $i" >> environment.yml ;
done <<< "$lines"
