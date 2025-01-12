#ifndef DYANMIC_ARRAY_H
#define DYANMIC_ARRAY_H

#include <stdlib.h>

#define da_define(name, type) \
  struct name \
  { \
    type *items; \
    size_t item_count; \
    size_t item_capacity; \
  }

#define da_nth_back(array, n) ((array).items[(array).item_count-(n)-1])
#define da_back(array) da_nth_back(array, 0)

#define da_alloc_exact(array) \
  do \
  { \
    (array).item_capacity = (array).item_count; \
    (array).items         = malloc((array).item_capacity * sizeof *(array).items); \
  } \
  while(0)

#define da_destroy(array) \
  do \
  { \
    free((array).items); \
    (array).item_count = 0; \
    (array).item_capacity = 0; \
  } \
  while(0)

#define da_reset(array) \
  do \
  { \
    (array).item_count = 0; \
  } \
  while(0)

#define da_append(array, value) \
  do \
  { \
    if((array).item_capacity == (array).item_count) \
    { \
      (array).item_capacity = (array).item_capacity != 0 ? (array).item_capacity * 2 : 1; \
      (array).items         = realloc((array).items, (array).item_capacity * sizeof *(array).items); \
    } \
    (array).items[(array).item_count++] = value; \
  } \
  while(0)

#define da_append_many(array, new_items, new_item_count) \
  do \
  { \
    while((array).item_capacity < (array).item_count + (new_item_count)) \
      (array).item_capacity = (array).item_capacity != 0 ? (array).item_capacity * 2 : 1; \
    (array).items = realloc((array).items, (array).item_capacity * sizeof *(array).items); \
    for(size_t i=0; i<(new_item_count); ++i) \
      (array).items[(array).item_count++] = (new_items)[i]; \
  } \
  while(0)

#endif // DYANMIC_ARRAY_H
