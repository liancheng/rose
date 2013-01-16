# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif

include $(QUEX_PATH)/quex/code_base/core.mkd

ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#      e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = gcc -ggdb 

CC = $(COMPILER) -c \
	 -I./ -I$(QUEX_PATH)              \
	 -Wall -Wconversion -pedantic     \
	 $(CCFLAG_ASSERTS)                \
	 -DPRINT_TOKEN                    \
	 # -DQUEX_OPTION_ASSERTS_DISABLED
	 # -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED \
     # -DQUEX_OPTION_DEBUG_SHOW
   
LD = $(COMPILER) 

# (*) RULES ____________________________________________________________________
# -- application
lexer:          example.o EasyLexer.o
	$(LD) -o $@ example.o EasyLexer.o 
          
# Object files:
EasyLexer.o: EasyLexer.c 
	$(CC) $< -o $@ 

example.o: ../example.c  EasyLexer.c
	$(CC) $< -o $@ 

# Macro expansions (only for debugging):
%.E: %.c    
	$(CC) -E $< -o $@ 

EasyLexer.c: c.qx $(QUEX_CORE)
	quex -i c.qx --engine EasyLexer \
		 --language C 

# (*) HELPERS __________________________________________________________________
clean:	
	rm -f EasyLexer*
	rm -f *.o
	rm -f *.bak
	rm -f *.E
	rm -f lexer
	rm -f *.svg
