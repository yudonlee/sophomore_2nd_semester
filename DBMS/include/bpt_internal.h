#ifndef __BPT_INTERNAL_H__
#define __BPT_INTERNAL_H__
#define Version "1.14"
/*
 *
 *  bpt:  B+ Tree Implementation
 *  Copyright (C) 2010-2016  Amittai Aviram  http://www.amittai.com
 *  All rights reserved.
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, 
 *  this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice, 
 *  this list of conditions and the following disclaimer in the documentation 
 *  and/or other materials provided with the distribution.
 
 *  3. Neither the name of the copyright holder nor the names of its 
 *  contributors may be used to endorse or promote products derived from this 
 *  software without specific prior written permission.
 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 *  POSSIBILITY OF SUCH DAMAGE.
 
 *  Author:  Amittai Aviram 
 *    http://www.amittai.com
 *    amittai.aviram@gmail.edu or afa13@columbia.edu
 *  Original Date:  26 June 2010
 *  Last modified: 17 June 2016
 *
 *  This implementation demonstrates the B+ tree data structure
 *  for educational purposes, includin insertion, deletion, search, and display
 *  of the search path, the leaves, or the whole tree.
 *  
 *  Must be compiled with a C99-compliant C compiler such as the latest GCC.
 *
 *  Usage:  bpt [order]
 *  where order is an optional argument
 *  (integer MIN_ORDER <= order <= MAX_ORDER)
 *  defined as the maximal number of pointers in any node.
 *
 */

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include "file.h"
#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

//#define TEST_MAIN 1

// Minimum order is necessarily 3.  We set the maximum
// order arbitrarily.  You may change the maximum order.
#define MIN_ORDER 3
#define MAX_ORDER 256

#define IS_DELAYED_MERGE

// Constants for printing part or all of the GPL license.
#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625

// GLOBALS.
extern int order_internal;
extern int order_leaf;
extern TableList tablemgr;
extern BufferMgr buffermgr;
//extern HeaderPage dbheader;
//extern int dbfile;

// FUNCTION PROTOTYPES.

// Output and utility.
bool find_leaf(int table_id,uint64_t key, LeafPage* out_leaf_node);
char *find_record(int table_id,uint64_t key);
Buffer* find_buf(int table_id,pagenum_t page_num);
// Helper function
int cut( int length );

// DB initialization
int init_db(int num_buf);
int open_or_create_db_file(const char* filename);
void close_db_table(int table_id);

// Insertion.
void start_new_tree(int table_id,uint64_t key, const char* value);
void insert_into_leaf(int table_id,LeafPage* leaf_node, uint64_t key, const char* value);
void insert_into_leaf_after_splitting(int table_id,LeafPage* leaf_node, uint64_t key,
        const char* value);
void insert_into_parent(int table_id,NodePage* left, uint64_t key, NodePage* right);
void insert_into_new_root(int table_id,NodePage* left, uint64_t key, NodePage* right);
int get_left_index(int table_id,InternalPage* parent, off_t left_offset);
void insert_into_node(int table_id,InternalPage * parent, int left_index, uint64_t key,
        off_t right_offset);
void insert_into_node_after_splitting(int table_id,InternalPage* parent, int left_index,
        uint64_t key, off_t right_offset);
int insert_record(int table_id,uint64_t key, const char* value);

// Deletion.
int get_neighbor_index(int table_id,NodePage* node_page);
void adjust_root(int table_id);
void coalesce_nodes(int table_id,NodePage* node_page, NodePage* neighbor_page,
                      int neighbor_index, int k_prime);
void redistribute_nodes(int table_id,NodePage* node_page, NodePage* neighbor_page,
                          int neighbor_index,
                          int k_prime_index, int k_prime);
void delete_entry(int table_id,NodePage* node_page, uint64_t key);
int delete_record(int table_id,uint64_t key);

#endif /* __BPT_INTERNAL_H__ */
