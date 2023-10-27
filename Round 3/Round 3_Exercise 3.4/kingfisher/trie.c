/* trie.c generic version 2.6. 2010 */
/* contains separate structure and data nodes */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <values.h>
#include "measures.h"
#include "trie.h"
#include "error.h"
#include "apufunktiot.h"

/* determine types of consequents for 1-sets */
void (*process1sets)(Node *t,int num, int *fr, int *oldaddr);
/* process a candidate set */
int (*prunecand)(Node *t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr);
/* check rules */
void (*checkrule)(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l);
void (*checkNullFr)(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l);
void (*checkLowFr)(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l);

Node* createNode(int label){
Node *t;
  int i;

  if ((t=(Node*)malloc(sizeof(Node)))==NULL)
      error(6);
  if ((t->data=(Data*)malloc(sizeof(Data)))==NULL)
      error(6);

  t->label=label; 
  t->chsize=SIZE; 
  if ((t->children=(Node**)malloc(SIZE*sizeof(Node*)))==NULL)
    error(6);
  for (i=0; i<SIZE; i++){
    t->children[i]=NULL;
  }

  t->data->fr=0;  t->data->parents=NULL; 
  t->data->parsize=0; t->data->parlabels=NULL; 
  t->data->pbest=NULL;   t->data->nbest=NULL; 
  t->data->ppossible=NULL; t->data->npossible=NULL; 
  t->data->pposlen=0; t->data->nposlen=0; 
  return t;
}

/* copy the parents possible-tables to p */
void copypossible(Node *p, Node *par){
  Data *pd=p->data;
  Data *pard=par->data; 

  if (pard==NULL){
    printf("pard==NULL in copypossible (trie.c)\n");
    error(9);
  }

  if ((POS)&&(pard->pposlen>0)){
    pd->ppossible=bv_alloc(pard->pposlen);
    pd->pposlen=pard->pposlen;
    bv_copy(pd->ppossible,pard->ppossible,bv_size(pard->pposlen));
  }

  if ((NEG)&&(pard->nposlen>0)){
    pd->npossible=bv_alloc(pard->nposlen);
    pd->nposlen=pard->nposlen;
    bv_copy(pd->npossible,pard->npossible,bv_size(pard->nposlen));
  }
}



/* When possible has been initialized, it can be updated using the other */
/* parents. If a bit is 0 in any parent, it should be 0 in p */ 
void updatepossible(Node *p,Node *par){ 
  /*int i;*/
  Data *pd=p->data;
  Data *pard=par->data; 

  if (POS){
    if (pard->pposlen==0){
      if (pd->pposlen>0)
	bv_free(pd->ppossible);
      pd->pposlen=0;
    }
    else {
      /* if par has a smaller possible-table, decrease the rest from p's */
      /* table */
      if (pard->pposlen<pd->pposlen){
	pd->ppossible=bv_realloc(pd->ppossible,pard->pposlen);
	pd->pposlen=pard->pposlen;
      }

      /* if p has 1 bit but par 0 bit, set p's bit to 0 */      
      /*   for (i=0;i<pd->pposlen;i++)
	if ((bv_tstbit(pd->ppossible,i)==1)&&(bv_tstbit(pard->ppossible,i)==0))
	bv_clrbit(pd->ppossible,i);*/

      bv_and(pd->ppossible,pard->ppossible,bv_size(pd->pposlen));

    }
  }

  if (NEG){
    if (pard->nposlen==0){
      if (pd->nposlen>0)
	bv_free(pd->npossible);
      pd->nposlen=0;
    }
    else {
      /* if par has a smaller possible-table, decrease the rest from p's */
      /* table */
      if (pard->nposlen<pd->nposlen){
	pd->npossible=bv_realloc(pd->npossible,pard->nposlen);
	pd->nposlen=pard->nposlen;
      }
      
      /*     for (i=0;i<pd->nposlen;i++)
	if ((bv_tstbit(pd->npossible,i)==1)&&(bv_tstbit(pard->npossible,i)==0))
	bv_clrbit(pd->npossible,i);*/

      bv_and(pd->npossible,pard->npossible,bv_size(pd->nposlen));

    }
  }
}


