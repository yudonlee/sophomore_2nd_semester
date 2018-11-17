#include "bpt.h"
#include "file.h"
#include "bpt_internal.h"
#include "panic.h"
//BufferMgr buffermgr; // buffermanager object 
//TableList tablemgr; // tablemanager object
// Opens a db file or creates a new file if not exist.
/*
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
        file_write_page((Page*)&dbheader);
    } else {
        // DB file exists. Loads header info
        file_read_page(0, (Page*)&dbheader);
        dbheader.pagenum = 0;
    }
    return 0;
}
*/
// Closes the db file.
int find_table_id(char* table_name){
	for(int i=1;i<=10;i++){
		if(strcmp(tablemgr.table_list[i].name,table_name))
			return 1; // success
	}
	return FAIL;

}
int init_db(int num_buf){
	buffermgr.buf_order = num_buf;
	buffermgr.buf_used = 0;
	//all fd is initialized -1.because when i open db, check for that free table.
	for(int i=1;i<=10;i++){
		tablemgr.table_list[i].fd = -1;
	}
	tablemgr.table_used =0; // there is no already open file .
	//firstbuffer prevBuffer is Last BufferIndex.
	/*new_buffer[0]->prevB = &new_buffer[num_buf-1];
	new_buffer[0]->nextB = &new_buffer[1];	
	// linked buffer pool
	for(int i=1;i<num_buf;i++){
		new_buffer[i]->prevB = &new_buffer[i-1];	
		new_buffer[i]->nextB = &new_buffer[i+1];
	}
	*/
	if(buffermgr.buf_used != num_buf){
		for(int i=1;i<=10;i++){
			if(tablemgr.table_list[i].fd!=-1)
				return -1;  //table allocate failed.so return nonzero value
		}
	}
	return 0; //success 
}

