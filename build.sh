#!/bin/bash

CXX_FLAGS=-DDEBUG

g++ days.cpp quads.cpp main.cpp ${CXX_FLAGS} --std=c++17 -I/usr/local/Cellar/opencv/4.0.1/include/opencv4 -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -ltesseract
