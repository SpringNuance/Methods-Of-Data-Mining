/*********************************************************************/
/* kingfisher.c  v1.2 8.8. 2012 Wilhelmiina H‰m‰l‰inen               */
/* searches the best non-redundant statistical dependency rules      */
/* of form X->A or X->~A expressing positive dependence between X    */
/* and A or ~A. Any well-behaving goodness measure can be used.      */
/* By default, uses Fisher's exact test.                             */
/*********************************************************************/ 
/*
  (C) Copyright 2010 by Wilhelmiina H‰m‰l‰inen

  The code can be freely used for academic/research purposes. Direct
  or indirect use for commercial advantage is not allowed without
  written permission from the author.
  
  The code can be modified and redistributed if this note is left
      intact.
    
  Reference:   
  H‰m‰l‰inen, W.: Efficient discovery of the top-K optimal dependency
  rules with Fisher's exact test of significance. Proceedings of the 
  10th IEEE International Conference on Data Mining
  (ICDM 2010), pp. 196-205 IEEE Computer Society 2010.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <float.h>
#include <values.h>
#include "bitmatrice.h"
#include "trie.h"
#include "error.h"
#include "measures.h"
#include "apufunktiot.h"
#include "glob.h"
#include "brules.h"
#include "constr.h"

/* for recording the execution time */
time_t tim1,tim2;
double tottim=0.0;
int TESTED=0;
int FREV=0;

LYHYT **constraints; /* compatibility constraints */
int eok=0; /* by default, no comp. constraints (option -e) */

void checkrulepF(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l);
int prunecandpF(Node* t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr);
void checkNullFrpF(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l);
void checkLowFrpF(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l);
void markParents(Node *p, int *order);
int interchildren(Node *p);

int certainkandi(int *kandi,int l){
   if ((l>=2)&&(((kandi[0]==464)&&(kandi[1]==747))||((kandi[0]==464)&&(kandi[1]==470))||((kandi[0]==747)&&(kandi[1]==470))))
    return 1;

  return 0;
}

/* frequency counting given the attribute matrix */
/* set contains the attribute labels which correspond the bitvectors */
/* all of them should be >=0 ! */
int frset(int *set,int len,bitvector **amatr){
  int i,fr=0;
  bitvector *tulos; 

  if (len<1) return 0;
  tulos=bv_alloc(n);
  /* initialize: results =  1. attribute vector */
  if (set[0]<0) {
    printf("Attribute label in set[0]<0! (frset)\n");
    error(9);
  }

  bv_copy(tulos,amatr[set[0]],bv_size(n));
  
  /* bit-and of all attribute vectors in the set */
  for (i=1;i<len;i++){
    if (set[i]<0) {
      printf("Attribute label in set[%d]<0! (frset)\n",i);
      error(9);
    }
    if (amatr[set[i]]==NULL){
      printf("Attr. A%d removed from the bitmatrics!\n",set[i]);
      error(9);
    }
    bv_and(tulos,amatr[set[i]],bv_size(n));
  }

  /* number of 1-bits in the result = abs. frequency */
  fr=bv_popcnt(tulos,bv_size(n));

  /*  printf("fr=%d\n",fr);*/
  bv_free(tulos);
  return fr;
}

void printset(int* set, int size){
  int i;
  for (i=0; i<size; i++)
    printf("A%d ",set[i]);
  printf("\n");

}


void printset2(bitvector* kandi){
  int i;

  for (i=0; i<k; i++)
    if (bv_tstbit(kandi,i))
      printf("A%d ",i);
  printf("\n");
}

void printbvector(bitvector *table,int l){
  int i; 
 for (i=0; i<l; i++)
    if (bv_tstbit(table,i))
      printf("1 ");
    else printf("0 ");
  printf("\n");
}



/* generate freq. sets of 1 attr. and insert into tree; return the number of */
/* added attrs. */

int generate1sets(bitvector** amatr, Node *t,int **fr,int **oldaddr,int **order,int *special){
  int i,j,min,position;
  Node* p;
  int ok,next;
  int* tmp;
  int nol=0;
  /* float maxpos;*/

  (*special)=0; /* number of special attributes */
  /* special attributes are marked into table specialattr */
  if ((specialattr=(int*)malloc(k*sizeof(int)))==NULL)
    error(6);

  if ((tmp=(int*)malloc(k*sizeof(int)))==NULL)
    error(6);

  for (i=0; i<k; i++){
    tmp[i]=0; 
    specialattr[i]=0;
  }

  if (((*fr)=(int*)malloc(k*sizeof(int)))==NULL)
    error(6);

  for (i=0; i<k; i++)
    (*fr)[i]=0;

  /* count frequencies of single attributes */

  for (j=0; j<k; j++)
    for (i=0; i<n; i++)
      if (bv_tstbit(amatr[j],i)) 
	(*fr)[j]++;

  

  /* testi: tulosta frekv */
  /*    printf("attr. frevenssit\n");*/ 

  /* prune A, if m(A)<absminfr or m(A)=n */
    for (i=0; i<k; i++){

      /*      if ((intergiven)&&(interattr[i]))
	      printf("m(A%d)=%d P=%.8f\n",i,(*fr)[i],(*fr)[i]/((float)n));*/

      if (((*fr)[i]<absminfr)||((*fr)[i]==n)){ 
	nol++;
	(*fr)[i]=0;
	/* remove the corresponding bit vector */
	bv_free(amatr[i]);
	  amatr[i]=NULL;
      }
      /* check if A is special attr. */
      else
        if (n-(*fr)[i]<absminfr){
	  (*special)++;
	  specialattr[i]=1;
	}
      tmp[i]=(*fr)[i];
    }

    tim2=clock();
    tottim+=(double)(tim2-tim1);

    printf("Removing %d attributes (P(A)<%.6f=minfr or P(A)=1), left %d attributes\n",nol,minfr,k-nol);

    printf("Data matrix takes %d bits (%.1f MB) space; freed %d bits (%.1f MB)\n",n*(k-nol),n*(((float)k)-nol)/(8.0*1024.0*1024.0),n*nol,n*((float)nol)/(8.0*1024.0*1024.0));


    tim1=clock();
    /* oldaddr only for attributes with fr>0 */
    if (((*oldaddr)=(int*)malloc((k-nol)*sizeof(int)))==NULL)
    error(6);
    if (((*order)=(int*)malloc((k)*sizeof(int)))==NULL)
    error(6);

    for (i=0; i<(k-nol); i++){
      (*oldaddr)[i]=-1;
    }

    for (i=0; i<(k); i++){
      (*order)[i]=-1;
    }
    
    /* add attributes in the order of their frequency */
   
    ok=0; next=0;
    while(!ok){
      min=n; position=-1;
      for (i=k-1; i>=0; i--) /* next smallest attr */
	if (((tmp)[i]>0)&&((tmp)[i]<min)){
	  min=(tmp)[i];
	  position=i;
	}
      if (position==-1){ 
	break; /* all added */
      }
      /* else printf("minfr=%d minp=%.5f\n",min,min/((float)n));*/
      
      /* ascending order */
      (*oldaddr)[next]=position; 
      (*order)[position]=next; 
      next++;
      tmp[position]=0; /* remove attribute */
    } /* while */


   /* add attributes to the tree in an ascending order */
    /*      printf("Attributes in order\n"); */

   for (i=0; i<k-nol;i++){
     p=addChild(t,(*oldaddr)[i],*order);
     p->data->fr=(*fr)[(*oldaddr)[i]];
     initbestandpossible(p,(*fr),(*oldaddr),(*order),k-nol);

     /*  printf("label %d fr=%d\n",p->label,p->data->fr);*/

   }
   /*  printf("\n");*/

    free(tmp); 

  return k-nol; 
}



/* determine type values for 1-sets */
/* num=number of attributes in the tree */
/* tsizes is already set to numl-special */
void detType1sets(Node *t,int num, int *fr, int *oldaddr){
  int i,j,s;

  if ((abses=(double*)malloc(num*sizeof(double)))==NULL)
    error(6); /* the same values for A and ~A */
  if ((posabsminfrx=(int*)malloc(num*sizeof(int)))==NULL)
    error(6); 
  if ((negabsminfrx=(int*)malloc(num*sizeof(int)))==NULL)
    error(6); 
  if ((maxlnrx=(double*)malloc(num*sizeof(double)))==NULL)
    error(6); /* the same values for A and ~A */
 
  /* the following are used when maxlnp dependent values are */
  /* updated */
  if ((postmpright=(double*)malloc(num*sizeof(double)))==NULL)
    error(6);
  if ((postmpleft=(double*)malloc(num*sizeof(double)))==NULL)
    error(6);
 if ((negtmpright=(double*)malloc(num*sizeof(double)))==NULL)
    error(6);
  if ((negtmpleft=(double*)malloc(num*sizeof(double)))==NULL)
    error(6);
 /* 1. calculate absolute lowerbounds ln(p_abs) for all possible cons. */
  /* attr. A_j to abses[i], j=oldaddr[i] <=> i=order[j] */
  /* 2. define absolute minfr values for m(X), given m(A=a), such that */
  /* X->A=a can be signifigicant. For cons. A_j the position is */
  /* posabsminfrx[i]  and for ~A_j negabsminfrx[i], where j=oldaddr[i] */
  /* 3. define ln of maximum r_x values for any significant rules */
  /* X->A or X->~A i.e. such that ln(r_x)*lnpabs[A]<=maxlnp */
  /* maxlnrx[i]=maxlnp/abses[i] */

  for (i=0;i<num;i++){
    postmpright[i]=0.0;
    negtmpright[i]=0.0;
    postmpleft[i]=0.0;
    negtmpleft[i]=0.0;
    /*  printf("attr no %d, label %d\n",i, oldaddr[i]);*/
    if (specialattr[oldaddr[i]]){
      /*      printf("%d is special\n",i);*/
      abses[i]=0.0; /* i.e. p=1.0 */
      posabsminfrx[i]=n;  /* no rule is possible */
      negabsminfrx[i]=n;
      maxlnrx[i]=-1*DBL_MAX; /* no rx value is possible */
    }
    else { /* normal attributes */
      abses[i]=lnpabs(fr[oldaddr[i]],n);
      /*      printf("abs=%.2e\n",abses[i]);*/
      /* detminfrx sets tmpright and tmpleft values, given index where */
      /* right and left values are stored */
      posabsminfrx[i]=detminfrx(fr[oldaddr[i]],n,valM,i,postmpright,postmpleft);
      negabsminfrx[i]=detminfrx(n-fr[oldaddr[i]],n,valM,i,negtmpright,negtmpleft);
      maxlnrx[i]=valM-abses[i]; /* r_x=maxp/p_abs */

      /*      printf("A%d abses=%.2f maxlnrx=%.2f\n",oldaddr[i],abses[i],maxlnrx[i]);*/

    }
  }

  /* define possible consequents */

  /* first check special attr */
  for (i=num-1;i>=0;i--)
    /* if special attr, mark type NAV to all nodes j<>i */
    if (specialattr[oldaddr[i]]){
      for (j=0;j<num;j++)
        if (j!=i){
	  if (POS)
	    setposnav(t->children[j],i);
	  if (NEG)
	    setnegnav(t->children[j],i);
        }
    }

  /* then go through all possible positive consequents from the most */
  /* frequent but non-special attr. to least frequent and define */
  /* type values for other nodes (if A_i is possible in node A_j). */
  /* Notice! min[A=a]-value is LB(r_x), where p(X->A=a)=r_x*p_abs[A] */

  if (POS){
    for (i=num-1;i>=0;i--) /* check attr. A_i */
      if (t->children[i]!=NULL) {
	if (specialattr[oldaddr[i]]==0){  /* normal attr */
	  for (j=num-1;j>=0;j--) /* check condition part , A_j*/
	    if (t->children[j]!=NULL){
	      /* set ppossible[i] in t->children[j] */
	      /* if  m(A_j)>=m(A_i), A_i is possible in A_j */
	      /* nothing to do, because ppossible[i] has been initialized */
	      /* to 1 */
	      if (fr[oldaddr[j]]<fr[oldaddr[i]]){ /* m(A_j)<m(A_i) */
		/* if m(A_j) too small, all m(A_s), s<j, are too small */
		if (fr[oldaddr[j]]<posabsminfrx[i]){
		  for (s=j;s>=0;s--)
		    if (t->children[s]!=NULL)
		      /* set type to nav in t->children[s] */
		      setposnav(t->children[s],i);
		  break; /* next i */
		}
	      }
	    } /* for j */
	} /* when normal attr */
      } /* for i */
  }


  /* then go through all possible negative consequents from the most */
  /* frequent cons. to least frequent and define type for other nodes */

  if (NEG){
    /* m(~Ai)s are in desc. order, when m(Ai)s are in asc. order */
    for (i=0;i<num;i++)
      if (t->children[i]!=NULL){
	if (specialattr[oldaddr[i]]==0) { /* when A_i is normal attr. */
	  for (j=num-1;j>=0;j--) /* check condition part */
	    if (t->children[j]!=NULL){
	      /* set npossible[i] t->children[j] */
	      /* nothing to do if m(A_j)>=m(~A_i) */         
	      if (fr[oldaddr[j]]<n-fr[oldaddr[i]]) /* when m(A_j)<n-m(A_i) */
		/* if m(A_j) too small, all m(A_s), s<j, are too small */
		if (fr[oldaddr[j]]<negabsminfrx[i]){
		  for (s=j;s>=0;s--)
		    /* keep type as AV in the node A_i itself */
		    if ((t->children[s]!=NULL)&&(s!=i)){
		      /* set type to nav in t->children[s] */
		      setnegnav(t->children[s],i);
		      /* printf("~A%d impossible in A%d\n",oldaddr[i],oldaddr[s]);*/
		    }
		  break; /* next i */
		}
	    } /* for j */
	} /* when A_i is normal attr. */
      } /* for i */
  }



  /* Now all possible-values have been defined and possible-tables can be */
  /* reduced */
 for (i=0;i<t->chsize;i++)
   if (t->children[i]!=NULL){
      reducepossible(t->children[i]);
      /*  printpossibleandbest(t->children[i],oldaddr,1);*/
  }
}


