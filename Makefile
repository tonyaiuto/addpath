# Makefile for addpath
#
# Copyright (C) 1999 Tony Aiuto
#
# The MIT License (MIT)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Author: Tony Aiuto (tony.aiuto@gmail.com)

SOURCES=Makefile addpath.c addpath.ref html2troff tests CHANGES
GENERATED=addpath.html addpath.1 addpath.txt
BINS=addpath
FILES=$(SOURCES) $(GENERATED)
RELEASE=2.1.1
PACKAGE=addpath-$(RELEASE).tgz
ZPACKAGE=addpath-$(RELEASE).tar.Z


MACHINE=$(shell uname -n)
OS=$(shell uname)

OS_DEFINES=-DOS=$(OS)

# Try to see if we are compiling under windows
ifeq ($(OS),CYGWIN32_NT)
OS_DEFINES=-DOS=windows -Dwindows=1
endif

ifeq ($(OS),Linux)
CC?=gcc -ansi
endif

CFLAGS=-g -DRELEASE=\"$(RELEASE)\" $(OS_DEFINES)


addpath:	addpath.o
	$(CC) -o $@ addpath.o

all:	$(BINS) $(FILES)


addpath.html:	addpath.ref Makefile
	cat addpath.ref | sed -e "s#@PACKAGE@#$(PACKAGE)#g" \
			| sed -e "s#@ZPACKAGE@#$(ZPACKAGE)#g" >$@

addpath.1:	html2troff addpath.ref
	RELEASE=$(RELEASE) PACKAGE=$(PACKAGE) sh html2troff addpath.ref >$@

addpath.txt:	addpath.1
	nroff -man addpath.1 >$@

install:	addpath addpath.1
	strip addpath
	cp addpath /usr/local/bin
	cp addpath.1 /usr/local/share/man/man1

package:	$(FILES)
	tar czf $(PACKAGE) $(FILES) 

$(ZPACKAGE):
	tar cf - $(FILES) | compress >$(@)

clean:
	rm -f $(GENERATED) $(PACKAGE) $(BINS) *.o

test:	all
	cd tests ; sh basic.sh