/* create best and possible tables for 1-sets and initialize them. */
/* best has just 1 element (the attr. itself) and best-value=INIT */
/* possible has first num elements, whose value is 1 (size can be */
/* reduced later).*/
void initbestandpossible(Node *p, int *fr, int *oldaddr,int *order,int num){
  int i;
  Data *pd=p->data;

  if (POS){
    pd->ppossible=bv_alloc(num);
    for (i=0;i<num;i++){
      bv_setbit(pd->ppossible,i);
    }
    pd->pposlen=num;
    if ((pd->pbest=(double*)malloc(1*sizeof(double)))==NULL) /* was float */
      error(6);
    pd->pbest[0]=INIT;
  }
  if (NEG){
    pd->npossible=bv_alloc(num);
    for (i=0;i<num;i++){
      bv_setbit(pd->npossible,i);
    }
    pd->nposlen=num;
    if ((pd->nbest=(double*)malloc(1*sizeof(double)))==NULL) /* was float */
      error(6);
    pd->nbest[0]=INIT;
  }
}


/* separately for pos. and neg. consequences */
void removebestandpossible(Node *p){
  Data *pd=p->data;

  if (POS){
    if (pd->pposlen>0){
      bv_free(pd->ppossible);
      pd->pposlen=0;      
    }
    if (pd->pbest!=NULL){
      free(pd->pbest);
      pd->pbest=NULL;
    }
  }

  if (NEG){
    if (pd->nposlen>0){
      bv_free(pd->npossible);
      pd->nposlen=0;      
    }
    if (pd->nbest!=NULL){
      free(pd->nbest);
      pd->nbest=NULL;
    }
  }
}

/* remove structures for only positive or only negative rules */ 
void removebestandpossibleforposorneg(Node *p,int sign){
  Data *pd=p->data;

  if (sign==1){
    if (pd->pposlen>0){
      bv_free(pd->ppossible);
      pd->pposlen=0;      
    }
    if (pd->pbest!=NULL){
      free(pd->pbest);
      pd->pbest=NULL;
    }
  }

  if (sign==0){
    if (pd->nposlen>0){
      bv_free(pd->npossible);
      pd->nposlen=0;      
    }
    if (pd->nbest!=NULL){
      free(pd->nbest);
      pd->nbest=NULL;
    }
  }
}

/* removes data node from p */
void removeData(Node *p){

  if (p->data!=NULL){
    removebestandpossible(p);
    if (p->data->parsize>0){
      free(p->data->parents);
      free(p->data->parlabels);
    }
    free(p->data);
    p->data=NULL;
  }
}



/* copy the parent's best-table to p. Use only for the "real parent" */
/* under which p is linked. Now the first l-1 attributes are in the same */
/* order. The last value (missing from par) is set to INIT (poorest */
/* possible value). Separately for pos. and neg. consequences. */
void copybest(Node *p,Node *par, int l){
  int i;
  Data *pd=p->data;
  Data *pard=par->data; 

  if ((POS)&&(pard->pbest!=NULL)){
  /* if the parent doesn't have pbest, do nothing */
    if ((pd->pbest=(double*)malloc(l*sizeof(double)))==NULL) /* was float */
      error(6);
    for (i=0;i<l-1;i++)
      pd->pbest[i]=pard->pbest[i];
    pd->pbest[l-1]=INIT;
  }

  /* the same for negative cons. */
  if ((NEG)&&(pard->nbest!=NULL)){
  /* if the parent doesn't have nbest, do nothing */
    if ((pd->nbest=(double*)malloc(l*sizeof(double)))==NULL) /* was float */
      error(6);
    for (i=0;i<l-1;i++)
      pd->nbest[i]=pard->nbest[i];
    pd->nbest[l-1]=INIT;
  }
}

