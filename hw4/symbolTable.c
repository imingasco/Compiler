#include "symbolTable.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
// This file is for reference only, you are not required to follow the implementation. //

int HASH(char * str) {
	int idx=0;
	while (*str){
		idx = idx << 1;
		idx+=*str;
		str++;
	}
	return (idx & (HASH_TABLE_SIZE-1));
}

SymbolTable symbolTable;

SymbolTableEntry* newSymbolTableEntry(int nestingLevel)
{
    SymbolTableEntry* symbolTableEntry = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    symbolTableEntry->nextInHashChain = NULL;
    symbolTableEntry->prevInHashChain = NULL;
    symbolTableEntry->nextInSameLevel = NULL;
    symbolTableEntry->sameNameInOuterLevel = NULL;
    symbolTableEntry->attribute = NULL;
    symbolTableEntry->name = NULL;
    symbolTableEntry->nestingLevel = nestingLevel;
    return symbolTableEntry;
}

void removeFromHashChain(int hashIndex, SymbolTableEntry* entry)
{
    if(entry == NULL) return;
    if(entry->prevInHashChain != NULL){
        entry->prevInHashChain->nextInHashChain = entry->nextInHashChain;
    }
    if(entry->nextInHashChain != NULL){
        entry->nextInHashChain->prevInHashChain = entry->prevInHashChain;
    }
    if(entry == symbolTable.hashTable[hashIndex]){
        symbolTable.hashTable[hashIndex] = entry->nextInHashChain;
    }
    entry->prevInHashChain = entry->nextInHashChain = NULL;
}

void enterIntoHashChain(int hashIndex, SymbolTableEntry* entry)
{
    if(entry == NULL) return;
    SymbolTableEntry *headEntry = symbolTable.hashTable[hashIndex];
    entry->nextInHashChain = headEntry;
    if(headEntry != NULL){
        headEntry->prevInHashChain = entry;
    }
    symbolTable.hashTable[hashIndex] = entry;
}

void initializeSymbolTable()
{
    for(int i = 0; i < HASH_TABLE_SIZE; i++){
        symbolTable.hashTable[i] = NULL;
    }
    symbolTable.scopeDisplay = malloc(sizeof(SymbolTableEntry*) * MAX_SCOPE_NUM);
    for(int i = 0; i < MAX_SCOPE_NUM; i++){
        symbolTable.scopeDisplay[i] = NULL;
    }
    symbolTable.currentLevel = -1;
    symbolTable.scopeDisplayElementCount = 0;
}

void symbolTableEnd()
{
}

SymbolTableEntry* retrieveSymbol(char* symbolName)
{
    int hashIndex = HASH(symbolName);
    SymbolTableEntry *entry = symbolTable.hashTable[hashIndex];
    while(entry != NULL){
        if(strcmp(entry->name, symbolName) == 0){
            return entry;
        }
        entry = entry->nextInHashChain;
    }
    return NULL;
}

SymbolTableEntry* enterSymbol(char* symbolName, SymbolAttribute* attribute)
{
    SymbolTableEntry *entry = newSymbolTableEntry(symbolTable.currentLevel);
    entry->name = symbolName;
    entry->attribute = attribute;
    // construct same level chain
    entry->nextInSameLevel = symbolTable.scopeDisplay[symbolTable.currentLevel];
    symbolTable.scopeDisplay[symbolTable.currentLevel] = entry;
    // construct same name chain
    SymbolTableEntry *retrEntry = retrieveSymbol(entry->name);
    entry->sameNameInOuterLevel = retrEntry;
    removeFromHashChain(HASH(entry->name), retrEntry);
    enterIntoHashChain(HASH(entry->name), entry);
    return entry;
}

//remove the symbol from the current scope
void removeSymbol(char* symbolName)
{
}

int isDeclaredLocally(char* symbolName)
{
    int hashIndex = HASH(symbolName);
    SymbolTableEntry *entry = symbolTable.hashTable[hashIndex];
    while(entry != NULL){
        if(strcmp(entry->name, symbolName) == 0){
            return entry->nestingLevel == symbolTable.currentLevel;
        }
        entry = entry->nextInHashChain;
    }
    return 0;
}

void openScope()
{
    symbolTable.currentLevel++;
}

void closeScope()
{
    SymbolTableEntry *entry = symbolTable.scopeDisplay[symbolTable.currentLevel], *tmpEntry;
    while(entry != NULL){
        tmpEntry = entry;
        removeFromHashChain(HASH(entry->name), entry);
        enterIntoHashChain(HASH(entry->name), entry->sameNameInOuterLevel);
        entry = entry->nextInSameLevel;
        //free(tmpEntry);
    }
    symbolTable.scopeDisplay[symbolTable.currentLevel] = NULL;
    symbolTable.currentLevel--;
}
