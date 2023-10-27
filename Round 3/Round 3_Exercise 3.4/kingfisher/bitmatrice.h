/* matr.h */
#include <stdio.h>
#include "error.h"
/*typedef unsigned char uchar;
  typedef	unsigned int bitvector;*/
#include "ibitvector.h"

void initmatr(FILE *f, bitvector ***matr,int n, int k);
void initattrmatr(FILE *f, bitvector ***matr, int n, int k);
void printdata(bitvector **matr, int row, int col);
void printdata2(bitvector **matr, int *fr, int row, int col, int absminfr);
void initmatr2(FILE *f, bitvector ***matr, int n1, int k1);
void freematr(bitvector **matr,int n1);
float avgtlen(bitvector **matr,int n1, int k1);
/*void readnames(FILE *f,bitvector ***matr,int n,int k);*/
