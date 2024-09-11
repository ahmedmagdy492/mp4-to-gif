# mp4-to-gif

## Description

Up until this moment the utility can produce gifs with the netscape loop application extension but it has some downsides since it's still under development:
 - the produced gif is large in size (still working on improving that)
 - the produced gif is in gray scale still looking for a good way to create either create one global color table and let all frame use it or create a local color table for each frame

for more info visit this gitbook page:
[![my gitbook](https://ahmedmagdy492s-organization.gitbook.io/programming-adventure/gif-programming)](https://ahmedmagdy492s-organization.gitbook.io/programming-adventure/gif-programming)

## Resources
[![giflib docs](https://giflib.sourceforge.net/gif_lib.html#EGifPutPixel)](https://giflib.sourceforge.net/gif_lib.html#EGifPutPixel)
[![gif animation](https://giflib.sourceforge.net/whatsinagif/animation_and_transparency.html)](https://giflib.sourceforge.net/whatsinagif/animation_and_transparency.html)
[![gif structure](https://giflib.sourceforge.net/whatsinagif/bits_and_bytes.html)](https://giflib.sourceforge.net/whatsinagif/bits_and_bytes.html)

## Usage

```console
$ make clear && make
$ ./mp4-to-gif <mp4-video-path.mp4> <no-of-frames-to-extract>
```

NOTE: no of frames to extract should be less than the no of frames in the video
