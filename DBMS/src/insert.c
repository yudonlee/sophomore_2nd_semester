#include "bpt_internal.h"
#include "panic.h"
BufferMgr buffermgr;
TableList tablemgr;
/* Helper function used in insert_into_parent to find the index of the parent's
 * pointer to the node to the left of the key to be inserted.
 */
int get_left_index(int table_id,InternalPage* parent, off_t left_offset) {
	int left_index = 0;
	while (left_index <= parent->num_keys && 
			INTERNAL_OFFSET(parent, left_index) != left_offset)
		left_index++;
	return left_index;
}

/* Inserts a new value and its corresponding key into a leaf.
 */
void insert_into_leaf(int table_id,LeafPage* leaf_node, uint64_t key, const char* value) {
	int insertion_point;
    int i;

	insertion_point = 0;
	while (insertion_point < leaf_node->num_keys &&
           LEAF_KEY(leaf_node, insertion_point) < key)
		insertion_point++;

    // Shifts keys and values to the right
    for (i = leaf_node->num_keys - 1; i >= insertion_point; i--) {
        LEAF_KEY(leaf_node, i+1) = LEAF_KEY(leaf_node, i);
        memcpy(LEAF_VALUE(leaf_node, i+1), LEAF_VALUE(leaf_node, i),
                SIZE_VALUE);
    }
    
    LEAF_KEY(leaf_node, insertion_point) = key;
    memcpy(LEAF_VALUE(leaf_node, insertion_point), value, SIZE_VALUE);
	leaf_node->num_keys++;

    // Flushes leaf node to the file page
    file_write_page(table_id,(Page*)leaf_node);
}

/* Inserts a new record into a leaf so as to exceed the tree's order, causing
 * the leaf to be split in half.
 */
void insert_into_leaf_after_splitting(int table_id,LeafPage* leaf, uint64_t key,
        const char* value) {
	int insertion_index, split, i, j;
    uint64_t new_key;

    // Makes a new leaf node
    LeafPage new_leaf;
    new_leaf.is_leaf = true;
    new_leaf.num_keys = 0;
	/*		if(buf->is_dirty == 1){
				int fd2 = dup(tablemgr.table_list[lastBuf->table_id].fd);
				lseek(fd2, PAGENUM_TO_FILEOFF(page->pagenum), SEEK_SET);
    			write(fd2, (Page*)lastbuf, PAGE_SIZE);
			}
	*/
    insertion_index = 0;
    while (insertion_index < order_leaf - 1 &&
            LEAF_KEY(leaf, insertion_index) < key){
		insertion_index++;
    }

	split = cut(order_leaf - 1);

    if (insertion_index < split) {
        // New key is going to be inserted to the old leaf
        for (i = split - 1, j = 0; i < order_leaf - 1; i++, j++) {
            LEAF_KEY(&new_leaf, j) = LEAF_KEY(leaf, i);
            memcpy(LEAF_VALUE(&new_leaf, j), LEAF_VALUE(leaf, i), SIZE_VALUE);

            new_leaf.num_keys++;
            leaf->num_keys--;
        }
        // Shifts keys to make space for new record
        for (i = split - 2; i >= insertion_index; i--) {
            LEAF_KEY(leaf, i+1) = LEAF_KEY(leaf, i);
            memcpy(LEAF_VALUE(leaf, i+1), LEAF_VALUE(leaf, i), SIZE_VALUE);
        }
        LEAF_KEY(leaf, insertion_index) = key;
        memcpy(LEAF_VALUE(leaf, insertion_index), value, SIZE_VALUE);
        leaf->num_keys++;
    } else {
        // New key is going to be inserted to the new leaf
        for (i = split, j = 0; i < order_leaf - 1; i++, j++) {
            if (i == insertion_index) {
                // Makes space for new record
                j++;
            }
            LEAF_KEY(&new_leaf, j) = LEAF_KEY(leaf, i);
            memcpy(LEAF_VALUE(&new_leaf, j), LEAF_VALUE(leaf, i), SIZE_VALUE);

            new_leaf.num_keys++;
            leaf->num_keys--;
        }
        LEAF_KEY(&new_leaf, insertion_index - split) = key;
        memcpy(LEAF_VALUE(&new_leaf, insertion_index - split), value,
                SIZE_VALUE);
        new_leaf.num_keys++;
    }
   
    // Allocates a page for new leaf
    new_leaf.pagenum = file_alloc_page(table_id);

    // Links the leaves
	new_leaf.sibling = leaf->sibling;
	leaf->sibling = PAGENUM_TO_FILEOFF(new_leaf.pagenum);
   
    // Cleans garbage records
	for (i = leaf->num_keys; i < order_leaf - 1; i++) {
		LEAF_KEY(leaf, i) = 0;
        memset(LEAF_VALUE(leaf, i), 0, SIZE_VALUE);
    }
	for (i = new_leaf.num_keys; i < order_leaf - 1; i++) {
		LEAF_KEY(&new_leaf, i) = 0;
        memset(LEAF_VALUE(&new_leaf, i), 0, SIZE_VALUE);
    }

	new_leaf.parent = leaf->parent;

    file_write_page(table_id,(Page*)leaf);
    file_write_page(table_id,(Page*)&new_leaf);

	new_key = LEAF_KEY(&new_leaf, 0);

    // Inserts new key and new leaf to the parent
	insert_into_parent(table_id,(NodePage*)leaf, new_key, (NodePage*)&new_leaf);
}

