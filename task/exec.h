#ifndef EXEC_H
#define EXEC_H

#include "tree.h"
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void nextExec(tree tr, int *flag);

#endif
