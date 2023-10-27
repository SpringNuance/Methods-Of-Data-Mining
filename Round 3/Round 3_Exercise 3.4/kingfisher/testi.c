#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ibitvector.h"

int main(){
  int i;
  bitvector *v, *w;

  v=bv_alloc(10);
  w=bv_alloc(4);

  for (i=0;i<4;i++)
    bv_clrbit(w,i);
  for (i=0;i<10;i++)
    bv_clrbit(v,i);


  bv_setbit(v,1);
  bv_setbit(v,4);
  bv_setbit(v,9);
  bv_setbit(w,2);
  bv_setbit(w,1);

  for (i=0;i<4;i++)
    printf("%d",bv_tstbit(w,i));
  printf("\n");
  for (i=0;i<10;i++)
    printf("%d",bv_tstbit(v,i));
  printf("\n");

  printf("Sitten bv_or_vlonger\n");
  bv_or_vlonger(v,w,10);

  for (i=0;i<4;i++)
    printf("%d",bv_tstbit(w,i));
  printf("\n");
  for (i=0;i<10;i++)
    printf("%d",bv_tstbit(v,i));
  printf("\n");

  return 1;

}
