#ifndef __BPT_H__
#define __BPT_H__
#include "bpt_internal.h"

int open_table(const char* filename);
int insert(int table_id,uint64_t key, const char* value);
int delete(int table_id,uint64_t key);
int find_table_id(char* table_name);

void print_tree(int table_id);
void find_and_print(int table_id,uint64_t key); 

void license_notice( void );
void usage_1( void );
void usage_2( void );

#endif // __BPT_H__
