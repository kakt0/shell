#include "tree.h"

/* Printing of list(input: list; output: none) */
extern void print(list p);

/* Removing of list(input: list; output: none) */
extern void del(list p);

/* Replacing four variables with their values(input: list; output: list) */
extern list sequenRep(list lst);

/* Creation of list with all input strings(input: none; output: list) */
extern list create(int *fl);

static void make_shift(int n){
	while(n--)
		putc(' ', stderr);
}

static void print_argv(char **p, int shift){
	char **q=p;
	if(p!=NULL){
		while(*p!=NULL){
			make_shift(shift);
			fprintf(stderr, "argv[%d]=%s\n",(int) (p-q), *p);
			p++;
		}
	}
}

void print_tree(tree t, int shift){
	char **p;
	if(t==NULL)
		return;
	p=t->argv;
	if(p!=NULL)
		print_argv(p, shift);
	else{
		make_shift(shift);
		fprintf(stderr, "psubshell\n");
	}
	make_shift(shift);
	if(t->infile==NULL)
		fprintf(stderr, "infile=NULL\n");
	else
		fprintf(stderr, "infile=%s\n", t->infile);
	make_shift(shift);
	if(t->outfile==NULL)
		fprintf(stderr, "outfile=NULL\n");
	else
		fprintf(stderr, "outfile=%s\n", t->outfile);
	make_shift(shift);
	fprintf(stderr, "append=%d\n", t->append);
	make_shift(shift);
	fprintf(stderr, "background=%d\n", t->backgrnd);
	make_shift(shift);
	if(t->pipe==NULL)
		fprintf(stderr, "pipe=NULL \n");
	else{
		fprintf(stderr, "pipe---> \n");
		print_tree(t->pipe, shift+5);
	}
	make_shift(shift);
	if(t->next==NULL)
		fprintf(stderr, "next=NULL \n");
	else{
		fprintf(stderr, "next---> \n");
		print_tree(t->next, shift+5);
	}
}

/* Create new tree */
static tree make_cmd(){
	tree tr;
	tr=malloc(sizeof(node1));
	tr->argv=NULL;	
	tr->infile=NULL;
	tr->outfile=NULL;
	tr->append=0;
	tr->backgrnd=0;
	tr->pipe=NULL;
	tr->next=NULL;
	return tr;
}

static void error(){
	fprintf(stderr, "%s", "ERROR: wrong format\n");
	longjmp(start,1);
}

/* Conveyor in the background */
static void make_bgrnd(tree t){
	while(t->next!=NULL) t=t->next;
	while(t!=NULL){
		t->backgrnd=1;
		t=t->pipe;
	}
}

/* Output in the end of file */
static void make_append(tree t){
	t->append=1;
}

static void errorMem(){
	fprintf(stderr, "%s", "ERROR: not enough memory\n");
	longjmp(start,1);
}

/* Deleting tree and clearing memory */
void clear_tree(tree tr){
	int i=0;
	if(tr==NULL) return;
	if(tr->argv!=NULL){
		for(i=0; tr->argv[i]!=NULL; i++)
			free(tr->argv[i]);
	}
	free(tr->argv);
	tr->argv=NULL;
	if(tr->infile!=NULL) free(tr->infile);
	tr->infile=NULL;
	if(tr->outfile!=NULL) free(tr->outfile);
	tr->outfile=NULL;
	if(tr->pipe!=NULL) clear_tree(tr->pipe);
	if(tr->next!=NULL) clear_tree(tr->next);
	free(tr);
	tr=NULL;
}

/* True, if word is not unavailable special character */
static int neSpecSlov(char *c){
	return strcmp(c, "&&")!=0&&strcmp(c, "(")!=0&&strcmp(c, ")")!=0&&strcmp(c, "||")!=0;
}

/* Add new argument to process */
static tree add_arg(tree tr, char *c, int i){
	char **buf;
	int len=snprintf(NULL, 0, "%s", c);;
	buf=(char**)realloc(tr->argv, (i+1)*sizeof(tr->argv)+sizeof(char*));
	if(buf!=NULL) tr->argv=buf;
	else errorMem();
	if(c!=NULL){
		tr->argv[i]=(char*)malloc(len+1);
		snprintf(tr->argv[i], len+1, "%s", c);
		//tr->argv[i][len]='\0';
	}
	else tr->argv[i]=NULL;
	return tr;
}

/* True, if word is not a special character */
static int cmd_check(char *t){
	return strcmp(t, ";")!=0&&strcmp(t, ">>")!=0&&strcmp(t, ">")!=0&&strcmp(t, "|")!=0&&strcmp(t, "<")!=0&&strcmp(t, "&")!=0; 
}

/* Add next process */
static void add_next(tree t, tree add){
	while(t->next!=NULL) t=t->next;
	t->next=add;
}

