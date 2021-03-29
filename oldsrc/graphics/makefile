all: main

CFLAGS=-fPIC -g -Wall -std=c++11
LIBS=-framework OpenGL -lGLEW -lGLFW

main: main.cpp common/controls.o common/objloader.o common/shader.o common/text2D.o common/texture.o graphics_module.o
	$(CXX) $(CFLAGS) $? -o $@ $(LIBS)

common/controls.o: common/controls.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

common/objloader.o: common/objloader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

common/shader.o: common/shader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

common/text2D.o: common/text2D.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

common/texture.o: common/texture.cpp common/stb_image.h
	$(CXX) -c $(CFLAGS) $< -o $@

graphics_module.o: graphics_module.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o common/*.o main.dSYM/ main
