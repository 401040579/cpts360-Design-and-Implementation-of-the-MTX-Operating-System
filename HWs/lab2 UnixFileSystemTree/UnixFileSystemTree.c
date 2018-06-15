/*******************************************************************
* Name: Ran Tao
* SID: 11488080
* Class: Cpts360
* LAB ASSIGNMENT #2
* Created: 1/30/2017
* Edited: 2/2/2017
* Objectives: C programming; pointers, link-lists, trees
********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <libgen.h>

/*1. NODE type:*/
typedef struct node {
	char name[64];
	char nodeType;
	struct node *childPtr;
	struct node *siblingPtr;
	struct node *parentPtr;
}NODE;

/*2. Global Variables:*/
NODE *root, *cwd;                             /* root and CWD pointers */
char line[128];                               /* user input line */
char command[16], pathname[64];               /* user inputs */
char dirname[64], basename[64];               /* string holders */

char *cmd[] = { "mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
"quit", "help", "?", "menu", "reload", "save", 0 };

NODE* searchNode(char* dirname);
NODE* returnChild(char* dirname, char* basename);
NODE* returnSibling(NODE* child);
NODE* returnLast(NODE* parent);
NODE* searchChild(char* childname, NODE* parent);
NODE *insertNode(char *dirname, char *basename, char fileType);
int delNode(char* dirname, char* basename, char type);
void breakPath(char *pathname, char *dirname, char *basename);

// cmd mkdir
int mkdir(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	printf("dirname = %s basename = %s\n", dirname, basename);
	if (insertNode(dirname, basename, 'D'))
	{
		printf("mkdir done !\n");
		return 1;
	}
	else
	{
		printf("mkdir failed !\n");
		return 0;
	}
}

// cwd rmdir
int rmdir(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	printf("dirname = %s basename = %s\n", dirname, basename);
	if (delNode(dirname, basename, 'D'))
	{
		printf("rmdir done !\n");
		return 1;
	}
	else
	{
		printf("rmdir failed !\n");
		return 0;
	}
}

//cwd cd
int cd(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	if (set_cwd(dirname, basename))
		return 1;
	printf("cd failed !\n");
	return 0;
}

//cwd ls
int ls(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	if (print_children(dirname, basename)) return 1;
	else printf("ls failed !\n");
	return 0;
}

//cwd rm
int rm(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	printf("dirname = %s basename = %s\n", dirname, basename);
	if (delNode(dirname, basename, 'F'))
	{
		printf("rmdir done !\n");
		return 1;
	}
	else
	{
		printf("rmdir failed !\n");
		return 0;
	}
}

//cwd quit
int quit(char *pathname)
{
	save("");
	exit(0);
}

//cwd help
int help(char *pathname)
{
	printf("\n========================= MENU =========================\n"
		"mkdir rmdir ls cd pwd creat rm save reload quit\n"
		"========================================================\n");
	return 1;
}

//cwd creat
int creat(char *pathname)
{
	breakPath(pathname, &dirname, &basename);
	printf("dirname = %s basename = %s\n", dirname, basename);
	if (insertNode(dirname, basename, 'F'))
	{
		printf("creat done !\n");
		return 1;
	}
	else
	{
		printf("creat failed !\n");
		return 0;
	}
}

//cwd reload
int reload(char *pathname)
{
	char* filename = "myfile.txt";
	FILE *save_file;
	char type;
	char path[128];
	char tp[256];
	char tpPath[128];
	save_file = fopen(filename, "r");
	if (save_file == NULL) {
		printf("Failed to open %s. Make sure file name is myfile.txt\n", filename);
		return 0;
	}
	while (!feof(save_file))
	{
		fscanf(save_file, "%c %s", &type, &path);
		if (strcmp(tpPath, path) == 0) break;
		strcpy(tpPath, path);
		fgets(tp, 256, save_file);
		if (strlen(path) == 1) initialize();
		else
		{
			breakPath(path, &dirname, &basename);
			if (type == 'D') printf("mkdir %s\n", path);
			else printf("creat %s\n", path);
			insertNode(dirname, basename, type);
		}
	}
	printf("reload done !\n");
	return 1;
}

//cwd save
int save(char *pathname)
{
	FILE *save_file;
	char* filename = "myfile.txt";
	save_file = fopen(filename, "w");
	if (print_tree(save_file) == 0)
		return 0;
	fclose(save_file);
	return 1;
}

int findCmd(char *command)
{
	int i = 0;
	while (cmd[i]) {
		if (strcmp(command, cmd[i]) == 0)
			return i;
		i++;
	}
	return -1;
}

