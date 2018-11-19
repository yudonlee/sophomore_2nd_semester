#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "file.h"
#include "panic.h"
#include "file.h"
#include "bpt_internal.h"

BufferMgr buffermgr;
TableList tablemgr;

// Expands file pages and prepends them to the free list

/*
void expand_file(size_t cnt_page_to_expand) {
    off_t offset = dbheader.num_pages * PAGE_SIZE;

    if (dbheader.num_pages > 1024 * 1024) {
        // Test code: do not expand over than 4GB
        PANIC("Test: you are already having a DB file over than 4GB");
    }
    
    int i;
    for (i = 0; i < cnt_page_to_expand; i++) {
        file_free_page(FILEOFF_TO_PAGENUM(offset));
        dbheader.num_pages++;
        offset += PAGE_SIZE;
    }

    file_write_page((Page*)&dbheader);
}

// Gets a free page to use.
// If there is no more free pages, expands the db file.
pagenum_t file_alloc_page() {
    off_t freepage_offset;
    
    freepage_offset = dbheader.freelist;
    if (freepage_offset == 0) {
        // No more free pages, expands the db file as twice
        expand_file(dbheader.num_pages);
        freepage_offset = dbheader.freelist;
    }
   
    FreePage freepage;
    file_read_page(FILEOFF_TO_PAGENUM(freepage_offset), (Page*)&freepage);
    dbheader.freelist = freepage.next;
    
    file_write_page((Page*)&dbheader);
    
    return FILEOFF_TO_PAGENUM(freepage_offset);
}

// Puts free page to the free list
void file_free_page(pagenum_t pagenum) {
    FreePage freepage;
    memset(&freepage, 0, PAGE_SIZE);

    freepage.next = dbheader.freelist;
    freepage.pagenum = pagenum;
    file_write_page((Page*)&freepage);
    
    dbheader.freelist = PAGENUM_TO_FILEOFF(pagenum);

    file_write_page((Page*)&dbheader);
}

void file_read_page(pagenum_t pagenum, Page* page) {
    lseek(dbfile, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
    read(dbfile, page, PAGE_SIZE);
    page->pagenum = pagenum;
}

void file_write_page(Page* page) {
    lseek(dbfile, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    write(dbfile, page, PAGE_SIZE);
}
*/
void expand_file(int table_id,size_t cnt_page_to_expand) {
    off_t offset = tablemgr.table_list[table_id].headerpage->num_pages * PAGE_SIZE;

    if (tablemgr.table_list[table_id].headerpage->num_pages > 1024 * 1024) {
        // Test code: do not expand over than 4GB
        PANIC("Test: you are already having a DB file over than 4GB");
    }
    
    int i;
    for (i = 0; i < cnt_page_to_expand; i++) {
        file_free_page(table_id,FILEOFF_TO_PAGENUM(offset));
        tablemgr.table_list[table_id].headerpage->num_pages++;
        offset += PAGE_SIZE;
    }

    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
}

// Gets a free page to use.
// If there is no more free pages, expands the db file.
pagenum_t file_alloc_page(int table_id) {
    off_t freepage_offset;
    
    freepage_offset = tablemgr.table_list[table_id].headerpage->freelist;
    if (freepage_offset == 0) {
        // No more free pages, expands the db file as twice
        expand_file(table_id,tablemgr.table_list[table_id].headerpage->num_pages);
        freepage_offset = tablemgr.table_list[table_id].headerpage->freelist;
    }
   
    FreePage freepage;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(freepage_offset), (Page*)&freepage);
    tablemgr.table_list[table_id].headerpage->freelist = freepage.next;
    
    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
    
    return FILEOFF_TO_PAGENUM(freepage_offset);
}

