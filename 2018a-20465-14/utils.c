/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "prototypes.h"
#include "assembler.h"
#include "ext_vars.h"
#include "utils.h"

/* This function extracts bits, given start and end positions of the bit-sequence (0 is LSB) */
unsigned int extract_bits(unsigned int word, int start, int end)
{
    unsigned int result;
    int length = end - start + 1; /* Length of bit-sequence */
    unsigned int mask = (int) pow(2, length) - 1; /* Creating a '111...1' mask with above line's length */

    mask <<= start; /* Moving mask to place of extraction */
    result = word & mask; /* The bits are now in their original position, and the rest is 0's */
    result >>= start; /* Moving the sequence to LSB */
    return result;
}

/* Converting a word to 2 digits in base 32 (as a string) */
char *convert_to_base_32(unsigned int num)
{
    char *base32_seq = (char *) malloc(BASE32_SEQUENCE_LENGTH);

    /* To convert from binary to base 32 we can just take the 5 right binary digits and 5 left */
    base32_seq[0] = base32[extract_bits(num, 5, 9)];
    base32_seq[1] = base32[extract_bits(num, 0, 4)];
    base32_seq[2] = '\0';

    return base32_seq;
}

/* This function checks if a string is a number (all digits) */
boolean is_number(char *seq)
{
    if(end_of_line(seq)) return FALSE;
    if(*seq == '+' || *seq == '-') /* a number can contain a plus or minus sign */
    {
        seq++;
        if(!isdigit(*seq++)) return FALSE; /* but not only a sign */
    }

    /* Check that the rest of the token is made of digits */
    while(!end_of_line(seq))
    {
        if(!isdigit(*seq++)) return FALSE;
    }
    return TRUE;
}

/* This function checks if a given sequence is a valid string (wrapped with "") */
boolean is_string(char *string)
{
    if(string == NULL) return FALSE;

    if (*string == '"') /* starts with " */
        string++;
    else
        return FALSE;

    while (*string && *string != '"') { /* Goes until end of string */
        string++;
    }

    if (*string != '"') /* a string must end with " */
        return FALSE;

    string++;
    if (*string != '\0') /* string token must end after the ending " */
        return FALSE;

    return TRUE;
}

/* This function inserts given A/R/E 2 bits into given info bit-sequence (the info is being shifted left) */
unsigned int insert_are(unsigned int info, int are)
{
    return (info << BITS_IN_ARE) | are; /* OR operand allows insertion of the 2 bits because 1 + 0 = 1 */
}

/* This function creates a file name by appending suitable extension (by type) to the original string */
char *create_file_name(char *original, int type)
{
    char *modified = (char *) malloc(strlen(original) + MAX_EXTENSION_LENGTH);
    if(modified == NULL)
    {
        fprintf(stderr, "Dynamic allocation error.");
        exit(ERROR);
    }

    strcpy(modified, original); /* Copying original filename to the bigger string */

    /* Concatenating the required file extension */

    switch (type)
    {
        case FILE_INPUT:
            strcat(modified, ".as");
            break;

        case FILE_OBJECT:
            strcat(modified, ".ob");
            break;

        case FILE_ENTRY:
            strcat(modified, ".ent");
            break;

        case FILE_EXTERN:
            strcat(modified, ".ext");

    }
    return modified;
}

/* This function inserts a given word to instructions memory */
void encode_to_instructions(unsigned int word)
{
    instructions[ic++] = word;
}

/* This functions returns 1 if there's an error (AKA: global variable err has changed) */
int is_error()
{
    return err != NO_ERROR;
}

/* This function receives line number as a parameter and prints a detailed error message
   accordingly to the error global variable */
