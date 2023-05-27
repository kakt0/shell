#include "exec.h"

extern void print(list p);
extern void del(list p);
extern list sequenRep(list lst);
extern list create(int *fl);
extern void print_tree(tree tr, int i);
extern tree build_tree(list lst);
extern void clear_tree(tree tr);
extern void inv(void);
extern void nextExec(tree tr, int *flag);

int main(){
        list ls=NULL;
        tree tr=NULL;
        int flag=0;
        signal(SIGINT, SIG_IGN);
        setbuf(stdout, NULL);
        setjmp(start);
        if(ls!=NULL) del(ls);
        while(flag!=1){
                inv();
                ls=create(&flag);
                ls=sequenRep(ls);
                tr=build_tree(ls);
                if(ls!=NULL){ 
			del(ls);
			ls=NULL;
		}
                if(ls!=NULL) del(ls);  
                nextExec(tr, &flag);
                if(tr!=NULL) clear_tree(tr);
        }
        putchar('\n');
}
