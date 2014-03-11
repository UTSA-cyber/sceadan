Approach for customers to train and validate sceadan:

1. Create a corpus of TRAIN and TEST documents.         
   - Roughly 20-40 documents per file type.
   - Place documents of each type in their own directory, organized like this:

   TRAIN/<filetype>/document1
   TRAIN/<filetype>/document2
   TEST/<filetype>/document1
   TEST/<filetype>/document2

   e.g.:

   TRAIN/doc/filename.doc
   TRAIN/doc/any_name_is_kay.doc
   TRAIN/docx/another_file.docx
   TRAIN/docx/yet_another_file.docx
...
   TEST/doc/a_test_file.doc

   That is, if you are trying to classify on 20 file types, you should
   have 40 directories with 20-40 documents, each, or between 800 and
   1600 files.

2. Compile sceadan

3. Use tools/sceadan_train.py to create the training vectors:

   tools/sceadan_train.py --build-vectors  --train-dir=<TRAINDIR>  --vector-file=vectors

   (The program will print how many files and blocks it has for each file type, 
   and will then choose a set of random blocks.)

4. Use tools/sceadan_train.py to train the model:

   tools/sceadan_train.py --train-model --vector-file=output --model-file=vectors.model

5. Use tools/sceadan_train.py to print the confusion matrix using the test data:

   tools/sceadan_train.py --validate  --test-dir=<TESTDIR> --model-file=vectors.model