void write_error(int line_num)
{
    fprintf(stderr, "ERROR (line %d): ", line_num);

    switch (err)
    {
        case SYNTAX_ERR:
            fprintf(stderr, "first non-blank character must be a letter or a dot.\n");

            break;

        case LABEL_ALREADY_EXISTS:
            fprintf(stderr, "label already exists.\n");

            break;

        case LABEL_TOO_LONG:
            fprintf(stderr, "label is too long (LABEL_MAX_LENGTH: %d).\n", LABEL_LENGTH);

            break;

        case LABEL_INVALID_FIRST_CHAR:
            fprintf(stderr, "label must start with an alphanumeric character.\n");

            break;

        case LABEL_ONLY_ALPHANUMERIC:
            fprintf(stderr, "label must only contain alphanumeric characters.\n");

            break;

        case LABEL_CANT_BE_COMMAND:
            fprintf(stderr, "label can't have the same name as a command.\n");

            break;

        case LABEL_CANT_BE_REGISTER:
            fprintf(stderr, "label can't have the same name as a register.\n");

            break;

        case LABEL_ONLY:
            fprintf(stderr, "label must be followed by a command or a directive.\n");

            break;

        case DIRECTIVE_NO_PARAMS:
            fprintf(stderr, "directive must have parameters.\n");

            break;

        case DIRECTIVE_INVALID_NUM_PARAMS:
            fprintf(stderr, "illegal number of parameters for a directive.\n");

            break;

        case DATA_COMMAS_IN_A_ROW:
            fprintf(stderr, "incorrect usage of commas in a .data directive.\n");

            break;

        case DATA_EXPECTED_NUM:
            fprintf(stderr, ".data expected a numeric parameter.\n");

            break;

        case DATA_EXPECTED_COMMA_AFTER_NUM:
            fprintf(stderr, ".data expected a comma after a numeric parameter.\n");

            break;

        case DATA_UNEXPECTED_COMMA:
            fprintf(stderr, ".data got an unexpected comma after the last number.\n");

            break;

        case STRING_TOO_MANY_OPERANDS:
            fprintf(stderr, ".string must contain exactly one parameter.\n");

            break;

        case STRING_OPERAND_NOT_VALID:
            fprintf(stderr, ".string operand is invalid.\n");

            break;

        case STRUCT_INVALID_NUM:
            fprintf(stderr, ".struct first parameter must be a number.\n");

            break;

        case STRUCT_EXPECTED_STRING:
            fprintf(stderr, ".struct must have 2 parameters.\n");

            break;

        case STRUCT_INVALID_STRING:
            fprintf(stderr, ".struct second parameter is not a string.\n");

            break;

        case STRUCT_TOO_MANY_OPERANDS:
            fprintf(stderr, ".struct must not have more than 2 operands.\n");

            break;

        case EXPECTED_COMMA_BETWEEN_OPERANDS:
            fprintf(stderr, ".struct must have 2 operands with a comma between them.\n");

            break;

        case EXTERN_NO_LABEL:
            fprintf(stderr, ".extern directive must be followed by a label.\n");

            break;

        case EXTERN_INVALID_LABEL:
            fprintf(stderr, ".extern directive received an invalid label.\n");

            break;

        case EXTERN_TOO_MANY_OPERANDS:
            fprintf(stderr, ".extern must only have one operand that is a label.\n");

            break;

        case COMMAND_NOT_FOUND:
            fprintf(stderr, "invalid command or directive.\n");

            break;

        case COMMAND_UNEXPECTED_CHAR:
            fprintf(stderr, "invalid syntax of a command.\n");

            break;

        case COMMAND_TOO_MANY_OPERANDS:
            fprintf(stderr, "command can't have more than 2 operands.\n");

            break;

        case COMMAND_INVALID_METHOD:
            fprintf(stderr, "operand has invalid addressing method.\n");

            break;

        case COMMAND_INVALID_NUMBER_OF_OPERANDS:
            fprintf(stderr, "number of operands does not match command requirements.\n");

            break;

        case COMMAND_INVALID_OPERANDS_METHODS:
            fprintf(stderr, "operands' addressing methods do not match command requirements.\n");

            break;

        case ENTRY_LABEL_DOES_NOT_EXIST:
            fprintf(stderr, ".entry directive must be followed by an existing label.\n");

            break;

        case ENTRY_CANT_BE_EXTERN:
            fprintf(stderr, ".entry can't apply to a label that was defined as external.\n");

            break;

        case COMMAND_LABEL_DOES_NOT_EXIST:
            fprintf(stderr, "label does not exist.\n");

            break;

        case CANNOT_OPEN_FILE:
            fprintf(stderr, "there was an error while trying to open the requested file.\n");
    }
}

