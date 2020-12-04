#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semanticAnalysis.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;


typedef enum ErrorMsgKind
{
    SYMBOL_IS_NOT_TYPE,
    SYMBOL_REDECLARE,
    SYMBOL_UNDECLARED,
    NOT_FUNCTION_NAME,
    TRY_TO_INIT_ARRAY,
    EXCESSIVE_ARRAY_DIM_DECLARATION,
    RETURN_ARRAY,
    VOID_VARIABLE,
    TYPEDEF_VOID_ARRAY,
    PARAMETER_TYPE_UNMATCH,
    TOO_FEW_ARGUMENTS,
    TOO_MANY_ARGUMENTS,
    RETURN_TYPE_UNMATCH,
    INCOMPATIBLE_ARRAY_DIMENSION,
    NOT_ASSIGNABLE,
    NOT_ARRAY,
    IS_TYPE_NOT_VARIABLE,
    IS_FUNCTION_NOT_VARIABLE,
    STRING_OPERATION,
    ARRAY_SIZE_NOT_INT,
    ARRAY_SIZE_NEGATIVE,
    ARRAY_SUBSCRIPT_NOT_INT,
    PASS_ARRAY_TO_SCALAR,
    PASS_SCALAR_TO_ARRAY
} ErrorMsgKind;

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    
    switch(errorMsgKind)
    {
        case SYMBOL_REDECLARE:
            printf("redeclaration of %s\n", name2);
            break;
        default:
            printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
            break;
    }
    
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    /*
    switch(errorMsgKind)
    {
        printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
        break;
    }
    */
}


void semanticAnalysis(AST_NODE *root)
{
    checkProgramNode(root);
}


DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2)
{
    if(dataType1 == FLOAT_TYPE || dataType2 == FLOAT_TYPE) {
        return FLOAT_TYPE;
    } else {
        return INT_TYPE;
    }
}


void checkProgramNode(AST_NODE *programNode)
{
    if(programNode->child == NULL) return;
    openScope();
    initType();
    AST_NODE *globalDecl = programNode->child;
    // globalDecl is a func_decl or a var_decl_list
    while(globalDecl != NULL){
        if(globalDecl->nodeType == VARIABLE_DECL_LIST_NODE){
            AST_NODE *declList = globalDecl->child;
            while(declList != NULL){
                checkDeclarationNode(declList);
                declList = declList->rightSibling;
            }
        }
        else if(globalDecl->nodeType == DECLARATION_NODE){
            checkDeclarationNode(globalDecl);
        }
        else{
            printf("There is an exception type = %d in program node.\n", globalDecl->nodeType);
        }
        globalDecl = globalDecl->rightSibling;
    }
    closeScope();
}

void initType(){
    SymbolAttribute *symbolAttr;
    // enter int type entry
    symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
    symbolAttr->attributeKind = TYPE_ATTRIBUTE;
    symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
    symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    symbolAttr->attr.typeDescriptor->properties.dataType = INT_TYPE;
    enterSymbol("int", symbolAttr);
    // enter float type entry
    symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
    symbolAttr->attributeKind = TYPE_ATTRIBUTE;
    symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
    symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    symbolAttr->attr.typeDescriptor->properties.dataType = FLOAT_TYPE;
    enterSymbol("float", symbolAttr);
    // enter void type entry
    symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
    symbolAttr->attributeKind = TYPE_ATTRIBUTE;
    symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
    symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
    symbolAttr->attr.typeDescriptor->properties.dataType = VOID_TYPE;
    enterSymbol("void", symbolAttr);
}

void checkDeclarationNode(AST_NODE* declarationNode)
{
    switch(declarationNode->semantic_value.declSemanticValue.kind){
        case TYPE_DECL:
            declareType(declarationNode);
            break;
        case VARIABLE_DECL:
            declareVariable(declarationNode);
            break;
        case FUNCTION_DECL:
            declareFunction(declarationNode);
            break;
    }
}