/* the same for other measures than pF */ 
void detType1setsM(Node *t,int num, int *fr, int *oldaddr){
  int i,j,s;
  double ub2=INIT;

  /* first check special attr */
  for (i=num-1;i>=0;i--)
    /* if special attr, mark type NAV to all nodes j<>i */
    if (specialattr[oldaddr[i]]){
      for (j=0;j<num;j++)
        if (j!=i){
	  if (POS)
	    setposnav(t->children[j],i);
	  if (NEG)
	    setnegnav(t->children[j],i);
        }
    }

  /* then go through all possible positive consequents and define */
  /* type values for other nodes (if A_i is possible in node A_j). */

  if (POS){
    for (i=num-1;i>=0;i--) /* check attr. A_i */
      if (t->children[i]!=NULL) {
	if (specialattr[oldaddr[i]]==0){  /* normal attr */
	  for (j=num-1;j>=0;j--) /* check condition part , A_j*/
	    if (t->children[j]!=NULL){
	      /* set ppossible[i] in t->children[j] */
	      /* use UB2; no pruning if m(A_j)>=m(A_i) */
	      if (fr[oldaddr[j]]<fr[oldaddr[i]]){ /* m(A_j)<m(A_i) */
		ub2=BOUND2(fr[oldaddr[j]],fr[oldaddr[i]],n);
		/* if m(A_j) too small, all m(A_s), s<j, are too small */
		if (better(valM,ub2)==1){
		  for (s=j;s>=0;s--)
		    if (t->children[s]!=NULL)
		    /* set type to nav in t->children[s] */
		    /* A_i remains AV in node A_i itself */
		    setposnav(t->children[s],i);
		  break; /* next i */
		}
	      }
	    } /* for j */
	} /* when normal attr */
      } /* for i */
  }



  /* then go through all possible negative consequents */

  if (NEG){
    for (i=0;i<num;i++)
      if (t->children[i]!=NULL){
	if (specialattr[oldaddr[i]]==0) { /* when A_i is normal attr. */
	  for (j=num-1;j>=0;j--) /* check condition part */
	    if (t->children[j]!=NULL){
	      /* set npossible[i] t->children[j] */
	      /* use UB2; nothing to do if m(A_j)>=m(~A_i) */         
	      if (fr[oldaddr[j]]<n-fr[oldaddr[i]]){ /* when m(A_j)<n-m(A_i) */
		/* if m(A_j) too small, all m(A_s), s<j, are too small */
		ub2=BOUND2(fr[oldaddr[j]],n-fr[oldaddr[i]],n);		
		if (better(valM,ub2)==1){
		  for (s=j;s>=0;s--)
		    /* keep type as AV in the node A_i itself */
		    if ((t->children[s]!=NULL)&&(s!=i)){
		      /* set type to nav in t->children[s] */
		      setnegnav(t->children[s],i);
		      		      
		    }
		  break; /* next i */
		}
	      }
	    } /* for j */
	} /* when A_i is normal attr. */
      } /* for i */
  }



  /* Now all possible-values have been defined and possible-tables can be */
  /* reduced */
 for (i=0;i<t->chsize;i++)
   if (t->children[i]!=NULL){
      reducepossible(t->children[i]);
      /*  printpossibleandbest(t->children[i],oldaddr,1);*/
  }
}


/* Disable consequences according to compatibility constraints. */
/* If constraints[i][j]=1, then Aj and ~Aj impossible consequences for Ai */
/* and vice versa. */
void addcompconstr(Node *t,LYHYT **constraints,int k,int *order){
  int i,j;

  for (i=0;i<t->chsize;i++)
    if (t->children[i]!=NULL){
      for (j=0;j<k;j++)
	if (order[j]>=0){ /* if ordernum==-1, Aj has been pruned out */
	  if (constraints[t->children[i]->label][j]==1){
	    /* set Aj and ~Aj as impossible consequences */
	    setposnav(t->children[i],order[j]);
	    setnegnav(t->children[i],order[j]);
	  }
	}
    }
}

/* Extra constraints: each row i lists forbidden consequences for XAi */ 
void addextraconstr(Node *t,LYHYT **extraconstr,int k,int *order){
  int i,j;

  for (i=0;i<t->chsize;i++)
    if (t->children[i]!=NULL){
      for (j=0;j<k;j++)
	if (order[j]>=0){ /* if ordernum==-1, Aj has been pruned out */
	  if (extraconstr[t->children[i]->label][j]==1){
	    /* set Aj and ~Aj as impossible consequences */

	    /* printf("Sets A%d NAV in node for A%d\n",j,t->children[i]->label);*/

	    setposnav(t->children[i],order[j]);
	    setnegnav(t->children[i],order[j]);
	  }
	}
    }
}


/* Extra pruning of possible consequences when interesting attributes given. */
/* Only those cons. which are possible in at least one inter. attr. can be */
/* possible in uninteresting attributes. */
void prunewhenintergiven(Node *t,LYHYT *interattr, int k){
  int i,j;
  bitvector *tmppos, *tmpneg;
  int maxp=0; int maxn=0;
  Node *apu;

  for (i=0;i<t->chsize;i++)
    if ((t->children[i]!=NULL)&&(interattr[t->children[i]->label])){
      if (t->children[i]->data->pposlen>maxp) maxp=t->children[i]->data->pposlen;
      if (t->children[i]->data->nposlen>maxn) maxn=t->children[i]->data->nposlen;
    }

  tmppos=bv_alloc(maxp);
  tmpneg=bv_alloc(maxn);


  /* Voisi t‰ytt‰‰ tmp-vektorit bittiorilla kiinnostavista. Sitten k‰yd. */
  /* ei-kiinnostavat ja karsitaan turhat bittiandilla tmp:n kanssa. */

  for (i=0;i<maxp;i++){
    for (j=0;j<t->chsize;j++)
      if ((t->children[j]!=NULL)&&(interattr[t->children[j]->label]))
	if ((t->children[j]->data->pposlen>i)&&(bv_tstbit(t->children[j]->data->ppossible,i))){
	  bv_setbit(tmppos,i);
	  break;
	}
  }

  for (i=0;i<maxn;i++){
    for (j=0;j<t->chsize;j++)
      if ((t->children[j]!=NULL)&&(interattr[t->children[j]->label]))
	if ((t->children[j]->data->nposlen>i)&&(bv_tstbit(t->children[j]->data->npossible,i))){
	  bv_setbit(tmpneg,i);
	  break;
	}
  } 


    /* temporary node */
  apu=createNode(-1); 
  apu->data->ppossible=tmppos;
  apu->data->pposlen=maxp;
  apu->data->npossible=tmpneg;
  apu->data->nposlen=maxn;

  for (i=0;i<t->chsize;i++)
    if ((t->children[i]!=NULL)&&(interattr[t->children[i]->label]==0)){
      updatepossible(t->children[i],apu);
    }



  free(tmppos); free(tmpneg);
  }


/* Given the list of possible consequences, disable all other */
/* consequences. */
void prunecons(Node *t,LYHYT *consattr,int k,int *order){
  int i,j;

  for (i=0;i<t->chsize;i++)
    if (t->children[i]!=NULL){
      for (j=0;j<k;j++)
	if ((consattr[j]==0)&&(order[j]>=0)){ 
	  /* Notice: if ordernum==-1, Aj has been pruned out */
	  /* set Aj and ~Aj as impossible consequences */
	  setposnav(t->children[i],order[j]);
	  setnegnav(t->children[i],order[j]);
	}
    }

  /* Now possible-tables can be reduced */
  for (i=0;i<t->chsize;i++)
    if (t->children[i]!=NULL){
      reducepossible(t->children[i]);
      /*  printpossibleandbest(t->children[i],oldaddr,1);*/
   }
}


