TEST_DIR = ./src/test/googletest-release-1.8.0/googletest


run: clean ./build compile
	cd ./src && ./cv_hw4_1

compile: ./build/cv_hw4_1.o  ./build/cannylit.o
	clang++ -std=c++11 ./build/cv_hw4_1.o ./build/cannylit.o -L./src/lib/X11/lib -lX11 -o ./src/cv_hw4_1 -O2

./build/cv_hw4_1.o: ./src/cv_hw4_1.cpp
	clang++ -std=c++11 -I./src/lib/X11/include -c ./src/cv_hw4_1.cpp -O2
	mv cv_hw4_1.o ./build

./build/cannylit.o:
	clang++ -std=c++11 -c  ./src/include/cannylit.cpp -I./src/lib/X11/include -o ./build/cannylit.o  -O2

./build:
	mkdir ./build

clean:
	rm -rf ./build ./src/cv_hw4_1
