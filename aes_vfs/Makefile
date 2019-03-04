
OBJDIR = obj

LIB_TARGETS += aes_vfs
aes_vfs_TARGET = $(OBJDIR)/libaes_vfs.a
aes_vfs_CXXSRCS = aes_vfs.cc aes_vfs_file.cc \
	page_cache.cc signal_backtrace.cc dll3.cc

LIB_TARGETS += schema
schema_TARGET = $(OBJDIR)/libschema.a
schema_CXXSRCS = $(OBJDIR)/schema.cc $(OBJDIR)/schema.pb.cc
schema_INCS = -I.

PROG_TARGETS += aes_vfs_test_make
aes_vfs_test_make_TARGET = $(OBJDIR)/make
aes_vfs_test_make_CXXSRCS = aes_vfs_test_make.cc
aes_vfs_test_make_INCS = -I.
aes_vfs_test_make_DEFS = -DSQLITE_MAX_MMAP_SIZE=0
aes_vfs_test_make_LIBS = ../sqlite3/sqlite3.o -ldl -lpthread
aes_vfs_test_make_DEPLIBS = $(aes_vfs_TARGET) $(schema_TARGET)

PROG_TARGETS += aes_vfs_test_search
aes_vfs_test_search_TARGET = $(OBJDIR)/search
aes_vfs_test_search_CXXSRCS = aes_vfs_test_search.cc
aes_vfs_test_search_INCS = -I.
aes_vfs_test_search_LIBS = ../sqlite3/sqlite3.o -ldl -lpthread
aes_vfs_test_search_DEPLIBS = $(aes_vfs_TARGET) $(schema_TARGET)

include ../Makefile.inc


$(OBJDIR)/schema.cc $(OBJDIR)/schema.h $(OBJDIR)/schema.proto $(OBJDIR)/schema.pb.cc: test.schema
	mkdir -p $(OBJDIR)
	../build_native/sql3gen test.schema \
		$(OBJDIR)/schema.cc $(OBJDIR)/schema.h \
		$(OBJDIR)/schema.proto
	cd $(OBJDIR) ; $(PROTOC_PATH) --cpp_out=. \
		--proto_path=..:. schema.proto
