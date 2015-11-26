FORCEINLINE inline void test(int* a)
{
    *a=1;
}

int main() {
    int x;
    test(&x);
    return 0;
}
