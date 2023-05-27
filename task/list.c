#include "list.h"

/* Printing of list(input: list; output: none) */
void print(list p){
	int j=0/*, numbElem=0*/;
	/*list cur=p;*/
	char *el;
	if(p==NULL)      
		return;
	while(p!=NULL){
		j=0;
		el=p->elem;
		while(el[j]!='\0'){
			 putchar(el[j]);
			 j++;
		}
		putchar('\n');
		p=p->next;
	}
	putchar('\n');
}

/* Removing of list(input: list; output: none) */
void del(list p){
	list cur=p;
	while(p!=NULL){
		cur=cur->next;
		free(p->elem);
		free(p);
		p=cur;
	}
}

/* Valid characters(input: char; output:  1-character is valid, 0-otherwise) */
/*static int chTest(int c){
	return  c=='\n' || c==' ' || c=='\t' || (c>='0'&&c<='9') || (c>='a'&&c<='z') || (c>='A'&&c<='Z') || c=='|' ||
		c=='/' || c=='_' || c=='.' || c=='&' || c==';' || c=='>' || c=='<' || c=='(' || c==')' || c==EOF || 
		c=='$' || c=='"' || c=='\\';
}*/

/* Reading from stdin, getting the current character(input: link to string; output: character in int format, so we can define EOF) */
static int getsymExam(char *st, int creat){
	static int curNum=0;
	static int n=0;
	if(n==0||creat==1){
		n=read(0, st, sizeOfStr);
		curNum=0;
	}
	curNum++; 
	return (--n>=0) ? st[curNum-1]:EOF;
}

/* Defining special characters(input: character in int format; output: 1-character is not special, 0-otherwise) */
static int symset(int c){
	return  
		c!=' ' &&
		c!='\t' &&
		c!='>' &&
		c!='<' &&
		c!='|' &&
		c!=';' &&
		c!='&' &&
		c!=')' &&
		c!='(';
}


/* Error handling associated with realloc */
static void error(){
	fprintf(stderr, "%s", "ERROR: not enough memory\n");
	longjmp(start,1);
	//exit(1);
}

/* Error handling associated with absence of closing quote */
static void errorQuot(){
	fprintf(stderr, "%s", "ERROR: no closing quote\n");
	longjmp(start,1);
	//exit(1);
}

/* Replacing four variables with their values */
list sequenRep(list lst){
	list p=lst;
	char buf[sizeOfBuf], /**buf1,*/ *tempBuf;
	int pw;
	struct passwd *log;
	while(p!=NULL){
		if(strcmp(p->elem, "$HOME")==0){
			sprintf(buf, "%s", getenv("HOME"));
			tempBuf=realloc(p->elem, sizeof(buf)+1);
			if(tempBuf!=NULL) p->elem=tempBuf;
			else error();
			strcpy(p->elem, buf);
		}
		else if(strcmp(p->elem, "$SHELL")==0){
			sprintf(buf, "%s", getenv("SHELL"));
			tempBuf=realloc(p->elem, sizeof(buf)+1);
			if(tempBuf!=NULL) p->elem=tempBuf;
			else error();
			strcpy(p->elem, buf);
		}
		else if(strcmp(p->elem, "$USER")==0){
			log=getpwuid(getuid());
			sprintf(buf, "%s", log->pw_name);
			tempBuf=realloc(p->elem, sizeof(buf)+1);
			if(tempBuf!=NULL) p->elem=tempBuf;
			else error();
			strcpy(p->elem, buf);
		}
		else if(strcmp(p->elem, "$EUID")==0){
			pw=geteuid();
			sprintf(buf, "%d", pw);
			tempBuf=realloc(p->elem, sizeof(buf)+1);
			if(tempBuf!=NULL) p->elem=tempBuf;
			else error();
			strcpy(p->elem, buf);
		}
		p=p->next;
	}	
	return lst;
}

/* Editing new element at the end of list(input: list, link to string; output: list) */
static list newElem(list ls, char *st){
	if(ls==NULL){
		ls=(link1)malloc(sizeof(node));
		if(ls==NULL) error();
		ls->elem=st;
		ls->next=NULL;
	}
	else
		ls->next=newElem(ls->next, st);
	return ls;
}

