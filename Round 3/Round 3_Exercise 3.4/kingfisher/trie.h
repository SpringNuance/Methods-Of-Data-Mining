/* trie19_2_09.h */
#ifndef trippeli
#define trippeli

#include  <stdlib.h>
#include "bitmatrice.h"
#include "ibitvector.h"
#include "glob.h"

#define SIZE 1 /* alkukoko osoitintaulukoille */
#define LK 1 /* 1 lk-muuttujaa */


typedef struct dnode{
  struct node** parents;
  short* parlabels; /* i:th element contains the label which is missing */
  /* from the set defined by the i:th parent */
  double *pbest; /* were float */
  double *nbest;
  /* own or parent's best M-values for all consequents */
  /* in kandi (in the same order), size l=length of kandi */
  /* best is NULL, if no non-red. rules found so far */
  bitvector *ppossible; /* is consequent A_i possible? */
  bitvector *npossible; /* is consequent ~A_i possible? */
  unsigned short pposlen; /* length of vector ppossible  */ 
  unsigned short nposlen; /* length of vector npossible  */ 
  /*  unsigned short uselen; */ 
  unsigned int fr:24;
  unsigned int parsize: 6; /* size of the parents array */
} Data;

typedef struct node{
  struct dnode* data;
  struct node** children;
  unsigned short label;
  unsigned short chsize; /* size of the children array*/ 
} Node;


/***********************************************************************/
/* indexing tables ppossible and npossible is encapsulated             */
/***********************************************************************/
/* index for ~A_j such that order[A_j]=i in npossible-table */
/* for A_j the position is ppossible[i] */
static inline int negind(int i){ 
  return (attnum-(i)-1);
}

/* if index in npossible in nind, what is the order number of attr? */
static inline int ordergivennegind(int nind){
  return attnum-nind-1;
}

/* if index in npossible in nind, what is the label of attr? */
static inline int labelgivennegind(int nind, int *oldaddr){
  int ind=ordergivennegind(nind);
  return oldaddr[ind];
}

/* is i:th attribute available in npossible? */
static inline int isnegav(Node *p,int i){
  int ind=negind(i);
  if ((p->data->nposlen>0)&&(ind<p->data->nposlen)&&(bv_tstbit(p->data->npossible,ind)))
    return 1;
  else return 0;
}

static inline int isposav(Node *p,int i){
  if ((p->data->pposlen>0)&&(i<p->data->pposlen)&&(bv_tstbit(p->data->ppossible,i)))
    return 1;
  else return 0;
}

/* set i:th attribute as NAV in npossible */
static inline void setnegnav(Node *p,int ind){
  int nind=negind(ind);
  if ((p->data->nposlen>0)&&(nind<p->data->nposlen))
    bv_clrbit(p->data->npossible,nind);
}

/* set i:th attribute as NAV in ppossible */
static inline void setposnav(Node *p,int ind){
  if ((p->data->pposlen>0)&&(ind<p->data->pposlen))
    bv_clrbit(p->data->ppossible,ind);
}

/**************************************************************************/
extern void (*process1sets)(Node *t,int num, int *fr, int *oldaddr);
extern int (*prunecand)(Node *t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr);
extern void (*checkrule)(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l);
extern void (*checkNullFr)(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l);
extern void (*checkLowFr)(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l);

/**************************************************************************/
Node* createNode(int label); /* luo solmu annetulla labelilla ja alusta */
/* kentat ja palauta os. siihen */
void copypossible(Node *p, Node *par);
void updatepossible(Node *p,Node *par);
void initbestandpossible(Node *p, int *fr, int *oldaddr,int *order,int num);
void removebestandpossible(Node *p);
void removebestandpossibleforposorneg(Node *p,int sign);
void removeData(Node *p);
void copybest(Node *p,Node *par, int l);
void updatebest(Node *p, Node *par, int *kandi, int *parset, int l);
void removeothers(Node *p,int *kandi,int *oldaddr,int l);
void reducepossible(Node *p);
void addused(Node *p,int size);
void addusedinfo(Node *p);
void freeNode(Node* t); 
Node* addChild(Node* p, int label,int* order); /* luo par:ille lapsi annetulla */
/* labelilla ja palauta os. siihen */
Node* addChild2(Node* p, int label, int* order,int* ind);
void removeCh(Node* p,int ind);
Node* searchParent(Node* p, int label, int* index);
void doublesize(Node*** arr, int size); /* tuplaa Node-taulukon koko */
void doublelabels(short** arr, int size); /* tuplaa label-talun koko */
Node* searchChild(Node* p, int label, int* index,int* order);/* etsi p:stä labelin */
Node* binsearchChild(Node* p, int label, int* index,int* order);/* etsi p:stä labelin */
/* os. lapsi, palauta sen os. ja indeksi  */
void delChild(Node* p, int label,int* order);/* poista p:stä labelin os. lapsi */
void delChildGivenInd(Node *p,int ind);
void delLeaf(Node *p,int* order);
void cutoff(Node* p);
void orderlabels(Node* t,int* order);
void orderlabels2(Node* t,int* order);
int nextFreeChild(short* labels, int size); /* next free index if any in */
/* children */
int nextFreeChildNodes(Node **children, int size);
Node* searchSet(int* set, int size, Node* t,int* order); /* search set of given size */
/* from tree t; pointer to the last node, if found, else NULL */ 
int nextFreePar(Node* p); /* return the index of the next free pos. in */
/* parents */
int nextPar(Node* p, int ind); /* next par after ind */
int nextCh(Node* p, int ind); /* samoin seur. lapsi */
int prevCh(Node* p, int ind); 
void addParent(Node* p, Node* par, int label); /* add parent par to node p */
void delParent(Node* p, Node* par); /* delete parent par */
void incr(Node* p); /* increase p->fr by 1 */
void reduceParSize(Node* p, int size); /* arrange pointers into */
/* beginning and reduce Parents-array to given size */
void reduceChSize(Node* p, int size); /* samoin lapsille */
void cleanCh(Node* p);
int removeNulls(Node* p);
void insertvector(Node* t,int* oldaddr, bitvector* v, int size,int* order);
int deleteTree(Node* p); /* delete the whole tree */
void orderlabels(Node* t,int* order);
void insertvector(Node* t,int* oldaddr, bitvector* v, int size,int* order);
Node* addChild3(Node* p, int label, int* order);
int binsearch(int a, int b, int* tab, int olabel,int* order);
void printtree(Node* t);
void printpossibleandbest(Node *p,int *oldaddr,int l);
#endif
