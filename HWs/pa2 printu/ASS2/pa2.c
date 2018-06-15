/*******************************************************************
* Name: Ran Tao
* SID: 11488080
* Class: Cpts360
* Programming Assignment #2
* Created: 1/26/2017
* Edited: 1/27/2017
* Objectives: Stack Usage and YOUR myprintf() FUNCTION
               Ordered Link List as Priority Queue
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
/*1. Use putchar() to write YOUR OWN prints(char *s) to print a string.*/
int prints(char *s, ...) {
	while (*s) {
		putchar(*s++);
	}
}

/*2. typedef unsigned int u32;*/
typedef unsigned int u32;

int BASE = 10;
int BASEO = 8;
int BASEX = 16;

char *table = "0123456789ABCDEF";

int rpu(u32 x)
{
	char c;
	if (x) {
		c = table[x % BASE];
		rpu(x / BASE);
		putchar(c);
	}
}

int printu(u32 x)
{
	if (x == 0)
		putchar('0');
	else
		rpu(x);
	//putchar(' '); do we need it? from http://www.eecs.wsu.edu/~cs360/myprintf.html
}

/*3. WRITE YOUR OWN functions:*/
/*3.1 int printd(int x): print an integer (which may be negative!!)*/
int printd(int x) 
{
	if (x < 0)
	{
		putchar('-');
		x = -x;
	}

	if (x == 0)
	{
		putchar('0');
	}
	else
	{
		rpu(x);
	}
}

/*3.2 int printo(u32 x): print x in OCTal as 0.....*/
int rpuO(u32 x)
{
	char c;
	if (x) {
		c = table[x % BASEO];
		rpuO(x / BASEO);
		putchar(c);
	}
}

int printo(u32 x) 
{
	if (x == 0)
		putchar('0');
	else
		rpuO(x);
}

/*3.3 int printx(u32 x): print x in HEX.  as 0x....*/
int rpuX(u32 x)
{
	char c;
	if (x) {
		c = table[x % BASEX];
		rpuX(x / BASEX);
		putchar(c);
	}
}

int printx(u32 x)
{
	putchar('0'); putchar('x');
	if (x == 0)
		putchar('0');
	else
		rpuX(x);
}

/*4. WRITE a myprintf(char *fmt, ...) for formatted printing*/
typedef unsigned short u16;
int myprintf(char *fmt, ...) // cited some parts from book p70
{
	if (fmt == NULL) return 0;
	char *cp = fmt;
	int *ip = (int *)&fmt + 1;
	while (*cp)
	{
		if (*cp != '%')
		{
			putchar(*cp);
			if (*cp == '\n')
				putchar('\r');
		}
		else {
			*cp++;
			switch (*cp)
			{
			case 'c': putchar((char)*ip++);
				break;
			case 's': prints((char *)*ip++);
				break;
			case 'u': printu((u32)*ip++);
				break;
			case 'd': printd((int)*ip++);
				break;
			case 'o': printo((u32)*ip++);
				break;
			case 'x': printx((u32)*ip++);
				break;
			}
		}
		cp++;
	}
	return 1;
}

/*Programming Assignment - Ordered Link List as Priority Queue*/
typedef struct node {
	struct node *next;
	char name[64];
	int priority;
}NODE;

NODE *readyQueue = 0;    // an empty queue to start

/*A priority QUEUE is a link list ordered by priority. In a priority queue, nodes 
with the same priority are ordered First-In-First-Out (FIFO). */
int enqueue(NODE **queue, NODE *p)
{
	//enters p into *queue by priority
	NODE *cur = *queue;
	NODE *prev = NULL;
	if (p == NULL)
	{
		return 0;
	}

	if (*queue == NULL)
	{
		*queue = p;
		p->next = NULL;
	}
	else
	{
		while (cur != NULL && p->priority <= cur->priority)
		{
			prev = cur;
			cur = cur->next;
		}
		if (prev == NULL) 
		{
			p->next = cur;
			*queue = p;
		}
		else
		{
			prev->next = p;
			p->next = cur;
		}
	}
	return 1;
}

NODE *dequeue(NODE **queue)
{
	//delete the first node from *queue(if any) and
	//return the NODE(pointer) with the highest priroity
	NODE *temp = *queue;

	if (*queue != NULL) 
	{
		*queue = (*queue)->next;
	}
	free(temp);
	return *queue;
}

void printReadyQueue() 
{
	NODE *cur = readyQueue;
	while (cur != NULL)
	{
		myprintf("[%s %d]->", cur->name, cur->priority);
		cur = cur->next;
	}
}

/*Write your main() to test YOUR myprintf() and enqueue() functions.
   Use YOUR myprintf() function to print !!!!*/
int main(int argc, char *argv[], char *env[])
{
	srand(time(NULL));
	NODE *p;
	myprintf("argc = %d\n", argc);
	int i = 0;
	while (*argv) 
	{
		myprintf("argv[%d] = %s\n", i,*argv++);
		i++;
	}

	myprintf("enter a key to print env variables: ");
	char c = getchar(); //getchar();//get the enter key;

	i = 0;
	while (*env) {
		myprintf("env[%d] = %s\n", i, *env);
		*env++;
		i++;
	}

	i = 0;
	while (1) {
		myprintf("\nenter a key to insert a node to readyQueue: ");
		c = getchar(); //getchar();//get the enter key;
		p = (NODE *)malloc(sizeof(NODE));
		int priority = rand() % 10;
		char count[16];
		sprintf(count, "%d", i);
		char name[64] = "node";
		strcat(name, count);
		p->priority = priority;
		strcpy(p->name, name);
		myprintf("\n[%s %d]\n", name, priority);
		enqueue(&readyQueue, p);
		myprintf("readyQueue = ");
		printReadyQueue();

		if (i % 10 == 5)
		{
			myprintf("\n\n*******************************************************\n");
			myprintf("For enqueue every ten nodes, it will dequeue one node\n");
			NODE *deq = dequeue(&readyQueue);
			myprintf("After dequeue = ");
			printReadyQueue();
			myprintf("\nThe node [%s %d] with the highest priroity", deq->name, deq->priority);
			myprintf("\n*******************************************************\n");
		}
		i++;
	}
}