#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>
#include "bpt_internal.h"
#include "file.h"
#include "bpt.h"
// MAIN
int main( int argc, char ** argv ) {
	int buf_num;
	uint64_t input_key;
    char input_value[SIZE_VALUE] = { 0,};
	char instruction;
	char table_name[FILENAME_MAX_LENGTH + 1] = { 0,};
	int table_id;

	if(argc != 2) {
		fprintf(stderr, 
				"Wrong execution of DB\n"
				"Execute DB correctly like\n"
				"./database [num_buf_blocks]\n");
		return FAIL;
	}

    license_notice();
	usage_1();  
	usage_2();

	init_db(atoi(argv[1]));

	printf("> ");
	while (scanf("%c", &instruction) != EOF) {
		switch (instruction) {
		case 'o':	// open table
			scanf("%s", table_name);
			open_table(table_name);
			break;
		case 'c':	// close table
			scanf("%s", table_name);
			table_id = find_table_id(table_name);
			if(table_id != FAIL)
				close_table(table_id);
			break;
		case 'i':
			scanf("%s %" PRIu64 " %s", table_name, &input_key, input_value);
			table_id = find_table_id(table_name);
			if(table_id != FAIL){
				insert(table_id, input_key, input_value);
				print_tree(table_id);
			printf("key: %jd\n",input_key);
			}
			break;
        case 'd':
			scanf("%s %" PRIu64 "", table_name, &input_key);
			table_id = find_table_id(table_name);
			if(table_id != FAIL) {
				delete(table_id, input_key);
				print_tree(table_id);
			}
			break;
		case 'f':
		case 'p':
			scanf("%s %" PRIu64 "", table_name, &input_key);
			table_id = find_table_id(table_name);
			if(table_id != FAIL)
				find_and_print(table_id, input_key);
			break;
		case 'q':
			while (getchar() != (int)'\n');
			goto exit;
		case 't':
			scanf("%s", table_name);
			table_id = find_table_id(table_name);
			if(table_id != FAIL)
				print_tree(table_id);
			break;
		case 'b':
			printf("bufmgr\n"
					"alloc_buf_num: %d\n"
					"capaicty: %d\n",
					buffermgr.buf_used, buffermgr.buf_order);
			Buffer* buf = buffermgr.firstBuf;
			for(int i = 0; i < buffermgr.buf_used; i++){
				buf = buf->nextB;
			}
			if(buf == NULL){
				puts("correct!");
			}
			break;
        default:
			usage_2();
			break;
		}
		memset(table_name, 0, FILENAME_MAX_LENGTH + 1);
		while (getchar() != (int)'\n');
		printf("> ");
	}
	printf("\n");

exit:
    shutdown_db();
	return EXIT_SUCCESS;
}
