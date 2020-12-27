#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int ARoffset = 4;
int constLabelIndex = 1;
int labelIndex = 1;
short t_reg_status[7];

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
    fprintf(fp, "\t_frameSize_%s: .word %d\n", functionName, 180 + ARoffset - 4);
    ARoffset = 4;
    return;
}

void loadConst(int constVal, int reg_num){
    if(constVal < 2048 && constVal >= -2048)
        fprintf(fp, "\taddi t%d, x0, %d\n", reg_num, constVal);
    else{
        int upper = constVal >> 12;
        int lower = constVal & 0x00000FFF;
        fprintf(fp, "\tlui t%d, %d\n", reg_num, upper);
        fprintf(fp, "\tori t%d, t%d, %d\n", reg_num, reg_num, lower);
    }
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
                        fprintf(fp, "\t_%s: .word\n", idName);
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
                            fprintf(fp, "\t_%s: .word %d\n", idName, idNode->semantic_value.const1->const_u.intval);
                        else
                            fprintf(fp, "\t_%s: .word %d\n", idName, idNode->semantic_value.const1->const_u.fval);
                    }
                    else{
                        if(idNode->dataType == INT_TYPE){
                            idEntry->offset = ARoffset;
                            int t_reg_num = get_t_reg();
                            int constVal = idNode->semantic_value.const1->const_u.intval;
                            loadConst(constVal, t_reg_num);
                            fprintf(fp, "\tsw t%d, -%d(fp)\n", t_reg_num, ARoffset);
                            free_t_reg(t_reg_num);
                            ARoffset += 4;
                        }
                        else{
                            // floating number
                        }
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
    int failLabelIndex = labelIndex++;
    // unconditional jump to check
    fprintf(fp, "\tj L%d\n", failLabelIndex);
    fprintf(fp, "L%d:\n", successLabelIndex);
    // gen block if test succeed
    genStmtNode(stmtNode);
    // check test
    fprintf(fp, "L%d:\n", failLabelIndex);
    genExprNode(testExprRoot);
    fprintf(fp, "\tbgtz t%d, L%d\n", testExprRoot->place, successLabelIndex);
    free_t_reg(testExprRoot->place);
}

/*
void genForStmt(AST_NODE* forNode)
{
    AST_NODE *initAssignExprRoot = forNode->child;
    AST_NODE *initAssignExpr = initAssignExprRoot->child;
    AST_NODE *relopExprRoot = initAssignExprRoot->rightSibling;
    AST_NODE *relopExpr = relopExprRoot->child;
    AST_NODE *updateAssignExprRoot = relopExprRoot->rightSibling;
    AST_NODE *updateAssignExpr = updateAssignExprRoot->child;
    AST_NODE *stmtNode = updateAssignExprRoot->rightSibling;
    while(initAssignExpr){
        if(initAssignExpr->nodeType == STMT_NODE && initAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(initAssignExpr);
        else{
            genExprNode(initAssignExpr);
        }
        initAssignExpr = initAssignExpr->rightSibling;
    }
    while(relopExpr){
        genExprNode(relopExpr);
        relopExpr = relopExpr->rightSibling;
    }
    while(updateAssignExpr){
        if(updateAssignExpr->nodeType == STMT_NODE && updateAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(updateAssignExpr);
        else{
            genExprNode(updateAssignExpr);
        }
        updateAssignExpr = updateAssignExpr->rightSibling;
    }
    genStmtNode(stmtNode);
    return;
}
*/

void genAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE *leftNode = assignmentNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    SymbolTableEntry *leftEntry = leftNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    int offset = leftEntry->offset;
    genExprNode(rightNode);
    if(leftNode->dataType == INT_TYPE){
        int t_reg_num = get_t_reg();
        if(leftEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            genArrayElement(leftNode, t_reg_num);
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        }
        else if(isGlobal(leftEntry)){
            fprintf(fp, "\tla t%d, _%s\n", t_reg_num, leftNode->semantic_value.identifierSemanticValue.identifierName);
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        }
        else{
            loadConst(offset, t_reg_num);
            fprintf(fp, "\tsub t%d, fp, t%d\n", t_reg_num, t_reg_num);
            fprintf(fp, "\tsw t%d, 0(t%d)\n", rightNode->place, t_reg_num);
        }
        free_t_reg(t_reg_num);
    }
    else{
        // float assignment
    }
    free_t_reg(rightNode->place);
    return;
}


