/**********************************************************************/
/* measures.c  Measure functions and constants                        */
/* This and measures.h are the only place where you have to make      */
/* modifications if you want to add your own measures.                */
/**********************************************************************/

#include <math.h>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>
#include "measures.h"
#include "error.h"
#include "glob.h"

/***********************************************************************/
/* The following table lists all implemented goodness measures. If you */
/* implement a new measure, add its name and update the number of      */
/* implemented measures. The current table allows up to 10 measures    */
/* and the name of the measure can be up to 20 characters; update      */
/* table dimensions if needed.                                         */
/***********************************************************************/
char MEASURES[10][30]={
  "Fisher's pF", /* option -w1 */
  "ln(pF)", /* option -w2 (default) */
  "chi2",   /* option -w3 etc. */
  "mutual information (MI)",
  "not implemented",
  "not implemented",
  "not implemented",
  "not implemented",
  "not implemented",
  "not implemented"};


/***********************************************************************/
/* Initialize the functions and values for your own measure here. Then */
/* just add he declarations of your own functions to measures.h and    */
/* implement them below. Parameter Mnum is the number of the measure   */
/* given in option -w.                                                 */  
/* Important: tell if your measure is increasing or decreasing by      */
/* goodness; i.e. does a high value indicate a good or bad rule? E.g.  */
/* Fisher's pF is decreasing (INC=0) and chi2 increasing (INC=1).      */
/***********************************************************************/

void initializeMeasure(int Mnum){
  
  if (Mnum<=NMeasures)
    printf("Using measure %s\n",MEASURES[Mnum-1]);
  else {
    printf("Measure number %d not implemented!\n",Mnum);
    error(0);
  }
  
  switch(Mnum){
  case 1:{
    INC=0; /* decreasing measure */
    INIT=DBL_MAX; /* set poorest possible value */
    EPSILON=0.001; /* how much precision error is allowed when M(X->A) */
    /* is compared to more general M(Y->A)? */
    /* Check that threshold valM is legal */
    if ((valM<=0)||(valM>=1.0)){
      printf("Maximal significance level maxp (option p) should be in [%.2e,1], in practice <0.05. If you want to use smaller values than %.2e, take natural logarithm and use option -w2\n",MINDOUBLE,MINDOUBLE);
      error(0);
    }
    maxp=valM;
    valM=log(maxp);
    printf("Using threshold p=%.3e i.e. maxlnp=%.3e\n",maxp,valM);
    detabsminfr=defineabsminfr;
    updatevalM=updatelnp;
    break;
  }
  case 2:{
    INC=0;
    INIT=DBL_MAX;
    EPSILON=0.001;
    if ((valM>=0.0) || (valM<=(-1)*MAXDOUBLE)){
      printf("Natural logarithm of maximal significance level ln(maxp) (option -w2) should be in ]-%.2e,0.0], in practice <-3.0 = ln(0.05)\n",MAXDOUBLE);
      error(0);
    }
    detabsminfr=defineabsminfr;
    updatevalM=updatelnp;
    break;
  }
  case 3: { /* chi2 */
    INC=1;
    INIT=0.0;
    EPSILON=0.01;
    if ((valM<=0.0)||(valM>n)){
      printf("Minimal chi2 value (option -w3) should be in ]0,n]\n"); 
      error(0);
    }
    updatevalM=defaultupdatevalM; /* most measures use this */ 
    MEASUREVAL=chi2val; /* function for evaluating the measure */
    BOUND2=ub2chi2; /* functions for upper/lower bounds */
    BOUND3=ub3chi2;
    detabsminfr=defaultdetabsminfr; /* if measure allows to determine */
    /* a minimum frequency threshold, call it here */
    break;
  }
    
  case 4: { /* MI */
    INC=1; 
    INIT=0.0;
    EPSILON=0.001;
    
    updatevalM=defaultupdatevalM; /* most measures use this */ 
    
    MEASUREVAL=MI; /* function for evaluating the measure */
    BOUND2=ub2MI; /* functions for upper/lower bounds */
    BOUND3=ub3MI;
    detabsminfr=detMIabsminfr; /* MI allows to determine */
    /* a minimum frequency threshold */
    
    break;
  }
    
    /* add your own case statement here */
  }


  
  if (INC==0){ /* decreasing measures */
    better=better1;
    /* changed 3.5. 2015 */
    /*    EPSILON=-1*EPSILON;  set negative */
    EPSILON=-1*EPSILON;  /*changed back 13.4.16 */
  }
  else { /* increasing measures */
    better=better2;
    /* changed 3.5. 2015 */
    /*  EPSILON=-1.0*EPSILON; */
    /* removed again 13.4.16 */
    
  }
}


