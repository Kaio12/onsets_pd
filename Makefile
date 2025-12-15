# Makefile to build class 'onsets' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.

# library name
lib.name = onsets

# input source file (class name == source file basename)
class.sources = onsets.c

# all extra files to be included in binary distribution of the library
datafiles = onsets-help.pd onsets-meta.pd README.md

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
PDLIBBUILDER_DIR=/Users/philippecaillot/Documents/programmation/c/tutoC/pd-lib-builder
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder