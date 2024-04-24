## Mergen Linux

Started at 16 October 2023, 07:11 as a subproject of [**Mergen IV**](https://github.com/fulcrum6378/mergen_android).

### How It Works

This is a mere command-line program. As soon as the program starts,
recording from first detected webcam, microphone and touchpad will begin.
The main thread listens if you press the Enter button; then it will stop recording.

~$ `make`

~$ `build/Mergen`

### History

#### Phase I

This project was initially regarded as a temporary bridge between Android and FreeBSD;
because FreeBSD has limited utilities for webcam, microphone and touch, compared to Linux,
I decided to first mount Mergen on Linux (Ubuntu) and then for (Free)BSD.

But then I realised that FreeBSD uses the same webcam drivers of Linux
ported as [webcamd](https://github.com/hselasky/webcamd), so it can easily be modified for use in FreeBSD.
By bringing [**Video4Linux2**](https://en.wikipedia.org/wiki/Video4Linux) headers,
I finally made it run on FreeBSD.

I had chosen [*Open Sound System (OSS)*](https://en.wikipedia.org/wiki/Open_Sound_System) for the task,
but Ubuntu had dropped support for it, therefore I couldn't use Ubuntu anymore.
I decided to completely switch to FreeBSD,
so I forked this project as [**Mergen4BSD**](https://github.com/fulcrum6378/mergen4bsd).

And MergenLinux was archived at 12 November 2023.

#### Phase II

After I realised that FreeBSD has so limited support for Vulkan, OpenCL and CUDA,
I was disappointed with FreeBSD and archived Mergen4BSD at 21 April 2024! And decided to continue MergenLinux.

### License

```
Copyright © Mahdi Parastesh - USE IT WELL!!
```
