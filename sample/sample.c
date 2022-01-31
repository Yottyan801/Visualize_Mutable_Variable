int twoBai(int number){
    number = number*2;
    return number;
}
int main()
{
  int b = 40,i;
  double c;
  for(i=0;i<5;i++)
    {
      b = twoBai(b);
    }
    c = 10+b;
    return 0;
}