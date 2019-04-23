/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*--------- Headers ---------*/
#include "ext_vars.h"
#include "prototypes.h"
#include "utils.h"

/* This function manages all the activities of the first pass */
void first_pass(FILE *fp)
{
    char line[LINE_LENGTH]; /* This string will contain each line at a time */
    int line_num = 1; /* Line numbers start from 1 */

    /* Initializing data and instructions counter */
    ic = 0;
    dc = 0;

    while(fgets(line, LINE_LENGTH, fp) != NULL) /* Read lines until end of file */
    {
        err = NO_ERROR; /* Reset the error global var before parsing each line */
        if(!ignore(line)) /* Ignore line if it's blank or ; */
            read_line(line);
        if(is_error()) {
            was_error = TRUE; /* There was at least one error through all the program */
            write_error(line_num); /* Output the error */
        }
        line_num++;
    }
    
    /* When the first pass ends and the symbols table is complete and IC is evaluated,
       we can calculate real final addresses */
    offset_addresses(symbols_table, MEMORY_START, FALSE); /* Instruction symbols will have addresses that start from 100 (MEMORY_START) */
    offset_addresses(symbols_table, ic + MEMORY_START, TRUE); /* Data symbols will have addresses that start fron NENORY_START + IC */
}

/* This function will analyze a given line from the file and will extract the information needed by the Maman's rules */
void read_line(char *line)
{
    /* Initializing variables for the type of the directive/command */
    int dir_type = UNKNOWN_TYPE;
    int command_type = UNKNOWN_COMMAND;

    boolean label = FALSE; /* This variable will hold TRUE if a label exists in this line */
    labelPtr label_node = NULL; /* This variable holds optional label in case we create it */
    char current_token[LINE_LENGTH]; /* This string will hold the current token if we analyze it */
    
    line = skip_spaces(line); /* skips to the next non-blank/whitepsace character */
    if(end_of_line(line)) return; /* a blank line is not an error */
    if(!isalpha(*line) && *line != '.') { /* first non-blank character must be a letter or a dot */
        err = SYNTAX_ERR;
        return;
    }

    copy_token(current_token, line); /* Assuming that label is separated from other tokens by a whitespace */
    if(is_label(current_token, COLON)) { /* We check if the first token is a label (and it should contain a colon) */
        label = TRUE;
        label_node = add_label(&symbols_table, current_token, 0, FALSE, FALSE); /* adding label to the global symbols table */
        if(label_node == NULL) /* There was an error creating label */
            return;
        line = next_token(line); /* Skipping to beginning of next token */
        if(end_of_line(line))
        {
            err = LABEL_ONLY; /* A line can't be label-only */
            return;
        }
        copy_token(current_token, line); /* Proceed to next token */
    } /* If there's a label error then exit this function */

    if(is_error()) /* is_label might return an error */
        return;

    if((dir_type = find_directive(current_token)) != NOT_FOUND) /* detecting directive type (if it's a directive) */
    {
        if(label)
        {
            if(dir_type == EXTERN || dir_type == ENTRY) { /* we need to ignore creation of label before .entry/.extern */
                delete_label(&symbols_table, label_node->name);
                label = FALSE;
            }
            else
                label_node -> address = dc; /* Address of data label is dc */
        }
        line = next_token(line);
        handle_directive(dir_type, line);
    }

    else if ((command_type = find_command(current_token)) != NOT_FOUND) /* detecting command type (if it's a command) */
    {
        if(label)
        {
            /* Setting fields accordingly in label */
            label_node -> inActionStatement = TRUE; 
            label_node -> address = ic;
        }
        line = next_token(line);
        handle_command(command_type, line);
    }

    else
    {
        err = COMMAND_NOT_FOUND; /* a line must have a directive/command */
    }

}

/* This function handles all kinds of directives (.data, .string, .struct, .entry, .extern)
 * and sends them accordingly to the suitable function for analyzing them
 * */
