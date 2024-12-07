all: com test_build

com: main.cpp functions.cpp
	g++ -o main main.cpp functions.cpp -lpthread

test_build: testcases.cpp functions.cpp
	g++ -o test_cases testcases.cpp functions.cpp -lpthread

main: com
	./main

test: test_build
	./test_cases

clean:
	rm -f main test_cases