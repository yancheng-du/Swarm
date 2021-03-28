all: main

CFLAGS=-fPIC -g -Wall
LIBS=-framework OpenGL -lGLEW -lGLFW  

main: ./main.cpp ./graphics_module.o ./common/shader.o ./common/objloader.o ./common/texture.o ./common/controls.o ./common/text2D.o
	$(CXX) $(CFLAGS) $? -o $@ $(LIBS)

./graphics_module.o: ./graphics_module.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

./common/text2D.o: ./common/text2D.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

./common/shader.o: ./common/shader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

./common/objloader.o: ./common/objloader.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

./common/texture.o: ./common/texture.cpp ./common/stb_image.h
	$(CXX) -c $(CFLAGS) $< -o $@

./common/controls.o: ./common/controls.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o ./common/*.o main.dSYM/ main
