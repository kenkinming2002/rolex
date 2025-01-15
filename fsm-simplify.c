#include "fsm.h"
#include "radix_tree.h"

static size_t powerset_construction_impl(struct fsm *output, const struct fsm *input, struct rt *indices_map, bool states[input->states.item_count])
{
  // Epsilon expansion
  bool end_epsilon_expansion = false;
  while(!end_epsilon_expansion)
  {
    end_epsilon_expansion = true;
    for(size_t state_index=0; state_index<input->states.item_count; ++state_index)
      if(states[state_index])
      {
        const struct fsm_state *state = &input->states.items[state_index];
        for(size_t transition_index = 0; transition_index < state->transitions.item_count; ++transition_index)
        {
          const struct fsm_transition *transition = &state->transitions.items[transition_index];
          if(transition->value == FSM_EPSILON)
            if(!states[transition->target])
            {
              states[transition->target] = true;
              end_epsilon_expansion = false;
            }
        }
      }
  }

  // Check if we have visited the current states
  size_t depth = input->states.item_count;
  size_t index = output->states.item_count;
  if(rt_insert(indices_map, depth, states, &index))
  {
    // Append new output state
    da_append(output->states, (struct fsm_state){0});
    for(size_t state_index=0; state_index<input->states.item_count; ++state_index)
      if(states[state_index])
        da_append_many(output->states.items[index].accepts, input->states.items[state_index].accepts.items, input->states.items[state_index].accepts.item_count);

    // Check what state a reachable after a single transition.
    bool reachable[256];
    bool new_states[256][input->states.item_count];
    memset(reachable, 0, sizeof reachable);
    memset(new_states, 0, sizeof new_states);
    for(size_t state_index=0; state_index<input->states.item_count; ++state_index)
      if(states[state_index])
      {
        const struct fsm_state *state = &input->states.items[state_index];
        for(size_t transition_index = 0; transition_index < state->transitions.item_count; ++transition_index)
        {
          const struct fsm_transition *transition = &state->transitions.items[transition_index];
          if(transition->value != FSM_EPSILON)
          {
            reachable[transition->value] = true;
            new_states[transition->value][transition->target] = true;
          }
        }
      }

    // Establish the edge recurisvely
    for(int i=0; i<256; ++i)
      if(reachable[i])
      {
        size_t target = powerset_construction_impl(output, input, indices_map, new_states[i]);
        da_append(output->states.items[index].transitions, ((struct fsm_transition){ .value = i, .target = target, }));
      }
  }
  return index;
}

static void powerset_construction(struct fsm *output, struct fsm *input)
{
  struct rt indices_map = rt_create();

  bool states[input->states.item_count];
  memset(states, 0, sizeof states);
  states[0] = true;

  powerset_construction_impl(output, input, &indices_map, states);
}

int main()
{
  struct fsm input = {0};
  fsm_read(&input, stdin);

  struct fsm output = {0};
  powerset_construction(&output, &input);

  fsm_write(&output, stdout);
}

