all:
	scons
	./bin/rsi test/test.scm | diff - test/expected.scm

clean:
	scons -c
