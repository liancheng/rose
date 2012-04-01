all: sexp

sexp: sexp.o lexer/r5rs_lexer.o
	clang\
		sexp.o lexer/r5rs_lexer.o\
		-o sexp\
		-L/lib\
		-lreadline

sexp.o: sexp.c lexer/r5rs_lexer.c
	clang\
		-c sexp.c\
		-o sexp.o\
		-I.\
		-I$(QUEX_PATH)

lexer/r5rs_lexer.o: lexer/r5rs_lexer.c
	clang\
		-c lexer/r5rs_lexer.c\
		-o lexer/r5rs_lexer.o\
		-I./lexer\
		-I.\
		-I$(QUEX_PATH)\
		-DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED

lexer/r5rs_lexer.c: r5rs.qx
	quex\
		-i r5rs.qx\
		-o r5rs_lexer\
		--language C\
		--token-prefix TKN_\
		--output-directory lexer\
		--token-policy single\
		--token-memory-management-by-user

clean:
	rm -f sexp.o sexp
	rm -rf lexer/*

# vim:ft=make ts=8 sw=8 noet
