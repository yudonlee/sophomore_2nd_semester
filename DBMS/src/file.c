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


void expand_file(int table_id,size_t cnt_page_to_expand) {
    //FIXME: this raise headerpage block too many.
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
    int fd2= tablemgr.table_list[table_id].fd;
	off_t freepage_offset;
    freepage_offset = tablemgr.table_list[table_id].headerpage->freelist;
    if (freepage_offset == 0) {
        // No more free pages, expands the db file as twice
        expand_file(table_id,tablemgr.table_list[table_id].headerpage->num_pages);
        freepage_offset = tablemgr.table_list[table_id].headerpage->freelist;
    }
   
    FreePage freepage;
    //FIXME:readpage fix.
    lseek(fd2,freepage_offset,SEEK_SET);
	read(fd2,(Page*)&freepage,PAGE_SIZE);
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
	//write_file_free_page_memory_direct_to_disk
	int fd2 = tablemgr.table_list[table_id].fd;
	lseek(fd2, PAGENUM_TO_FILEOFF(freepage.pagenum), SEEK_SET);
    write(fd2,(Page*)&freepage, PAGE_SIZE);
    
    tablemgr.table_list[table_id].headerpage->freelist = PAGENUM_TO_FILEOFF(pagenum);
    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
}
void buffer_read_to_page(int table_id,pagenum_t pagenum){
	//TODO: buffer_read_page must check! 
	//Buffer* find = find_buf(table_id,pagenum);
	//if(find !=NULL) return;
	Buffer* tmp_buf = (Buffer*)malloc(sizeof(Buffer));
	int fd2 = tablemgr.table_list[table_id].fd; 
	printf("fd2 is :%d\n",fd2);
	lseek(fd2, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
   	read(fd2,(Page*)tmp_buf,PAGE_SIZE);
	
	tmp_buf->table_id = table_id;
	tmp_buf->page_num = pagenum;
	tmp_buf->is_dirty = 0;
	tmp_buf->is_pinned = 1; //pinned is 1 because this is update circumstance
	if(buffermgr.buf_used ==0){
		tmp_buf->nextB =NULL;
		tmp_buf->prevB = tmp_buf;
		buffermgr.buf_used++;
		tmp_buf->is_pinned = 0;
		buffermgr.firstBuf = tmp_buf;
	}
	else if(buffermgr.buf_used < buffermgr.buf_order){
		//firstBuf and last Buf is pinned because they will be modicated in prev/nextB
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
	else if(buffermgr.buf_used == buffermgr.buf_order){
		buffer_write_to_page(); //flush.
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
		memcpy(page,buf,PAGE_SIZE);
	}
	//it cannot exit in buffer.so we take a 2 case.
	else{
		buffer_read_to_page(table_id,pagenum);
		buf = find_buf(table_id,pagenum);
		
		if(buf!=NULL)
			memcpy(page,buf,PAGE_SIZE);
		else
			fprintf(stderr,"buf is null after file_read_page_to_buffer\n");
		/*{//it does not exit in page.so we write page into  disk
			file_write_page(table_id,page);
		
		}*/
	}
	page->pagenum = pagenum; //FIXME
}

int overwrite_buffer(int table_id,Buffer* tmp_buf,Page* page){
	//if tmp_buf is only one buffer,then nextBuffer can cause SEGV.so i make the case when buffer->nextBuffer is null,then just buffermgr.buf_used -- and free.
	
	if(tmp_buf->nextB ==NULL){
		if(tmp_buf !=buffermgr.firstBuf){
			buffermgr.firstBuf->is_pinned =1;
			tmp_buf->prevB->is_pinned=1;
			buffermgr.firstBuf->prevB = tmp_buf->prevB;
			tmp_buf->prevB->nextB = NULL; // whether or not tmp_buf is freed, NextBuffer must be deallocated because of the heap and system memory.
			tmp_buf->prevB->is_pinned =0;
			buffermgr.firstBuf->is_pinned=0;
		}
		else{
			buffermgr.firstBuf =NULL; //memory allocation problem.free does not always return NULL.
		}
		buffermgr.buf_used--;
		free(tmp_buf); //FIXME
	}
	else{	
		
		tmp_buf->is_pinned=1;
		tmp_buf->prevB->is_pinned=1;
		tmp_buf->nextB->is_pinned=1;
		if(tmp_buf == buffermgr.firstBuf){
			buffermgr.firstBuf = tmp_buf->nextB;
			buffermgr.firstBuf->prevB = tmp_buf->prevB;
		}
		else{
			tmp_buf->prevB->nextB = tmp_buf->nextB;
			tmp_buf->nextB->prevB = tmp_buf->prevB;
		}
		tmp_buf->prevB->is_pinned=0;
		tmp_buf->nextB->is_pinned=0;
		tmp_buf->is_pinned=0;
		buffermgr.buf_used--;
		free(tmp_buf);
		tmp_buf =NULL; //FIXME
	}
	
	return memory_write_to_buffer(table_id,page);
}
void file_write_page(int table_id,Page* page){		
	Buffer* buf = find_buf(table_id,page->pagenum);
	int status;
	//there is no already buffer,then just update buffer.
	if(buf ==NULL){
		status = memory_write_to_buffer(table_id,page);
	}
	//if there is already buffer which point same page,then free that buffer and create new buffer
	else{
		status = overwrite_buffer(table_id,buf,page);
	}
	
}
//return if buffer have to be  flushed,return 1;
int memory_write_to_buffer(int table_id,Page* page) {
    //build buffer pool to linked list;
	//if buffer_used is 0,then create logically first buffer.
	if(buffermgr.buf_used==0){
		Buffer* buf = (Buffer*)malloc(sizeof(Buffer));
		memcpy(buf,page,PAGE_SIZE);
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
	else if(buffermgr.buf_used < buffermgr.buf_order){
	//the logical first Buf and last Buf need to be modified.so I want to pin up both of them.
	//TODO: buffer is cycle.
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
		int status =  buffer_write_to_page(); //al
		if(status ==0)
			return memory_write_to_buffer(table_id,page);	
		else{
			fprintf(stderr,"there is no space to evict buffer\n");
			return status;
		}
	}
	return 0;

}
int buffer_write_to_page(){	
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
			lastBuf->is_pinned =1;
			lastBuf->nextB->is_pinned=1;
			lastBuf->prevB->is_pinned=1;
			if(lastBuf == buffermgr.firstBuf){
				buffermgr.firstBuf = buffermgr.firstBuf->nextB;
				buffermgr.firstBuf->nextB = lastBuf->nextB;
			}
			else{
				lastBuf->prevB->nextB = lastBuf->nextB;
				lastBuf->nextB->prevB = lastBuf->prevB;
			}
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
			lastBuf->is_pinned =1;
			lastBuf->prevB->is_pinned=1;
			buffermgr.firstBuf->is_pinned=1;
			buffermgr.firstBuf->prevB = lastBuf->prevB;
			lastBuf->prevB->nextB=NULL;
			lastBuf->prevB->is_pinned=0;
			buffermgr.firstBuf->is_pinned=0;	
			buffermgr.buf_used--;
		}	
		//if(lastBuf->is_pinned == 0){
		if(lastBuf->is_dirty==1){
			//int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			int fd2 = tablemgr.table_list[lastBuf->table_id].fd;
			lseek(fd2, PAGENUM_TO_FILEOFF(lastBuf->page_num), SEEK_SET);
    		write(fd2, (Page*)lastBuf, PAGE_SIZE);
		}
		//buffermgr->firstBuf->prevB = lastBuf->prevB;
		//buffermgr->firstBuf->prevB->nextB = NULL;
		//buffermgr->buf_used--;
		//}
		lastBuf->is_pinned = 0;
		free(lastBuf);
		return 0;
}
