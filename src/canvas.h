// SPDX-License-Identifier: MIT
// Copyright (C) 2022 Artem Senichev <artemsen@gmail.com>

#pragma once

#include <cairo/cairo.h>
#include <stdbool.h>
#include <stdint.h>

// Grid background mode id
#define BACKGROUND_GRID UINT32_MAX

// Convert color components from RGB to float
#define RGB_RED(c)   ((double)((c >> 16) & 0xff) / 255.0)
#define RGB_GREEN(c) ((double)((c >> 8) & 0xff) / 255.0)
#define RGB_BLUE(c)  ((double)(c & 0xff) / 255.0)

/** Rotate angles. */
typedef enum {
    rotate_0,   ///< No rotate
    rotate_90,  ///< 90 degrees, clockwise
    rotate_180, ///< 180 degrees
    rotate_270  ///< 270 degrees, clockwise
} rotate_t;

/** Flags of the flip transformation. */
typedef enum {
    flip_none,
    flip_vertical,
    flip_horizontal,
} flip_t;

/** Scaling operations. */
typedef enum {
    scale_fit_or100,  ///< Fit to window, but not more than 100%
    scale_fit_window, ///< Fit to window size
    scale_100,        ///< Real image size (100%)
    zoom_in,          ///< Enlarge by one step
    zoom_out          ///< Reduce by one step
} scale_t;

/** Direction of movement. */
typedef enum {
    center_vertical,   ///< Center vertically
    center_horizontal, ///< Center horizontally
    step_left,         ///< One step to the left
    step_right,        ///< One step to the right
    step_up,           ///< One step up
    step_down          ///< One step down
} move_t;

/** Canvas context. */
typedef struct {
    double scale;    ///< Scale, 1.0 = 100%
    rotate_t rotate; ///< Rotation angle
    flip_t flip;     ///< Flip mode flags
    int x;           ///< X-coordinate of the top left corner
    int y;           ///< Y-coordinate of the top left corner
} canvas_t;

/** Rectangle description. */
typedef struct {
    int x;
    int y;
    int width;
    int height;
} rect_t;

/**
 * Reset canvas parameters to default values.
 * @param[in] canvas canvas context
 */
void reset_canvas(canvas_t* canvas);

/**
 * Draw image.
 * @param[in] canvas canvas context
 * @param[in] image surface to draw
 * @param[in] cairo paint context
 */
void draw_image(const canvas_t* canvas, cairo_surface_t* image, cairo_t* cairo);

/**
 * Draw background grid for transparent images.
 * @param[in] canvas canvas context
 * @param[in] image surface to draw
 * @param[in] cairo paint context
 */
void draw_grid(const canvas_t* canvas, cairo_surface_t* image, cairo_t* cairo);

/**
 * Move view point.
 * @param[in] canvas canvas context
 * @param[in] image image surface
 * @param[in] direction move direction
 * @return true if coordinates were changed
 */
bool move_viewpoint(canvas_t* canvas, cairo_surface_t* image, move_t direction);

/**
 * Apply scaling operation.
 * @param[in] canvas canvas context
 * @param[in] image image surface
 * @param[in] op scale operation type
 * @return true if scale was changed
 */
bool apply_scale(canvas_t* canvas, cairo_surface_t* image, scale_t op);

/**
 * Rotate image on 90 degrees.
 * @param[in] canvas canvas context
 * @param[in] clockwise rotate direction
 */
void apply_rotate(canvas_t* canvas, bool clockwise);

/**
 * Flip image.
 * @param[in] canvas canvas context
 * @param[in] flip axis type
 */
void apply_flip(canvas_t* canvas, flip_t flip);
