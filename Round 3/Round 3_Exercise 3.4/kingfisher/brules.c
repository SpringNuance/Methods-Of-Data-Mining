/* brules.c */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <values.h>
#include <float.h>
#include "measures.h"
#include "trie.h"
#include "error.h"
#include "brules.h"
#include "glob.h"

/* reserve space and initialize structures for the Q best rules */
void initBStructures(bitvector ***brules,int Q,int *lastQ,int **cons, int **xfreq, int **xafreq, float **bestMval,int **bestsigns){
  int i;

  /* bit vectors for Q best rules */
  if (((*brules)=(bitvector**)malloc(Q*sizeof(bitvector*)))==NULL)
    error(6);
  /* for consequent attributes, frequencies and best chi2 */
  if (((*cons)=(int*)malloc(Q*sizeof(int)))==NULL)
    error(6);
  if (((*xfreq)=(int*)malloc(Q*sizeof(int)))==NULL)
    error(6);
  if (((*bestMval)=(float*)malloc(Q*sizeof(float)))==NULL)
    error(6);
  if (((*xafreq)=(int*)malloc(Q*sizeof(int)))==NULL)
    error(6);
  if (((*bestsigns)=(int*)malloc(Q*sizeof(int)))==NULL)
    error(6);

  for (i=0; i<Q; i++){
    (*cons)[i]=-1;
    (*xfreq)[i]=0;
    (*xafreq)[i]=0;
    (*bestMval)[i]=INIT;
    (*bestsigns)[i]=1.0; /* by default, positive rules */
    (*brules)[i]=bv_alloc(k);
    bv_clr((*brules)[i],k);
  }
  (*lastQ)=-1;
}

/* check if the rule is among best rules and add it; update statistics */
void addrule(float val,Node *p,Node *par,int cons,int sign,int *kandi,int l){
  int fr,i;
  int ok=0;

  /* If only rules containing interesting attributes are asked, check that */
  /* the rule contains them. */
  if (intergiven){
    for (i=0;i<l;i++)
      if (interattr[kandi[i]]){
	ok=1; break;
      }
    if (ok==0) return;
  }

  if (sign==0)
    fr=par->data->fr-p->data->fr;
  else fr=p->data->fr;
  if (insertBestfromNode(val,cons,par->data->fr,fr,kandi,l,k,BRULES,Q,&LASTQ,CONS,XFR,XAFR,MVAL,BSIGNS,sign)==1){
    ADDEDRULES++;      
  }
  if (better(val,BESTVAL)==1)
    BESTVAL=val;
  TOTRULES++;
}


/* add a rule X->A to brules, if among Q best rules. Given M value of the */
/* the rule, consequent A, its sign (0=neg, 1=pos), m(XA), m(X), and table */
/* kandi, which lists all attributes in X, and its length l */
/* for both pos. and neg. rules; signs are in table bestsign */
int insertBestfromNode(float val,int cons, int xfreq, int xafreq,int *kandi,int l,int k,bitvector **brules,int Q,int *lastQ,int *bcons, int *bxfreq, int *bxafreq, float *bestMval,int *bestsigns,int sign){
  float cf;

  /* if already Q rules and the last is better than X->A, don't add */
  /* assumption: smaller M-values better */
  if ((*lastQ==Q-1)&&(better(val,bestMval[*lastQ])==0))
    return 0;

  /* if the confidence of the rule is too low, don't add */
  cf=((float)xafreq)/xfreq;
  if (cf<mincf)
    return 0;

  /* else add rule*/
  insertrule2(val,cons,xfreq,xafreq,kandi,k,l,brules,Q,lastQ,bcons,bxfreq,bxafreq,bestMval,bestsigns,sign);


  /* update threshold */
  if ((*lastQ==Q-1)&&(better(bestMval[*lastQ],valM)==1)){
    valM=bestMval[*lastQ];

  }
  return 1;
}



