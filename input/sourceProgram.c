int program(int a, int b, int c) {
    int i;
    int j;
    i = 0;
    if (a > (b + c)) {
        j = a + (b * c + 1);
    } else {
        j = a;
    }
    while (j <= 100) {
        j = j * 2;
    }
    return j;
}

int demo(int a) {
    a = a + 2;
    return a * 2;
}

int main() {
    int a;
	int b;
	int c;
    
    a = 3;
    b = 4;
    c = b;
    a = program(a, b, demo(c));
    return 0;
}