/* This function copies the next token of a list (comma separated e.x. 1, "abc", 4) to a dest array.
 * Returns a pointer to the first character after the token
 */
char *next_list_token(char *dest, char *line)
{
    char *temp = dest;

    if(end_of_line(line)) /* If the given line is empty, copy an empty token */
    {
        dest[0] = '\0';
        return NULL;
    }

    if(isspace(*line)) /* If there are spaces in the beginning of the token, skip them */
        line = skip_spaces(line);

    if(*line == ',') /* A comma deserves a separate, single-character token */
    {
        strcpy(dest, ",");
        return ++line;
    }

    /* Manually copying token until a ',', whitespace or end of line */
    while(!end_of_line(line) && *line != ',' && !isspace(*line))
    {
        *temp = *line;
        temp++;
        line++;
    }
    *temp = '\0';

    return line;
}

/* This function copies supposedly next string into dest array and returning a pointer to the
 * first character after it
 */
char *next_token_string(char *dest, char *line)
{
    char temp[LINE_LENGTH];
    line = next_list_token(dest, line);
    if(*dest != '"') return line;
    while(!end_of_line(line) && dest[strlen(dest) - 1] != '"')
    {
        line = next_list_token(temp, line);
        if(line) strcat(dest, temp);
    }
    return line;
}

/* Checking for the end of line/given token in the character that char* points to */
int end_of_line(char *line)
{
    return line == NULL || *line == '\0' || *line == '\n';
}

/* This function returns a pointer to the start of next token in the line */
char *next_token(char *seq)
{
    if(seq == NULL) return NULL;
    while(!isspace(*seq) && !end_of_line(seq)) seq++; /* Skip rest of characters in the current token (until a space) */
    seq = skip_spaces(seq); /* Skip spaces */
    if(end_of_line(seq)) return NULL;
    return seq;
}

/* This function copies the next token (until a space or end of line) to a destination string */
void copy_token(char *dest, char *line)
{
    int i = 0;
    if(dest == NULL || line == NULL) return;

    while(i < LINE_LENGTH && !isspace(line[i]) && line[i] != '\0') /* Copying token until its end to *dest */
    {
        dest[i] = line[i];
        i++;
    }
    dest[i] = '\0';
}

/* This function finds an index of a string in an array of strings */
int find_index(char *token, const char *arr[], int n)
{
    int i;
    for(i = 0; i < n; i++)
        if (strcmp(token, arr[i]) == 0)
            return i;
    return NOT_FOUND;
}

/* Check if a token matches a register name */
boolean is_register(char *token)
{
    /* A register must have 2 characters, the first is 'r' and the second is a number between 0-7 */
    return strlen(token) == REGISTER_LENGTH && token[0] == 'r' &&
            token[1] >= '0' &&
            token[1] <= '7';
}

/* Check if a token matches a directive name */
int find_directive(char *token)
{
    if(token == NULL || *token != '.') return NOT_FOUND;
    return find_index(token, directives, NUM_DIRECTIVES);
}

/* Check if a token matches a command name */
int find_command(char *token)
{
    int token_len = strlen(token);
    if(token_len > MAX_COMMAND_LENGTH || token_len < MIN_COMMAND_LENGTH)
        return NOT_FOUND;
    return find_index(token, commands, NUM_COMMANDS);
}

/* This function skips spaces of a string and returns a pointer to the first non-blank character */
char *skip_spaces(char *ch)
{
    if(ch == NULL) return NULL;
    while (isspace(*ch)) /* Continue the loop if the character is a whitespace */
        ch++;
    return ch;
}

/* Function that ignores a line if it's blank/a comment */
int ignore(char *line)
{
    line = skip_spaces(line);
    return *line == ';' || *line == '\0' || *line == '\n';
}
