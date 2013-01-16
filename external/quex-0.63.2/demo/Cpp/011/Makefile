ifdef ASSERTS_ENABLED_F
	CCFLAG_ASSERTS=# By default asserts are enabled
else 
	CCFLAG_ASSERTS=-DQUEX_OPTION_ASSERTS_DISABLED
endif

all: iso8859-7-lexer utf8-lexer 

ISO8859_7_Lex.cpp: greek.qx
	quex -i greek.qx         \
		 --codec iso8859_7   \
		 --token-id-prefix TKN_ \
		 -o ISO8859_7_Lex

iso8859-7-lexer.o: iso8859-7-lexer.cpp ISO8859_7_Lex.cpp
	g++ -c -ggdb -D__QUEX_TEST_OPTION_ISO8859_7 \
		-Wall              \
		-I$(QUEX_PATH) -I. \
		$<                 \
		$(CCFLAG_ASSERTS)  \
		-o iso8859-7-lexer.o

iso8859-7-lexer: iso8859-7-lexer.o ISO8859_7_Lex.cpp
	g++ -ggdb -D__QUEX_TEST_OPTION_ISO8859_7 \
		-Wall              \
		$(CCFLAG_ASSERTS)  \
		-I$(QUEX_PATH) -I. \
		iso8859-7-lexer.o  \
		ISO8859_7_Lex.cpp  \
		-o $@

# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

utf8-lexer: utf8-lexer.cpp 
	quex -i greek.qx         \
		 --codec utf8        \
		 --token-id-prefix TKN_ \
		 -o UTF8Lex 
	g++ -ggdb \
		-Wall                    \
		-I$(QUEX_PATH) -I.       \
		$<                       \
		$(CCFLAG_ASSERTS)        \
		UTF8Lex.cpp              \
		-o $@
		# -DQUEX_OPTION_DEBUG_SHOW \
	
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

utf16-lexer: utf16-lexer.cpp 
	quex -i             greek.qx  \
		 --codec        utf16     \
		 --token-id-prefix TKN_      \
		 -o             UTF16Lex  \
	     -b             2
	g++ -ggdb -D__QUEX_TEST_OPTION_UTF16BE             \
		-Wall                   \
		-I$(QUEX_PATH) -I.      \
		$<                      \
		$(CCFLAG_ASSERTS)       \
		UTF16Lex.cpp            \
		-o $@

utf16-lexer-other: utf16-lexer-other.cpp 
	quex -i             greek-other.qx  \
		 --codec        utf16     \
		 --token-id-prefix TKN_      \
		 -o             UTF16Lex  \
	     -b             2
	g++ -ggdb -D__QUEX_TEST_OPTION_UTF16BE             \
		-Wall                   \
		-I$(QUEX_PATH) -I.      \
		$<                      \
		$(CCFLAG_ASSERTS)       \
		UTF16Lex.cpp            \
		-o $@

# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

			
%.E: %.cpp
	g++ -E -D__QUEX_TEST_OPTION_ISO8859_7 \
		-Wall \
		-I$(QUEX_PATH) -I. \
		$< -o $@
	
# -DQUEX_OPTION_ASSERTS_DISABLED
# -DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED 

clean:
	rm -f ISO8859_7_Lex*
	rm -f ISO8859_7_Lex-converter-iso8859_7
	rm -f iso8859-7-lexer
	rm -f UTF8Lex*
	rm -f utf8-lexer
	rm -f UTF16Lex*
	rm -f utf16-lexer
	rm -f utf16-lexer-other
	rm -f *.E
	rm -f *.svg
	rm -f *.o