int handle_directive(int type, char *line)
{
    if(line == NULL || end_of_line(line)) /* All directives must have at least one parameter */
    {
        err = DIRECTIVE_NO_PARAMS;
        return ERROR;
    }

    switch (type)
    {
        case DATA:
            /* Handle .data directive and insert values separated by comma to the memory */
            return handle_data_directive(line);

        case STRING:
            /* Handle .string directive and insert all characters (including a '\0') to memory */
            return handle_string_directive(line);

        case STRUCT:
            /* Handle .struct directive and insert both number and string to memory */
            return handle_struct_directive(line);

        case ENTRY:
            /* Only check for syntax of entry (should not contain more than one parameter) */
            if(!end_of_line(next_token(line))) /* If there's a next token (after the first one) */
            {
                err = DIRECTIVE_INVALID_NUM_PARAMS;
                return ERROR;
            }
            break;

        case EXTERN:
            /* Handle .extern directive */
            return handle_extern_directive(line);
    }
    return NO_ERROR;
}

/* This function analyzes a command, given the type (mov/jmp/etc...) and the sequence of
 characters starting after the command.

 It will detect the addressing methods of the operands and will encode the first word of
  the command to the instructions memory. */
int handle_command(int type, char *line)
{
    boolean is_first = FALSE, is_second = FALSE; /* These booleans will tell which of the operands were
                                                     received (not by source/dest, but by order) */
    int first_method, second_method; /* These will hold the addressing methods of the operands */
    char first_op[20], second_op[20]; /* These strings will hold the operands */

    /* Trying to parse 2 operands */
    line = next_list_token(first_op, line);
    if(!end_of_line(first_op)) /* If first operand is not empty */
    {
        is_first = TRUE; /* First operand exists! */
        line = next_list_token(second_op, line);
        if(!end_of_line(second_op)) /* If second operand (should hold temporarily a comma) is not empty */
        {
            if(second_op[0] != ',') /* A comma must separate two operands of a command */
            {
                err = COMMAND_UNEXPECTED_CHAR;
                return ERROR;
            }

            else
            {
                line = next_list_token(second_op, line); 
                if(end_of_line(second_op)) /* If second operand is not empty */
                {
                    err = COMMAND_UNEXPECTED_CHAR;
                    return ERROR;
                }
                is_second = TRUE; /* Second operand exists! */
            }
        }
    }
    line = skip_spaces(line);
    if(!end_of_line(line)) /* If the line continues after two operands */
    {
        err = COMMAND_TOO_MANY_OPERANDS;
        return ERROR;
    }

    if(is_first)
        first_method = detect_method(first_op); /* Detect addressing method of first operand */
    if(is_second)
        second_method = detect_method(second_op); /* Detect addressing method of second operand */

    if(!is_error()) /* If there was no error while trying to parse addressing methods */
    {
        if(command_accept_num_operands(type, is_first, is_second)) /* If number of operands is valid for this specific command */
        {
            if(command_accept_methods(type, first_method, second_method)) /* If addressing methods are valid for this specific command */
            {
                /* encode first word of the command to memory and increase ic by the number of additional words */
                encode_to_instructions(build_first_word(type, is_first, is_second, first_method, second_method));
                ic += calculate_command_num_additional_words(is_first, is_second, first_method, second_method);
            }

            else
            {
                err = COMMAND_INVALID_OPERANDS_METHODS;
                return ERROR;
            }
        }
        else
        {
            err = COMMAND_INVALID_NUMBER_OF_OPERANDS;
            return ERROR;
        }
    }

    return NO_ERROR;
}


