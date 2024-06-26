#include "list.h"

/**
 * Modified vec_list.c to fit the new list_t interface.
 */

#include "assert.h"
#include "stdio.h"
#include "stdlib.h"

typedef struct list {
  void **data;
  size_t length;
  size_t size;
  free_func_t freer;
} list_t;

const size_t GROWTH_FACTOR = 2;

list_t *list_init(size_t initial_size, free_func_t freer) {
  list_t *list = malloc(sizeof(list_t));
  assert(list);
  list->data = malloc(initial_size * sizeof(void *));
  assert(list->data);
  list->length = 0;
  list->size = initial_size;
  list->freer = freer;

  return list;
}

void list_free(list_t *list) {
  if (list->freer) {
    for (size_t i = 0; i < list->length; ++i) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

void list_free_with_function(list_t *list, free_func_t free_func) {
    for (size_t i = 0; i < list_size(list); i++) {
        free_func(list_get(list, i));
    }
    list_free(list);
}

size_t list_size(list_t *list) { return list->length; }

void *list_get(list_t *list, size_t index) {
  assert(index < list->length);
  return list->data[index];
}

void list_add(list_t *list, void *value) {
  assert(value != NULL);
  if (list->length == list->size) {
    // Resize the list
    list->size *= GROWTH_FACTOR;
    list->data = realloc(list->data, list->size * sizeof(void *));
    assert(list->data);
  }
  list->data[list->length] = value;
  ++list->length;
}

void *list_remove(list_t *list, size_t index) {
  assert(index < list->length);
  void *value = list->data[index];
  for (size_t i = index; i < list->length - 1; ++i) {
    list->data[i] = list->data[i + 1];
  }
  --list->length;
  return value;
}
