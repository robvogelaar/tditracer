/* $Copyright: $
 * Copyright (c) 2007 by Steve Baker (ice@mama.indstate.edu)
 * All Rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Make: gcc -O4 -o memspeed memspeed.c -s
 *   Add -march=<your architecture> to make better.
 */
/*
 * Memspeed: Measures your memory speed.
 * -a = tries to eliminate CPU overhead to get at raw memory speed.
 *      Probably not truely accurate.
 */
#define _GNU_SOURCE
#define LARGEFILE64_SOURCE
#define FILE_OFFSET_BITS 64
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <string.h>
#include <stdint.h>

double time2double(struct timeval start, struct timeval end);

/* 256 bit structure */
struct bigbit {
  int32_t x[8];
};

int main(int argc, char **argv) {
  struct timeval start, end;
  char *mem;
  double t, at, mb, tbytes;
  int64_t megs, iter, bytes, i, j, c, adjtime = 0;
  uint8_t b;
  uint32_t x;
  uint64_t llx;
  struct bigbit bbx;

  volatile uint8_t *m;
  volatile uint32_t *lm;
  volatile uint64_t *llm;
  volatile struct bigbit *bbm;

  setvbuf(stdout, (char *)NULL, _IONBF, 0);

  if (argc < 2) {
    fprintf(stderr, "usage: memspeed [-a] <megs of memory to allocate>\n");
    return 1;
  }
  i = 1;
  if (!strcmp(argv[i], "-a")) {
    adjtime = 1;
    i++;
  }

  megs = atoi(argv[i]);
  iter = 8192 / megs;
  mb = iter * megs;
  bytes = megs * 1024 * 1024;
  tbytes = (double)bytes * (double)iter;

  mem = malloc(bytes + 4);
  if (mem == NULL) {
    fprintf(stderr, "memspeed: memory allocation.\n");
    return 1;
  }

  /* init memory to reduce cache effects for inital test */
  printf("Initializing...");
  memset(mem, 0, bytes);
  if (adjtime) {
    gettimeofday(&start, NULL);
    for (i = 0; i < iter; i++)
      for (m = mem, j = bytes; j; j--) m++;
    gettimeofday(&end, NULL);
    at = time2double(start, end);
  }

  printf(
      "\r Tests iterate over %dMB, %d times for a simulated total of "
      "%.0fMB\n\n",
      megs, iter, mb);

//  goto three;

one:
  /* Test one: read bytes at a time. */
  printf("Test  1: reading bytes         : ");

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (m = mem, j = bytes; j; j--) b = *m++;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= at;

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

two:
  /* Test two: write bytes at a time. */
  printf("Test  2: writing bytes         : ");

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (m = mem, j = bytes; j; j--) *m++ = 0;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= at;

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

three:
  /* Test three: read 32 bit longs at a time. */
  printf("Test  3: reading %d bit longs  : ", sizeof(uint32_t) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (lm = (uint32_t *)mem, j = bytes / sizeof(uint32_t); j; j--) x = *lm++;
  }
  gettimeofday(&end, NULL);
  x++;
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(uint32_t));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

four:
  /* Test four: write 32 bit longs at a time. */
  printf("Test  4: writing %d bit longs  : ", sizeof(uint32_t) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (lm = (uint32_t *)mem, j = bytes / sizeof(uint32_t); j; j--)
      *lm++ = (uint32_t)0;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(uint32_t));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

five:
  /* Test five: read 64 bit longs at a time. */
  printf("Test  5: reading %d bit longs  : ", sizeof(uint64_t) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (llm = (uint64_t *)mem, j = bytes / sizeof(uint64_t); j; j--)
      llx = *llm++;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(uint64_t));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

six:
  /* Test six: write 64 bit longs at a time. */
  printf("Test  6: writing 64 bit longs  : ", sizeof(uint64_t) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (llm = (uint64_t *)mem, j = bytes / sizeof(uint64_t); j; j--)
      *llm++ = 0L;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(uint64_t));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

seven:
  /* Test seven: read 256 bit struct at a time. */
  printf("Test  7: reading %d bit struct: ", sizeof(struct bigbit) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (bbm = (struct bigbit *)mem, j = bytes / sizeof(struct bigbit); j; j--)
      bbx = *bbm++;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(struct bigbit));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

eight:
  /* Test eight: write 256 bit struct at a time. */
  printf("Test  8: writing %d bit struct: ", sizeof(struct bigbit) * 8);

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (bbm = (struct bigbit *)mem, j = bytes / sizeof(struct bigbit); j; j--)
      *bbm++ = bbx;
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(struct bigbit));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

nine:
  /* Test eight: read 256 bit struct with memcpy() */
  printf("Test  9: reading with memcpy() : ");

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    for (bbm = (struct bigbit *)mem, j = bytes / sizeof(struct bigbit); j;
         j--, bbm++)
      memcpy((void *)&bbx, (void *)bbm, sizeof(bbx));
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);
  if (adjtime) t -= (at / (double)sizeof(struct bigbit));

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

ten:
  /* Test nine: write with memset at a time. */
  printf("Test 10: writing with memset() : ");

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    memset(mem, 0, bytes);
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

eleven:
  /* Test eight: copy with memcpy() */
  printf("Test 11: copying with memcpy() : ");

  x = bytes / 2;

  gettimeofday(&start, NULL);
  for (i = 0; i < iter; i++) {
    memcpy(mem + x, mem, x);
    memcpy(mem, mem + x, x);
  }
  gettimeofday(&end, NULL);
  t = time2double(start, end);

  printf("%5.2f seconds, %7.2f MB/s\n", t, mb / t);

  return 0;
}

double time2double(struct timeval start, struct timeval end) {
  double secs;

  if (end.tv_sec <= start.tv_sec) {
    secs = (double)end.tv_sec - (double)start.tv_sec;
    secs += (end.tv_usec - start.tv_usec) / (double)1000000;
  } else {
    secs = ((double)end.tv_sec - (double)start.tv_sec) - 1.0;
    secs += ((1000000.0 + (double)end.tv_usec) - (double)start.tv_usec) /
            (double)1000000;
  }
  return secs;
}
