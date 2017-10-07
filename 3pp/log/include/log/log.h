/*
 * Copyright (c) 2017 Jon Olsson <jlo@wintermute.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef log_log_h
#define log_log_h

#include <stdio.h>

#ifdef LOG_DISABLED
#define LOG(level, ...)
#else
#define LOG(level, ...) \
    do { \
      if (g_log_level <= level && level < LOG_OFF) { \
        log_print(level, __VA_ARGS__); \
      } \
    } while (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
    LOG_OFF
} log_Level;

extern log_Level g_log_level;

void log_init(log_Level level);
void log_init_color(log_Level level);
void log_init_out(log_Level level, FILE * out);
void log_init_err(log_Level level, FILE * err);
void log_init_out_err(log_Level level, FILE * out, FILE * err);
void log_print(log_Level level, char const * fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