/* Update best-values given a parent. Now best values can be in */
/* different order. The attributes are listed in kandi and */
/* parset. All attributes in parset occur in kandi. */
/* l is the size of kandi and l-1 of parset */
/* Separately for pos. and neg. cons. */
void updatebest(Node *p, Node *par, int *kandi, int *parset, int l){
  int i,j;
  Data *pd=p->data;
  Data *pard=par->data; 

  if ((POS)&&(pd->pbest!=NULL)&&(pard->pbest!=NULL)){
    /* if p or parent doesn't have pbest, do nothing */
    /* for all attr. in parset, search the same attr. in kandi */
    for (i=0;i<l-1;i++){
      for (j=0;j<l;j++)
	if (parset[i]==kandi[j])
	  break;
      /* now j should be  the correct position, just checking */
      if (parset[i]!=kandi[j]){
	printf("attribute % in parset was not found in kandi!\n",parset[i]);
	error(9);
      }
      /* if parent has a better (smaller) best-value, update p's best value */
      if (better(pard->pbest[i],pd->pbest[j]))
	pd->pbest[j]=pard->pbest[i];
    }
  }

  /* the same for neg. cons. */
  if ((NEG)&&(pd->nbest!=NULL)&&(pard->nbest!=NULL)){
    /* if p or parent doesn't have pbest, do nothing */
    /* for all attr. in parset, search the same attr. in kandi */
    for (i=0;i<l-1;i++){
      for (j=0;j<l;j++)
	if (parset[i]==kandi[j])
	  break;
      /* now j should be  the correct position, just checking */
      if (parset[i]!=kandi[j]){
	printf("attribute % in parset was not found in kandi!\n",parset[i]);
	error(9);
      }
      /* if parent has a better best-value, update p's best value */
      if (better(pard->nbest[i],pd->nbest[j]))
	pd->nbest[j]=pard->nbest[i];
    }
  }
}



/* for all attributes not in kandi, set the bit in possible to 0 */
/* separately for ppossible and npossible */
void removeothers(Node *p,int *kandi,int *oldaddr,int l){
  int i;
  Data *pd=p->data;

  if (POS){
    for (i=0;i<pd->pposlen;i++) /* for all positive consequents */
      if ((bv_tstbit(pd->ppossible,i)==1)&&(occursinset(oldaddr[i],kandi,l)==0))
	bv_clrbit(pd->ppossible,i);
  }
  if (NEG){
    for (i=0;i<pd->nposlen;i++) /* for all negative consequents */
      if ((bv_tstbit(pd->npossible,i)==1)&&(occursinset(labelgivennegind(i,oldaddr),kandi,l)==0))
	bv_clrbit(pd->npossible,i);
  }
}


/* if table possible contains only 0-bits in the end, it can be reduced */
/* separately for ppossible and npossible */
void reducepossible(Node *p){
  int i;
  Data *pd=p->data;

  if ((POS)&&(pd->pposlen>0)){
    /* search the last index with 1-bit */
    for (i=pd->pposlen-1;i>=0;i--)
      if (bv_tstbit(pd->ppossible,i)==1)
	break;
    /* now i gives the last index with bit 1 or i=-1 */
    if (i<0){
      /* no consequents possible */
      bv_free(pd->ppossible);
      pd->pposlen=0;
    }
    else { /* check if decreased */
      if (i<pd->pposlen-1){
	pd->pposlen=i+1;
	pd->ppossible=bv_realloc(pd->ppossible,pd->pposlen);
      } 
    }
  }

  if ((NEG)&&(pd->nposlen>0)){
    /* search the last index with 1-bit */
    for (i=pd->nposlen-1;i>=0;i--)
      if (bv_tstbit(pd->npossible,i)==1)
	break;
    /* now i gives the last index with bit 1 or  i=-1 */
    if (i<0){
      /* no consequents possible */
      bv_free(pd->npossible);
      pd->nposlen=0;
    }
    else { /* check if decreased */
      if (i<pd->nposlen-1){
	pd->nposlen=i+1;
	pd->npossible=bv_realloc(pd->npossible,pd->nposlen);
      } 
    }
  }
}

