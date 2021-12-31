#include "bpt_internal.h"
#include "panic.h"
//initialize new_buffer with start;
int init_db(int num_buf){
	Buffer* new_buffer = (Buffer*)malloc(sizeof(BUffer)*num_buf);
	firstBuf = &new_buffer[0]; //firstBuf store the first buffer address
	return 0;
}
// Opens a db file or creates a new file if not exist.
int open_or_create_db_file(const char* filename) {
    dbfile = open(filename, O_RDWR);
  	 
	if (dbfile < 0) {
        // Creates a new db file
        dbfile = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if (dbfile < 0) {
            PANIC("failed to create new db file");
        }
        
        memset(&dbheader, 0, PAGE_SIZE);
        dbheader.freelist = 0;
        dbheader.root_offset = 0;
        dbheader.num_pages = 1;
        dbheader.pagenum = 0;
		dbheader.table_id = tablenum++;
        file_write_page((Page*)&dbheader);
    } else {
        // DB file exists. Loads header info
        file_read_page(0, (Page*)&dbheader);
        dbheader.pagenum = 0;
    }
    return dbheader.table_id; //return table_id on defined dbheader;
}

// Closes the db file.
void close_db_file() {
    close(dbfile);
}
