## Mergen Linux

Started at 16 October 2023, 07:11 as a subproject of [**Mergen**](https://github.com/fulcrum6378/mergen_android).

Since FreeBSD has limited utilities for webcam, microphone and touch, compared to Linux,
I decided to first mount Mergen on Linux (Ubuntu) and then for (Free)BSD.

But then I realised that FreeBSD uses the same webcam drivers of Linux
ported as [**webcamd**](https://github.com/hselasky/webcamd), so it can easily be modified for use in FreeBSD.
By copying some of [**Video4Linux2**](https://en.wikipedia.org/wiki/Video4Linux) headers
as [*videodev2.hpp*](vis/videodev2.hpp), I finally made it run on FreeBSD.

Ubuntu had dropped support for [**Open Sound System (OSS)**](https://en.wikipedia.org/wiki/Open_Sound_System),
so I couldn't use it anymore. I decided to switch to FreeBSD completely,
so I forked this project as [**Mergen4BSD**](https://github.com/fulcrum6378/mergen4bsd).

And MergenLinux was archived at 12 November 2023.

### Compilation

- [GNUmakefile](GNUmakefile) for **Linux**
- [Makefile](Makefile) for **FreeBSD**

But the only thing you need to do regardless of your OS, is to open terminal in the project folder and call:

~$ `make`

### Execution

This is a mere command-line program. As soon as the program starts,
recording from first detected webcam, microphone and touchpad will begin.
The main thread listens if you press the Enter button; then it will stop recording.

~$ `build/MergenLinux`

### License

```
Copyright © Mahdi Parastesh - All Rights Reserved.
```
