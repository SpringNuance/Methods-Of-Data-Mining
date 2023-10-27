/* apufunktiot.h */

float avgf(float *t,int l);
double maxd(double *t,int l);
float maxf(float *t,int l);
double mind(double *t,int l);
double minf(float *t,int l);
float stdevf(float *t,int l,float avgx);
float osuusd(double *t,int l);
int ekaarvof(float *t,int l);
int vikaarvof(float *t, int l);
int nofrows(FILE *f);
float *normalisoi(float *t,int l);
int siirrapuuttuvat(float *t, int l);
float sumf(float *t, int l);
int aktlkm(float *t, int l, float thr);
void printvector(float *t,int l);
void printtable(float **t, int riv, int sar);
void selprinttable(float **t, int riv, int sar, int l, float thr);
int occursinset(int a, int *table, int size);
int indexinset(int a, int *table, int size);
int emptystring(char *t);