int open_or_create_db_file(const char* filename) {
    int dbfile = open(filename, O_RDWR);
	if (dbfile < 0) {
        // Creates a new db file
        dbfile = open(filename, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if (dbfile < 0) {
            PANIC("failed to create new db file");
        }
		/// create new table and record headerPage in table_id;
    	//Table* new_table = (Table*)malloc(sizeof(Table));
		/* find free table in table_list*/
		int table_index=-1;
		for(int i=1;i<=10;i++){
			if(tablemgr.table_list[i].fd <0){
				table_index =i;
				break; //for loop break
			}
		}
		if(table_index==-1){
			fprintf(stderr,"No seat for table_list.please close db more than 1\n");
			return -1;
		}
			//i will design int firstIndex in TableList struct.
		Table* new_table = &tablemgr.table_list[table_index];
		new_table->fd = dbfile;
		strcpy(new_table->name,filename);
		new_table->headerpage = (HeaderPage*)malloc(sizeof(PAGE_SIZE));
		memset(new_table->headerpage, 0, PAGE_SIZE); //&new_table->headerpage or new_table->headerpage
        new_table->headerpage->freelist = 0;
        new_table->headerpage->root_offset = 0;
        new_table->headerpage->num_pages = 1;
        new_table->headerpage->pagenum = 0;
		//new_table->table_id = table_index;   
        file_write_page(table_index,(Page*)new_table->headerpage); //&new_table->headerpage or new_table->headerpage
    	return table_index;
	} else {
        // DB file exists. Loads header info
		int table_index=-1;
		for(int i=1;i<=10;i++){
			if(tablemgr.table_list[i].fd <0)
				table_index =i;
		}
		if(table_index==-1)
        	fprintf(stderr,"No seat for table_list.please close db more than 1\n");
		Table* new_table = &tablemgr.table_list[table_index];
		new_table->fd = dbfile;
		strcpy(new_table->name,filename);
		file_read_page(table_index,0,(Page*)new_table->headerpage);
    	return table_index;
	}
}

int close_db_table(int table_id){
	Buffer* buf;
	buf = buffermgr.firstBuf->prevB;
	//if logically last buffer's table_id is eqaul to table_id ,then free( firstBuf->prev) and firstBuf->prev  = lastBUf->prev
	while(buf==buffermgr.firstBuf->prevB&& buf->table_id == table_id){
		buffermgr.firstBuf->is_pinned=1;
		buf->prevB->is_pinned = 1;
		buf->is_pinned =1;
		if(buf->is_dirty == 1){
			int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    		write(fd2, (Page*)buf, PAGE_SIZE);
		}
		buffermgr.firstBuf->prevB = buf->prevB;
		buffermgr.firstBuf->is_pinned =0;
		buf->prevB->is_pinned =0;
		free(buf);
		buffermgr.buf_used--;
		buf = buffermgr.firstBuf->prevB;
	}
	if(buf == NULL)
		return 0;

	/*if(buf->table_id == table_id){
		buf->is_pinned =1;
		buffermgr->firstBuf->is_pinned=1;
		buffermgr->firstBuf->prevB->is_pinned =1;
		buffermgr->firstBuf->prevB = buf->prevB;
		if(buf->is_dirty == 1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    		write(fd2, (Page*)lastbuf, PAGE_SIZE);
		}
		buffermgr->firstBuf->is_pinned=0;
		buffermgr->firstBuf->prevB->is_pinned=0;
		free(buf);
		buf->buffermgr->firstBuf->prevB;
	}*/
	//then this buffer is not logically end of buffer.so if buffer's table_id is equal to table_id,then free the buffer and linked bufer->prev and bufer->next. 
	while(buf != buffermgr.firstBuf){
		Buffer* prevBuf = buf->prevB;
		if(buf->table_id == table_id)
		{
			buf->is_pinned =1;
			buf->prevB->is_pinned=1;
			buf->nextB->is_pinned=1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
			buf->prevB->nextB = buf->nextB;
			buf->nextB->prevB = buf->prevB;
			buf->prevB->is_pinned=0;
			buf->nextB->is_pinned=0;	
			free(buf);
			buffermgr.buf_used--;
		}
		buf = prevBuf;
	}
	if(buf== buffermgr.firstBuf){
		if(buf->table_id == table_id && buf->nextB !=NULL){
			buf->is_pinned = 1;
			buf->prevB->is_pinned = 1;
			buf->nextB->is_pinned =1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
			buffermgr.firstBuf = buf->nextB;
			buf->nextB->prevB = buf->prevB;
			buffermgr.firstBuf->is_pinned = 0;
			buffermgr.firstBuf->prevB->is_pinned =0;
			free(buf);
			buffermgr.buf_used--;
		}
		else if(buf->table_id ==table_id && buf->nextB ==NULL){
			buf->is_pinned =1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
			free(buf);
			buffermgr.buf_used--;
		}
	}
	return 0;
}
int shutdown_db(){
	Buffer* buf = buffermgr.firstBuf->prevB;
	Buffer* prevBuf;
	while(buf!=buffermgr.firstBuf){
		buffermgr.firstBuf->is_pinned =1;
		buf->is_pinned=1;
		buf->prevB->is_pinned=1;
		if(buf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    		write(fd2, (Page*)buf, PAGE_SIZE);
		}
		prevBuf = buf->prevB;
		buffermgr.firstBuf->prevB = prevBuf;
		buffermgr.firstBuf->is_pinned=0;
		prevBuf->is_pinned=0;
		free(buf);
		buffermgr.buf_used--;
		buf = prevBuf;
	}
	if(buf==buffermgr.firstBuf){
		buf->is_pinned=1;
		if(buf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[buf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    		write(fd2, (Page*)buf, PAGE_SIZE);
		}
		free(buf);
		buffermgr.buf_used--;
	}
	else	
		return -1;// buf must be equal to buffermgr->firstBuf.
	if(buffermgr.buf_used !=0)
	{	fprintf(stderr,"shutdowndb but buf_used is not 0!\n");
		return -1;
	}
	for(int i=1;i<=10;i++){
		if(tablemgr.table_list[i].fd>0)
			close(tablemgr.table_list[i].fd);//close all already open file.
	}
	return 0; //success 
}