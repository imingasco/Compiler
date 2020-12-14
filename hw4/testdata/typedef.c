typedef int GLOBAL_INT, GLOBAL_INT_2;

GLOBAL_INT f() { return 0; }
GLOBAL_INT g(GLOBAL_INT x) { return 0; }
GLOBAL_INT h(GLOBAL_INT x[2]) { return 0; }

typedef int GLOBAL_INT_ARRAY[10];

void f2(GLOBAL_INT_ARRAY x) {}
void g2(GLOBAL_INT_ARRAY x[2]) {}

typedef GLOBAL_INT_ARRAY GLOBAL_MULTI_ARRAY[3][3][3];

void f3(GLOBAL_MULTI_ARRAY x) {}
void g3(GLOBAL_MULTI_ARRAY x[3][3][3]) {}

int main() {
  typedef int INT;
  typedef int TNI;
  typedef float FLOAT;
  typedef INT INTINT;
  typedef int INT;
  typedef INT INTINT;
  typedef INTINT ij;
  typedef INT ij;


  INT x = 0;
  TNI y = 0;
  INTINT z = 0;
  FLOAT f = 0;
  GLOBAL_INT_ARRAY ii[3][3][3];
  int jj[5][3][3][10][3][3][3];
  int kk[9][3][3][3][3][3][10];
  INTINT ll[3][3][3][10];
  INT mm[3][3][3][3];
  TNI nn[2][5];

  typedef int X[2];
  X A;

  typedef X Y;
  Y B;

  typedef X Z[2];
  Z zzz;

  typedef Z Q[2][3][4];

  Q asdasd;
  Q QQQ[12312];

  typedef void VOID;
  typedef VOID VOID1, VOID2, VOID3;
  typedef VOID1 TEST[10];
  typedef int INT1[1], INT2[2], INT123[1][2][3];

  typedef GLOBAL_INT_ARRAY LOCAL_INT_ARRAY;
  LOCAL_INT_ARRAY P;
  LOCAL_INT_ARRAY S[2];
  LOCAL_INT_ARRAY R[3];

  INT2 ad;

  x = h(ad);
  x = h(ad[0]);
  f2(S[0]);
  g2(S);
  g2(R);
  g2(S[2]);
  g2(P);
  f3(ii);
  f3(jj);
  f3(kk);
  f3(kk[0][0][0]);
  f3(ll);
  f3(mm);
  f3(ll[0]);
  f3(jj[0][0][0]);
  g3(kk);
  g3(ll);
  g3(mm);
  g3(nn);
  g3(jj[1]);
}
