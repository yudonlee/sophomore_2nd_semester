#ifndef __BUFFER_H__
#define TABLE_ORDER 10
extern BufferMgr buffermgr;
extern Buffer* new_buffer;
extern TableList tablemgr;
extern num_Tables;
//extern table_id array to store table_id.
typedef struct _table{
	int fd;
	char name[256];
	HeaderPage* headerpage;

}Table;
typedef struct tablelist{
	Table table_list[TABLE_ORDER+1];
	int num_tables=0;
}TableList;
typedef struct union buffermgr{
	Buffer* firstBuf;
	int buf_order; //allocated buffer pool num
	int buf_used; //used buffer num.
}BufferMgr;
typedef struct buffer{
	char frame[4096];
	uint64_t page_num;
	int table_id;
	int is_dirty;
	int is_pinned;
	struct* buffer prevB;
	struct* buffer nextB;
}Buffer;
int init_db(int num_buf);
Buffer find_buf(int table_id);
int close_table(int table_id);
int shutdown_db();
#endif // __BUFFER_H__