void genIfStmt(AST_NODE* ifNode)
{
    AST_NODE *ifTest = ifNode->child;
    AST_NODE *stmtNode = ifTest->rightSibling;
    int elseIndex = labelIndex++;
    genExprNode(ifTest);
    fprintf(fp, "\tbnez t%d, L%d\n", ifTest->place, elseIndex);
    free_t_reg(ifTest->place);
    // gen stmt for successful ifTest
    genStmtNode(stmtNode);
    // gen stmt for else
    fprintf(fp, "L%d:\n", elseIndex);
    genStmtNode(stmtNode->rightSibling);
}

void genWriteFunction(AST_NODE* functionCallNode)
{
    // check if there is one argument
    AST_NODE *toWrite = functionCallNode->child->rightSibling->child;
    genExprNode(toWrite);
    // write a constant string
    if(toWrite->nodeType == CONST_VALUE_NODE){
        fprintf(fp, ".text\n");
        int t_reg_num = get_t_reg();
        fprintf(fp, "\tlui a5, %%hi(_CONSTANT_%d)\n", toWrite->place);
        fprintf(fp, "\taddi a0, a5, %%lo(_CONSTANT_%d)\n", toWrite->place);
        // fprintf(fp, "\tla t%d, _CONSTANT_%d\n", t_reg_num, toWrite->place);
        // fprintf(fp, "\tmv a0, t%d\n", t_reg_num);
        fprintf(fp, "\tjal _write_str\n");
        free_t_reg(t_reg_num);
    }
    else if(toWrite->dataType == INT_TYPE){
        int t_reg_num = get_t_reg();
        fprintf(fp, "\tmv a0, t%d\n", toWrite->place);
        fprintf(fp, "\tjal _write_int\n");
        free_t_reg(toWrite->place);
        free_t_reg(t_reg_num);
    }
    else{
        // write float
    }
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
        fprintf(fp, "\tjal _read_int\n");
        return;
    }
    else if(strcmp(idName, "fread") == 0){
        fprintf(fp, "\tjal _read_float\n");
        return;
    }

    SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    // genParameterPassing(idEntry->attribute->attr.functionSignature->parameterList, paramNode, idNode);
    fprintf(fp, "\tjal _start_%s\n", idName);
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
        }
        else{
            char *string = exprNode->semantic_value.const1->const_u.sc;
            char buf[1024];
            int len = strlen(string);
            strncpy(buf, string, len - 1);
            fprintf(fp, ".data\n");
            fprintf(fp, "\t_CONSTANT_%d: .ascii \"%s\\000\"\n", constLabelIndex, &buf[1]);
            fprintf(fp, "\t.align 3\n");
            exprNode->place = constLabelIndex;
            constLabelIndex++;
        }
        return;
    }
    // function call node
    else if(exprNode->nodeType == STMT_NODE && \
            exprNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
        genFunctionCall(exprNode);
        int t_reg_num = get_t_reg();
        if(exprNode->dataType == INT_TYPE){
            fprintf(fp, "\tmv t%d, a0\n", t_reg_num);
            exprNode->place = t_reg_num;
        }
        else{
            // float function
        }
        return;
    }
    // identifier node
    else if(exprNode->nodeType == IDENTIFIER_NODE){
        char *identifierName = exprNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *identifier = exprNode->semantic_value.identifierSemanticValue.symbolTableEntry;
        // array element
        if(identifier->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            int t_reg_num = get_t_reg();
            if(exprNode->dataType == INT_TYPE){
                genArrayElement(exprNode, t_reg_num);
                fprintf(fp, "\tlw t%d, 0(t%d)\n", t_reg_num, t_reg_num);
                exprNode->place = t_reg_num;
            }
            else{
                // float
            }
        }
        // identifier is a scalar
        else{
            exprNode->dataType = identifier->attribute->attr.typeDescriptor->properties.dataType;
            int t_reg_num = get_t_reg();
            // global variable
            if(isGlobal(identifier)){
                if(exprNode->dataType == INT_TYPE){
                    int addr_reg_num = get_t_reg();
                    fprintf(fp, "\tla t%d, _%s\n", addr_reg_num, identifierName);
                    fprintf(fp, "\tlw t%d, 0(t%d)\n", t_reg_num, addr_reg_num);
                    exprNode->place = t_reg_num;
                    free_t_reg(addr_reg_num);
                }
                else{
                    // global float
                }
            }
            else if(exprNode->dataType == INT_TYPE){
                fprintf(fp, "\tlw t%d, -%d(fp)\n", t_reg_num, identifier->offset);
                exprNode->place = t_reg_num;
            }
            else{
                // local float
            }
        }
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
                }
                break;
            case UNARY_OP_LOGICAL_NEGATION:
                if(exprNode->dataType == INT_TYPE){
                    fprintf(fp, "\tsltiu t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    exprNode->place = leftNode->place;
                }
                else{
                    // float
                }
                break;
        }
    }
    else{
        genExprNode(leftNode);
        genExprNode(rightNode);
        int t_reg_num;
        DATA_TYPE dataType;
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
        // put result in register of left operand
        // free register of right operand
        // set place of this node to register of left operand
        switch(exprNode->semantic_value.exprSemanticValue.op.binaryOp){
            case BINARY_OP_ADD:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tadd t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_SUB:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_MUL:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tmul t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_DIV:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tdiv t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_EQ:
                if(dataType == INT_TYPE){
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tsltiu t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_GE:
                // a >= b -> a - b >= 0 -> -1 < a - b
                if(dataType == INT_TYPE){
                    t_reg_num = get_t_reg();
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    loadConst(-1, t_reg_num);
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, t_reg_num, leftNode->place);
                    free_t_reg(t_reg_num);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_LE:
                if(dataType == INT_TYPE){
                    // a <= b -> a - b <= 0 -> a - b < 1
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tslti t%d, t%d, 1\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_NE:
                if(dataType == INT_TYPE){
                    // a != b -> a - b != 0
                    fprintf(fp, "\tsub t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    fprintf(fp, "\tsltu t%d, x0, t%d\n", leftNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_GT:
                if(dataType == INT_TYPE){
                    // a > b -> b < a
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, rightNode->place, leftNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_LT:
                if(dataType == INT_TYPE){
                    // a < b
                    fprintf(fp, "\tslt t%d, t%d, t%d\n", leftNode->place, leftNode->place, rightNode->place);
                    free_t_reg(rightNode->place);
                }
                else{

                }
                break;
            case BINARY_OP_AND:
                if(dataType == INT_TYPE){
                    // a && b
                    int failLabelIndex = labelIndex++;
                    int successLabelIndex = labelIndex++;
                    fprintf(fp, "\tbeqz t%d, L%d\n", leftNode->place, failLabelIndex);
                    fprintf(fp, "\tbeqz t%d, L%d\n", rightNode->place, failLabelIndex);
                    loadConst(1, leftNode->place);
                    fprintf(fp, "\tj L%d\n", successLabelIndex);
                    fprintf(fp, "L%d:\n", failLabelIndex);
                    loadConst(0, leftNode->place);
                    fprintf(fp, "L%d:\n", successLabelIndex);
                }
                else{

                }
                break;
            case BINARY_OP_OR:
                if(dataType == INT_TYPE){
                }
                else{

                }
                break;
        }
        exprNode->place = leftNode->place;
    }
    return;
}

void genArrayElement(AST_NODE *idNode, int offset_reg){
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
            free_t_reg(arrayDimension->place);
            free_t_reg(t_reg_num);
            nowDimension++; 
            arrayDimension = arrayDimension->rightSibling;
        }
        fprintf(fp, "\tsub t%d, fp, t%d\n", offset_reg, offset_reg);
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
    genExprNode(returnItem);
    fprintf(fp, "\tmv a0, t%d\n", returnItem->place);
    fprintf(fp, "\tj _end_%s\n", functionName);
    free_t_reg(returnItem->place);
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
        /*
        if(blockNode->parent->nodeType == DECLARATION_NODE && \
           blockNode->parent->semantic_value.declSemanticValue.kind == FUNCTION_DECL)
            fprintf(fp, ".text\n");
        */
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
                    //genForStmt(stmtNode);
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

