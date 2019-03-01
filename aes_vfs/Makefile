
OBJDIR = obj

LIB_TARGETS += aes_vfs
aes_vfs_TARGET = $(OBJDIR)/libaes_vfs.a
aes_vfs_CXXSRCS = aes_vfs.cc

PROG_TARGETS += aes_vfs_test_make
aes_vfs_test_make_TARGET = $(OBJDIR)/aes_vfs_test_make
aes_vfs_test_make_CXXSRCS = aes_vfs_test_make.cc
aes_vfs_test_make_LIBS = ../sqlite3/sqlite3.o -ldl -lpthread
aes_vfs_test_make_DEPLIBS = $(aes_vfs_TARGET)

PROG_TARGETS += aes_vfs_test_search
aes_vfs_test_search_TARGET = $(OBJDIR)/aes_vfs_test_search
aes_vfs_test_search_CXXSRCS = aes_vfs_test_search.cc
aes_vfs_test_search_LIBS = ../sqlite3/sqlite3.o -ldl -lpthread
aes_vfs_test_search_DEPLIBS = $(aes_vfs_TARGET)

include ../Makefile.inc
