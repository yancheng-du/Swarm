all: main

CFLAGS=-fPIC -g -Wall
FREE_LIBS = -lglfw3 -lGLEW -lGL -lm -lXrandr -lXi -lX11 -ldl -lXxf86vm -lpthread
# CXX=g++-5

main: ./main.cpp ./graphics_module.o ./common/shader.o ./common/objloader.o ./common/texture.o ./common/controls.o ./common/text2D.o
	$(CXX) $(INCLUDE) $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS)

./graphics_module.o: ./graphics_module.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./common/text2D.o: ./common/text2D.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./common/shader.o: ./common/shader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./common/objloader.o: ./common/objloader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./common/texture.o: ./common/texture.cpp ./common/stb_image.h
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

./common/controls.o: ./common/controls.cpp
	$(CXX) -c $(CFLAGS) $< -o $@ $(FREE_LIBS)

clean:
	rm -rf *.o main
	rm -rf ./common/*.o
