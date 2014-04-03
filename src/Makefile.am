SCEADAN = sceadan.cpp sceadan.h sceadan_model_precompiled.c

bin_PROGRAMS = sceadan_app mcompile
sceadan_app_SOURCES = main.c $(SCEADAN)
mcompile_SOURCES = mcompile.cpp $(SCEADAN)

new: mcompile
	./mcompile model > sceadan_model_precompiled.c

TESTS = test.sh
