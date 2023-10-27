#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "measures.h"
#include "glob.h"
#include <values.h>

int main(){
  int frxa;
  int frx;
  int fra;
  int i;  
  float lnp;
  long double neper=expl(1.0);
  initlnfact(100);

  n=100;
  lnp=exactlnp(50,60,50,100);
  printf("lnp=%f p=%e\n",lnp,pow(neper,lnp));



  return EXIT_SUCCESS;
}
