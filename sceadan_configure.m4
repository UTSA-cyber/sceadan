################################################################
AC_CHECK_HEADERS([linear.h liblinear/linear.h])
AC_CHECK_LIB([m],[fmax],,AC_MSG_ERROR([missing -lm]))
liblinear="no"
AC_CHECK_LIB([linear],[load_model],
              [AC_DEFINE(HAVE_LIBLINEAR,1,[We have liblinear])
               liblinear="yes"],
              [AC_MSG_WARN([Sceadan requires liblinear])])

# This file may be in several locations depending on how this file is included
# But don't do it when cross-compiling
# AC_CHECK_FILES won't check for files when cross-compiling so we do something else

if test -f src/sceadan_model_precompiled.dat ; then
  AC_DEFINE(HAVE_SRC_SCEADAN_MODEL_PRECOMPILED_DAT,1,[we have a precompiled model])
fi

# Check which version of liblinear we have
AC_CHECK_MEMBER([struct parameter.p], 
                [AC_DEFINE(LIBLINEAR_19, 1, [We have liblinear 1.9+])],
                [AC_MSG_WARN([liblinear 1.9 NOT detected. Consider upgrading.])],
        [[#include <linear.h>]])



