mkdir _swarm_build_win
cd _swarm_build_win
cmake .. -DBUILD_CPP=OFF -DBUILD_C_SYNC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_FAKENECT=OFF -DCMAKE_CONFIGURATION_TYPES="Debug;RelWithDebInfo" -DCMAKE_INSTALL_PREFIX=..\..\swarm\ext\libfreenect-0.6.2 -DLIBUSB_1_INCLUDE_DIR=..\..\swarm\ext\libusb-1.0.23\include -DLIBUSB_1_LIBRARY=..\..\swarm\ext\libusb-1.0.23\lib\win\debug\libusb-1.0.lib