JSONP_DEBUG ?=

CXX ?= g++
CXXFLAGS = -Wall -Wextra -Werror
LDFLAGS = -L.
CPPFLAGS = -Iinclude

ifeq ($(JSONP_DEBUG),1)
DEBUG_CXXFLAGS += -g -O0
else
DEBUG_CXXFLAGS += -O2
endif

PREFIX ?= /usr/local

DEPS = cppunit

CXXFLAGS += $(shell pkg-config --cflags $(DEPS))
LIBS = -Wl,--as-needed $(shell pkg-config --libs $(DEPS))

TARGET = libjsonp.so
TESTS = test_jsonp

SRC = $(wildcard src/*.cc)
OBJ = $(SRC:%.cc=%.o)
COV_OBJ = $(SRC:%.cc=%.cov.o)

TEST_SRC = $(wildcard tests/*.cc)
TEST_OBJ = $(TEST_SRC:%.cc=%.cov.o)
TEST_LIB = libjsonp_test.a

ALL_OBJ = $(OBJ) $(TEST_OBJ) $(COV_OBJ)
GCNO = $(ALL_OBJ:%.o=%.gcno)
GCDA = $(ALL_OBJ:%.o=%.gcda)

all: $(TARGET) $(TESTS)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(LDFLAGS) -shared $(LIBS)

$(TEST_LIB): $(COV_OBJ)
	ar rcs $@ $^

%.cov.o : %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -O0 -g --coverage -c $< -o $@

%.o : %.cc
	$(CXX) $(CXXFLAGS) $(DEBUG_CXXFLAGS) $(CPPFLAGS) -fPIC -c $< -o $@

$(TESTS): $(TEST_LIB) $(TEST_OBJ)
	$(CXX) -o $@ $(TEST_OBJ) $(TEST_LIB) $(LDFLAGS) --coverage $(LIBS)

run_tests: $(TESTS)
	./$(TESTS)

run_valgrind: $(TESTS)
	LD_LIBRARY_PATH=. valgrind --leak-check=full ./$(TESTS)

run_gdb: $(TESTS)
	LD_LIBRARY_PATH=. gdb ./$(TESTS)

coverage: run_tests
	lcov -c -b . -d src -d tests --output-file coverage/lcov.raw; \
	lcov -r coverage/lcov.raw "/usr/include/*" --output-file coverage/lcov.info; \
	genhtml coverage/lcov.info --output-directory coverage

install: all
	install -d $(PREFIX)/lib
	install -m 755 $(TARGET) $(PREFIX)/lib/
	install -d $(PREFIX)/include/
	install -m 644 include/*.h $(PREFIX)/include/

clean:
	rm -rf $(TARGET) $(TESTS) $(TEST_LIB) $(ALL_OBJ) $(GCNO) $(GCDA) coverage/*

.PHONY: all clean run_tests run_valgrind run_gdb coverage
