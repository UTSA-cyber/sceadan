CONCURRENCY=4			# number of processes to run at once
BLOCKSIZE=512			# size in bytes of blocks to train
SAMPLING=50			# percentage of blocks to sample
MAXSAMPLES=10000		# maximum number of blocks to sample per file
MAXSAMPLESPERFILE=1000		# number of samples for each type
TRAINDIR=../data_train
VECTORDIR=../data_train_vectors
VECTORSC=../data_train_vectors_combined
MODEL=$(VECTORSC).model

$(VECTORSC):
	if [ ! -d  $(TRAINDIR) ]; then mkdir $(TRAINDIR) ; echo put your data in $(TRAINDIR) ; exit 1; fi
	/bin/rm -rf $(VECTORDIR)
	if [ ! -d  $(VECTORDIR) ]; then mkdir $(VECTORDIR) ; fi
	python3  sceadan_train.py --maxsamples=$(MAXSAMPLES) --blocksize=$(BLOCKSIZE) \
		--j=$(CONCURRENCY) --outdir=$(VECTORDIR) $(TRAINDIR)
	/bin/rm -f $(VECTORSC)
	cat $(VECTORDIR)/* > $(VECTORSC)

train:
	python3 grid.py -svmtrain `which train` -log2g null -gnuplot `which gnuplot` v2

clean:
	/bin/rm -rf $(VECTORDIR) $(ALLVECTORS)
