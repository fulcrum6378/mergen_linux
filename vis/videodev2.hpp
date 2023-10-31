/*
 *  Video for Linux Two header file
 *
 *  Copyright (C) 1999-2012 the contributors
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  Alternatively you can redistribute this file under the terms of the
 *  BSD license as stated below:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	Header file for v4l or V4L2 drivers and applications
 * with public API.
 * All kernel-specific stuff were moved to media/v4l2-dev.h, so
 * no #if __KERNEL tests are allowed here
 *
 *	See https://linuxtv.org for more info
 *
 *	Author: Bill Dirks <bill@thedirks.org>
 *		Justin Schoeman
 *              Hans Verkuil <hverkuil@xs4all.nl>
 *		et al.
 */
#ifndef LINUX_VIDEODEV2_H
#define LINUX_VIDEODEV2_H

#include <asm-generic/int-ll64.h>
#include <ctime>

#define VIDEO_MAX_PLANES               8

/*
 * I O C T L   C O D E S   F O R   V I D E O   D E V I C E S
 */
#define VIDIOC_QUERYCAP         _IOR('V',  0, struct v4l2_capability)
#define VIDIOC_S_FMT           _IOWR('V',  5, struct v4l2_format)
#define VIDIOC_REQBUFS         _IOWR('V',  8, struct v4l2_requestbuffers)
#define VIDIOC_QUERYBUF        _IOWR('V',  9, struct v4l2_buffer)
#define VIDIOC_QBUF            _IOWR('V', 15, struct v4l2_buffer)
#define VIDIOC_DQBUF           _IOWR('V', 17, struct v4l2_buffer)
#define VIDIOC_STREAMON         _IOW('V', 18, int)
#define VIDIOC_STREAMOFF        _IOW('V', 19, int)

/*  Four-character-code (FOURCC) */
#define v4l2_fourcc(a, b, c, d)\
    ((__u32)(a) | ((__u32)(b) << 8) | ((__u32)(c) << 16) | ((__u32)(d) << 24))

/* Luminance+Chrominance formats */
#define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y', 'U', 'Y', 'V') /* 16  YUV 4:2:2     */

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedLocalVariable"

struct v4l2_capability {
    __u8 driver[16];
    __u8 card[32];
    __u8 bus_info[32];
    __u32 version;
    __u32 capabilities;
    __u32 device_caps;
    __u32 reserved[3];
};

enum v4l2_buf_type {
    V4L2_BUF_TYPE_VIDEO_CAPTURE = 1,
    /*V4L2_BUF_TYPE_VIDEO_OUTPUT         = 2,
    V4L2_BUF_TYPE_VIDEO_OVERLAY        = 3,
    V4L2_BUF_TYPE_VBI_CAPTURE          = 4,
    V4L2_BUF_TYPE_VBI_OUTPUT           = 5,
    V4L2_BUF_TYPE_SLICED_VBI_CAPTURE   = 6,
    V4L2_BUF_TYPE_SLICED_VBI_OUTPUT    = 7,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY = 8,
    V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE = 9,
    V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE  = 10,
    V4L2_BUF_TYPE_SDR_CAPTURE          = 11,
    V4L2_BUF_TYPE_SDR_OUTPUT           = 12,
    V4L2_BUF_TYPE_META_CAPTURE         = 13,
    V4L2_BUF_TYPE_META_OUTPUT	   = 14,
    *//* Deprecated, do not use *//*
    V4L2_BUF_TYPE_PRIVATE              = 0x80,*/
};

enum v4l2_field {
    // ...
    V4L2_FIELD_NONE = 1, /* this device has no fields ... */
    // ...
};

struct v4l2_meta_format {
    __u32 dataformat;
    __u32 buffersize;
} __attribute__ ((packed));

struct v4l2_pix_format {
    __u32 width;
    __u32 height;
    __u32 pixelformat;
    __u32 field;        /* enum v4l2_field */
    __u32 bytesperline;    /* for padding, zero if unused */
    __u32 sizeimage;
    __u32 colorspace;    /* enum v4l2_colorspace */
    __u32 priv;        /* private data, depends on pixelformat */
    __u32 flags;        /* format flags (V4L2_PIX_FMT_FLAG_*) */
    union {
        /* enum v4l2_ycbcr_encoding */
        __u32 ycbcr_enc;
        /* enum v4l2_hsv_encoding */
        __u32 hsv_enc;
    };
    __u32 quantization;    /* enum v4l2_quantization */
    __u32 xfer_func;    /* enum v4l2_xfer_func */
};

