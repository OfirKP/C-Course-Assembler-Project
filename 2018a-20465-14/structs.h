/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

/*
	This file contains definitions of types and structures that are being used in the program.
*/

#ifndef STRUCTS_H

#define STRUCTS_H

#include "assembler.h"

extern unsigned int data[]; /* Data array of words */
extern unsigned int instructions[]; /* Instructions array of words */

typedef enum {FALSE, TRUE} boolean; /* Defining a boolean type (it doesn't exist in ANSI C) */

/* Defining linked list of labels and a pointer to that list */
typedef struct structLabels * labelPtr;
typedef struct structLabels {
	char name[LABEL_LENGTH]; /* the name of the label */
	unsigned int address; /* the address of the label */
	boolean external; /* a boolean type variable to store if the label is extern or not */
	boolean inActionStatement; /* a boolean type varialbe to store if the label is in an action statement or not */
	boolean entry; /* a boolean type varialbe to store if the label is entry or not */
	labelPtr next; /* a pointer to the next label in the list */
} Labels;

/* Defining a circular double-linked list to store each time the program uses an extern label, and a pointer to that list */
typedef struct ext * extPtr;
typedef struct ext {
    char name[LABEL_LENGTH]; /* the name of the extern label */
    unsigned int address; /* the address in memory where the external address should be replaced */
    extPtr next; /* a pointer to the next extern in the list */
    extPtr prev; /* a pointer to the previous extern in the list */
} ext;

#endif
