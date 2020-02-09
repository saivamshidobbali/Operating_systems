#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/syscall.h> 
#include <unistd.h>

uint64_t byte_sort(uint64_t arg);

uint64_t nibble_sort(uint64_t arg);

struct elt {
  char val;
  struct elt* link;
};

struct elt* circular_list(const char* str);

enum format_t {
  OCT = 66, BIN, HEX
};

int convert(enum format_t mode, const char* str, uint64_t* out);

void log_pid(void);