Training Sceadan
================

This document describes the procedure for training Sceadan and
building a custom classifier:

1. Compile sceadan and run the self-test

   $ (cd src ; ./configure ; make ; make check)


2. Make a list of the file types that you wish to classify.

3. Collect at least 40 samples of each file type, where the average file size is 100K - 1MB.

   Ideally, the files should be from the country of interest.  If you
   intend to analyze data from multiple languages, create sub file
   types with the ISO 639-1 language code. e.g. doc-EN, doc-ES, doc-AR  
   (http://www.loc.gov/standards/iso639-2/php/code_list.php)

4. Place the files in a directory hiearchy DATA/TYPE/file  where TYPE
   is the code above (e..g doc-EN, JPG, etc.)

   DATA/<filetype>/document1
   DATA/<filetype>/document2

   e.g.:

   DATA/doc/filename.doc
   DATA/doc/any_name_is_kay.doc
   DATA/docx/another_file.docx
   DATA/docx/yet_another_file.docx
   ...

5. Validate the training data with the sceadan_train.py program:

   $ python3 tools/sceadan_train.py --validate --data=DATA/ --exp=EXPERIMENT/

   --validate   --- Validate what's in DATA
   --data       --- specifies data repository
   --exp        --- Specifies directory for experiment.


FUTURE:
   Create the TEST and TRAIN data sets by sampling the DATA:

   $ python3 tools/sceadan_train.py --split --data=DATA/

   This will create the DATA-TRAIN.txt and DATA-TEST.txt list of files
   for the training and test sets.

Currently, this is a multi-step process; it needs to be a single-step
process:



3. Use tools/sceadan_train.py to create the training vectors:

   tools/sceadan_train.py --build-vectors  --train-dir=<TRAINDIR>  --vector-file=vectors

   (The program will print how many files and blocks it has for each file type, 
   and will then choose a set of random blocks.)

4. Use tools/sceadan_train.py to train the model:

   tools/sceadan_train.py --train-model --vector-file=output --model-file=vectors.model

5. Use tools/sceadan_train.py to print the confusion matrix using the test data:

   tools/sceadan_train.py --validate  --test-dir=<TESTDIR> --model-file=vectors.model










