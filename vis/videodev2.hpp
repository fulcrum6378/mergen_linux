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

#include <cstdint>
#include <ctime>

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
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

/* Luminance+Chrominance formats */
#define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y', 'U', 'Y', 'V') /* 16  YUV 4:2:2     */

#define VIDEO_MAX_PLANES               8

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "UnusedLocalVariable"

struct v4l2_capability {
    uint8_t driver[16];
    uint8_t card[32];
    uint8_t bus_info[32];
    uint32_t version;
    uint32_t capabilities;
    uint32_t device_caps;
    uint32_t reserved[3];
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
    uint32_t dataformat;
    uint32_t buffersize;
} __attribute__ ((packed));

struct v4l2_pix_format {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t field;        /* enum v4l2_field */
    uint32_t bytesperline;    /* for padding, zero if unused */
    uint32_t sizeimage;
    uint32_t colorspace;    /* enum v4l2_colorspace */
    uint32_t priv;        /* private data, depends on pixelformat */
    uint32_t flags;        /* format flags (V4L2_PIX_FMT_FLAG_*) */
    union {
        /* enum v4l2_ycbcr_encoding */
        uint32_t ycbcr_enc;
        /* enum v4l2_hsv_encoding */
        uint32_t hsv_enc;
    };
    uint32_t quantization;    /* enum v4l2_quantization */
    uint32_t xfer_func;    /* enum v4l2_xfer_func */
};

struct v4l2_plane_pix_format {
    uint32_t sizeimage;
    uint32_t bytesperline;
    uint16_t reserved[6];
} __attribute__ ((packed));

struct v4l2_pix_format_mplane {
    uint32_t width;
    uint32_t height;
    uint32_t pixelformat;
    uint32_t field;
    uint32_t colorspace;

    struct v4l2_plane_pix_format plane_fmt[VIDEO_MAX_PLANES];
    uint8_t num_planes;
    uint8_t flags;
    union {
        uint8_t ycbcr_enc;
        uint8_t hsv_enc;
    };
    uint8_t quantization;
    uint8_t xfer_func;
    uint8_t reserved[7];
} __attribute__ ((packed));

struct v4l2_rect {
    int32_t left;
    int32_t top;
    uint32_t width;
    uint32_t height;
};

struct v4l2_clip {
    struct v4l2_rect c;
    struct v4l2_clip *next;
};

struct v4l2_window {
    struct v4l2_rect w;
    uint32_t field;
    uint32_t chromakey;
    struct v4l2_clip *clips;
    uint32_t clipcount;
    void *bitmap;
    uint8_t global_alpha;
};

struct v4l2_vbi_format {
    uint32_t sampling_rate;
    uint32_t offset;
    uint32_t samples_per_line;
    uint32_t sample_format;
    int32_t start[2];
    uint32_t count[2];
    uint32_t flags;
    uint32_t reserved[2];
};

struct v4l2_sliced_vbi_format {
    uint16_t service_set;
    uint16_t service_lines[2][24];
    uint32_t io_size;
    uint32_t reserved[2];
};

struct v4l2_sdr_format {
    uint32_t pixelformat;
    uint32_t buffersize;
    uint8_t reserved[24];
} __attribute__ ((packed));

struct v4l2_format {
    uint32_t type{};
    union {
        struct v4l2_pix_format pix{};     /* V4L2_BUF_TYPE_VIDEO_CAPTURE */
        struct v4l2_pix_format_mplane pix_mp;  /* V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE */
        struct v4l2_window win;     /* V4L2_BUF_TYPE_VIDEO_OVERLAY */
        struct v4l2_vbi_format vbi;     /* V4L2_BUF_TYPE_VBI_CAPTURE */
        struct v4l2_sliced_vbi_format sliced;  /* V4L2_BUF_TYPE_SLICED_VBI_CAPTURE */
        struct v4l2_sdr_format sdr;     /* V4L2_BUF_TYPE_SDR_CAPTURE */
        struct v4l2_meta_format meta;    /* V4L2_BUF_TYPE_META_CAPTURE */
        uint8_t raw_data[200];                   /* user-defined */
    } fmt;
};

enum v4l2_memory {
    V4L2_MEMORY_MMAP = 1,
    V4L2_MEMORY_USERPTR = 2,
    V4L2_MEMORY_OVERLAY = 3,
    V4L2_MEMORY_DMABUF = 4,
};

struct v4l2_requestbuffers {
    uint32_t count;
    uint32_t type;        /* enum v4l2_buf_type */
    uint32_t memory;        /* enum v4l2_memory */
    uint32_t capabilities;
    uint32_t reserved[1];
};

struct v4l2_timecode {
    uint32_t type;
    uint32_t flags;
    uint8_t frames;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t userbits[4];
};

struct v4l2_buffer {
    uint32_t index;
    uint32_t type;
    uint32_t bytesused;
    uint32_t flags;
    uint32_t field;
    struct timeval timestamp;
    struct v4l2_timecode timecode;
    uint32_t sequence;

    /* memory location */
    uint32_t memory;
    union {
        uint32_t offset;
        unsigned long userptr;
        struct v4l2_plane *planes;
        int32_t fd;
    } m;
    uint32_t length;
    uint32_t reserved2;
    union {
        int32_t request_fd;
        uint32_t reserved;
    };
};

#pragma clang diagnostic pop

#endif //LINUX_VIDEODEV2_H