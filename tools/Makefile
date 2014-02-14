CONCURRENCY=4			# number of processes to run at once
BLOCKSIZE=512			# size in bytes of blocks to train
SAMPLES=10000			# number of samples for each type
SAMPLING=10			# percentage of blocks to sample
MAXSAMPLES=1000			# maximum number of blocks to sample per file

all:
	if [ ! -d  train_data ]; then mkdir train_data ; echo put your data in train_data ; exit 1; fi
	if [ ! -d  train_vectors ]; then mkdir train_vectors ; fi
	python3  sceadan_train.py --blocksize=$(BLOCKSIZE) --j=$(CONCURRENCY) train_data
