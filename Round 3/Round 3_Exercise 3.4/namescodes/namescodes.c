/* Old code by WH, comments translated to English 2021. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "error.h"


/*#include "auxfunc.h"*/



/* Transforms attribute names to numeical cdoes and vice versa. */
/* Note: If you create a new conversion table, the input can contain only */
/* attribute abels. If you reuse an existing table, input can already */
/* contain numerical codes (they are not encoded again). */
/* syntax: namescodes -(n or c)<input file> -t<table file> [-L] */
/* where -n tells the input contains numbers and  -c that codes. */
/* If -L given, creates a new codetable. */

void luoKoodit(FILE *f1,FILE *f2,FILE *f3);
int etsikoodi(char **t,int clkm,char *seur);
int lisaakoodi(char ***t,int *clkm,char *seur, int *size);
char **kasvatataulua(char **t,int *size);
void muunnanimet(FILE *f1,FILE *f2,FILE *f3);
void muunnakoodit(FILE *f1,FILE *f2,FILE *f3);
char **luekoodit(FILE *f,int *clkm);
int kokluku(char *s,int *v);
int nofrows(FILE *f);

  
/***************************************************************************/
int main(int argc,char**argv){
  FILE *f1=NULL;FILE *f2=NULL;FILE *f3=NULL;
  int nimia=0;
  int luo=0;
  char *fnimi1=NULL;
  char *fnimi2=NULL;
  char *fnimi3=NULL;
  int opt;

  if (argc<3)
    error(0);

 while ((opt=getopt(argc,argv,"t:n:c:L"))!=-1){
    switch(opt){
    case 't': printf("Code table file %s\n",optarg);
      fnimi2=(char*)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(fnimi2,optarg);
      break;
    case 'n': printf("Input file with names%s\n",optarg);
      nimia=1; fnimi1=(char*)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(fnimi1,optarg);
      if ((f1=fopen(optarg,"r"))==NULL)
	error(1);     
      break;
    case 'c': printf("Input file with number codes %s\n",optarg);
      nimia=0; /* default */
      fnimi1=(char*)malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(fnimi1,optarg);
      if ((f1=fopen(optarg,"r"))==NULL)
	error(1);     
      break;
    case 'L': printf("Creates a new conversion table\n");
      luo=1;
      break;
    default: 
      printf("Unknown option %s\n",optarg);
      error(0);
    }
 }

 if ((luo)&&(nimia==0)){
   printf("If a new table is created, the input must contain attribute names.\n");
   error(0);
 }

 /* the output file suffix tells the type */
 fnimi3=malloc((strlen(fnimi1)+7)*sizeof(char));
 if (nimia)
   sprintf(fnimi3,"%s.codes",fnimi1);
 else sprintf(fnimi3,"%s.names",fnimi1);
 printf("Output file %s\n",fnimi3);
 if ((f3=fopen(fnimi3,"w"))==NULL)
   error(1);     

 /* now names */
 if (luo){ 
   if ((f2=fopen(fnimi2,"w"))==NULL)
     error(1);        
   luoKoodit(f1,f2,f3);
 }
 else {
   if ((f2=fopen(fnimi2,"r"))==NULL)
     error(1);        
   if (nimia)
     muunnanimet(f1,f2,f3);
   else muunnakoodit(f1,f2,f3);
 }

 if(fclose(f1)==EOF)
    error(4);
 if(fclose(f2)==EOF)
    error(4);
 if(fclose(f3)==EOF)
    error(4);

 return EXIT_SUCCESS;
}


/***************************************************************************/

