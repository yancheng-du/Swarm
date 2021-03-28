http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/

-followed 'building on linux instructions'
-these did not work for me, I was able to make and run the examples, but the
cmake files were too complicated for me to replicate the builds

-used the instructions on http://www.codebind.com/linux-tutorials/install-opengl-ubuntu-linux/

$ sudo apt-get update
$ sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev

-compiled with
g++ main.cpp -o firstOpenGlApp -lglut -lGLU -lGL

-using GLUT will not be enough for this project, as it relies on a single event
loop that can't be exited from

-to install glew
https://sourceforge.net/projects/glew/
installed using this link, unzipped, then ran
$make install

-install glfw3
https://stackoverflow.com/questions/17768008/how-to-build-install-glfw-3-and-use-it-in-a-linux-project
installed using the guide in this Link, specific steps =>
-downloaded latest version from - http://www.glfw.org/download.html
-ran $cmake . in project directory after unzipping
-ran $make install

-install glm
sudo apt-get install libglm-dev

g++ -o tutorial0 tutorial02.cpp ../common/shader.cpp -lglfw3 -lGLEW -lGL -lm -lXrandr -lXi -lX11 -ldl -lXxf86vm -lpthread


3. installing libpng from http://www.libpng.org/pub/png/libpng.html
  - downloaded libpng from https://sourceforge.net/projects/libpng/files/libpng16/1.6.37/libpng-1.6.37.tar.xz/download?use_mirror=svwh
  - $configure
  - $make check
  - $sudo make install

4. used stb_image instead
https://learnopengl.com/Getting-started/Textures

------------------------
resize an image to be square while keeping aspect ratio - add transparency to sides
$convert -background none -resize 64x64 -gravity center -extent 64x64 bee_2.png bee_2_correct.png
