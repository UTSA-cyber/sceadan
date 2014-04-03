################################################################
AC_CHECK_HEADERS([linear.h liblinear/linear.h])
AC_CHECK_LIB([linear],[load_model])

# Check which version of liblinear we have
AC_CHECK_MEMBER([struct parameter.p], 
                [AC_DEFINE(LIBLINEAR_19, 1, [We have liblinear 1.9+])],
                [AC_DEFINE(LIBLINEAR_18, 1, [We have liblinear 1.8])
                 AC_MSG_WARN([liblinear 1.8 detected. Consider upgrading.])
                 ],
        [[#include <linear.h>]])