/* free the node and its datanode, if exists + all children nodes */
void freeNode(Node* t){
  int i;

  /* remove data node */
  removeData(t);

  if (t->chsize>0){
    for (i=0; i<t->chsize; i++)
      if (t->children[i]!=NULL)
	free(t->children[i]);
    free(t->children);   
  }
  free(t);
}


void doublesize(Node*** arr, int size){
  int i;
  if ((*arr=(Node**)realloc(*arr,2*size*sizeof(Node*)))==NULL)
      error(6);

  for (i=size; i<2*size; i++)
    (*arr)[i]=NULL;  
}

void doublelabels(short** arr, int size){
  int i;
  if ((*arr=(short*)realloc(*arr,2*size*sizeof(short)))==NULL)
    error(6);
  for (i=size; i<2*size; i++)
    (*arr)[i]=-1;
}



void delChild(Node* p, int label,int* order){
  Node* ch; /* child */
  int ind; /* index of the child */
  if ((ch=searchChild(p, label, &ind,order))!=NULL){
    removeCh(p,ind);
    freeNode(ch);
  }
}

void delChildGivenInd(Node *p,int ind){

  freeNode(p->children[ind]);
  removeCh(p,ind);
}


/* remove a leaf node when it has a link to the parent */
void delLeaf(Node *p,int* order){
  int ind;

  if (p->data->parents[0]==NULL)
    printf("1. parent null (delleaf)\n");

  /* poista tiedot  vanhemmasta */
    if (p->data->parents[0]!=NULL){
      if (searchChild(p->data->parents[0],p->label,&ind,order)==NULL)
      (p->data->parents[0])->children[ind]=NULL;
     }

  freeNode(p);
}


/* remove just the child pointer, given its label */
/* the child node itself may be already deleted */
void removeCh(Node* p,int ind){
  p->children[ind]=NULL;
}


/* search parent from node p, given its label */
/* Don't call with label=-1! Parents do not have to be in order. */
Node* searchParent(Node* p, int label, int* index){
  int i;
  Data *pd=p->data;

  if (label==-1){
    printf("searchParent\n");
    error(9);
  }

  for (i=0; i<pd->parsize; i++)
    if (pd->parlabels[i]>=0){
      if (pd->parlabels[i]==label){
	(*index)=i;
	return pd->parents[i];
      }
    }
  (*index)=-1;
  return NULL;
}


/* Don't call if label==-1! */
Node* searchChild(Node* p, int label, int* index, int* order){
  int i;

  if (label==-1){
    printf("searchChild\n");
    error(9);
  }

  for (i=0; i<p->chsize; i++)
    if (p->children[i]!=NULL){
      if (order[p->children[i]->label]>order[label]){
	(*index)=-1;
	return NULL;
      }
      if (p->children[i]->label==label){
	(*index)=i;
	return p->children[i];
      }
    }
  (*index)=-1;
  return NULL;
}


/* binary search without recursion, 5.10.08 */ 
/* olabel=orderl[label] */
int binsearch2(int a, int b, short* tab, short olabel,int* order){
  int  c;


  /* enta jos valissa labeleita -1?? */

  if (a>b){
    printf("a>b in binsearch2 (trie.c)\n");
    error(9);
  }

  while (a<b){
    if ((order[tab[a]]>olabel)||(order[tab[b]]<olabel))
 
      return -1; /* not found */
    c=(a+b)/2;
    if (order[tab[c]]<olabel)
      a=c+1;
    else {
      if (order[tab[c]]>olabel)
	b=c-1;
      else return c; /* shoud be equal */
    }
  }
  /* now a==b */
  if (order[tab[a]]==olabel) return a;
 /* otherwise not found */
  return -1;
}



