# hrbek
OpenCV&amp;Tesseract reader of the *.jpg menu of the most famous restaurant in the World:: >> <br>
http://www.stravovanihrbek.websnadno.cz/jidelnicek.html

# prerequisties
## 1) [opencv4](https://github.com/opencv/opencv)
 * opencv_core
 * opencv_highgui 
 * opencv_imgcodecs 
 * opencv_imgproc 
 
 ## 2) [tesseract](https://github.com/tesseract-ocr/tesseract)
 
 # build 
 As simple as triggering `build.sh` (considering all prereqisities are satisfied)
 
# usage
Download jpg from Hrbek's URL to local storage e.g at `path/to/hrbek/menu.jpg` <br>
`$ ./a.out path/to/hrbek/menu.jpg` -> it will generate `hrbek.txt` that may be processed further on
