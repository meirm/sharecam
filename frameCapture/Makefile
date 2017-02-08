all: frameCapture

frameCapture: frameCapture.cpp
	g++ -o  frameCapture frameCapture.cpp -I /usr/local/opencv-2.4.13/include -O2 -g -Wall -L /usr/local/opencv-2.4.13/lib -lopencv_core  -lopencv_imgcodecs -lopencv_videoio -lopencv_highgui -lopencv_video  -lopencv_photo -lopencv_imgproc  -std=gnu++11  -lboost_program_options -std=gnu++11 -lrt -pthread

install: frameCapture frameCapture.sh 
	mkdir -p /usr/local/share/riunx/sharedcam/bin
	cp frameCapture /usr/local/share/riunx/sharedcam/bin/
	cp frameCapture.sh frameCapture /usr/local/bin

.PHONY: clean
