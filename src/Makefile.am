MODEL   = https://www.dropbox.com/s/z3zm2s6c9owic2r/sceadan_model_precompiled.dat
SCEADAN = sceadan.cpp sceadan.h sceadan_model_precompiled.c

LIBLINEAR = ../liblinear/tron.cpp ../liblinear/linear.cpp ../liblinear/blas/blasp.h ../liblinear/blas/daxpy.c ../liblinear/blas/ddot.c \
	../liblinear/blas/dnrm2.c ../liblinear/blas/dscal.c


DISTCLEANFILES = sceadan_model_precompiled.dat

bin_PROGRAMS = sceadan_app mcompile
lib_LTLIBRARIES = libsceadan.la 

libsceadan_la_SOURCES = $(SCEADAN) $(LIBLINEAR)
#libsceadan_la_LDFLAGS = -static -avoid-version -llinear
libsceadan_la_LDFLAGS = -static -avoid-version 

sceadan_app_SOURCES = main.cpp dig.h dig.cpp utf8.h utf8/checked.h utf8/core.h utf8/unchecked.h
sceadan_app_LDADD   = libsceadan.la -llinear -lstdc++
mcompile_SOURCES    = mcompile.cpp 
mcompile_LDADD      = libsceadan.la -llinear -lstdc++ 

AM_LDFLAGS = -static

download:
	wget $(MODEL)
	@echo 'Type "make distclean" to erase downloaded model.'

new: mcompile
	./mcompile model sceadan_model_precompiled.dat

TESTS = test.sh
