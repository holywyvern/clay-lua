#include "clay.h"
#include <stddef.h>
#include <string.h>

struct CacheNode {
 struct CacheNode *left;
 struct CacheNode *right;
 char *string;
 size_t len;
};

struct CacheNode root = {NULL, NULL, "", 0 };

static int
compare_strings(const char *s1, size_t len1, const char *s2, size_t len2)
{
  if (len1 < len2) return -1;
  if (len1 > len2) return 1;
  return strncmp(s1, s2, len1);
}

static const char *
find_in_node(const char *text, size_t size, struct CacheNode *node)
{
  char *str;
  int cmp = compare_strings(text, size, node->string, node->len);
  if (cmp == 0) return node->string;
  if (cmp < 0)
  {
    if (node->left) return find_in_node(text, size, node->left);
    node->left = malloc(sizeof(struct CacheNode));
    str = malloc(size + 1);
    strncpy(str, text, size);
    str[size] = '\0';
    node->left->left = node->left->right = NULL;
    node->left->string = str;
    node->left->len = size;
    return str;
  }
  if (node->right) return find_in_node(text, size, node->right);
  node->right = malloc(sizeof(struct CacheNode));
  str = malloc(size + 1);
  strncpy(str, text, size);
  str[size] = '\0';
  node->right->left = node->right->right = NULL;
  node->right->string = str;
  node->right->len = size;
  return str;
}

void
clay_lua_initStringCache(void)
{
}

const char *
clay_lua_storeString(const char *text, size_t len)
{
  return find_in_node(text, len, &root);
}
