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
COMPILER = g++ -ggdb 

CC = $(COMPILER) -c \
	 -I./ -I$(QUEX_PATH)                             \
	 -Wall -Wconversion -pedantic                    \
	 $(CCFLAG_ASSERTS)                               \
	 -DQUEX_SETTING_TOKEN_QUEUE_SIZE=10              \
	 -DQUEX_SETTING_TOKEN_QUEUE_SAFETY_BORDER=4      
	 # -DQUEX_OPTION_ASSERTS_DISABLED
	 # -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED \
     # -DQUEX_OPTION_DEBUG_SHOW
   
LD = $(COMPILER) 

# (*) RULES ____________________________________________________________________
# -- application
lexer:          exampleLexer.o EasyLexer.o
	$(LD) -o $@ exampleLexer.o EasyLexer.o 
         
# A lexer where 'spaces' are considered bad
lexer-bs:       exampleLexer_bad_space.o EasyLexer_bad_space.o
	$(LD) -o $@ exampleLexer_bad_space.o EasyLexer_bad_space.o 
          
# A lexer where 'tabs' are considered bad
lexer-bt:       exampleLexer_bad_tab.o EasyLexer_bad_tab.o
	$(LD) -o $@ exampleLexer_bad_tab.o EasyLexer_bad_tab.o 
          
# Object files:
Easy%.o: Easy%.cpp 
	$(CC) $< -o $@ 

example%.o: ../example.cpp  Easy%.cpp
	$(CC) $<  -o $@ 

# Macro expansions (only for debugging):
%.E: %.cpp    
	$(CC) -E $< -o $@ 

EasyLexer EasyLexer.cpp: simple.qx $(QUEX_CORE)
	quex -i simple.qx --engine EasyLexer 

EasyLexer_bad_space EasyLexer_bad_space.cpp: simple.qx $(QUEX_CORE)
	quex -i simple_bad_space.qx --engine EasyLexer_bad_space  

EasyLexer_bad_tab EasyLexer_bad_tab.cpp: simple.qx $(QUEX_CORE)
	quex -i simple_bad_tab.qx --engine EasyLexer_bad_tab  

# (*) HELPERS __________________________________________________________________
clean:	
	rm -f EasyLexer*
	rm -f EasyLexer_bad_space*
	rm -f EasyLexer_bad_tab*
	rm -f *.o
	rm -f *.bak
	rm -f *.E
	rm -f lexer
