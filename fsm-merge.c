#include "fsm.h"

static void fsm_merge(struct fsm *dst, struct fsm *src)
{
  if(dst->states.item_count == 0 || src->states.item_count == 0)
  {
    fprintf(stderr, "error: empty fsm\n");
    exit(EXIT_FAILURE);
  }

  for(size_t state_index=0; state_index<src->states.item_count; ++state_index)
  {
    struct fsm_state *state = &src->states.items[state_index];
    for(size_t transition_index=0; transition_index<state->transitions.item_count; ++transition_index)
    {
      struct fsm_transition *transition = &state->transitions.items[transition_index];
      transition->target += dst->states.item_count;
    }
  }

  da_append(dst->states.items[0].transitions, ((struct fsm_transition) { .value = FSM_EPSILON, .target = dst->states.item_count }));
  da_append_many(dst->states, src->states.items, src->states.item_count);
  da_reset(src->states);
}

int main(int argc, char *argv[])
{
  struct fsm acc = fsm_create();
  struct fsm fsm = {0};
  for(int i=1; i<argc; ++i)
  {
    fsm_read_from(&fsm, argv[i]);
    fsm_merge(&acc, &fsm);
  }
  fsm_write(&acc, stdout);
}
