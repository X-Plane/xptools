#!/bin/sh
if [ $# != 3 ]; then
	echo Usage: "$0 <west> <south> <dirname>"
	exit 1
fi

folder=`./genpath folder $1 $2`
mkdir -p $3/$folder
