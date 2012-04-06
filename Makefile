all: prepare sexp test

prepare:
	mkdir -p build

sexp: build/sexp.o build/r5rs_lexer.o
	clang++\
		build/sexp.o build/r5rs_lexer.o\
		-o build/sexp\
		-largtable2

build/sexp.o: src/sexp.cpp build/r5rs_lexer.cpp
	clang++\
		-c src/sexp.cpp\
		-o build/sexp.o\
		-Iinclude\
		-Ibuild\
		-I.\
		-I$(QUEX_PATH)\
		-DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED\
		-g

build/r5rs_lexer.o: build/r5rs_lexer.cpp
	clang++\
		-c build/r5rs_lexer.cpp\
		-o build/r5rs_lexer.o\
		-I.\
		-Iinclude\
		-I$(QUEX_PATH)\
		-DQUEX_OPTION_ASSERTS_WARNING_MESSAGE_DISABLED\
		-g

build/r5rs_lexer.cpp: src/r5rs.qx
	quex\
		-i src/r5rs.qx\
		-o r5rs_lexer\
		--language C++\
		--token-id-prefix TKN_\
		--output-directory build\
		--token-policy single

test: build/test
	./build/test

build/test: build/main.o
	clang++\
		build/main.o\
		-o build/test\
		-lgtest\
		-pthread

build/main.o: test/main.cpp
	clang++\
		-c test/main.cpp\
		-o build/main.o\
		-g

clean:
	rm -rf build/

# vim:ft=make ts=8 sw=8 noet
