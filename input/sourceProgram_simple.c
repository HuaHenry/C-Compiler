
int myAdd(int x, int y)
{
	return x + y;
}

int main() {
    int a;
	int b;
	int c;
	
	a = 0;
	b = a + 1;
	a = myAdd(a, b);
    return myAdd(1, 2);
}


