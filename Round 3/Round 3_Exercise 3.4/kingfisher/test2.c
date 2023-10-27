#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <values.h>


int main(){
  long double sum=1.0;
  int i;

  for (i=1;i<=30;i++){
    sum*=(20+((long double)i))/(70+((long double)i));
      if (isfinite(sum)==0)
	printf("Underflow\n");
    }
  printf("p=%Le\n",sum);

  sum=1.0;

  for (i=1;i<=40;i++){
    sum*=(10+((long double)i))/(60+((long double)i));
      if (isfinite(sum)==0)
	printf("Underflow\n");
    }
  printf("p=%Le\n",sum);



  return EXIT_SUCCESS;
}
