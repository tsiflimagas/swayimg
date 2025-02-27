// SPDX-License-Identifier: MIT
// Copyright (C) 2020 Artem Senichev <artemsen@gmail.com>

//
// GIF image format support
//

#include "common.h"

#include <cairo/cairo.h>
#include <gif_lib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// GIF signature
static const uint8_t signature[] = { 'G', 'I', 'F' };

// Buffer description for GIF reader
struct buffer {
    const uint8_t* data;
    const size_t size;
    size_t position;
};

// GIF reader callback, see `InputFunc` in gif_lib.h
static int gif_reader(GifFileType* gif, GifByteType* dst, int sz)
{
    struct buffer* buf = (struct buffer*)gif->UserData;
    if (sz >= 0 && buf && buf->position + sz <= buf->size) {
        memcpy(dst, buf->data + buf->position, sz);
        buf->position += sz;
        return sz;
    }
    return -1;
}

// GIF loader implementation
cairo_surface_t* load_gif(const uint8_t* data, size_t size, char* format,
                          size_t format_sz)
{
    cairo_surface_t* surface = NULL;

    // check signature
    if (size < sizeof(signature) ||
        memcmp(data, signature, sizeof(signature))) {
        return NULL;
    }

    struct buffer buf = {
        .data = data,
        .size = size,
        .position = 0,
    };

    int err;
    GifFileType* gif = DGifOpen(&buf, gif_reader, &err);
    if (!gif) {
        fprintf(stderr, "Unable to open GIF decoder: [%i] %s\n", err,
                GifErrorString(err));
        return NULL;
    }

    // decode with high-level API
    if (DGifSlurp(gif) != GIF_OK) {
        fprintf(stderr, "Unable to decode GIF: [%i] %s\n", err,
                GifErrorString(err));
        goto done;
    }
    if (!gif->SavedImages) {
        fprintf(stderr, "No saved images in GIF\n");
        goto done;
    }

    // prepare surface and metadata
    surface = create_surface(gif->SWidth, gif->SHeight, true);
    if (!surface) {
        return NULL;
    }
    snprintf(format, format_sz, "GIF");

    // we don't support animation, show the first frame only
    const GifImageDesc* frame = &gif->SavedImages->ImageDesc;
    const GifColorType* colors =
        gif->SColorMap ? gif->SColorMap->Colors : frame->ColorMap->Colors;
    uint32_t* base =
        (uint32_t*)(cairo_image_surface_get_data(surface) +
                    frame->Top * cairo_image_surface_get_stride(surface));
    for (int y = 0; y < frame->Height; ++y) {
        uint32_t* pixel = base + y * gif->SWidth + frame->Left;
        const uint8_t* raster = &gif->SavedImages->RasterBits[y * gif->SWidth];
        for (int x = 0; x < frame->Width; ++x) {
            const uint8_t color = raster[x];
            if (color != gif->SBackGroundColor) {
                const GifColorType* rgb = &colors[color];
                *pixel =
                    0xff000000 | rgb->Red << 16 | rgb->Green << 8 | rgb->Blue;
            }
            ++pixel;
        }
    }

    cairo_surface_mark_dirty(surface);

done:
    DGifCloseFile(gif, NULL);
    return surface;
}
