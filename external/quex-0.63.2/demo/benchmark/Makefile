# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean 


include info.mk

# (*) SETUP ____________________________________________________________________
SETUP= \
	  -DCC_OPTIMIZATION_FLAGS="$(COMPILER_OPT)" \
      -DQUEX_SETTING_BUFFER_SIZE="$(BUFFER_SIZE)" \
      -DEMAIL='"$(AUTHOR_EMAIL)"' \
      -DCPU_NAME=$(CPU_NAME) \
      -DCPU_CODE="$(CPU_CODE)" \
	  -DCPU_FREQ_MHZ=$(CPU_MHZ) \
      -DCHARACTER_SIZE="((unsigned)(1))" \
	  -DOS_NAME='"$(OS_NAME)"' \
	  -DCC_NAME="$(COMPILER)" \
	  -D'CC_VERSION=$(COMPILER_V)' \
	  -DNOTE="$(NOTE)"

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif
include $(QUEX_PATH)/quex/code_base/core.mkd

SOURCES = $(addprefix in/, lexer.cpp report.cpp main.cpp)

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#     e.g. "make COMPILER=icpc" will use intel's c++ compiler)
# PROFILE_ENABLE = -fprofile-arcs -fprofile-values
# PROFILE_USE    = -fbranch-probabilities
INCLUDE = -I. -I$(QUEX_PATH)  
WARNING = -Wno-deprecated -Wall -Wimplicit 
DEFINES = \
		 -DNDEBUG \
		 -D__QUEX_CORE_OPTION_SUPPORT_BEGIN_OF_LINE_PRE_CONDITION_DISABLED \
		 -DQUEX_OPTION_ASSERTS_DISABLED	\
		 -DQUEX_OPTION_TOKEN_STAMPING_WITH_LINE_AND_COLUMN_DISABLED \
		 -DQUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE_DISABLED \
		 -DQUEX_BENCHMARK_SERIOUS \
		 -DQUEX_OPTION_COMPUTED_GOTOS
		 # -DQUEX_OPTION_DEBUG_SHOW \
		 # -DQUEX_SETTING_BUFFER_SIZE=64 \
		 # -D__QUEX_OPTION_UNIT_TEST \
		 # -D__QUEX_OPTION_UNIT_TEST_QUEX_BUFFER_LOADS \

CC_CORE = $(COMPILER) $(COMPILER_OPT) \
		  $(WARNING) $(INCLUDE) $(DEFINES) \
		  $(SETUP) 

CC = $(CC_CORE) \
	 -DQUEX_OPTION_LINE_NUMBER_COUNTING_DISABLED \
	 -DQUEX_OPTION_COLUMN_NUMBER_COUNTING_DISABLED 

CC_LC = $(CC_CORE) \
        -DQUEX_OPTION_LINE_NUMBER_COUNTING   \
        -DQUEX_OPTION_COLUMN_NUMBER_COUNTING 

LD = $(COMPILER) $(COMPILER_OPT) $(PROFILER)

# (*) RULES ____________________________________________________________________
#    lexer    --> application w/o line or column counting
#    lexer-lc --> application w/ line and column counting
EXECUTABLES = lexer-quex lexer-quex-lc lexer-flex lexer-re2c

all: $(addprefix run/, $(EXECUTABLES))

run/lexer-flex: out/flex_scan.c   $(SOURCES) 
	$(CC)    -DANALYZER_GENERATOR_FLEX $< $(SOURCES) in/flex/adaption.c -lfl -o $@ 

run/lexer-re2c: out/re2c_scan.c   $(SOURCES) 
	$(CC)    -DANALYZER_GENERATOR_RE2C $< $(SOURCES) in/re2c/adaption.c -o $@

run/lexer-hey-quex: out/quex_hey_scan.cpp $(SOURCES) 
	$(CC)    -DANALYZER_GENERATOR_QUEX $< $(SOURCES) in/quex/adaption.c -o $@ 

run/lexer-hey-quex.E: out/quex_hey_scan.cpp $(SOURCES) 
	$(CC)    -DANALYZER_GENERATOR_QUEX $< $(SOURCES) in/quex/adaption.c -o $@ -E

run/lexer-quex: out/quex_scan.cpp $(SOURCES) 
	$(CC)    -DANALYZER_GENERATOR_QUEX $< $(SOURCES) in/quex/adaption.c -o $@ 

run/lexer-quex.s:
	$(CC)     -DANALYZER_GENERATOR_QUEX $< out/quex_scan.cpp -c -g -Wa,-a,-ad  > $@

out/quex_scan.E:
	$(CC)     -DANALYZER_GENERATOR_QUEX out/quex_scan.cpp in/quex/adaption.c -E -o $@

run/lexer-quex-lc: out/quex_scan.cpp $(SOURCES) 
	$(CC_LC) -DANALYZER_GENERATOR_QUEX $< $(SOURCES) in/quex/adaption.c -o $@ 

# Core ________________________________________________________________________

# Scanners ____________________________________________________________________

out/quex_scan.cpp: in/quex/c.qx $(QUEX_CORE)
	quex -i $< \
         --foreign-token-id-file  in/token-ids.h \
	     --output-directory       out            \
		 --engine                 quex_scan      \
		 --token-id-offset        3              \
		 --token-id-prefix           TKN_           \
	     --token-policy           single         \
		 --no-string-accumulator                 \
	     --no-mode-transition-check              \
		 --single-mode-analyzer

out/quex_hey_scan.cpp: in/quex/hey.qx $(QUEX_CORE)
	quex -i $< \
	     --output-directory       out            \
		 --engine                 quex_scan      \
		 --no-string-accumulator                 \
		 --token-id-offset        3              \
		 --token-id-prefix           TKN_           \
	     --token-policy           single         
	mv out/quex_scan.cpp out/quex_hey_scan.cpp
	# ./move_it.sh hey

out/flex_scan.c:  in/flex/c.lex
	flex -o $@ $< 

out/re2c_scan.c:  in/re2c/c-re2c.in
	re2c -F -si $< | sed -e 's/\#line[^\n]*//g' > $@
	
# (*) HELPERS __________________________________________________________________
clean:	
	rm -f out/*
	rm -f $(addprefix run/, $(EXECUTABLES))

cpu_mhz:
	@echo $(CPU_MHZ)

os_name:
	@echo $(OS_NAME)