/* This function handles a .string directive by analyzing it and encoding it to data */
int handle_string_directive(char *line)
{
    char token[LINE_LENGTH];

    /*line = next_list_token(token, line);
    line = next_token(line);*/
    line = next_token_string(token, line);
    if(!end_of_line(token) && is_string(token)) { /* If token exists and it's a valid string */
        line = skip_spaces(line);
        if(end_of_line(line)) /* If there's no additional token */
        {
            /* "Cutting" quotation marks and encoding it to data */
            token[strlen(token) - 1] = '\0';
            write_string_to_data(token + 1);
        }

        else /* There's another token */
        {
            err = STRING_TOO_MANY_OPERANDS;
            return ERROR;
        }

    }

    else /* Invalid string */
    {
        err = STRING_OPERAND_NOT_VALID;
        return ERROR;
    }

    return NO_ERROR;
}

/* This function handles analyzing and encoding a .struct directive to data memory,
 * given the char sequence starting from after the ".struct"
*/
int handle_struct_directive(char *line)
{
    char token[LINE_LENGTH];
    line = next_list_token(token, line); /* Getting the firs token into token array in the line above */

    if(!end_of_line(token) && is_number(token)) /* First token must be a number */
    {
        write_num_to_data(atoi(token)); /* Encode number to data */
        line = next_list_token(token, line); /* Get next token */

        if(!end_of_line(token) && *token == ',') { /* There must be a comma between .struct operands */
            line = next_token_string(token, line); /* Get next token (second operand) */
            if(!end_of_line(token)) { /* There's a second operand */
                if (is_string(token)) {
                    /* Encode valid string by "cutting" the "" and sending it to the encoding function */
                    token[strlen(token) - 1] = '\0';
                    write_string_to_data(token + 1);
                } else {
                    err = STRUCT_INVALID_STRING;
                    return ERROR;
                }
            }
            else /* There is no second operand */
            {
                err = STRUCT_EXPECTED_STRING;
                return ERROR;
            }
        }
        else
        {
            err = EXPECTED_COMMA_BETWEEN_OPERANDS;
            return ERROR;
        }
    }
    else
    {
        err = STRUCT_INVALID_NUM;
        return ERROR;
    }
    if(!end_of_line(next_list_token(token, line)))
    {
        err = STRUCT_TOO_MANY_OPERANDS;
        return ERROR;
    }
    return NO_ERROR;
}

/* This function parses parameters of a data directive and encodes them to memory */
int handle_data_directive(char *line)
{
    char token[20]; /* Holds tokens */

    /* These booleans mark if there was a number or a comma before current token,
     * so that if there wasn't a number, then a number will be required and
     * if there was a number but not a comma, a comma will be required */
    boolean number = FALSE, comma = FALSE;

    while(!end_of_line(line))
    {
        line = next_list_token(token, line); /* Getting current token */

        if(strlen(token) > 0) /* Not an empty token */
        {
            if (!number) { /* if there wasn't a number before */
                if (!is_number(token)) { /* then the token must be a number */
                    err = DATA_EXPECTED_NUM;
                    return ERROR;
                }

                else {
                    number = TRUE; /* A valid number was inputted */
                    comma = FALSE; /* Resetting comma (now it is needed) */
                    write_num_to_data(atoi(token)); /* encoding number to data */
                }
            }

            else if (*token != ',') /* If there was a number, now a comma is needed */
            {
                err = DATA_EXPECTED_COMMA_AFTER_NUM;
                return ERROR;
            }

            else /* If there was a comma, it should be only once (comma should be false) */
            {
                if(comma) {
                    err = DATA_COMMAS_IN_A_ROW;
                    return ERROR;
                }
                else {
                    comma = TRUE;
                    number = FALSE;
                }
            }

        }
    }
    if(comma == TRUE)
    {
        err = DATA_UNEXPECTED_COMMA;
        return ERROR;
    }
    return NO_ERROR;
}

/* This function encodes a given number to data */
void write_num_to_data(int num)
{
    data[dc++] = (unsigned int) num;
}

/* This function encodes a given string to data */
void write_string_to_data(char *str)
{
    while(!end_of_line(str))
    {
        data[dc++] = (unsigned int) *str; /* Inserting a character to data array */
        str++;
    }
    data[dc++] = '\0'; /* Insert a null character to data */
}

