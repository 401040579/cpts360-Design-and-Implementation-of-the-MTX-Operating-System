#include <stdio.h>
int A(int x, int y);
int B(int x, int y);

int *FP, *SP; //a global pointer

int main(int argc, char *argv[], char *env[])
{
	int a, b, c;
	printf("enter main\n");
	//print argv, argv, env
	printf("&argc: %x \n&argv: %x \n&env: %x \n", &argc, *argv, *env);
	printf("&a: %x \n&b: %x \n&c: %x \n", &a, &b, &c);
	a = 1; a = 2; c = 3;
	A(a, b);
	printf("exit main\n");
	
	return 0;
}

int A(int x, int y)
{
	int d , e, f;
	printf("enter A\n");
	printf("&x: %x \n&y: %x \n", &x, &y);
	printf("&d: %x \n&e: %x \n&f: %x \n", &d, &e, &f);
	d = 4; e = 5; f = 6;
	B(d, e);
	printf("exit A\n");
}

int B(int x, int y)
{
	int u, v, w, *fp, *sp;
	printf("enter B\n");
	
	printf("&x: %x \n&y: %x \n", &x, &y);
	printf("&u: %x \n&v: %x \n&w: %x \n", &u, &v, &w);
	
	u = 7; v = 8; w = 9;
	//set FP = ebp register;
	asm("movl %ebp, FP \n" 
	    "movl %esp, SP \n");	
	
	// Write C code to do (1)-(3) as specified below 
	printf("\ncpu's FP = %x \n", FP);
		
	printf("\n(1) Print the stack frame link list:\n");
	
	fp = FP;
	
	while(fp)
	{
		printf("FP: %x \t *FP: %x  \n", fp, *fp);
		fp = *fp;
	}
	
	printf("\n(2) Contents of Stack from FP to the stack frame of main():\n");
	fp = FP;
	
	while(*fp != 0)
	{
		printf("%x \t %x \n", fp, *fp);
		fp++;
	}
	printf("%x \t %x \n", fp, *fp);
	
	fp++;
	
	while(*fp != 0) {
		printf("%x \t %x \n", fp, *fp);
		fp++;
	}
	printf("%x \t %x \n", fp, *fp);
	
	printf("\nSP to main():\n");
	
	sp = SP;

	while(*sp != 0)
	{
		 printf("%x \t %x \n", sp, *sp);
		 sp++;
	}
	
	
			
	

	printf("exit B\n");
}
