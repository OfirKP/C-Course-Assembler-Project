/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

/*--------- Headers ---------*/
#include "ext_vars.h"
#include "prototypes.h"
#include "utils.h"

void second_pass(FILE *fp, char *filename)
{
    char line[LINE_LENGTH]; /* This string will contain each line at a time */
    int line_num = 1; /* Line numbers start from 1 */

    ic = 0; /* Initializing global instructions counter */

    while(fgets(line, LINE_LENGTH, fp) != NULL) /* Read lines until end of file */
    {
        err = NO_ERROR;
        if(!ignore(line)) /* Ignore line if it's blank or ; */
            read_line_second_pass(line); /* Analyze one line at a time */
        if(is_error()) { /* If there was an error in the current line */
            was_error = TRUE; /* There was at least one error through all the program */
            write_error(line_num);
        }
        line_num++;
    }
    if(!was_error) /* Write output files only if there weren't any errors in the program */
        write_output_files(filename);

    /* Free dynamic allocated elements */
    free_labels(&symbols_table);
    free_ext(&ext_list);
}

/* This function analyzes and extracts information needed for the second pass from a given line */
void read_line_second_pass(char *line)
{
    int dir_type, command_type;
    char current_token[LINE_LENGTH]; /* will hold current token as needed */
    line = skip_spaces(line); /* Proceeding to first non-blank character */
    if(end_of_line(line)) return; /* a blank line is not an error */

    copy_token(current_token, line);
    if(is_label(current_token, COLON)) { /* If it's a label, skip it */
        line = next_token(line);
        copy_token(current_token, line);
    }

    if((dir_type = find_directive(current_token)) != NOT_FOUND) /* We need to handle only .entry directive */
    {
        line = next_token(line);
        if(dir_type == ENTRY)
        {
            copy_token(current_token, line);
            make_entry(symbols_table, current_token); /* Creating an entry for the symbol */
        }
    }

    else if ((command_type = find_command(current_token)) != NOT_FOUND) /* Encoding command's additional words */
    {
        line = next_token(line);
        handle_command_second_pass(command_type, line);
    }
}

/* This function writes all 3 output files (if they should be created)*/
int write_output_files(char *original)
{
    FILE *file;

    file = open_file(original, FILE_OBJECT);
    write_output_ob(file);

    if(entry_exists) {
        file = open_file(original, FILE_ENTRY);
        write_output_entry(file);
    }

    if(extern_exists)
    {
        file = open_file(original, FILE_EXTERN);
        write_output_extern(file);
    }

    return NO_ERROR;
}

/* This function writes the .ob file output.
 * The first line is the size of each memory (instructions and data).
 * Rest of the lines are: address in the first column, word in memory in the second.
 */
void write_output_ob(FILE *fp)
{
    unsigned int address = MEMORY_START;
    int i;
    char *param1 = convert_to_base_32(ic), *param2 = convert_to_base_32(dc);

    fprintf(fp, "%s\t%s\n\n", param1, param2); /* First line */
    free(param1);
    free(param2);

    for (i = 0; i < ic; address++, i++) /* Instructions memory */
    {
        param1 = convert_to_base_32(address);
        param2 = convert_to_base_32(instructions[i]);

        fprintf(fp, "%s\t%s\n", param1, param2);

        free(param1);
        free(param2);
    }

    for (i = 0; i < dc; address++, i++) /* Data memory */
    {
        param1 = convert_to_base_32(address);
        param2 = convert_to_base_32(data[i]);

        fprintf(fp, "%s\t%s\n", param1, param2);

        free(param1);
        free(param2);
    }

    fclose(fp);
}

/* This function writes the output of the .ent file.
 * First column: name of label.
 * Second column: address of definition.
 */
void write_output_entry(FILE *fp)
{
    char *base32_address;

    labelPtr label = symbols_table;
    /* Go through symbols table and print only symbols that have an entry */
    while(label)
    {
        if(label -> entry)
        {
            base32_address = convert_to_base_32(label -> address);
            fprintf(fp, "%s\t%s\n", label -> name, base32_address);
            free(base32_address);
        }
        label = label -> next;
    }
    fclose(fp);
}

/* This function writes the output of the .ext file.
 * First column: label name.
 * Second column: address where the external label should be replaced.
 */
void write_output_extern(FILE *fp)
{
    char *base32_address;
    extPtr node = ext_list;

    /* Going through external circular linked list and pulling out values */
    do
    {
        base32_address = convert_to_base_32(node -> address);
        fprintf(fp, "%s\t%s\n", node -> name, base32_address); /* Printing to file */
        free(base32_address);
        node = node -> next;
    } while(node != ext_list);
    fclose(fp);
}

/* This function opens a file with writing permissions, given the original input filename and the
 * wanted file extension (by type)
 */
FILE *open_file(char *filename, int type)
{
    FILE *file;
    filename = create_file_name(filename, type); /* Creating filename with extension */

    file = fopen(filename, "w"); /* Opening file with permissions */
    free(filename); /* Allocated modified filename is no longer needed */

    if(file == NULL)
    {
        err = CANNOT_OPEN_FILE;
        return NULL;
    }
    return file;
}

