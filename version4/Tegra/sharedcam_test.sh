#!/bin/bash
srv4.sh  --video 0 --xres 1920 --yres 1080 &
sleep 1;
cli4.sh  --resize 0.15 --maxfps 30.0


