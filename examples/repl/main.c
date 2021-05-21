// Copyright (c) 2013-2021 Cesanta Software Limited
// All rights reserved
//
// Example Elk REPL.
// cc main.c ../../elk.c -I../.. -o repl -lm

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "elk.h"

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer() {
  InputBuffer* input_buffer = malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

// Function that loads JS code from a given file.
static jsval_t require(struct js *js, const char *filename) {
  char data[32 * 1024];
  FILE *fp = fopen(filename, "rb");
  if (fp == NULL) return 0;
  size_t len = fread(data, 1, sizeof(data), fp);
  return js_eval(js, data, len);
}

void print_prompt() { printf("js > "); }

void read_input(InputBuffer* input_buffer) {
  ssize_t bytes_read =
      getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    printf("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  input_buffer->input_length = bytes_read - 1;
  input_buffer->buffer[bytes_read - 1] = 0;
}

void close_input_buffer(InputBuffer* input_buffer) {
    free(input_buffer->buffer);
    free(input_buffer);
}

int main(int argc, char *argv[]) {
  InputBuffer* input_buffer = new_input_buffer();
  char mem[8192];
  struct js *js = js_create(mem, sizeof(mem));

  // Import our custom function "require" into the global namespace.
  js_set(js, js_glob(js), "require", js_import(js, (uintptr_t) require, "jms"));
  
  while (true) {
    print_prompt();
    read_input(input_buffer);
    jsval_t res = js_eval(js, input_buffer->buffer, strlen(input_buffer->buffer));
    printf("%s\n", js_str(js, res));
  }
}