tree build_tree(list lst){
	list p=lst;
	vertex1 V=begin;
	int i=0;
	tree beg_cmd=NULL;
	tree cur_cmd;
	tree prev_cmd;
	while(1==1){
		switch(V){
		  case begin:
			  i=0;
			  if(p==NULL){
				V=end;
			  }
			  else if(cmd_check(p->elem)&&neSpecSlov(p->elem)){
				  if(beg_cmd==NULL) beg_cmd=cur_cmd=make_cmd();
				  else{
					cur_cmd=make_cmd();
					add_next(beg_cmd, cur_cmd);
					prev_cmd=cur_cmd;
				  }
				  if(cur_cmd->argv==NULL){
					 cur_cmd->argv=(char**)malloc(sizeof(char**));
				  }
				  cur_cmd=add_arg(cur_cmd, p->elem, i);
				  i++;
				  prev_cmd=cur_cmd;			  
				  V=conv;
			  }
			  else{ 
				  clear_tree(beg_cmd);	  
				  error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case conv:
			  if(p==NULL){
				  cur_cmd=add_arg(cur_cmd, NULL, i);
				  V=end;
			  }
			  else if(cmd_check(p->elem)&&neSpecSlov(p->elem)){
				  cur_cmd=add_arg(cur_cmd, p->elem, i);
				  i++;
				  V=conv;
			  }
			  else if(strcmp(p->elem, "|")==0){ 
				  cur_cmd=add_arg(cur_cmd, NULL, i);
				  i=0;
				  V=conv1;
			  }
			  else if(strcmp(p->elem, "<")==0){ 
				  V=in;
			  }
			  else if(strcmp(p->elem, ">")==0){ 
				  V=out;
			  }
			  else if(strcmp(p->elem, ">>")==0){
				  V=out2;
			  }
			  else if(strcmp(p->elem, "&")==0){ 
				  cur_cmd=add_arg(cur_cmd, NULL, i);
				  V=beckgrnd;
			  }
			  else if(strcmp(p->elem, ";")==0){
				cur_cmd=add_arg(cur_cmd, NULL, i);
				V=begin;
			  }
			  else{ 
				 cur_cmd=add_arg(cur_cmd, NULL, i);
				 clear_tree(beg_cmd);
				 error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case conv1:
			  if(p!=NULL&&cmd_check(p->elem)&&neSpecSlov(p->elem)){
				cur_cmd=make_cmd();
				if(cur_cmd->argv==NULL){
					cur_cmd->argv=(char**)malloc(sizeof(char**));
				}
				cur_cmd=add_arg(cur_cmd, p->elem, i);
				i++;
				prev_cmd->pipe=cur_cmd;
				prev_cmd=cur_cmd;
				V=conv;
			  }
			  else{ 
				  clear_tree(beg_cmd);
				  error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case in:
			  if(p!=NULL&&cmd_check(p->elem)&&neSpecSlov(p->elem)){
				if(p->elem!=NULL){
					cur_cmd->infile=(char*)malloc(strlen(p->elem)+1);
					strcpy(cur_cmd->infile,p->elem);
				}
				else cur_cmd->infile=NULL;
				V=in1;
			  }
			  else{
				  cur_cmd=add_arg(cur_cmd, NULL, i);
				  clear_tree(beg_cmd);
				  error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case in1:
			  if(p!=NULL&&strcmp(p->elem, ">")==0){
				  V=out;
				  if(p!=NULL) p=p->next;
			  }
			  else{ 
				  V=conv;
			  }
			  break;
		  case out:
			  if(p!=NULL&&cmd_check(p->elem)&&neSpecSlov(p->elem)){
				if(p->elem!=NULL){
					cur_cmd->outfile=(char*)malloc(strlen(p->elem)+1);
					strcpy(cur_cmd->outfile,p->elem);
				}
				else cur_cmd->outfile=NULL;
				V=out1;
			  }
			  else{
				  cur_cmd=add_arg(cur_cmd, NULL, i);
				  clear_tree(beg_cmd);
				  error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case out1:
			  V=conv;
			  break;
		  case out2:
			  make_append(cur_cmd);
			  if(p!=NULL&&cmd_check(p->elem)&&neSpecSlov(p->elem)){
				if(p->elem!=NULL){
					cur_cmd->outfile=(char*)malloc(strlen(p->elem)+1);
					strcpy(cur_cmd->outfile,p->elem);
				}
				V=out3;
			  }
			  else{ 
				  clear_tree(beg_cmd);
				  error();
			  }
			  if(p!=NULL) p=p->next;
			  break;
		  case out3:
			  V=conv;
			  break;
		  case beckgrnd:
			  cur_cmd=add_arg(cur_cmd, NULL, i);
			  if(p==NULL){
			  	V=end;
				make_bgrnd(beg_cmd);	
			  }
			  else{ 
				  clear_tree(beg_cmd);
				  error();
			  }
			  break;
		  case end:
			  return beg_cmd;
			  break;
		}
	}	
}

/* Prompt to enter */
void inv(){
	printf("%s", "\x1b[32m"); /*здесь изменяется цвет на зеленый */
	char s[100]; /* ограничение: имя хоста и текущей директории не должно быть слишком длинным! */
	gethostname(s, 100);
	printf("%s@%s", getenv("USER"), s);
	printf("%s", "\x1B[37m"); /* здесь изменяется цвет на серый */
	getcwd(s, 100);
	printf(":%s$ ", s);
}

/*int main(void){
	list ls=NULL;
	tree tr=NULL;
	int flag=0;
	setbuf(stdout, NULL);
	setjmp(start);
	del(ls);
	while(flag!=1){
		inv();
		ls=create(&flag);
		ls=sequenRep(ls);
		tr=build_tree(ls);
		del(ls);
		print_tree(tr, 5);
		clear_tree(tr);
	}
}*/
