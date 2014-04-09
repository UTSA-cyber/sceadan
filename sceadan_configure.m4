################################################################
AC_CHECK_HEADERS([linear.h liblinear/linear.h])
AC_CHECK_LIB([linear],[load_model])

# This file may be in several locations depending on how this file is included
AC_CHECK_FILES([sceadan_model_precompiled.dat
                src/sceadan_model_precompiled.dat
                sceadan/src/sceadan_model_precompiled.dat
                src/sceadan/src/sceadan_model_precompiled.dat])

# Check which version of liblinear we have
AC_CHECK_MEMBER([struct parameter.p], 
                [AC_DEFINE(LIBLINEAR_19, 1, [We have liblinear 1.9+])],
                [AC_MSG_WARN([liblinear 1.9 NOT detected. Consider upgrading.])],
        [[#include <linear.h>]])



