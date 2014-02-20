CONCURRENCY=4			# number of processes to run at once
BLOCKSIZE=512			# size in bytes of blocks to train
SAMPLES=10000			# number of samples for each type
SAMPLING=10			# percentage of blocks to sample
MAXSAMPLES=1000			# maximum number of blocks to sample per file
TRAIN=../data_train
OUTDIR=../data_train_vectors

all:
	if [ ! -d  $(TRAIN) ]; then mkdir $(TRAIN) ; echo put your data in $(TRAIN) ; exit 1; fi
	if [ ! -d  $(OUTDIR) ]; then mkdir $(OUTDIR) ; fi
	python3  sceadan_train.py --blocksize=$(BLOCKSIZE) --j=$(CONCURRENCY) --outdir=$(OUTDIR) $(TRAIN)

clean:
	/bin/rm -rf $(OUTDIR)