/* generate a parent for kandi, containing all attributes except the ind:th */
/* and attr. a in the end. Length l-1 */
int* genpar(int* kandi, int ind, int a, int l){
  int i;
  int* par;

  if ((par=(int*)malloc((l-1)*sizeof(int)))==NULL)
    error(6);
  for (i=0; i<ind; i++)
    par[i]=kandi[i];
  for (i=ind; i<l-2; i++)
    par[i]=kandi[i+1];
  par[l-2]=a;
  return par;
}




/* check that all parents are in the tree and upate possible an best tables */
int checkpar(int* kandi, int l, Node* t, Node* child, int* order,int *fr){
  int a1, a2, i;
  int* par;
  Node* pt;


  a1=kandi[l-1];
  a2=kandi[l-2];

  /* first two parents already added! */
  for (i=l-3; i>0; i--){
    par=genpar(kandi,i,a1,l);

    /* if a parent is missing or only its own children (under it) could */
    /* be non-red., the node can be deleted */
    if (((pt=searchSet(par,l-1,t,order))==NULL)){
      free(par);
      return 0;
    }
    else { 
      addParent(child,pt,kandi[i]);
      updatepossible(child,pt);
      updatebest(child,pt,kandi,par,l);

    }
    free(par);
  }

  /* check the last parent; this can be a special parent with larger mina */
  if (l>=3){ /* otherwise a1 occurs twice! */
    par=genpar(kandi,0,a1,l);

    if ((pt=searchSet(par,l-1,t,order))==NULL) { 

      free(par);
      return 0;
    }
    else {
      addParent(child,pt,kandi[0]);
      updatepossible(child,pt);
      updatebest(child,pt,kandi,par,l);
    }
    free(par);
  } /* l>=3  */


  /* check if child has any possible consequets left */
  reducepossible(child);
  if (((NEG==0)&&(child->data->pposlen==0))||((POS==0)&&(child->data->nposlen==0))||
      ((child->data->pposlen==0)&&(child->data->nposlen==0))){

    /*if (certainkandi(kandi,3)){
      printf("ei yhtaan mahd. seurosaa!");
      }*/

    return 0;
  }
  return 1; /* all needed parents in the tree & can be PS */
}



/* traverse all paths of length l-1 and generate candidate l-sets from */
/* sister nodes; check all parents */ 
int visit(Node* p, Node* t,int len, int l, int* next, int** kandi,int* order,bitvector **amatr,int *fr,int *oldaddr){
  int i,j;
  Node *child;
  int *par2set;

  /*  int q;*/

  if (len==l-2){ /* path length l-2, i.e. we are in a leaf (do nothing) or */
    /* in a parent of a leaf */

    /* create candidates from all pair of sisters, unless compatibility */
    /* constraints forbid it */
    if (p->chsize>=2)
      for (i=0; i<(p->chsize)-1; i++)
	if (p->children[i]!=NULL){
	  for (j=i+1; j<(p->chsize); j++)
	    if (p->children[j]!=NULL){
	    /* Constraints added in v1.2 */   
	      if (((eok==0)||(constraints[p->children[i]->label][p->children[j]->label]==0))){
		(*kandi)[len]=p->children[i]->label; 
		(*kandi)[len+1]=p->children[j]->label; 
		
		child=addChild(p->children[i],p->children[j]->label,order);

		/*if (certainkandi(*kandi,l)) {		
		  printf("lis‰t‰‰n kandi\n");
		   printset(*kandi,l);
		   }*/
		
		addParent(child,p->children[i],p->children[j]->label);
		addParent(child,p->children[j],p->children[i]->label);
		/* initalize possible and best-tables */
		copypossible(child,p->children[i]);
		copybest(child,p->children[i],l);
		
		/*if (certainkandi(*kandi,l)) 
		  printpossibleandbest(child,oldaddr,l);*/

		/* update with the second parent's information */
		/* now we need the kandi set for parent 2 */
		par2set=genpar(*kandi,l-2,p->children[j]->label,l);
		updatepossible(child,p->children[j]);
		
		updatebest(child,p->children[j],*kandi,par2set,l);
		free(par2set);

		/*if (certainkandi(*kandi,l)) 
		  printpossibleandbest(child,oldaddr,l);*/

		/* check parents; if 0, then remove candidate */
		/* first l parents have been added, but this */
		/* checks if any consequents were left */
		/* if ((l>2)&&(checkpar(*kandi,l,t,child,order,fr)==0)){*/
		if (checkpar(*kandi,l,t,child,order,fr)==0){

		  /* if (certainkandi(*kandi,l)) {		  
		   printf("poistetaan checkparin jalkeen ");
		   printset(*kandi,l);}*/
		  
		  /* add info to parents before removing */
		  
		  markParents(child, order); 
		  delChild(p->children[i],(p->children[j]->label),order);
		  (*kandi)[len]=-1;(*kandi)[len+1]=-1;
		}
		else { (*next)++; /* remains */
		  
		  /* printf("j‰‰ puuhun\n");
		     printpossibleandbest(child,oldaddr,l);*/
		  
		  /* check upperbounds and rules */
		  if (prunecand(t,child,amatr,l,fr,*kandi,order,oldaddr)==0){
		    
		    /* if (certainkandi(*kandi,l)) 		    
		       printf("poistetaan prunecandin jalkeen\n");*/ 
		   
		    /* if returns 0, child can be removed */
		    /* add info to parents before removing */
		    
		    markParents(child, order);
		    
		    delChild(p->children[i],(p->children[j]->label),order);
		    (*kandi)[len]=-1;(*kandi)[len+1]=-1;
		    (*next)--;
		  }
		  else { /* node child remains; parent info can be removed */
		    	    
		    /* if (certainkandi(*kandi,l)) {
		     printf("jaa prunecandin jalkeen\n"); 
		       printpossibleandbest(child,oldaddr,l);
		       }*/
		    
		    if (child->data->parsize>0){
		      
		      /* printf("vanhempien possiblet\n");
			 for (q=0;q<child->data->parsize;q++)
			 if (child->data->parents[q]!=NULL){
			 printf("parentista puuttuva attr %d , label %d ",child->data->parlabels[q],child->data->parents[q]->label);
			 printpossibleandbest(child->data->parents[q],oldaddr,l-1);
			 }*/
		      
		      
		      free(child->data->parents);
		      free(child->data->parlabels);
		      child->data->parsize=0;
		    }
		    
		  }
		} /* candidate remains */
	      }
	    } /* for j */


	  /* the parent's possible-tables may have been updated; update */
	  /* also remaining children's tables */
	  for (j=0;j<p->children[i]->chsize;j++)
	    if (p->children[i]->children[j]!=NULL){
	      child=p->children[i]->children[j];
	      updatepossible(child,p->children[i]);

	      /*   printf("sisarjoukot, label %d\n",child->label);
		   printpossibleandbest(child,oldaddr,l);*/


	      if (((NEG==0)&&(child->data->pposlen==0))||
		  ((POS==0)&&(child->data->nposlen==0))||
		  ((child->data->pposlen==0)&&(child->data->nposlen==0)))
		/* delChild(p->children[i],p->children[i]->children[j]->label,order); */
		  delChildGivenInd(p->children[i],j); /* muutos */
		/* (*next)--;  LISAYS, em. poistaa paljon */
	    }

	  /* compress children-tables in p->children[i] */
	  cleanCh(p->children[i]); /* tiivistet‰‰n lapsitaulut */


	  /* now data content can be removed from chilren[i] */
	  removeData(p->children[i]);
	  
	  /* if children[i] had no children, it can be removed */
	  /* nobody needs it anymore */
	  if (p->children[i]->chsize==0)
	    delChild(p,p->children[i]->label,order);
	  
	} /* p->children[i] existed  */  
    
    /* p's last child has never children and can be removed */
    /* remove extra from the p's  last child and */
    /* check if it can be removed */
    if (p->children[p->chsize-1]!=NULL){
	 delChild(p,p->children[p->chsize-1]->label,order);
    } /* last child */
    cleanCh(p); /* compact p's children table*/

    /* if p had no children, return 0 */
    if (p->chsize==0)
      return 0;
    else return 1;
  } /* if (len==l-1) */ 
  
  else {
    for (i=0; i<p->chsize; i++){ /* polku kesken, jos lapsia. */
      if (p->children[i]!=NULL){
	(*kandi)[len]=p->children[i]->label;
	if ((visit(p->children[i],t,len+1,l,next,kandi,order,amatr,fr,oldaddr)==0)&&(len>0))
	  delChild(p,p->children[i]->label,order);
      }
    } /* for */

    /* clean p and return 0, if it can be deleted */
    if (len>0) cleanCh(p); /* compact p's children table */
    /* if p had no children, return 0 */
    if (p->chsize==0)
      return 0;
    else return 1;
  } /* else */
}


