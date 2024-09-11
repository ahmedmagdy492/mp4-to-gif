#!/bin/bash

set x+
make clear && make
./mp4-to-gif ~/Downloads/video.mp4 100
firefox output/out.gif