/*********************************************************************/
/* No more changes are needed after this point. Just add the         */
/* implementation for your own measures.                             */
/*********************************************************************/

double *alllnfactorials;

/* calculate all ln(m!) for i=0,...,nn */
void initlnfact(int nn){
  int i;

  if ((alllnfactorials=(double*)malloc((nn+1)*sizeof(double)))==NULL)
    error(6);

  alllnfactorials[0]=0.0; /* ln(0!)=ln(1)=0 */
  for (i=1;i<=nn;i++)
    alllnfactorials[i]=alllnfactorials[i-1]+log(i);
}


/* Assumption:  float should be enough when l is taken */

/* calculates ln(m!) */
double lnfactorial(int m){

  if (m<0){
    printf("m<0 in lnfactorial\n");
    error(9); 
  }

  if (m>n){
    printf("m>n in lnfactorial\n");
    error(9); 
  }

  return (alllnfactorials[m]);

}


double lnmoverl(int m, int l){

  if ((m==l)||(l==0)) return 0.0; 
  if (m<l){
    printf("In lnmoverl l=%d>%d=m\n",l,m);
    error(9);
  }

  if (m>n){
    printf("m=%d>n=%d (lnmoverl)\n",m,n);
    error(9);
  } 

  return (alllnfactorials[m]-alllnfactorials[l]-alllnfactorials[m-l]);
}


/* calculates ln(l*(l+1)*...m) */
double lnprod(int m, int l){
  int i;
  double sum=0.0;

  if (l>m) {
    printf("In lnprod l>m\n");
    error(9);
  }
  if (l==m) return log(l);
  for (i=l; i<=m; i++){
    sum+=log(i);
    if (isfinite(sum)==0) /* overflow */ 
      error(16);
  }
  return sum;
}

/* determine minimum abs. frequency m(X) such that rule X->A=a can be */
/* significant at level maxp. Parameters: fra=m(A=a), nn=n data size. */
/* lnmaxp=ln(maxp), ind tells where right and left values are stored  */
/* in tmpright and tmpleft. */
/* Condition: m(~X)!/(m(A=a)-m(X))!<=maxp* n!/m(A=a)! */
/* When called for positive consequent, give postmpright and postmpleft and */
/* for negative consequent negtmpright and negtmpleft. */
int detminfrx(int fra, int nn, double lnmaxp, int ind,double *tmpright,double *tmpleft){
  /*  int i; */
  double left=0.0; /* log of left hand side in expression */
  double right=0.0;
  int frx=fra-1; /* frx<fra, first value to check is fra-1 */

  /* right hand side is constant */
  /* ln(n!/m(A=a)!) 
  for (i=fra+1;i<=nn;i++){
    right+=log(i);
    if (isfinite(right)==0) 
      error(16);
      } 
      right+=lnmaxp;*/

  right=alllnfactorials[nn]-alllnfactorials[fra]+lnmaxp;

  if (isfinite(right)==0)
    error(16);

  tmpright[ind]=right; /* when frx is updated, this remains the same */

  /* m(~X)!/(m(A)-m(X))!=m(~X)*...*(m(A)-m(X)+1) */
  /* try first with frx=fra-1. Now fra-frx+1=2 */
  /* for (i=2;i<=nn-frx;i++){
    left+=log(i);
    if (isfinite(left)==0) 
      error(16);
      }*/

  left=alllnfactorials[nn-fra+1]-alllnfactorials[2];

  if (left>right) return fra; /* frx cannot be smaller */
  while (frx>0){
    /* try smaller frx */
    /* update left by multiplying with (m(~X)+1)/(m(A)-m(X)+1) 
    left+=(log(nn-frx+1)-log(fra-frx+1));
    if (isfinite(left)==0)
      error(16);*/

    left=alllnfactorials[nn-frx+1]-alllnfactorials[fra-frx+1];
    if (left>right) return frx;
    frx--;
  }
  tmpleft[ind]=left; /* left can be used, when frx is updated */
  return frx; /* now frx==1 */
}