/* The same when interesting attr. given. Parameter iinpath=1 if interesting */
/* attr. occurs in the path. */
int intervisit(Node* p, Node* t,int len, int l, int* next, int** kandi,int* order,bitvector **amatr,int *fr,int *oldaddr, int iinpath){
  int i,j;
  Node *child;
  int *par2set;
  int oldiinpath;
  /*  int q;*/

  if (len==l-2){ /* path length l-2, i.e. we are in a leaf (do nothing) or */
    /* in a parent of a leaf */
    /*      printf("polun pituus len %d, l=%d\n",len,l);*/

    /* create candidates from all pair of sisters, unless compatibility */
    /* constraints forbid it */
    oldiinpath=iinpath;

    /* Necessary condition: the path or p's children contains inter. attr. */

    /* not necessarily!!!!!!!! */




    if ((p->chsize>=2)&&((iinpath)||(interchildren(p))))
      for (i=0; i<(p->chsize)-1; i++)
	if (p->children[i]!=NULL){
	  if (interattr[p->children[i]->label]) iinpath=1;

	  /* Constraints added in v1.2 */
	  /* eok constraint: if constraints[Ai][Aj]=1, then forbidden */
	  for (j=i+1; j<(p->chsize); j++)
	    if ((p->children[j]!=NULL)&&((eok==0)||(constraints[p->children[i]->label][p->children[j]->label]==0))){

	      (*kandi)[len]=p->children[i]->label; 
	      (*kandi)[len+1]=p->children[j]->label; 
	      
	      child=addChild(p->children[i],p->children[j]->label,order);
	      
	      /*if (certainkandi(*kandi,l)) 
		{		
		printf("lis‰t‰‰n kandi\n");
		printset(*kandi,l);
		}*/
	      
	      addParent(child,p->children[i],p->children[j]->label);
	      addParent(child,p->children[j],p->children[i]->label);
	      /* initalize possible and best-tables */
	      copypossible(child,p->children[i]);
	      copybest(child,p->children[i],l);
	      
	      /*if (certainkandi(*kandi,l)) 
		printpossibleandbest(child,oldaddr,l);*/
	      
	      /* update with the second parent's information */
	      /* now we need the kandi set for parent 2 */
	      par2set=genpar(*kandi,l-2,p->children[j]->label,l);
	      updatepossible(child,p->children[j]);
	      
	      updatebest(child,p->children[j],*kandi,par2set,l);
	      free(par2set);
	      
	      /*if (certainkandi(*kandi,l)) 
		printpossibleandbest(child,oldaddr,l);*/
	      
	      /* check parents; if 0, then remove candidate */
	      /* first l parents have been added, but this */
	      /* checks if any consequents were left */
	      /* if ((l>2)&&(checkpar(*kandi,l,t,child,order,fr)==0)){*/
	      if (checkpar(*kandi,l,t,child,order,fr)==0){
		
		/*if (certainkandi(*kandi,l)) {		  
		   printf("poistetaan checkparin jalkeen ");
		   printset(*kandi,l);} */
		
		/* add info to parents before removing */
		
		markParents(child, order); 
		delChild(p->children[i],(p->children[j]->label),order);
		(*kandi)[len]=-1;(*kandi)[len+1]=-1;
	      }
	      else { (*next)++; /* remains */
		
		/* printf("j‰‰ puuhun\n");
		   printpossibleandbest(child,oldaddr,l);*/
		
		/* check upperbounds and rules */
		if (prunecand(t,child,amatr,l,fr,*kandi,order,oldaddr)==0){
		  
		  /* if (certainkandi(*kandi,l)) 		    
		     printf("poistetaan prunecandin jalkeen\n"); */
		  
		  /* if returns 0, child can be removed */
		  /* add info to parents before removing */
		  
		  markParents(child, order);
		  
		  delChild(p->children[i],(p->children[j]->label),order);
		  (*kandi)[len]=-1;(*kandi)[len+1]=-1;
		  (*next)--;
		}
		else { /* node child remains; parent info can be removed */
		  
		  
		  /* if (certainkandi(*kandi,l)) {
		     printf("jaa prunecandin jalkeen\n"); 
		     printpossibleandbest(child,oldaddr,l);
		     }*/
		  
		  
		  if (child->data->parsize>0){
		    
		    /* printf("vanhempien possiblet\n");
		       for (q=0;q<child->data->parsize;q++)
		       if (child->data->parents[q]!=NULL){
		       printf("parentista puuttuva attr %d , label %d ",child->data->parlabels[q],child->data->parents[q]->label);
		       printpossibleandbest(child->data->parents[q],oldaddr,l-1);
		       }*/
		    
		    
		    free(child->data->parents);
		    free(child->data->parlabels);
		    child->data->parsize=0;
		  }
		  
		} /* candidate remains */
	      }
	    } /* for j */


	  /* If the path contained no inter attr, check if any Ai's */
	  /* interesting children remained - otherwise all children */
	  /* can be removed. */

	  /*  if ((iinpath==0)&&(interchildren(p->children[i])==0)){
	        for (j=0;j<p->children[i]->chsize;j++)
	      if (p->children[i]->children[j]!=NULL){
		delChildGivenInd(p->children[i],j);  muutos 		
		(*next)--;  LISAYS 
		} 
	  }

	  else {  normal checking, if children can remain */

	    /* the parent's possible-tables may have been updated; update */
	    /* also remaining children's tables */
	    for (j=0;j<p->children[i]->chsize;j++)
	      if (p->children[i]->children[j]!=NULL){
		child=p->children[i]->children[j];
		updatepossible(child,p->children[i]);
		
		/*   printf("sisarjoukot, label %d\n",child->label);
		     printpossibleandbest(child,oldaddr,l);*/
		
		
		if (((NEG==0)&&(child->data->pposlen==0))||
		    ((POS==0)&&(child->data->nposlen==0))||
		    ((child->data->pposlen==0)&&(child->data->nposlen==0)))
		  delChildGivenInd(p->children[i],j); /* muutos */
		/* (*next)--;  LISAYS, em. poistaa paljon */
	      }
	    
	    /* compress children-tables in p->children[i] */
	    cleanCh(p->children[i]); /* tiivistet‰‰n lapsitaulut */
	  
	    /*}  else */
	    
	    
	  /* now data content can be removed from chilren[i] */
	  removeData(p->children[i]);
	  
	  /* if children[i] had no children, it can be removed */
	  /* nobody needs it anymore */
	  if (p->children[i]->chsize==0)
	    delChild(p,p->children[i]->label,order);
	 
	  iinpath=oldiinpath; /* return value to original before next A_i */
 	} /* p->children[i] existed (for i) */  
    
    /* p's last child has never children and can be removed */
    /* remove extra from the p's  last child and */
    /* check if it can be removed */
    if (p->children[p->chsize-1]!=NULL){
      delChild(p,p->children[p->chsize-1]->label,order);
    } /* last child */
    cleanCh(p); /* compact p's children table */
    
    /* if p had no children, return 0 */
    if (p->chsize==0)
      return 0;
    else return 1;
  } /* if (len==l-1) */ 
  
  else {
    oldiinpath=iinpath;
    for (i=0; i<p->chsize; i++){ /* polku kesken, jos lapsia. */
      if (p->children[i]!=NULL){
	/* Check if the path contained interesting attributes. */
	if (interattr[p->children[i]->label]) iinpath=1;
	(*kandi)[len]=p->children[i]->label;
	if ((intervisit(p->children[i],t,len+1,l,next,kandi,order,amatr,fr,oldaddr,iinpath)==0)&&(len>0))
	  delChild(p,p->children[i]->label,order);
	iinpath=oldiinpath;
      }
    } /* for */
    
    /* clean p and return 0, if it can be deleted */
    if (len>0) cleanCh(p); /* compact p's children table */
    /* if p had no children, return 0 */
    if (p->chsize==0)
      return 0;
    else return 1;
  } /* else */
}

/* generates l-sets and returns the number of them */
int generatecandidates(Node* t,int l,int* order,bitvector **amatr,int *fr,int *oldaddr){
  int i,iinpath=0; 
  int* kandi;
  int next=0;
  float prevvalM=valM;  
  
  if ((kandi=(int*)malloc(l*sizeof(int)))==NULL) error(6);
  for (i=0; i<l; i++)
    kandi[i]=-1;

  /* traverse tree and generate candidates */
  if (l==2){ /* level 2 */
    if (intergiven==0)
      visit(t,t,0,l,&next,&kandi,order,amatr,fr,oldaddr);
    else
      intervisit(t,t,0,l,&next,&kandi,order,amatr,fr,oldaddr,iinpath);


    /*if (maxlnp<prevlnp){
      updatelnp(prevlnp,maxlnp,fr,oldaddr);
      prevlnp=maxlnp;
      }*/

    if (better(valM,prevvalM)){
      updatevalM(prevvalM,valM,fr,oldaddr);
      prevvalM=valM;
    }
  }

  else {
    for (i=0;i<t->chsize;i++) /* for all branches */
      if (t->children[i]!=NULL){
	kandi[0]=t->children[i]->label;
	if (intergiven==0)
	  visit(t->children[i],t,1,l,&next,&kandi,order,amatr,fr,oldaddr);
	else {
	  if (interattr[t->children[i]->label]) 
	    intervisit(t->children[i],t,1,l,&next,&kandi,order,amatr,fr,oldaddr,1);
	  else intervisit(t->children[i],t,1,l,&next,&kandi,order,amatr,fr,oldaddr,0);
	}
	if (better(valM,prevvalM)){
	  updatevalM(prevvalM,valM,fr,oldaddr);
	  prevvalM=valM;
	}	
      }
  }

  free(kandi);
  return next; /* number of PS l-sets */
}


/* check rule Y->A=a, where fry=m(Y),frxa=m(XA=a),fra=m(A=a), cons = A, */
/* sign=0 if neg. rule and 1, if pos. rule. */
/* It has already been checked that in principle, the rule is AV, but */
/* not minimal (with cf=1.0) */
/* Notice: for cons ~A give fra=m(~A) and frxa=m(X~A) */
void checkrulepF(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l){
  float lb=0.0;
  float ub=0.0;
  int indinkandi=-1;
  int onum,onum2; /* order number */
  float ub1,ub2;

  /* if (certainkandi(kandi,l))
     printf("checkrule\n");*/

  onum=order[cons];
  /* check that wasn't NAV */
  if (((sign>0)&&(isposav(p,onum)==0))||((sign==0)&&(isnegav(p,onum)==0))) 
      return; 
  /* check that positive dependency */
  if ((double)n*frxa<=(double)frx*fra){
    return;
  }


  /* if cf<mincf, no reason to check further */
  if (frxa<mincf*frx)
    return;

  /* if (certainkandi(kandi,l))
     printf("checkrule, oli pos. riipp. ja seur.osa AV\n");*/


  /* index of cons in best-table is the same as its index in kandi */
  indinkandi=indexinset(cons,kandi,l);
  if (indinkandi<0){
    printf("Attr. A%d does not occur in kandi! (checkrule)\n",cons);
    error(9); /* should not occur */
  }

  /* first check lowerbound, ln of the first term */
  /* calculate only ln(firstterm), without pabs */

  /* changed 3.6. 10 */
  /*lb=lnlbmoverl(frx,frxa)+lnlbmoverl(n-frx,n-frx-fra+frxa);*/

  lb=lnfirstterm(frxa,frx,fra,n);

  /* if lb sufficiently small, calculate real p_X */

  /* positive rules and lb sufficiently low */
  if (sign>0){

    /* if (certainkandi(kandi,l))
       printf("lb=%.3e maxlnrx=%.3e best=%.3e\n",lb,maxlnrx[onum],p->pbest[indinkandi]+EPSILON);*/

    if (p->data->pbest==NULL)
      printf("pbest=null\n");
    if ((lb<=maxlnrx[onum])&&(lb<p->data->pbest[indinkandi]+EPSILON)){
      /* ub=lb+lnqterms(frxa,frx,fra,n);*/
      ub1=lnfirstterm(frxa,frx,fra,n);
      ub2=lnqterms(frxa,frx,fra,n);
      ub=ub1+ub2;

      if (isfinite(ub)==0)
	printf("Overflow in ub=%.2e+%.2e frxa=%d,frx=%d,fra=%d,n=%d (checkrule, pos rule)\n",ub1,ub2,frxa,frx,fra,n);


      /*  if (certainkandi(kandi,l))
	  printf("Pos. rule (cons=A%d) ub=%.3e maxlnrx=%.3e best=%.3e\n",cons,ub,maxlnrx[onum],p->data->pbest[indinkandi]);*/


      /* allow some error in precision */
      if ((ub<=maxlnrx[onum])&&(ub<p->data->pbest[indinkandi]+EPSILON)){

	if ((l==2)&&(p->data->pbest[indinkandi]<INIT+EPSILON))
	  return; /* the permutation rule A->B for B->A already found */

	p->data->pbest[indinkandi]=ub;

	/* given A->B, set best-value to B->A, too */
	if ((l==2)&&(indinkandi==1)){
	  onum2=order[kandi[0]];
	  /* defines r_x, when p_F is the same */
	  p->data->pbest[0]=ub+abses[onum]-abses[onum2];
	}
	
	/* notice: ub=ln(r_X); add ln(p_abs) */
	addrule(ub+abses[onum],p,par,cons,1,kandi,l);
      }
    }
  }


  /* negative rules and lb sufficiently low */
  if (sign==0){

    /*    if (certainkandi(kandi,l))
	  printf("Certainkandi checkrule cons=~A%d\n",cons);*/

    if (p->data->nbest==NULL)
      printf("nbest=null\n");
    if ((lb<=maxlnrx[onum])&&(lb<p->data->nbest[indinkandi]+EPSILON)){
      /*ub=lb+lnqterms(frxa,frx,fra,n);*/
      /* ub=lnfirstterm(frxa,frx,fra,n)+lnqterms(frxa,frx,fra,n);*/

      ub1=lnfirstterm(frxa,frx,fra,n);
      ub2=lnqterms(frxa,frx,fra,n);
      ub=ub1+ub2;

      if (isfinite(ub)==0)
	printf("Overflow in ub=%.2e+%.2e frxa=%d,frx=%d,fra=%d,n=%d (checkrule, neg. rule)\n",ub1,ub2,frxa,frx,fra,n);

      /*  if (certainkandi(kandi,l))
	  printf("Neg. rule (cons=A%d) ub=%.3e maxlnrx=%.3e best=%.3e\n",cons,ub,maxlnrx[onum],p->data->nbest[indinkandi]);*/

      if ((ub<=maxlnrx[onum])&&(ub<p->data->nbest[indinkandi]+EPSILON)){

	if ((l==2)&&(p->data->nbest[indinkandi]<INIT+EPSILON))
	  return; /* the permutation rule A->~B for B->~A already found */


	/*	if (l==2)
		printf("set %d %d cons. %d ub=%.5e best=%.5e\n",kandi[0],kandi[1],kandi[indinkandi],ub,p->data->nbest[indinkandi]);*/

	p->data->nbest[indinkandi]=ub;

	/* given A->~B, set best-value to B->~A, too */
	if ((l==2)&&(indinkandi==0)){
	  onum2=order[kandi[1]];
	  p->data->nbest[1]=ub+abses[onum]-abses[onum2];
	}

	addrule(ub+abses[onum],p,par,cons,0,kandi,l);
      }
    }
  }
}

