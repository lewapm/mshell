#!/bin/sh

. ./common

if [ $# != 2 ]; then 
		echo Syntax: $0 shell_path test_directory;
			exit 1;
		fi

shell=$(readlink -f $1)

echo ":" $shell ":"
if [ -z  $shell ]; then shell=$(which $1); fi 
if [ -z  $shell ]; then 
	echo shell file \" $1 \" not found
	exit 1
fi 

echo GEN TEST OUTPUTS:
for t in $(ls $INPUT_DIR/*.in)
do
	filename=$(basename $t)
	./gen_one.sh $shell $2 ${filename%.in}
done

