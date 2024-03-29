
VERSION := $(shell git describe --match 'SQLITE3GEN_v*' --dirty)

ifeq ($(wildcard gtest.config),gtest.config)
include gtest.config
else
$(info gtest.config not found, skipping)
endif

export TARGET=native

ifneq ($(wildcard /etc/fedora-release),)
PROTOC_PATH= protoc
PROTOINC=
PROTOLIB= -lprotobuf
else
ifneq ($(wildcard /etc/redhat-release),)
PROTOC_PATH= /shared/mh_tools/tools/protobuf/2.5.0/rhel_5_4/bin/protoc
PROTOINC= -I/shared/mh_tools/tools/protobuf/2.5.0/rhel_5_4/include
PROTOLIB= -L/shared/mh_tools/tools/protobuf/2.5.0/rhel_5_4/lib -lprotobuf
else
PROTOC_PATH= protoc
PROTOINC=
PROTOLIB= -lprotobuf
endif
endif

OBJDIR = build_$(TARGET)

INCS = $(PROTOINC)
# NOTE protoc 3.6.1 on gcc 9.2.1 won't compile with -Wall -Werror
CXXFLAGS = # -Wall -Werror

PROG_TARGETS += template_to_c
template_to_c_TARGET = $(OBJDIR)/template_to_c
template_to_c_CXXSRCS = template_to_c.cc
ifneq ($(TRUE_LINE_NUMBERS),)
template_to_c_DEFS = -DTRUE_LINE_NUMBERS=1
endif

TEMPLATES = \
	header proto \
	source_top \
	source_alltabs \
	source_custom_select \
	source_customs \
	source_subtable \
	source_proto \
	source_table \
	source_table_get \
	source_table_insert \
	source_xml

TEMPLATE_OBJS = $(foreach t,$(TEMPLATES),$(OBJDIR)/template_$(t).o)

PROG_TARGETS += sql3gen
sql3gen_TARGET = $(OBJDIR)/sql3gen
sql3gen_LLSRCS = tokenizer.ll
sql3gen_YYSRCS = parser.yy
sql3gen_CXXSRCS = main.cc template_patterns.cc \
	emit_header.cc emit_source.cc emit_proto.cc
sql3gen_DEFS = -DPARSER_YY_HDR=\"$(sql3gen_parser.yy_HDR)\" \
	-DYY_TYPEDEF_YY_SIZE_T=1 -Dyy_size_t=int -DVERSION=\"$(VERSION)\"
sql3gen_LIBS = $(TEMPLATE_OBJS)
sql3gen_PREMAKE = $(template_to_c_TARGET) $(TEMPLATE_OBJS)

LIB_TARGETS += sample

sample_TARGET = $(OBJDIR)/libsample.a
sample_INCS = -Isqlite3 -I. -Itinyxml2 $(PROTOINC) $(GTEST_INCS)
sample_PROTOSRCS = sample2.proto
sample_PREMAKE = $(OBJDIR)/sample.o
sample_EXTRAOBJS = $(OBJDIR)/sample.o $(OBJDIR)/sample.pb.o

ifeq ($(BUILD_SAMPLETEST),1)

PROG_TARGETS += sampletest
sampletest_TARGET = $(OBJDIR)/sample
sampletest_CXXSRCS = sample_test.cc
sampletest_DEFS = -DSAMPLE_H_HDR=\"sample.h\"
sampletest_DEPLIBS = $(sample_TARGET)
sampletest_LIBS = $(OBJDIR)/tinyxml2.o sqlite3/sqlite3.o \
		$(PROTOLIB) $(GTEST_LIBS) -lpthread -ldl
sampletest_INCS = -Isqlite3 -I. -Itinyxml2 $(PROTOINC) $(GTEST_INCS)
sampletest_PREMAKE = $(OBJDIR)/tinyxml2.o

ifeq ($(RUN_SAMPLETEST),1)
SAMPLE_TEST_LOG = $(OBJDIR)/sample-test-log.txt
sampletest_POSTMAKE = $(SAMPLE_TEST_LOG)
endif

endif

include Makefile.inc

