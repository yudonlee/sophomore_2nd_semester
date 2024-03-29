#include "bpt.h"
#include "panic.h"

// Closes the db file.
int find_table_id(char* table_name){
	for(int i=1;i<=10;i++){
		if(strcmp(tablemgr.table_list[i].name,table_name)==0)
			return i; // success
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
				break;
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
		new_table->headerpage = (HeaderPage*)malloc(sizeof(HeaderPage));
		memset(new_table->headerpage, 0, PAGE_SIZE); //&new_table->headerpage or new_table->headerpage
        new_table->headerpage->freelist = 0;
        new_table->headerpage->root_offset = 0;
        new_table->headerpage->num_pages = 1;
        new_table->headerpage->pagenum = 0;
		//new_table->table_id = table_index; 
		file_write_page(table_index,(Page*)new_table->headerpage); //&new_table->headerpage or new_table->headerpage
    	return table_index;
	} else {
		//fprintf(stderr,"open error\n");
        // DB file exists. Loads header info
		int table_index=-1;
		for(int i=1;i<=10;i++){
			if(tablemgr.table_list[i].fd <0){
				table_index =i;
				break;
			}
		}
		if(table_index==-1)
        	fprintf(stderr,"No seat for table_list.please close db more than 1\n");
		Table* new_table = &tablemgr.table_list[table_index];
		new_table->headerpage = (HeaderPage*)malloc(sizeof(HeaderPage));//headerpage allocated.	
		new_table->fd = dbfile;
		strcpy(new_table->name,filename);
	file_read_headerpage(table_index,0);
    	file_read_page(table_index,0,(Page*)new_table->headerpage);
		return table_index;
	}
}

int close_db_table(int table_id){
	Buffer* buf;
	buf = buffermgr.firstBuf;
	while(buf->nextB !=NULL){
		if(buf == buffermgr.firstBuf && buf->table_id == table_id){
			buf->is_pinned =1;
			buf->nextB->is_pinned =1;
			buf->prevB->is_pinned =1;
			buf->nextB->prevB = buffermgr.firstBuf->prevB;
			buf->is_pinned =0;
			buf->nextB->is_pinned =0;
			buf->prevB->is_pinned =0;
			buffermgr.buf_used--;
			buffermgr.firstBuf = buf->nextB;
			if(buf->is_dirty == 1){
				int fd2 = tablemgr.table_list[buf->table_id].fd;
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
		}
		else if(buf->table_id == table_id){
			buf->is_pinned=1;
			buf->prevB->is_pinned=1;
			buf->nextB->is_pinned=1;
			buf->prevB->nextB = buf->nextB;
			buf->nextB->prevB = buf->prevB;
			buf->is_pinned=0;
			buf->prevB->is_pinned=0;
			buf->nextB->is_pinned=0;
			buffermgr.buf_used--;
			if(buf->is_dirty == 1){
				int fd2 = tablemgr.table_list[buf->table_id].fd;
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}

		}
		buf = buf->nextB;
	}
	if(buf->nextB == NULL && buf->table_id == table_id){
		if(buffermgr.buf_used == 1){	
			if(buf->is_dirty == 1){
				int fd2 = tablemgr.table_list[buf->table_id].fd;
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
			buffermgr.buf_used--;
			buffermgr.firstBuf = NULL;
		}
		else{
			buffermgr.firstBuf->is_pinned =1;
			buf->is_pinned =1;
			buf->prevB->is_pinned =1;
			buffermgr.firstBuf->prevB = buf->prevB;
			buf->prevB->nextB = NULL;
			buf->is_pinned =0;
			buffermgr.firstBuf->is_pinned =0;
			buf->prevB->is_pinned =0;
			if(buf->is_dirty == 1){
				int fd2 = tablemgr.table_list[buf->table_id].fd;
				lseek(fd2, PAGENUM_TO_FILEOFF(buf->page_num), SEEK_SET);
    			write(fd2, (Page*)buf, PAGE_SIZE);
			}
			buffermgr.buf_used--;
		}
	}
	memset(&tablemgr.table_list[table_id],0,sizeof(Table));
	tablemgr.table_list[table_id].headerpage = NULL;
	tablemgr.table_list[table_id].fd = -1;
	return 0;
}
int shutdown_db(){
	if(buffermgr.buf_used ==0)
		return 0;
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
			close(tablemgr.table_list[i].fd);//close all already open file.this is no need.
	}
	return 0; //success 
	
}
