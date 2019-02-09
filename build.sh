#!/bin/bash

g++ quads.cpp main.cpp -lopencv_core -lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc \
	-ltesseract
