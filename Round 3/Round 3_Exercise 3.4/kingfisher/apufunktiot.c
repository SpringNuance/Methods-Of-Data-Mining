#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "error.h"
#include <float.h>


#define round(X) floor((X)+.5)


float avgf(float *t,int l){
  int i;
  float sum=0.0;

  for (i=0;i<l;i++)
    sum+=t[i];
  return sum/l;
}



float stdevf(float *t,int l,float avgx){
  int i;
  float sum=0.0;

  for (i=0;i<l;i++)
    sum+=pow((t[i]-avgx),2);
  return sqrt(sum/l);
}


double maxd(double *t,int l){
  double m=0.0;
  int i;
  for (i=0;i<l;i++)
    if (t[i]>m) m=t[i];

  return m;
}



float maxf(float *t,int l){
  float m=0.0;
  int i;
  for (i=0;i<l;i++)
    if (t[i]>m) m=t[i];

  return m;
}


double mind(double *t,int l){
  double m=DBL_MAX;
  int i;
  for (i=0;i<l;i++)
    if (t[i]<m) m=t[i];

  return m;
}


float minf(float *t,int l){
  float m=FLT_MAX;
  int i;
  for (i=0;i<l;i++)
    if (t[i]<m) m=t[i];

  return m;
}


/* 1-arvojen osuus kaikista */
float osuusd(double *t,int l){
  int lkm=0;
  int i;
  for (i=0;i<l;i++)
    if (t[i]>0.0) lkm++;

  return ((float)lkm)/l;
}


/* taulukon 1. indeksi jossa arvo >=0; -1 jos ei loydy */
 int ekaarvof(float *t,int l){
   int i;
   for (i=0;i<l;i++)
     if (t[i]>=0) break;

   if (i<l-1) return i;
   else{ 
     if (t[l-1]>=0) return l-1;
     else return -1; 
   }
 }


/* palauttaa viimeisen indeksin jossa arvo >-FLT_MAX ja -1 jos ei löydy */
int vikaarvof(float *t, int l){
  int i;

  for (i=l-1;i>=0;i--)
    if (t[i]>-1.0*FLT_MAX) break;

  if (i>0) return i;
  /* jos tuli taulukon alkuun */
  if (t[0]>-1.0*FLT_MAX) return 0;
  else return -1;
}



/* laskee rivien lkm:n tiedostossa */
int nofrows(FILE *f){
  int lkm=0;
  char *row;

  if ((row=(char*)malloc(6000))==NULL) error(6);

  while (fgets(row, 6000, f)!=NULL)
      lkm++;

  rewind(f);

  return lkm;
}


/* siirtaa arvot -FLT_MAX loppuun ja palauttaa niiden lkm joissa oikea arvo */
int siirrapuuttuvat(float *t, int l){
  int i, ind;
  int viim=l;

  for (i=0;i<l;i++)
    if (t[i]<=-1.0*FLT_MAX){
      ind=vikaarvof(t,viim);
      if ((ind<=0)||(ind<i)) /* ei enää siirtämistä */
	break;
      else {
	t[i]=t[ind];
	t[ind]=-1.0*FLT_MAX;
	viim=ind;
      }
    }

  return i;
}



/* luo uuden taulukon, johon sijoittaa normalisoidut arvot */
float *normalisoi(float *t,int l){
  float ka, khaj;
  int i;
  float *ut;

  if ((ut=(float*)malloc(l*sizeof(float)))==NULL) error(6);

  ka=avgf(t,l);
  khaj=stdevf(t,l,ka);
  if (khaj==0.0) 
    for (i=0;i<l;i++)
      ut[i]=0.0;
  else 
    for (i=0;i<l;i++)
      ut[i]=(t[i]-ka)/khaj;

  return ut;
}

float sumf(float *t, int l){
  float sum=0.0;
  int i;
  for (i=0;i<l;i++)
    sum+=t[i];
  return sum;
}

/* moniko ylittää raja-arvon thr? */
int aktlkm(float *t, int l, float thr){
  int i;
  int lkm=0;
  for (i=0;i<l;i++)
    if (t[i]>=thr)
      lkm++;
  return lkm;
}




void printvector(float *t,int l){
  int i;

  for (i=0;i<l;i++)
    printf("%.2f ",t[i]);
  printf("\n");
}



void printtable(float **t, int riv, int sar){
  int i,j;

  for (i=0;i<riv;i++){
    printf("%d ",i);
    for (j=0;j<sar;j++)
      printf("%.2f ",t[i][j]);
    printf("\n");
  }
}

/* tulosta ne rivit joilla attr l arvo > thr */
void selprinttable(float **t, int riv, int sar, int l, float thr){
  int i,j;

  for (i=0;i<riv;i++){
    if (t[i][l]>thr){
      printf("%d ",i);
      for (j=0;j<sar;j++)
	printf("%.2f ",t[i][j]);
      printf("\n");
    }
  }
}


/* return 1 if a occurs in table, otherwise 0 */
int occursinset(int a, int *table, int size){
  int i;

  for (i=0;i<size;i++)
    if (table[i]==a)
      return 1;
  return 0;
}

/* give index in table where a occurs. return -1, if it doesn't */
int indexinset(int a, int *table, int size){
  int i;

  for (i=0;i<size;i++)
    if (table[i]==a)
      return i;
  return -1;
}

int emptystring(char *t){
  int i=0;

  while (t[i]!='\0'){
    if ((t[i]!=' ')&&(t[i]!='\t')&&(t[i]!='\n'))
      return 0;
    i++;
  }
  return 1;
}