/* define minimal m(A) such that m(A)!m(~A)!/n!<=maxp <=> */
/* ln(m(A)!m(~A)!)<=ln(n!*maxp) */
/* if even (n/2)!(n/2)!>n!maxp, then return special value -1 */
/* Begin from ma=1 and increase it, as long as the condition */
/* holds. */
int defineabsminfr(double lnmaxp, int nn){
  /*  float sum=0.0;*/
  double left,right;
  int ma=1;
  /* int i;*/

  /*  for (i=1;i<=ma;i++)
      sum+=(log(i)-log(nn-i+1));

  while ((sum>lnmaxp)&&(ma<=nn/2)){
    ma++;
    sum+=(log(ma)-log(nn-ma+1));

    if (isfinite(sum)==0){
      printf("Overflow, sum is not finite (defineabsminfr)\n");
      error(16);
    }
    }*/

  right=alllnfactorials[nn]+valM; /* ln(n!*maxp) */
  left=alllnfactorials[1]+alllnfactorials[nn-1]; /* ln(1!(n-1)! */
  while ((left>right)&&(ma<nn/2)){
    ma++;
    left=alllnfactorials[ma]+alllnfactorials[nn-ma];
  }

  /* if ma>n/2, return special value -1 */  
  if (ma>nn/2)
    return -1;
  return ma;
}


/* calculates absolute lowerbound for ln(p) of any rule X->A or X->~A */
/* = ln(m(A)!m(~A)!/n!) */ 
double lnpabs(int afr, int nn){
  int i;
  double sum=0.0;
  double tmp=1.0;
  double tmp2;

  /* m(A)>m(~A), reverse m(A) and m(~A) for calculation */
  if (2*afr>nn) 
    afr=nn-afr;

  /* calculate multiplying doubles, if possible - faster */
   if (isfinite(pow((double)nn-afr+1,afr))){
     for (i=0;i<afr;i++)
       tmp*=((double)nn-i)/(afr-i);
     tmp2=log(tmp);
     if (tmp2>DBL_MAX){
       printf("moverl does not fit to double\n");
       error(16);
     }
     sum=-1.0*tmp2;
   }
   
   
   else {
     sum=0.0;
     for (i=1; i<=afr; i++){
       sum+=log(((double)i)/(nn-afr+i));      
       if (isfinite(sum)==0){
	 printf("sum is not finite! (lnabs)\n");
	 error(16);
       }
     }  
   }


   return sum;
}

/* calculates ln(t0) for the first term in Fisher's p   */
/* p=p_abs*(t0+...+tm). this gives a lowerbound for ln(r_x) */
double lnfirstterm(int frxa, int frx, int fra, int nn){
  double sum=0.0;

  /*  printf("calculate ln(%d over %d)+ln(%d over %d)\n",frx,frxa,nn-frx,nn-frx-fra+frxa);*/

  /* m(~X~A)=n-m(X)-m(A)+m(XA) */

  sum=lnmoverl(frx,frxa)+lnmoverl(nn-frx,nn-frx-fra+frxa);

  if (isfinite(sum)==0){
    printf("sum is not finite! (lnfirstterm)\n");
    error(16);
  }

  return sum;
}

/* UB(ln(p))=lnfirstterm+lnubfactor. Returns  */
/* ln(1+(1-ag-xg+xag^2)/(g-1)), where g=lift, a=P(A) */
/* x=P(X), or P(XA)P(~X~A)/(P(XA)-xa) */
double lnubfactor(int frxa, int frx, int fra, int nn){
  double tmp,tmp1;
  double tmp2,tmp3;

  /* if not positive dependency, return maxfloat */
  if ((double)nn*frxa<=(double)frx*fra){
    printf("not a positive dependency! (lnubfactor)\n");
    error(9); /* should not occur */
  }

  tmp1=((double)frxa)/nn*(nn-fra-frx+frxa);
  tmp3=((double)nn*frxa-frx*fra)/nn;
  tmp2=tmp1/tmp3;
  tmp=log(tmp2);

  if (isfinite(tmp)==0){
    printf("tmp is not finite! tmp1=%.5e tmp3=%.2e tmp2=%.2e,frxa=%d,frx=%d,fra=%d m(~X~A)=%d n=%d n*frxa=%d frx*fra=%.1e (lnubfactor)\n",tmp1,tmp3,tmp2,frxa,frx,fra,nn-fra-frx+frxa,nn,nn*frxa,((double)frx)*fra);
    error(16); 
  }

  if (tmp>=DBL_MAX){
    printf("tmp does not fit to double (lnubfactor)\n");
    error(16);
  }
  return tmp;

}

