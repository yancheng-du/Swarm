#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "./graphics_module.hpp"

using namespace std;

/*
IMPORTANT - POSITIONING
the bee and qr codes interpret coordinates like 

 x + <----- -
            |
            |
            V
            +
            y

while the text interprets coordinates like

x
+
^
|
|
- ------> y

this is because the kinect seems to read in data backwards and mirrored,
the bee code compensates for this by flipping the 'camera' upside down
and the qr code code uses the same camera as the bee code

However, the text code does not use the same type of vertex mapping as the
bees and qr code. Therefore it's coordinates are 'normal'
*/
int main( void ){
  int maxX = 500;
  int maxY = 1000;
  GraphicsModule gm (6, maxX, maxY, 1, 2.0f,
                     "abee.png", "./");

  //particle initialization
  int bee_x_arr[] = {0, maxX, maxX, 0, maxX/4, maxX/2};
  int bee_y_arr[] = {0, 0, maxY, maxY, maxY/4, maxY/2};

  vector<int> bee_x (bee_x_arr, bee_x_arr+6);
  vector<int> bee_y (bee_y_arr, bee_y_arr+6);
  vector<int> zeros (6);

  //give particles to graphics module
  gm.update_particles(bee_x, bee_y, zeros, zeros);

  //add text to the graphics module
  gm.add_text("Hello World", 0, 500, 1.0f);
  gm.add_text("Hello2", 100, 750, 0.5f);

  do{
    gm.update_qr(true, "./qr.png", 500, 1000, 4.0f); //update the status of the qr -> done every iteration
    gm.update_display(); //writes added particles and texts to the frame
  }while(!gm.should_close());

  //deallocate resources
  gm.cleanup();
  return 0;
}
