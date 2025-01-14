#include "fsm.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>

static int handle_escape_sequence(int c, const char *verbatim, FILE *file)
{
  if(c != '\\')
    return c;

  switch((c = fgetc(file)))
  {
  // We provide the single letter escape sequence as in C. This is
  // technically unnecessary since we actually support non-printable
  // character in input.
  case 'a': return '\a';
  case 'b': return '\b';
  case 'f': return '\f';
  case 'n': return '\n';
  case 'r': return '\r';
  case 't': return '\t';
  case 'v': return '\v';

  // Hex escape sequences. Again, this is unnecessary.
  case 'x':
    {
      int value = 0;
      for(int i=0; i<2; ++i)
      {
        int digit = fgetc(file);
        if('0' <= digit && digit <= '9') {
          digit = digit - '0';
        } else if('a' <= digit && digit <= 'f') {
          digit = digit - 'a' + 10;
        } else if('A' <= digit && digit <= 'F') {
          digit = digit - 'A' + 10;
        } else {
          fprintf(stderr, "error: unexpected %s when interpreting hex escape sequence\n", digit != EOF ? "character" : "eof");
          exit(EXIT_FAILURE);
        }

        value <<= 4;
        value += digit;
      }
      return value;
    }

    default:
      if(strchr(verbatim, c))
        return c;

      // The character after backslash is not special. Put it back and treat the
      // backslash as a regular backslash.
      ungetc(c, stdin);
    case EOF:
      return '\\';
  }
}

static bool fsm_from_regex_impl(struct fsm *fsm, FILE *file, size_t depth)
{
  size_t begin_state_index_greedy = fsm->states.item_count - 1;
  size_t begin_state_index = fsm->states.item_count - 1;

  int c;
  while((c = fgetc(file)) != EOF)
    switch(c)
    {
    // Selectors
    case '.':
      {
        begin_state_index = fsm->states.item_count - 1;
        for(int i=0; i<256; ++i)
          da_append(fsm->states.items[begin_state_index].transitions, ((struct fsm_transition){ .value = i, .target = fsm->states.item_count, }));
        da_append(fsm->states, (struct fsm_state){0});
      }
      break;
    case '[':
      {
        bool negate;
        switch((c = fgetc(file)))
        {
        case '^':
          negate = true;
          break;
        default:
          ungetc(c, stdin);
        case EOF:
          negate = false;
          break;
        }

        bool accept[256];
        for(int i=0; i<256; ++i)
          accept[i] = negate;

        int range_begin = -1;
        bool range = false;
        while((c = fgetc(file)) != EOF && c != ']')
        {
          if(c == '-' && range_begin != -1 && !range)
          {
            range = true;
            continue;
          }

          c = handle_escape_sequence(c, "-]", file);

          if(range)
          {
            for(int i=range_begin+1; i<=c; ++i) accept[i] = !negate;
            range_begin = -1;
            range = false;
          }
          else
          {
            accept[c] = !negate;
            range_begin = c;
          }
        }

        begin_state_index = fsm->states.item_count - 1;
        for(int i=0; i<256; ++i)
          if(accept[i])
            da_append(fsm->states.items[begin_state_index].transitions, ((struct fsm_transition){ .value = i, .target = fsm->states.item_count, }));
        da_append(fsm->states, (struct fsm_state){0});
      }
      break;
    default:
      {
        c = handle_escape_sequence(c, ".[()|?*+", file);

        begin_state_index = fsm->states.item_count - 1;
        da_append(fsm->states.items[begin_state_index].transitions, ((struct fsm_transition){ .value = c, .target = fsm->states.item_count, }));
        da_append(fsm->states, (struct fsm_state){0});
      }
      break;

    // Group
    case '(':
      begin_state_index = fsm->states.item_count - 1;
      if(!fsm_from_regex_impl(fsm, file, depth+1))
      {
        fprintf(stderr, "error: missing ) after (\n");
        exit(EXIT_FAILURE);
      }
      break;
    case ')':
      if(depth == 0)
      {
        fprintf(stderr, "error: unexpected ) without matching (\n");
        exit(EXIT_FAILURE);
      }
      return true;
    case '|':
      {
        size_t end_state_index_greedy = fsm->states.item_count - 1;

        da_append(fsm->states, (struct fsm_state){0});

        size_t begin_state_index_new = fsm->states.item_count - 1;
        bool result = fsm_from_regex_impl(fsm, file, depth);
        size_t end_state_index_new = fsm->states.item_count - 1;

        da_append(fsm->states.items[begin_state_index_greedy].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = begin_state_index_new }));
        da_append(fsm->states.items[end_state_index_greedy].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = end_state_index_new }));

        return result;
      }

    // Modifier
    case '?':
      da_append(fsm->states.items[begin_state_index].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = fsm->states.item_count - 1 }));
      break;
    case '*':
    case '+':
      da_append(fsm->states.items[fsm->states.item_count - 1].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = begin_state_index }));
      da_append(fsm->states.items[c == '*' ? begin_state_index : fsm->states.item_count-1].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = fsm->states.item_count }));
      da_append(fsm->states, (struct fsm_state){0});
      break;
    }

  return false;
}

static struct fsm fsm_from_regex(FILE *file)
{
  struct fsm fsm = fsm_create();
  fsm_from_regex_impl(&fsm, file, 0);
  da_back(fsm.states).accepting = true;
  return fsm;
}

int main()
{
  struct fsm fsm = fsm_from_regex(stdin);
  fsm_write(&fsm, stdout);
}

