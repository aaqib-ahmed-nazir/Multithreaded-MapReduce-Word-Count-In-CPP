all: com test_build

com: src/main.cpp src/functions.cpp
	g++ -o main src/main.cpp src/functions.cpp -lpthread

test_build: src/testcases.cpp src/functions.cpp
	g++ -o test_cases src/testcases.cpp src/functions.cpp -lpthread -I src/

main: com
	./main

test: test_build
	./test_cases

clean:
	rm -f main test_cases