
PROG_TARGETS = template_to_c sql3gen sample xmltest1 xmltest2

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

CXXFLAGS = -Wall -Werror $(PROTOINC)

template_to_c_TARGET = $(OBJDIR)/template_to_c
template_to_c_CXXSRCS = template_to_c.cc

TEMPLATES = header source proto
TEMPLATE_OBJS = $(foreach t,$(TEMPLATES),$(OBJDIR)/template_$(t).o)

sql3gen_TARGET = $(OBJDIR)/sql3gen
sql3gen_LLSRCS = tokenizer.ll
sql3gen_YYSRCS = parser.yy
sql3gen_CXXSRCS = main.cc template_patterns.cc \
	emit_header.cc emit_source.cc emit_proto.cc
sql3gen_DEFS = -DPARSER_YY_HDR=\"$(sql3gen_parser.yy_HDR)\" \
	-DYY_TYPEDEF_YY_SIZE_T=1 -Dyy_size_t=int
sql3gen_LIBS = $(TEMPLATE_OBJS)
sql3gen_PREMAKE = $(template_to_c_TARGET) $(TEMPLATE_OBJS)

sample_TARGET = $(OBJDIR)/sample
sample_CXXSRCS = sample_test.cc myXml.cc
sample_PROTOSRCS = sample2.proto
sample_DEFS = -DSAMPLE_H_HDR=\"sample.h\" -DSAMPLE_PB_HDR=\"sample.pb.h\"
sample_LIBS = $(OBJDIR)/sample.pb.o sqlite3/sqlite3.o $(OBJDIR)/sample.o \
	-lpthread $(PROTOLIB) -ldl
sample_INCS = -Isqlite3 $(PROTOINC) -I.
sample_PREMAKE = $(OBJDIR)/sample.o

xmltest1_TARGET = $(OBJDIR)/xmltest1
xmltest1_CXXSRCS = myXml.cc
xmltest1_DEFS = -DINCLUDE_MYXML_TEST=1

xmltest2_TARGET = $(OBJDIR)/xmltest2
xmltest2_CXXSRCS = myXml.cc
xmltest2_DEFS = -DINCLUDE_MYXML_TEST=2

include Makefile.inc

echo:
	@echo $(TEMPLATE_OBJS)

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

$(sample_TARGET): $(OBJDIR)/sample.o $(OBJDIR)/sample.pb.o

$(sample_CXXOBJS): $(OBJDIR)/sample.o $(OBJDIR)/sample.pb.o

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

tokenize:
	cd $(OBJDIR) ; DEBUG_TOKENIZER=1 ./sql3gen ../sample.schema \
		sample.cc.tmp sample.h sample.proto

bundle:
	git bundle create sqlite3gen.bundle --all
	git bundle verify sqlite3gen.bundle

