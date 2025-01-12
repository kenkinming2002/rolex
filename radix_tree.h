#ifndef RADIX_TREE_H
#define RADIX_TREE_H

#include "dynamic_array.h"

#include <stdbool.h>
#include <stddef.h>

/// Implementation of a radix tree.
///
/// Note: This is not actually a radix tree. I just *feel* like calling it a
///       radix tree because I could not come up with a better name. This is
///       actually a just map from fixed-size bit array to a non-zero size_t.
///
/// There are two requirements for using the radix tree:
///   - depth is non-zero
///   - value is non-zero

struct rt_node
{
  ssize_t childs[2];
};

da_define(rt, struct rt_node);

static struct rt rt_create(void)
{
  struct rt rt = {};
  da_append(rt, ((struct rt_node){ .childs = { -1, -1 }}));
  return rt;
}

static bool rt_insert(struct rt *rt, size_t depth, bool key[depth], size_t *value)
{
  size_t index = 0;
  size_t i = 0;

  for(; i<depth; ++i)
  {
    ssize_t child = rt->items[index].childs[key[i]];
    if(child == -1)
      break;

    index = child;
  }

  if(i == depth)
  {
    *value = index;
    return false;
  }

  for(; i<depth-1; ++i)
  {
    index = rt->items[index].childs[key[i]] = rt->item_count;
    da_append(*rt, ((struct rt_node){ .childs = { -1, -1 }}));
  }

  rt->items[index].childs[key[i]] = *value;
  return true;
}

#endif // RADIX_TREE_H