// Puts free page to the free list
void file_free_page(int table_id,pagenum_t pagenum) {
    FreePage freepage;
    memset(&freepage, 0, PAGE_SIZE);

    freepage.next = tablemgr.table_list[table_id].headerpage->freelist;
    freepage.pagenum = pagenum;
    file_write_page(table_id,(Page*)&freepage);
    
    tablemgr.table_list[table_id].headerpage->freelist = PAGENUM_TO_FILEOFF(pagenum);

    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
}
void file_read_page_to_buffer(int table_id,pagenum_t pagenum){
	Buffer* tmp_buf = (Buffer*)malloc(sizeof(Buffer));
	int fd2 = dup( tablemgr.table_list[table_id].fd ); 
	lseek(fd2, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
   	read(fd2,tmp_buf,PAGE_SIZE);
	tmp_buf->table_id = table_id;
	tmp_buf->page_num = pagenum;
	tmp_buf->is_dirty = 0;
	tmp_buf->is_pinned = 1; //pinned is 1 because this is update circumstance
	if(buffermgr.buf_used ==0){
		buffermgr.firstBuf = tmp_buf;
		tmp_buf->nextB =NULL;
		tmp_buf->prevB = tmp_buf;
		buffermgr.buf_used++;
		tmp_buf->is_pinned = 0;
	}
	else if(buffermgr.buf_used < buffermgr.buf_order){
		//firstBuf and last Buf is pinned because they will be modicated in prev/nextB
		buffermgr.firstBuf->is_pinned =1;
		//buffermgr.firstBuf->prevB->is_pinned =1; 
		tmp_buf->prevB = buffermgr.firstBuf->prevB;
		tmp_buf->nextB = buffermgr.firstBuf;
		buffermgr.firstBuf->prevB = tmp_buf;
		buffermgr.buf_used++;
		//tmp_buf->prevB->is_pinned=0;
		tmp_buf->nextB->is_pinned=0;
		tmp_buf->is_pinned=0;
	}
	else if(buffermgr.buf_used == buffermgr.buf_order){
		drop_victim();
		if(buffermgr.buf_used < buffermgr.buf_order){
			buffermgr.firstBuf->is_pinned =1;
			buffermgr.firstBuf->prevB->is_pinned =1; 
			tmp_buf->prevB = buffermgr.firstBuf->prevB;
			tmp_buf->nextB = buffermgr.firstBuf;
			buffermgr.firstBuf->prevB = tmp_buf;
			buffermgr.buf_used++;
			tmp_buf->prevB->is_pinned=0;
			tmp_buf->nextB->is_pinned=0;
			tmp_buf->is_pinned=0;
		}
		else
			fprintf(stderr,"drop_victim error\n");
	}

}
//read only buffer in memory page
void file_read_page(int table_id,pagenum_t pagenum,Page* page) {
	Buffer* buf = find_buf(table_id,pagenum);
	if(buf !=NULL){
		memcpy(buf,page,PAGE_SIZE);
	}
	//it cannot exit in buffer.so we take a 2 case.
	else{
		file_read_page_to_buffer(table_id,pagenum);
		buf = find_buf(table_id,pagenum);
		if(buf!=NULL)
			memcpy(buf,page,PAGE_SIZE);
		else
			fprintf(stderr,"buf is null after file_read_page_to_buffer\n");
		/*{//it does not exit in page.so we write page into  disk
			file_write_page(table_id,page);
		
		}*/
	}

}

void file_write_page(int table_id,Page* page){		
	Buffer* buf = find_buf(table_id,page->pagenum);
	int status;
	//there is no already buffer,then just update buffer.
	if(buf ==NULL){
		status = file_write_to_buffer(table_id,page);
	}
	//if there is already buffer which point same page,then free that buffer and create new buffer
	else{
		buf->is_pinned=1;
		buf->prevB->is_pinned=1;
		buf->nextB->is_pinned=1;
		buf->prevB->nextB = buf->nextB;
		buf->nextB->prevB = buf->prevB;
		buf->prevB->is_pinned=0;
		buf->nextB->is_pinned=0;
		buf->is_pinned=0;
		buffermgr.buf_used--;
		free(buf);
		status = file_write_to_buffer(table_id,page);
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
int file_write_to_buffer(int table_id,Page* page) {
    //build buffer pool to linked list;
	//if buffer_used is 0,then create logically first buffer.
	if(buffermgr.buf_used==0){
		fprintf(stderr,"bufferm malloc error\n");
		Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
		memcpy(buf,page,PAGE_SIZE); //buf page change
		buf->page_num = page->pagenum;
		buf->table_id = table_id;
		buf->is_dirty =1;
		buf->is_pinned =0;
		buf->prevB = buf;
		buf->nextB =NULL;
		buffermgr.firstBuf=buf;
		buffermgr.buf_used++;
	}
	//if buffer_used is smaller than buf_order,we can make new buffer logically first buffer
	//because of LRU policy latest used buffer must be the first buffer.
	if(buffermgr.buf_used < buffermgr.buf_order){
	//the logical first Buf and last Buf need to be modified.so I want to pin up both of them.
	buffermgr.firstBuf->prevB->is_pinned=1;
	buffermgr.firstBuf->is_pinned=1;
	Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
	memcpy(buf,page,PAGE_SIZE);
	buf->page_num = page->pagenum;
	buf->table_id = table_id;
	buf->is_dirty = 1;
	buf->is_pinned =1;
	buf->prevB = buffermgr.firstBuf->prevB;
	buffermgr.firstBuf->prevB = buf;
	buf->nextB = buffermgr.firstBuf;
	buffermgr.firstBuf->is_pinned=0;
	buf->prevB->is_pinned=0;
	//buffermgr firstBuf is now buf;
	buffermgr.firstBuf = buf;
	buf->is_pinned=0;
	buffermgr.buf_used++;
	}
	else if(buffermgr.buf_used == buffermgr.buf_order){
		int status = drop_victim(); //al
		if(status ==0)
			return file_write_to_buffer(table_id,page);	
		else{
			fprintf(stderr,"there is no space to evict buffer\n");
			return status;
		}
	}
	return 0;

}
int drop_victim(){	
		Buffer* lastBuf;
		//duplicate filedescriptor using table_id ,struct table,struct tablelst;
		//int fd2 = dup( tablemgr.table_list[table_id].fd ) 
		lastBuf = buffermgr.firstBuf->prevB;	
		while(lastBuf->is_pinned>0){
			lastBuf = lastBuf->prevB;
		//	lastBuf->nextB->prevB = lastBuf->prevB;// buffer frame flushed,(it does not logically end, lastBuf->prevB is equal to lastBuf->nextB->prevB 
		}
		// if lastBuf does not logically end,prevB of lastBuf->nextB is lastBuf->prevB
		// so i made lastBuf logically final order
		if(lastBuf->is_pinned !=0)
			return -1; //error.there is no space to evict buffer.
		
		//in this case, last buffer(evicted buffer) is not LRU last buffer.
		if(lastBuf->nextB != NULL){
			lastBuf->nextB->is_pinned=1;
			lastBuf->prevB->is_pinned=1;
			lastBuf->prevB->nextB = lastBuf->nextB;
			lastBuf->nextB->prevB = lastBuf->prevB;
			lastBuf->nextB->is_pinned=0;
			lastBuf->prevB->is_pinned=0;
			/*buffermgr->firstBuf->prevB = lastBuf->prevB;
			buffermgr->firstBuf->prevB->nextB = NULL;*/
			buffermgr.buf_used--;
			/*buffermgr->firstbuf->prevB = lastBuf;
			buffermgr->firstbuf->prevB= lastBuf;
			lastBuf->nextB = NULL;*/
		}
		//in this case,last buffer is LRU last buffer.(firstBuf->prevB)
		else{
			lastBuf->prevB->is_pinned++;
			buffermgr.firstBuf->is_pinned++;
			buffermgr.firstBuf->prevB = lastBuf->prevB;
			lastBuf->prevB->nextB=NULL;
			lastBuf->prevB->is_pinned=0;
			buffermgr.firstBuf->is_pinned=0;	
			buffermgr.buf_used--;
		}	
		//if(lastBuf->is_pinned == 0){
		if(lastBuf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(lastBuf->page_num), SEEK_SET);
    		write(fd2, (Page*)lastBuf, PAGE_SIZE);
		}
		//buffermgr->firstBuf->prevB = lastBuf->prevB;
		//buffermgr->firstBuf->prevB->nextB = NULL;
		//buffermgr->buf_used--;
		//}
		free(lastBuf);
		return 0;
}
/*
int close_table(int table_id){
	Buffer* buf;
	buf = buffermgr->firstBuf->prevB;
	//if logically last buffer's table_id is eqaul to table_id ,then free( firstBuf->prev) and firstBuf->prev  = lastBUf->prev
	while(buf==buffermgr->firstBuf->prevB&& buf->table_id == table_id){
		buffermgr->firstBuf->is_pinned=1;
		buf->prevB->is_pinned = 1;
		buf->is_pinned =1;
		if(buf->is_dirty == 1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    		write(fd2, (Page*)lastbuf, PAGE_SIZE);
		}
		buffermgr->firstBuf->prevB = buf->prevB;
		buffermgr->firstBuf->is_pinned =0;
		buf->prevB->is_pinned =0;
		free(buf);
		buffermgr->buf_used--;
		buf = buffermgr->firstBuf->prevB;
	}
	if(buf == NULL)
		return 0;

	/if(buf->table_id == table_id){
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
	} /
	//then this buffer is not logically end of buffer.so if buffer's table_id is equal to table_id,then free the buffer and linked bufer->prev and bufer->next. 
	while(buf != buffermgr->firstBuf){
		Buffer* prevBuf = buf->prevB;
		if(buf->table_id == table_id)
		{
			buf->is_pinned =1;
			buf->prevB->is_pinned=1;
			buf->nextB->is_pinned=1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    			write(fd2, (Page*)lastbuf, PAGE_SIZE);
			}
			buf->prevB->nextB = buf->next;
			buf->nextB->prevB = buf->prevB;
			buf->prevB->is_pinned=0;
			buf->nextB->is_pinned=0;	
			free(buf);
			buffermgr->buf_used--;
		}
		buf = prevBuf;
	}
	if(buf== buffermgr->firstBuf){
		if(buf->table_id == table_id && buf->nextB !=NULL){
			buf->is_pinned = 1;
			buf->prevB->is_pinned = 1;
			buf->nextB->is_pinned =1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    			write(fd2, (Page*)lastbuf, PAGE_SIZE);
			}
			buffermgr->firstBuf = buf->nextB;
			buffer->nextB->prevB = buf->prevB;
			buffermgr->firstBuf->is_pinned = 0;
			buffermgr->firstBuf->prevB->is_pinned =0;
			free(buf);
			buffermgr->buf_used--;
		}
		else if(buf->table_id ==table_id && buf->nextB ==NULL){
			buf->is_pinned =1;
			if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    			write(fd2, (Page*)lastbuf, PAGE_SIZE);
			}
			free(buf);
			buffermgr->buf_used--;
		}
	}
	return 0;
}
int shutdown_db(){
	Buffer* buf = buffermgr->firstBuf->prevB
	Buffer* prevBuf;
	while(buf!=buffermgr->firstBuf){
		buffermgr->firstBuf->is_pinned =1;
		buf->is_pinned=1;
		buf->prevB=1;
		if(buf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    		write(fd2, (Page*)lastbuf, PAGE_SIZE);
		}
		prevBuf = buf->prev;
		buffermgr->firstBuf->prevB = prevBuf;
		buffermgr->firstBuf->is_pinned=0;
		prevBuf->is_pinned=0;
		free(buf);
		buffermgr->buf_used--;
		buf = prevBuf;
	}
	if(buf==buffermgr->firstBuf){
		buf->is_pinned=1;
		if(buf->is_dirty==1){
			int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    		write(fd2, (Page*)lastbuf, PAGE_SIZE);
		}
		free(buf);
		buffermgr->buf_used--;
	}
	else	
		return -1// buf must be equal to buffermgr->firstBuf.
	if(buffermgr->buf_used !=0)
	{	fprintf(stderr,"shutdowndb but buf_used is not 0!\n");
		return -1;
	}
	for(i=1;i<=10;i++){
		if(tablemgr->table_list[i].fd>0)
			close(tablemgr->table_list[i].fd);//close all already open file.
	}
	return 0; //success 
}
*/