void declareType(AST_NODE *declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    DATA_TYPE dataType;
    checkTypeNode(typeNode, &dataType);
    while(idNode != NULL){
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        if(isDeclaredLocally(idName)){
            char errMsg[ERR_MSG_LEN];
            sprintf(errMsg, "\'typedef %s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
            printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
        }
        else{
            SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
            symbolAttr->attributeKind = TYPE_ATTRIBUTE;
            symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
            // ignore the ARRAY_TYPE
            symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
            symbolAttr->attr.typeDescriptor->properties.dataType = dataType;
            enterSymbol(idName, symbolAttr);
        }
        idNode = idNode->rightSibling;
    }

}

void declareVariable(AST_NODE *declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    DATA_TYPE dataType;
    checkTypeNode(typeNode, &dataType);
    while(idNode != NULL){
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        if(isDeclaredLocally(idName)){
            char errMsg[ERR_MSG_LEN];
            sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
            printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
        }
        else{
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
                    getArrayDimensionAndSize(symbolAttr, idNode, 0);
                    break;
                case WITH_INIT_ID:
                    break;
            }
            SymbolTableEntry *entry = enterSymbol(idName, symbolAttr);
        }
        idNode = idNode->rightSibling;
    }
}

void getArrayDimensionAndSize(SymbolAttribute *symbolAttr, AST_NODE *idNode, int ignoreFirstDimSize){
    int *dim = &(symbolAttr->attr.typeDescriptor->properties.arrayProperties.dimension);
    int *size = symbolAttr->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
    if(ignoreFirstDimSize){
        size[(*dim)++] = -1;
        idNode = idNode->rightSibling;
    }
    while(idNode != NULL){
        switch(idNode->nodeType){
            case CONST_VALUE_NODE:
                switch(idNode->semantic_value.const1->const_type){
                    case INTEGERC:
                        size[*(dim)++] = idNode->semantic_value.const1->const_u.intval;
                        if(size[*dim - 1] < 0){
                            // error : negative size
                        }
                        break;
                    default:
                        // error : e.g. int a[3.2], b["abc"]
                        break;
                }
                break;
            // this case is fucking complicated, process later.
            case EXPR_NODE:
                if(idNode->semantic_value.exprSemanticValue.isConstEval){
                    if(idNode->semantic_value.exprSemanticValue.constType == INTEGERC){
                        size[(*dim)++] = idNode->semantic_value.exprSemanticValue.constEvalValue.iValue;
                    }
                    else{
                        // error : e.g. int a[3.2], b["abc"]
                    }
                }
                else{
                    // size is an expr, not in hw
                }
                break;
        }
        idNode = idNode->rightSibling;
    }
}

void declareFunction(AST_NODE* declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    AST_NODE *paramListNode = idNode->rightSibling;
    AST_NODE *blockNode = paramListNode->rightSibling;
    DATA_TYPE dataType;
    checkTypeNode(typeNode, &dataType);
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    if(isDeclaredLocally(idName)){
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
        printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
    }
    else{
        SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
        symbolAttr->attributeKind = FUNCTION_SIGNATURE;
        symbolAttr->attr.functionSignature = (FunctionSignature *)malloc(sizeof(FunctionSignature));
        symbolAttr->attr.functionSignature->returnType = dataType;
        symbolAttr->attr.functionSignature->parametersCount = 0;
        symbolAttr->attr.functionSignature->parameterList = NULL;
        // enter function symbol first
        // construct the paramList in funcSignature and declare param at the same time
        enterSymbol(idName, symbolAttr);
        openScope();
        declareFunctionParam(paramListNode, symbolAttr->attr.functionSignature);
        // error : check if function is legal ( e.g. illegal paramList)
        checkBlockNode(blockNode);
        closeScope();
    }
}

