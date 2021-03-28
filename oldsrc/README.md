# Docker instructions
## build image
```bash
$docker build -t augmented_art .
```

## allow docker container to display X applications
```bash
$xhost local:root
```

## start container
* --privileged allows container to use all host devices
* /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY allows the container to use the host screen
* -it allows us to run in interactive mode
* /bin/bash calls /.bashrc once the container is initialized
```bash
$sudo docker run --privileged -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -it augmented_art /bin/bash
```

to add a library in the makefile simply add -I<LIBRARY_PATH> after "INCLUDES = "

# Docker Tips
## Exiting container - container will be left running
just press ctrl+p, ctrl+q

## show images
```bash
$docker images
```

## show all containers
```bash
docker ps -a
```

# Explanation
when you run "$docker build . ", Docker creates an image based off of the
Dockerfile in the current working directory. Our Dockerfile does the following.
1. Create a basic image based on Ubuntu 18.04
2. Downloads dependencies for opencv and libfreenect
3. Clones and builds opencv
4. Downloads libfreenect
5. Copies over the files in the current working directory, such as main.cpp and
the makefile, into the container
6. Moves an updated libfreenect.hpp header file into /usr/include/
7. runs "$make" - creating the final executable
8. Specifies the entrypoint of our container - the final command that is run
when our container is run in detached mode. If the container is run in
interactive mode, the entrypoint isn't used.

The container may take a long time to build the first go-round, but Docker
uses a cache so we only build what is necessary on each subsequent build.
However, everything after the ADD command is always rebuilt, as the ADD command
invalidates Docker's cache. Also, if you update or add a new command, all
commands after the new commands will be run in the next build.

The command to start the container ($sudo docker start ... ) is a little
complicated and is reviewed above. The idea here is that we have to map all
the devices on our machine to the virtual devices in the container. This
includes our screen, and our USB ports.

IF YOU ARE EXPERIENCING ERRORS AND ARE NOT RUNNING ON LINUX IT PROBABLY HAS TO
DO WITH DEVICE MAPPING - YOU MAY NEED TO CHANGE THE '$xhost local:root' commands
and the '$sudo docker run ... ' commands.

# Scripts
## stop_remove_images_containers.sh
stops and removes all Docker images and containers. Probably don't want to use
this if you have a long rebuild time.

## build_run.sh
builds the Docker image using the Dockerfile, and runs it.
