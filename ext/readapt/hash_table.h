#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include "ruby.h"

typedef struct ht_long_array
{
    long *items;
    long size;
} ht_long_array;

// ht_item is an item in the hash table
typedef struct ht_item
{
    ID key;
    ht_long_array *value;
} ht_item;

typedef struct ht_hash_table
{
    // int size_index;
    // int size;
    // int count;
    long size;
    ht_item **items;
} ht_hash_table;

ht_hash_table *ht_new();
void ht_del_hash_table(ht_hash_table *ht);

void ht_insert(ht_hash_table *ht, ID key, const long *value, const long size);
ht_long_array *ht_search(ht_hash_table *ht, ID key);
void ht_delete(ht_hash_table *h, ID key);

#endif // HASH_TABLE_H_
