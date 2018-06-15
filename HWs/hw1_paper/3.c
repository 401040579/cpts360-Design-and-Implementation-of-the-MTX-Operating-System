int *FP,*SP; // a global pointer
main(int argc, char *argv[], char *env[])
{
  int a,b,c;
  printf("enter main\n");
  printf("argc: %x\nargv: %x\nenv: %x\n", &argc, *argv, *env);
  printf("a: %x\nb: %x\nc: %x\n", &a, &b, &c);
  a=1; b=2; c=3;
  A(a,b);
  printf("exit main\n");
}
int A(int x, int y)
{
  int d,e,f;
  printf("enter A\n");
  printf("x: %x\ny: %x\n", &x, &y);
  printf("d: %x\ne: %x\nf: %x\n", &d, &e, &f);
  d=4; e=5; f=6;
  B(d,e);
  printf("exit A\n");
}
int B(int x, int y)
{
  int u,v,w;
  int *b_fp;
  printf("enter B\n");
  printf("x: %x\ny: %x\n", &x, &y);
  printf("u: %x\nv: %x\nw: %x\n", &u, &v, &w);
  u=7; v=8; w=9;
  asm("movl %ebp, FP \n"
      "movl %esp, SP \n"); // set FP=CPU’s %ebp register
  // Write C code to DO (1)-(3) AS SPECIFIED BELOW
  //(1)
  printf("(1)Print the stack frame link list.\n");
  b_fp = FP;
  while(b_fp){
    printf("stack frame link list: address: %x data: %x\n", b_fp, *b_fp);
    if(*b_fp==0) break;
    b_fp = *b_fp;
  }

  //printf("in (B) FP: %x, SP: %x\n",FP,SP);

  //(2)
  printf("(2)Print in HEX the address and contents of the stack from FP to the stack frame of main().\n");
  while(FP!=b_fp){
    printf("address: %x data: %x\n", FP, *FP);
    FP++;
  }
  
  printf("exit B\n");
}
