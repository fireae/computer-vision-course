TEST_DIR = ./src/test/googletest-release-1.8.0/googletest


build:
	clang++ ./src/main.cpp ./src/line_detection/cannylit.cpp -o main.out `pkg-config --libs --cflags ImageMagick libjpeg CImg opencv boost` -std=c++11 -O3

test:
	clang++ ./src/test.cpp -o test.out `pkg-config --libs --cflags ImageMagick libjpeg CImg opencv` -std=c++11 -O3
	./test.out 41886 output/bbox/7-7.bmp

train:
	echo "It may take a long time..."
	clang++ ./src/train.cpp -o train.out `pkg-config --libs --cflags opencv CImg` -std=c++11 -O3
	echo "...Builded"
	./train.out