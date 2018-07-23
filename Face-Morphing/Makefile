TEST_DIR = ./src/test/googletest-release-1.8.0/googletest


compile: 
	sudo clang++ -std=c++11 -I./src/lib ./src/lib/dlib/all/source.cpp -I./src/lib/X11/include -L./src/lib/X11/lib -lX11 -lpng -DDLIB_PNG_SUPPORT `pkg-config --libs --cflags opencv` ./src/cv_hw4_2.cpp -o ./src/cv_hw4_2.out -O3

clean:
	rm -rf ./build ./src/cv_hw4_2
