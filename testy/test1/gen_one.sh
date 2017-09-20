#!/bin/sh

. ./common

if [ $# != 3 ]; then 
	echo Syntax: $0 shell_path test_directory test_number;
	exit 1;
fi

base_dir=$PWD
shell=$1
test_directory=$2
tn=$3

cd $test_directory

echo -n "Generating " $tn ": " 
head -1 $base_dir/$INPUT_DIR/$tn.in
echo -n "Command:"
echo $shell "<" $base_dir/$INPUT_DIR/$tn.in ">" $base_dir/$EXPECTED_DIR/$tn.out "2>"$base_dir/$EXPECTED_DIR/$tn.err

$shell < $base_dir/$INPUT_DIR/$tn.in > $base_dir/$EXPECTED_DIR/$tn.out 2>$base_dir/$EXPECTED_DIR/$tn.err

echo done.
