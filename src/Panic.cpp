#include "Panic.h"

#ifndef __XTENSA__
#include <stdlib.h>
#include <unistd.h>
#else
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

#include <stdio.h>
#include <cstdarg>

void Panic(const char *aFmt, ...) {
  char msg[4096];
  va_list args;
  va_start(args, aFmt);
  vsprintf(msg, aFmt, args);
  printf("PANIC: %s\n", msg);
#ifndef __XTENSA__
  exit(1);
#else
  while (true) {
    vTaskDelay(10000);
  }
#endif
}

void Dump(TUint32 *ptr, int width, int height) {
  TUint32 addr = 0;
  for (int r = 0; r < height; r++) {
    printf("%08x ", addr);
    addr += width;
    for (int i=0; i<width; i++) {
      printf("%08x ", *ptr++);
    }
    printf("\n");
  }
}

void Dump(TUint8 *ptr, int width, int height) {
  printf("%d resources %d\n", width, height);
  TUint32 addr = 0;
  for (int r = 0; r < height; r++) {
    printf("%08x ", addr);
    addr += width;
    for (int i = 0; i < width; i++) {
      printf("%02x ", *ptr++);
    }
    printf("\n");
  }
}
