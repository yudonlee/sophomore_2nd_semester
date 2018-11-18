#include "bpt_internal.h"
BufferMgr buffermgr;
TableList tablemgr;
/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */

Buffer* find_buf(int table_id,pagenum_t page_num){
	Buffer* output;
	output = buffermgr.firstBuf;
	while(output != NULL){
		if(output->table_id == table_id && output->page_num){
			return output;
		}
		output = output->nextB;
	}
	if(output ==NULL){
		int fd2 = dup( tablemgr.table_list[table_id].fd );
		lseek(fd2, PAGENUM_TO_FILEOFF(page_num), SEEK_SET);
    	read(fd2,output,PAGE_SIZE);
		file_write_to_buffer(table_id,(Page*)output);
	}
	return NULL;
}

bool find_leaf(int table_id,uint64_t key, LeafPage* out_leaf_node) {
    int i = 0;
    off_t root_offset = tablemgr.table_list[table_id].headerpage->root_offset;

	if (root_offset == 0) {
		return false;
	}
    
    NodePage page;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(root_offset), (Page*)&page);

	while (!page.is_leaf) {
        InternalPage* internal_node = (InternalPage*)&page;

        i = 0;
		while (i < internal_node->num_keys) {
			if (key >= INTERNAL_KEY(internal_node, i)) i++;
			else break;
		}
        
        file_read_page(table_id,FILEOFF_TO_PAGENUM(INTERNAL_OFFSET(internal_node, i)),
                       (Page*)&page);
	}

    memcpy(out_leaf_node, &page, sizeof(LeafPage));

	return true;
}

/* Finds and returns the value to which
 * a key refers.
 */
char* find_record(int table_id,uint64_t key) {
    int i = 0;
    char* out_value;

    LeafPage leaf_node;
    if (!find_leaf(table_id,key, &leaf_node)) {
        return NULL;
    }

	for (i = 0; i < leaf_node.num_keys; i++) {
		if (LEAF_KEY(&leaf_node, i) == key) {
            out_value = (char*)malloc(SIZE_VALUE * sizeof(char));
            memcpy(out_value, LEAF_VALUE(&leaf_node, i), SIZE_VALUE);
            return out_value;
        }
    }

    return NULL;
}

/* Finds the appropriate place to
 * split a node that is too big into two.
 */
int cut( int length ) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}
