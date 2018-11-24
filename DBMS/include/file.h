#ifndef __FILE_H__
#define __FILE_H__

#include <stddef.h>
#include <inttypes.h>

#define BPTREE_INTERNAL_ORDER       4//249
#define BPTREE_LEAF_ORDER           4//32

#define FILENAME_MAX_LENGTH 256
#define FAIL -1

#define PAGE_SIZE                   4096

#define SIZE_KEY                    8
#define SIZE_VALUE                  120
#define SIZE_RECORD                 (SIZE_KEY + SIZE_VALUE)

#define BPTREE_MAX_NODE             (1024 * 1024) // for queue

typedef uint64_t pagenum_t;
#define PAGENUM_TO_FILEOFF(pgnum)   ((pgnum) * PAGE_SIZE)
#define FILEOFF_TO_PAGENUM(off)     ((pagenum_t)((off) / PAGE_SIZE))

#define TABLE_ORDER 10 // Table_id must be between 1 to 10.
/* Type representing the record
 * to which a given key refers.
 * In a real B+ tree system, the
 * record would hold data (in a database)
 * or a file (in an operating system)
 * or some other information.
 * Users can rewrite this part of the code
 * to change the type and content
 * of the value field.
 */

typedef struct _Record {
    uint64_t key;
    char value[SIZE_VALUE];
} Record;

typedef struct _InternalRecord {
    uint64_t key;
    off_t offset;
} InternalRecord;

typedef struct _Page {
    char bytes[PAGE_SIZE];
    
    // in-memory data
    pagenum_t pagenum;
} Page;

typedef struct _FreePage {
    off_t next;
    char reserved[PAGE_SIZE - 8];

    // in-memory data
    pagenum_t pagenum;
} FreePage;

typedef struct _HeaderPage {
    off_t freelist;
    off_t root_offset;
    uint64_t num_pages;
    char reserved[PAGE_SIZE - 24];

    // in-memory data
    pagenum_t pagenum;
} HeaderPage;

#define INTERNAL_KEY(n, i)    ((n)->irecords[(i)+1].key)
#define INTERNAL_OFFSET(n, i) ((n)->irecords[(i)].offset)
typedef struct _InternalPage {
    union {
        struct {
            off_t parent;
            int is_leaf;
            int num_keys;
            char reserved[112 - 16];
            InternalRecord irecords[BPTREE_INTERNAL_ORDER];
        };
        char space[PAGE_SIZE];
    };
    // in-memory data
    pagenum_t pagenum;
} InternalPage;

#define LEAF_KEY(n, i)      ((n)->records[(i)].key)
#define LEAF_VALUE(n, i)    ((n)->records[(i)].value)
typedef struct _LeafPage {
    union {
        struct {
            off_t parent;
            int is_leaf;
            int num_keys;
            char reserved[120 - 16];
            off_t sibling;
            Record records[BPTREE_LEAF_ORDER-1];
        };
        char space[PAGE_SIZE];
    };

    // in-memory data
    pagenum_t pagenum;
} LeafPage;

typedef struct _NodePage {
    union {
        struct {
            off_t parent;
            int is_leaf;
            int num_keys;
        };
        char space[PAGE_SIZE];
    };

    // in-memory data
    pagenum_t pagenum;
} NodePage;

typedef struct Table{
	int fd;
	char name[256];
	HeaderPage* headerpage;

}Table;

typedef struct TableList{
	Table table_list[TABLE_ORDER+1];
	int table_used;
}TableList;

typedef struct Buffer{
	char frame[4096];
	int table_id;
	uint64_t page_num;
	int is_dirty;
	int is_pinned;
	struct Buffer* prevB;
	struct Buffer* nextB;
}Buffer;

typedef struct Buffermgr{
	Buffer* firstBuf;
	int buf_order; //allocated buffer pool num
	int buf_used; //used buffer num.
}BufferMgr;

//extern TableList tablemgr;
//extern BufferMgr buffermgr;
// initialize db.
int init_db(int num_buf);
void close_table(int table_id);
// Open a db file. Create a file if not exist.
int open_or_create_db_file(const char* filename);

// Close a db table
int close_db_table(int table_id);

// shutdown db
int shutdown_db();

// expand a db file
void expand_file(int table_id,size_t cnt_page_to_expand);

// Get free page to use in table_id(dbfile)
pagenum_t file_alloc_page(int table_id);

// Put free page to the free list in table_id(dbfile)
void file_free_page(int table_id,pagenum_t pagenum);

/*
// Load file page into the in-memory page
//update buffer-pool to read page
//read in-memory page to buffer pool
//read in-memory page to buffer pool or page
void file_read_page_to_buffer(int table_id,pagenum_t pagenum); 
void file_read_buffer(int table_id,pagenum_t pagenum,Page* page);
void file_read_page(int table_id,pagenum_t pagenum, Page* page);

// write page into the buffer or page
void file_write_page(int table_id,Page* page);
int file_write_to_buffer(int table_id,Page* page);
int drop_victim(); //if there is no spac to evict buffer then return -1.success return 0.
*/
//extern HeaderPage dbheader;
void file_read_headerpage(int table_id,pagenum_t pagenum);
void buffer_read_to_page(int table_id,pagenum_t pagenum); 
void memory_read_to_buffer(int table_id,pagenum_t pagenum,Page* page);
void file_read_page(int table_id,pagenum_t pagenum, Page* page);

// write page into the buffer or page
void file_write_page(int table_id,Page* page);
int memory_write_to_buffer(int table_id,Page* page);
int overwrite_buffer(int table_id,Buffer* tmp_buf,Page* page);
int buffer_write_to_page();
#endif /* __FILE_H__  */