void printlabels(Node **nodes, int size){
  int i;

  for (i=0;i<size;i++)
    if (nodes[i]==NULL)
      printf("-1 ");
    else printf("%d ",nodes[i]->label);
  printf("\n");
}


/* binary search when labels are in the nodes and only a node pointer table */
/* is given. Notice: olabel=orderl[label]  */
int binsearchnodes(int a, int b, Node **nodes, short olabel,int* order){
  int  c,prevc;

  if (a>b){
    printf("a>b in binsearch2 (trie.c)\n");
    error(9);
  }


  while (a<b){ 
    while ((nodes[a]==NULL)&&(a<b))
      a++;
    while ((nodes[b]==NULL)&&(b>a))
      b--;
    if (a==b) break;

    if ((order[nodes[a]->label]>olabel)||(order[nodes[b]->label]<olabel))
      return -1; /* not found */
    c=(a+b)/2; prevc=c;
    while (nodes[c]==NULL)
      prevc--;
    /* in the end prevc==a, if only NULLs between a and c */
    /* if no nulls, then prevc==c */


    if (order[nodes[prevc]->label]<olabel)
      a=c+1;
    else {
      if (order[nodes[prevc]->label]>olabel){
	b=prevc-1;
	if (b<a) return -1;
      }
      else return prevc; /* shoud be equal */
    }
  }
  /* now a==b */
  if ((nodes[a]!=NULL)&&(order[nodes[a]->label]==olabel)) return a;
  /* otherwise not found */
  return -1;
}


Node *bfsearchChild(Node *p,int label, int *index){
  int i;
  for (i=0;i<p->chsize;i++)
  if ((p->children[i]!=NULL)&&(p->children[i]->label==label)){
    (*index)=i;
    return p->children[i];
  }
(*index)=-1;
return NULL;
}

/* version 5.10.08 */
Node* binsearchChild(Node* p, int label, int* index, int* order){
  int vika;
  if (label==-1){
    printf("binsearchChild\n");
    error(9);
  }

  /*  printf("binsearchschild, sijoitettavana %d\n",label);*/

  /* is table empty? */
  if (p->chsize<1){(*index)=-1; return NULL;}

  /* special case: just one element */
  if (p->chsize==1){
    if ((p->children[0]!=NULL)&&(p->children[0]->label==label)) {
      (*index)=0; return p->children[0];}
    else {(*index)=-1; return NULL;}
  }
  else {
    if (p->children[p->chsize-1]==NULL){ /* empty cell */
      vika=prevCh(p,p->chsize-1); /* last non-empty cell */
      if (vika<0){ /* empty table */
	(*index)=-1; return NULL;}   
    }
    else vika=p->chsize-1; /* full table */
    if (((*index)=binsearchnodes(0,vika,p->children,order[label],order))==-1)
      return NULL; /* not found */
    else {
      return p->children[*index];
    }
  }
}


int nextFreeChildNodes(Node **children, int size){
  int i;
  for (i=0; i<size; i++)
    if (children[i]==NULL)
      return i;
  return -1; /* no free indices left */
}


int nextFreeChild(short* labels, int size){
  int i;
  for (i=0; i<size; i++)
    if (labels[i]==-1)
      return i;
  return -1; /* no free indices left */
}


/* lis‰tty orderlabels */
 Node* addChild(Node* p, int label, int* order){
  int ind;
  Node* ch;

  ch=createNode(label);
  if ((ind=nextFreeChildNodes(p->children,p->chsize))!=-1){ /* add as i's child */
    /*    printf("1. vapaa ind on %d\n",ind);*/
    p->children[ind]=ch;
  }
  else { /* no free indices left */
    doublesize(&(p->children),p->chsize);
    p->children[p->chsize]=ch;
    (p->chsize)*=2;
  }

  if (p->chsize>1)
    orderlabels2(p,order);

  return ch;
}


