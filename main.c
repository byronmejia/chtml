#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKTYPE_NONE	-1
#define BLOCKTYPE_HTML	0
#define BLOCKTYPE_C	1

/**
 * String block - Linked list element
 */
typedef struct block block;
struct block {
	int start;
	int end;
	int type;
	block *next_block;
};

block *new_block() {
	block * rblock = malloc(sizeof *rblock);
	rblock->start = -1;
	rblock->end = -1;
	rblock->type = BLOCKTYPE_NONE;
	rblock->next_block = NULL;
	return rblock;
}


int main(int argc, char *argv[]) {
	/**
	 * Usage
	 */
	if(argc < 2) {
		printf("Usage: %s <file>\n", argv[0]);
		exit(1);
	}

	/**
	 * Opening file
	 */
	FILE *fp;
	fp = fopen(argv[1], "r");
	if(fp == NULL) {
		fprintf(stderr, "Error opening file: %s\n", argv[1]);
		exit(1);
	}

	/**
	 * Getting blocks
	 */
	int pos = 0;
	int n;
	char *cur_char = malloc(sizeof *cur_char);
	char prev_chars[2] = {0, 0};

	block *start_block = new_block();
	block *cur_block = start_block;
	// Tag handling is simple, it's currently not worth-while to
	// deal with more complicated tag handling.

	cur_block->start = 0;
	cur_block->type = BLOCKTYPE_HTML;
	while(!feof(fp)) {
		n = fread(cur_char, 1, 1, fp);
		if(*cur_char == ':') {
			if(prev_chars[0] == '<' && prev_chars[1] == ':') {
				// Open tag
				cur_block->end = pos - 2;
				block *tmp = new_block();
				cur_block->next_block = tmp;
				cur_block = tmp;

				cur_block->start = pos + 1;
				cur_block->type = BLOCKTYPE_C;
			}
		}else if(*cur_char == '>') {
			if(prev_chars[0] == ':' && prev_chars[1] == ':') {
				// Close tag
				if(cur_block->start != -1) {
					cur_block->end = pos - 2;
					block *tmp = new_block();
					cur_block->next_block = tmp;
					cur_block = tmp;

					cur_block->start = pos + 1;
					cur_block->type = BLOCKTYPE_HTML;
				}
			}
		}
		prev_chars[0] = prev_chars[1];
		prev_chars[1] = *cur_char;
		pos++;
	}
	cur_block->end = pos - 2;

	rewind(fp);
	FILE *out = fopen("temp.c", "w");
	if(out == NULL) {
		fputs("Error opening temp file\n", stderr);
		// XXX exit ( and remove else )
	}else {
		fputs("#include <stdio.h>\nint main(int argc, char *argv[]){",
			out);
		char buffer[512];
		memset(buffer, 0, 512);
		cur_block = start_block;
		while(cur_block->next_block != NULL) {
			n = fread(buffer, 1,
					cur_block->end - cur_block->start, fp);
			printf("%s\n", buffer);

			if(cur_block->type == BLOCKTYPE_HTML) {
				fputs("puts(\"", out);
				for(int i = 0; i < strlen(buffer); i++) {
					if(buffer[i] == '\n') {
						fputs("\\n", out);
					}else {
						fputc(buffer[i], out);
					}
				}
				fputs("\");", out);
			}else {
				fputs(buffer, out);
			}
			n = fread(buffer, 1,
				cur_block->next_block->start - cur_block->end,
				fp);

			cur_block = cur_block->next_block;
			memset(buffer, 0, strlen(buffer));
		}
		n = fread(buffer, 1,
				cur_block->end - cur_block->start, fp);
		printf("%s\n", buffer);

		if(cur_block->type == BLOCKTYPE_HTML) {
			fputs("puts(\"", out);
			for(int i = 0; i < strlen(buffer); i++) {
				if(buffer[i] == '\n') {
					fputs("\\n", out);
				}else {
					fputc(buffer[i], out);
				}
			}
			fputs("\");", out);
		}else {
			fputs(buffer, out);
		}

		cur_block = cur_block->next_block;
		memset(buffer, 0, strlen(buffer));

		fputs("return 0;}", out);
		fclose(out);
	}
	
	cur_block = start_block;
	block *tmp;
	int count = 1;
	while(cur_block->next_block != NULL) {
		printf("Block %d: %d (%d -> %d)\n", count, cur_block->type,
					cur_block->start, cur_block->end);
		count++;
		tmp = cur_block;
		cur_block = cur_block->next_block;
		free(tmp);
	}
	printf("Block %d: %d (%d -> %d)\n", count, cur_block->type,
				cur_block->start, cur_block->end);
	fclose(fp);
	free(cur_char);
	free(cur_block);
	
	return 0;
}