/* calculates exact factorial qsum=1+q1+q1q2+...+q1q2...qm such that */
/* p_F= p_abs*firstterm*qsum. qi=(m(X~A)-i+1)(m(~XA)-i+1)/((m(XA)+i)(m(~X~A)+i)) */
/* Notice: returns ln(p_F) */

double exactlnfact(int frxa, int frx, int fra, int nn){
  double frxnega=((double)frx)-frxa;
  double frnegxa=((double)fra)-frxa; 
  double frnegxnega=((double)nn)-fra-frx+frxa;
  int m,i; 
  long double ed=1.0; 
  long double sum=1.0;
  long double qi;
  long double tmp;


  if ((frxa==frx)||(frxa==fra))
    /* return 1.0;  corrected 8.12.10; should be ln(1.0)=0.0 */
    return 0.0; /* special case */

  if (frxnega<=frnegxa) m=frxnega;
  else m=frnegxa;

  for (i=1;i<=m;i++){
    /* define qi */
    qi=((double)frxnega-i+1)*(frnegxa-i+1)/((frxa+i)*(frnegxnega+i));
    ed*=qi; /* q1*...*qi */
    sum+=ed;

    if (isfinite(ed)==0){
      printf("ed not finite! i=%d qi=%Le\n",i,qi);
      error(16);
    }

    if (isfinite(sum)==0){
      printf("sum not finite! i=%d qi=%Le ed=%Le\n",i,qi,ed);
      error(16);
    }

    /*    if (qi<0.0){
      printf("i=%d sum=%Le ed=%Le qi=%Le (exactlnfact) frxa=%d,frx=%d,fra=%d,n=%d\n",i,sum,ed,qi,frxa,frx,fra,nn);
      error(16);
      }*/

  }

  if (isfinite(sum)==0){
    printf("Sum is not finite! (exactlnfact) frxa=%d,frx=%d,fra=%d,n=%d\n",frxa,frx,fra,nn);
    error(16);
  }

  tmp=logl(sum);
  if (tmp>=DBL_MAX){
    printf("Does not fit to double! (exactlnfact)\n");
    error(16);
  }

  if (isfinite(tmp)==0){
    printf("tmp not finite, sum=%Le (exactlnfact)\n",sum);
    error(16);
  }

  return ((double)tmp);
}


/* calculate exact lnp */
double exactlnp(int frxa, int frx, int fra, int nn){
  double result;

  double q=exactlnfact(frxa,frx,fra,nn);
  double pabs=lnpabs(fra,nn);
  double lnfirst=lnfirstterm(frxa, frx, fra, nn);

  /*  printf("q=%.3e,lnpabs=%.3e, lnfirst=%.3e\n",q,pabs,lnfirst);*/

  result=q+pabs+lnfirst;
  if (result>=DBL_MAX){
    printf("Does not fit to double (exactlnp)\n");
    error(16);
  }
  return result;
}


