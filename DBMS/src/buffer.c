#include "buffer.h"

int find_table_id(char* table_name){
	for(int i=1;i<=10;i++){
		if(strcmp(tablemgr.table_list[i].name,table_name))
			return 1; // success
	}
	return FAIL;

}
void file_read_page_to_buffer(int table_id,pagenum_t pagenum){
	Buffer* tmp_buf = (Buffer*)malloc(sizeof(Buffer));
	//int fdd2 = dup( tablemgr.table_list[table_id].fd ); 
	int fd2 = tablemgr.table_list[table_id].fd; 
	printf("fd2 is :%d\n",fd2);
	lseek(fd2, PAGENUM_TO_FILEOFF(pagenum), SEEK_SET);
   	read(fd2,tmp_buf,PAGE_SIZE);
	tmp_buf->table_id = table_id;
	tmp_buf->page_num = pagenum;
	tmp_buf->is_dirty = 0;
	tmp_buf->is_pinned = 1; //pinned is 1 because this is update circumstance
	if(buffermgr.buf_used ==0){
		tmp_buf->nextB =NULL;
		tmp_buf->prevB = tmp_buf;
		buffermgr.buf_used++;
		tmp_buf->is_pinned = 0;
		printf("tmp buf is well worked\n");
		buffermgr.firstBuf = tmp_buf;
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
		memcpy(page,buf,PAGE_SIZE);
	}
	//it cannot exit in buffer.so we take a 2 case.
	else{
		file_read_page_to_buffer(table_id,pagenum);
		buf = find_buf(table_id,pagenum);
		printf("buf table_id,page_num,:%d %jd\n",buf->table_id,buf->page_num);
		if(buf!=NULL)
			memcpy(page,buf,PAGE_SIZE);
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
			//int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
			int fd2 = tablemgr.table_list[lastBuf->table_id].fd;
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
