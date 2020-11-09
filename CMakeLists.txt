cmake_minimum_required(VERSION 3.17)
project(imageview)

set(CMAKE_CXX_STANDARD 11)


find_package(GTK2 REQUIRED gtk)
find_package(OpenCV REQUIRED core imgcodecs imgproc)


add_library(imageview imageview.cpp)
target_include_directories(imageview PUBLIC "./include" ${GTK2_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(imageview ${GTK2_LIBRARIES} ${OpenCV_LIBRARIES})

#add_executable(demo demo.cpp)
#target_include_directories(demo PUBLIC ${OpenCV_INCLUDE_DIRS})
#target_link_libraries(demo imageview ${OpenCV_LIBRARIES})