/* return 1, if insertion succeeded, otherwise 0 */
/* for both pos. and neg. rules; signs are in table bestsign */
int insertrule2(float val,int cons, int xfreq, int xafreq,int* kandi,int k, int l,bitvector **brules,int Q,int *lastQ,int *bcons, int *bxfreq, int *bxafreq,float *bestMval,int *bestsigns,int sign){
  int i;
  bitvector *uusi;
  /* temporary variables for moving values one step further */
  bitvector *tmp1;
  bitvector *tmp2;
  int xafr1, xafr2;
  int xfr1, xfr2;
  int cons1, cons2;
  float Mval1, Mval2;
  int sign1,sign2;


  /* create a bitvector for the rule and read from kandi */
  uusi=bv_alloc(k);
  bv_clr(uusi,k);
  for (i=0;i<l;i++)
    bv_setbit(uusi,kandi[i]);
  tmp1=uusi;

  /* determine cons, neg, bfr, cf, gamma, and lnp */
  cons1=cons;
  Mval1=val;
  xafr1=xafreq;
  xfr1=xfreq;
  sign1=sign;

  /* search the position for the new rule */
  if (*lastQ>=0){
    i=0;
    /*  while ((i<=(*lastQ))&&(Mval1>=bestMval[i]))*/
    while ((i<=(*lastQ))&&(better(Mval1,bestMval[i])==0))
      i++;
    
    /* now position is found */
    /* move all rules one step further */
    while (i<=(*lastQ) && i<Q){
      /* save old values */
      tmp2=brules[i]; 
      cons2=bcons[i];
      xafr2=bxafreq[i];
      xfr2=bxfreq[i];
      Mval2=bestMval[i];
      sign2=bestsigns[i];
      /* insert new values */
      brules[i]=tmp1;
      bcons[i]=cons1;
      bxafreq[i]=xafr1;
      bxfreq[i]=xfr1;
      bestMval[i]=Mval1;
      bestsigns[i]=sign1;
      i++;
      /* move old values for inserting variables */
      tmp1=tmp2;
      cons1=cons2;
      xafr1=xafr2;
      xfr1=xfr2;
      Mval1=Mval2;
      sign1=sign2;
    } 
  } 

  /* put the last rule into its position; this adds also the first rule to */
  /* empty table */
  if ((*lastQ)<Q-1){
    (*lastQ)++;
    brules[*lastQ]=tmp1;
    bcons[*lastQ]=cons1;
    bxafreq[*lastQ]=xafr1;
    bxfreq[*lastQ]=xfr1;
    bestMval[*lastQ]=Mval1;
    bestsigns[*lastQ]=sign1;
  }
  return 1;
}


