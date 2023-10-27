/* constr.h */
#ifndef constrli
#define constrli

typedef unsigned int LYHYT;

LYHYT **readconstraints(FILE *f, int k);
void printconstr(LYHYT **constraints,int k);
LYHYT *readinterattr(FILE *f, int k);
LYHYT **readextraconstr(FILE *f, int k);
void printinterattr(LYHYT *t,int k);
#endif
