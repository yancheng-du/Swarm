all: main

GRAPHICS_CFLAGS=-fPIC -g -Wall -std=c++11
CFLAGS=$(GRAPHICS_CFLAGS) `pkg-config --cflags opencv4` -Wno-deprecated
INCLUDE=-I/usr/local/include/libfreenect/ -I/usr/local/include/opencv4/opencv2/ -I/usr/local/include/libusb-1.0/
LIBS=-framework OpenAL -framework OpenGL -lfreenect -lGLFW -lGLEW `pkg-config --libs opencv4`

main:  main.cpp graphics/common/controls.o graphics/common/objloader.o graphics/common/shader.o graphics/common/text2D.o graphics/common/texture.o graphics/graphics_module.o #utils.o
	$(CXX) $(INCLUDE) $(CFLAGS) $? -o $@ $(LIBS) $(FREE_LIBS)

graphics/common/controls.o: graphics/common/controls.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

graphics/common/objloader.o: graphics/common/objloader.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

graphics/common/shader.o: graphics/common/shader.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

graphics/common/text2D.o: graphics/common/text2D.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

graphics/common/texture.o: graphics/common/texture.cpp graphics/common/stb_image.h
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

graphics/graphics_module.o: graphics/graphics_module.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@

#LONG LIVE TENSORFLOW 
#utils.o: utils.cpp utils.h
#	$(CXX) $(INCLUDE) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o graphics/*.o graphics/common/*.o main.dSYM/ main
