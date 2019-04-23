/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

/*
	This file contains prototypes for utilities and helper functions that are being used in the program.
*/

#include "structs.h"

/* Helper functions that are used for parsing tokens and navigating through them */
char *next_token_string(char *dest, char *line);
char *next_list_token(char *dest, char *line);
char *next_token(char *seq);
char *skip_spaces(char *ch);
void copy_token(char *dest, char *line);
int end_of_line(char *line);
int ignore(char *line);

/* Helper functions that are used to determine types of tokens */
int find_index(char *token, const char *arr[], int n);
int find_command(char *token);
int find_directive(char *token);
boolean is_string(char *string);
boolean is_number(char *seq);
boolean is_register(char *token);

/* Helper functions that are used for creating files and assigning required extensions to them */
char *create_file_name(char *original, int type);
FILE *open_file(char *filename, int type);
char *convert_to_base_32(unsigned int num);

/* Functions of external labels positions' linked list */
extPtr add_ext(extPtr *hptr, char *name, unsigned int reference);
void free_ext(extPtr *hptr);
void print_ext(extPtr h);

/* Functions of symbols table */
labelPtr add_label(labelPtr *hptr, char *name, unsigned int address, boolean external, ...);
int delete_label(labelPtr *hptr, char *name);
void free_labels(labelPtr *hptr);
void offset_addresses(labelPtr label, int num, boolean is_data);
unsigned int get_label_address(labelPtr h, char *name);
labelPtr get_label(labelPtr h, char *name);
boolean is_existing_label(labelPtr h, char *name);
boolean is_external_label(labelPtr h, char *name);
int make_entry(labelPtr h, char *name);
void print_labels(labelPtr h);

/* Functions that handle errors */
void write_error(int line_num); /* This function is called when an error output is needed */
int is_error();

/* Helper functions for encoding and building words */
unsigned int extract_bits(unsigned int word, int start, int end);
void encode_to_instructions(unsigned int word);
unsigned int insert_are(unsigned int info, int are);