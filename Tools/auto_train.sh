#!/bin/bash
# 
# This script is developed as a tool to automatically train a linear model using liblinear.
# It relies on grid.py to conduct a grid search to find optimal parameters for liblinear.
# For configuration and usage of grid.py, check website:
# http://www.csie.ntu.edu.tw/~cjlin/libsvmtools/grid_parameter_search_for_regression
# This script use solver -- L2-regularized L2-loss support vector classification (primal).
# 
# Usage:
# 	./auto_train input_dir data_name output_dir
# 
# NOTE:
# 	1. First of all, two parameters should be configured before use: GridDir and LiblinearDir
#	2. Three data files should exist - data.grid, data.train, data.test, and should reside in input_dir.
#	   More details about data format, please check README file of LIBSVM.  
#	3. Above data files should have the same name (except postfix), e.g. 'data'
#	4. Three output files will be generated: '.model', '.predict', and '.grid.out'. First two resides 
#	   in output_dir, the last one reside in current working dirrectory.
#

GridDir='< Directory where grid.py reside. >'
LiblinearDir='< Directory where liblinear reside. >'
GridDir='/home/lishu/SVM/libsvm-3.12/python'
LiblinearDir='/home/lishu/SVM/liblinear-1.91'

INPUT=$1	# input path, relative to current working directory
DataName=$2	# data name (without postfix of '.grid', '.train' and '.test')	
OUTPUT=$3	# output path, relative to current working directory
mkdir -p $OUTPUT

# Check if data files exist
if [ ! -f "$INPUT/$DataName.grid" ]
then 
	echo "File '$INPUT/$DataName.grid' does not exitst."
	exit
fi
if [ ! -f "$INPUT/$DataName.train" ]
then 
	echo "File '$INPUT/$DataName.train' does not exitst."
	exit
fi
if [ ! -f "$INPUT/$DataName.test" ]
then 
	echo "File '$INPUT/$DataName.test' does not exitst."
	exit
fi

# grid
echo "Running grid.py ..."
GridOut=`python $GridDir/grid.py $INPUT/$DataName.grid`
arr=(${GridOut// / })
c=${arr[${#arr[@]}-3]}	# get the 3rd to the last as C 
p=${arr[${#arr[@]}-1]}
echo "Optimal C chosen: $c"
echo "Estimated precision: $p"

# train
echo "Training ..."
trainCmd="$LiblinearDir/train -s 2 -c $c $INPUT/$DataName.train $OUTPUT/$DataName.c$c.model"
trainOut=`$trainCmd`
#echo $trainOut >> "$OUTPUT/train.log"	# [optional] redirect training output to log file

# test
echo "Testing ..."
testCmd="$LiblinearDir/predict $INPUT/$DataName.test $OUTPUT/$DataName.c$c.model $OUTPUT/$DataName.c$c.predict"
$testCmd
echo "DONE"
