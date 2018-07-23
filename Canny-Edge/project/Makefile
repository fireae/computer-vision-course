TEST_DIR = ./src/test/googletest-release-1.8.0/googletest

run: clean ./build compile
	cd ./src && ./cv_hw2

compile: ./build/cv_hw2.o ./build/canny.o
	g++ ./build/cv_hw2.o ./build/canny.o -L./src/lib/X11/lib -lX11 -lpthread -o ./src/cv_hw2

./build/cv_hw2.o: ./src/cv_hw2.cpp
	g++ -I./src/lib/X11/include -c ./src/cv_hw2.cpp
	mv cv_hw2.o ./build

test: clean ./build ./build/test.o ./build/canny.o
	g++ ./build/test.o ./build/canny.o -L./src/lib/X11/lib -lX11 -lpthread -o ./src/test_exe
	cd ./src && ./test_exe

./build/test.o: ./src/test/test.cpp
	g++ -I./src/lib/X11/include -c ./src/test/test.cpp
	mv test.o ./build

./build/canny.o:
	g++ -c  ./src/include/canny.cpp  -I./src/lib/X11/include -o ./build/canny.o

./build:
	mkdir ./build

clean:
	rm -rf ./build ./src/cv_hw2 