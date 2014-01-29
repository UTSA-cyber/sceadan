#                                               -*- Autoconf -*- 
# Process this file with autoconf to produce a configure script. 
# Order is largely irrevellant, although it must start with AC_INIT and end with AC_OUTPUT 
# See http://autotoolset.sourceforge.net/tutorial.html 
# and http://www.openismus.com/documents/linux/automake/automake.shtml 

AC_INIT(SCEADAN, 1.2.1, bugs@afflib.org)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([m4])

# Programs we will be using
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

m4_include([m4/slg_searchdirs.m4])
m4_include([m4/gcc_all_warnings.m4])

################################################################
# Specify our other headers

AC_HEADER_STDC
AC_CHECK_HEADERS([string.h])

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

AC_CHECK_HEADERS([ assert.h ctype.h errno.h fcntl.h ftw.h getopt.h limits.h stdbool.h stdio.h stdlib.h string.h sys/select.h sys/time.h sys/wait.h time.h ])



############## drop optimization flags if requested ################
############## or if GCC_NOOPT variable is set      ################
AC_ARG_WITH(noopt, AC_HELP_STRING([--with-noopt], [Drop -O C flags]))
if test x"${GCC_NOOPT}" != "x" ; then
   with_noopt="yes";
fi

if test "${with_noopt}" = "yes" ; then
  CFLAGS=`echo "$CFLAGS" | sed s/-O[[0-9]]//`             # note the double quoting! 
  CXXFLAGS=`echo "$CXXFLAGS" | sed s/-O[[0-9]]//`
fi

AC_OUTPUT
