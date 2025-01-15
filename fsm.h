#ifndef FSM_H
#define FSM_H

#include "dynamic_array.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define FSM_EPSILON (-1)

struct fsm_transition
{
  // FIXME: Realistically, we only 256+1 distinct value here. 256 for 256 value
  // of a byte and the last for epsilon transition. We want to be able to use
  // uint8_t here, by how do we squeeze an out an extra possible value.
  int value;
  size_t target;
};

da_define(fsm_transitions, struct fsm_transition);

struct fsm_accept
{
  size_t length;
  char *data;
};

da_define(fsm_accepts, struct fsm_accept);

struct fsm_state
{
  struct fsm_transitions transitions;
  struct fsm_accepts accepts;
};

da_define(fsm_states, struct fsm_state);

struct fsm
{
  struct fsm_states states;
};

struct fsm fsm_create(void)
{
  struct fsm fsm = {0};
  da_append(fsm.states, (struct fsm_state){0});
  return fsm;
}

static void do_write_impl(const void *data, size_t size, FILE *file)
{
  if(fwrite(data, size, 1, file) != 1)
  {
    fprintf(stderr, "error: failed to write output\n");
    exit(EXIT_FAILURE);
  }
}

#define do_write(item) do_write_impl(&item, sizeof item, file)

static void fsm_transition_write(const struct fsm_transition *transition, FILE *file)
{
  do_write(transition->value);
  do_write(transition->target);
}

static void fsm_accept_write(const struct fsm_accept *accept, FILE *file)
{
  do_write(accept->length);
  do_write(*(char(*)[accept->length])accept->data);
}

static void fsm_state_write(const struct fsm_state *state, FILE *file)
{
  do_write(state->transitions.item_count);
  for(size_t i=0; i<state->transitions.item_count; ++i)
    fsm_transition_write(&state->transitions.items[i], file);

  do_write(state->accepts.item_count);
  for(size_t i=0; i<state->accepts.item_count; ++i)
    fsm_accept_write(&state->accepts.items[i], file);
}

static void fsm_write(const struct fsm *fsm, FILE *file)
{
  do_write(fsm->states.item_count);
  for(size_t i=0; i<fsm->states.item_count; ++i)
    fsm_state_write(&fsm->states.items[i], file);
}

static void do_read_impl(void *data, size_t size, FILE *file)
{
  if(fread(data, size, 1, file) != 1)
  {
    fprintf(stderr, "error: failed to read output\n");
    exit(EXIT_FAILURE);
  }
}

#define do_read(item) do_read_impl(&item, sizeof item, file)

static void fsm_transition_read(struct fsm_transition *transition, FILE *file)
{
  do_read(transition->value);
  do_read(transition->target);
}

static void fsm_accept_read(struct fsm_accept *accept, FILE *file)
{
  do_read(accept->length);
  accept->data = malloc(accept->length);
  do_read(*(char(*)[accept->length])accept->data);
}

static void fsm_state_read(struct fsm_state *state, FILE *file)
{
  do_read(state->transitions.item_count);
  da_alloc_exact(state->transitions);
  for(size_t i=0; i<state->transitions.item_count; ++i)
    fsm_transition_read(&state->transitions.items[i], file);

  do_read(state->accepts.item_count);
  da_alloc_exact(state->accepts);
  for(size_t i=0; i<state->accepts.item_count; ++i)
    fsm_accept_read(&state->accepts.items[i], file);
}

static void fsm_read(struct fsm *fsm, FILE *file)
{
  do_read(fsm->states.item_count);
  da_alloc_exact(fsm->states);
  for(size_t i=0; i<fsm->states.item_count; ++i)
    fsm_state_read(&fsm->states.items[i], file);

  if(fsm->states.item_count == 0)
  {
    fprintf(stderr, "error: empty fsm\n");
    exit(EXIT_FAILURE);
  }
}

static void fsm_read_from(struct fsm *fsm, const char *path)
{
  FILE *file = fopen(path, "r");
  if(!file)
  {
    fprintf(stderr, "error: failed to open file %s for reading: %s\n", path, strerror(errno));
    exit(EXIT_FAILURE);
  }
  fsm_read(fsm, file);
}

#endif // FSM_H
