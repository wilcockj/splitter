#ifndef LOG_H
#define LOG_H

typedef enum { DEBUG, WARNING, ERROR } LogLevel;

void pulog(LogLevel level, const char *file, const int line_number,
           const char *msg, ...);

void set_loglevel(LogLevel level);

#ifdef PULOG_IMPLEMENTATION

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOG(level, msg, ...)                                                   \
  pulog(level, __FILE__, __LINE__, msg, ##__VA_ARGS__)

static LogLevel LOG_LEVEL = DEBUG; // default level debug

void set_loglevel(LogLevel level) { LOG_LEVEL = level; }

void pulog(LogLevel level, const char *file, const int line_number,
           const char *msg, ...) {
  if (level == DEBUG && LOG_LEVEL <= DEBUG) {
    printf("\033[0;34mDEBUG: ");
  } else if (level == WARNING && LOG_LEVEL <= WARNING) {
    printf("\033[1;33mWARNING: ");
  } else if (level == ERROR && LOG_LEVEL <= ERROR) {
    printf("\033[1;31mERROR: ");
  } else {
    return;
  }
  printf("\033[0m");

  time_t now = time(NULL);
  struct tm tm_now;
  localtime_r(&now, &tm_now);
  char buff[100];
  strftime(buff, sizeof(buff), "%T", &tm_now);
  printf("%s: ", buff);
  printf("%s:%d: ", file, line_number);

  va_list args;
  va_start(args, msg);
  vprintf(msg, args);
  va_end(args);

  if (level == DEBUG) {
    printf("...\n");
  } else if (level == WARNING) {
    printf(".\n");
  } else if (level == ERROR) {
    printf("!\n");
    exit(-1);
  }
}

#endif

#endif // !LOG_H