/* Inserts a new key and pointer to a node into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(int table_id,InternalPage* n, int left_index, uint64_t key,
        off_t right_offset) {
    int i;

	for (i = n->num_keys; i > left_index; i--) {
		INTERNAL_OFFSET(n, i + 1) = INTERNAL_OFFSET(n, i);
		INTERNAL_KEY(n, i) = INTERNAL_KEY(n, i - 1);
	}
	INTERNAL_OFFSET(n, left_index + 1) = right_offset;
	INTERNAL_KEY(n, left_index) = key;
	n->num_keys++;
}

/* Inserts a new key and pointer to a node into a node, causing the node's size
 * to exceed the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(int table_id,InternalPage* old_node, int left_index,
        uint64_t key, off_t right_offset) {
    int i, j, split, k_prime;
	uint64_t* temp_keys;
	off_t* temp_pointers;

	/* First create a temporary set of keys and pointers to hold everything in
     * order, including the new key and pointer, inserted in their correct
     * places. 
	 * Then create a new node and copy half of the keys and pointers to the old
     * node and the other half to the new.
	 */

	temp_pointers = malloc( (order_internal + 1) * sizeof(off_t) );
	if (temp_pointers == NULL) {
		PANIC("Temporary pointers array for splitting nodes.");
	}
	temp_keys = malloc( order_internal * sizeof(uint64_t) );
	if (temp_keys == NULL) {
		PANIC("Temporary keys array for splitting nodes.");
	}

	for (i = 0, j = 0; i < old_node->num_keys + 1; i++, j++) {
		if (j == left_index + 1) j++;
		temp_pointers[j] = INTERNAL_OFFSET(old_node, i);
	}

	for (i = 0, j = 0; i < old_node->num_keys; i++, j++) {
		if (j == left_index) j++;
		temp_keys[j] = INTERNAL_KEY(old_node, i);
	}

	temp_pointers[left_index + 1] = right_offset;
	temp_keys[left_index] = key;

	/* Creates the new node and copy half the keys and pointers to the old and
     * half to the new.
	 */  
	split = cut(order_internal);

    InternalPage new_node;
	new_node.num_keys = 0;
    new_node.is_leaf = 0;
    new_node.pagenum = file_alloc_page(table_id);

    old_node->num_keys = 0;
	for (i = 0; i < split - 1; i++) {
		INTERNAL_OFFSET(old_node, i) = temp_pointers[i];
		INTERNAL_KEY(old_node, i) = temp_keys[i];
		old_node->num_keys++;
	}
	INTERNAL_OFFSET(old_node, i) = temp_pointers[i];
	k_prime = temp_keys[split - 1];
	for (++i, j = 0; i < order_internal; i++, j++) {
		INTERNAL_OFFSET(&new_node, j) = temp_pointers[i];
		INTERNAL_KEY(&new_node, j) = temp_keys[i];
		new_node.num_keys++;
	}
	INTERNAL_OFFSET(&new_node, j) = temp_pointers[i];
	free(temp_pointers);
	free(temp_keys);
	new_node.parent = old_node->parent;
	for (i = 0; i <= new_node.num_keys; i++) {
		NodePage child_page;
        file_read_page(table_id,FILEOFF_TO_PAGENUM(INTERNAL_OFFSET(&new_node, i)),
                       (Page*)&child_page);
        child_page.parent = PAGENUM_TO_FILEOFF(new_node.pagenum);
	    file_write_page(table_id,(Page*)&child_page);
    }

    // Cleans garbage records
    for (i = old_node->num_keys; i < order_internal - 1; i++) {
        INTERNAL_OFFSET(old_node, i+1) = 0;
        INTERNAL_KEY(old_node, i) = 0;
    }

    for (i = new_node.num_keys; i < order_internal - 1; i++) {
        INTERNAL_OFFSET(&new_node, i+1) = 0;
        INTERNAL_KEY(&new_node, i) = 0;
    }

    // Flushes old, new nodes
    file_write_page(table_id,(Page*)&new_node);
    file_write_page(table_id,(Page*)old_node);

	/* Inserts a new key into the parent of the two nodes resulting from the
     * split, with the old node to the left and the new to the right.
	 */
	insert_into_parent(table_id,(NodePage*)old_node, k_prime, (NodePage*)&new_node);
}

