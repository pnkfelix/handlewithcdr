Q="
Q="
extract_deps = $(subst $Q,,$(subst \#include,,$(shell grep '^.include "' $(1))))
TEST_DEPS:=$(call extract_deps,test.cpp)
CORE_DEPS:=$(call extract_deps,core.cpp)

default: test
	./test

core.o: core.cpp $(CORE_DEPS) Makefile
	true $(CORE_DEPS)
	clang++ -g -c $< -o $@

test.o: test.cpp $(TEST_DEPS) Makefile
	true $(TEST_DEPS)
	clang++ -g -c $< -o $@

test: core.o test.o
	clang++ -g -o $@ $^
