
MAIN_FILE=src/main.c

SRC_FILES=\
	src/huffman.c

TEST_FILES=\
	test/unity/unity.c \
	test/tests.c

all: algolabra test

build: ${MAIN_FILE} ${SRC_FILES}
	gcc -Wall -std=c99 ${MAIN_FILE} ${SRC_FILES} -o algolabra

test: ${SRC_FILES} ${TEST_FILES}
	gcc -Wall -std=c99 ${SRC_FILES} ${TEST_FILES} -o algolabra-test
	./algolabra-test
	rm algolabra-test

coverage: ${SRC_FILES} ${TEST_FILES}
	gcc -Wall -std=c99 -fprofile-arcs -ftest-coverage ${SRC_FILES} ${TEST_FILES} -o algolabra-test
	./algolabra-test
	lcov --directory . --capture --output-file coverage.info --gcov-tool gcov
	lcov --remove coverage.info '*/test/*' --output-file coverage.info
	genhtml coverage.info --output-directory coverage
	rm algolabra-test *gcda *gcno coverage.info