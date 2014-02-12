Training Sceadan
================

Sceadan is trained using the liblinear `train` command. This is installed with liblinear and should be in your search path.

Training steps:

1. Place examplar files in the directory `train_data/`. Filetype for each file is determined by its extension.

2. Type 'make'

3. Sceadan will run the script `sceadan_train.py` to generate vectors from the training files.

   - Tuning parameters specified in the Makefile.
   - A file will be created in the directory `train_vectors/` for each data type.
   - All of the vectors will be combined into a single file called `train_vectors.model`
   - The LaTeX file `train_vectors.tex` will be created that indicates the source material.
   - `grid.py` will run on the model to find an optimal training.


Document types should be placed in the directory 