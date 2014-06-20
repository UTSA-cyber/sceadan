#include "config.h"
#include "../liblinear/linear.h"
#include "sceadan.h"

#ifdef HAVE_SRC_SCEADAN_MODEL_PRECOMPILED_DAT
#  include "sceadan_model_precompiled.dat"
#define HAVE_MODEL
#endif


#ifdef HAVE_SCEADAN_SRC_SCEADAN_MODEL_PRECOMPILED_DAT
#  include "sceadan/src/sceadan_model_precompiled.dat"
#define HAVE_MODEL
#endif

#ifndef HAVE_MODEL
  const struct model *sceadan_model_precompiled(){return 0;}
#endif