/* kertoo myˆs indeksin */
 Node* addChild2(Node* p, int label, int* order,int* ind){
  Node* ch;
  int i;

  ch=createNode(label);
  if (((*ind)=nextFreeChildNodes(p->children,p->chsize))!=-1){ /* add as i's child */
    /*    printf("1. vapaa ind on %d\n",ind);*/
    p->children[*ind]=ch;
  }
  else { /* no free indices left */
    doublesize(&(p->children),p->chsize);
    p->children[p->chsize]=ch;
    (p->chsize)*=2;
  }


  if (p->chsize>1){
    orderlabels2(p,order);
  }
  
  /* p‰ivit‰ viel‰ ind */
  if (binsearchChild(p,label, ind,order)==NULL){
    printf("addchild2 label=%d chsize=%d\n", label,p->chsize);
        for (i=0;i<p->chsize;i++)
	  printf("%d ",p->children[i]->label);
    error(9);
  }

  return ch;
}


 Node* searchSet(int* set, int size, Node* t,int* order){
  Node* p=t;
  Node* ch;
  int i, dummy;
  
  for (i=0; i<size; i++){
     ch=binsearchChild(p,set[i],&dummy,order);

     /*  ch=bfsearchChild(p,set[i],&dummy);*/

    if (ch==NULL)
      return NULL; /* not found */
    p=ch;
  }
  return p;
}

int nextFreePar(Node* p){
  int i;

  for (i=0; i<p->data->parsize; i++)
    if (p->data->parlabels[i]==-1)
      return i;
  return -1;
}

/* seuraava vanh. indeksin ind j‰lkeen p:ss‰*/
int nextPar(Node* p, int ind){
  int i;

  for (i=ind+1; i<p->data->parsize; i++)
    if (p->data->parlabels[i]>=0)
      return i;
  return -1;
}

/* seuraava lapsen indeksi ind j‰lkeen */
int nextCh(Node* p, int ind){
  int i;

  for (i=ind+1; i<p->chsize; i++)
    if (p->children[i]!=NULL)
      return i;
  return -1;
}


/* edellisen lapsen indeksi ennen ind:i‰ */
int prevCh(Node* p, int ind){
  int i;

  for (i=ind-1; i>=0; i--)
    if (p->children[i]!=NULL)
      return i;
  return -1; /* olisi joka tap. i=-1 lopussa */
}


/* set to parlabels the given label A such that p describes set YA and */
/* par describes set Y */
void addParent(Node* p, Node* par,int label){
  int pos, i;
  Data *pd=p->data;

  /* first parent: reserve memory */
  if (pd->parents==NULL){
    if ((pd->parents=(Node**)malloc(SIZE*sizeof(Node*)))==NULL)
      error(6);
    if ((pd->parlabels=(short*)malloc(SIZE*sizeof(short)))==NULL)
    error(6);
    
    for (i=0; i<SIZE; i++){
      pd->parents[i]=NULL;
      pd->parlabels[i]=-1;
    }
    pd->parsize=SIZE;
  }

  if ((pos=nextFreePar(p))<0){ /* ei tilaa */
    doublesize(&(pd->parents),pd->parsize);
    doublelabels(&(pd->parlabels),pd->parsize);
    pos=pd->parsize;
    (pd->parsize)*=2;
  }
  /* lis‰‰ */

  pd->parents[pos]=par;
  pd->parlabels[pos]=label;
}


