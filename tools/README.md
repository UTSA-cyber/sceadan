Training Sceadan
================

Sceadan is trained using the liblinear `train` command. This is
installed with liblinear and should be in your search path. 

Directories:

$ROOT/data_train - The training data. Extension is assumed to be the
                   file type. Can contain subdirectories
$ROOT/src        - Sceadan source code
$ROOT/src/sceadan_app - generates vectors
$ROOT/tools/     - This directory
$ROOT/tools/Makefile - runs all training
$ROOT/tools/sceadan_train.py - Python program run by Makefile

Training steps:

1. Place examplar files in the directory `$ROOT/train_data/`. Filetype for each file is determined by its extension.

2. cd $ROOT/tools

3. 'make'

4. Sceadan will run the script `sceadan_train.py` to generate vectors from the training files.

   - Tuning parameters specified in the Makefile.
   - A file will be created in the directory `train_vectors/` for each data type.
   - All of the vectors will be combined into a single file called `train_vectors.model`
   - The LaTeX file `train_vectors.tex` will be created that indicates the source material.
   - `grid.py` will run on the model to find an optimal training.


Document types should be placed in the directory 