/* update maxlnrx and absminfrx when the current lnp is updated */
/* separate tables for postmpright, postmpleft, negtmpright, negtmpleft,*/
/* posabsminfrx, negabsminfrx */
void updatelnp(double prevlnp, double newlnp, int *fr, int *oldaddr){
  int i, fra, frx;
  double left=0.0; 
  double right=0.0;

  for (i=0;i<attnum;i++)
    /* check only normal attributes */
    if (specialattr[oldaddr[i]]==0){
      /* first check if attr. became special */
      if (abses[i]>newlnp){
	specialattr[oldaddr[i]]=1;
	abses[i]=0.0;
	posabsminfrx[i]=n; 
	negabsminfrx[i]=n;
	maxlnrx[i]=-1*FLT_MAX;
      }
      else { /* remained as normal attr. */
	/* update maxlnrx; for normal attributes maxlnrx[i]=maxlnp-abses[i] */
	/* old maxlnrx[i]=prevlnp-abses[i], should be newlnp-abses[i]*/
	maxlnrx[i]+=newlnp-prevlnp; 
	/* update absminfrx */
	/* first neg. attr. */
	/* update negtmpright */
	negtmpright[i]+=(newlnp-prevlnp);
	right=negtmpright[i];
	left=negtmpleft[i];
	frx=negabsminfrx[i]; /* old value */
	fra=n-fr[oldaddr[i]];
	/* updating */
	while ((frx<fra)&&(left>right)){
	  left+=(log(n-frx+1)-log(fra-frx+1));
	  frx++;
	}
	negabsminfrx[i]=frx; /* new value */
	negtmpleft[i]=left; /* new left-value */

	/* then pos. attr. */
	/* update tmpright */
	postmpright[i]+=(newlnp-prevlnp);
	right=postmpright[i];
	left=postmpleft[i];
	frx=posabsminfrx[i]; /* old value */
	fra=fr[oldaddr[i]];
	/* updating */
	while ((frx<fra)&&(left>right)){
	  left+=(log(n-frx+1)-log(fra-frx+1));
	  frx++;
	}
	posabsminfrx[i]=frx; /* new value */
	postmpleft[i]=left; /* new left-value */

      } /* remained normal */
    } /* normal attr. */
}


/* return q_l=(m(X~A)-m+1)(m(~XA)-m+1)/((m(XA)+m)(m(~X~A)+m)) */ 
double qfactor(int m,int frxa, int frx, int fra, int nn){

  return ((double)frx-frxa-m+1)*((double)fra-frxa-m+1)/(((double)frxa+m)*(nn-fra-frx+frxa+m));
}

/* return ln of the m:th term in r_x, i.e. ln[(m(X) over m(XA)+m)(m(~X) over */
/* m(~X~A)+m)] */
double lnmthterm(int m,int frxa, int frx, int fra, int nn){

  return lnmoverl(frx,frxa+m)+lnmoverl(nn-frx,nn-frx-fra+frxa+m);

}

/* a lower bound for ln(m over l) */
double lnlbmoverl(int m, int l){
  double tmp;

  if ((m==l)||(l==0)) return 0.0; /* ln(1) */
   if (m<l){
     printf("In lnmoverl l=%d>%d=m (lnlbmoverl)\n",l,m);
    error(9);
  }

 if (l==1) return log((double)m);

 if (m-l>l)
   tmp=(m-l)*log(((double)m)/(m-l));
 else tmp=l*log(((double)m)/l);

  if (isfinite(tmp)==0){
    printf("Overflow\n");
    error(16);
  }
  return tmp;
}


double chi2val(int frxa, int frx, int fra, int nn){
  long double tmp;
  long double pxa=((long double)frxa)/nn;
  long double px=((long double)frx)/nn;
  long double pa=((long double)fra)/nn;

  if ((frx==nn)||(frx==0)||(fra==nn)||(fra==0)){
    printf("Error in chi2 frxa=%d frx=%d fra=%d\n",frxa,frx,fra);
    error(9);
  }

  if (CCORR==0)
    tmp=nn*pow(pxa-px*pa,2)/(px*(1-px)*pa*(1-pa));
  else 
    tmp=nn*pow(pxa-px*pa-0.5/nn,2)/(px*(1-px)*pa*(1-pa));

  if (tmp>nn+0.000001) printf("chi2=%Lf>%d=n frxa=%d frx=%d fra=%d (chi2val)\n",tmp,nn,frxa,frx,fra);

  return ((double)tmp);
}

double ub2chi2(int frx,int fra,int nn){

  if ((frx==nn)||(fra==0)||(frx==0)||(fra==nn)){
    printf("Error in ub2chi2 frx=%d fra=%d (measures.c)\n",frx,fra);
    error(9);
  }

  if (CCORR==0){
    if (frx>=fra) return ((double)nn); /* maximum */
    /* return (((double)frx)*(((double)nn)-fra)/((((double)nn)-frx)*(fra)))*nn;*/
    return chi2val(frx,frx,fra,nn);
  }
  else { /* continuity correction */
    if (frx>=fra) return chi2val(fra,fra,fra,nn);
    else return chi2val(frx,frx,fra,nn);
  }
}


