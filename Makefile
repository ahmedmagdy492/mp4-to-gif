CPP=g++
LIBS=-lavformat -lavcodec -lavutil -lswscale -lswresample -lavdevice -lstdc++
CFLAGS=-g

main:	main.cpp include/stb_image_write.h
	$(CPP) main.cpp stb_image.cpp $(LIBS) $(CFLAGS) -o mp4-to-gif && mkdir output
clear:
	rm -rf main
	rm -rf output/
