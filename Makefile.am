SUBDIRS = doc src tools liblinear
ACLOCAL_AMFLAGS = -I m4
DISTCLEANFILES = mp.d penmp.d
AM_CPPFLAGS = -I$(srcdir)/liblinear

.PHONY: download
download:
	(cd src;$(MAKE) download)
	./configure

.PHONY: pull
pull:
	git pull
