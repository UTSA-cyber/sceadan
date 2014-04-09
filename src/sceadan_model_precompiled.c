#include "config.h"
#ifdef HAVE_LINEAR_H
#  include <linear.h>
#endif
#ifdef HAVE_LIBLINEAR_LINEAR_H
#  include <liblinear/linear.h>
#endif

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
