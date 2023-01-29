## RAII Move-Only SocketCAN API C++ wrapper

1. Goal of this project was to write a C++ wrapper over linux SocketCAN API. CAN standard used was FD-CAN.
2. Personal learning goals were to implement and understand C++ rule of 5 , and inplace construction in a vector.
3. ROS2's ament-cmake build system was used to build this project, but the source files can be taken and modified to run in any build system of your choice
4. The skeleton of this wrapper can be used around any other communication method which supports a socket.
5. In-place construction was done in a small onlinegdb project here : https://onlinegdb.com/3BMp4CyuEF

## How to setup Virtual CAN before executing tests : 

1. Open this directory in a terminal outside of dev container
2. If running the script for the first time, Please give yourself execute permission on cansetup.sh. ``sudo chmod u+x cansetup.sh``
3. Run the script by ``sudo sh cansetup.sh``
4. Now you are free to run all unit-tests 
5. Easiest way to run this package is by cloning it in to the src directory of "https://github.com/vedran97/ros2-workspace-template.git"

## To-Do : 

1. Add a composition/inheritance model to enforce Read Only/Write Only on socket wrapper