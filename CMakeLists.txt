project(cse408phase2)
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
set(CMAKE_BUILD_TYPE Debug)
add_definitions(-std=c++11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}")

find_package(OpenCV REQUIRED)
find_package(GMP REQUIRED)

# Project Executable
add_executable (task1 task1.cpp )
add_executable (task2 task2.cpp )
add_executable (task3 task3.cpp )

add_executable (task4_ACD task4_ACD.cpp )
add_executable (task4LZW task4LZW.cpp )
add_executable (task4-shannon task4-shannon.cpp )
add_executable (task4_NC task4_NC.cpp )
add_executable (task4 task4.cpp )

add_executable (task5-1 task5-1.cpp )
add_executable (task5-2 task5-2.cpp )
add_executable (task5 task5.cpp )

target_link_libraries(task1 ${OpenCV_LIBS})
target_link_libraries(task2 ${OpenCV_LIBS})
target_link_libraries(task3 ${OpenCV_LIBS})

target_link_libraries(task5-1 ${OpenCV_LIBS})
target_link_libraries(task5-2 ${OpenCV_LIBS})
target_link_libraries(task5 ${OpenCV_LIBS})

target_link_libraries(task4_ACD ${GMPXX_LIBRARIES})
target_link_libraries(task4_ACD ${GMP_LIBRARIES})