void reduceParSize(Node* p, int size){
  int ind,ind2,i; 
  int last=-1;
  Data *pd=p->data;

  if ((size==0)&&(pd->parents!=NULL)){
    free(pd->parents);
    free(pd->parlabels);
    pd->parsize=0;
  }
  else {

    printf("ennen redusointia ");
    for (i=0; i<pd->parsize; i++)
      if (pd->parlabels[i]>=0)
	printf("%d ",pd->parlabels[i]);
      else printf("_");
    printf("\n");

    while ((ind=nextFreePar(p))!=-1){
      if ((ind2=nextPar(p,ind))!=-1){
	pd->parents[ind]=pd->parents[ind2];
	pd->parlabels[ind]=pd->parlabels[ind2];
	pd->parents[ind2]=NULL;
	pd->parlabels[ind2]=-1;
	last=ind; /* viimeinen t‰ytetty */
      }
      else break; /* ei en‰‰ vanhempia */
    } /* while */

    printf("redusoinnin j‰lkeen ");
    for (i=0; i<pd->parsize; i++)
      if (pd->parlabels[i]>=0)
	printf("%d ",pd->parlabels[i]);
      else printf("_");
    printf("\n");


    if (last+1!=size)
      error(11);
    pd->parsize=size;
    if ((pd->parents=(Node**)realloc(pd->parents,size*sizeof(Node*)))==NULL)
      error(6);
    if ((pd->parlabels=(short*)realloc(pd->parlabels,size*sizeof(short)))==NULL)
      error(6);
  } /* else */
}


/* remove Nulls to the end and return the number of used cells */
int removeNulls(Node* p){
 int ind,ind2; 
 int i, lkm=0;

 while ((ind=nextFreeChildNodes(p->children,p->chsize))!=-1){
   if ((ind2=nextCh(p,ind))!=-1){
     p->children[ind]=p->children[ind2];
     p->children[ind2]=NULL;
   }
   else break; /* no more children */
 }
 /* number of children */
 for (i=0;i<p->chsize;i++)
   if (p->children[i]!=NULL) lkm++;

 return lkm;
}

/* cut p's children table from the first empty cell */
void cutoff(Node* p){
  int size;

  if ((size=nextFreeChildNodes(p->children,p->chsize))!=-1){
    p->chsize=size;
    if ((p->children=(Node**)realloc(p->children,size*sizeof(Node*)))==NULL)
      error(6);
  }
}


void reduceChSize(Node* p, int size){
  int occupied=-1;
  
  if (size==0){  /* Jos tuhotaan kokonaan, on tarkistettava delNode:ssa */
    free(p->children);
    p->chsize=0;
  }
  else {
    occupied=removeNulls(p);
    if (occupied!=size)
      error(11);
    p->chsize=size;
    if ((p->children=(Node**)realloc(p->children,size*sizeof(Node*)))==NULL)
      error(6);
  } /* else */
}



/* siivoaa solmun lapsista NULL-osoittimet ja pienent‰‰ taulukkoja */
void cleanCh(Node* p){
  int occupied; /* k‰ytettyjen solujen lkm */

  occupied=removeNulls(p);
  if (occupied<p->chsize){
    if (occupied==0)
      occupied=1;
    p->chsize=occupied;
  if ((p->children=(Node**)realloc(p->children,occupied*sizeof(Node*)))==NULL)
    error(6);
  }
}



