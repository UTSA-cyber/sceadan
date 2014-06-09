MODEL   = https://www.dropbox.com/s/z3zm2s6c9owic2r/sceadan_model_precompiled.dat
SCEADAN = sceadan.cpp sceadan.h sceadan_model_precompiled.c

DISTCLEANFILES = sceadan_model_precompiled.dat

bin_PROGRAMS = sceadan_app mcompile
lib_LTLIBRARIES = libsceadan.la 

libsceadan_la_SOURCES = $(SCEADAN)
libsceadan_la_LDFLAGS = -static -avoid-version

sceadan_app_SOURCES = main.c $
sceadan_app_LDADD   = libsceadan.la -lstdc++
mcompile_SOURCES    = mcompile.cpp 
mcompile_LDADD      = libsceadan.la -lstdc++

AM_LDFLAGS = -static

download:
	wget $(MODEL)
	@echo 'Type "make distclean" to erase downloaded model.'

new: mcompile
	./mcompile model sceadan_model_precompiled.dat

TESTS = test.sh
