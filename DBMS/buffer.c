#include "buffer.h"
#include "bpt_internal.h"
#include "panic.h"
//initialize new_buffer with start;
int close_table(int table_id){
	Buffer* buf;
	buf = buffermgr->firstBuf->prevB;
	while(buf!=firstBuf){
		if(buf->table_id == table_id){
			//buffer drop.if is_dirty ==1 firle write page;
			if(buf->is_dirty==1)
				file_write_
		}
	}
}
int shutdown_db(){

}
int init_db(int num_buf){
	buffermgr.buf_order = num_buf;
	buffermgr.buf_used = 0;
	//all fd is initialized -1.because when i open db, check for that free table.
	for(int i=1;i<=10;i++){
		tablemgr.table_list[i].fd = -1;
	}
	//firstbuffer prevBuffer is Last BufferIndex.
	/*new_buffer[0]->prevB = &new_buffer[num_buf-1];
	new_buffer[0]->nextB = &new_buffer[1];	
	// linked buffer pool
	for(int i=1;i<num_buf;i++){
		new_buffer[i]->prevB = &new_buffer[i-1];	
		new_buffer[i]->nextB = &new_buffer[i+1];
	}
	*/
	return buffermgr.buf_order;
}

Buffer find_buf(int table_id,pagenum page_num){
	Buffer* output;
	output = buffermgr->firstBuf;
	while(output != NULL){
		if(output->table_id == table_id && output->page_num){
			return output;
		}
		output = output->nextB;
	}
	if(output ==NULL){
		int fd2 = dup( tablemgr.table_list[table_id].fd ) 
		lseek(fd2, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
    	read(fd2,page,PAGE_SIZE);
		update_buffer(page,table_id);
	}
	return NULL;
}

// Opens a db file or creates a new file if not exist.
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
			if(tablemgr.table_list[i].fd <0)
				table_index =i;
		}
		if(table_index==-1)
			fprintf(stderr,"No seat for table_list.please close db more than 1\n");
		Table* new_table = &tablemgr.table_list[table_index];
		new_table->fd = dbfile;
		strcpy(new_table->name,filename);
		new_table->headerpage = (HeaderPage*)malloc(sizeof(PAGE_SIZE));
		memset(new_table->headerpage, 0, PAGE_SIZE); //&new_table->headerpage or new_table->headerpage
        new_table->headerpage->freelist = 0;
        new_table->headerpage->root_offset = 0;
        new_table->headerpage->num_pages = 1;
        new_table->headerpage->pagenum = 0;
		new_table->headerpage->table_id = table_index;   
        file_write_page((Page*)new_table->headerpage,table_index); //&new_table->headerpage or new_table->headerpage
    	return new_table->headerpage->table_id;
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
		file_read_page(0, (Page*)new_table->headerpage,table_index);
    	return new_table->headerpage->table_id;
	}
}

// Closes the db file.
void close_db_file(int table_id) {
    close(tablemgr.table_list[table_id].fd);
}


// Expands file pages and prepends them to the free list
void expand_file(size_t cnt_page_to_expand,int table_id) {
    off_t offset = dbheader.num_pages * PAGE_SIZE;

    if (tablemgr.table_list->headerpage->num_pages > 1024 * 1024) {
        // Test code: do not expand over than 4GB
        PANIC("Test: you are already having a DB file over than 4GB");
    }
    
    int i;
    for (i = 0; i < cnt_page_to_expand; i++) {
        file_free_page(FILEOFF_TO_PAGENUM(offset),table_id);
        tablemgr.table_list[table_id]->headerpage->num_pages++;
        offset += PAGE_SIZE;
    }

    file_write_page((Page*)tablemgr.table_list[table_id]->headerpage,table_id);
}

