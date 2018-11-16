#ifndef __BPT_H__
#define __BPT_H__

int open_db(const char* filename);
char* find(uint64_t key);
int insert(uint64_t key, const char* value);
int delete(uint64_t key);

void print_tree();
void find_and_print(uint64_t key); 

void license_notice( void );
void usage_1( void );
void usage_2( void );
#endif // __BPT_H__
