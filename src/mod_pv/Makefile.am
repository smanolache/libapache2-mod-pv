noinst_LTLIBRARIES = libpvalid.la libmodule.la
pkglib_LTLIBRARIES = mod_pv.la
pkglibdir = $(libdir)/apache2/modules

mod_pv_la_DEPENDENCIES = libpvalid.la libmodule.la

AM_CXXFLAGS = @EXTRA_CXXFLAGS@ @CXXSTD@

libpvalid_la_SOURCES =  \
	Logger.hh \
	url_codec.hh \
	url_codec.cc \
	#

libpvalid_la_CXXFLAGS = \
	$(AM_CXXFLAGS) \
	$(APR_INCLUDES)                    \
	$(APACHE2_INCLUDES)                \
	#

libmodule_la_SOURCES =  \
	ApacheLogger.hh    \
	ApacheLogger.cc    \
	config.hh          \
	config.cc          \
	handler.hh         \
	handler.cc         \
	mod_pv.cc \
	visibility.h \
	#

libmodule_la_CXXFLAGS = -DLOG_NAMESPACE=PV_LOGGER \
	$(AM_CXXFLAGS) \
	$(APR_INCLUDES)                    \
	$(APACHE2_INCLUDES)                \
	#

libmodule_la_LIBADD = \
	#

mod_pv_la_SOURCES =

mod_pv_la_LIBADD = \
	$(top_builddir)/src/mod_pv/libmodule.la \
	$(top_builddir)/src/mod_pv/libpvalid.la \
	-lpv \
	-lstdc++ \
	-lcurl \
	-lapr-1 \
	-lcrypto \
	#

mod_pv_la_LDFLAGS = -version-info 0:0:0 -module -rpath $(libdir)/apache2/modules \
	#

include $(top_srcdir)/coverage.mk
