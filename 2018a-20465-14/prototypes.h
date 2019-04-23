/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

#ifndef PROTOTYPES_H

#define PROTOTYPES_H
#include "structs.h"

void first_pass(FILE *fp);
void second_pass(FILE *fp, char *filename);

/*------------------ First Pass Functions ------------------*/

unsigned int build_first_word(int type, int is_first, int is_second, int first_method, int second_method);
int calculate_command_num_additional_words(int is_first, int is_second, int first_method, int second_method);
boolean command_accept_methods(int type, int first_method, int second_method);
boolean command_accept_num_operands(int type, boolean first, boolean second);
int detect_method(char * operand);
int handle_command(int type, char *line);
int handle_data_directive(char *line);
int handle_directive(int type, char *line);
int handle_extern_directive(char *line);
int handle_string_directive(char *line);
int handle_struct_directive(char *line);
boolean is_label(char *token, int colon);
int num_words(int method);
void read_line(char *line);
void write_num_to_data(int num);
void write_string_to_data(char *str);

/*------------------ Second Pass Functions ------------------*/

unsigned int build_register_word(boolean is_dest, char *reg);
void check_operands_exist(int type, boolean *is_src, boolean *is_dest);
int encode_additional_words(char *src, char *dest, boolean is_src, boolean is_dest, int src_method, int dest_method);
void encode_additional_word(boolean is_dest, int method, char *operand);
void encode_label(char *label);
int handle_command_second_pass(int type, char *line);
void read_line_second_pass(char *line);
void write_output_entry(FILE *fp);
void write_output_extern(FILE *fp);
int write_output_files(char *original);
void write_output_ob(FILE *fp);

#endif
