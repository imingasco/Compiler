void f() {}
int g() { return 1; }
int b;
int h() { return b; }
int f1(int x){ 
    float shit = 3.3;
    return shit * 2; 
}
int f2(int x, int y) { return x - y; }
float g1() { return 1.2; }
float g2(float x) { return 1.0 / x; }
int f3(int a[2]) { return a[1]; }
int f4(int a[2][3]) { return a[0][1]; }
int f5(int a[][10]) { return a[0][0] + g2(0); }
void f6(int a[]) {}

int fib(int n) {
  if (n <= 1)
    return 1;
  return fib(n - 1) + fib(n - 2);
}

void yy(int a, float b, int c[][10], float d[][2][5]){
    int i;
    int j;
    if(a > b && c[0][1] * d[0][1][3] < (a - b)){
        for(i = 0; i < a; i = i + 1){
            while(j > 0){
                j = j - 1;
                return fuck;
            }
        }
    }
    a = a + 1;
    return;
}

int main() {
  int a;
  float f;
  int b[2], c[2][3], d[100][10], e[100][3];
  float g[10][2][5], k[10][3][6];
  int y;
  y = f1(f);
  y = f1(a);
  f3(b);
  f4(c);
  f4(e);
  f5(d);
  f6(b);
  f();
  yy(a, f, d, g);
  y = g2(f1(f1(0)));
  y = f2(y, fib(y));
}
