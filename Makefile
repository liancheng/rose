all: sexp test

sexp: build/sexp.o lexer/r5rs_lexer.o
	clang++\
		build/sexp.o lexer/r5rs_lexer.o\
		-o build/sexp\
		-lreadline\
		-largtable2

build/sexp.o: sexp.cpp lexer/r5rs_lexer.cpp
	clang++\
		-c sexp.cpp\
		-o build/sexp.o\
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

test: build/test
	./build/test

build/test: build/test.o
	clang++\
		build/test.o\
		-o build/test\
		-lgtest\
		-pthread

build/test.o: test.cpp
	clang++\
		-c test.cpp\
		-o build/test.o\
		-g

clean:
	rm -rf build/*
	rm -rf lexer/*

# vim:ft=make ts=8 sw=8 noet
