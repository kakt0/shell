#ifndef LIST_H
#define LIST_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include<setjmp.h>

#define additionalSizeOfBuf 10
#define sizeOfStr 20
#define sizeOfBuf 1024

typedef struct Node *link1;
typedef char *elemtype;

typedef struct Node{
	        elemtype elem;
	        link1 next;
	}node;

typedef link1 list;

typedef enum {Start, Word, Greater, Greater2, Newline, Stop} vertex;

jmp_buf start;

/* Printing of list(input: list; output: none) */
void print(list p);

/* Removing of list(input: list; output: none) */
void del(list p);

/* Replacing four variables with their values(input: list; output: list) */
list sequenRep(list lst);

/* Creation of list with all input strings(input: none; output: list) */
list create(int *flag);

#endif
