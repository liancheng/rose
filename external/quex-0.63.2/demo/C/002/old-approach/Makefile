# PURPOSE: Makefile for unit tests.
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
include $(QUEX_PATH)/quex/code_base/core.mkd

# (*) SETUP ____________________________________________________________________
#
# QUEX_PATH    = ../../ # there should be an environment variable 'QUEX_PATH'
#                       # thus this does have to be defined explicitly here.
#
# -- INPUT
MODE_FILES   = simple.qx
DEBUG_F      = # --debug # set flag if debugging info is required
#
# -- APPLICATION / LIBRARY specification
APPLICATION_NAME = lexer
LIBRARY_NAME     = libLexer
ENGINE_NAME      = indy_lex# NOTE: a whitespace after this name creates chaos!
#
ENGINE_SOURCES   = $(ENGINE_NAME)                     \
                   $(ENGINE_NAME).c                   \
                   $(ENGINE_NAME)-token_ids           \
                   $(ENGINE_NAME)-configuration       
#
LIBRARY_OBJS     = $(ENGINE_NAME).o


ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#     e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = g++
# -- compiler
CC = $(COMPILER) -c -fPIC -ggdb -Wno-deprecated -Wall \
	 -I./ -I$(QUEX_PATH) $(CCFLAG_ASSERTS)      \
	 -DINDENTATION_STACK_SIZE=64
	 # -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED \
     # -DQUEX_OPTION_ASSERTS_DISABLED
	 # -DQUEX_OPTION_DEBUG_SHOW

# -- linker
LD         = $(COMPILER) 
STATIC_LD  = ar -rv
DYNAMIC_LD = $(COMPILER) -shared 


# (*) RULES ____________________________________________________________________
#
# -- application
$(APPLICATION_NAME): ./$(APPLICATION_NAME).o $(ENGINE_NAME).o 
	$(LD) ./$(APPLICATION_NAME).o $(ENGINE_NAME).o   \
          -o $(APPLICATION_NAME) 

# -- engine and object files
$(APPLICATION_NAME).o: $(APPLICATION_NAME).c $(ENGINE_SOURCES)
#       # require: $(ENGINE_SOURCES) for header files
	$(CC) $(APPLICATION_NAME).c -o $(APPLICATION_NAME).o

$(ENGINE_NAME).o: $(ENGINE_SOURCES)
	$(CC) $(ENGINE_NAME).c -o $(ENGINE_NAME).o

%.o: %.c
	$(CC) $< -o $@

%.E: %.c
	$(CC:-c=-E) $< -o $@

$(ENGINE_SOURCES): $(MODE_FILES) $(QUEX_CORE)
	quex --mode-files $(MODE_FILES)    \
	     --engine     $(ENGINE_NAME)   \
		 --language C


# (*) HELPERS __________________________________________________________________
#
clean:	
	touch $(MODE_FILES)
	rm -f $(ENGINE_NAME) $(ENGINE_NAME).c \
		  $(ENGINE_NAME)-token \
		  $(ENGINE_NAME)-token.i \
		  $(ENGINE_NAME)-token_ids \
		  $(ENGINE_NAME)-configuration
	rm -f $(ENGINE_NAME).o
	rm -f lexer.[oE]
	rm -f lexer
	rm -f gmon.out
	rm -f token_ids
	rm -f *.bak
