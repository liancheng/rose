all: sexp

sexp: sexp.o lexer/r5rs_lexer.o
	clang++\
		sexp.o lexer/r5rs_lexer.o\
		-o sexp\
		-L/lib\
		-lreadline

sexp.o: sexp.cpp lexer/r5rs_lexer.cpp
	clang++\
		-c sexp.cpp\
		-o sexp.o\
		-I.\
		-I$(QUEX_PATH)\
		-DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED\
		-g

lexer/r5rs_lexer.o: lexer/r5rs_lexer.cpp
	clang++\
		-c lexer/r5rs_lexer.cpp\
		-o lexer/r5rs_lexer.o\
		-I./lexer\
		-I.\
		-I$(QUEX_PATH)\
		-DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED\
		-g

lexer/r5rs_lexer.cpp: r5rs.qx
	quex\
		-i r5rs.qx\
		-o r5rs_lexer\
		--language C++\
		--token-id-prefix TKN_\
		--output-directory lexer\
		--token-policy single

clean:
	rm -f sexp.o sexp
	rm -rf lexer/*

# vim:ft=make ts=8 sw=8 noet