void checkruleM(Node *p,Node *par,int frx, int frxa, int fra, int cons, int sign, int *order,int *kandi, int l){
  float val=INIT;
  int indinkandi=-1;
  int onum,onum2; /* order number */


  onum=order[cons];
  /* check that wasn't NAV */
  if (((sign>0)&&(isposav(p,onum)==0))||((sign==0)&&(isnegav(p,onum)==0))) 
      return; 
  /* check that positive dependency */
  if ((double)n*frxa<=(double)frx*fra){
    return;
  }


  /* if cf<mincf, no reason to check further */
  if (frxa<mincf*frx)
    return;



  /* index of cons in best-table is the same as its index in kandi */
  indinkandi=indexinset(cons,kandi,l);
  if (indinkandi<0){
    printf("Attr. A%d does not occur in kandi! (checkrule)\n",cons);
    error(9); /* should not occur */
  }


    val=MEASUREVAL(frxa,frx,fra,n);

    if (better(valM,val)==1)
      return;

  /* positive rules */
  if (sign>0){
    if (p->data->pbest==NULL)
      printf("pbest=null\n");

      /* allow some error in precision */
    if (better(val,p->data->pbest[indinkandi]+EPSILON)==1){
      if ((l==2)&&(better(p->data->pbest[indinkandi],INIT+EPSILON)==1))
	return; /* the permutation rule A->B for B->A already found */
      p->data->pbest[indinkandi]=val;
      
      /* given A->B, set best-value to B->A, too */
      if ((l==2)&&(indinkandi==1)){
	onum2=order[kandi[0]];
	p->data->pbest[0]=val;
      }
      
      
      addrule(val,p,par,cons,1,kandi,l);
    }
  }


  /* negative rules and lb sufficiently low */
  if (sign==0){
    if (p->data->nbest==NULL)
      printf("nbest=null\n");

      /* allow some error in precision */
    if (better(val,p->data->nbest[indinkandi]+EPSILON)==1){
      if ((l==2)&&(better(p->data->nbest[indinkandi],INIT+EPSILON)==1))
	return; /* the permutation rule A->~B for B->~A already found */
      p->data->nbest[indinkandi]=val;

      /* given A->~B, set best-value to B->~A, too */
      if ((l==2)&&(indinkandi==0)){
	onum2=order[kandi[1]];
	p->data->nbest[1]=val;
      }
      addrule(val,p,par,cons,0,kandi,l);
    }
  }
}




/* Returns 0, if the candidate can be removed (all its supersets would  */
/* produce only redundant or non-significant rules) */
int prunecandpF(Node* t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr){
  int i,fra,fry;
  int cf1found=0; /* was any cf==1? */
  int oind,oind2;
  int indinkandi,indinkandi2;
  float best,lnlb;

  /*   printf("prunecand\n"); */
  
  /* count frequency */
  p->data->fr=frset(kandi,l,amatr);

  FREV++; /* number of fr evaluations */

  /* if fr<nminfr, only neg. rules possible */ 
  if (p->data->fr<absminfr){

    /*  printf("prunecandF prunes: fr=%d<%d=absminfr\n",p->data->fr,absminfr);*/

    if (POS){
    /* no positive rules possible and their structures can be freed */

    removebestandpossibleforposorneg(p,1);
    /* in all parents Y_i, set A_i NAV, where kandi=Y_iA_i */
    for (i=0;i<p->data->parsize;i++)
      if (p->data->parlabels[i]>=0){
	setposnav(p->data->parents[i],order[p->data->parlabels[i]]);
	TESTED++; /* for an upperbound of exp. tested rules */
      }
    }
    if (NEG==0) /* nothing else to do */
      return 0;
    else { /* neg. rules are searched */

      if (p->data->fr==0){
	checkNullFr(p,fr,oldaddr,order,kandi,l); 
	return 0; /* can be removed */
      }
      else {
	checkLowFr(p,fr,oldaddr,order,kandi,l);
	reducepossible(p); 
	if (p->data->nposlen==0)
	  return 0; /* can be removed */
	return 1; /* else save */
      }
    }
  }

  /* first check all attributes which occur in set X */
  /* first pos. consequents */
  if (POS){
    for (i=0;i<p->data->parsize;i++)
      if (p->data->parlabels[i]>=0){
	fry=p->data->parents[i]->data->fr;

	fra=fr[p->data->parlabels[i]];
	if (fry==p->data->fr){
	  cf1found=1;
	}
	oind=order[p->data->parlabels[i]];
	indinkandi=indexinset(p->data->parlabels[i],kandi,l);
	if (indinkandi<0){
	  printf("Attr. A%d does not occur in kandi! (prunecand)\n",p->data->parlabels[i]);
	  error(9); /* should not occur */
	}

	/* if A possible */
	if (isposav(p,oind)){

	  TESTED++; /* number of explicitely tested rules */

	  /* estimate lower bound 3; if it is not sufficient, the */
	  /* consequent is not possible */
	  /* allow some error in precision */
	  
	  if ((p->data->fr<posabsminfrx[oind])||((lnlb=lnmoverl(n-p->data->fr,n-fra))>=p->data->pbest[indinkandi]+EPSILON)||(lnlb>maxlnrx[oind])||((best=lnmoverl(n-p->data->fr,n-fra))>=p->data->pbest[indinkandi]+EPSILON)||(best>maxlnrx[oind])){
	    /* notice: if permutation rule A->B has already been found, */
	    /* pbest is set, and B->A is not checked */

	    /* set as NAV to p and par */

	    /*  printf("prunecand sets A%d NAV\n",p->data->parlabels[i]);*/

	    setposnav(p,oind);
	    setposnav(p->data->parents[i],oind);
	  }
	  else {/* check rule; if cf was 1, then best=real p_X and can be added */
	    if (p->data->fr==fry){ /* cf=1 */
	      p->data->pbest[indinkandi]=best;

		/* given A->B, set best-value to B->A, too */
	      if ((l==2)&&(i==0)){
		oind2=order[p->data->parlabels[1]];
		indinkandi2=indexinset(p->data->parlabels[1],kandi,l);
		/* defines r_x when p_F is the same */
		p->data->pbest[indinkandi2]=best+abses[oind]-abses[oind2];
	      }

	      /* changed 7.6. 10 */
	      /*   if ((l>2)||(p->data->pbest[indinkandi]<INIT+EPSILON))*/
		/* add only, if the permutation rule  not found */
		/* notice best=ln(p_X); add ln(p_abs) */
		addrule(best+abses[oind],p,p->data->parents[i],p->data->parlabels[i],1,kandi,l);
	      /* since minimal rule, set cons. as nav to p and par */

		/*   printf("prunecand sets A%d NAV (minimal rule)\n",p->data->parlabels[i]);*/

	      setposnav(p,oind); 	      
	      setposnav(p->data->parents[i],oind); 
	      /* also ~cons is nav, because it has frequency=0 */
	      setnegnav(p,oind); 	      
	      setnegnav(p->data->parents[i],oind); 
	      if (NEG) TESTED++;
	    }
	    else{
	      checkrule(p,p->data->parents[i],fry,p->data->fr,fra,p->data->parlabels[i],1,order,kandi,l);
	    }
	    
	  }
	}
      }
  } /* pos. rules */

  /* then neg. rules; check in the reversed order so that the */
  /* redundancy of 2-rules is handled correctly */
  if (NEG){
    for (i=p->data->parsize-1;i>=0;i--)
      if (p->data->parlabels[i]>=0){
	fry=p->data->parents[i]->data->fr;
	fra=fr[p->data->parlabels[i]];
	oind=order[p->data->parlabels[i]];
	indinkandi=indexinset(p->data->parlabels[i],kandi,l);
	if (indinkandi<0){
	  printf("Attr. A%d does not occur in kandi! (prunecand)\n",p->data->parlabels[i]);
	  error(9); /* should not occur */
	}
	
	/* if ~A possible */
	if (isnegav(p,oind)){

	  TESTED++; /* number of explicitely tested rules */

	  if ((fry-p->data->fr<negabsminfrx[oind])||((lnlb=lnmoverl(n-fry+p->data->fr,fra))>=p->data->nbest[indinkandi]+EPSILON)||(lnlb>maxlnrx[oind])||((best=lnmoverl(n-fry+p->data->fr,fra))>=p->data->nbest[indinkandi]+EPSILON)||(best>maxlnrx[oind])){

	    /* set as NAV to p and par */
	    setnegnav(p,oind);
	    setnegnav(p->data->parents[i],oind);
	  }
	  else {/* check rule */
	    checkrule(p,p->data->parents[i],fry,fry-p->data->fr,n-fra,p->data->parlabels[i],0,order,kandi,l);
	  }
	}

      } /* for i */
  } /* neg. rules */



  /* if cf==1 was found, only X's own attributes remain possible */
  if (cf1found==1){ /* set others to 0 */
    removeothers(p,kandi,oldaddr,l);
  }

  else {
  /* else check all attributes which do not occur in X but are */
  /* possible consequents. Calculate LB:s for them and decide */
  /* whether they are possible */

    /* first positive consequents, if searched */
    if (POS){
    for (i=0;i<p->data->pposlen;i++) /* for all positive consequents */
      if ((bv_tstbit(p->data->ppossible,i)==1)&&(occursinset(oldaddr[i], kandi,l)==0)){
	/* if A is not in set X */
	if (p->data->fr<posabsminfrx[i]){

	  /*printf("Prunecand disables A%d\n",oldaddr[i]);*/

	  bv_clrbit(p->data->ppossible,i);
	}
      }
    }
    if (NEG){
    for (i=0;i<p->data->nposlen;i++) /* for all negative consequents */
      if ((bv_tstbit(p->data->npossible,i)==1)&&(occursinset(labelgivennegind(i,oldaddr), kandi,l)==0)){
	/* if A is not in set X */
	if (p->data->fr<negabsminfrx[ordergivennegind(i)])
	  bv_clrbit(p->data->npossible,i);
      }
    }
  }

  /* possible-tables can be reduced; if both became zero, the node can be */
  /* deleted */
    reducepossible(p); /* reduces ppossible and npossible */
    if (((NEG==0)&&(p->data->pposlen==0))||((POS==0)&&(p->data->nposlen==0))||((p->data->pposlen==0)&&(p->data->nposlen==0))){
    return 0;
    }
  return 1;
}