void declareFunctionParam(AST_NODE *paramListNode, FunctionSignature *functionSignature){
    Parameter **paramListTail = &(functionSignature->parameterList);
    AST_NODE *paramNode = paramListNode->child;
    while(paramNode != NULL){
        AST_NODE *typeNode = paramNode->child;
        AST_NODE *idNode = typeNode->rightSibling;
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        DATA_TYPE dataType;
        checkTypeNode(typeNode, &dataType);
        // declare param
        if(dataType != ERROR_TYPE && isDeclaredLocally(idName)){
            char errMsg[ERR_MSG_LEN];
            sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
            printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
        }
        else{
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
            enterSymbol(idName, symbolAttr);
        }
        // construct paramList
        if(dataType != ERROR_TYPE){
            SymbolTableEntry *entry = retrieveSymbol(idName);
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


void checkTypeNode(AST_NODE* typeNode, DATA_TYPE *dataType)
{
    SymbolTableEntry *typeEntry = retrieveSymbol(typeNode->semantic_value.identifierSemanticValue.identifierName);
    if(typeEntry == NULL || typeEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
        // error : unknown type or not a type
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s\' is not a type", typeNode->semantic_value.identifierSemanticValue.identifierName);
        printErrorMsgSpecial(typeNode, errMsg, SYMBOL_IS_NOT_TYPE);
    }
    else{
        // ignore the ARRAY_TYPE ( which is defined as array e.g. typedef int II[2])
        *dataType = typeEntry->attribute->attr.typeDescriptor->properties.dataType;
    }
}


void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
}

void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
}

void checkWhileStmt(AST_NODE* whileNode)
{
    AST_NODE *testExprRoot = whileNode->child;
    AST_NODE *stmtNode = testExprRoot->rightSibling;
    checkExprRelatedNode(testExprRoot);
    checkStmtNode(stmtNode);
}


void checkForStmt(AST_NODE* forNode)
{
    AST_NODE *initAssignExprRoot = forNode->child;
    AST_NODE *initAssignExpr = initAssignExprRoot->child;
    AST_NODE *relopExprRoot = initAssignExprRoot->rightSibling;
    AST_NODE *relopExpr = relopExprRoot->child;
    AST_NODE *updateAssignExprRoot = relopExprRoot->rightSibling;
    AST_NODE *updateAssignExpr = updateAssignExprRoot->child;
    AST_NODE *stmtNode = updateAssignExprRoot->rightSibling;
    while(initAssignExpr){
        checkAssignmentStmt(initAssignExpr);
        initAssignExpr = initAssignExpr->rightSibling;
    }
    while(relopExpr){
        checkExprRelatedNode(relopExpr);
        relopExpr = relopExpr->rightSibling;
    }
    while(updateAssignExpr){
        checkAssignmentStmt(updateAssignExpr);
        updateAssignExpr = updateAssignExpr->rightSibling;
    }
    checkStmtNode(stmtNode);
    return;
}

void checkArrayReference(AST_NODE *arrayDimension, ArrayProperties property)
{
    int nowDimension = 0;

    while(arrayDimension){
        int assignDimensionIndex = arrayDimension->semantic_value.const1->const_u.intval;
        if(assignDimensionIndex >= property.sizeInEachDimension[nowDimension]){
            // out of bound error
        }
        else if(assignDimensionIndex < 0){
            // negative index error
        }
        arrayDimension = arrayDimension->rightSibling;
        nowDimension++;
    }
    if(nowDimension < property.dimension){
        // assign to an array address error    
    }
    else if(nowDimension > property.dimension){
        // dimension error
    }
    return;
}

void checkAssignmentStmt(AST_NODE* assignmentNode)
{
    AST_NODE *leftNode = assignmentNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    DATA_TYPE leftNodeType;
    DATA_TYPE rightNodeType;
    // check variable is available at this scope
    char *variableName = leftNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *leftNodeSymbol = retrieveSymbol(variableName);
    if(leftNodeSymbol == NULL){
        // unavailable, error
    }
    // available, check this name is function, typedef or variable
    else if(leftNodeSymbol->attribute->attributeKind == FUNCTION_SIGNATURE){
        // name is function, error
    }
    else if(leftNodeSymbol->attribute->attributeKind == TYPE_ATTRIBUTE){
        // name is typedef, error
    }
    // name is an array name
    else if(leftNodeSymbol->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
        // check dimension
        ArrayProperties property = leftNodeSymbol->attribute->attr.typeDescriptor->properties.arrayProperties;
        checkArrayReference(leftNode->child, property);
        leftNodeType = property.elementType;
    }
    // name is a scalar variable
    else
        leftNodeType = leftNodeSymbol->attribute->attr.typeDescriptor->properties.dataType;
    checkExprRelatedNode(rightNode);
    rightNodeType = rightNode->dataType;
    return;
}


void checkIfStmt(AST_NODE* ifNode)
{
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
    AST_NODE *idNode = functionCallNode->child;
    AST_NODE *paramNode = idNode->rightSibling->child;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *idEntry = retrieveSymbol(idName);
    if(idEntry == NULL){
        // warning : implicit declaration
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    else if(idEntry->attribute->attributeKind != FUNCTION_SIGNATURE){
        // error : not callable
        functionCallNode->dataType = ERROR_TYPE;
        return;
    }
    checkParameterPassing(idEntry->attribute->attr.functionSignature->parameterList, paramNode);
    functionCallNode->dataType = idEntry->attribute->attr.functionSignature->returnType;
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
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

void checkExprRelatedNode(AST_NODE* exprRelatedNode)
{
    if(exprRelatedNode->nodeType == CONST_VALUE_NODE){
        // put data type in dataType field of AST_NODE
        if(exprRelatedNode->semantic_value.const1->const_type == INTEGERC)
            exprRelatedNode->dataType = INT_TYPE;
        else
            exprRelatedNode->dataType = FLOAT_TYPE;
        return;
    }
    else if(exprRelatedNode->nodeType == IDENTIFIER_NODE){
        char *identifierName = exprRelatedNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *identifier = retrieveSymbol(identifierName);
        // identifier not available
        if(identifier == NULL){
            // error
        }
        // identifier is a function
        else if(identifier->attribute->attributeKind == FUNCTION_SIGNATURE){
            // check function call
            checkFunctionCall(exprRelatedNode);
            exprRelatedNode->dataType = identifier->attribute->attr.functionSignature->returnType;
            return;
        }
        // identifier is a typedef
        else if(identifier->attribute->attributeKind == TYPE_ATTRIBUTE){
            // error
        }
        // identifier is an array
        else if(identifier->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            ArrayProperties property = identifier->attribute->attr.typeDescriptor->properties.arrayProperties;
            checkArrayReference(exprRelatedNode->child, property);
            exprRelatedNode->dataType = identifier->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
            return;
        }
        else{
            exprRelatedNode->dataType = identifier->attribute->attr.typeDescriptor->properties.dataType;
            return;
        }
    }
    else if(isRelativeOperation(exprRelatedNode)){
        checkExprRelatedNode(exprRelatedNode->child);
        checkExprRelatedNode(exprRelatedNode->child->rightSibling);
        // value of relative expression: 0 or 1(integer)
        exprRelatedNode->dataType = INT_TYPE;
    }
    else
        checkExprNode(exprRelatedNode);
    return;
}

void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, float* fValue)
{
}

void evaluateExprValue(AST_NODE* exprNode)
{
}


void checkExprNode(AST_NODE* exprNode)
{
}


void checkVariableLValue(AST_NODE* idNode)
{
}

void checkVariableRValue(AST_NODE* idNode)
{
}


void checkConstValueNode(AST_NODE* constValueNode)
{
}


void checkReturnStmt(AST_NODE* returnNode)
{
}


void checkBlockNode(AST_NODE* blockNode)
{
    // Open a scope in the previous node, since not every block opens a new scope. e.g. funcID--paramList--Block
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
            checkDeclarationNode(decl);
            decl = decl->rightSibling;
        }
    }
    if(stmtList != NULL){
        AST_NODE *stmt = stmtList->child;
        while(stmt != NULL){
            checkStmtNode(stmt);
            stmt = stmt->rightSibling;
        }
    }
}


void checkStmtNode(AST_NODE* stmtNode)
{
}


void checkGeneralNode(AST_NODE *node)
{
}

void checkDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize)
{
}



