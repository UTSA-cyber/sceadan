SUBDIRS = doc src tools
ACLOCAL_AMFLAGS = -I m4

download:
	(cd src;make download)
	./configure

.PHONY: pull
pull:
	git pull
