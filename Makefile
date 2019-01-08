
PROG_TARGETS = template_to_c sql3gen sample

OBJDIR = obj

CXXFLAGS = -Wall -Werror

template_to_c_TARGET = $(OBJDIR)/template_to_c
template_to_c_CXXSRCS = template_to_c.cc

sql3gen_TARGET = $(OBJDIR)/sql3gen
sql3gen_LLSRCS = tokenizer.ll
sql3gen_YYSRCS = parser.yy
sql3gen_CXXSRCS = main.cc emit_header.cc emit_source.cc template_patterns.cc
sql3gen_DEFS = -DPARSER_YY_HDR=\"$(sql3gen_parser.yy_HDR)\" \
	-DYY_NO_UNPUT=1 -DYY_TYPEDEF_YY_SIZE_T=1 -Dyy_size_t=int
sql3gen_LIBS = $(OBJDIR)/template_1.o
sql3gen_PREMAKE = $(template_to_c_TARGET) $(OBJDIR)/template_1.o

sample_TARGET = $(OBJDIR)/sample
sample_CXXSRCS = sample_test.cc
sample_DEFS = -DSAMPLE_H_HDR=\"sample.h\"
sample_LIBS = sqlite3/sqlite3.o $(OBJDIR)/sample.o -lpthread -ldl
sample_INCS = -Isqlite3
sample_PREMAKE = $(OBJDIR)/sample.o

include Makefile.inc

$(sql3gen_TARGET): $(OBJDIR)/template_1.o

$(OBJDIR)/template_1.o: $(OBJDIR)/template_1.cc
	@echo compiling $(OBJDIR)/template_1.cc
	$(Q)g++ -O3 -c $(OBJDIR)/template_1.cc -o $(OBJDIR)/template_1.o -I$(OBJDIR) -I.

$(OBJDIR)/template_1.cc: $(template_to_c_TARGET) template_1
	@echo generating $(OBJDIR)/template_1.cc
	$(Q)./obj/template_to_c template_1 $(OBJDIR)/template_1.cc.tmp $(OBJDIR)/template_1.h.tmp
	$(Q)mv $(OBJDIR)/template_1.cc.tmp $(OBJDIR)/template_1.cc
	$(Q)mv $(OBJDIR)/template_1.h.tmp $(OBJDIR)/template_1.h

$(sample_TARGET): $(OBJDIR)/sample.o

$(OBJDIR)/sample.o: $(OBJDIR)/sample.cc
	@echo compiling $(OBJDIR)/sample.cc
	$(Q)g++ $(sample_INCS) -O3 -c $(OBJDIR)/sample.cc -o $(OBJDIR)/sample.o

$(OBJDIR)/sample.cc: $(sql3gen_TARGET) sample.schema
	@echo generating $(OBJDIR)/sample.cc
	$(Q)cd $(OBJDIR) ; ./sql3gen ../sample.schema sample.cc.tmp sample.h
	$(Q)mv $(OBJDIR)/sample.cc.tmp $(OBJDIR)/sample.cc
