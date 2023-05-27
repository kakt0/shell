#include "exec.h"

#define sizeOfCmd 128

/* Error in execution of commands */
static void errorConv(tree tr, char *str, int *pid){
	fprintf(stderr, "ERROR: command '%s' not found\n", str);
	clear_tree(tr);
	free(pid);
	pid=NULL;
	exit(-1);
}

/* Pipe can't be created */
static void errorConvPipe(tree tr){
	clear_tree(tr);
	fprintf(stderr, "%s", "ERROR: cannot create pipe\n");
	longjmp(start,1);
}

/* File does not exist/open */
static void errorConvFile(tree tr, char *str, int *pid){
	fprintf(stderr, "ERROR: file '%s' cannot be open\n", str);
	clear_tree(tr);
	free(pid);
	pid=NULL;
	exit(-1);
}

/* Directory does not exist/open */
static void errorCd(tree tr, char *str){
	fprintf(stderr, "ERROR: directory '%s' cannot be found\n", str);
	clear_tree(tr);
	longjmp(start,1);
}

/* Redirect to output from file */
static void fRes(tree t,char *str, int append, int *fres, int *pid){
	if(append!=1){ 
		if((*fres=open(str, O_WRONLY|O_TRUNC|O_CREAT, 0666))==-1) errorConvFile(t, str, pid);
	}
	else if((*fres=open(str,O_WRONLY|O_APPEND|O_CREAT, 0666))==-1) errorConvFile(t, str, pid);
        dup2(*fres, 1);
        close(*fres);
}

/* Redirect to input from file */
static void fDat(tree t,char *str, int *fdat, int *pid){
	if((*fdat=open(str, O_RDONLY, 0666))==-1) errorConvFile(t, str, pid);
	dup2(*fdat, 0);
	close(*fdat);
}

/* Internal command check */
static int notCdExit(char *str){
	return strcmp(str, "cd")!=0&&strcmp(str, "exit")!=0;
}

/* Internal command realisation */
static void notPwd(tree t, char **str){
	if(strcmp(str[0], "exit")==0){
		clear_tree(t);
		exit(0);
	}
	if(strcmp(str[0], "cd")==0){
		if (str[1] == NULL){
			chdir(getenv("HOME"));
		}
		else{
			if(chdir(str[1])!=0){
				errorCd(t, str[1]);
			}
		}
	}
}

/* Not enough memory */
static void errorMem(){
	fprintf(stderr, "%s", "ERROR: not enough memory\n");
	longjmp(start,1);
}

/* Realloc with verification */
static int *reall(int *pid, int size){
	int *buf;
	buf=realloc(pid, size);
	if(buf!=NULL) pid=buf;
	else errorMem();
	return pid;
}

/* Close standard input */
static void bckgr(int b, int fnull){
	if(b==1){
                fnull=open("/dev/null", 0666);
                dup2(fnull, 0);
        	close(fnull);
        }
}

