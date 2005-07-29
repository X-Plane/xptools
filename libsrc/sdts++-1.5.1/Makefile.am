## Process this file with automake to produce Makefile.in

SUBDIRS := sysutils sdts++ doc

EXTRA_DIST :=  tests contrib Windows Jamrules Jamfile project-root.jam boost-build.jam

DISTCHECK_CONFIGURE_FLAGS := --with-boost=$(BOOST_HEADER)

## insure that the sysutils stuff gets configured and built, too

dist-hook:
	for f in $(EXTRA_DIST); do (rm -rf `find $(distdir)/$f -name CVS`); done
