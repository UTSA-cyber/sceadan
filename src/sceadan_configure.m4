#
# mix-ins for sceadan
#

AC_MSG_NOTICE([Including sceadan_configure.m4])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_FUNCS([vasprintf])

#
# sceadan_lib_check
#


# -lbz2
AC_CHECK_HEADERS([bzlib.h],,AC_MSG_ERROR([missing bzlib.h]))
AC_CHECK_LIB([bz2],[BZ2_bzCompressInit],,AC_MSG_ERROR([missing -lbz2]))
AC_CHECK_FUNCS([BZ2_bzCompressInit BZ2_bzCompress BZ2_bzCompressEnd],,AC_MSG_ERROR([missing bzip2 functions]))


# -llinear
AC_CHECK_HEADERS([linear.h liblinear/linear.h])
AC_CHECK_LIB([linear],[load_model],,AC_MSG_ERROR([missing -llinear]))
AC_CHECK_FUNCS([load_model check_probability_model predict_probability predict],,AC_MSG_ERROR([missing linear functions]))


# -lm
AC_CHECK_HEADERS([math.h],,AC_MSG_ERROR([missing math.h]))
AC_CHECK_LIB([m],[fmax],,AC_MSG_ERROR([missing -lm]))
AC_CHECK_FUNCS([fmin fmax log exp fabs sqrt],,AC_MSG_ERROR([missing math functions]))


# -lz
AC_CHECK_HEADERS([zlib.h],,AC_MSG_ERROR([missing zlib.h]))
AC_CHECK_LIB([z],[deflate],,AC_MSG_ERROR([missing -lz]))
#AC_CHECK_FUNCS([deflateInit2 deflate deflateEnd],,AC_MSG_ERROR([missing zlib functions]))


#
# sceadan_header_check
#


AC_CHECK_HEADERS([assert.h],,AC_MSG_ERROR([missing assert.h]))
AC_CHECK_HEADERS([ftw.h],,AC_MSG_ERROR([missing ftw.h]))
AC_CHECK_HEADERS([stdbool.h],,AC_MSG_ERROR([missing stdbool.h]))
AC_CHECK_HEADERS([stdio.h],,AC_MSG_ERROR([missing stdio.h]))
AC_CHECK_HEADERS([errno.h],,AC_MSG_ERROR([missing errno.h]))
AC_CHECK_HEADERS([ctype.h],,AC_MSG_ERROR([missing ctype.h]))
AC_CHECK_HEADERS([stdlib.h],,AC_MSG_ERROR([missing stdlib.h]))
AC_CHECK_HEADERS([string.h],,AC_MSG_ERROR([missing string.h]))
AC_CHECK_HEADERS([getopt.h],,AC_MSG_ERROR([missing getopt.h]))
AC_CHECK_HEADERS([limits.h],,AC_MSG_ERROR([missing limits.h]))
AC_CHECK_HEADERS([time.h],,AC_MSG_ERROR([missing time.h]))
AC_CHECK_HEADERS([fcntl.h],,AC_MSG_ERROR([missing fcntl.h]))
AC_CHECK_HEADERS([sys/select.h],,AC_MSG_ERROR([missing sys/select.h]))
AC_CHECK_HEADERS([sys/time.h],,AC_MSG_ERROR([missing sys/time.h]))
AC_CHECK_HEADERS([sys/wait.h],,AC_MSG_ERROR([missing sys/wait.h]))