/* Read names from f1. If already in the table, transform and output to f3. */
/* Otherwise create a new code and output to f3. */
/* Finally write the table to f2.*/
void luoKoodit(FILE *f1,FILE *f2,FILE *f3){
  char **koodit;
  int size,next, clkm,code,n,i;
  char *row,*seur;
  int LLEN=50000;

  n=nofrows(f1); /* rows in input  */
  /* initial size for the code table */
 if ((koodit=(char**)malloc(1000*sizeof(char*)))==NULL)
      error(6);
 size=1000; clkm=0; /* no codes yet */

 if ((row=(char*)malloc(LLEN))==NULL) error(6); 

 next=0;
 while (next<n) {
   if (fgets(row, LLEN, f1)==NULL)
     error(5);
	
 if ((seur=strtok(row," "))!=NULL){
   if (seur[strlen(seur)-1]=='\n')
     seur[strlen(seur)-1]='\0';
   code=etsikoodi(koodit,clkm,seur);
   if (code==-1) /* not found */
     code=lisaakoodi(&koodit,&clkm,seur,&size);
   if (code!=-1) /* if added */
     fprintf(f3,"%d",code); /* write code */
 }
 while ((seur=strtok(NULL," "))!=NULL){
   if (seur[strlen(seur)-1]=='\n')
     seur[strlen(seur)-1]='\0';
   code=etsikoodi(koodit,clkm,seur);
   if (code==-1) /* not found */
     code=lisaakoodi(&koodit,&clkm,seur,&size);
   if (code!=-1) /* if added */
   fprintf(f3," %d",code); /* write code */
 }
 fprintf(f3,"\n");
 next++;
 } /* while */

 /* write code table f2 */
 for (i=0;i<clkm;i++)
   fprintf(f2,"%d %s\n",i,koodit[i]);

}

int etsikoodi(char **t,int clkm,char *seur){
  int i;

  for (i=0;i<clkm;i++)
    if (strcmp(seur,t[i])==0)
      return i;
  return -1; /* not found */
}

/* update clkm and possibly size */
int lisaakoodi(char ***t,int *clkm,char *seur, int *size){
  int len=strlen(seur)+1;

  /* if empty, don't add */
  if (strcmp(seur,"\0")==0)
    return -1;

  /* is enough space? if not, increase */
  if ((*clkm)>=(*size)){
    *t=kasvatataulua(*t,size);
  } 
  if (((*t)[*clkm]=(char*)malloc(len*sizeof(char)))==NULL)
   error(6);

  strcpy((*t)[*clkm],seur);
  (*clkm)++;
  return ((*clkm)-1);
}

/* double table size */
char **kasvatataulua(char **t,int *size){

  if ((t=(char**)realloc(t,2*(*size)*sizeof(char*)))==NULL) 
    error(6);
  (*size)*=2;
  return t;
}

/*code table in f2. Read input f1 and write transformed to f3.*/
void muunnanimet(FILE *f1,FILE *f2,FILE *f3){
  char **koodit;
  int clkm=0;
  int n,next,code;
  int LLEN=50000;
  char *seur,*row;

  koodit=luekoodit(f2,&clkm); /* clkm = largest used code number */
  
  /*  for (i=0;i<=clkm;i++)
    if (koodit[i]!=NULL)
    printf("%d %s\n",i,koodit[i]);*/

  n=nofrows(f1); /* input file size */

  if ((row=(char*)malloc(LLEN))==NULL){ 
    error(6);
  } 
  next=0;
  while (next<n) {
    if (fgets(row, LLEN, f1)==NULL){
      printf("Error in muunnanimet (data row %d)\n",next+1);
      error(5);
    }

    if ((seur=strtok(row," "))!=NULL){
      if (seur[strlen(seur)-1]=='\n')
	seur[strlen(seur)-1]='\0';
      code=etsikoodi(koodit,clkm+1,seur);
      if (code>-1)
	fprintf(f3,"%d ",code); /* if found, write code */

      /* otherwise skip */
      /*      else fprintf(f3,"%s",seur); */
    }


    while ((seur=strtok(NULL," "))!=NULL){
      if (seur[strlen(seur)-1]=='\n')
	seur[strlen(seur)-1]='\0';
      code=etsikoodi(koodit,clkm+1,seur);
      if (code>-1) 
	fprintf(f3,"%d ",code); /* if found, output code */
      /*else fprintf(f3," %s",seur); */
    }
    fprintf(f3,"\n");
    next++;
  } /* while */


}

