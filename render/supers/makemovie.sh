#!/bin/bash
avconv -framerate 29 -i frame%d.png -b 25M -threads 0 -f mp4 -vcodec h264 -pix_fmt yuv420p  flops.mp4

