/**********************************************************************/
/* measures.h  Measure functions and constants                        */
/* This and measures.c are the only place where you have to make      */
/* modifications if you want to add your own measures.                */
/**********************************************************************/

#ifndef mesurili
#define mesurili

/**********************************************************************/
/* Absolute minimum frequency. m(X->A) should be >=MIN for all rules. */
/* You can use even MIN=0, if you want. Higher minimum frequency      */
/* thresholds are given on the command line.                          */
/**********************************************************************/
#define MINMFR 5

extern char MEASURES[10][30];
#define NMeasures 4 /* number of implemented measures - update, if you */
/* add a new measure */

void initializeMeasure(int Mnum); 
/* initialize the functions and values for your own measure here         */
/* (in measures.c). Then just add decalarations for your own functions   */
/* below and implement them in measures.c                                */
/*************************************************************************/

void initlnfact(int nn);
double lnfactorial(int m);
double lnprod(int m, int l);
double lnmoverl(int m, int l);
int detminfrx(int fra, int nn, double lnmaxp, int ind,double *tmpright,double *tmpleft);
int defineabsminfr(double lnmaxp, int nn);
int detMIabsminfr(double valM, int nn);
double lnpabs(int afr, int nn);
double lnfirstterm(int frxa, int frx, int fra, int nn);
double lnubfactor(int frxa, int frx, int fra, int nn);
/*double exactfact(int frxa, int frx, int fra, int nn);*/
double exactlnfact(int frxa, int frx, int fra, int nn);
double exactlnp(int frxa, int frx, int fra, int nn);
void updatelnp(double prevlnp, double newlnp, int *fr, int *oldaddr);
double qfactor(int m,int frxa, int frx, int fra, int nn);
double lnmthterm(int m,int frxa, int frx, int fra, int nn);
double lnlbmoverl(int m, int l);
/* chi2 measure */
double chi2val(int frxa, int frx, int fra, int nn);
/* upper bounds for chi2 */
double ub2chi2(int frx,int fra,int nn);
double ub3chi2(int frxa,int frx,int fra,int nn);

double MI(int frxa, int frx, int fra, int nn);
double ub2MI(int frx,int fra,int nn);
double ub3MI(int frxa,int frx,int fra,int nn);
double specMI(int frx,int frq,int fra,int frxa,int frqa,int frxq,int frxqa,int nn);
#endif
