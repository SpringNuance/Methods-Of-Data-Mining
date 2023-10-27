#include <stdio.h>
#include <stdlib.h>
#include "error.h"


void error(int code) {
  switch (code) {
  case 0:  printf("Extra or missing command line parameters. The syntax is\n\n");
    printf("kingfisher -i<inputfile> -k<max item number> -M<threshold> [-w<measure> -t<type of rules>\n -m<minfr> -c<mincf> -l<rule length> -q<#rules to print> -p<output format>\n -o<output file> -e<compatibility constraints> -b<extra constraints> -a<attr.file> -f<fixed consequences> -u -d]\n\n"); 
    printf("-i<inputfile>\n");    
    printf("\t in transaction form; each row lists attributes which have value 1 in data\n");
    printf("-k<max item number>\n");
printf("\t integer, >=2 (otherwise not sensible)\n");
 printf("-w<measure>\n");
 printf("\t1=Fisher's p, 2=ln(p) (default), 3=chi2, 4=MI (mutual information)\n");
    printf("-M<threshold for the measure>\n");
printf("\t should be [0,1] for measure 1, <0.0 for measure 2, and >0 for measure 3\n\t(if the program is slow, use smaller thresholds)  \n");
 printf("-t<type of rules>\n");
 printf("\t 1=only positive rules (default), 2=only negative rules, 3=both positive and\n\t negative rules\n");
 printf("[-m<minfr>]\n");
 printf("\t minimum frequency threshold, minimum value for p(XA), in [0,1]\n");
 printf("\t (default 5/n, where n is the number of rows)\n"); 
 printf("[-c <mincf>]\n");
printf("\t minimum confidence threshold, minimum value for P(A|X), in [0,1] (default 0.0) \n");
 printf("[-l<rule length>]\n");
 printf("\t maximum number of attributes in the rule, including consequent\n\t (default 50; if the program is slow, try a smaller value)\n");
 printf("[-q<#rules to print>]\n"); 
printf("\t number of best rules which are searched, should be >=0 (default 100) \n\t (if the program is slow, try a smaller value)\n");
 printf("[-p<output format>]\n");
printf("\t 1 or 2 (default 1)\n");
 printf("\t format 1 outputs X -> A fr cf lift M (X -> ~A for negative rules)\n");
 printf("\t format 2 outputs X A or X ~A (negative rules)\n");
 printf("[-o<outputfile>]\n");
 printf("\t(default stdout)\n");
 printf("[-e<compatibility constraints>]\n");
 printf("\t file for compatibility constraints lists on each row attributes, any two\n\t of which cannot occur together\n");
 printf("[-b<extra constraints>]\n");
printf("\t file for extra constraints lists on each row first a consequence attribute\n\t and then its forbidden condition attributes\n");
/* printf("[-a <attr.file>]\n");
   printf("\t file for listing interesting attributes. If this option is given, then all\n\t discovered rules will contain at least one interesting attribute.\n");*/
 printf("[-u]\n");
 printf("\t uses an upperbound estimate for  ln(1+q1+...+q1q2...qm)\n");
 printf("[-d]\n");
 printf("\t disables continuity correction (otherwise used with chi2)\n");

 printf("\n Example: kingfisher -i mushroom.dat -k120 -M-4000\n");
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
  case 8: printf("All attributes pruned!\n");
  break;
  case 9: printf("This error should not happen!!!\n");
  break;
  case 10: printf("Errourneously called delLeaf!\n");
  break;
  case 11: printf("Error in reduceParSize!\n");
  break;
  case 12: printf("Unknown attribute in bestrule\n");
    break;
  case 13:  printf("Extra or missing command line parameters. The syntax is\n");
    printf("testaaja -r<rules> -f<testidata> -n<#rows> -k<#cols> \n");

  break;
  case 14:  printf("Extra or missing command line parameters. The syntax is\n");
    printf("ppros -r<rules> -h<output for chi2-rules> -j<output for J-rules> -c<output for certainty factor rules> -z<output for z-score rules> -f<output for frequent rules> -n<#rows> -k<#cols> \n");
  break;

  case 15:  printf("Extra or missing command line parameters. The syntax is\n");
    printf("pprosmol -r<rules> -h<output for chi2-rules> -j<output for J-rules> -c<output for certainty factor rules> -z<output for z-score rules> -f<output for frequent rules> -n<#rows in data> -k<#cols> -q<mincf>\n");
  break;
  case 16: printf("Underflow, overflow or nan occured in estimating upper- or lowerbounds for ln(p). Check measures.c.\n");
    break;
  }
  exit(EXIT_FAILURE);
}
