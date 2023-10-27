/* brules.h */
/* structures for keeping record on the best rules */
#ifndef bruleli
#define bruleli

#include "ibitvector.h"

int LASTQ; /* last used index in the following tables */
bitvector **BRULES; /* bit vectors for rules */
float *MVAL; /* their M values */
int *XFR,*XAFR; /* their frequencies */
int *CONS; /* consequent attributes */
int *BSIGNS; /* their signs; pos. or neg. rule */
/* records on the discovered rules at each level */
float BESTVAL; /* the best M-value */
int TOTRULES; /* number of non-red. sign. rules found */
int ADDEDRULES; /* number of rules added to the best Q rules */ 

void initBStructures(bitvector ***brules,int Q,int *lastQ,int **cons, int **xfreq, int **xafreq, float **bestMval,int **bestsigns);
int insertBestfromNode(float val,int cons, int xfreq, int xafreq,int *kandi,int l,int k,bitvector **brules,int Q,int *lastQ,int *bcons, int *bxfreq, int *bxafreq, float *bestMval,int *bestsigns,int sign);
int insertrule2(float val,int cons, int xfreq, int xafreq,int* kandi,int k, int l,bitvector **brules,int Q,int *lastQ,int *bcons, int *bxfreq, int *bxafreq,float *bestMval,int *bestsigns,int sign);
void addrule(float val,Node *p,Node *par,int cons,int sign,int *kandi,int l);


/* saada seuraavat myös neg. saannnoille! */

void printresults(FILE *f, int Q,int lastQ,int pstyle,bitvector **brules,int *bcons, int *bxafreq,int *bxfreq, float *bestMval, int *bestsigns,int *fr);
void calctatistics(int num,int *bfr,float *cf, float *gamma, float *delta,float *bestMval,int *bestsigns,int n);

#endif