void printresults(FILE *f, int Q,int lastQ,int pstyle,bitvector **brules,int *bcons, int *bxafreq,int *bxfreq, float *bestMval, int *bestsigns,int *fr){
  int i,j;
  /* for calculating statistics: */
  float *cf;
  float *gamma;
  float *delta;

  if (lastQ<0)
    return;


  if ((cf=(float*)malloc((lastQ+1)*sizeof(float)))==NULL)
    error(6);
  if ((gamma=(float*)malloc((lastQ+1)*sizeof(float)))==NULL)
    error(6);
  if ((delta=(float*)malloc((lastQ+1)*sizeof(float)))==NULL)
    error(6);
  for (i=0;i<=lastQ; i++){
    cf[i]=0.0;
    gamma[i]=0.0;
    delta[i]=0.0;
  }


  /* print best rule */
  for (i=0; i<=lastQ; i++){
    for (j=0; j<k; j++){ 
      if (bv_tstbit(brules[i],j)){
	if (j!=bcons[i]) /* print condition part */
	  fprintf(f,"%d ",j);
      }
    } /* for j, condition part printed */
    if (pstyle==1)
      fprintf(f,"-> ");
    if (bestsigns[i]==0)
      fprintf(f,"~");
    /* print consequent attribute */
    fprintf(f,"%d",bcons[i]);
    
    /* calculate cf, gamma and delta */
    cf[i]=((float)bxafreq[i])/bxfreq[i];
    if (bestsigns[i]==0){
      gamma[i]=(((float)n)*bxafreq[i])/(((float)bxfreq[i])*(n-fr[bcons[i]]));
      delta[i]=(((float)bxafreq[i])-((float)bxfreq[i])*(n-fr[bcons[i]])/((float)n))/((float)n);
    }
    else {
      gamma[i]=(((float)n)*bxafreq[i])/(((float)bxfreq[i])*fr[bcons[i]]);
      delta[i]=(((float)bxafreq[i])-((float)bxfreq[i])*fr[bcons[i]]/((float)n))/((float)n);
    }

    /* in pstyle=1 print also Mvalue, freq, cf, lift and leverage */    
    if (pstyle==1)
      fprintf(f," fr=%d (%.4f), cf=%.3f, gamma=%.3f, delta=%.3f, M=%.3e\n",bxafreq[i], bxafreq[i]/((float)n),cf[i],gamma[i],delta[i],bestMval[i]);
    else fprintf(f,"\n");
  } /* for i */
  
  printf("Totally %d rules, when mincf=%.2f and threshold-M=%.2e\n",lastQ+1,mincf,valM);
  
  if (lastQ<9)  calctatistics(lastQ+1,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
      if(lastQ>=9)
	calctatistics(10,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
      if(lastQ>=49)
	calctatistics(50,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
      if(lastQ>=99)
	calctatistics(100,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
      if(lastQ>=499)
	calctatistics(500,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
      if ((lastQ!=9)&&(lastQ!=49)&&(lastQ!=99)&&(lastQ!=499))
	calctatistics(lastQ+1,bxafreq,cf,gamma,delta,bestMval,bestsigns,n);
}


/* calculate statistics among the first num rules */
void calctatistics(int num,int *bfr,float *cf, float *gamma, float *delta,float *bestMval,int *bestsigns,int n){ 
  int i;
  float gsum, dsum, cfsum,avgg,avgd,avgcf; 
  float frsum, avgfr,Msum, avgM;
  int nrules=0; /* number of neg. rules */

  /* mean values */
  gsum=0.0; dsum=0.0; cfsum=0.0;frsum=0.0;Msum=0.0;
  for (i=0;i<num;i++){
    cfsum+=cf[i]; 
    gsum+=gamma[i];
    dsum+=delta[i];
    frsum+=bfr[i];
    Msum+=bestMval[i];
    if (bestsigns[i]==0)
      nrules++;
  }
  avgcf=cfsum/num;
  avgg=gsum/num; 
  avgd=dsum/num;
  avgfr=frsum/num;
  avgM=Msum/num;

  
  /* stdev */
  
  gsum=0.0; dsum=0.0; cfsum=0.0;frsum=0.0;Msum=0.0;
  for (i=0;i<num;i++){
    cfsum+=pow(cf[i]-avgcf,2);
    gsum+=pow(gamma[i]-avgg,2);
    dsum+=pow(delta[i]-avgd,2);
    frsum+=pow(bfr[i]-avgfr,2);
    Msum+=pow(bestMval[i]-avgM,2);
  }


  printf("Among %d best rules %d negative rules, avg(M)=%.2e stdev(M)=%.2e\n  avg(cf)=%.3f stdev(cf)=%.3f avg(gamma)=%.3f stdev(gamma)=%.3f\n  avg(delta)=%.3f stdev(delta)=%.3f avg(fr)=%.1f stdev(fr)=%.1f\n", num,nrules,avgM,sqrt(Msum/num),avgcf,sqrt(cfsum/num),avgg,sqrt(gsum/num),avgd,sqrt(dsum/num),avgfr,sqrt(frsum/num));
}