int prunecandM(Node* t,Node *p,bitvector** amatr,int l,int* fr,int *kandi, int *order, int *oldaddr){
  int i,fra,fry;
  int cf1found=0; /* was any cf==1? */
  int oind,oind2;
  int indinkandi,indinkandi2;
  float val;

  /*   printf("prunecand\n"); */

  /* count frequency */
  p->data->fr=frset(kandi,l,amatr);

    FREV++; /* number of fr evaluations */

  /* if fr<nminfr, only neg. rules possible */ 
  if (p->data->fr<absminfr){

    if (POS){
    /* no positive rules possible and their structures can be freed */
    removebestandpossibleforposorneg(p,1);
    /* in all parents Y_i, set A_i NAV, where kandi=Y_iA_i */
    for (i=0;i<p->data->parsize;i++)
      if (p->data->parlabels[i]>=0){
	setposnav(p->data->parents[i],order[p->data->parlabels[i]]);
	TESTED++; /* for an upperbound of exp. tested rules */
      }
    }
    if (NEG==0) /* nothing else to do */
      return 0;
    else { /* neg. rules are searched */

      if (p->data->fr==0){
	checkNullFr(p,fr,oldaddr,order,kandi,l); 
	return 0; /* can be removed */
      }
      else {
	checkLowFr(p,fr,oldaddr,order,kandi,l);
	reducepossible(p); 
	if (p->data->nposlen==0)
	  return 0; /* can be removed */
	return 1; /* else save */
      }
    }
  }

  /* first check all attributes which occur in set X */
  /* first pos. consequents */
  if (POS){
    for (i=0;i<p->data->parsize;i++)
      if (p->data->parlabels[i]>=0){
	fry=p->data->parents[i]->data->fr;
	fra=fr[p->data->parlabels[i]];
	if (fry==p->data->fr)
	  cf1found=1;
	oind=order[p->data->parlabels[i]];
	indinkandi=indexinset(p->data->parlabels[i],kandi,l);
	if (indinkandi<0){
	  printf("Attr. A%d does not occur in kandi! (prunecand)\n",p->data->parlabels[i]);
	  error(9); /* should not occur */
	}

	/* if A possible */
	if (isposav(p,oind)){

	  TESTED++; /* number of explicitely tested rules */

	  /* calculate BOUND3; if it is not sufficient, the consequent is */
	  /* not possible. Notice: BOUND3 gives the same value as case cf=1 */
	  /* in the given node */
	  val=BOUND3(p->data->fr,fry,fra,n);

	  /* allow some error in precision in redundancy checking */
	  if ((better(valM,val)==1)||(better(p->data->pbest[indinkandi]+EPSILON,val)==1)){
	    /* set as NAV to p and par */
	    setposnav(p,oind);
	    setposnav(p->data->parents[i],oind);
	  }
	  else {/* check rule */
	    if (p->data->fr==fry){ /* minimal rule */
	      p->data->pbest[indinkandi]=val;
	      /* given A->B, set best-value to B->A, too */
	      if ((l==2)&&(i==0)){
		oind2=order[p->data->parlabels[1]];
		indinkandi2=indexinset(p->data->parlabels[1],kandi,l);
		p->data->pbest[indinkandi2]=val;
	      }


	      /*  printf("prunecand adds rule M=%f m(XA)=%d\n",val,p->data->fr);
		  printset(kandi,l);*/


		addrule(val,p,p->data->parents[i],p->data->parlabels[i],1,kandi,l);
	      /* since minimal rule, set cons. as nav to p and par */
	      setposnav(p,oind); 	      
	      setposnav(p->data->parents[i],oind); 
	      /* also ~cons is nav, because it has frequency=0 */
	      setnegnav(p,oind); 	      
	      TESTED++;
	      setnegnav(p->data->parents[i],oind); 
	    } /* minimal rule*/

	    else { /* non-minimal rule */
	      checkrule(p,p->data->parents[i],fry,p->data->fr,fra,p->data->parlabels[i],1,order,kandi,l);
	    }
	  }
	}
      }
  } /* pos. rules */

  /* then neg. rules; check in the reversed order so that the */
  /* redundancy of 2-rules is handled correctly */
  if (NEG){
    for (i=p->data->parsize-1;i>=0;i--)
      if (p->data->parlabels[i]>=0){
	fry=p->data->parents[i]->data->fr;
	fra=fr[p->data->parlabels[i]];
	oind=order[p->data->parlabels[i]];
	indinkandi=indexinset(p->data->parlabels[i],kandi,l);
	if (indinkandi<0){
	  printf("Attr. A%d does not occur in kandi! (prunecand)\n",p->data->parlabels[i]);
	  error(9); /* should not occur */
	}
	
	/* if ~A possible */
	if (isnegav(p,oind)){

	  TESTED++; /* number of explicitely tested rules */

	  /* calculate BOUND3 */
	  val=BOUND3(fry-p->data->fr,fry,n-fra,n);
	  /* allow some error in precision in redundancy checking */
	  if ((fry-p->data->fr<absminfr)||(better(valM,val)==1)||(better(p->data->nbest[indinkandi]+EPSILON,val)==1)){
	    /* set as NAV to p and par */
	    setnegnav(p,oind);
	    setnegnav(p->data->parents[i],oind);
	  }
	  else {/* check rule */
	    checkrule(p,p->data->parents[i],fry,fry-p->data->fr,n-fra,p->data->parlabels[i],0,order,kandi,l);
	  }
	}
      } /* for i */
  } /* neg. rules */


  /* if cf==1 was found, only X's own attributes remain possible */
  if (cf1found==1) /* set others to 0 */
    removeothers(p,kandi,oldaddr,l);

  else {
  /* else check all attributes which do not occur in X but are */
  /* possible consequents. Calculate LB:s for them and decide */
  /* whether they are possible */

    /* first positive consequents, if searched */
    if (POS){
    for (i=0;i<p->data->pposlen;i++) /* for all positive consequents */
      if ((bv_tstbit(p->data->ppossible,i)==1)&&(occursinset(oldaddr[i], kandi,l)==0)){
	/* if A is not in set X, calculate BOUND2 */
	if (better(valM,BOUND2(p->data->fr,fr[oldaddr[i]],n))==1)
	  bv_clrbit(p->data->ppossible,i);
      }
    }
    if (NEG){
    for (i=0;i<p->data->nposlen;i++) /* for all negative consequents */
      if ((bv_tstbit(p->data->npossible,i)==1)&&(occursinset(labelgivennegind(i,oldaddr), kandi,l)==0)){
	/* if A is not in set X */

	if (better(valM,BOUND2(p->data->fr,n-fr[labelgivennegind(i,oldaddr)],n))==1)
	  /* was bug! corrected in v2b (2015) */
	  /*if (better(valM,BOUND2(p->data->fr,n-fr[oldaddr[i]],n))==1)*/


	  bv_clrbit(p->data->npossible,i);
      }
    }
  }

  /* possible-tables can be reduced; if both became zero, the node can be */
  /* deleted */
    reducepossible(p); /* reduces ppossible and npossible */
    if (((NEG==0)&&(p->data->pposlen==0))||((POS==0)&&(p->data->nposlen==0))||((p->data->pposlen==0)&&(p->data->nposlen==0)))
    return 0;
  return 1;
}


/* special case: if p->data->fr==0, all rules Yi->~Ai, where Y_iA_i is kandi, */
/* have cf=1.0 and are minimal. All positive consequents are impossible,*/
/* but marked on the call level. All negative consequents not in kandi */
/* are NAV. */
void checkNullFrpF(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l){
  int i,indinkandi,label,index;
  Node *par=NULL;
  float best;
  int oind2;


  /*     if (certainkandi(kandi,l))
	 printf("checkNullfr\n");*/

  if (p->data->fr==0){
    for (i=p->data->nposlen-1;i>=0;i--){
      /* proceed from the most frequent to leastfrequent cons.; */
      /* now redundancy of 2-rules gets handled */
      /*  for (i=0;i<p->nposlen;i++){*/
      /* the label of attribute in position npossible[i] */
      label=labelgivennegind(i,oldaddr);
      /* its position in kandi; -1, if does not occur in kandi */
      indinkandi=indexinset(label,kandi,l);
      if (indinkandi<0)
	bv_clrbit(p->data->npossible,i); /* set to NAV */
      else { /* occurs in kandi */
	par=searchParent(p,label,&index);
	if (par==NULL){
	  printf("par=NULL in checkNullFr\n");
	  error(9); /* should not happen */
	}
	index=order[label];
	if (bv_tstbit(p->data->npossible,i)){ /* was AV */
	  /* first check if YQ->~A is possible */

	  TESTED++; /* number of explicitely tested rules */

	  /* the p_X is m(~Y) over m(A) = (n-m(Y~A) over m(A) */
	  /* allow some error in precision */

	  /* now m(~XA)=m(A) */
	  	  if ((par->data->fr>=negabsminfrx[index])&&((best=lnmoverl(n-par->data->fr,fr[label]))<p->data->nbest[indinkandi]+EPSILON)&&(best<=maxlnrx[index])){
	    p->data->nbest[indinkandi]=best;

	      /* given rule A->~B, set best to B->~A, too */
	     if ((l==2)&&(indinkandi==0)){ 
	      oind2=order[p->data->parlabels[1]];
	      p->data->nbest[1]=best+abses[index]-abses[oind2];
	      }

	     /* changed 7.6.10 */
	     /*	      if ((l>2)||(p->data->nbest[indinkandi]<INIT+EPSILON))*/
		/* add only, if the permutation rule A->~B for B->~A not found */
		addrule(best+abses[index],p,par,label,0,kandi,l); /* adds to BRULES, if sufficiently good */
	  }
	  /* the rule is always minimal, so set as NAV to p and par */
	  bv_clrbit(p->data->npossible,i);
	} /* was AV */
	/* anyway set ~A as NAV to par */
	setnegnav(par,index); 
      } /* else */
    } /* for i */
  }  /* fr==0 */
}


