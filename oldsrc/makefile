all: main

CFLAGS=-fPIC -g -Wall `pkg-config --cflags opencv` -fpermissive
LIBS = `pkg-config --libs opencv`
INCLUDE = -Wl,-rpath='$ORIGIN/lib' -Iinclude -Llib -I/usr/include/libusb-1.0/ -I/usr/local/include/opencv4/opencv2/ -I/usr/local/include/eigen3/ -I/usr/local/include/google/protobuf/ -I/usr/local/include/tf/third-party/ -I/usr/local/include/tf/tensorflow/ -I/usr/local/include/tf/bazel-genfiles/ -I/usr/local/include/tf/ -I/usr/local/lib/
FREE_LIBS = -lfreenect -lpthread -lglfw3 -lGLEW -lGL -lm -lXrandr -lXi -lX11 -ldl -lXxf86vm -lpthread -lopenal -lalut


GRAPHICS_CFLAGS = -fPIC -g -Wall
GRAPHICS_LIBS = -lglfw3 -lGLEW -lGL -lm -lXrandr -lXi -lX11 -ldl -lXxf86vm -lpthread

main:  main.cpp utils.cpp utils.h ./graphics/graphics_module.o ./graphics/common/shader.o ./graphics/common/objloader.o ./graphics/common/texture.o ./graphics/common/controls.o ./graphics/common/text2D.o
	$(CXX) $(INCLUDE) $(CFLAGS) $? -ltensorflow_cc -ltensorflow_framework -o $@ $(LIBS) $(FREE_LIBS)

./graphics/graphics_module.o: ./graphics/graphics_module.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@ $(GRAPHICS_LIBS)

./graphics/common/text2D.o: ./graphics/common/text2D.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./graphics/common/shader.o: ./graphics/common/shader.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@ $(GRAPHICS_LIBS)

./graphics/common/objloader.o: ./graphics/common/objloader.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@ $(GRAPHICS_LIBS)

./graphics/common/texture.o: ./graphics/common/texture.cpp ./graphics/common/stb_image.h
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@ $(GRAPHICS_LIBS)

./graphics/common/controls.o: ./graphics/common/controls.cpp
	$(CXX) -c $(GRAPHICS_CFLAGS) $< -o $@ $(GRAPHICS_LIBS)

clean:
	rm -rf *.o ./graphics/*.o ./graphics/common/*.o main
