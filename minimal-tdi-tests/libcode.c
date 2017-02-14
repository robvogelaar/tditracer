

#define code_4K \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k);k%=(1+l);l%=(1+k); \
  k%=(1+l);l%=(1+k);l%=(1+k);l%=(1+k); \
  while (i) {i--;} if (j == 0) return ; j--;

#define code_1M {\
\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
  code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;code_4K;\
}

#define code_2M {code_1M;code_1M;}
#define code_4M {code_1M;code_1M;code_1M;code_1M;}
#define code_8M {code_1M;code_1M;code_1M;code_1M;code_1M;code_1M;code_1M;code_1M;}

extern void f1(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
extern void f2(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
extern void f3(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
extern void f4(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
extern void f5(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
extern void f6(int _i, int _j) {

  long long int i= (unsigned long long)_i * 1000000LL;
  long long int j= _j;
  long long int k=0;
  long long int l=0;

  code_4M;
  code_4M;
  code_2M;
}
