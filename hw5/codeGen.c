#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semanticAnalysis.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;
int ARoffset = 4;
short t_reg_status[7];

#define UNUSED 0
#define NS     1
#define S      2

#define FLOAT_DIMENSION -64
#define IGNORE_DIMENSION -1
#define INVALID_STRING_TYPE 1
#define INVALID_VOID_TYPE 2
#define INVALID_PTR_TYPE 4

FILE *fp;

int get_t_reg(){
    for(int i = 0; i < 7; i++){
        if(t_reg_status[i] == UNUSED){
            t_reg_status[i] = NS;
            return i;
        }
    }
}

void free_t_reg(int t_reg_num){
    t_reg_status[i] = UNUSED;
}

void prologue(char *functionName){
    fprintf(fp, "_start_%s\n", functionName);
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
    fprintf(fp, "_end_%s\n", functionName);
    int offset = 8;
    for(int i = 0; i < 7; i++, offset += 8)
        fprintf(fp, "\tld t%d, %d(sp)\n", i, offset);
    for(int i = 2; i < 12; i++, offset += 8)
        fprintf(fp, "\tld s%d, %d(sp)\n", i, offset);
    fprintf(fp, "\tld fp, %d(sp)\n", offset);
    offset += 8;
    for(int i = 0; i < 8; i++, offset += 4)
        fprintf(fp, "\tflw ft%d %d(sp)\n", i, offset);
    fprintf(fp, "\tld ra, 8(fp)\n");
    fprintf(fp, "\tmv sp, fp\n");
    fprintf(fp, "\tadd sp, sp, 8\n");
    fprintf(fp, "\tld fp, 0(fp)\n");
    fprintf(fp, "\tjr ra\n");
    fprintf(fp, ".data\n");
    fprintf(fp, "\t_frameSize_%s: .word %d\n", functionName, ARoffset);
    ARoffset = 4;
    return;
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
            switch(declarationNode->semantic_value.declSemanticValue.kind){
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
                        fprintf(fp, "\t_%s .word\n", idName);
                    else{
                        idEntry->offset = ARoffset;
                        ARoffset += 4;
                    }   
                    break;
                case ARRAY_ID:
                    int space = getArraySpace(idEntry);
                    if(isGlobal(idEntry))
                        fprintf(fp, "\t_%s .space %d\n", idName, space);
                    else{
                        idEntry->offset = ARoffset + space - 4;
                        ARoffset += space;
                    }
                    break;
                case WITH_INIT_ID:
                    // must be CONST_VALUE_NODE
                    exprNode = idNode->child;
                    if(isGlobal(idEntry)){
                        if(idNode->dataType == INT_TYPE)
                            fprintf(fp, "\t_%s .word %d\n", idName, idNode->semantic_value.const1->const_u.intval);
                        else
                            fprintf(fp, "\t_%s .word %d\n", idName, idNode->semantic_value.const1->const_u.fval);
                    }
                    else{
                        if(idNode->dataType == INT_TYPE){
                            idEntry->offset = ARoffset;
                            int t_reg_num = get_t_reg();
                            int constVal = idNode->semantic_value.const1->const_u.intval;
                            int upper = constVal >> 12;
                            int lower = constVal & 0x00000FFF;
                            if(upper){
                                fprintf(fp, "\tlui t%d, %d\n", t_reg_num, upper);
                                fprintf(fp, "\tori t%d, t%d, %d\n", t_reg_num, t_reg_num, lower);
                                fprintf(fp, "\tsw t%d, -%d(fp)\n", t_reg_num, ARoffset);
                            }
                            else{
                                fprintf(fp, "\taddi t%d, x0, %d\n", t_reg_num, constVal);
                                fprintf(fp, "\tsw t%d, -%d(fp)\n", t_reg_num, ARoffset);
                            }
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
            ArrayProperties *symbolProperty = &(symbolAttr->attr.typeDescriptor->properties.arrayProperties);
            ArrayProperties *typeProperty = &(typeEntry->attribute->attr.typeDescriptor->properties.arrayProperties);
            AST_NODE *exprNode;
            int space = getArraySpace(idEntry);
            if(isGlobal(idEntry))
                fprintf(fp, "\t_%s .space %d\n", idName, space);
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
    int ARsize = genBlockNode(blockNode);
    epilogue(idName);
}

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

void genWhileStmt(AST_NODE* whileNode)
{
    AST_NODE *testExprRoot = whileNode->child;
    AST_NODE *stmtNode = testExprRoot->rightSibling;
    genExprNode(testExprRoot);
    isInvalidExpr(testExprRoot, INVALID_PTR_TYPE + INVALID_VOID_TYPE + INVALID_STRING_TYPE);
    genStmtNode(stmtNode);
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
    while(initAssignExpr){
        if(initAssignExpr->nodeType == STMT_NODE && initAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(initAssignExpr);
        else{
            genExprNode(initAssignExpr);
            isInvalidExpr(initAssignExpr, INVALID_PTR_TYPE);
        }
        initAssignExpr = initAssignExpr->rightSibling;
    }
    while(relopExpr){
        genExprNode(relopExpr);
        isInvalidExpr(relopExpr, INVALID_PTR_TYPE);
        relopExpr = relopExpr->rightSibling;
    }
    while(updateAssignExpr){
        if(updateAssignExpr->nodeType == STMT_NODE && updateAssignExpr->semantic_value.stmtSemanticValue.kind == ASSIGN_STMT)
            genAssignmentStmt(updateAssignExpr);
        else{
            genExprNode(updateAssignExpr);
            isInvalidExpr(updateAssignExpr, INVALID_PTR_TYPE);
        }
        updateAssignExpr = updateAssignExpr->rightSibling;
    }
    genStmtNode(stmtNode);
    return;
}

void genAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE *leftNode = assignmentNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;

    // check variable is available at this scope
    char *variableName = leftNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *leftNodeSymbol = retrieveSymbol(variableName);

    // unavailable, error
    if(leftNodeSymbol == NULL){
        printErrorMsgSpecial(leftNode, variableName, SYMBOL_UNDECLARED);
        leftNode->dataType = ERROR_TYPE;
    }
    // available, name is function
    else if(leftNodeSymbol->attribute->attributeKind == FUNCTION_SIGNATURE){
        printErrorMsgSpecial(leftNode, variableName, IS_FUNCTION_NOT_VARIABLE);
        leftNode->dataType = ERROR_TYPE;
    }
    // available, name is typedef
    else if(leftNodeSymbol->attribute->attributeKind == TYPE_ATTRIBUTE){
        printErrorMsgSpecial(leftNode, variableName, IS_TYPE_NOT_VARIABLE);
        leftNode->dataType = ERROR_TYPE;
    }
    // available, name is an array name
    else if(leftNodeSymbol->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
        // check dimension
        ArrayProperties property = leftNodeSymbol->attribute->attr.typeDescriptor->properties.arrayProperties;
        genArrayReference(leftNode, property, 1);
    }
    // available, name is a scalar
    else{
        leftNode->dataType = leftNodeSymbol->attribute->attr.typeDescriptor->properties.dataType;
    }

    // check relop on RHS of assignment
    genExprNode(rightNode);
    if(isInvalidExpr(rightNode, INVALID_STRING_TYPE + INVALID_VOID_TYPE + INVALID_PTR_TYPE))
        rightNode->dataType = ERROR_TYPE;
    /*
    if(rightNode->nodeType == CONST_VALUE_NODE){
        switch(rightNode->dataType){
            case INT_TYPE:
                printf("data: %d\n", rightNode->semantic_value.const1->const_u.intval);
                break;
            case FLOAT_TYPE:
                printf("data: %f\n", rightNode->semantic_value.const1->const_u.fval);
                break;
        }
    }
    */

    // type conversion
    if(leftNode->dataType == ERROR_TYPE || rightNode->dataType == ERROR_TYPE)
        assignmentNode->dataType = ERROR_TYPE;
    else
        assignmentNode->dataType = leftNode->dataType;
    return;
}


void genIfStmt(AST_NODE* ifNode)
{
    AST_NODE *ifTest = ifNode->child;
    AST_NODE *stmtNode = ifTest->rightSibling;
    genExprNode(ifTest);
    isInvalidExpr(ifTest, INVALID_PTR_TYPE + INVALID_VOID_TYPE);
    genStmtNode(stmtNode);
    genStmtNode(stmtNode->rightSibling);
}

void genWriteFunction(AST_NODE* functionCallNode)
{
    // check if there is one argument
    AST_NODE *toWrite = functionCallNode->child->rightSibling->child;
    if(toWrite == NULL){
        printErrorMsgSpecial(functionCallNode, "write", TOO_FEW_ARGUMENTS);
        return;
    }
    // check if there are more arguments
    AST_NODE *shouldNull = toWrite->rightSibling;
    if(shouldNull != NULL)
        printErrorMsgSpecial(functionCallNode, "write", TOO_MANY_ARGUMENTS);

    genExprNode(toWrite);
    isInvalidExpr(toWrite, INVALID_VOID_TYPE + INVALID_PTR_TYPE);
    // write a constant, should be string
    if(toWrite->nodeType == CONST_VALUE_NODE && toWrite->dataType != CONST_STRING_TYPE)
        printErrorMsg(functionCallNode, NOT_WRITABLE);
    // write an constant expression, invalid
    else if(toWrite->nodeType == EXPR_NODE && toWrite->semantic_value.exprSemanticValue.isConstEval == 1)
        printErrorMsg(functionCallNode, NOT_WRITABLE);
    // others: write identifier & write function call(handled in checkExprNode)
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

    SymbolTableEntry *idEntry = idNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    // genParameterPassing(idEntry->attribute->attr.functionSignature->parameterList, paramNode, idNode);
    fprintf(fp, "\tjal _start_%s\n", idName);
}

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

int isRelativeOperation(AST_NODE *exprRelatedNode){
    EXPRSemanticValue val = exprRelatedNode->semantic_value.exprSemanticValue;
    if(val.kind == BINARY_OPERATION && \
      (val.op.binaryOp == BINARY_OP_EQ || val.op.binaryOp == BINARY_OP_GE || val.op.binaryOp == BINARY_OP_LE || \
       val.op.binaryOp == BINARY_OP_NE || val.op.binaryOp == BINARY_OP_GT || val.op.binaryOp == BINARY_OP_LT || \
       val.op.binaryOp == BINARY_OP_OR || val.op.binaryOp == BINARY_OP_AND))
        return 1;
    else 
        return 0;
}


void getExprOrConstValue(AST_NODE* exprOrConstNode, int** iValue, double** fValue)
{
    if(exprOrConstNode->semantic_value.const1->const_type == INTEGERC){
        *iValue = &exprOrConstNode->semantic_value.const1->const_u.intval;
    }
    else{
        *fValue = &exprOrConstNode->semantic_value.const1->const_u.fval;
    }
    return;
}



void evaluateExprValue(AST_NODE* exprNode)
{
    AST_NODE *leftNode = exprNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    // for unary operation
    if(rightNode == NULL){
       if(leftNode->nodeType == CONST_VALUE_NODE){
            int *livalue = NULL;
            double *lfvalue = NULL;
            getExprOrConstValue(leftNode, &livalue, &lfvalue);
            exprNode->nodeType = CONST_VALUE_NODE;
            CON_Type *constInfo = (CON_Type *)malloc(sizeof(CON_Type));
            //printf("before evaluateExprValue: %d\n", *livalue);
            if(livalue){
                if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION)
                    constInfo->const_u.intval = (*livalue == 0);
                else if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_NEGATIVE)
                    constInfo->const_u.intval = -(*livalue);
                else
                    constInfo->const_u.intval = *livalue;
                constInfo->const_type = INTEGERC;
                exprNode->semantic_value.const1 = constInfo;
                exprNode->dataType = INT_TYPE;
            }
            else if(lfvalue){
                if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION){
                    constInfo->const_u.intval = (*lfvalue == 0);
                    constInfo->const_type = INTEGERC;
                    exprNode->semantic_value.const1 = constInfo;
                    exprNode->dataType = INT_TYPE;
                }
                else if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_NEGATIVE){
                    constInfo->const_u.fval = -(*lfvalue);
                    constInfo->const_type = FLOATC;
                    exprNode->semantic_value.const1 = constInfo;
                    exprNode->dataType = FLOAT_TYPE;
                }
                else{
                    constInfo->const_u.fval = *lfvalue;
                    constInfo->const_type = FLOATC;
                    exprNode->semantic_value.const1 = constInfo;
                    exprNode->dataType = FLOAT_TYPE;
                }
            }
            //printf("after evaluateExprValue: %d\n", exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
        }
        else{
            exprNode->semantic_value.exprSemanticValue.isConstEval = 0;
            if(leftNode->dataType == ERROR_TYPE)
                exprNode->dataType = ERROR_TYPE;
            else if(exprNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION)
                exprNode->dataType = INT_TYPE;
            else
                exprNode->dataType = leftNode->dataType;
        }
    }
    // binary logical operation
    else if(isRelativeOperation(exprNode) && leftNode->nodeType == CONST_VALUE_NODE && rightNode->nodeType == CONST_VALUE_NODE){
        int *livalue = NULL;
        int *rivalue = NULL;
        double *lfvalue = NULL;
        double *rfvalue = NULL;
        getExprOrConstValue(leftNode, &livalue, &lfvalue);
        getExprOrConstValue(rightNode, &rivalue, &rfvalue);
        exprNode->nodeType = CONST_VALUE_NODE;
        CON_Type *constInfo = (CON_Type *)malloc(sizeof(CON_Type));
        // both integer
        if(livalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_EQ)
                constInfo->const_u.intval = *livalue == *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GE)
                constInfo->const_u.intval = *livalue >= *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LE)
                constInfo->const_u.intval = *livalue <= *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_NE)
                constInfo->const_u.intval = *livalue != *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GT)
                constInfo->const_u.intval = *livalue > *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LT)
                constInfo->const_u.intval = *livalue < *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_AND)
                constInfo->const_u.intval = *livalue && *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_OR)
                constInfo->const_u.intval = *livalue || *rivalue;
        }
        // left integer, right float
        else if(livalue && rfvalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_EQ)
                constInfo->const_u.intval = (double)(*livalue) == *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GE)
                constInfo->const_u.intval = (double)(*livalue) >= *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LE)
                constInfo->const_u.intval = (double)(*livalue) <= *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_NE)
                constInfo->const_u.intval = (double)(*livalue) != *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GT)
                constInfo->const_u.intval = (double)(*livalue) > *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LT)
                constInfo->const_u.intval = (double)(*livalue) < *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_AND)
                constInfo->const_u.intval = (double)(*livalue) && *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_OR)
                constInfo->const_u.intval = (double)(*livalue) || *rfvalue;
        }
        // left float, right integer
        else if(lfvalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_EQ)
                constInfo->const_u.intval = *lfvalue == (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GE)
                constInfo->const_u.intval = *lfvalue >= (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LE)
                constInfo->const_u.intval = *lfvalue <= (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_NE)
                constInfo->const_u.intval = *lfvalue != (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GT)
                constInfo->const_u.intval = *lfvalue > (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LT)
                constInfo->const_u.intval = *lfvalue < (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_AND)
                constInfo->const_u.intval = *lfvalue && (double)(*rivalue);
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_OR)
                constInfo->const_u.intval = *lfvalue || (double)(*rivalue);
        }
        // both float
        else{
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_EQ)
                constInfo->const_u.intval = *lfvalue == *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GE)
                constInfo->const_u.intval = *lfvalue >= *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LE)
                constInfo->const_u.intval = *lfvalue <= *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_NE)
                constInfo->const_u.intval = *lfvalue != *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_GT)
                constInfo->const_u.intval = *lfvalue > *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_LT)
                constInfo->const_u.intval = *lfvalue < *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_AND)
                constInfo->const_u.intval = *lfvalue && *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_OR)
                constInfo->const_u.intval = *lfvalue || *rfvalue;
        }
        constInfo->const_type = INTEGERC;
        exprNode->semantic_value.const1 = constInfo;
        exprNode->dataType = INT_TYPE;
    }
    // binary arithmetic operation, both side are constant evaluations
    else if(leftNode->nodeType == CONST_VALUE_NODE && rightNode->nodeType == CONST_VALUE_NODE){
        int *livalue = NULL;
        int *rivalue = NULL;
        double *lfvalue = NULL;
        double *rfvalue = NULL;
        getExprOrConstValue(leftNode, &livalue, &lfvalue);
        getExprOrConstValue(rightNode, &rivalue, &rfvalue);
        exprNode->nodeType = CONST_VALUE_NODE;
        CON_Type *constInfo = (CON_Type *)malloc(sizeof(CON_Type));
        // both integer
        if(livalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                constInfo->const_u.intval = *livalue + *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                constInfo->const_u.intval = *livalue - *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                constInfo->const_u.intval = *livalue * *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                constInfo->const_u.intval = *livalue / *rivalue;
            constInfo->const_type = INTEGERC;
            exprNode->semantic_value.const1 = constInfo;
            exprNode->dataType = INT_TYPE;
        }
        // left integer, right float
        else if(livalue && rfvalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                constInfo->const_u.fval = (double)*livalue + *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                constInfo->const_u.fval = (double)*livalue - *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                constInfo->const_u.fval = (double)*livalue * *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                constInfo->const_u.fval = (double)*livalue / *rfvalue;
            constInfo->const_type = FLOATC;
            exprNode->semantic_value.const1 = constInfo;
            exprNode->dataType = FLOAT_TYPE;
        }
        // left float, right integer
        else if(lfvalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                constInfo->const_u.fval = *lfvalue + (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                constInfo->const_u.fval = *lfvalue - (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                constInfo->const_u.fval = *lfvalue * (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                constInfo->const_u.fval = *lfvalue / (double)*rivalue;
            constInfo->const_type = FLOATC;
            exprNode->semantic_value.const1 = constInfo;
            exprNode->dataType = FLOAT_TYPE;
        }
        // both float
        else{
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                constInfo->const_u.fval = *lfvalue + *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                constInfo->const_u.fval = *lfvalue - *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                constInfo->const_u.fval = *lfvalue * *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                constInfo->const_u.fval = *lfvalue / *rfvalue;
            constInfo->const_type = FLOATC;
            exprNode->semantic_value.const1 = constInfo;
            exprNode->dataType = FLOAT_TYPE;
        }
    }
    else{
        exprNode->semantic_value.exprSemanticValue.isConstEval = 0;
        exprNode->dataType = getBiggerType(leftNode->dataType, rightNode->dataType);
    }
    return;
}

int isInvalidExpr(AST_NODE *exprNode, int invalidType){
    if(exprNode->dataType == INT_TYPE || exprNode->dataType == FLOAT_TYPE) return 0;
    switch(exprNode->dataType){
        case CONST_STRING_TYPE:
            if(invalidType & INVALID_STRING_TYPE){
                printErrorMsg(exprNode, STRING_OPERATION);
                return 1;
            }
            break;
        case VOID_TYPE:
            if(invalidType & INVALID_VOID_TYPE){
                printErrorMsgSpecial(exprNode, exprNode->child->semantic_value.identifierSemanticValue.identifierName, INVALID_OPERAND);
                return 1;
            }
            break;
        case INT_PTR_TYPE: case FLOAT_PTR_TYPE:
            if(invalidType & INVALID_PTR_TYPE){
                printErrorMsg(exprNode, INVALID_ARRAY_POINTER);
                return 1;
            }
            break;
        default: // ERROR_TYPE
            return 1;
            break;
    }
    return 0;
}

void genExprNode(AST_NODE* exprNode)
{
    if(exprNode->nodeType == NUL_NODE) return;
    // constant node
    if(exprNode->nodeType == CONST_VALUE_NODE){
        if(exprNode->dataType == INT_TYPE){
            int t_reg_num = get_t_reg();
            int constVal = exprNode->semantic_value.const1->const_u.intval;
            int upper = constVal >> 12;
            int lower = constVal & 0x00000FFF;
            if(upper){
                fprintf(fp, "\tlui t%d, %d\n", t_reg_num, upper);
                fprintf(fp, "\tori t%d, t%d, %d\n", t_reg_num, t_reg_num, lower);
            }
            else
                fprintf(fp, "\taddi t%d, x0, %d\n", t_reg_num, constVal);
            exprNode->place = t_reg_num;
        }
        else if(exprNode->dataType == FLOAT_TYPE){
            // TODO: float const
        }
        else{
            fprintf(fp, ".data\n");
            fprintf(fp, "");
        }
        return;
    }
    // function call node
    else if(exprNode->nodeType == STMT_NODE && \
            exprNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
        genFunctionCall(exprNode);
        return;
    }
    // identifier node
    else if(exprNode->nodeType == IDENTIFIER_NODE){
        char *identifierName = exprNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *identifier = exprNode->semantic_value.identifierSemanticValue.symbolTableEntry;
        if(identifier->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            int t_reg_num = get_t_reg();
            exprNode->place = t_reg_num;
        }
        // identifier is a scalar
        else{
            exprNode->dataType = identifier->attribute->attr.typeDescriptor->properties.dataType;
            int t_reg_num = get_t_reg();
            exprNode->place = t_reg_num;
        }
        return;
    }
    // nonterminal nodes
    AST_NODE *leftNode = exprNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    int invalidType = INVALID_VOID_TYPE + INVALID_STRING_TYPE + INVALID_PTR_TYPE;
    // relative operation, binary, e.g, a > 0, (a > 0 || b < 0)
    if(isRelativeOperation(exprNode)){
        genExprNode(leftNode);
        genExprNode(rightNode);
        // value of relative expression: 0 or 1(integer)
        if(isInvalidExpr(leftNode, invalidType) || isInvalidExpr(rightNode, invalidType))
            exprNode->dataType = ERROR_TYPE;
        else
            evaluateExprValue(exprNode);
    }
    // unary operation
    else if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION){
        genExprNode(leftNode);
        if(isInvalidExpr(leftNode, invalidType))
            exprNode->dataType = ERROR_TYPE;
        else
            evaluateExprValue(exprNode);
    }
    // binary arithmetic operation, e.g, +, -, *, /
    else{
        genExprNode(leftNode);
        genExprNode(rightNode);
        if(isInvalidExpr(leftNode, invalidType) || isInvalidExpr(rightNode, invalidType))
            exprNode->dataType = ERROR_TYPE;
        else
            evaluateExprValue(exprNode);
        // if(leftNode->dataType == CONST_STRING_TYPE || rightNode->dataType == CONST_STRING_TYPE){
        //     exprNode->dataType = ERROR_TYPE;
        //     printErrorMsg(exprNode, STRING_OPERATION);
        // }
        // else if(leftNode->dataType != ERROR_TYPE && rightNode->dataType != ERROR_TYPE)
        //     evaluateExprValue(exprNode);
        // else
        //     exprNode->dataType = ERROR_TYPE;
    }
    return;
}

/*
void checkVariableLValue(AST_NODE* idNode)
{
}

void checkVariableRValue(AST_NODE* idNode)
{
}


void checkConstValueNode(AST_NODE* constValueNode)
{
}
*/

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
    isInvalidExpr(returnItem, INVALID_PTR_TYPE + INVALID_STRING_TYPE);
    if(typeNode->dataType != ERROR_TYPE){
        if(typeNode->dataType == VOID_TYPE && returnItem->nodeType != NUL_NODE){
            printErrorMsgSpecial(returnNode, functionName, RETURN_IN_VOID_FUNCTION);
        }
        else if(typeNode->dataType != VOID_TYPE && returnItem->nodeType == NUL_NODE){
            // warning : return value in non-void function
        }
    }
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
        fprintf(fp, ".data\n");
        AST_NODE *decl = declList->child;
        while(decl != NULL){
            genDeclarationNode(decl);
            decl = decl->rightSibling;
        }
    }
    if(stmtList != NULL){
        fprintf(fp, ".text\n");
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

