
MAIN_FILE=src/main.c

SRC_FILES=\
	src/huffman.c \
	src/util.c \
	src/lzw.c

TEST_FILES=\
	test/unity/unity.c \
	test/test_main.c \
	test/test_huffman.c \
	test/test_lzw.c

all: algolabra test

build: ${MAIN_FILE} ${SRC_FILES}
	gcc -Wall -std=c99 -O3 ${MAIN_FILE} ${SRC_FILES} -o algolabra

test: ${SRC_FILES} ${TEST_FILES}
	gcc -Wall -std=c99 -O3 ${SRC_FILES} ${TEST_FILES} -o algolabra-test
	./algolabra-test

coverage: ${SRC_FILES} ${TEST_FILES}
	gcc -Wall -std=c99 -O3 -fprofile-arcs -ftest-coverage ${SRC_FILES} ${TEST_FILES} -o algolabra-coverage
	./algolabra-coverage
	lcov --directory . --capture --output-file coverage.info --gcov-tool gcov
	lcov --remove coverage.info '*/test/*' '*/usr/include/bits/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage
	rm algolabra-coverage *gcda *gcno coverage.info