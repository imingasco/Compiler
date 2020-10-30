#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<math.h>
#include"header.h"

#define TABLE_SIZE	512

symtab * hash_table[TABLE_SIZE];
extern int linenumber;

int HASH(char * str){
	int idx=0;
	while(*str){
		idx = idx << 1;
		idx+=*str;
		str++;
	}	
	return (idx & (TABLE_SIZE-1));
}

/*returns the symbol table entry if found else NULL*/

symtab * lookup(char *name){
	int hash_key;
	symtab* symptr;
	if(!name)
		return NULL;
	hash_key=HASH(name);
	symptr=hash_table[hash_key];

	while(symptr){
		if(!(strcmp(name,symptr->lexeme)))
			return symptr;
		symptr=symptr->front;
	}
	return NULL;
}

void insertRW(char *name){
	int hash_key;
	symtab* ptr;
	symtab* symptr=(symtab*)malloc(sizeof(symtab));	
	
	hash_key=HASH(name);
	ptr=hash_table[hash_key];
	
	if(ptr==NULL){
		/*first entry for this hash_key*/
		hash_table[hash_key]=symptr;
		symptr->front=NULL;
		symptr->back=symptr;
	}
	else{
		symptr->front=ptr;
		ptr->back=symptr;
		symptr->back=symptr;
		hash_table[hash_key]=symptr;	
	}
	strcpy(symptr->lexeme,name);
	symptr->line=linenumber;
	symptr->counter=1;
    symptr->reserved=1;
}

void insertID(char *name){
	int hash_key;
	symtab* ptr;
	symtab* symptr=(symtab*)malloc(sizeof(symtab));	
	
	hash_key=HASH(name);
	ptr=hash_table[hash_key];
	
	if(ptr==NULL){
		/*first entry for this hash_key*/
		hash_table[hash_key]=symptr;
		symptr->front=NULL;
		symptr->back=symptr;
	}
	else{
		symptr->front=ptr;
		ptr->back=symptr;
		symptr->back=symptr;
		hash_table[hash_key]=symptr;	
	}
	
	strcpy(symptr->lexeme,name);
	symptr->line=linenumber;
	symptr->counter=1;
    symptr->reserved=0;
}

void printSym(symtab* ptr) 
{
	printf("%-32s %d\n", ptr->lexeme, ptr->counter);
}

void sortSym(symtab* symHead, symtab* symPtr){
	symtab* symCur = symHead;
	while(symCur->front != NULL && strcmp(symPtr->lexeme, symCur->front->lexeme) > 0){
		symCur = symCur->front;
	}
	symtab* symInsertion = (symtab*)malloc(sizeof(symtab));
	// copy the info
	strcpy(symInsertion->lexeme, symPtr->lexeme);
	symInsertion->line = symPtr->line;
	symInsertion->counter = symPtr->counter;
	// reconstruct the linked list
	// we may ignore the parameter "back" here
	symInsertion->front = symCur->front;
	symCur->front = symInsertion;
}

void printSymTab()
{
    int i;
    printf("----- Symbol Table ---------\n");
    symtab* symHead = (symtab*)malloc(sizeof(symtab));
    symHead->front = NULL;
    for (i=0; i<TABLE_SIZE; i++)
    {
        symtab* symptr;
		symptr = hash_table[i];
		while (symptr != NULL)
		{
            if(!symptr->reserved)
			    sortSym(symHead, symptr);
	    	symptr=symptr->front;
		}
    }
    while(symHead->front != NULL){
    	printSym(symHead->front);
    	symHead = symHead->front;
    }
}
