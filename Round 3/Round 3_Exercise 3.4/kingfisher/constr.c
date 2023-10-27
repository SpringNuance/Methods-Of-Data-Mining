/* constr.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "error.h"
#include "constr.h"
#include "apufunktiot.h"

/* Each row of the files lists attributes (their numbers), any two of */
/* which cannot occur together in the same rule. Creates a 2-d 0/1 matrix */
/* M, where M[i][j]=1, when Ai and Aj are mutually exclusive, and 0 */
/* otherwise. Only attributes 0,..,k are listed in M. */ 
LYHYT **readconstraints(FILE *f, int k){
  char *row, *tmp;
  char *endp;
  int i,j,v,next,l;
  LYHYT **t;
  int apu[k];

  if ((t=(LYHYT**)malloc(k*sizeof(LYHYT*)))==NULL)
    error(6);
  for (i=0;i<k;i++){
    if ((t[i]=(LYHYT*)malloc(k*sizeof(LYHYT)))==NULL)
      error(6);
    for (j=0;j<k;j++)
      t[i][j]=0;
  }

  if ((row=(char*)malloc(10000))==NULL) error(6);
  l=0;
  while (fgets(row,10000,f)!=NULL){
    next=0;
    if (emptystring(row)){ 
      l++; continue;
    }
    /* read first attr */
    v=strtol(row, &endp, 10);
    if (endp==row){
      printf("Compatibility constraints: Non-integer value at row %d\n",l+1);
      error(5);
    }
    if ((v>=0)&&(v<k)){ /* add only values in the given range */
      apu[next]=v; next++;
    }
    tmp=endp;
    while ((*tmp==',')||(*tmp==' ')) 
      tmp++; 
    /* read the rest */
    while ((*tmp!='\n')&&(*tmp!='\0')) {
      v=strtol(tmp, &endp, 10);
      if (endp==tmp){
	printf("Compatibility constraints: Non-integer value at row %d\n",l+1);
        error(5);
      }
      if ((v>=0)&&(v<k)){
	apu[next]=v; next++;
      }
      tmp=endp;
      while ((*tmp==',')||(*tmp==' ')) 
        tmp++; 
    }
    /* now add to constr matrix t */ 
    for (i=0;i<next-1;i++)
      for (j=i+1;j<next;j++)
	if ((apu[i]>=0)&&(apu[i]<k)&&(apu[j]>=0)&&(apu[j]<k))
	{
	t[apu[i]][apu[j]]=1; t[apu[j]][apu[i]]=1; 
      }
    l++;
  } /* next row */
  free(row);

  return t;
}

/* On each row, the first attribute refers to a consequence and the */
/* rest are attributes which are not allowed to occur in the condition */
/* parts of its rules. */
LYHYT **readextraconstr(FILE *f, int k){
  char *row, *tmp;
  char *endp;
  int i,j,v,v2,l;
  LYHYT **t;

  if ((t=(LYHYT**)malloc(k*sizeof(LYHYT*)))==NULL)
    error(6);
  for (i=0;i<k;i++){
    if ((t[i]=(LYHYT*)malloc(k*sizeof(LYHYT)))==NULL)
      error(6);
    for (j=0;j<k;j++)
      t[i][j]=0;
  }

  if ((row=(char*)malloc(10000))==NULL) error(6);
  l=0;
  while (fgets(row,10000,f)!=NULL){
    if (emptystring(row)){ 
      l++; continue;
    }
    /* read first attr */
    v=strtol(row, &endp, 10);
    if (endp==row){
      printf("Extra constraints: Non-integer value at row %d\n",l+1);
      error(5);
    }
    tmp=endp;

    if ((v>=0)&&(v<k)){ /* add only values in the given range */
      while ((*tmp==',')||(*tmp==' ')) 
	tmp++; 
      /* read the rest */
      while ((*tmp!='\n')&&(*tmp!='\0')) {
	v2=strtol(tmp, &endp, 10);
	if (endp==tmp){
	  printf("Extra constraints: Non-integer value at row %d\n",l+1);
	  error(5);
	}
	if ((v>=0)&&(v<k)){
	  t[v][v2]=1; t[v2][v]=1; /* Notice: sets both ways! */
	}
	tmp=endp;
	while ((*tmp==',')||(*tmp==' ')) 
	  tmp++; 
      }
    }
    l++;
  } /* next row */
  free(row);

  return t;
}



void printconstr(LYHYT **constraints,int k){
  int i,j;

  printf("Compatibility constraints:\n");
  for (i=0;i<k;i++){
    printf("%d: ",i);
    for (j=0;j<k;j++)
      if (constraints[i][j])
	printf("%d ",j);
    printf("\n");
  }
}

LYHYT *readinterattr(FILE *f, int k){
  LYHYT *t;
  char *row, *tmp;
  char *endp;
  int i,v,l=0;

  if ((t=(LYHYT*)malloc(k*sizeof(LYHYT)))==NULL)
    error(6);
  for (i=0;i<k;i++)
    t[i]=0;

  if ((row=(char*)malloc(10000))==NULL) error(6);

  while (fgets(row,10000,f)!=NULL){
     /* read first attr */
    if (emptystring(row)){ 
      l++; continue;
    }
    v=strtol(row, &endp, 10);
    if (endp==row){
      printf("Interesting attributes: Non-integer value at row %d\n",l+1);
      error(5);
    }
    if ((v>=0)&&(v<k))
      t[v]=1;
     tmp=endp;
    while ((*tmp==',')||(*tmp==' ')) 
      tmp++; 
    /* read the rest */
    while ((*tmp!='\0')&&(*tmp!='\n')) {
      /* oli: while (*tmp!='\n') { */
      v=strtol(tmp, &endp, 10);
      if (endp==tmp){
	printf("Interesting attributes: Non-integer value at row %d (\''%s\'')\n",l+1,tmp);
        error(5);
      }
      if ((v>=0)&&(v<k))
	t[v]=1;
      tmp=endp;
      while ((*tmp==',')||(*tmp==' ')) 
        tmp++; 
    }
    l++;
  } /* next row */
  free(row);

  return t;
}


void printinterattr(LYHYT *t,int k){
  int i;

  printf("Given interesting attributes:\n");
  for (i=0;i<k;i++)
    if (t[i]==1)
      printf("A%d ",i);
  printf("\n");

}