struct v4l2_plane_pix_format {
    __u32 sizeimage;
    __u32 bytesperline;
    __u16 reserved[6];
} __attribute__ ((packed));

struct v4l2_pix_format_mplane {
    __u32 width;
    __u32 height;
    __u32 pixelformat;
    __u32 field;
    __u32 colorspace;

    struct v4l2_plane_pix_format plane_fmt[VIDEO_MAX_PLANES];
    __u8 num_planes;
    __u8 flags;
    union {
        __u8 ycbcr_enc;
        __u8 hsv_enc;
    };
    __u8 quantization;
    __u8 xfer_func;
    __u8 reserved[7];
} __attribute__ ((packed));

struct v4l2_rect {
    __s32 left;
    __s32 top;
    __u32 width;
    __u32 height;
};

struct v4l2_clip {
    struct v4l2_rect c;
    struct v4l2_clip *next;
};

struct v4l2_window {
    struct v4l2_rect w;
    __u32 field;
    __u32 chromakey;
    struct v4l2_clip *clips;
    __u32 clipcount;
    void *bitmap;
    __u8 global_alpha;
};

struct v4l2_vbi_format {
    __u32 sampling_rate;
    __u32 offset;
    __u32 samples_per_line;
    __u32 sample_format;
    __s32 start[2];
    __u32 count[2];
    __u32 flags;
    __u32 reserved[2];
};

struct v4l2_sliced_vbi_format {
    __u16 service_set;
    __u16 service_lines[2][24];
    __u32 io_size;
    __u32 reserved[2];
};

struct v4l2_sdr_format {
    __u32 pixelformat;
    __u32 buffersize;
    __u8 reserved[24];
} __attribute__ ((packed));

struct v4l2_format {
    __u32 type{};
    union {
        struct v4l2_pix_format pix{};     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
        struct v4l2_pix_format_mplane pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
        struct v4l2_window win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
        struct v4l2_vbi_format vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
        struct v4l2_sliced_vbi_format sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
        struct v4l2_sdr_format sdr;     /* V4L2_BUF_TYPE_SDR_CAPTURE */
        struct v4l2_meta_format meta;    /* V4L2_BUF_TYPE_META_CAPTURE */
        __u8 raw_data[200];                   /* user-defined */
    } fmt;
};

enum v4l2_memory {
    V4L2_MEMORY_MMAP = 1,
    V4L2_MEMORY_USERPTR = 2,
    V4L2_MEMORY_OVERLAY = 3,
    V4L2_MEMORY_DMABUF = 4,
};

struct v4l2_requestbuffers {
    __u32 count;
    __u32 type;        /* enum v4l2_buf_type */
    __u32 memory;        /* enum v4l2_memory */
    __u32 capabilities;
    __u32 reserved[1];
};

struct v4l2_timecode {
    __u32 type;
    __u32 flags;
    __u8 frames;
    __u8 seconds;
    __u8 minutes;
    __u8 hours;
    __u8 userbits[4];
};

/*#if __TIMESIZE == 64 && defined __LIBC
# define __time64_t __time_t
#elif __TIMESIZE != 64
typedef __TIME64_T_TYPE __time64_t;
#endif
struct timeval {
#ifdef __USE_TIME_BITS64
    __time64_t tv_sec;		*//* Seconds.  *//*
  __suseconds64_t tv_usec;	*//* Microseconds.  *//*
#else
    __time_t tv_sec;        *//* Seconds.  *//*
    __suseconds_t tv_usec;    *//* Microseconds.  *//*
#endif
};*/

struct v4l2_buffer {
    __u32 index;
    __u32 type;
    __u32 bytesused;
    __u32 flags;
    __u32 field;
    struct timeval timestamp;
    struct v4l2_timecode timecode;
    __u32 sequence;

    /* memory location */
    __u32 memory;
    union {
        __u32 offset;
        unsigned long userptr;
        struct v4l2_plane *planes;
        __s32 fd;
    } m;
    __u32 length;
    __u32 reserved2;
    union {
        __s32 request_fd;
        __u32 reserved;
    };
};

#pragma clang diagnostic pop

#endif //LINUX_VIDEODEV2_H