/* Read, creation, printing, sorting of all input strings(input: none; output: none) */
list create(int *flag){
	char str[sizeOfStr];
	char *buf=NULL;
	char *tempBuf;
	int curSym, temp='\n', quotes=0, slash=0;
	int i=-1; /* buffer counter */
	list lst=NULL;
	vertex V=Start;
	curSym=getsymExam(str, 1);
	while(1==1) switch(V){
		case Start:
			if((curSym==' '||curSym=='\t')&&quotes!=1) curSym=getsymExam(str, 0);
			else if (curSym==EOF||curSym=='\n'||(curSym=='#'&&slash!=1)){
				if(curSym==EOF) *flag=1;
				else *flag=0;
				return lst;
				V=Stop;
			}
			else{
				buf=NULL;
				i=-1;
				if(i+1>=sizeof(*buf)-1){ 
					tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
					if(tempBuf!=NULL) buf=tempBuf;
					else error();
				}
				if(curSym!='"'&&curSym!='\\'&&curSym!='#') buf[++i]=curSym;
				if((curSym=='>'||curSym=='&'||curSym=='|'||curSym=='<'||curSym=='('||curSym==')'||curSym==';')&&quotes!=1) V=Greater;
				else if(slash!=1&&curSym=='"'){ 
					V=Word;
					quotes++;
					if(quotes==2) quotes=0;
				}
				else if(temp=='\\'&&curSym=='"'&&slash==1){
					V=Word;
					slash=0;
				}
				else if(curSym=='\\'){
					slash++;
					if(slash==2) slash=0;
					V=Word;
				}
				else{ 
					slash=0;
					V=Word;
				}
				temp=curSym;
				curSym=getsymExam(str, 0);
			}
		break;
		case Word:
		 if(((curSym!=EOF)&&(curSym!='\n')&&((symset(curSym)==1)&&(quotes==0)&&(curSym!='#')))||((curSym=='#')&&(slash==1))||(quotes==1)||((symset(curSym)==0)&&(slash==1))){
				if(i+1>=sizeof(*buf)-1){
					tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
				        if(tempBuf!=NULL) buf=tempBuf;
					else error();
				}
				if(slash!=1&&curSym=='"')
				       	quotes++;
				else if(curSym!='\\'&&slash==1){ 
					slash=0;
					buf[++i]=curSym;
				}
				else if(curSym=='\\'){
					slash++;
					if(slash==2){
						buf[++i]=curSym;
					       	slash=0;
					}
				}
				else buf[++i]=curSym;
				temp=curSym;
				curSym=getsymExam(str, 0);
			}
			else{
				V=Start;
				if(curSym!='#') slash=0; 
				if(quotes==1){
					free(buf);
				       	errorQuot();
				}
				if(i>=sizeof(*buf)-1){
					tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
					if(tempBuf!=NULL) buf=tempBuf;
					else error();
				}
				buf[++i]='\0';
				tempBuf=realloc(buf, i+1);
				if(tempBuf!=NULL) buf=tempBuf;
				else error();
				if(strcmp(buf, "\0")!=0) lst=newElem(lst, buf);
				else free(buf);
				quotes=0;
			}
		break;
		case Greater:
			if(quotes!=1&&((curSym=='>'&&temp==curSym)||(curSym=='&'&&temp==curSym)||(curSym=='|'&&temp==curSym))){
				if(i>=sizeof(*buf)-1){
					tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
					if(tempBuf!=NULL) buf=tempBuf;
					else error();
				}
				buf[++i]=curSym;
				temp=curSym;
				curSym=getsymExam(str, 0);
				V=Greater2;
			}
			else{
				V=Start;
				if(i>=sizeof(*buf)-1){
					tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
					if(tempBuf!=NULL) buf=tempBuf;
					else error();
				}
				buf[++i]='\0';
				tempBuf=realloc(buf, i+1);
				if(tempBuf!=NULL) buf=tempBuf;
				else error();
				lst=newElem(lst, buf);
			}
		break;
		case Greater2:
			V=Start;
			if(i>=sizeof(*buf)-1){
				tempBuf=realloc(buf, i+1+additionalSizeOfBuf);
				if(tempBuf!=NULL) buf=tempBuf;
				else error();
			}
			buf[++i]='\0';
			tempBuf=realloc(buf, i+1);
			if(tempBuf!=NULL) buf=tempBuf;
			else error();
			lst=newElem(lst, buf);
		break;
		case Stop:
			//*flag=1;
			//putchar('\n');
			return NULL;
		break;
	}
}

