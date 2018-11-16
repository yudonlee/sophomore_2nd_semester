#include <stddef.h>
#include <inttypes.h>

#define BPTREE_INTERNAL_ORDER       4//249
#define BPTREE_LEAF_ORDER           4//32

#define PAGE_SIZE                   4096

#define SIZE_KEY                    8
#define SIZE_VALUE                  120
#define SIZE_RECORD                 (SIZE_KEY + SIZE_VALUE)

#define BPTREE_MAX_NODE             (1024 * 1024) // for queue

typedef uint64_t pagenum_t;
#define PAGENUM_TO_FILEOFF(pgnum)   ((pgnum) * PAGE_SIZE)
#define FILEOFF_TO_PAGENUM(off)     ((pagenum_t)((off) / PAGE_SIZE))

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

// Open a db file. Create a file if not exist.
int open_db(const char* filename);

// Close a db file
void close_db();

// Get free page to use
pagenum_t file_alloc_page();

// Put free page to the free list
void file_free_page(pagenum_t pagenum);

// Load file page into the in-memory page
void file_read_page(pagenum_t pagenum, Page* page);

// Flush page into the file
void file_write_page(Page* page);

extern HeaderPage dbheader;

