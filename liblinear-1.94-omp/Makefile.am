AUTOMAKE_OPTIONS = subdir-objects
BLAS=blas/blas.h blas/blasp.h blas/daxpy.c blas/ddot.c blas/dnrm2.c blas/dscal.c
bin_PROGRAMS = train predict
train_SOURCES = train.c tron.cpp linear.cpp linear.h $(BLAS)
predict_SOURCES = predict.c tron.cpp linear.cpp linear.h $(BLAS)
