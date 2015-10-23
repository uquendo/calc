extern void link_error();

void test(int* RESTRICT a, int* RESTRICT b)
{
    *a=1;
    *b=2;
    if(*a==2) link_error();
    return;
}

int main() {
    int x,y;
    test(&x,&y);
    return 0;
}
