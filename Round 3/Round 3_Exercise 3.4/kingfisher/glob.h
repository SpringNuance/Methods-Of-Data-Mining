 /* glob.h */

#ifndef globuli
#define globuli

#include <stdio.h>
#include "constr.h"

extern int n;
extern int k;
extern int Q;
extern int MAXTASO;
extern int CCORR;
extern float mincf;
extern float minfr;
extern int nminfr;
extern int absminfr; 
extern int *specialattr; 
extern double maxp; 
extern double valM;
extern int *posabsminfrx; 
extern int *negabsminfrx; 
extern double *abses; 
extern double *maxlnrx;  
extern double *postmpright; 
extern double *postmpleft;  
extern double *negtmpright; 
extern double *negtmpleft;  
extern int attnum; 
extern int POS;
extern int NEG;
extern double INIT;
extern int INC; 
extern double EPSILON;
extern int intergiven;
extern LYHYT *interattr;
extern LYHYT *ATTUSED;

extern double (*lnqterms)(int frxa, int frx, int fra, int nn);
/*extern void (*process1sets)(Node *t,int num, int *fr, int *oldaddr);
extern int (*prunecand)(Node *t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr);
extern void (*checkrule)(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l);
extern void (*checkNullFr)(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l);
extern void (*checkLowFr)(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l);*/

extern int (*detabsminfr)(double valM,int nn);
extern int defaultdetabsminfr(double valM, int nn);

extern double (*MEASUREVAL)(int frxa,int frx, int fra, int nn);

extern double (*BOUND2)(int frx,int fra,int nn);
extern double (*BOUND3)(int frxa,int frx,int fra,int nn);

extern void (*updatevalM)(double prevvalM, double newvalM, int *fr, int *oldaddr);
extern void defaultupdatevalM(double prevvalM, double newvalM, int *fr, int *oldaddr);

extern int (*better)(double a1, double a2);
extern int better1(double a1, double a2);
extern int better2(double a1, double a2);
#endif