/* This function tries to find the addressing method of a given operand and returns -1 if it was not found */
int detect_method(char * operand)
{
    char *struct_field; /* When determining if it's a .struct directive, this will hold the part after the dot */

    if(end_of_line(operand)) return NOT_FOUND;

    /*----- Immediate addressing method check -----*/
    if (*operand == '#') { /* First character is '#' */
        operand++;
        if (is_number(operand))
            return METHOD_IMMEDIATE;
    }

    /*----- Register addressing method check -----*/
    else if (is_register(operand))
        return METHOD_REGISTER;

    /*----- Direct addressing method check ----- */
    else if (is_label(operand, FALSE)) /* Checking if it's a label when there shouldn't be a colon (:) at the end */
        return METHOD_DIRECT;

    /*----- Struct addressing method check -----*/
    else if (is_label(strtok(operand, "."), FALSE)) { /* Splitting by dot character */
        struct_field = strtok(NULL, ""); /* Getting the rest of the string */
        if (strlen(struct_field) == 1 && (*struct_field == '1' || /* After the dot there should be '1' or '2' */
                *struct_field == '2'))
            return METHOD_STRUCT;
    }
    err = COMMAND_INVALID_METHOD;
    return NOT_FOUND;
}

/* This function checks for the validity of given addressing methods according to the opcode */
boolean command_accept_methods(int type, int first_method, int second_method)
{
    switch (type)
    {
        /* These opcodes only accept
         * Source: 0, 1, 2, 3
         * Destination: 1, 2, 3
         */
        case MOV:
        case ADD:
        case SUB:
            return (first_method == METHOD_IMMEDIATE ||
                   first_method == METHOD_DIRECT ||
                   first_method == METHOD_STRUCT ||
                   first_method == METHOD_REGISTER)
                &&
                   (second_method == METHOD_DIRECT ||
                    second_method == METHOD_STRUCT ||
                    second_method == METHOD_REGISTER);

        /* LEA opcode only accept
         * Source: 1, 2
         * Destination: 1, 2, 3
        */
        case LEA:
            return (first_method == METHOD_DIRECT ||
                    first_method == METHOD_STRUCT)
                   &&
                   (second_method == METHOD_DIRECT ||
                    second_method == METHOD_STRUCT ||
                    second_method == METHOD_REGISTER);

        /* These opcodes only accept
         * Source: NONE
         * Destination: 1, 2, 3
        */
        case NOT:
        case CLR:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case RED:
        case JSR:
            return first_method == METHOD_DIRECT ||
                   first_method == METHOD_STRUCT ||
                   first_method == METHOD_REGISTER;

        /* These opcodes are always ok because they accept all methods/none of them and
         * number of operands is being verified in another function
        */
        case PRN:
        case CMP:
        case RTS:
        case STOP:
            return TRUE;
    }

    return FALSE;
}

/* This function checks for the validity of given methods according to the opcode */
boolean command_accept_num_operands(int type, boolean first, boolean second)
{
    switch (type)
    {
        /* These opcodes must receive 2 operands */
        case MOV:
        case CMP:
        case ADD:
        case SUB:
        case LEA:
            return first && second;

        /* These opcodes must only receive 1 operand */
        case NOT:
        case CLR:
        case INC:
        case DEC:
        case JMP:
        case BNE:
        case RED:
        case PRN:
        case JSR:
            return first && !second;

        /* These opcodes can't have any operand */
        case RTS:
        case STOP:
            return !first && !second;
    }
    return FALSE;
}

/* This function calculates number of additional words for a command */
int calculate_command_num_additional_words(int is_first, int is_second, int first_method, int second_method)
{
    int count = 0;
    if(is_first) count += num_words(first_method);
    if(is_second) count += num_words(second_method);

    /* If both methods are register, they will share the same additional word */
    if(is_first && is_second && first_method == METHOD_REGISTER && second_method == METHOD_REGISTER) count--;

    return count;
}

