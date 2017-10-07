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

#include <log/log.h>
#include <stdbool.h>
#include <stdarg.h>
#include <assert.h>

#define RED        "\033[40m\033[31m"
#define GREEN      "\033[40m\033[32m"
#define YELLOW     "\033[40m\033[33m"
#define BLUE       "\033[40m\033[34m"
#define MAGENTA    "\033[40m\033[35m"
#define CYAN       "\033[40m\033[36m"
#define WHITE      "\033[40m\033[37m"
#define END_LINE   "\033[0m"

static bool g_color = false;
static FILE * g_out = NULL;
static FILE * g_err = NULL;

static
inline
char const *
log_begin(log_Level const level)
{
  if (g_color) {
    switch (level) {
    case LOG_DEBUG:   return    CYAN "DEBUG: ";
    case LOG_INFO:    return   WHITE "INFO: ";
    case LOG_WARNING: return  YELLOW "WARNING: ";
    case LOG_ERROR:   return     RED "ERROR: ";
    case LOG_FATAL:   return MAGENTA "FATAL: ";
    case LOG_OFF:     return    BLUE "OFF: ";
    }
    return "<INVALID>";
  } else {
    switch (level) {
    case LOG_DEBUG:   return "DEBUG: ";
    case LOG_INFO:    return "INFO: ";
    case LOG_WARNING: return "WARNING: ";
    case LOG_ERROR:   return "ERROR: ";
    case LOG_FATAL:   return "FATAL: ";
    case LOG_OFF:     return "OFF: ";
    }
    return "<INVALID>";
  }
}

static
inline
char const *
log_end()
{
  return g_color ? END_LINE "\n" : "\n";
}

log_Level g_log_level = LOG_WARNING;

void
log_init(log_Level const level)
{
  if (!g_out) {
    g_log_level = level;
    g_out = stdout;
    g_err = stderr;
  }
}

void
log_init_color(log_Level const level)
{
  if (!g_out) {
    g_log_level = level;
    g_color = true;
    g_out = stdout;
    g_err = stderr;
  }
}

void
log_init_out(log_Level const level,
             FILE * const out)
{
  assert(out && "out is NULL");
  if (!g_out) {
    g_log_level = level;
    g_out = out;
    g_err = stderr;
  }
}

void
log_init_err(log_Level const level,
             FILE * const err)
{
  assert(err && "err is NULL");
  if (!g_out) {
    g_log_level = level;
    g_out = stdout;
    g_err = err;
  }
}

void
log_init_out_err(log_Level const level,
                 FILE * const out,
                 FILE * const err)
{
  assert(out && "out is NULL");
  assert(err && "err is NULL");
  if (!g_out) {
    g_log_level = level;
    g_out = out;
    g_err = err;
  }
}

void
log_print(log_Level const level, char const * const fmt, ...)
{
  assert(fmt && "fmt is NULL");
  FILE * const out = level > LOG_INFO ? g_err : g_out;
  fputs(log_begin(level), out);
  va_list args;
  va_start(args, fmt);
  vfprintf(out, fmt, args);
  va_end(args);
  fputs(log_end(), out);
  fflush(out);
}
