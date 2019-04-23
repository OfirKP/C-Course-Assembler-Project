/*****************************************
	Ofir Krupik, Shay Yosipov
*****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"

/* This function adds a node to the end of the list */
extPtr add_ext(extPtr *hptr, char *name, unsigned int reference)
{
    extPtr t=*hptr;
    extPtr temp;

    temp=(extPtr) malloc(sizeof(ext));
    if(!temp)
    {
        printf("\nerror, cannot allocate memory\n");
        exit(1);
    }

    temp -> address = reference;
    strcpy(temp->name, name);

    if(!(*hptr)) /* If the list is empty */
    {
        *hptr = temp;
        temp -> next = temp;
        temp -> prev = temp;
        return temp;
    }


    ((*hptr)->prev)->next = temp;
    temp->next = t;
    temp->prev = t->prev;
    (*hptr)->prev = temp;

    return temp;
}

/* This function frees the allocated memory for the list */
void free_ext(extPtr *hptr)
{
    /* Frees the extern list by going over each extern node and free it*/
    extPtr temp=*hptr;
    if(temp) {
        unsigned int last_reference = ((*hptr)->prev)->address;
        unsigned int reference = 0;
        do {
            temp = *hptr;
            reference = temp->address;
            *hptr = (*hptr)->next;
            free(temp);
        } while (reference != last_reference);
    }
}

/* This function prints the ext list */
void print_ext(extPtr h)
{
    extPtr orig=h;
    do
    {
        printf("\nname: %s, reference: %d - >", h->name, h->address);
        h=h->next;
    } while(h -> address != orig -> address);
    printf("*\n");
}