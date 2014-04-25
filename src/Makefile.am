MODEL   = https://www.dropbox.com/s/z3zm2s6c9owic2r/sceadan_model_precompiled.dat
SCEADAN = sceadan.cpp sceadan.h sceadan_model_precompiled.c

DISTCLEANFILES = sceadan_model_precompiled.dat

bin_PROGRAMS = sceadan_app mcompile
sceadan_app_SOURCES = main.c $(SCEADAN)
mcompile_SOURCES = mcompile.cpp $(SCEADAN)

download:
	wget $(MODEL)
	@echo 'Type "make distclean" to erase downloaded model.'

new: mcompile
	./mcompile model sceadan_model_precompiled.dat

TESTS = test.sh
