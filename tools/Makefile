CONCURRENCY=4			# number of processes to run at once
BLOCKSIZE=512			# size in bytes of blocks to train
SAMPLES=10000			# number of samples for each type
SAMPLING=1			# percentage of blocks to sample
MAXSAMPLES=1000			# maximum number of blocks to stample per file

all:
	python3  sceadan_train.py --blocksize=$(BLOCKSIZE) -j=$(CONCURRENCY)