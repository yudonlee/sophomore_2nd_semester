#include "bpt_internal.h"
#include "panic.h"
BufferMgr buffermgr;
TableList tablemgr;

// DELETION.

/* Utility function for deletion.  Retrieves the index of a node's nearest
 * neighbor (sibling) to the left if one exists.  If not (the node is the
 * leftmost child), returns -1 to signify this special case.
 */
int get_neighbor_index(int table_id,NodePage* node_page) {
	int i;

	/* Returns the index of the key to the left of the pointer in the parent
     * pointing to n. If n is the leftmost child, this means return -1.
	 */
    InternalPage parent_node;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(node_page->parent), (Page*)&parent_node);
	for (i = 0; i <= parent_node.num_keys; i++)
		if (INTERNAL_OFFSET(&parent_node, i)
                    == PAGENUM_TO_FILEOFF(node_page->pagenum))
			return i - 1;

	// Error state.
    PANIC("Search for nonexistent pointer to node in parent.");
    return -1;
}

void remove_entry_from_node(int table_id,NodePage* node_page, uint64_t key) {
	int i;
    int key_idx = 0;

    if (node_page->is_leaf) {
        LeafPage* leaf_node = (LeafPage*)node_page;

        // Finds a slot of deleting key
        for (i = 0; i < leaf_node->num_keys; i++) {
            if (LEAF_KEY(leaf_node, i) == key) {
                key_idx = i;
                break;
            }
        }
        if (i == leaf_node->num_keys) {
            PANIC("remove_entry_from_node: no key in this page");
        }

        // Shifts records
        for (i = key_idx; i < leaf_node->num_keys - 1; i++) {
            LEAF_KEY(leaf_node, i) = LEAF_KEY(leaf_node, i+1);
            memcpy(LEAF_VALUE(leaf_node, i), LEAF_VALUE(leaf_node, i+1),
                    SIZE_VALUE);
        }
        // Cleans garbage records
        LEAF_KEY(leaf_node, leaf_node->num_keys - 1) = 0;
        memset(LEAF_VALUE(leaf_node, leaf_node->num_keys - 1), 0, SIZE_VALUE);

        leaf_node->num_keys--;
    } else {
        InternalPage* internal_node = (InternalPage*)node_page;

        // Finds a slot of deleting key
        for (i = 0; i < internal_node->num_keys; i++) {
            if (INTERNAL_KEY(internal_node, i) == key) {
                key_idx = i;
                break;
            }
        }
        if (i == internal_node->num_keys) {
            PANIC("remove_entry_from_node: no key in this page");
        }

        // Shifts keys/pointers
        for (i = key_idx; i < internal_node->num_keys - 1; i++) {
            INTERNAL_KEY(internal_node, i) = INTERNAL_KEY(internal_node, i+1);
            INTERNAL_OFFSET(internal_node, i+1) =
                INTERNAL_OFFSET(internal_node, i+2);
        }
        // Clears garbage key/pointer
        INTERNAL_KEY(internal_node, internal_node->num_keys - 1) = 0;
        INTERNAL_OFFSET(internal_node, internal_node->num_keys) = 0;

        internal_node->num_keys--;
    }

    file_write_page(table_id,(Page*)node_page);
}

void adjust_root(int table_id) {
    NodePage root_page;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(tablemgr.table_list[table_id].headerpage->root_offset), (Page*)&root_page);

	/* Case: nonempty root.
     * Key and pointer have already been deleted, so nothing to be done.
	 */

	if (root_page.num_keys > 0)
        return;

	/* Case: empty root. 
	 */

	/* If it has a child, promote the first (only) child as the new root.
     */

	if (!root_page.is_leaf) {
        InternalPage* root_node = (InternalPage*)&root_page;
        tablemgr.table_list[table_id].headerpage->root_offset = INTERNAL_OFFSET(root_node, 0);
		
        NodePage node_page;
        file_read_page(table_id,FILEOFF_TO_PAGENUM(tablemgr.table_list[table_id].headerpage->root_offset), (Page*)&node_page);
        node_page.parent = 0;
        file_write_page(table_id,(Page*)&node_page);
        file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
	}

	/* If it is a leaf (has no children), then the whole tree is empty.
	 */ 

	else {
        tablemgr.table_list[table_id].headerpage->root_offset = 0;
        file_write_page(table_id,(Page*)tablemgr.table_list[table_id].headerpage);
    }

    file_free_page(table_id,root_page.pagenum);
}

