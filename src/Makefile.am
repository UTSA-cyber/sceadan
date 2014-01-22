SCEADAN = sceadan.c sceadan.h sceadan_model_precompiled.c
EXTRA_DIST = model =model.ucv-bcv.20130509.c256.s2.e005

bin_PROGRAMS = sceadan_app mcompile
sceadan_app_SOURCES = main.c $(SCEADAN)
mcompile_SOURCES = mcompile.cpp

sceadan_model_precompiled.o: mcompile
	./mcompile model > sceadan_model_precompiled.c
	$(CC) $(CPPFLAGS) $(AM_FLAGS) -I${top_srcdir} -I${srcdir} -c sceadan_model_precompiled.c

TESTS = test.sh