/* This function encodes the first word of the command */
unsigned int build_first_word(int type, int is_first, int is_second, int first_method, int second_method)
{
    unsigned int word = 0;

    /* Inserting the opcode */
    word = type;

    word <<= BITS_IN_METHOD; /* Leave space for first addressing method */

    /* If there are two operands, insert the first */
    if(is_first && is_second)
        word |= first_method;

    word <<= BITS_IN_METHOD; /* Leave space for second addressing method */

    /* If there are two operands, insert the second. */
    if(is_first && is_second)
        word |= second_method;
    /* If not, insert the first one (a single operand is a destination operand). */
    else if(is_first)
        word |= first_method;

    word = insert_are(word, ABSOLUTE); /* Insert A/R/E mode to the word */

    return word;
}

/* This function returns how many additional words an addressing method requires */
int num_words(int method)
{
    if(method == METHOD_STRUCT) /* Struct addressing method requires two additional words */
        return 2;
    return 1;
}

/* This function handles an .extern directive */
int handle_extern_directive(char *line)
{
    char token[LABEL_LENGTH]; /* This will hold the required label */

    copy_token(token, line); /* Getting the next token */
    if(end_of_line(token)) /* If the token is empty, then there's no label */
    {
        err = EXTERN_NO_LABEL;
        return ERROR;
    }
    if(!is_label(token, FALSE)) /* The token should be a label (without a colon) */
    {
        err = EXTERN_INVALID_LABEL;
        return ERROR;  
    }  

    line = next_token(line);
    if(!end_of_line(line))
    {
        err = EXTERN_TOO_MANY_OPERANDS;
        return ERROR;
    }

    /* Trying to add the label to the symbols table */
    if(add_label(&symbols_table, token, EXTERNAL_DEFAULT_ADDRESS, TRUE) == NULL)
        return ERROR;
    return is_error(); /* Error code might be 1 if there was an error in is_label() */
}

/* This function checks whether a given token is a label or not (by syntax).
 * The parameter colon states whether the function should look for a ':' or not
 * when parsing parameter (to make it easier for both kinds of tokens passed to this function.
 */
boolean is_label(char *token, int colon)
{
    boolean has_digits = FALSE; /* If there are digits inside the label, we can easily skip checking if
                                   it's a command name. */
    int token_len = strlen(token);
    int i;

    /* Checking if token's length is not too short */
    if(token == NULL ||
            token_len < (colon ? MINIMUM_LABEL_LENGTH_WITH_COLON: MINIMUM_LABEL_LENGTH_WITHOUT_COLON))
        return FALSE;

    if(colon && token[token_len - 1] != ':') return FALSE; /* if colon = TRUE, there must be a colon at the end */

    if (token_len > LABEL_LENGTH) {
        if(colon) err = LABEL_TOO_LONG; /* It's an error only if we search for a label definition */
        return FALSE;
    }
    if(!isalpha(*token)) { /* First character must be a letter */
        if(colon) err = LABEL_INVALID_FIRST_CHAR;
        return FALSE;
    }

    if (colon) {
        token[token_len - 1] = '\0'; /* The following part is more convenient without a colon */
        token_len--;
    }
	
    /* Check if all characters are digits or letters */
    for(i = 1; i < token_len; i++) /* We have already checked if the first character is ok */
    {
        if(isdigit(token[i]))
            has_digits = TRUE;
        else if(!isalpha(token[i])) {
            /* It's not a label but it's an error only if someone put a colon at the end of the token */
            if(colon) err = LABEL_ONLY_ALPHANUMERIC;
            return FALSE;
        }
    }

    if(!has_digits) /* It can't be a command */
    {
        if (find_command(token) != NOT_FOUND) {
            if(colon) err = LABEL_CANT_BE_COMMAND; /* Label can't have the same name as a command */
            return FALSE;
        }
    }

    if(is_register(token)) /* Final obstacle: it's a label only if it's not a register */
    {
        if(colon) err = LABEL_CANT_BE_REGISTER;
        return FALSE;
    }

    return TRUE;
}