void checkNullFrM(Node *p,int *fr, int *oldaddr, int *order,int *kandi, int l){
  int i,indinkandi,label,index;
  Node *par=NULL;
  float val;

  /*       if (certainkandi(kandi,l))
	   printf("checkNullfr nposlen=%d\n",p->data->nposlen);*/


  if (p->data->fr==0){
    for (i=p->data->nposlen-1;i>=0;i--){
      /* proceed from the most frequent to leastfrequent cons.; */
      /* now redundancy of 2-rules gets handled */
      /*  for (i=0;i<p->nposlen;i++){*/
      /* the label of attribute in position npossible[i] */
      label=labelgivennegind(i,oldaddr);
      /* its position in kandi; -1, if does not occur in kandi */
      indinkandi=indexinset(label,kandi,l);
      if (indinkandi<0){
	bv_clrbit(p->data->npossible,i); /* set to NAV */
      }
      else { /* occurs in kandi */
	par=searchParent(p,label,&index);
	if (par==NULL){
	  printf("par=NULL in checkNullFr\n");
	  error(9); /* should not happen */
	}
	index=order[label];
	if (bv_tstbit(p->data->npossible,i)){ /* was AV */
	  /* first check if YQ->~A is possible */

	  TESTED++; /* number of explicitely tested rules */

	  /* allow some error in precision */
	  /* now m(~XA)=m(A) */

	  val=MEASUREVAL(par->data->fr,par->data->fr,n-fr[label],n);
	  /*if (certainkandi(kandi,l))
	    printf("val=%.2f vs. %.2f\n",val,p->data->nbest[indinkandi]);*/

		    /*	  if ((better(val,valM)==1)&&(better(val,p->data->nbest[indinkandi]+EPSILON)==1)){*/

		    if ((better(valM,val)==0)&&(better(p->data->nbest[indinkandi]+EPSILON,val)==0)){

	    p->data->nbest[indinkandi]=val;
	      /* given rule A->~B, set best to B->~A, too */
	     if ((l==2)&&(indinkandi==0)){ 
	       p->data->nbest[1]=val;
	     }

	     addrule(val,p,par,label,0,kandi,l); /* adds to BRULES, if sufficiently good */
	  }
	  /* the rule is always minimal, so set as NAV to p and par */
	  bv_clrbit(p->data->npossible,i);
	} /* was AV */
	
	/* anyway set ~A as NAV to par */
	setnegnav(par,index); 
      } /* else */
    } /* for i */
  }  /* fr==0 */

}

/* Special case when p->data->fr<absminfr. Only rules Y_i->~A_i are possible, */
/* where Y_iA_i is kandi; all other (both pos. and neg.) are NAV. */
/* Positive rules have been handled already on the call level. */
/* No rule can be minimal, because case fr=0 is checked elsewhere. */
void checkLowFrpF(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l){
  int i,index,label,indinkandi;
  Node *par;
  float best;

  /*  if (certainkandi(kandi,l))
      printf("checklowfr\n");*/


  /*   for (i=0;i<p->nposlen;i++){*/
  /* go from the end to the beginning so that the redundancy of */
  /* 2-rules is handled correctly */
   for (i=p->data->nposlen-1;i>=0;i--){
    /* the label of attribute in position npossible[i] */
    label=labelgivennegind(i,oldaddr);
    /* its position in kandi; -1, if does not occur in kandi */
    indinkandi=indexinset(label,kandi,l);
    if (indinkandi<0){
      bv_clrbit(p->data->npossible,i); /* set to NAV */
    }
    else /* occurs in kandi */
      if (bv_tstbit(p->data->npossible,i)){ /* was AV */

	  TESTED++; /* number of explicitely tested rules */

	par=searchParent(p,label,&index);
	if (par==NULL){
	  printf("par=NULL in checkLowFr\n");
	  error(9); /* should not happen */
	}
	/* first check if YQ->~A is possible */
	index=order[label];
	/* the LB is m(~Y) over m(A) = (n-m(Y~A) over m(A) */
	/* allow some error in precision */

	
	if ((par->data->fr-p->data->fr<negabsminfrx[index])||((best=lnmoverl(n-par->data->fr+p->data->fr,fr[label]))>=p->data->nbest[indinkandi]+EPSILON)||(best>maxlnrx[index])){

	  
	  /* YQ->~A would be insignificant or redundant */
	  bv_clrbit(p->data->npossible,i); /* set to NAV */
	  /* set as NAV to par, too */
	  setnegnav(par,index); 
	}
	else { /* check if rule is nonred significant and add it */
	  checkrule(p,par,par->data->fr,par->data->fr-p->data->fr,n-fr[label],label,0,order,kandi,l);
	  
	}
      } /* was AV */
  } /* for i */
}


void checkLowFrM(Node *p,int *fr,int *oldaddr,int *order,int *kandi, int l){
  int i,index,label,indinkandi;
  Node *par;
  float val;

  /* go from the end to the beginning so that the redundancy of */
  /* 2-rules is handled correctly */
   for (i=p->data->nposlen-1;i>=0;i--){
    /* the label of attribute in position npossible[i] */
    label=labelgivennegind(i,oldaddr);
    /* its position in kandi; -1, if does not occur in kandi */
    indinkandi=indexinset(label,kandi,l);
    if (indinkandi<0){
      bv_clrbit(p->data->npossible,i); /* set to NAV */
    }
    else /* occurs in kandi */
      if (bv_tstbit(p->data->npossible,i)){ /* was AV */

	  TESTED++; /* number of explicitely tested rules */

	par=searchParent(p,label,&index);
	if (par==NULL){
	  printf("par=NULL in checkLowFr\n");
	  error(9); /* should not happen */
	}
	/* first check if YQ->~A is possible, use BOUND3 */
	index=order[label];
	val=BOUND3(par->data->fr-p->data->fr,par->data->fr,n-fr[label],n);
	/* allow some error in precision */
	if ((better(valM,val)==1)||(better(p->data->nbest[indinkandi]+EPSILON,val)==1)){
	  /* YQ->~A would be insignificant or redundant */
	  bv_clrbit(p->data->npossible,i); /* set to NAV */
	  /* set as NAV to par, too */
	  setnegnav(par,index); 
	}
	else { /* check if rule is nonred significant and add it */
	  checkrule(p,par,par->data->fr,par->data->fr-p->data->fr,n-fr[label],label,0,order,kandi,l);	  
	}
      } /* was AV */
  } /* for i */
}


/* when node p correponding set X is removed, mark all parents par */
/* corresponding set Y, X=YA, that A and ~A are impossible */
void markParents(Node *p, int *order){
  int i;

  for (i=0;i<p->data->parsize;i++)
    if (p->data->parlabels[i]>=0){
      /* parlabel[i] gives cons. attr. which is set nav */
      if (POS)
	setposnav(p->data->parents[i],order[p->data->parlabels[i]]);
      if (NEG)
	setnegnav(p->data->parents[i],order[p->data->parlabels[i]]);
    }
}

/* Does p have interesting children? */
int interchildren(Node *p){
  int i;
  for (i=0;i<p->chsize;i++)
    if ((p->children[i]!=NULL)&&(interattr[p->children[i]->label]))
      return 1;
  return 0;
}

