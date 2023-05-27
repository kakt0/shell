#ifndef TREE_H
#define TREE_H

#include "list.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>

enum type_of_next{
    NXT=1, AND=2, OR=3   // виды связей соседних команд в списке команд
};

struct cmd_inf {
    char ** argv; // список из имени команды и аргументов
    char *infile; // переназначенный файл стандартного ввода
    char *outfile; // переназначенный файл стандартного вывода
    int append; //=1, если запись в конец файла
    int backgrnd; // =1, если команда подлежит выполнению в фоновом режиме
    struct cmd_inf* pipe; // следующая команда после "|"
    struct cmd_inf* next; // следующая после ";" (или после "&")
};

typedef struct cmd_inf *tree;
typedef struct cmd_inf node1;

typedef enum {begin, conv, conv1, in, in1, out, out1, out2, out3, beckgrnd, end} vertex1;

void print_tree(tree, int);
tree build_tree(list );
void clear_tree(tree );
void inv(void);
#endif
