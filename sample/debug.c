#include <stdio.h>

int proc(int n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return 1;
    return proc(n - 1) + proc(n - 2);
}

int main()
{
    // input number
    int num1;
    int num2;

    //for loop count
    //int i,j;

    printf("input 1> ");
    scanf("%d", &num1);
    num2 = proc(num1);
    printf("output is %d", num2);
}
