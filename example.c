#include "rolex.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

static bool read_entire_file(const char *path, char **data, size_t *length)
{
  FILE *file = fopen(path, "r");
  if(!file)
  {
    fprintf(stderr, "error: failed to open file: %s: %s\n", path, strerror(errno));
    return false;
  }

  long begin = ftell(file);
  if(begin == -1)
  {
    fprintf(stderr, "error: failed to tell file position: %s: %s\n", path, strerror(errno));
    return false;
  }

  if(fseek(file, 0, SEEK_END) == -1)
  {
    fprintf(stderr, "error: failed to seek in file: %s: %s\n", path, strerror(errno));
    return false;
  }

  long end = ftell(file);
  if(end == -1)
  {
    fprintf(stderr, "error: failed to tell file position: %s: %s\n", path, strerror(errno));
    return false;
  }

  if(fseek(file, 0, SEEK_SET) == -1)
  {
    fprintf(stderr, "error: failed to seek in file: %s: %s\n", path, strerror(errno));
    return false;
  }

  *length = end - begin;
  *data = malloc(*length);
  if(fread(*data, *length, 1, file) != 1)
  {
    fprintf(stderr, "error: failed to read from file: %s\n", path);
    return false;
  }

  return true;
}

enum token_type {
  TOKEN_TYPE_SPACE,
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_INTEGER,
  TOKEN_TYPE_FLOAT,
};

const char *token_type_to_str(enum token_type value)
{
  switch(value)
  {
  case TOKEN_TYPE_SPACE: return "space";
  case TOKEN_TYPE_IDENTIFIER: return "identifier";
  case TOKEN_TYPE_INTEGER: return "integer";
  case TOKEN_TYPE_FLOAT: return "float";
  }
}

static char *data;
static size_t length;

static char *curr_p;
static char *best_p;
static enum token_type best_type;

int rolex_getc(void)
{
  if(curr_p == data + length) return EOF;
  return *curr_p++;
}

#define accept(name, type) \
  void rolex_accept_##name(void) \
  { \
    best_p = curr_p; \
    best_type = type; \
  }

accept(space, TOKEN_TYPE_SPACE)
accept(identifier, TOKEN_TYPE_IDENTIFIER)
accept(integer, TOKEN_TYPE_INTEGER)
accept(float, TOKEN_TYPE_FLOAT)

static void usage(char *program_name)
{
  fprintf(stderr, "Usage: %s INPUT\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  if(argc != 2)
    usage(argv[0]);

  if(!read_entire_file(argv[1], &data, &length))
    return EXIT_FAILURE;

  while(length > 0)
  {
    curr_p = data;
    best_p = data;
    rolex_run();
    if(best_p == data)
    {
      printf("error: No chararacters could be accepted by the state machine\n");
      printf("error: Remaining input is <%.*s>\n", (int)length, data);
      return EXIT_FAILURE;
    }

    size_t n = best_p - data;
    printf("info: Accepted %s <%.*s>\n", token_type_to_str(best_type), (int)n, data);
    data += n;
    length -= n;
  }
}