/* Assumption: at most one child can be in the wrong position */
void orderlabels2(Node* t,int* order){
  int i,ind2,ind=-1;
  Node* tmp;
  int tmplabel;

  /*  printf("labelit alussa\n");
  for (i=0; i<t->chsize;i++)
  printf("%d ",t->children[i]->label);*/
 

  for (i=0;i<t->chsize;i++)
    if (t->children[i]!=NULL){
      ind=nextCh(t,i);
      if ((ind>0)&&(order[t->children[i]->label]>order[t->children[ind]->label])){
	/*printf("alkiot %d ja %d v‰‰rin\n",t->children[i]->label,t->children[ind]->label);*/
	/* joko i tai ind v‰‰r‰ss‰ paikassa */
	break;
      }
    }
  if (i<t->chsize-1){
    /* lˆytyi virhe */
    tmplabel=t->children[i]->label;
    tmp=t->children[i];
    /* swapataan ne */
    t->children[i]=t->children[ind];
    /* onko tmp yh‰ v‰‰rin? */
    ind2=nextCh(t,ind);
    while ((ind2>=0)&&(order[tmplabel]>order[t->children[ind2]->label])){
      /* siirr‰ tmp:t‰ oikealle */
      t->children[ind]=t->children[ind2];
      ind=ind2;
      ind2=nextCh(t,ind);
    }
    /* nyt tmp oikein */
    t->children[ind]=tmp;
    /* onko uusi children[i] v‰‰rin? */
    tmp=t->children[i];
    tmplabel=t->children[i]->label;
    ind2=prevCh(t,i);
    ind=i;
    while ((ind2>=0)&&(order[tmplabel]<order[t->children[ind2]->label])){
      /* siirr‰ children[i] vasemmalle */
      t->children[ind]=t->children[ind2];
      ind=ind2;
      ind2=prevCh(t,ind);
    }
    /* nyt tmp oikein */
    t->children[ind]=tmp;
  } /* if lˆytyi virhe */


  /*  printf("\n labelit lopussa\n");
  for (i=0; i<t->chsize;i++)
  printf("%d ",t->children[i]->label);*/
}


/* delet tree; return the number of deleted nodes */
int deleteTree(Node* p){
  int i;
  int ch=0; 
  int lkm=0;
  if (p!=NULL){

    for (i=0; i<p->chsize; i++){
      if (p->children[i]!=NULL){
	/* printf("lapsen label %d\n",p->children[i]->label);*/
	ch++;
	lkm+=deleteTree(p->children[i]);
	p->children[i]=NULL;
      }
    }
    /*    printf("viel‰ juuri %d, oli %d lasta\n",p->label,ch);*/
    freeNode(p);
    lkm++;
  }
  return lkm;
}


/* printtaa puu puurakenteisena */
void printrectree(Node* t,int taso){
  int i,j;
  int ch=0;
  /* int spt=0;*/

  if (t==NULL) return;

  printf("--(A%d",t->label);
  if (t->data!=NULL)
    printf("fr%d)\n",t->data->fr);
  else printf(")\n");
  /* onko lapsia? */
  for (i=0; i<t->chsize; i++)
    if ((t->children[i])!=NULL)
      ch++;

  /* jos oli */
  if (ch>0){
    for (i=0; i<t->chsize; i++){
      if ((t->children[i])!=NULL){
        for (j=0; j<taso; j++)
          printf("  |"); /* sisennykset joka lapselle */
        printrectree(t->children[i],taso+1);
      } 
    }
  }
  /* muuten on lehti */
  /* printf("\n");*/
}



void printtree(Node* t){
  int i;

  printf("juuri %d\n",t->label);
  for (i=0; i<t->chsize; i++){
    if ((t->children[i])!=NULL){
      printrectree(t->children[i],1);
      printf("\n");
    }
  }
  printf("\n");
}

/* l is the size of kandi or best-tables */
void printpossibleandbest(Node *p,int *oldaddr,int l){
  int i;
  Data *pd=p->data;

  printf("ppossible, pposlen=%d. Possible consequents:\n",pd->pposlen);
  for (i=0;i<pd->pposlen;i++)
    if (bv_tstbit(pd->ppossible,i))
      printf("A%d ",oldaddr[i]);
  printf("\n");

  printf("npossible, nposlen=%d. Possible consequents:\n",pd->nposlen);
  for (i=0;i<pd->nposlen;i++)
    if (bv_tstbit(pd->npossible,i))
      printf("~A%d ",labelgivennegind(i,oldaddr));
  printf("\n");

  printf("pbest: ");
  if (pd->pbest==NULL)
    printf("missing\n");
  else 
    for (i=0;i<l;i++)
      printf("%.2e ",pd->pbest[i]);
  printf("\n");

  printf("nbest: ");
  if (pd->nbest==NULL)
    printf("missing\n");
  else 
    for (i=0;i<l;i++)
      printf("%.2e ",pd->nbest[i]);
  printf("\n");

}
