#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

int ARoffset = 4;
int constLabelIndex = 1;
int floatLabelIndex = 1;
int labelIndex = 1;
int ifExitLabelIndex = 1;
int whileExitLabelIndex = 1;
int forExitLabelIndex = 1;
short t_reg_status[7];
short ft_reg_status[8];

#define UNUSED 0
#define NS     1
#define S      2

#define FLOAT_DIMENSION -64
#define IGNORE_DIMENSION -1
#define INVALID_STRING_TYPE 1
#define INVALID_VOID_TYPE 2
#define INVALID_PTR_TYPE 4
#define RETURN_REG 64

FILE *fp;

int get_t_reg(){
    for(int i = 0; i < 7; i++){
        if(t_reg_status[i] == UNUSED){
            t_reg_status[i] = NS;
            return i;
        }
    }
    fprintf(stderr, "free your register!\n");
    exit(1);
}

void free_t_reg(int t_reg_num){
    t_reg_status[t_reg_num] = UNUSED;
}

int get_ft_reg(){
    for(int i = 0; i < 8; i++){
        if(ft_reg_status[i] == UNUSED){
            ft_reg_status[i] = NS;
            return i;
        }
    }
    fprintf(stderr, "free your floating point register!\n");
    exit(1);
}

void free_ft_reg(int ft_reg_num){
    ft_reg_status[ft_reg_num] = UNUSED;
}

void prologue(char *functionName){
    fprintf(fp, "_start_%s:\n", functionName);
    fprintf(fp, "\tsd ra, 0(sp)\n");
    fprintf(fp, "\tsd fp, -8(sp)\n");
    fprintf(fp, "\tadd fp, sp, -8\n");
    fprintf(fp, "\tadd sp, sp, -16\n");
    fprintf(fp, "\tla ra, _frameSize_%s\n", functionName);
    fprintf(fp, "\tlw ra, 0(ra)\n");
    fprintf(fp, "\tsub sp, sp, ra\n");
    int offset = 8;
    for(int i = 0; i < 7; i++, offset += 8)
        fprintf(fp, "\tsd t%d, %d(sp)\n", i, offset);
    for(int i = 2; i < 12; i++, offset += 8)
        fprintf(fp, "\tsd s%d, %d(sp)\n", i, offset);
    fprintf(fp, "\tsd fp, %d(sp)\n", offset);
    offset += 8;
    for(int i = 0; i < 8; i++, offset += 4)
        fprintf(fp, "\tfsw ft%d, %d(sp)\n", i, offset);
    fflush(fp);
    return;
}