int initialize()
{
	root = NULL;
	root = insertNode(NULL, "/", 'D');
	if (!root)
		return 0;
	cwd = root;
	printf("Root initialized OK\n");
	return 1;
}

/*
mkdir(char *pathname)
(1). Break up pathname into dirname and basename, e.g.
ABSOLUTE: pathname=/a/b/c/d. Then dirname=/a/b/c, basename=d
RELATIVE: pathname= a/b/c/d. Then dirname=a/b/c,  basename=d

(2). Search for the dirname node:
ASSOLUTE pathname: search from /
RELATIVE pathname: search from CWD.

if nonexist ==> error messages and return
if exist but not DIR ==> errot message and return

(3). (dirname exists and is a DIR):
Search for basename in (under) the dirname node:
if already exists ==> error message and return;

ADD a new DIR node under dirname.
*/

void breakPath(char *pathname, char *dirname, char *basename)
{
	/*(1). Break up pathname into dirname and basename*/
	int temp = 0; int flag = 0;
	char pathTemp[128];
	char sTemp[128];
	strcpy(pathTemp, pathname);
	if (pathTemp[0] == '/') strcpy(dirname, "/");
	char *s = strtok(pathTemp, "/"); 
	if (s == NULL)
	{
		return;
	}
	else strcpy(sTemp, s);
	strcat(dirname, s);
	while (s = strtok(NULL, "/")) {
		strcat(dirname, "/");
		strcat(dirname, s);
		strcpy(basename, s);
		flag = 1;
	}
	if (flag == 1) {
		for (int i = 0; i < 64; i++)
		{
			if (dirname[i] == '/')
			{
				temp = i;
			}
		}
		//temp++;//keep '/' or not
		while (dirname[temp])
		{
			dirname[temp] = NULL;
			temp++;
		}
	}
	else
	{
		strcpy(basename, sTemp);
		if (dirname[0] == '/') strcpy(dirname, "/");
		else strcpy(dirname, "");
	}
}

NODE* searchNode(char* dirname)
{
	NODE* current = NULL;
	char *str = NULL;

	if (dirname == "")
		return cwd;

	if (isAbsolute(dirname))
		current = root; 
	else
		current = cwd; 

	char path[128];
	strcpy(path, dirname);

	str = strtok(path, "/"); 
	while (str)
	{
		current = searchChild(str, current);
		if (!current)
		{
			return NULL; 
		}

		str = strtok(0, "/");
	}
	return current;
}

int isDir(NODE* node)
{
	if (node == NULL) return 0;
	if (node->nodeType == 'D')
		return 1;
	else
		return 0;
}

NODE* searchChild(char* childname, NODE* parent)
{
	NODE* child = NULL;

	if (!parent || !isDir(parent) || !hasChild(parent))
		return NULL;

	child = parent->childPtr; //main

	while (child && strcmp(childname, child->name) != 0)
		child = child->siblingPtr;

	return child;
}

NODE* returnChild(char* dirname, char* basename)
{
	NODE* dir = searchNode(dirname);
	if (!dir || !isDir(dir) || !hasChild(dir))
		return NULL;
	return searchChild(basename, dir);
}

NODE* returnSibling(NODE* child)
{
	NODE* sibling = child->parentPtr->childPtr;
	while (sibling->siblingPtr)
	{
		if (sibling->siblingPtr == child)
			return sibling;
		sibling = sibling->siblingPtr;
	}
	return NULL;
}

int hasChild(NODE* node)
{
	if (node->childPtr)
		return 1;
	else
		return 0;
}

NODE* returnLast(NODE* parent)
{
	NODE* sibling = parent->childPtr;
	while (sibling->siblingPtr)
		sibling = sibling->siblingPtr;
	return sibling;
}

int isAbsolute(char* path)
{
	if (!path)
		return 1;
	if (path[0] == '/')
		return 1;
	else
		return 0;
}

NODE *insertNode(char *dirname, char *basename, char fileType)
{
	NODE* newNode = NULL;
	NODE* parent = NULL;
	NODE* sibling = NULL;
	if (root)
	{
		parent = searchNode(dirname);
		if (!parent || !isDir(parent) || returnChild(dirname, basename))
			return NULL;
	}
	// Use of calloc means node properties default to null. cited from github thanks!
	//newNode = (NODE *)malloc(sizeof(NODE));
	newNode = (NODE *)calloc(1, sizeof(NODE));
	strcpy(newNode->name, basename);
	newNode->nodeType = fileType;
	if (!parent)
		return newNode; // root
	newNode->parentPtr = parent;
	if (parent->childPtr)
	{
		sibling = returnLast(parent);
		sibling->siblingPtr = newNode;
	}
	else
		parent->childPtr = newNode;
	return newNode;
}