/* Coalesces a node that has become too small after deletion with a neighboring
 * node that can accept the additional entries without exceeding the maximum.
 */
void coalesce_nodes(int table_id,NodePage* node_page, NodePage* neighbor_page,
        int neighbor_index, int k_prime) {

	int i, j, neighbor_insertion_index, n_end;
	NodePage* tmp;

	/* Swaps neighbor with node if node is on the extreme left and neighbor is to
     * its right.
	 */

	if (neighbor_index == -1) {
		tmp = node_page;
		node_page = neighbor_page;
		neighbor_page = tmp;
	}

	/* Starting point in the neighbor for copying keys and pointers from n.
	 * Recall that n and neighbor have swapped places in the special case of n
     * being a leftmost child.
	 */

	neighbor_insertion_index = neighbor_page->num_keys;

	/* Case:  nonleaf node.
	 * Appends k_prime and the following pointer.
	 * Appends all pointers and keys from the neighbor.
	 */

	if (!node_page->is_leaf) {
        InternalPage* node = (InternalPage*)node_page;
        InternalPage* neighbor_node = (InternalPage*)neighbor_page;

		/* Appends k_prime.
		 */

		INTERNAL_KEY(neighbor_node, neighbor_insertion_index) = k_prime;
		neighbor_node->num_keys++;

		n_end = node->num_keys;

		for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
			INTERNAL_KEY(neighbor_node, i) = INTERNAL_KEY(node, j);
			INTERNAL_OFFSET(neighbor_node, i) = INTERNAL_OFFSET(node, j);
			neighbor_node->num_keys++;
			node->num_keys--;
		}

		/* The number of pointers is always one more than the number of keys.
		 */

		INTERNAL_OFFSET(neighbor_node, i) = INTERNAL_OFFSET(node, j);

		/* All children must now point up to the same parent.
		 */

		for (i = 0; i < neighbor_node->num_keys + 1; i++) {
            NodePage child_page;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(INTERNAL_OFFSET(neighbor_node, i)),
                           (Page*)&child_page);
            child_page.parent = PAGENUM_TO_FILEOFF(neighbor_node->pagenum);
		    file_write_page(table_id,(Page*)&child_page);
        }

        file_write_page(table_id,(Page*)neighbor_node);

        file_free_page(table_id,node->pagenum);
	}

	/* In a leaf, appends the keys and pointers of n to the neighbor.
	 * Sets the neighbor's last pointer to point to what had been n's right
     * neighbor.
	 */

	else {
        LeafPage* node = (LeafPage*)node_page;
        LeafPage* neighbor_node = (LeafPage*)neighbor_page;

		for (i = neighbor_insertion_index, j = 0; j < node->num_keys; i++, j++) {
			LEAF_KEY(neighbor_node, i) = LEAF_KEY(node, j);
            memcpy(LEAF_VALUE(neighbor_node, i), LEAF_VALUE(node, j),
                    SIZE_VALUE);
			neighbor_node->num_keys++;
		}
        neighbor_node->sibling = node->sibling;

        file_write_page(table_id,(Page*)neighbor_node);

        file_free_page(table_id,node->pagenum);
	}

    NodePage parent_node;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(node_page->parent), (Page*)&parent_node);
	delete_entry(table_id,&parent_node, k_prime);
}

/* Redistributes entries between two nodes when one has become too small after
 * deletion but its neighbor is too big to append the small node's entries
 * without exceeding the maximum.
 */