int main(int argc,char**argv){
  /*  float K=sqrt(k/2.0);  significance level */
  FILE *f=NULL;
  FILE *f2=NULL;
  FILE *f4=NULL;
  FILE *f5=NULL;
  FILE *f6=NULL;
  FILE *f7=NULL;
  bitvector **amatr;
  Node* t;  /* data structure */
  int numl; /* number of l-sets in the tree after pruning */
  int totnum=0; /* number of all added sets */
  int totrules=0; /* number of all discovered non-red. sign. rules */
  int l,i;
  int* fr;
  int* oldaddr;
  int* order;
  int opt;
  /* check if obligatory parameters given: */
  int inok=0;int kok=0; int Mok=0; 
  /* voluntary parameters */
  int outok=0; /* is output file given? */
  int pstyle=1; /* output format */
  int frok=0; /* is minfr given */
  int cfok=0;  /* is mincf given */
  int aok=0; /* are interesting attributes given? */
  int bok=0; /* are extra constraints given? */
  int fok=0; /* are fixed consequences given? */
  int measureok=0; /* is measure function selected? */
  int typenum=1;
  int measurenum=2;
  int special; /* number of special attributes */
  LYHYT **extraconstr=NULL;
  char *inputfilename=NULL;
  float origM=0.0, correctedM=0.0;
  LYHYT *consattr=NULL;
  /*************** check commandline options *****************/

  i=0;
  POS=1; NEG=0; /* by default, only positive rules */
  lnqterms=exactlnfact; /* by default, exact ln(p) is calculated */
  /* give option -u to use tight upperbounds */


  /* at least 4: command, input file, k, threshold for M */
  if (argc<4){
    printf("Obligatory options missing\n");
    error(0);
  }

  while ((opt=getopt(argc,argv,"i:k:M:c:o:m:l:q:p:w:t:e:b:f:ud"))!=-1){
    /* option a disabled */
    /* while ((opt=getopt(argc,argv,"i:k:M:c:o:m:l:q:p:w:t:e:a:b:f:ud"))!=-1){*/
    switch(opt){
    case 'i':  printf("Input file [%s]\n",optarg);
      if ((f=fopen(optarg,"r"))==NULL)
	error(1);
      inok=1;
      inputfilename=optarg;
      break; 
    case 'k': k=atoi(optarg);
      if (k<0){
	printf("Last item number (option k) should be >=1. E.g. if your attributes are A1,A3,A5, then k=5.\n");
	error(1);
      }
      kok=1;
      break;
    case 'M': /* threshold for measure value, for Fisher's p, maxp */ 
      valM=atof(optarg);
      printf("Threshold %.2f\n",valM);
      origM=valM;
      Mok=1;
      break;
    case 'w': /* measure function */
      measurenum=atoi(optarg);
      if ((measurenum>NMeasures)||(measurenum<=0)){
	printf("Measure num %d not defined, using ln(p) (for Fisher's p)\n",measurenum);
	measurenum=2;
      }
      measureok=1;
      break;
    case 't': /* type: pos, neg. or both? */
      typenum=atoi(optarg);
      if (typenum==1){
	NEG=0;
      }
      if (typenum==2){
	POS=0; NEG=1;
      }
      if (typenum==3)
	NEG=1; /* already POS=1 */
      if ((typenum<0)||(measurenum>3)) {
	printf("Wrong type %d. By default searches positive rules.\n",typenum);
	typenum=3;
      }
      break;
    case 'c': 
      mincf=atof(optarg);
      if ((mincf<0)||(mincf>1.0)){ 
	printf("mincf (option c) should be in [0,1]\n");
	error(0);
      }
      cfok=1;
      break;
    case 'm': 
      minfr=atof(optarg);
      if ((minfr<0)||(minfr>1.0)){ 
	printf("minfr (option m) should be in [0,1].");
	error(0);
      }
      frok=1;
      break;
    case 'o':  printf("Output file [%s]\n",optarg);
      if ((f2=fopen(optarg,"w"))==NULL)
	error(1);
      outok=1;
      break; 
    case 'l':MAXTASO=atoi(optarg);
      if (MAXTASO<=1){ 
	printf("Rule length (option l) should be at least 2 (1 attribute in the condition part, 1 in the consequence part)\n");
	error(0);
      }
      break;
    case 'q':Q=atoi(optarg);
      if (Q<0){
	printf("Number of best rules to be printed (option q) should be >=0\n");
	error(0);
      }
      break; 
    case 'p': pstyle=atoi(optarg);
      if ((pstyle!=1)&&(pstyle!=2))
	printf("Output format (option p) should be 1 (rules with fr, cf, lift, and measure value) or 2 (only rules). Now using 1.\n");
      break;
    case 'e': printf("Compatibility constraints in file [%s]\n",optarg);
      if ((f4=fopen(optarg,"r"))==NULL)
	error(1);
      eok=1;
      break; 

    case 'b': printf("Special constraints in file [%s]\n",optarg);
      if ((f6=fopen(optarg,"r"))==NULL)
	error(1);
      bok=1;
      break; 

      /*   case 'a': printf("Interesting attributes in file [%s]\n",optarg);
      if ((f5=fopen(optarg,"r"))==NULL)
	error(1);
      aok=1;
      break; */

    case 'f': printf("Fixed consequences in file [%s]\n",optarg);
      if ((f7=fopen(optarg,"r"))==NULL)
	error(1);
      fok=1;
      break; 

    case 'u':
      /* uses uppebound estimate for ln(1+q1+...+q1q2...qm) */
      printf("Using an upper bound approximation for pF (for exact pF skip option -u)\n");
      lnqterms=lnubfactor;
      break;
    case 'd':
      printf("Continuity correction disabled (with chi2)\n");
      CCORR=0;
      break;
    default: 
      printf("Unknown option %s\n",optarg);
      error(0);
    }
  }

  if (!inok||!kok||!Mok){
    printf("Input file, number of attributes k, or minimum M value missing\n");
    error(0);
  }
  
  n=nofrows(f); /* number of rows */
  /* if attribute numbering begins from 1, the last index can be k+1 */
  /* therefore increase k by one */
  k++; 
  printf("Last item number <=%d, %d rows data.\n",k,n);

  if (frok){ /* user has defined a minimum frequency threshold */
    nminfr=n*minfr;
    printf("Minimum frequency minfr=%.4f\n",minfr);
  }
  else{
    nminfr=MINMFR;
    minfr=MINMFR/((float)n);   /* default is set: minfr=MINMFR/n */
    printf("No minimum frequency defined, using %.6f (abs. minfr %d).\nIf you want use smaller minfr, modify measures.h.\n",minfr,MINMFR);
  }
  if (cfok)
    printf("Minimum confidence mincf=%.3f\n",mincf);
  else printf("No minimum confidence defined, using 0.0\n");

  if (typenum==1) 
    printf("Searches only positive rules (default, change with option t).\n");
  if (typenum==2)
    printf("Searches only negative rules.\n");
  if (typenum==3)
    printf("Searches both positive and negative rules.\n");
    
  /* set function pointers  */
  if ((measurenum==1)||(measurenum==2)){
    process1sets=detType1sets;
    prunecand=prunecandpF;
    checkrule=checkrulepF;
    checkNullFr=checkNullFrpF;
    checkLowFr=checkLowFrpF;
  }
  else { /* other measures */
    process1sets=detType1setsM;
    prunecand=prunecandM;
    checkrule=checkruleM;
    checkNullFr=checkNullFrM;
    checkLowFr=checkLowFrM;
  }
  /* initialize the rest for the given measure */
  updatevalM=defaultupdatevalM;
  initializeMeasure(measurenum);


  /************ commandline parameter checking done *************/
  /*********** initialization begins ****************************/

  /*  initmatr(f, &matr,&rowc); */
  initattrmatr(f, &amatr,n,k); 
  if(fclose(f)==EOF)
    error(4);
  /*  printdata(matr,n,k);*/

  /*  printf("avg transction length=%.1f\n",avgtlen(amatr,n,k));*/

  /* with pF, one can define a minimum frequency threshold */
  if ((measurenum==1)||(measurenum==2)){
    /* initialize lnallfactorials */
    printf("Calculates all ln-factorials ln(m!) for m=1 to %d\n",n);
    tim1=clock();
    initlnfact(n);
    tim2=clock();
    tottim+=(double)(tim2-tim1);
    printf("(%.1f seconds)\n",((double)(tim2-tim1))/CLOCKS_PER_SEC);
    
    /* minimum frequency derived from maxp */
    printf("Determines absolute minimum frequency threshold from maximal p-value\n");
    tim1=clock();
    /* absminfr=defineabsminfr(valM,n); changed, see below */
    tim2=clock();
    tottim+=(double)(tim2-tim1);
    printf("(%.1f seconds)\n",((double)(tim2-tim1))/CLOCKS_PER_SEC);
    if (absminfr<0){ /* special case, not rules can be found at level maxp */
      printf("Too small maxlnp (maxp), no significant rules can be found.\n");
      exit(1);
    } 
  }
  /* Changed: for all here */
  absminfr=detabsminfr(valM,n);

    printf("Based on threshold valM, m(A=a)>=%d for any significant rule X->A=a\n",absminfr);
  /* user could have defined minfr=min(m(XA)); if minfr>absminfr, change it */
  if (absminfr<nminfr){
    printf ("User has defined a larger minfr, m(XA=a)>=%d. Therefore some rules significant at level valM=%.2e can be missed\n",nminfr,valM);
    absminfr=nminfr;
  }
  else { 
    nminfr=absminfr;
    minfr=((float)nminfr)/n;
  }
  /* now absminfr gives a minimum for any m(A) and also m(XA) */

  if (eok){ /* compatibility constraints given in file f4 */
    constraints=readconstraints(f4,k);
    if(fclose(f4)==EOF)
      error(4);
  }

  /*  printconstr(constraints,k);*/
  if (bok){ /* extra constraints given in file f6 */
    extraconstr=readextraconstr(f6,k);
    if(fclose(f6)==EOF)
      error(4);
  }

  if (aok){ /* interesting attributes given in file f5 */
    interattr=readinterattr(f5,k);
    if(fclose(f5)==EOF)
      error(4);
    intergiven=1;
    if ((ATTUSED=(LYHYT*)malloc(k*sizeof(LYHYT)))==NULL)
      error(6);    
    for (i=0;i<k;i++)
      ATTUSED[i]=0;
    /*    printinterattr(interattr,k);*/
  }

  if (fok){ /* fixed consequences given */
    consattr=readinterattr(f7,k);
    if(fclose(f7)==EOF)
      error(4);
  }

  printf("Initializes\n");
  t=createNode(-1); /* luo juuri, tyypill‰ ei v‰li‰ */
  tim1=clock();

  numl=generate1sets(amatr,t,&fr,&oldaddr,&order,&special);
  totnum=numl; /* number of added attribute sets */
  attnum=numl; /* number of attributes, when infrequent have been pruned */ 
 
  tim2=clock();
  tottim+=(double)(tim2-tim1);


  /* printf("Attributes in order:\n");
  for (i=0;i<numl;i++)
    printf("%d A%d (mfr=%d)",i,oldaddr[i],fr[oldaddr[i]]);
    printf("\n"); */

  /* define possible consequents and calculate absminfrx-values */
  printf("level 1\n");
  tim1=clock();
  process1sets(t,attnum,fr,oldaddr);


  if (eok)
    addcompconstr(t,constraints,k,order);
  if (bok)
    addextraconstr(t,extraconstr,k,order);
  if (fok)
    prunecons(t,consattr,k,order);
  if (aok)
    prunewhenintergiven(t,interattr,k);

  initBStructures(&BRULES,Q,&LASTQ,&CONS,&XFR,&XAFR,&MVAL,&BSIGNS);
  tim2=clock();
  tottim+=(double)(tim2-tim1);



  /*   for (i=0;i<t->chsize;i++)
     if (t->children[i]!=NULL) {
    printf("label %d ",t->children[i]->label);
    printpossibleandbest(t->children[i],oldaddr,1);
    }*/


  /******************* search begins **********************/

  l=2;
  /* for a l-set at least l parents are needed */
  /* while ((numl>=2)&&(numl>=l-2)&&(l<=MAXTASO)){*/
  while ((numl>=l)&&(l<=MAXTASO)){
    /*  check l-candidates */
    printf("level %d\n",l);
    BESTVAL=INIT; /* best M-value of the level */
    TOTRULES=0; ADDEDRULES=0;
    tim1=clock();
    numl=generatecandidates(t,l,order,amatr,fr,oldaddr);    
    tim2=clock();
    tottim+=(double)(tim2-tim1);
    printf("number of %d-sets %d\n",l,numl);
    if (ADDEDRULES>0)
      printf("best of level %d M=%.2f. %d non-red. significant rules found, %d of them\n added to the best Q rules. Updated valM=%.3e\n",l,BESTVAL,TOTRULES,ADDEDRULES,valM);
    else printf("No rules found.\n");

    /*   printresults(stdout,Q,LASTQ,pstyle,BRULES,CONS,XAFR,XFR,MVAL,BSIGNS,fr);*/


    /* printtree(t); */


    totrules+=TOTRULES;

    if (numl>0){
      totnum+=numl;
      l++;
    }
  }

  /************************ search done ***********************/

  /*    printtree(t);*/



  tottim/=CLOCKS_PER_SEC;
  printf("Execution time %.0f s=%.0f min (%.6f s)\n",tottim,tottim/60,tottim);

  printf("Last level %d. Last valM=%.2f.\nNumber of all non-redundant significant rules %d.\nNumber of added sets %d. Number of frequency evaluations %d.\n",l-1,valM,totrules,totnum, FREV);
  printf("Number of explicitely tested rules at most tests=%d\n",TESTED); 

  if (measurenum==2){ /* ln(p) */
    correctedM=valM+logf((float)TESTED);
    printf("Since measure was ln(p), ln(p*tests)=valM+ln(tests)=%.2f.\n",correctedM);
}


  printf("The best rules: (gamma=lift, delta=leverage)\n");

  if (outok){ /* print to output file */
    printresults(f2,Q,LASTQ,pstyle,BRULES,CONS,XAFR,XFR,MVAL,BSIGNS,fr);
    if (fclose(f2)==EOF)
      error(4);
  }
  else { /* print to stdout */
    printresults(stdout,Q,LASTQ,pstyle,BRULES,CONS,XAFR,XFR,MVAL,BSIGNS,fr);
  }


  deleteTree(t);
    freematr(amatr,k);


  return EXIT_SUCCESS;
}