int isFile(NODE* node)
{
	if (node == NULL) return 0;
	if (node->nodeType == 'F')
		return 1;
	else
		return 0;
}

int delNode(char* dirname, char* basename, char type)
{
	NODE* deleting = returnChild(dirname, basename);

	if (type == 'F' && !isFile(deleting))
	{
		fprintf(stderr, "Not a file\n");
		return 0;
	}
	else if (type == 'D' && !isDir(deleting))
	{
		fprintf(stderr, "Not a directory\n");
		return 0;
	}
	if (remove_node2(deleting))
		return 1;
	return 0;
}

int remove_node2(NODE* deleting)
{
	NODE* sibling = NULL;
	if (!deleting || hasChild(deleting))
		return 0;
	if (deleting != root)
	{
		sibling = returnSibling(deleting);

		if (sibling) // Older sibling point to younger
			sibling->siblingPtr = deleting->siblingPtr;
		else		// Parent point to next child
			deleting->parentPtr->childPtr = deleting->siblingPtr;
	}
	//printf("Removing Node: %s\n", deleting->name);
	free(deleting);
	return 1;
}

int print_children(char* dirname, char* basename)
{
	NODE* nPtr = NULL;
	if (strlen(basename)!=0)
	{
		nPtr = returnChild(dirname, basename);
		if (!nPtr)
			return 0;
	}
	else
		nPtr = cwd;
	if (isFile(nPtr))
	{
		printf("%c \t %s\n", nPtr->nodeType, nPtr->name);
		return 1;
	}
	// List all children
	nPtr = nPtr->childPtr;
	while (nPtr)
	{
		printf("%c \t %s\n", nPtr->nodeType, nPtr->name);
		nPtr = nPtr->siblingPtr;  // Next child
	}
	return 1;
}

int set_cwd(char* dirname, char* basename)
{
	NODE* tmp = NULL;
	if (strlen(basename) == 0 && dirname[0] == '/')
	{
		cwd = root;
		return 1;
	}
	if (strcmp(basename,"..")==0)
	{
		if(cwd->parentPtr == NULL) cwd = root;
		else cwd = cwd->parentPtr;
		return 1;
	}
	if (!dirname && !basename)
	{
		cwd = root;
		return 1;
	}
	tmp = returnChild(dirname, basename);
	if (!tmp || isFile(tmp))
		return 0;
	cwd = tmp;
	return 1;
}

int pwd(char *pathname)
{
	char* wd = (char*)calloc(256, 1);
	if (print_cwd(&wd) == 0)
	{
		free(wd);
		return 0;
	}
	printf("%s\n", wd);
	free(wd);
	return 1;
}

int print_cwd(char** wd)
{
	return rpwd(cwd, wd);
}

int rpwd(NODE* dir, char** wd)
{
	if (dir == root)
	{
		strcat(*wd, "/");
		return 1;
	}
	if (rpwd(dir->parentPtr, wd) != 1)
		return 0;
	strcat(*wd, dir->name);
	strcat(*wd, "/");
	return 1;
}

int print_tree(FILE *out)
{
	return rec_print_tree(root, out);
}

int rec_print_tree(NODE* current, FILE *out)
{
	char* wd = NULL; // wd is current pathname

	if (current == NULL)
		return 1;

	wd = (char*)calloc(256, 1);

	if (rpwd(current, &wd) != 1)
	{
		free(wd);
		return 0;
	}
	if (strlen(wd) != 1)
	{
		wd[strlen(wd)-1] = NULL;
	}
	fprintf(out, "%c %s\n", current->nodeType, wd); // Current 
	printf("%c %s\n", current->nodeType, wd); // Current 
	rec_print_tree(current->childPtr, out);
	rec_print_tree(current->siblingPtr, out);

	free(wd);
	return 1;
}

main()
{
	initialize();      /* initialize / node of the file system tree */
	printf("Enter ? for help menu\n");
	while (1) {
		strcpy(dirname, "");
		strcpy(basename, "");
		strcpy(pathname, "");
		printf("\nCommand : ");
		//read a line containting  command[pathname]; // [ ] means optional
		gets(line, 128); line[strlen(line)] = 0;// kill the \r at end.
		sscanf(line, "%s %s", command, pathname);
		//printf("cmd: %s path: %s\n", command, pathname);

		//Find the command string and call the corresponding function;
		int(*fptr[])(char *) = { (int(*)())mkdir,rmdir,ls,cd,pwd,creat,rm,
		quit,help,help,help,reload,save };

		if (findCmd(command) == -1) 
			printf("invalid command\n");
		else 
		{
			int r = fptr[findCmd(command)](pathname);
		}
	}
}