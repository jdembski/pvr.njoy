#
# Makefile for the XBMC Video Disk Recorder PVR AddOn
#
# See the README for copyright information and
# how to reach the author.
#

LIBS   = -ldl
LIBDIR = ../../addons/pvr.njoy
LIB    = ../../addons/pvr.njoy/XBMC_Njoy.pvr

lib_LTLIBRARIES = libnjoy.la

libnjoy_la_SOURCES = \
	client.cpp \
        xmlParser.cpp \
	N7Xml.cpp 

libnjoy_la_LDFLAGS = \
	-avoid-version


CPPFLAGS = \
	$(ARCH_DEFINES)

INCLUDES = -I$(srcdir)/.. -I$(srcdir)/../.. -I$(srcdir)/../../include/xbmc -D__STDC_CONSTANT_MACROS -D_LINUX

EXTRA_DIST = \
	project

XBMC_Njoy.pvr: libnjoy.la
	cp -f .libs/libnjoy.so XBMC_Njoy.pvr
	$(STRIP) -x XBMC_Njoy.pvr

$(LIB): XBMC_Njoy.pvr
	cp -f XBMC_Njoy.pvr $(LIB)

all: $(LIB)

clean-local:
	-rm -f $(LIB) XBMC_Njoy.pvr

install:

