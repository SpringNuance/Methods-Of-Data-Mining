/* Error messages for namescodes.c */
#include <stdio.h>
#include <stdlib.h>
#include "error.h"


void error(int code) {
  switch (code) {
  case 0:  printf("Extra or missing command line parameters. The syntax is\n");
    printf("namescodes -n<namefile> -t<codetable> -L for names-to-codes transformation and \n"); 
    printf("namescodes -c<codefile> -t<codetable> for codes-to-names transformation.\n");
    printf("Option -L means that a new codetable is created (you can skip it, if the codetable already exists)\n");
    printf("The transformed file has suffix .names or .codes.\n");

  break;
  case 1: printf("Error in opening input file\n");
  break;
  case 2: printf("File error\n");
  break;
  case 3: printf("Error in file format: number of rows missing \n");
  break;
  case 4: printf("Error in closing file\n");
  break;
  case 5: printf("Error in data format\n");
  break;
  case 6: printf("Error in memory allocation\n");
  break;
  case 7: printf("Error in writing file\n");
  break;
  case 8: printf("This error should not occur!\n");
    break;
  }
  exit(EXIT_FAILURE);
}