double ub3chi2(int frxa,int frx,int fra,int nn){

  if (frxa==0) return 0.0;
  if ((frxa==nn)||(fra==0)||(fra==nn)){
    printf("Error in ub3chi2 frxa=%d fra=%d frx=%d (measures.c)\n",frxa,fra,frx);
    error(9);
  }
  return chi2val(frxa,frxa,fra,nn);
  /*if (CCORR==0)
    return (((double)frxa)*(((double)nn)-fra)/((((double)nn)-frxa)*(fra)))*nn;
  else 
  return chi2val(frxa,frxa,fra,nn);*/
}

/* mutual information */
 double MI(int frxa, int frx, int fra, int nn){
   double sum=0.0;
   
  if ((frx==nn)||(frx==0)||(fra==nn)||(fra==0)){
    printf("Error in MI frxa=%d frx=%d fra=%d\n",frxa,frx,fra);
    error(9);
  }

  if (frxa>0) sum+=frxa*log2f((double)frxa);
  if (frxa<frx) sum+=(frx-frxa)*log2f((double)(frx-frxa));
  if (frxa<fra) sum+=(fra-frxa)*log2f((double)(fra-frxa));
  if (frxa>frx+fra-nn) sum+=(nn-frx-fra+frxa)*log2f((double)nn-frx-fra+frxa);
  sum-=(frx)*log2f((double)frx)+(nn-frx)*log2f((double)nn-frx)+(fra)*log2f((double)fra)+(nn-fra)*log2f((double)nn-fra);
  sum+=nn*log2f((double)nn);

  if (isfinite(sum)==0){
    printf("sum is not finite (MI in measures.c)\n");
    error(9);
  }

  return sum;
 }

double ub2MI(int frx,int fra,int nn){
  double tmp=0.0;

  if ((frx==nn)||(fra==0)||(frx==0)||(fra==nn)){
    printf("Error in ub2MI frx=%d fra=%d (measures.c)\n",frx,fra);
    error(9);
  }

  if (fra<=frx) /* best possible value, non-informative ub */
    tmp=(nn*log2f((double)nn)-fra*log2f((double)fra)-(nn-fra)*log2f((double)nn-fra));
  else /* frx<fra */
    tmp=(nn*log2f((double)nn)+(fra-frx)*log2f((double)fra-frx)-(nn-frx)*log2f((double)nn-frx)-fra*log2f((double)fra));

 if (isfinite(tmp)==0){
    printf("sum is not finite (ub2MI in measures.c)\n");
    error(9);
  }
 return tmp;

 }

double ub3MI(int frxa,int frx,int fra,int nn){
  double tmp=0.0;

  if ((frx==nn)||(frx==0)||(fra==nn)||(fra==0)){
    printf("Error in ub3MI frxa=%d frx=%d fra=%d\n",frxa,frx,fra);
    error(9);
  }
  
  /*tmp=(nn*log2f((double)nn)+(fra-frxa)*log2f((double)fra-frxa)-fra*log2f((double)fra)-(nn-frxa)*log2f((double)nn-frxa));*/

 tmp=nn*log2f((float)nn)-fra*log2f((float)fra)-(nn-frxa)*log2f((float)nn-frxa);
  if (frxa<fra)
    tmp+=(fra-frxa)*log2f((float)fra-frxa);


  if (isfinite(tmp)==0){
    printf("tmp=%e is not finite (ub3MI in measures.c)\n",tmp);
    error(9);
  }

  return tmp;
  
}

/* Determine minimal frequency of a single attribute, F(A)<n/2, such that */
/* a rule involving A could be significant. */
/* Begin from ma=1 and increase it, as long as the condition */
/* holds. */
int detMIabsminfr(double valM, int nn){
  int ma=1;
  double val=-1.0*FLT_MAX;
  double valtmp=valM-nn*log2f((double)nn);
  
  while ((ma<nn/2)&&(val<valtmp)){
    val=(-1.0*ma*log2f((double)ma)-(((double)nn)-ma)*log2f(((double)nn)-ma));
    ma++;
  }
  
  /* if ma>n/2, return special value -1 */  
  if (ma>nn/2)
    return -1;
  return ma;
}