/* Pipeline: creating processes and executing commands */
static int *convExec(tree t, tree tr, int *pid){
	int fd[2], in, out, next_in, i, fres, fdat, argc=0, fnull;
	if(tr==NULL) return NULL;
	if(tr->pipe==NULL){
		if(!notCdExit(tr->argv[0])) notPwd(t, tr->argv);
		else{ 
			pid=reall(pid, sizeof(int)*2);
			pid[1]=-1;
			if((pid[0]=fork())==-1) errorConv(t, tr->argv[0], pid);
			else if(pid[0]==0){
				if(tr->backgrnd!=1)signal(SIGINT, SIG_DFL);
				if(tr->outfile!=NULL) fRes(t, tr->outfile, tr->append, &fres, pid);
				bckgr(tr->backgrnd, fnull);
				if(tr->infile!=NULL) fDat(t, tr->infile, &fdat, pid);
				execvp(*tr->argv, tr->argv);
				errorConv(t, tr->argv[0], pid);
			}
		}
		if(tr->backgrnd!=1) wait(NULL);
	}
	else{
		if(pipe(fd)==-1) errorConvPipe(t);
		out=fd[1]; 
		next_in=fd[0];
		if(notCdExit(tr->argv[0])){
			if((pid[argc]=fork())==-1) errorConv(t, tr->argv[0], pid);
			else if(pid[argc]==0){
				if(tr->backgrnd!=1)signal(SIGINT, SIG_DFL);
				close(next_in);
				dup2(out,1);
				close(out);
				bckgr(tr->backgrnd, fnull);
				if(tr->infile!=NULL) fDat(t, tr->infile, &fdat, pid);
				if(tr->outfile!=NULL) fRes(t, tr->outfile, tr->append, &fres, pid);
				execvp(*tr->argv, tr->argv);
				errorConv(t, tr->argv[0], pid);
			}
			argc++;
			in=next_in;
		}
		tr=tr->pipe;
		while(tr!=NULL&&tr->pipe!=NULL){
			close(out);
			if(pipe(fd)==-1) errorConvPipe(t);
			out=fd[1];
			next_in=fd[0];
			if(notCdExit(tr->argv[0])){
				pid=reall(pid, sizeof(int)*argc+sizeof(int));
				if((pid[argc]=fork())==-1) errorConv(t, tr->argv[0], pid);
				else if(pid[argc]==0){
					if(tr->backgrnd!=1)signal(SIGINT, SIG_DFL);
					close(next_in);
					dup2(in,0);
					close(in);
					dup2(out,1);
					close(out);
					if(tr->infile!=NULL) fDat(t, tr->infile, &fdat, pid);
                                	if(tr->outfile!=NULL) fRes(t, tr->outfile, tr->append, &fres, pid);
					execvp(*tr->argv, tr->argv);
					errorConv(t, tr->argv[0], pid);
				}
				close(in);
				in=next_in;
				argc++;
			}
			tr=tr->pipe;
		}
		close(out);
		if(notCdExit(tr->argv[0])){
			pid=reall(pid, sizeof(int)*argc+sizeof(int));	
			if((pid[argc]=fork())==-1) errorConv(t, tr->argv[0], pid);
			else if(pid[argc]==0){
				if(tr->backgrnd!=1)signal(SIGINT, SIG_DFL);
				if(tr->outfile!=NULL) fRes(t, tr->outfile, tr->append, &fres, pid);
				dup2(in,0);
				close(in);
				if(tr->infile!=NULL) fDat(t, tr->infile, &fdat, pid);
				execvp(*tr->argv, tr->argv);
				errorConv(t, tr->argv[0], pid);
			}
			argc++;
		}
		pid=reall(pid, sizeof(pid)*argc+sizeof(int));
		pid[argc]=-1;
		close(in);
		if(tr->backgrnd!=1){ 
			for(i=1; i<=argc; i++){ 
				wait(NULL);
			}
		}
	}
	return pid;	
}

/* Terminating background processes */
static void waitPrBack(int *pid1, int *pid, int *pred_back, char *prev_cmd){
	int status;
	pid=pid1;
        if(pid1!=NULL&&*pred_back==1){
                pid=pid1;
                while(*pid1!=-1){
                        waitpid(*pid1, &status, 0);
                        pid1++;
                }
                printf("%d: process '%s..' ended\n", pid[0], prev_cmd);
                *pred_back=-1;
                free(pid);
                pid=NULL;
                pid1=pid;
        }
}

/* Terminating not background processes */
static void waitPr(int *pid1, int *pid, int *pred_back, char *prev_cmd){
        pid=pid1;
	if(pid1!=NULL&&*pred_back!=1){
                pid=pid1;
                while(*pid1!=-1){
                        wait(NULL);
                        pid1++;
                }
                free(pid);
                pid=NULL;
        }
}

/* Execution of pipelines, finishing background processes */
void nextExec(tree tr, int *flag){
	tree t=tr;
	static int *pid1=NULL, pred_back=-1;
	static int *pid=NULL;
	static char prev_cmd[sizeOfCmd];
	waitPrBack(pid1, pid, &pred_back, prev_cmd);
	if(tr==NULL){
		pred_back=-1;
		return;
	}
	while(tr!=NULL){
		if(notCdExit(tr->argv[0])){
			pid1=malloc(sizeof(int));
			*pid1=-1;
		}
		pid1=convExec(t, tr, pid1);
		pid=pid1;
		pred_back=tr->backgrnd;
		strcpy(prev_cmd, tr->argv[0]);
		if(notCdExit(tr->argv[0])) waitPr(pid1, pid, &pred_back, prev_cmd);
		tr=tr->next;
	}
	if(tr==NULL&&*flag==1){
		waitPrBack(pid1, pid, &pred_back, prev_cmd);
        }
}

/*int main(){
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
		nextExec(tr, &flag);
		if(tr!=NULL) clear_tree(tr);
	}
	putchar('\n');
}*/

