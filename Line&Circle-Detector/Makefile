TEST_DIR = ./src/test/googletest-release-1.8.0/googletest


line: clean ./build compile
	cd ./src && ./cv_hw3_1

circle: clean ./build compile
	cd ./src && ./cv_hw3_2

compile: ./build/cv_hw3_1.o  ./build/cv_hw3_2.o ./build/cannylit.o
	g++ ./build/cv_hw3_1.o ./build/cannylit.o -L./src/lib/X11/lib -lX11 -lpthread -o ./src/cv_hw3_1
	g++ ./build/cv_hw3_2.o ./build/cannylit.o -L./src/lib/X11/lib -lX11 -lpthread -o ./src/cv_hw3_2

./build/cv_hw3_1.o: ./src/cv_hw3_1.cpp
	g++ -I./src/lib/X11/include -c ./src/cv_hw3_1.cpp
	mv cv_hw3_1.o ./build

./build/cv_hw3_2.o: ./src/cv_hw3_2.cpp
	g++ -I./src/lib/X11/include -c ./src/cv_hw3_2.cpp
	mv cv_hw3_2.o ./build

./build/cannylit.o:
	g++ -c  ./src/include/cannylit.cpp -I./src/lib/X11/include -o ./build/cannylit.o

./build:
	mkdir ./build

clean:
	rm -rf ./build ./src/cv_hw3_1 ./src/cv_hw3_2