/* Transforms all integer tokens */
void muunnakoodit(FILE *f1,FILE *f2,FILE *f3){
  char **koodit;
  int clkm=0;
  int n,next,v;
  int LLEN=50000;
  char *seur,*row;

  koodit=luekoodit(f2,&clkm); /* clkm=largest used code number */
  n=nofrows(f1); 

  if ((row=(char*)malloc(LLEN))==NULL) error(6); 
  next=0;
  while (next<n) {
    if (fgets(row, LLEN, f1)==NULL){
      printf("muunnakoodit\n");
      error(5);
    }
    if (row[0]=='\n'){ /* empty line */
      fprintf(f3,"\n");
      next++; continue;
    }
    if ((seur=strtok(row," "))!=NULL){
      if (seur[strlen(seur)-1]=='\n')
	seur[strlen(seur)-1]='\0';
      /* id preceding ~, write it */
      if (seur[0]=='~'){ 
	fprintf(f3,"~");
	seur++;
      }
      /* if integer and in the table, transform */
      if ((kokluku(seur,&v))&&(v<=clkm)&&(koodit[v]!=NULL))
	fprintf(f3,"%s ",koodit[v]);
      else fprintf(f3,"%s ",seur);
    }

    while ((seur=strtok(NULL," "))!=NULL){
      if (seur[strlen(seur)-1]=='\n')
	seur[strlen(seur)-1]='\0';
      /* jos alussa ~, tulosta se pois */
      if (seur[0]=='~'){ 
	fprintf(f3,"~");
	seur++;
      }
      /* if integer and in the table, transform */
      if ((kokluku(seur,&v))&&(v<=clkm)&&(koodit[v]!=NULL))
	fprintf(f3,"%s ",koodit[v]);
      else fprintf(f3,"%s ",seur);
    }

    fprintf(f3,"\n");
    next++;
  } /* while */

}

char **luekoodit(FILE *f,int *clkm){
  int n,next,v,i,len;
  char **koodit;
  char *row,*endp,*tmp;
  int LLEN=1000;
  char *seur;

  n=nofrows(f); /* gives an upperbound for the number of codes */
  (*clkm)=0;

 if ((koodit=(char**)malloc(n*sizeof(char*)))==NULL)
      error(6);
 for (i=0;i<n;i++)
   koodit[i]=NULL;
 if ((row=(char*)malloc(LLEN))==NULL) error(6); 

 next=0;
 while (next<n) {
   if (fgets(row, LLEN, f)==NULL){
     printf("luekoodit\n");
     error(5);
   }
   tmp=row;
   v=strtol(tmp, &endp, 10); 
   if (endp==row){
     printf("Error on line %d\n",next+1);
     error(5);
   }

   if (v>n){
     printf("Larger code than expected!\n");
     /* could handle by increasing the table...*/
     error(8);
   }

   tmp=endp;
   if (*tmp==' ') /* one separator */
     tmp++;
   
   if ((seur=strtok(tmp," "))!=NULL){
   if (seur[strlen(seur)-1]=='\n')
     seur[strlen(seur)-1]='\0';

   len=strlen(seur)+1;
   if ((koodit[v]=(char*)malloc(len*sizeof(char)))==NULL)
     error(6);
   strcpy(koodit[v],seur);
   if (*clkm<v) (*clkm)=v; /* clkm contains the largest code number */
   }
   next++;
 }

 return koodit;
}


/* return 1, if unsigned integer, and the value to v. Otherwise return 0. */
int kokluku(char *s,int *v){
  int len=strlen(s);
  int i;


  for (i=0;i<len;i++)
    if (isdigit(s[i])==0){
	return 0;
    }

  /* otherwise read integer */
  (*v)=atoi(s);
  return 1;
}

/* number of rows in a file */
int nofrows(FILE *f){
  int lkm=0;
  char *row;
  int LLEN=20000;

  if ((row=(char*)malloc(LLEN))==NULL) error(6);
  while (fgets(row, LLEN, f)!=NULL)
      lkm++;
  rewind(f);
  return lkm;
}