void redistribute_nodes(int table_id,NodePage* node_page, NodePage* neighbor_page,
                        int neighbor_index, 
		                int k_prime_index, int k_prime) {  

	int i;

	/* Case: n has a neighbor to the left. 
	 * Pulls the neighbor's last key-pointer pair over from the neighbor's right
     * end to n's left end.
	 */

	if (neighbor_index != -1) {
		if (!node_page->is_leaf) {
            InternalPage* node = (InternalPage*)node_page;
            InternalPage* neighbor_node = (InternalPage*)neighbor_page;
			INTERNAL_OFFSET(node, node->num_keys + 1) =
                INTERNAL_OFFSET(node, node->num_keys);
            
            for (i = node->num_keys; i > 0; i--) {
		    	INTERNAL_KEY(node, i) = INTERNAL_KEY(node, i - 1);
		    	INTERNAL_OFFSET(node, i) = INTERNAL_OFFSET(node, i - 1);
		    }
            INTERNAL_OFFSET(node, 0) =
                INTERNAL_OFFSET(neighbor_node, neighbor_node->num_keys);
            NodePage child_page;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(INTERNAL_OFFSET(node, 0)),
                           (Page*)&child_page);
            child_page.parent = PAGENUM_TO_FILEOFF(node->pagenum);
            file_write_page(table_id,(Page*)&child_page);

			INTERNAL_OFFSET(neighbor_node, neighbor_node->num_keys) = 0;
			INTERNAL_KEY(node, 0) = k_prime;

            InternalPage parent_node;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(node->parent),
                           (Page*)&parent_node);
            INTERNAL_KEY(&parent_node, k_prime_index) =
                INTERNAL_KEY(neighbor_node, neighbor_node->num_keys - 1);
            file_write_page(table_id,(Page*)&parent_node);

            /* n now has one more key and one more pointer;
             * the neighbor has one fewer of each.
             */
            node->num_keys++;
            neighbor_node->num_keys--;
            
            file_write_page(table_id,(Page*)node_page);
            file_write_page(table_id,(Page*)neighbor_page);

        } else {
            LeafPage* node = (LeafPage*)node_page;
            LeafPage* neighbor_node = (LeafPage*)neighbor_page;

            for (i = node->num_keys; i > 0; i--) {
			    LEAF_KEY(node, i) = LEAF_KEY(node, i - 1);
			    memcpy(LEAF_VALUE(node, i), LEAF_VALUE(node, i - 1), SIZE_VALUE);
		    }
            memcpy(LEAF_VALUE(node, 0),
                    LEAF_VALUE(neighbor_node, neighbor_node->num_keys - 1),
                    SIZE_VALUE);
			memset(LEAF_VALUE(neighbor_node, neighbor_node->num_keys - 1), 0,
                    SIZE_VALUE);
			LEAF_KEY(node, 0) = LEAF_KEY(neighbor_node, neighbor_node->num_keys - 1);

            InternalPage parent_node;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(node->parent),
                           (Page*)&parent_node);
			INTERNAL_KEY(&parent_node, k_prime_index) = LEAF_KEY(node, 0);
            file_write_page(table_id,(Page*)&parent_node);

            /* n now has one more key and one more pointer;
             * the neighbor has one fewer of each.
             */
            node->num_keys++;
            neighbor_node->num_keys--;
            
            file_write_page(table_id,(Page*)node_page);
            file_write_page(table_id,(Page*)neighbor_page);
        }
    }

	/* Case: n is the leftmost child.
	 * Takes a key-pointer pair from the neighbor to the right.
	 * Moves the neighbor's leftmost key-pointer pair to n's rightmost position.
	 */

	else {  
		if (node_page->is_leaf) {
            LeafPage* node = (LeafPage*)node_page;
            LeafPage* neighbor_node = (LeafPage*)neighbor_page;;

			LEAF_KEY(node, node->num_keys) = LEAF_KEY(neighbor_node, 0);
			memcpy(LEAF_VALUE(node, node->num_keys), LEAF_VALUE(neighbor_node, 0),
                    SIZE_VALUE);

            InternalPage parent_node;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(node->parent),
                           (Page*)&parent_node);
			INTERNAL_KEY(&parent_node, k_prime_index) = LEAF_KEY(neighbor_node, 1);
            file_write_page(table_id,(Page*)&parent_node);
            
            for (i = 0; i < neighbor_node->num_keys - 1; i++) {
			    LEAF_KEY(neighbor_node, i) = LEAF_KEY(neighbor_node, i + 1);
			    memcpy(LEAF_VALUE(neighbor_node, i), LEAF_VALUE(neighbor_node, i + 1),
                        SIZE_VALUE);
            }

            /* n now has one more key and one more pointer;
             * the neighbor has one fewer of each.
             */
            node->num_keys++;
            neighbor_node->num_keys--;
            
            file_write_page(table_id,(Page*)node_page);
            file_write_page(table_id,(Page*)neighbor_page);

		}
		else {
            InternalPage* node = (InternalPage*)node_page;
            InternalPage* neighbor_node = (InternalPage*)neighbor_page;

			INTERNAL_KEY(node, node->num_keys) = k_prime;
			INTERNAL_OFFSET(node, node->num_keys + 1) =
                INTERNAL_OFFSET(neighbor_node, 0);
            
            NodePage child_page;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(INTERNAL_OFFSET(node, node->num_keys + 1)),
                           (Page*)&child_page);
            child_page.parent = PAGENUM_TO_FILEOFF(node->pagenum);
			file_write_page(table_id,(Page*)&child_page);

            InternalPage parent_node;
            file_read_page(table_id,FILEOFF_TO_PAGENUM(node->parent),
                           (Page*)&parent_node);
            INTERNAL_KEY(&parent_node, k_prime_index) =
                INTERNAL_KEY(neighbor_node, 0);
            file_write_page(table_id,(Page*)&parent_node);

            for (i = 0; i < neighbor_node->num_keys - 1; i++) {
			    INTERNAL_KEY(neighbor_node, i) = INTERNAL_KEY(neighbor_node, i + 1);
			    INTERNAL_OFFSET(neighbor_node, i) =
                    INTERNAL_OFFSET(neighbor_node, i + 1);

		    }
			INTERNAL_OFFSET(neighbor_node, i) = INTERNAL_OFFSET(neighbor_node, i + 1);


            /* n now has one more key and one more pointer;
             * the neighbor has one fewer of each.
             */

            node->num_keys++;
            neighbor_node->num_keys--;
            
            file_write_page(table_id,(Page*)node_page);
            file_write_page(table_id,(Page*)neighbor_page);

		}
    }
}