// Gets a free page to use.
// If there is no more free pages, expands the db file.
pagenum_t file_alloc_page(int table_id) {
    off_t freepage_offset;
    
    freepage_offset = tablemgr.table_list[table_id]->headerpage->freelist;
    if (freepage_offset == 0) {
        // No more free pages, expands the db file as twice
        expand_file(tablemgr.table_list[table_id]->num_pages);
        freepage_offset = tablemgr.table_list[table_id]->headerpage->freelist;
    }
   
    FreePage freepage;
    file_read_page(FILEOFF_TO_PAGENUM(freepage_offset), (Page*)&freepage);
    tablemgr.table_list[table_id]->headerpage->freelist = freepage.next;
    
    file_write_page((Page*)tablemgr.table_list[table_id]->headerpage,table_id);
    
    return FILEOFF_TO_PAGENUM(freepage_offset);
}

// Puts free page to the free list
void file_free_page(int table_id,pagenum_t pagenum) {
    FreePage freepage;
    memset(&freepage, 0, PAGE_SIZE);

    freepage.next = tablemgr.table_list[table_id]->headerpage->freelist;
    freepage.pagenum = pagenum;
    file_write_page((Page*)&freepage,table_id);
    
    tablemgr.table_list[table_id]->headerpage->freelist = PAGENUM_TO_FILEOFF(pagenum);

    file_write_page((Page*)tablemgr.table_list[table_id]->headerpage);
}
void file_read_page_to_buffer(int table_id,pagenum_t pagenum){
	Buffer* tmp_buf = (Buffer*)malloc(sizeof(Buffer));
	int fd2 = dup( tablemgr.table_list[table_id].fd ) 
	lseek(fd2, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
   	read(fd2,tmp_buf,PAGE_SIZE);
	tmp_buf->page_num = pagenum;
	tmp_buf->table_id = table_id;
	tmp_buf->is_dirty = 0;
	tmp_buf->is_pinned = 0;
	if(buffermgr->buf_used ==0){
		buffermgr->firstBuf = tmp_buf;
		tmp_buf->nextB =NULL;
		tmp_buf->prevB = tmp_buf;
		buffermgr->buf_used++;
	}else if(buffermgr->buf_used<buffermgr->buf_order){
		tmp_buf->prevB = buffermgr->firstBuf->prevB;
		tmp_buf->nextB = buffermgr->firstBuf;
		buffermgr->firstBuf->prevB = tmp_buf;
		buffermgr->buf_used++;
	
	}else if(buffermgr->buf_used = buffermgr->buf_order){
		drop_victim();
		if(buffermgr->buf_used<buffermgr->buf_order){
			tmp_buf->prevB = buffermgr->firstBuf->prevB;
			tmp_buf->nextB = buffermgr->firstBuf;
			buffermgr->firstBuf->prevB = tmp_buf;
			buffermgr->buf_used++;
		}
		else
			fprintf(stderr,"drop_victim error\n");
	}

}
//file_read_buffer
void file_read_buffer(int table_id,pagenum_t pagenum, Page* page) {
	Buffer* buf = find_buf(table_id,pagenum);
	if(buf !=NULL){
		memcpy(buf,page,PAGE_SIZE);
	}
	else{
		file_read_page_to_buffer(pagenum,table_id);
		buf = find_buf(table_id,pagenum);
		if(buf!=NULL)
			memcpy(buf,page,PAGE_SIZE);
		else
			fprintf(stderr,"buf is null after fille_read_page_to_buffer\n");
	}

}

void file_write_page(int table_id,Page* page){		
	Buffer* buf = find_buf(table_id,page->pagenum);
	int status;
	//there is no already buffer,then just update buffer.
	if(buf ==NULL){
		status = update_buffer(table_id,page);
	}
	//if there is already buffer which point same page,then free that buffer and create new buffer
	else{
		buf->prevB->nextB = buf->nextB;
		buf->nextB->prevB = buf->prevB;
		free(buf);
		status = update_buffer(page,table_id);
	}
	
	if(status ==0){
		//update_buffer_success
		//file_write_page 
	}
	/*else if(status ==1){
		//buffer pool is full.so it does have to flush.
	}*/
	else{
		fprintf(stderr,"file_write_page executing failed!\n");
	}
}
//return if buffer have to be  flushed,return 1;
int update_buffer(Page* page,int table_id) {
    //build buffer pool to linked list;
	//if buffer_used is 0,then create logically first buffer.
	if(buffermgr->buf_used==0){
		Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
		memcpy(page,buf,PAGE_SIZE);
		buf->page_num = page->pagenum;
		buf->table_id = table_id;
		buf->is_dirty =1;
		buf->is_pinned =0;
		buf->prevB = buf;
		buf->nextB =NULL;
		buffermgr->firstBuf=buf;
		buffermgr->buf_used++;
	}
	//if buffer_used is smaller than buf_order,we can make new buffer logically first buffer
	//because of LRU policy latest used buffer must be the first buffer.
	if(buffermgr->buf_used < buffermgr->buf_order){
	//the logical first Buf and last Buf need to be modified.so I want to pin up both of them.
	buffermgr->firstBuf->prevB->is_pinned++;
	buffermgr->firstBuf->is_pinned++;
	Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
	memcpy(page,buf,PAGE_SIZE);
	buf->page_num = page->pagenum;
	buf->table_id = table_id;
	buf->is_dirty = 1;
	buf->is_pinned =0;
	buf->prevB = buffermgr->firstBuf->prevB;
	buffermgr->firstBuf->prevB = buf;
	buf->nextB = buffermgr->firstBuf;
	buffermgr->firstBuf->is_pinned--;
	buffermgr->firstBuf->prevB->is_pinned--;
	//buffermgr firstBuf is now buf;
	buffermgr->firstBuf = buf;
	buffermgr->buf_used++;
	}
	else if(buffermgr->buf_used == buffermgr->buf_order){
		drop_victim(); //al
		return update_buffer(page,table_id);	
	}
	return 0;

}
void drop_victim(){	
		Buffer* buf;
		Buffer* lastBuf;
		//duplicate filedescriptor using table_id ,struct table,struct tablelst;
		//int fd2 = dup( tablemgr.table_list[table_id].fd ) 
		lastBuf = buffermgr->firstBuf->prevB;	
		while(lastBuf->is_pinned>0){
			lastBuf = lastBuf->prevB;
		//	lastBuf->nextB->prevB = lastBuf->prevB;// buffer frame flushed,(it does not logically end, lastBuf->prevB is equal to lastBuf->nextB->prevB 
		}
		// if lastBuf does not logically end,prevB of lastBuf->nextB is lastBuf->prevB
		// so i made lastBuf logically final order
		if(lastBuf->nextB != NULL){
			lastBuf->nextB->is_pinned++;
			lastBuf->prevB->is_pinned++;
			lastBuf->prevB->nextB = lastBuf->nextB;
			lastBuf->nextB->prevB = lastBuf->prevB;
			lastBuf->nextB->is_pinned--;
			lastBuf->prevB->is_pinned--;
			/*buffermgr->firstBuf->prevB = lastBuf->prevB;
			buffermgr->firstBuf->prevB->nextB = NULL;*/
			buffermgr->buf_used--;
			/*buffermgr->firstbuf->prevB = lastBuf;
			buffermgr->firstbuf->prevB= lastBuf;
			lastBuf->nextB = NULL;*/
		}else{
			lastBuf->prevB->is_pinned++;
			buffermgr->firstbuf->prevB = lastBuf->prevB;
			lastBuf->prevb->nextB=NULL;
			lastBuf->prevB->is_pinned--;
			buffermgr->buf_used--;
		}	
		//if(lastBuf->is_pinned == 0){
		if(lastBuf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    		write(fd2, (Page*)lastbuf, PAGE_SIZE);
		}
		//buffermgr->firstBuf->prevB = lastBuf->prevB;
		//buffermgr->firstBuf->prevB->nextB = NULL;
		//buffermgr->buf_used--;
		//}
}
