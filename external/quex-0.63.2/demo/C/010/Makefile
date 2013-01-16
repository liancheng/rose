# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean UTF8 NON_UTF8

FILES = copy.exe    fill.exe    point.exe \
		copy-ez.exe fill-ez.exe re-point.exe

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif

include $(QUEX_PATH)/quex/code_base/core.mkd

# (*) SETUP ____________________________________________________________________
OBJS      = tiny_lexer.o      messaging-framework.o
UTF8_OBJS = tiny_lexer_utf8.o messaging-framework_utf8.o

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#      e.g. "make COMPILER=icpc" will use intel's c++ compiler)
ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

COMPILER = gcc -ggdb -Wall -pedantic -std=c89

CC = $(COMPILER) -I./ -I$(QUEX_PATH) $(CCFLAG_ASSERTS) 
     # -DQUEX_OPTION_DEBUG_SHOW    
     # -DQUEX_OPTION_ASSERTS_DISABLED
	 # -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED

CC_UTF8 = $(CC) -D__QUEX_OPTION_MESSAGE_UTF8 \
		        -DQUEX_TYPE_CHARACTER=wchar_t \
		        -DQUEX_OPTION_CONVERTER_ICONV 

# (*) RULES ____________________________________________________________________
all: $(FILES)

# -- char application
          
copy-conversion.exe:        copy-conversion.c $(UTF8_OBJS) $(QUEX_CORE) 
	$(CC_UTF8) -o $@ $< $(UTF8_OBJS) 
          
copy-conversion-direct.exe: copy-conversion-direct.c $(UTF8_OBJS) $(QUEX_CORE) 
	$(CC_UTF8) -o $@ $< $(UTF8_OBJS) 

fill-conversion-direct.exe: fill-conversion-direct.c $(UTF8_OBJS) $(QUEX_CORE) 
	$(CC_UTF8) -o $@ $< $(UTF8_OBJS) 

%.exe:                      %.c $(OBJS) $(QUEX_CORE) 
	$(CC)      -o $@ $< $(OBJS) 

point.exe:                  point.c $(OBJS:.o=.c) $(QUEX_CORE) NON_UTF8
	## All related files must be compiled with fallback_n = 0 
	$(CC)      -o $@ -DQUEX_SETTING_BUFFER_MIN_FALLBACK_N=0 $< $(OBJS:.o=.c) 
          
re-point.exe:               re-point.c $(QUEX_CORE) $(OBJS:.o=.c) NON_UTF8
	## All related files must be compiled with fallback_n = 0
	$(CC)      -o $@ -DQUEX_SETTING_BUFFER_MIN_FALLBACK_N=0 $< $(OBJS:.o=.c) 
          
%_utf8.o:                   %_utf8.c $(QUEX_CORE) UTF8
	$(CC_UTF8) -c $< -o $@ 

messaging-framework_utf8.o: messaging-framework.c $(QUEX_CORE) UTF8
	$(CC_UTF8) -c $< -o $@ 
          
%.o:                     %.c $(QUEX_CORE) NON_UTF8
	$(CC)      -c $< -o $@  
          
%.E:                     %.c $(QUEX_CORE) NON_UTF8
	$(CC)      -E $< -o $@  
          
tiny_lexer tiny_lexer.c:            NON_UTF8

tiny_lexer_utf8 tiny_lexer_utf8.c:	UTF8

NON_UTF8: simple.qx $(QUEX_CORE)
	# Don't need to set '--iconv'. The flag 'QUEX_OPTION_CONVERTER_ICONV'
	# does all what is required.
	quex -i             simple.qx          \
		 --engine       tiny_lexer         \
		 --token-policy single             \
		 --language     C                  \
		 --token-memory-management-by-user 

UTF8: simple_utf8.qx $(QUEX_CORE)
	# Don't need to set '--iconv'. The flag 'QUEX_OPTION_CONVERTER_ICONV'
	# does all what is required.
	quex -i                    simple_utf8.qx   \
		 --engine              tiny_lexer_utf8  \
		 --token-policy        single           \
		 --language            C                \
		 --buffer-element-type wchar_t          \
		 --token-memory-management-by-user     

# (*) HELPERS __________________________________________________________________
clean:	
	touch simple.qx
	rm -f tiny_lexer*
	rm -f $(SOURCES)
	rm -f tiny_lexer-configuration
	rm -f $(FILES)
	rm -f *.exe
	rm -f *.o
	rm -f *.E
	rm -f copy fill point
	rm -f *.bak
	rm -f tmp*
