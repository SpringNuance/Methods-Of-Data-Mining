/* glob.c */
#include "glob.h"
/*#include <stdlib.h>*/
#include <values.h>

int n;
int k;
int Q=100; /* number of best rules to be printed (default can be changed) */
/* on the command line */
int MAXTASO=50; /* The last level tells how deep the search continues. The */
/* most specific rule can contain up to MAXTASO attributes including the */
/* the consequent. Can be changed on the command line */    
int CCORR=1; /* by default, continuity correction with chi2 */
float mincf=0.0; /* minimum cf and frequency*/
float minfr=0.0; 
int nminfr=0;
int absminfr=0; /* no attribute can have smaller frequency */
int *specialattr; /* mark special attributes which cannot occur in the cons */
double maxp; 
double valM=0.0; /* threshold for M, when not pF or ln(pF) */
int *posabsminfrx; /* absolute minfr values for m(X) such that X->A  */
/* can be significant */
int *negabsminfrx; /* absolute minfr values for m(X) such that X->~A can */
/* be significant */
double *abses;  /* absolute lowerbounds ln(p_abs) for all possible */
/* cons. attr., i.e. ln(m(A)!m(~A)!/n!) */
double *maxlnrx;   /* for all possible consequent attr. A,  ln of maximum */
/* r_x vlues for any significant rules X->A or X->~A i.e. such that */
/* ln(r_x)*lnpabs[A]<=maxlnp i.e. maxlnrx[i]=maxlnp/abses[i] */
/* Notice! The same for A and ~A */
double *postmpright; /* for storing right and left values in detminfrx and */
double *postmpleft;  /* updatelnp */
double *negtmpright; 
double *negtmpleft; 
int attnum; /* number of attributes which are used (1-sets in t) */
/* by defult, both pos. and neg. rules are searched */
int POS=1;
int NEG=1;
/* The following are set in measures.c; just default values below. */ 
double INIT=DBL_MAX; /* initial value for best; poorest possible; */
/* with Fisher's p, you can use DBL_MAX */
int INC=0; /* is the measure increasing or decreasing? */
double EPSILON=0.000001; /* how much precision error allowed? */
/* default for ln(p_F); too large for p, ok for chi2 and MI */
int intergiven=0;
LYHYT *interattr; /* interesting attributes marked by 1 bits */
LYHYT *ATTUSED; /* for controlling if uninteresting attributes needed*/

/* function pointers */ 

/* calculates exact or estimated ln(1+q1+q1q2+...+q1...qm) */
double (*lnqterms)(int frxa, int frx, int fra, int nn);
int (*detabsminfr)(double valM,int nn);
int defaultdetabsminfr(double valM, int nn){
  return 1;
}

double (*MEASUREVAL)(int frxa,int frx, int fra, int nn); /* measure value */

/* upper or lower bounds */
double (*BOUND2)(int frx,int fra,int nn);
double (*BOUND3)(int frxa,int frx,int fra,int nn);

void (*updatevalM)(double prevvalM, double newvalM, int *fr, int *oldaddr);
void defaultupdatevalM(double prevvalM, double newvalM, int *fr, int *oldaddr){
}

/* comparing two values */
int (*better)(double a1, double a2);

/* implementation of better: */
/* use this for decreasing goodness measures like p_F */
int better1(double a1,double a2){
  if (a1<a2) return 1;
  else return 0;
}

/* use this for inreasing goodness measures like chi2 */
int better2(double a1,double a2){
  if (a1>a2) return 1;
  else return 0;
}
