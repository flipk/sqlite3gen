
OBJDIR = obj

MBEDTLS = $(HOME)/pfk/$(PFKARCH)/mbedtls-2.6.1
MBEDTLS_INCS = -I$(MBEDTLS)/include
MBEDTLS_LIBS = -L$(MBEDTLS)/lib -lmbedcrypto

export PROTOBUF_VERS=2.5.0
PROTOC_PATH = /shared/mh_tools/bin/protoc.sh
PROTO_INCS = -I/shared/mh_tools/tools/protobuf/2.5.0/rhel_5_4/include

SQLITE_INC = -I../sqlite3
SQLITE_LIB = ../sqlite3/sqlite3.o

LIB_TARGETS += aes_vfs
aes_vfs_TARGET = $(OBJDIR)/libaes_vfs.a
aes_vfs_CXXSRCS = aes_vfs.cc aes_vfs_file.cc \
	page_cache.cc signal_backtrace.cc dll3.cc
aes_vfs_INCS = $(SQLITE_INC) $(MBEDTLS_INCS) $(PROTO_INCS)

LIB_TARGETS += schema
schema_TARGET = $(OBJDIR)/libschema.a
schema_CXXSRCS = $(OBJDIR)/schema.cc $(OBJDIR)/schema.pb.cc
schema_INCS = -I. $(SQLITE_INC) $(PROTO_INCS)

PROG_TARGETS += aes_vfs_test_make
aes_vfs_test_make_TARGET = $(OBJDIR)/make
aes_vfs_test_make_CXXSRCS = aes_vfs_test_make.cc
aes_vfs_test_make_INCS = -I. $(SQLITE_INC) $(MBEDTLS_INCS) $(PROTO_INCS)
aes_vfs_test_make_DEFS = -DSQLITE_MAX_MMAP_SIZE=0
aes_vfs_test_make_LIBS = $(SQLITE_LIB) -ldl -lpthread $(MBEDTLS_LIBS)
aes_vfs_test_make_DEPLIBS = $(aes_vfs_TARGET) $(schema_TARGET)

PROG_TARGETS += aes_vfs_test_search
aes_vfs_test_search_TARGET = $(OBJDIR)/search
aes_vfs_test_search_CXXSRCS = aes_vfs_test_search.cc
aes_vfs_test_search_INCS = -I. $(SQLITE_INC) $(MBEDTLS_INCS) $(PROTO_INCS)
aes_vfs_test_search_LIBS = $(SQLITE_LIB) -ldl -lpthread $(MBEDTLS_LIBS)
aes_vfs_test_search_DEPLIBS = $(aes_vfs_TARGET) $(schema_TARGET)

include ../Makefile.inc


$(OBJDIR)/schema.cc $(OBJDIR)/schema.h $(OBJDIR)/schema.proto $(OBJDIR)/schema.pb.cc: test.schema
	mkdir -p $(OBJDIR)
	../build_native/sql3gen test.schema \
		$(OBJDIR)/schema.cc $(OBJDIR)/schema.h \
		$(OBJDIR)/schema.proto
	cd $(OBJDIR) ; $(PROTOC_PATH) --cpp_out=. \
		--proto_path=..:. schema.proto
