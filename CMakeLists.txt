cmake_minimum_required(VERSION 3.17)
project(imageview)

set(CMAKE_CXX_STANDARD 11)


find_package(GTK2 REQUIRED gtk)

set(OpenCV_DIR "/home/ych/software/opencv-4.4.0-bin")
find_package(OpenCV REQUIRED core imgcodecs imgproc)

add_library(imageview imageview.cpp imageview.h)
target_include_directories(imageview PUBLIC ${GTK2_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(imageview ${GTK2_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(demo demo.cpp)
target_include_directories(demo PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(demo imageview ${OpenCV_LIBRARIES})
