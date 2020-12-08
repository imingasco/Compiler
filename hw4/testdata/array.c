
int func(){
    return 1;
}

int a[2];
int iii = 1;
int b[2][3];
int c[2][3][4];
int d[100000];
float x[2];
float y[1 + 2];
float z[1][2][3][5 * 6];
float jk[-3];

int main() {
    int a = -1;
    int local[3][4];
    local[a][x] = 0;
    local[func()] = 1;
    return 0;
}
