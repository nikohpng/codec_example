# codec_example
codec_example是实现一个简单且可运行的视频的编解码例子集合

codec_example is a collection of simple and runnable video codec examples

## Enviroment

- [x] ffmpeg-4.2.2
- [x] Centos-7.6.1810
- [x] gcc-4.8.5

## Usage

里面实现的所有内容都是在Linux环境下实现，Win环境未做任何测试。

all of example edit in linux。if you want to use it in win,you need to test by yourself。

## Features
- [x] rgb24 convert to yuv. eg: yuv444, yv24, yuyv, uyvy, yv16, nv16, I422, I420(420p), yv12, nv12, nv21
- [x] yuv convert to rgb24. eg:yuv420p, yuv422, yuv444.
- [x] yuv convert to h264, mp4 by ffmpeg
- [x] format convert to pcm by ffmpeg. eg: MP4 convert to pcm
- [x] pcm convert to format by ffmpeg. eg: pcm convert to mp3、wav
- [x] pcm convert to codec by libopus. eg: pcm convert to opus
- [ ] yuv convert to h264 by open264

