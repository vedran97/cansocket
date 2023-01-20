## SocketCAN API C++ wrapper

1. Goal of this project was to write a C++ wrapper over linux SocketCAN API. CAN standard used was FD-CAN.
2. Personal learning goals were to implement and understand C++ rule of 5 , and inplace construction in a vector.
3. ROS2's ament-cmake build system was used to build this project, but the source files can be taken and modified to run in any build system of your choice

## TODO: 

1. Further steps to be taken : Write a free function for in-place construction
2. Write GTests to demonstrate working of th library with virtual-can network
3. Elaborate on usage of cansetup.sh script
4. Add basic read and write functions.