void epilogue(char *functionName){
    fprintf(fp, "_end_%s:\n", functionName);
    int offset = 8;
    for(int i = 0; i < 7; i++, offset += 8)
        fprintf(fp, "\tld t%d, %d(sp)\n", i, offset);
    for(int i = 2; i < 12; i++, offset += 8)
        fprintf(fp, "\tld s%d, %d(sp)\n", i, offset);
    fprintf(fp, "\tld fp, %d(sp)\n", offset);
    offset += 8;
    for(int i = 0; i < 8; i++, offset += 4)
        fprintf(fp, "\tflw ft%d, %d(sp)\n", i, offset);
    fprintf(fp, "\tld ra, 8(fp)\n");
    fprintf(fp, "\tmv sp, fp\n");
    fprintf(fp, "\tadd sp, sp, 8\n");
    fprintf(fp, "\tld fp, 0(fp)\n");
    fprintf(fp, "\tjr ra\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "\t_frameSize_%s: .word %d\n", functionName, 180 + ARoffset);
    fflush(fp);
    ARoffset = 4;
    return;
}

void loadConst(int constVal, int reg_num){
    unsigned int upper = (unsigned int)constVal >> 12;
    // int lower = constVal & 0x00000FFF;
    int lower = constVal << 20 >> 20;
    if(constVal >= -2048 && constVal < 2048){
	fprintf(fp, "\taddi t%d, x0, %d\n", reg_num, constVal);
	return;
    }
    if(lower >> 11 && upper < (1 << 20) - 1){
	upper++;
    }
    fprintf(fp, "\tlui t%d, %u\n", reg_num, upper);
    fprintf(fp, "\taddi t%d, t%d, %d\n", reg_num, reg_num, lower);
    if(lower >> 11 && upper == (1 << 20) - 1){
	int t_reg_num = get_t_reg();
	loadConst(4096, t_reg_num);
	fprintf(fp, "\tadd t%d, t%d, t%d\n", reg_num, reg_num, t_reg_num);
	free_t_reg(t_reg_num);
    }
    fflush(fp);
}

void loadFloat(float constVal, int reg_num){
    int t_reg_num = get_t_reg();
    int *ptr = &constVal;
    fprintf(fp, ".data\n");
    fprintf(fp, "\tFC_%d: .word %d\n", floatLabelIndex, *ptr);
    fprintf(fp, ".text\n");
    fprintf(fp, "\tla t%d, FC_%d\n", t_reg_num, floatLabelIndex);
    fprintf(fp, "\tflw ft%d, 0(t%d)\n", reg_num, t_reg_num);
    fflush(fp);
    floatLabelIndex++;
    free_t_reg(t_reg_num);
}

void codeGen(AST_NODE *root)
{
    fp = fopen("output.s", "w");
    if(fp == NULL){
        fprintf(stderr, "cannot open output.s.\n");
        return;
    }
    genProgramNode(root);
}

void genProgramNode(AST_NODE *programNode)
{
    if(programNode->child == NULL) return;
    fprintf(fp, ".data\n");
    
    AST_NODE *globalDecl = programNode->child;
    // globalDecl is a func_decl or a var_decl_list
    while(globalDecl != NULL){
        if(globalDecl->nodeType == VARIABLE_DECL_LIST_NODE){
            fprintf(fp, ".data\n");
            AST_NODE *declList = globalDecl->child;
            while(declList != NULL){
                genDeclarationNode(declList);
                declList = declList->rightSibling;
            }
        }
        else if(globalDecl->nodeType == DECLARATION_NODE){
            switch(globalDecl->semantic_value.declSemanticValue.kind){
                case VARIABLE_DECL:
                    fprintf(fp, ".data\n");
                    break;
                case FUNCTION_DECL:
                    fprintf(fp, ".text\n");
                    break;
            }
            genDeclarationNode(globalDecl);
        }
        else{
            printf("There is an exception type = %d in program node.\n", globalDecl->nodeType);
        }
        globalDecl = globalDecl->rightSibling;
    }
}

void genDeclarationNode(AST_NODE* declarationNode)
{
    switch(declarationNode->semantic_value.declSemanticValue.kind){
        case TYPE_DECL:
            break;
        case VARIABLE_DECL:
            genDeclareVariable(declarationNode);
            break;
        case FUNCTION_DECL:
            genDeclareFunction(declarationNode);
            break;
    }
}

void genDeclareVariable(AST_NODE *declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    SymbolTableEntry *typeEntry = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    while(idNode != NULL){
        AST_NODE *exprNode;
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
        // scalar type or error type
        if(typeEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
            switch (idNode->semantic_value.identifierSemanticValue.kind){
                case NORMAL_ID:
                    if(isGlobal(idEntry))
                        fprintf(fp, "\t_%s: .word 0\n", idName);
                    else{
                        idEntry->offset = ARoffset;
                        ARoffset += 4;
                    }   
                    break;
                case ARRAY_ID:
                    if(isGlobal(idEntry)){
                        int space = getArraySpace(idEntry);
                        fprintf(fp, "\t_%s: .space %d\n", idName, space);
                    }
                    else{
                        int space = getArraySpace(idEntry);
                        idEntry->offset = ARoffset + space - 4;
                        ARoffset += space;
                    }
                    break;
                case WITH_INIT_ID:
                    // must be CONST_VALUE_NODE
                    exprNode = idNode->child;
                    if(isGlobal(idEntry)){
                        if(idNode->dataType == INT_TYPE)
                            fprintf(fp, "\t_%s: .word %d\n", idName, exprNode->semantic_value.const1->const_u.intval);
                        else{
                            int *ptr = &(exprNode->semantic_value.const1->const_u.fval);
                            fprintf(fp, "\t_%s: .word %d\n", idName, *ptr);
                        }
                    }
                    else{
                        idEntry->offset = ARoffset;
                        if(idNode->dataType == INT_TYPE){
                            int t_reg_num = get_t_reg();
                            int constVal = exprNode->semantic_value.const1->const_u.intval;
                            loadConst(constVal, t_reg_num);
                            fprintf(fp, "\tsw t%d, -%d(fp)\n", t_reg_num, ARoffset);
                            free_t_reg(t_reg_num);
                        }
                        else{
                            // floating number
                            int ft_reg_num = get_ft_reg();
                            float constVal = exprNode->semantic_value.const1->const_u.fval;
                            loadFloat(constVal, ft_reg_num);
                            fprintf(fp, "\tfsw ft%d, -%d(fp)\n", ft_reg_num, ARoffset);
                            free_ft_reg(ft_reg_num);
                        }
                        ARoffset += 4;
                    }
                    break;
            }
        } 
        // array type
        else{
            AST_NODE *exprNode;
            int space = getArraySpace(idEntry);
            if(isGlobal(idEntry))
                fprintf(fp, "\t_%s: .space %d\n", idName, space);
            else{
                idEntry->offset = ARoffset + space - 4;
                ARoffset += space;
            }
        }
        idNode = idNode->rightSibling;
	fflush(fp);
    }
}

int getArraySpace(SymbolTableEntry *idEntry){
    int space = 1;
    ArrayProperties property = idEntry->attribute->attr.typeDescriptor->properties.arrayProperties;
    for(int i = 0; i < property.dimension; i++)
        space *= property.sizeInEachDimension[i];
    return space * 4;
}

int isGlobal(SymbolTableEntry *entry){
    return !(entry->nestingLevel);
}

void genDeclareFunction(AST_NODE* declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    AST_NODE *paramListNode = idNode->rightSibling;
    AST_NODE *blockNode = paramListNode->rightSibling;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    char *typeName = typeNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *typeEntry = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    prologue(idName);
    // declareFunctionParam(paramListNode, symbolAttr->attr.functionSignature);
    genBlockNode(blockNode);
    epilogue(idName);
}

/*
void declareFunctionParam(AST_NODE *paramListNode, FunctionSignature *functionSignature){
    Parameter **paramListTail = &(functionSignature->parameterList);
    AST_NODE *paramNode = paramListNode->child;
    while(paramNode != NULL){
        AST_NODE *typeNode = paramNode->child;
        AST_NODE *idNode = typeNode->rightSibling;
        DATA_TYPE dataType;
        checkTypeNode(typeNode, &dataType);
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        // declare param
        if(dataType != ERROR_TYPE && isDeclaredLocally(idName)){
            char errMsg[ERR_MSG_LEN];
            sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
            printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
        }
        else{
            declareVariable(paramNode);
            SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
            symbolAttr->attributeKind = VARIABLE_ATTRIBUTE;
            symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
            switch (idNode->semantic_value.identifierSemanticValue.kind){
                case NORMAL_ID:
                    symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                    symbolAttr->attr.typeDescriptor->properties.dataType = dataType;
                    break;
                case ARRAY_ID:
                    symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                    symbolAttr->attr.typeDescriptor->properties.arrayProperties.elementType = dataType;
                    getArrayDimensionAndSize(symbolAttr, idNode, 1);
                    break;  
            }
            idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, symbolAttr);
        }
        // construct paramList
        if(dataType != ERROR_TYPE){
            // SymbolTableEntry *entry = retrieveSymbol(idName);
            SymbolTableEntry *entry = paramNode->child->rightSibling->semantic_value.identifierSemanticValue.symbolTableEntry;
            functionSignature->parametersCount++;
            *paramListTail = (Parameter *)malloc(sizeof(Parameter));
            (*paramListTail)->parameterName = entry->name;
            (*paramListTail)->type = entry->attribute->attr.typeDescriptor;
            (*paramListTail)->next = NULL;
            paramListTail = &((*paramListTail)->next);
        }
        paramNode = paramNode->rightSibling;
    }
}
*/

void genWhileStmt(AST_NODE* whileNode)
{
    AST_NODE *testExprRoot = whileNode->child;
    AST_NODE *stmtNode = testExprRoot->rightSibling;
    
    int successLabelIndex = labelIndex++;
    int successJumpIndex = labelIndex++;
    int failLabelIndex = labelIndex++;
    int exitLabelIndex = whileExitLabelIndex++;
    // unconditional jump to check
    int t_reg_num = get_t_reg();
    fprintf(fp, "\tla t%d, L%d\n", t_reg_num, failLabelIndex);
    fprintf(fp, "\tjalr x0, 0(t%d)\n", t_reg_num);
    free_t_reg(t_reg_num);
    fprintf(fp, "L%d:\n", successLabelIndex);
    // gen success block
    genStmtNode(stmtNode);
    // gen while test
    fprintf(fp, "L%d:\n", failLabelIndex);
    genExprNode(testExprRoot);
    // case: while(a + 1.1), where a is a float
    if(testExprRoot->dataType == FLOAT_TYPE){
        int t_reg_num = get_t_reg();
        fprintf(fp, "\tfmv.x.w t%d, ft%d\n", t_reg_num, testExprRoot->place);
        free_ft_reg(testExprRoot->place);
        testExprRoot->place = t_reg_num;
    }
    t_reg_num = get_t_reg();
    fprintf(fp, "\tbeqz t%d, while_Exit_%d\n", testExprRoot->place, exitLabelIndex);
    fprintf(fp, "L%d:\n", successJumpIndex);
    fprintf(fp, "\tla t%d, L%d\n", t_reg_num, successLabelIndex);
    fprintf(fp, "\tjalr x0, 0(t%d)\n", t_reg_num);
    free_t_reg(t_reg_num);
    fprintf(fp, "while_Exit_%d:\n", exitLabelIndex);
    fflush(fp);
    free_t_reg(testExprRoot->place);
}


void genForStmt(AST_NODE* forNode)
{
    AST_NODE *initAssignExprRoot = forNode->child;
    AST_NODE *initAssignExpr = initAssignExprRoot->child;
    AST_NODE *relopExprRoot = initAssignExprRoot->rightSibling;
    AST_NODE *relopExpr = relopExprRoot->child;
    AST_NODE *updateAssignExprRoot = relopExprRoot->rightSibling;
    AST_NODE *updateAssignExpr = updateAssignExprRoot->child;
    AST_NODE *stmtNode = updateAssignExprRoot->rightSibling;
    int successLabelIndex = labelIndex++;
    int testLabelIndex = labelIndex++;
    int incrementLabelIndex = labelIndex++;
    int exitLabelIndex = forExitLabelIndex++;
    while(initAssignExpr){
        if(initAssignExpr->nodeType == STMT_NODE && initAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(initAssignExpr);
        else{
            genExprNode(initAssignExpr);
        }
        initAssignExpr = initAssignExpr->rightSibling;
    }
    fprintf(fp, "L%d:\n", testLabelIndex);
    while(relopExpr){
        genExprNode(relopExpr);
        if(relopExpr->rightSibling == NULL){
            if(relopExpr->dataType == FLOAT_TYPE){
                int t_reg_num = get_t_reg();
                fprintf(fp, "\tfmv.x.w t%d, ft%d\n", t_reg_num, relopExpr->place);
                free_ft_reg(relopExpr->place);
                relopExpr->place = t_reg_num;
            }
            fprintf(fp, "\tbeqz t%d, for_Exit_%d\n", relopExpr->place, exitLabelIndex);
        }
        else{
            if(relopExpr->dataType == INT_TYPE)
                free_t_reg(relopExpr->place);
            else if(relopExpr->dataType == FLOAT_TYPE)
                free_ft_reg(relopExpr->place);
        }
        relopExpr = relopExpr->rightSibling;
    }
    genStmtNode(stmtNode);
    fprintf(fp,"L%d:\n", incrementLabelIndex);
    while(updateAssignExpr){
        if(updateAssignExpr->nodeType == STMT_NODE && updateAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(updateAssignExpr);
        else{
            genExprNode(updateAssignExpr);
        }
        updateAssignExpr = updateAssignExpr->rightSibling;
    }
    fprintf(fp, "\tj L%d\n", testLabelIndex);
    fprintf(fp, "for_Exit_%d:\n", exitLabelIndex);

    return;
}


void genAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE *leftNode = assignmentNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    SymbolTableEntry *leftEntry = leftNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    int offset = leftEntry->offset;
    int t_reg_num = get_t_reg();
    genExprNode(rightNode);
    // implicit conversion: Hw6
    if(leftEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
        // both local and global case are handled in genArrayElement
        // genArrayElement puts address of the element in t_reg_num
        genArrayElement(leftNode, t_reg_num);
        if(leftNode->dataType == INT_TYPE)
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        else
            fprintf(fp, "\tfsw ft%d, 0(t%d)\n", rightNode->place, t_reg_num);
    }
    else if(isGlobal(leftEntry)){
        // global variable
        fprintf(fp, "\tla t%d, _%s\n", t_reg_num, leftNode->semantic_value.identifierSemanticValue.identifierName);
        if(leftNode->dataType == INT_TYPE)
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        else
            fprintf(fp, "\tfsw ft%d, 0(t%d)\n", rightNode->place, t_reg_num);
    }
    else{
        // local variable
        // load offset to a register, calculate fp - offset
        loadConst(offset, t_reg_num);
        fprintf(fp, "\tsub t%d, fp, t%d\n", t_reg_num, t_reg_num);
        if(leftNode->dataType == INT_TYPE)
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        else
            fprintf(fp, "\tfsw ft%d, 0(t%d)\n", rightNode->place, t_reg_num);
    }
    free_t_reg(t_reg_num);
    if(rightNode->dataType == INT_TYPE)
        free_t_reg(rightNode->place);
    else
        free_ft_reg(rightNode->place);
    fflush(fp);
    return;
}


void genIfStmt(AST_NODE* ifNode)
{
    AST_NODE *ifTest = ifNode->child;
    AST_NODE *stmtNode = ifTest->rightSibling;
    int successIndex = labelIndex++;
    int elseJumpIndex = labelIndex++;
    int elseIndex = labelIndex++;
    int exitIndex = ifExitLabelIndex++;
    int else_reg_num;
    genExprNode(ifTest);
    // case: if(a + 1.1), where a is a float
    if(ifTest->dataType == FLOAT_TYPE){
        int t_reg_num = get_t_reg();
        fprintf(fp, "\tfmv.x.w t%d, ft%d\n", t_reg_num, ifTest->place);
        free_ft_reg(ifTest->place);
        ifTest->place = t_reg_num;
    }
    fprintf(fp, "\tbeqz t%d, L%d\n", ifTest->place, elseJumpIndex);
    fprintf(fp, "\tj L%d\n", successIndex);

    else_reg_num = get_t_reg();
    fprintf(fp, "L%d:\n", elseJumpIndex);
    fprintf(fp, "\tla t%d, L%d\n", else_reg_num, elseIndex);
    fprintf(fp, "\tjalr x0, 0(t%d)\n", else_reg_num);
    
    free_t_reg(ifTest->place);
    free_t_reg(else_reg_num);
    fprintf(fp, "L%d:\n", successIndex);
    // gen stmt for successful ifTest
    genStmtNode(stmtNode);

    int t_reg_num = get_t_reg();
    fprintf(fp, "\tla t%d, ifExit_%d\n",t_reg_num, exitIndex);
    fprintf(fp, "\tjalr x0, 0(t%d)\n", t_reg_num);
    free_t_reg(t_reg_num);
    // gen stmt for else
    fprintf(fp, "L%d:\n", elseIndex);
    genStmtNode(stmtNode->rightSibling);
    fprintf(fp, "ifExit_%d:\n", exitIndex);
    fflush(fp);
}

void genWriteFunction(AST_NODE* functionCallNode)
{
    // check if there is one argument
    AST_NODE *toWrite = functionCallNode->child->rightSibling->child;
    genExprNode(toWrite);
    int t_reg_num = get_t_reg();
    // write a constant string
    if(toWrite->dataType == CONST_STRING_TYPE){
        fprintf(fp, ".text\n");
        fprintf(fp, "\tlui a5, %%hi(_CONSTANT_%d)\n", toWrite->place);
        fprintf(fp, "\taddi a0, a5, %%lo(_CONSTANT_%d)\n", toWrite->place);
        // fprintf(fp, "\tla t%d, _CONSTANT_%d\n", t_reg_num, toWrite->place);
        // fprintf(fp, "\tmv a0, t%d\n", t_reg_num);
	fprintf(fp, "\tla t%d, _write_str\n", t_reg_num);
        fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
	// fprintf(fp, "\tjal _write_str\n");
        free_t_reg(toWrite->place);
    }
    else if(toWrite->dataType == INT_TYPE){
        fprintf(fp, "\tmv a0, t%d\n", toWrite->place);
	fprintf(fp, "\tla t%d, _write_int\n", t_reg_num);
        fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
	// fprintf(fp, "\tjal _write_int\n");
        free_t_reg(toWrite->place);
    }
    else{
        fprintf(fp, "\tfmv.s fa0, ft%d\n", toWrite->place);
	fprintf(fp, "\tla t%d, _write_float\n", t_reg_num);
        fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
	// fprintf(fp, "\tjal _write_float\n");
        free_ft_reg(toWrite->place);
        // write float
    }
    free_t_reg(t_reg_num);
    fflush(fp);
    return;
}

void genFunctionCall(AST_NODE* functionCallNode)
{
    AST_NODE *idNode = functionCallNode->child;
    AST_NODE *paramNode = idNode->rightSibling->child;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    // special case: write function
    if(strcmp(idName, "write") == 0){
        genWriteFunction(functionCallNode);
        return;
    }
    else if(strcmp(idName, "read") == 0){
	int t_reg_num = get_t_reg();
	fprintf(fp, "\tla t%d, _read_int\n", t_reg_num);
	fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
	free_t_reg(t_reg_num);
        return;
    }
    else if(strcmp(idName, "fread") == 0){
	int t_reg_num = get_t_reg();
	fprintf(fp, "\tla t%d, _read_float\n", t_reg_num);
	fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
	free_t_reg(t_reg_num);
        return;
    }

    SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    int t_reg_num = get_t_reg();
    // genParameterPassing(idEntry->attribute->attr.functionSignature->parameterList, paramNode, idNode);
    fprintf(fp, "\tla t%d, _start_%s\n", t_reg_num, idName);
    fprintf(fp, "\tjalr ra, 0(t%d)\n", t_reg_num);
    free_t_reg(t_reg_num);
    fflush(fp);
}

/*
void getFormalParameterType(Parameter *formalParameter, char *formalParameterType){
    TypeDescriptor *typeDescriptor = formalParameter->type;
    if(typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
        if(typeDescriptor->properties.dataType == INT_TYPE)
            strcpy(formalParameterType, "int");
        else
            strcpy(formalParameterType, "float");
    }
    else{
        if(typeDescriptor->properties.arrayProperties.elementType == INT_TYPE)
            strcpy(formalParameterType, "int (*)");
        else if(typeDescriptor->properties.arrayProperties.elementType == FLOAT_TYPE)
            strcpy(formalParameterType, "float (*)");
        else
            strcpy(formalParameterType, "char (*)");
        for(int i = 1; i < typeDescriptor->properties.arrayProperties.dimension; i++){
            char dim[MSG_LEN];
            sprintf(dim, "[%d]", typeDescriptor->properties.arrayProperties.sizeInEachDimension[i]);
            strcat(formalParameterType, dim);
        }
    }
}

void getActualParameterType(AST_NODE *actualParameter, char *actualParameterType){
    DATA_TYPE dataType = actualParameter->dataType;
    if(dataType == ERROR_TYPE)
        strcpy(actualParameterType, "");
    else if(dataType == INT_TYPE)
        strcpy(actualParameterType, "int");
    else if(dataType == FLOAT_TYPE)
        strcpy(actualParameterType, "float");
    else if(dataType == VOID_TYPE)
        strcpy(actualParameterType, "void");
    else if(dataType == CONST_STRING_TYPE)
        strcpy(actualParameterType, "char (*)");
    else{
        if(dataType == INT_PTR_TYPE)
            strcpy(actualParameterType, "int (*)");
        else
            strcpy(actualParameterType, "float (*)");
        SymbolTableEntry *idEntry = retrieveSymbol(actualParameter->semantic_value.identifierSemanticValue.identifierName);
        TypeDescriptor *typeDescriptor = idEntry->attribute->attr.typeDescriptor;
        int dimCount = 0;
        AST_NODE *dimReference = actualParameter->child;
        while(dimReference != NULL){
            dimReference = dimReference->rightSibling;
            dimCount += 1;
        }
        dimCount += 1;
        for(int i = dimCount; i < typeDescriptor->properties.arrayProperties.dimension; i++){
            char dim[MSG_LEN];
            sprintf(dim, "[%d]", typeDescriptor->properties.arrayProperties.sizeInEachDimension[i]);
            strcat(actualParameterType, dim);
        }
    }
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter, AST_NODE *idNode)
{
    while(formalParameter != NULL && actualParameter != NULL){
        char formalParameterType[MSG_LEN], actualParameterType[MSG_LEN];
        genExprNode(actualParameter);
        getFormalParameterType(formalParameter, formalParameterType);
        getActualParameterType(actualParameter, actualParameterType);
        if((strcmp(formalParameterType, "int") != 0 && strcmp(formalParameterType, "float") != 0) || \
            (strcmp(actualParameterType, "int") != 0 && strcmp(actualParameterType, "float") != 0)){
                if(actualParameter->dataType != ERROR_TYPE && strcmp(formalParameterType, actualParameterType) != 0){
                    char errMsg[ERR_MSG_LEN];
                    sprintf(errMsg, "\'%s\' to \'%s\'", actualParameterType, formalParameterType);
                    printErrorMsgSpecial(idNode, errMsg, PARAMETER_TYPE_UNMATCH);
                }
        }
        formalParameter = formalParameter->next;
        actualParameter = actualParameter->rightSibling;
    }
    if(formalParameter != NULL){
        // error : too few arguments
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s\'", idNode->semantic_value.identifierSemanticValue.identifierName);
        printErrorMsgSpecial(idNode, errMsg, TOO_FEW_ARGUMENTS);
    }
    if(actualParameter != NULL){
        // error ; too many arguments
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s\'", idNode->semantic_value.identifierSemanticValue.identifierName);
        printErrorMsgSpecial(idNode, errMsg, TOO_MANY_ARGUMENTS);
    }
}
*/

void genExprNode(AST_NODE* exprNode)
{
    if(exprNode->nodeType == NUL_NODE) return;
    // constant node
    if(exprNode->nodeType == CONST_VALUE_NODE){
        if(exprNode->dataType == INT_TYPE){
            int t_reg_num = get_t_reg();
            int constVal = exprNode->semantic_value.const1->const_u.intval;
            loadConst(constVal, t_reg_num);
            exprNode->place = t_reg_num;
        }
        else if(exprNode->dataType == FLOAT_TYPE){
            // float const
            int ft_reg_num = get_ft_reg();
            float constVal = exprNode->semantic_value.const1->const_u.fval;
            loadFloat(constVal, ft_reg_num);
            exprNode->place = ft_reg_num;
        }
        else{
            char *string = exprNode->semantic_value.const1->const_u.sc;
            char buf[1024] = "";
            int len = strlen(string);
            strncpy(buf, string, len - 1);
            fprintf(fp, ".data\n");
            fprintf(fp, "\t_CONSTANT_%d: .string \"%s\\000\"\n", constLabelIndex, &buf[1]);
            //fprintf(fp, "\t_CONSTANT_%d: .string %s\n", constLabelIndex, string);
            // fprintf(fp, "\t.align 3\n");
            exprNode->place = constLabelIndex;
            constLabelIndex++;
        }
	fflush(fp);
        return;
    }
    // function call node
    else if(exprNode->nodeType == STMT_NODE && \
            exprNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
        genFunctionCall(exprNode);
        if(exprNode->dataType == INT_TYPE){
            int t_reg_num = get_t_reg();
            fprintf(fp, "\tmv t%d, a0\n", t_reg_num);
            exprNode->place = t_reg_num;
        }
        else{
            // float function
            int ft_reg_num = get_ft_reg();
            fprintf(fp, "\tfmv.s ft%d, fa0\n", ft_reg_num);
            exprNode->place = ft_reg_num;
        }
	fflush(fp);
        return;
    }
    // identifier node
    else if(exprNode->nodeType == IDENTIFIER_NODE){
        char *identifierName = exprNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *identifier = exprNode->semantic_value.identifierSemanticValue.symbolTableEntry;
        // array element
        if(identifier->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            int t_reg_num = get_t_reg();
            genArrayElement(exprNode, t_reg_num);
            if(exprNode->dataType == INT_TYPE){
                fprintf(fp, "\tlw t%d, 0(t%d)\n", t_reg_num, t_reg_num);
                exprNode->place = t_reg_num;
            }
            else{
                // float
                int ft_reg_num = get_ft_reg();
                fprintf(fp, "\tflw ft%d, 0(t%d)\n", ft_reg_num, t_reg_num);
                exprNode->place = ft_reg_num;
                free_t_reg(t_reg_num);
            }
        }
        // identifier is a global variable
        else if(isGlobal(identifier)){
            int addr_reg_num = get_t_reg();
            fprintf(fp, "\tla t%d, _%s\n", addr_reg_num, identifierName);
            if(exprNode->dataType == INT_TYPE){
                int t_reg_num = get_t_reg();
                fprintf(fp, "\tlw t%d, 0(t%d)\n", t_reg_num, addr_reg_num);
                exprNode->place = t_reg_num;
            }
            else{
                int ft_reg_num = get_ft_reg();
                fprintf(fp, "\tflw ft%d, 0(t%d)\n", ft_reg_num, addr_reg_num);
                exprNode->place = ft_reg_num;
            }
            free_t_reg(addr_reg_num);
        }
        // identifier is a local int
        else if(exprNode->dataType == INT_TYPE){
            int t_reg_num = get_t_reg();
	    if(identifier->offset >= 2048){
		loadConst(identifier->offset, t_reg_num);
		fprintf(fp, "\tsub t%d, fp, t%d\n", t_reg_num, t_reg_num);
		fprintf(fp, "\tlw t%d, 0(t%d)\n", t_reg_num, t_reg_num);
	    }
	    else
            	fprintf(fp, "\tlw t%d, -%d(fp)\n", t_reg_num, identifier->offset);
            exprNode->place = t_reg_num;
        }
        // identifier is a local float
        else{
            int ft_reg_num = get_ft_reg();
	    if(identifier->offset >= 2048){
		int t_reg_num = get_t_reg();
		loadConst(identifier->offset, t_reg_num);
		fprintf(fp, "\tsub t%d, fp, t%d\n", t_reg_num, t_reg_num);
		fprintf(fp, "\tflw ft%d, 0(t%d)\n", ft_reg_num, t_reg_num);
		free_t_reg(t_reg_num);
	    }
	    else
            	fprintf(fp, "\tflw ft%d, -%d(fp)\n", ft_reg_num, identifier->offset);
            exprNode->place = ft_reg_num;
        }
	fflush(fp);
        return;
    }
    // nonterminal nodes
    AST_NODE *leftNode = exprNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;

    // unary operation
    if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION){
        genExprNode(leftNode);
        switch(exprNode->semantic_value.exprSemanticValue.op.unaryOp){
            case UNARY_OP_POSITIVE:
                exprNode->place = leftNode->place;
                break;
            case UNARY_OP_NEGATIVE:
                if(exprNode->dataType == INT_TYPE){
                    fprintf(fp, "\tsub t%d, x0, t%d\n", leftNode->place, leftNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    // float
                    int ft_reg_num = get_ft_reg();
                    fprintf(fp, "\tfmv.w.x ft%d, x0\n", ft_reg_num);
                    fprintf(fp, "\tfsub.s ft%d, ft%d, ft%d\n", leftNode->place, ft_reg_num, leftNode->place);
                    free_ft_reg(ft_reg_num);
                    exprNode->place = leftNode->place;
                }
                break;
            case UNARY_OP_LOGICAL_NEGATION:
                if(exprNode->dataType == INT_TYPE){
                    fprintf(fp, "\tsltiu t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    // float
                    int t_reg_num = get_t_reg();
                    fprintf(fp, "\tfmv.x.w t%d, ft%d\n", t_reg_num, leftNode->place);
                    fprintf(fp, "\tsltiu t%d, t%d, 1\n", t_reg_num, t_reg_num);
                    free_ft_reg(leftNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
        }
	fflush(fp);
    }
    else{
        // for short-circuit
        int t_reg_num, ft_reg_num;
        DATA_TYPE dataType;
        if(exprNode->semantic_value.exprSemanticValue.op.binaryOp != BINARY_OP_AND && \
           exprNode->semantic_value.exprSemanticValue.op.binaryOp != BINARY_OP_OR){
            genExprNode(leftNode);
            genExprNode(rightNode);
            if(leftNode->dataType == INT_TYPE && rightNode->dataType == INT_TYPE)
                dataType = INT_TYPE;
            else if(leftNode->dataType == INT_TYPE){
                // convert leftNode to float
                dataType = FLOAT_TYPE;
            }
            else if(rightNode->dataType == INT_TYPE){
                // convert rightNode to float
                dataType = FLOAT_TYPE;
            }
            else
                dataType = FLOAT_TYPE;
        }
        // put result in register of left operand
        // free register of right operand
        // set place of this node to register of left operand
        int failLabelIndex, successLabelIndex, nextLabelIndex, zeroLoaded;
        switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
            case BINARY_OP_ADD:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tadd t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    fprintf(fp, "\tfadd.s ft%d, ft%d, ft%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                break;
            case BINARY_OP_SUB:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    fprintf(fp, "\tfsub.s ft%d, ft%d, ft%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                break;
            case BINARY_OP_MUL:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tmul t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    fprintf(fp, "\tfmul.s ft%d, ft%d, ft%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                break;
            case BINARY_OP_DIV:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tdiv t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    fprintf(fp, "\tfdiv.s ft%d, ft%d, ft%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                break;
            case BINARY_OP_EQ:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tsltiu t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, rightNode->place);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_GE:
                if(dataType == INT_TYPE){
                    // a >= b -> a - b >= 0 -> -1 < a - b
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    loadConst(-1, t_reg_num);
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, t_reg_num, leftNode->place);
                    free_t_reg(t_reg_num);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    // a >= b -> b <= a
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tfle.s t%d, ft%d, ft%d\n", t_reg_num, rightNode->place, leftNode->place);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_LE:
                if(dataType == INT_TYPE){
                    // a <= b -> a - b <= 0 -> a - b < 1
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tslti t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tfle.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, rightNode->place);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_NE:
                if(dataType == INT_TYPE){
                    // a != b -> a - b != 0
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tsltu t%d, x0, t%d\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    t_reg_num = get_t_reg();
                    /*
                    fprintf(fp, "\tfsub.s ft%d, ft%d, ft%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tfmv.x.w t%d, ft%d\n", t_reg_num, leftNode->place);
                    fprintf(fp, "\tsnez t%d, t%d\n", t_reg_num, t_reg_num);
                    */
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, rightNode->place);
                    fprintf(fp, "\tseqz t%d, t%d\n", t_reg_num, t_reg_num);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_GT:
                if(dataType == INT_TYPE){
                    // a > b -> b < a
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, rightNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    // a > b -> b < a
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tflt.s t%d, ft%d, ft%d\n", t_reg_num, rightNode->place, leftNode->place);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_LT:
                if(dataType == INT_TYPE){
                    // a < b
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tflt.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, rightNode->place);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(rightNode->place);
                    exprNode->place = t_reg_num;
                }
                break;
            case BINARY_OP_AND:
                failLabelIndex = labelIndex++;
                nextLabelIndex = labelIndex++;
                genExprNode(leftNode);
                if(leftNode->dataType == INT_TYPE)
                    fprintf(fp, "\tbeqz t%d, L%d\n", leftNode->place, failLabelIndex);
                else{
                    t_reg_num = get_t_reg();
                    ft_reg_num = get_ft_reg();
                    fprintf(fp, "\tfmv.w.x ft%d, x0\n", ft_reg_num);
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, ft_reg_num);
                    fprintf(fp, "\tbnez t%d, L%d\n", t_reg_num, failLabelIndex);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(ft_reg_num);
                    leftNode->place = t_reg_num;
                }
                genExprNode(rightNode);
                if(rightNode->dataType == INT_TYPE){
                    fprintf(fp, "\tbeqz t%d, L%d\n", rightNode->place, failLabelIndex);
                    free_t_reg(rightNode->place);
                }
                else{
                    t_reg_num = get_t_reg();
                    ft_reg_num = get_ft_reg();
                    fprintf(fp, "\tfmv.w.x ft%d, x0\n", ft_reg_num);
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, rightNode->place, ft_reg_num);
                    fprintf(fp, "\tbnez t%d, L%d\n", t_reg_num, failLabelIndex);
                    free_ft_reg(rightNode->place);
                    free_ft_reg(ft_reg_num);
                    free_t_reg(t_reg_num);
                }
                loadConst(1, leftNode->place);
		t_reg_num = get_t_reg();
		fprintf(fp, "\tla t%d ,L%d\n", t_reg_num, nextLabelIndex);
		fprintf(fp, "\tjalr x0, 0(t%d)\n", t_reg_num);
		free_t_reg(t_reg_num);
                fprintf(fp, "L%d:\n", failLabelIndex);
                loadConst(0, leftNode->place);
                fprintf(fp, "L%d:\n", nextLabelIndex);
                exprNode->place = leftNode->place;
                break;
            case BINARY_OP_OR:
                successLabelIndex = labelIndex++;
                nextLabelIndex = labelIndex++;
                genExprNode(leftNode);
                if(leftNode->dataType == INT_TYPE)
                    fprintf(fp, "\tbnez t%d, L%d\n", leftNode->place, successLabelIndex);
                else{
                    t_reg_num = get_t_reg();
                    ft_reg_num = get_ft_reg();
                    fprintf(fp, "\tfmv.w.x ft%d, x0\n", ft_reg_num);
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, leftNode->place, ft_reg_num);
                    fprintf(fp, "\tbeqz t%d, L%d\n", t_reg_num, successLabelIndex);
                    free_ft_reg(leftNode->place);
                    free_ft_reg(ft_reg_num);
                    leftNode->place = t_reg_num;
                }
                genExprNode(rightNode);
                if(rightNode->dataType == INT_TYPE){
                    fprintf(fp, "\tbnez t%d, L%d\n", rightNode->place, successLabelIndex);
                    free_t_reg(rightNode->place);
                }
                else{
                    t_reg_num = get_t_reg();
                    ft_reg_num = get_ft_reg();
                    fprintf(fp, "\tfmv.w.x ft%d, x0\n", ft_reg_num);
                    fprintf(fp, "\tfeq.s t%d, ft%d, ft%d\n", t_reg_num, rightNode->place, ft_reg_num);
                    fprintf(fp, "\tbeqz t%d, L%d\n", t_reg_num, successLabelIndex);
                    free_ft_reg(rightNode->place);
                    free_ft_reg(ft_reg_num);
                    free_t_reg(t_reg_num);
                }
                loadConst(0, leftNode->place);
		t_reg_num = get_t_reg();
		fprintf(fp, "\tla t%d, L%d\n", t_reg_num, nextLabelIndex);
		fprintf(fp, "\tjalr x0, 0(t%d)\n", t_reg_num);
		free_t_reg(t_reg_num);
                fprintf(fp, "L%d:\n", successLabelIndex);
                loadConst(1, leftNode->place);
                fprintf(fp, "L%d:\n", nextLabelIndex);
                exprNode->place = leftNode->place;
                break;
        }
	fflush(fp);
    }
    return;
}

void genArrayElement(AST_NODE *idNode, int offset_reg){
    /* this function put address of desired array element in offset_reg */
    AST_NODE *arrayDimension = idNode->child;
    SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    ArrayProperties property = idEntry->attribute->attr.typeDescriptor->properties.arrayProperties;
    int nowDimension = 0;
    if(isGlobal(idEntry)){
        fprintf(fp, "\tla t%d, _%s\n", offset_reg, idNode->semantic_value.identifierSemanticValue.identifierName);
        while(arrayDimension){
            int offsetPerShift = 1;
            int t_reg_num = get_t_reg();
            genExprNode(arrayDimension);
            for(int i = nowDimension + 1; i < property.dimension; i++)
                offsetPerShift *= property.sizeInEachDimension[i];
            if(offsetPerShift > 1){
                loadConst(offsetPerShift, t_reg_num);
                fprintf(fp, "\tmul t%d, t%d, t%d\n", arrayDimension->place, arrayDimension->place, t_reg_num);
            }
            fprintf(fp, "\tslli t%d, t%d, 2\n", arrayDimension->place, arrayDimension->place);
            fprintf(fp, "\tadd t%d, t%d, t%d\n", offset_reg, offset_reg, arrayDimension->place);
	    fflush(fp);
            free_t_reg(arrayDimension->place);
            free_t_reg(t_reg_num);
            nowDimension++; 
            arrayDimension = arrayDimension->rightSibling;
        }
    }
    else{
        int arrayStartOffset = idEntry->offset;
        loadConst(arrayStartOffset, offset_reg);
        while(arrayDimension){
            int offsetPerShift = 1;
            int t_reg_num = get_t_reg();
            genExprNode(arrayDimension);
            for(int i = nowDimension + 1; i < property.dimension; i++)
                offsetPerShift *= property.sizeInEachDimension[i];
            if(offsetPerShift > 1){
                loadConst(offsetPerShift, t_reg_num);
                fprintf(fp, "\tmul t%d, t%d, t%d\n", arrayDimension->place, arrayDimension->place, t_reg_num);
            }
            fprintf(fp, "\tslli t%d, t%d, 2\n", arrayDimension->place, arrayDimension->place);
            fprintf(fp, "\tsub t%d, t%d, t%d\n", offset_reg, offset_reg, arrayDimension->place);
	    fflush(fp);
            free_t_reg(arrayDimension->place);
            free_t_reg(t_reg_num);
            nowDimension++; 
            arrayDimension = arrayDimension->rightSibling;
        }
        fprintf(fp, "\tsub t%d, fp, t%d\n", offset_reg, offset_reg);
	fflush(fp);
    }
}

void genReturnStmt(AST_NODE* returnNode)
{
    AST_NODE *returnItem = returnNode->child;
    AST_NODE *parentNode = returnNode->parent;
    while((parentNode->nodeType != DECLARATION_NODE) || (parentNode->semantic_value.declSemanticValue.kind != FUNCTION_DECL)){
        parentNode = parentNode->parent;
    }
    AST_NODE *typeNode = parentNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    char *functionName = idNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *entry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    genExprNode(returnItem);
    if(entry->attribute->attr.functionSignature->returnType == INT_TYPE){
        // convert float to int
        fprintf(fp, "\tmv a0, t%d\n", returnItem->place);
        free_t_reg(returnItem->place);
    }
    else{
        // convert int to float
        fprintf(fp, "\tfmv.s fa0, ft%d\n", returnItem->place);
        free_ft_reg(returnItem->place);
    }
    fprintf(fp, "\tj _end_%s\n", functionName);
    fflush(fp);
}


void genBlockNode(AST_NODE* blockNode)
{
    if(blockNode->child == NULL) return;
    AST_NODE *declList, *stmtList;
    if(blockNode->child->nodeType == VARIABLE_DECL_LIST_NODE){
        declList = blockNode->child;
        stmtList = declList->rightSibling;
    }
    else{
        declList = NULL;
        stmtList = blockNode->child;
    }
    if(declList != NULL){
        AST_NODE *decl = declList->child;
        while(decl != NULL){
            genDeclarationNode(decl);
            decl = decl->rightSibling;
        }
    }
    if(stmtList != NULL){
        AST_NODE *stmt = stmtList->child;
        while(stmt != NULL){
            genStmtNode(stmt);
            stmt = stmt->rightSibling;
        }
    }
}

void genStmtNode(AST_NODE* stmtNode)
{
    if(stmtNode == NULL) return;
    switch(stmtNode->nodeType){
        case BLOCK_NODE:
            genBlockNode(stmtNode);
            break;
        case NUL_NODE:
            break;
        default:
            switch(stmtNode->semantic_value.stmtSemanticValue.kind){
                case IF_STMT:
                    genIfStmt(stmtNode);
                    break;
                case WHILE_STMT:
                    genWhileStmt(stmtNode);
                    break;
                case FOR_STMT:
                    genForStmt(stmtNode);
                    break;
                case RETURN_STMT:
                    genReturnStmt(stmtNode);
                    break;
                case FUNCTION_CALL_STMT:
                    genFunctionCall(stmtNode);
                    break;
                case ASSIGN_STMT:
                    genAssignmentStmt(stmtNode);
                    break;
            }
            break;
    }

}