/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
void insert_into_parent(int table_id,NodePage* left, uint64_t key, NodePage* right) {
	int left_index;
    InternalPage parent_node;

    /* Case: new root. */
	if (left->parent == 0) {
		insert_into_new_root(table_id,left, key, right);
        return;
    }

    file_read_page(table_id,FILEOFF_TO_PAGENUM(left->parent), (Page*)&parent_node);

	/* Case: leaf or node. (Remainder of function body.)  
	 */

	/* Finds the parent's pointer to the left node.
	 */
	left_index = get_left_index(table_id,&parent_node,PAGENUM_TO_FILEOFF(left->pagenum));

	/* Simple case: the new key fits into the node. 
	 */
	if (parent_node.num_keys < order_internal - 1) {
		insert_into_node(table_id,&parent_node, left_index, key,
                         PAGENUM_TO_FILEOFF(right->pagenum));
        file_write_page(table_id,(Page*)&parent_node);
        return;
    }

	/* Harder case: splits a node in order to preserve the B+ tree properties.
	 * 
	 */
	return insert_into_node_after_splitting(table_id,&parent_node, left_index, key,
                                            PAGENUM_TO_FILEOFF(right->pagenum));
}

/* Creates a new root for two subtrees and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(int table_id,NodePage* left, uint64_t key, NodePage* right) {
    // Makes new root node
    InternalPage root_node;
    memset(&root_node, 0, sizeof(InternalPage));
    root_node.pagenum = file_alloc_page(table_id);
    INTERNAL_KEY(&root_node, 0) = key;
    INTERNAL_OFFSET(&root_node, 0) = PAGENUM_TO_FILEOFF(left->pagenum);
    INTERNAL_OFFSET(&root_node, 1) = PAGENUM_TO_FILEOFF(right->pagenum);
    root_node.num_keys++;
    root_node.parent = 0;
    root_node.is_leaf = 0;
    left->parent = PAGENUM_TO_FILEOFF(root_node.pagenum);
    right->parent = PAGENUM_TO_FILEOFF(root_node.pagenum);

    file_write_page(table_id,(Page*)&root_node);
    file_write_page(table_id,(Page*)left);
    file_write_page(table_id,(Page*)right);

    tablemgr.table_list[table_id].headerpage->root_offset = PAGENUM_TO_FILEOFF(root_node.pagenum);
    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
}
/* First insertion: start a new tree.
 */
void start_new_tree(int table_id,uint64_t key, const char* value) {
    LeafPage root_node;
    
    pagenum_t root_pagenum = file_alloc_page(table_id);
    root_node.pagenum = root_pagenum;

    root_node.parent = 0;
    root_node.is_leaf = 1;
    root_node.num_keys = 1;
    LEAF_KEY(&root_node, 0) = key;
    root_node.sibling = 0;
    memcpy(LEAF_VALUE(&root_node, 0), value, SIZE_VALUE);
    
    file_write_page(table_id,(Page*)&root_node);

    tablemgr.table_list[table_id].headerpage->root_offset = PAGENUM_TO_FILEOFF(root_pagenum);
    file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
}

/* Master insertion function.
 * Inserts a key and an associated value into the B+ tree, causing the tree to
 * be adjusted however necessary to maintain the B+ tree properties.
 */
int insert_record(int table_id,uint64_t key, const char* value) {
    char* value_found = NULL;

    /* The current implementation ignores duplicates.
	 */
    if ((value_found = find_record(table_id,key)) != 0) {
        free(value_found);
        return -1;
    }

	/* Case: the tree does not exist yet. Start a new tree.
	 */
	if (tablemgr.table_list[table_id].headerpage->root_offset == 0) {
		start_new_tree(table_id,key, value);
        return 0;
    }
	
    /* Case: the tree already exists. (Rest of function body.)
	 */
    LeafPage leaf_node;
    find_leaf(table_id,key, &leaf_node);

	/* Case: leaf has room for key and pointer.
	 */
	if (leaf_node.num_keys < order_leaf - 1) {
        insert_into_leaf(table_id,&leaf_node, key, value);
	} else {
    	/* Case:  leaf must be split.
	     */
        insert_into_leaf_after_splitting(table_id,&leaf_node, key, value);
    }
    return 0;
}
