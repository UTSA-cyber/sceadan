SCEADAN = sceadan.c sceadan.h sceadan_model_precompiled.c
EXTRA_DIST = model =model.ucv-bcv.20130509.c256.s2.e005

bin_PROGRAMS = sceadan_app mcompile
sceadan_app_SOURCES = main.c $(SCEADAN)
mcompile_SOURCES = mcompile.cpp $(SCEADAN)

sceadan_model_precompiled.c: mcompile
	./mcompile model > sceadan_model_precompiled.c

TESTS = test.sh
