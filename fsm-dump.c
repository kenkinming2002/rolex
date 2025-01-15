#include "fsm.h"

#include <string.h>

static void fsm_dump_dot(const struct fsm *fsm, FILE *out)
{
  fprintf(out, "digraph {\n");
  for(size_t state_index=0; state_index<fsm->states.item_count; ++state_index)
  {
    const struct fsm_state *state = &fsm->states.items[state_index];
    if(state->accepts.item_count > 0)
      fprintf(out, "  %zu [color=\"green\"];", state_index);

    for(size_t transition_index=0; transition_index<state->transitions.item_count; ++transition_index)
    {
      const struct fsm_transition *transition = &state->transitions.items[transition_index];
      fprintf(out, "  %zu -> %zu [label=\"", state_index, transition->target);
      if(transition->value != FSM_EPSILON)
        printf("%d", transition->value);
      else
        printf("ε");
      fprintf(out, "\"];\n");
    }
  }
  fprintf(out, "}\n");
}

static void fsm_dump_c_code(const struct fsm *fsm, FILE *out)
{
  fprintf(out, "extern int rolex_accept(void);\n");
  fprintf(out, "extern int rolex_getc(void);\n");
  fprintf(out, "\n");
  fprintf(out, "void rolex_run(void)\n");
  fprintf(out, "{\n");
  fprintf(out, "  int c;\n");
  for(size_t state_index=0; state_index<fsm->states.item_count; ++state_index)
  {
    const struct fsm_state *state = &fsm->states.items[state_index];
    fprintf(out, "\n");
    fprintf(out, "state_%zu:\n", state_index);

    for(size_t accept_index=0; accept_index<state->accepts.item_count; ++accept_index)
    {
      const struct fsm_accept *accept = &state->accepts.items[accept_index];
      fprintf(out, "  extern void rolex_accept_%.*s(void);\n", (int)accept->length, accept->data);
      fprintf(out, "  rolex_accept_%.*s();\n", (int)accept->length, accept->data);
    }

    fprintf(out, "  switch((c = rolex_getc()))\n");
    fprintf(out, "  {\n");
    for(size_t transition_index=0; transition_index<state->transitions.item_count; ++transition_index)
    {
      const struct fsm_transition *transition = &state->transitions.items[transition_index];
      if(transition->value == FSM_EPSILON)
      {
        fprintf(stderr, "error: fsm contains epsilon transition\n");
        exit(EXIT_FAILURE);
      }

      fprintf(out, "  case %d:;\n", transition->value);
      fprintf(out, "    goto state_%zu;\n", transition->target);
    }
    fprintf(out, "  default:\n");
    fprintf(out, "    return;\n");
    fprintf(out, "  }\n");
  }
  fprintf(out, "}\n");
  fprintf(out, "\n");

}

static void usage(char *program_name)
{
  fprintf(stderr, "Usage: %s dot|c\n", program_name);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  if(argc != 2)
    usage(argv[0]);

  static struct fsm fsm = {0};
  if(strcmp(argv[1], "dot") == 0) {
    fsm_read(&fsm, stdin);
    fsm_dump_dot(&fsm, stdout);
  } else if(strcmp(argv[1], "c") == 0) {
    fsm_read(&fsm, stdin);
    fsm_dump_c_code(&fsm, stdout);
  } else {
    usage(argv[0]);
  }
}

