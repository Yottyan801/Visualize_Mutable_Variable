int main(void)
{
    int *a;
    int a2[2][2];
    a = &a2[1];
    a = &a2[0];
    return 0;
}