$(OBJDIR)/tinyxml2.o: tinyxml2/tinyxml2.cpp
	@echo compiling tinyxml2.cpp
	@g++ -c -O3 -Wall -DTINYXML2_DEBUG tinyxml2/tinyxml2.cpp \
		-o $(OBJDIR)/tinyxml2.o

$(sql3gen_TARGET): $(foreach t,$(TEMPLATES),$(OBJDIR)/template_$(t).o)

$(sql3gen_CXXOBJS): $(foreach t,$(TEMPLATES),$(OBJDIR)/template_$(t).o)

define TEMPLATE_RULES

$(OBJDIR)/template_$(1).o: $(OBJDIR)/template_$(1).cc
	@echo compiling $(OBJDIR)/template_$(1).cc
	$(Q)g++ -O3 -c $(OBJDIR)/template_$(1).cc -o $(OBJDIR)/template_$(1).o -I$(OBJDIR) -I.

$(OBJDIR)/template_$(1).cc $(OBJDIR)/template_$(1).h: $(template_to_c_TARGET) template_$(1)
	@echo generating $(OBJDIR)/template_$(1).cc
	$(Q)./$(OBJDIR)/template_to_c template_$(1) $(OBJDIR)/template_$(1).cc $(OBJDIR)/template_$(1).h || ( rm -f $(OBJDIR)/template_$(1).cc $(OBJDIR)/template_$(1).h ; exit 1 )

endef

$(eval $(foreach t,$(TEMPLATES),$(call TEMPLATE_RULES,$(t))))

SAMPLE_PB_O = $(OBJDIR)/sample.pb.o

$(sample_TARGET): $(OBJDIR)/sample.o $(SAMPLE_PB_O)

$(sample_CXXOBJS): $(OBJDIR)/sample.o $(SAMPLE_PB_O)

$(OBJDIR)/sample.pb.o: $(OBJDIR)/sample.pb.cc
	@echo compiling $(OBJDIR)/sample.pb.cc
	$(Q)g++ $(sample_INCS) $(CXXFLAGS) -O3 -c $(OBJDIR)/sample.pb.cc -o $(OBJDIR)/sample.pb.o

# making this depend on sample.cc makes it depend on sample.schema
# and running sql3gen, which is what we want
$(OBJDIR)/sample.pb.cc: $(sql3gen_TARGET) $(OBJDIR)/sample.cc
	@echo generating $(OBJDIR)/sample.pb.cc
	cd $(OBJDIR) ; $(PROTOC_PATH) --cpp_out=. --proto_path=..:. sample.proto
	cd $(OBJDIR) ; rm -f sample2.pb.h ; ln -s sample-sample2.pb.h sample2.pb.h

$(OBJDIR)/sample.o: $(OBJDIR)/sample.cc $(OBJDIR)/sample.pb.cc
	@echo compiling $(OBJDIR)/sample.cc
	$(Q)g++ $(sample_INCS) $(CXXFLAGS) -O3 -c $(OBJDIR)/sample.cc -o $(OBJDIR)/sample.o

$(OBJDIR)/sample.cc: $(sql3gen_TARGET) sample.schema
	@echo generating $(OBJDIR)/sample.cc
	$(Q)cd $(OBJDIR) ; ./sql3gen ../sample.schema \
		sample.cc.tmp sample.h sample.proto
	$(Q)mv $(OBJDIR)/sample.cc.tmp $(OBJDIR)/sample.cc
	grep -v '^#line' $(OBJDIR)/sample.cc  >  obj_sample.cc
	grep -v '^#line' $(OBJDIR)/sample.h   >  obj_sample.h
	cp $(OBJDIR)/sample.proto obj_sample.proto

$(SAMPLE_TEST_LOG): $(sampletest_TARGET)
	rm -f $(SAMPLE_TEST_LOG)
	script $(SAMPLE_TEST_LOG).new -c $(sampletest_TARGET)
	mv $(SAMPLE_TEST_LOG).new $(SAMPLE_TEST_LOG)

tokenize:
	cd $(OBJDIR) ; DEBUG_TOKENIZER=1 ./sql3gen ../sample.schema \
		sample.cc.tmp sample.h sample.proto

bundle:
	git bundle create sqlite3gen.bundle --all
	git bundle verify sqlite3gen.bundle