/* Deletes an entry from the B+ tree.
 * Removes the record and its key and pointer from the leaf, and then makes all
 * appropriate changes to preserve the B+ tree properties.
 */
void delete_entry(int table_id,NodePage* node_page, uint64_t key) {

	int min_keys;
	off_t neighbor_offset;
	int neighbor_index;
	int k_prime_index, k_prime;
	int capacity;

	// Removes key and pointer from node.

	remove_entry_from_node(table_id,node_page, key);
	/* Case:  deletion from the root. 
	 */

	if (tablemgr.table_list[table_id].headerpage->root_offset == PAGENUM_TO_FILEOFF(node_page->pagenum)) {
		adjust_root(table_id);
        return;
    }

	/* Case:  deletion from a node below the root.
	 * (Rest of function body.)
	 */

	/* Determines minimum allowable size of node, to be preserved after deletion.
	 */

#ifdef IS_DELAYED_MERGE
	min_keys = 1;
#else
	min_keys = node_page->is_leaf ? cut(order_leaf - 1) : cut(order_internal) - 1;
#endif

	/* Case:  node stays at or above minimum.
	 * (The simple case.)
	 */

	if (node_page->num_keys >= min_keys)
        return;
	
    /* Case:  node falls below minimum.
	 * Either coalescence or redistribution is needed.
	 */

	/* Finds the appropriate neighbor node with which to coalesce.
	 * Also finds the key (k_prime) in the parent between the pointer to node n
     * and the pointer to the neighbor.
	 */

	neighbor_index = get_neighbor_index(table_id,node_page);
	k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;

    InternalPage parent_node;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(node_page->parent), (Page*)&parent_node);

	k_prime = INTERNAL_KEY(&parent_node, k_prime_index);
	neighbor_offset = neighbor_index == -1 ? INTERNAL_OFFSET(&parent_node, 1) : 
		INTERNAL_OFFSET(&parent_node, neighbor_index);

	capacity = node_page->is_leaf ? order_leaf : order_internal - 1;

    NodePage neighbor_page;
    file_read_page(table_id,FILEOFF_TO_PAGENUM(neighbor_offset), (Page*)&neighbor_page);
	/* Coalescence. */

	if (neighbor_page.num_keys + node_page->num_keys < capacity)
		coalesce_nodes(table_id,node_page, &neighbor_page, neighbor_index, k_prime);

	/* Redistribution. */

	else
		redistribute_nodes(table_id,node_page, &neighbor_page, neighbor_index, k_prime_index,
                k_prime);
}

/* Master deletion function.
 */
int delete_record(int table_id,uint64_t key) {
    char* value_found = NULL;
    if ((value_found = find_record(table_id,key)) == 0) {
        // This key is not in the tree
        free(value_found);
        return -1;
    }

    LeafPage leaf_node;
    find_leaf(table_id,key, &leaf_node);

    delete_entry(table_id,(NodePage*)&leaf_node, key);

    return 0;
}