/* This function determines if source and destination operands exist by opcode */
void check_operands_exist(int type, boolean *is_src, boolean *is_dest)
{
    switch (type)
    {
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            *is_src = TRUE;
            *is_dest = TRUE;
            break;

        case NOT:
        case CLR:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case RED:
        case PRN:
        case JSR:
            *is_src = FALSE;
            *is_dest = TRUE;
            break;

        case RTS:
        case STOP:
            *is_src = FALSE;
            *is_dest = FALSE;
    }
}

/* This function handles commands for the second pass - encoding additional words */
int handle_command_second_pass(int type, char *line)
{
    char first_op[LINE_LENGTH], second_op[LINE_LENGTH]; /* will hold first and second operands */
    char *src = first_op, *dest = second_op; /* after the check below, src will point to source and
 *                                              dest to destination operands */
    boolean is_src = FALSE, is_dest = FALSE; /* Source/destination operands existence */
    int src_method = METHOD_UNKNOWN, dest_method = METHOD_UNKNOWN; /* Their addressing methods */

    check_operands_exist(type, &is_src, &is_dest);

    /* Extracting source and destination addressing methods */
    if(is_src)
        src_method = extract_bits(instructions[ic], SRC_METHOD_START_POS, SRC_METHOD_END_POS);
    if(is_dest)
        dest_method = extract_bits(instructions[ic], DEST_METHOD_START_POS, DEST_METHOD_END_POS);

    /* Matching src and dest pointers to the correct operands (first or second or both) */
    if(is_src || is_dest)
    {
        line = next_list_token(first_op, line);
        if(is_src && is_dest) /* There are 2 operands */
        {
            line = next_list_token(second_op, line);
            next_list_token(second_op, line);
        }
        else
        {
            dest = first_op; /* If there's only one operand, it's a destination operand */
            src = NULL;
        }
    }

    ic++; /* The first word of the command was already encoded in this IC in the first pass */
    return encode_additional_words(src, dest, is_src, is_dest, src_method, dest_method);
}

/* This function encodes the additional words of the operands to instructions memory */
int encode_additional_words(char *src, char *dest, boolean is_src, boolean is_dest, int src_method,
                                int dest_method) {
    /* There's a special case where 2 register operands share the same additional word */
    if(is_src && is_dest && src_method == METHOD_REGISTER && dest_method == METHOD_REGISTER)
    {
        encode_to_instructions(build_register_word(FALSE, src) | build_register_word(TRUE, dest));
    }
    else /* It's not the special case */
    {
        if(is_src) encode_additional_word(FALSE, src_method, src);
        if(is_dest) encode_additional_word(TRUE, dest_method, dest);
    }
    return is_error();
}

/* This function builds the additional word for a register operand */
unsigned int build_register_word(boolean is_dest, char *reg)
{
    unsigned int word = (unsigned int) atoi(reg + 1); /* Getting the register's number */
    /* Inserting it to the required bits (by source or destination operand) */
    if(!is_dest)
        word <<= BITS_IN_REGISTER;
    word = insert_are(word, ABSOLUTE);
    return word;
}

/* This function encodes a given label (by name) to memory */
void encode_label(char *label)
{
    unsigned int word; /* The word to be encoded */

    if(is_existing_label(symbols_table, label)) { /* If label exists */
        word = get_label_address(symbols_table, label); /* Getting label's address */

        if(is_external_label(symbols_table, label)) { /* If the label is an external one */
            /* Adding external label to external list (value should be replaced in this address) */
            add_ext(&ext_list, label, ic + MEMORY_START);
            word = insert_are(word, EXTERNAL);
        }
        else
            word = insert_are(word, RELOCATABLE); /* If it's not an external label, then it's relocatable */

        encode_to_instructions(word); /* Encode word to memory */
    }
    else /* It's an error */
    {
        ic++;
        err = COMMAND_LABEL_DOES_NOT_EXIST;
    }
}

/* This function encodes an additional word to instructions memory, given the addressing method */
void encode_additional_word(boolean is_dest, int method, char *operand)
{
    unsigned int word = EMPTY_WORD; /* An empty word */
    char *temp;

    switch (method)
    {
        case METHOD_IMMEDIATE: /* Extracting immediate number */
            word = (unsigned int) atoi(operand + 1);
            word = insert_are(word, ABSOLUTE);
            encode_to_instructions(word);
            break;

        case METHOD_DIRECT:
            encode_label(operand);
            break;

        case METHOD_STRUCT: /* Before the dot there should be a label, and after it a number */
            temp = strchr(operand, '.');
            *temp = '\0';
            encode_label(operand); /* Label before dot is the first additional word */
            *temp++ = '.';
            word = (unsigned int) atoi(temp);
            word = insert_are(word, ABSOLUTE);
            encode_to_instructions(word); /* The number after the dot is the second */
        break;

        case METHOD_REGISTER:
            word = build_register_word(is_dest, operand);
            encode_to_instructions(word);
    }
}
