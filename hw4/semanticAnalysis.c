#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semanticAnalysis.h"
// This file is for reference only, you are not required to follow the implementation. //
// You only need to check for errors stated in the hw4 document. //
int g_anyErrorOccur = 0;

#define FLOAT_DIMENSION -64
#define IGNORE_DIMENSION -1

extern SymbolTable symbolTable;

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
    RETURN_IN_VOID_FUNCTION,
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
    PASS_SCALAR_TO_ARRAY,
    NOT_WRITABLE,
    PASS_INCOMPATIBLE_DIMENSION,
    INVALID_OPERAND,
    NONCONSTANT_GLOBAL_DECLARATION,
    CONFLICT_TYPE
} ErrorMsgKind;

void printErrorMsgSpecial(AST_NODE* node1, char* name2, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node1->linenumber);
    
    switch(errorMsgKind)
    {
        case TOO_MANY_ARGUMENTS:
            printf("too many arguments to function %s\n", name2);
            break;
        case TOO_FEW_ARGUMENTS:
            printf("too few arguments to function %s\n", name2);
            break;
        case SYMBOL_REDECLARE:
            printf("redeclaration of %s\n", name2);
            break;
        case SYMBOL_UNDECLARED:
            printf("%s was not declared in this scope\n", name2);
            break;
        case IS_FUNCTION_NOT_VARIABLE:
            printf("%s is a function, not variable\n", name2);
            break;
        case IS_TYPE_NOT_VARIABLE:
            printf("%s is a type, not variable\n", name2);
            break;
        case PASS_ARRAY_TO_SCALAR:
            printf("invalid conversion from %s\n", name2);
            break;
        case ARRAY_SIZE_NEGATIVE:
            printf("size of array %s is negative\n", name2);
            break;
        case ARRAY_SIZE_NOT_INT:
            printf("size of array %s is not an integet\n", name2);
            break;
        case PASS_SCALAR_TO_ARRAY:
            printf("invalid conversion from %s\n", name2);
            break;
        case PASS_INCOMPATIBLE_DIMENSION:
            printf("invalid conversion from %s\n", name2);
            break;
        case RETURN_ARRAY:
            printf("return type %s is an array\n", name2);
            break;
        case INVALID_OPERAND:
            printf("type of operand %s is void\n", name2);
            break;
        case VOID_VARIABLE:
            printf("variable %s has incomplete type \'void\'\n", name2);
            break;
        case RETURN_IN_VOID_FUNCTION:
            printf("return value in void function \'%s\'\n", name2);
            break;
        case NOT_FUNCTION_NAME:
            printf("called object \'%s\' is not a function or function pointer\n", name2);
            break;
        case CONFLICT_TYPE:
            printf("conflicting types for \'%s\'\n", name2);
            break;
            /*
        default:
            printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
            break;
            */
    }
    
}


void printErrorMsg(AST_NODE* node, ErrorMsgKind errorMsgKind)
{
    g_anyErrorOccur = 1;
    printf("Error found in line %d\n", node->linenumber);
    switch(errorMsgKind)
    {   
        case NOT_ASSIGNABLE:
            printf("assignment to expression with array type\n");
            break;
        case INCOMPATIBLE_ARRAY_DIMENSION:
            printf("subscripted value is neither array nor pointer nor vector\n");
            break;
        case ARRAY_SUBSCRIPT_NOT_INT:
            printf("array subscript is not an integer\n");
            break;
        case NOT_WRITABLE:
            printf("argument of write() is neither identifier nor constant string nor function call nor expression of above\n");
            break;
        case STRING_OPERATION:
            printf("opeartion on string\n");
            break;
        case TYPEDEF_VOID_ARRAY:
            printf("array has incomplete element type \'void\'\n");
            break;
        case TRY_TO_INIT_ARRAY:
            printf("initializing an array is not allowed\n");
            break;
        case NONCONSTANT_GLOBAL_DECLARATION:
            printf("initializer element is not constant\n");
            break;
        default:
            printf("Unhandled case in void printErrorMsg(AST_NODE* node, ERROR_MSG_KIND* errorMsgKind)\n");
            break;
    }
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
    initFunction();
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
    return;
}

void initFunction(){
    SymbolAttribute *symbolAttr;
    // enter read() entry
    symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
    symbolAttr->attributeKind = FUNCTION_SIGNATURE;
    symbolAttr->attr.functionSignature = (FunctionSignature *)malloc(sizeof(FunctionSignature));
    symbolAttr->attr.functionSignature->parametersCount = 0;
    symbolAttr->attr.functionSignature->parameterList = NULL;
    symbolAttr->attr.functionSignature->returnType = INT_TYPE;
    enterSymbol("read", symbolAttr);
    // enter fread() entry
    symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
    symbolAttr->attributeKind = FUNCTION_SIGNATURE;
    symbolAttr->attr.functionSignature = (FunctionSignature *)malloc(sizeof(FunctionSignature));
    symbolAttr->attr.functionSignature->parametersCount = 0;
    symbolAttr->attr.functionSignature->parameterList = NULL;
    symbolAttr->attr.functionSignature->returnType = FLOAT_TYPE;
    enterSymbol("fread", symbolAttr);
    return;
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
            SymbolTableEntry *idEntry = retrieveSymbol(idName);
            if(idEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
                char errMsg[ERR_MSG_LEN];
                sprintf(errMsg, "\'typedef %s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
                printErrorMsgSpecial(declarationNode, errMsg, SYMBOL_REDECLARE);
            }
            else
                printErrorMsgSpecial(declarationNode, idName, CONFLICT_TYPE);
        }
        else{
            AST_NODE *dimensionNode = idNode->child;
            SymbolTableEntry *typeEntry = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry;
            SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
            symbolAttr->attributeKind = TYPE_ATTRIBUTE;
            symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
            // type is not declared, idNode is scalar type -> scalar
            if(typeEntry == NULL && dimensionNode == NULL){
                symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                symbolAttr->attr.typeDescriptor->properties.dataType = ERROR_TYPE;
            }
            // type is not declared, idNode is array type -> array
            else if(typeEntry == NULL && dimensionNode != NULL){
                symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                symbolAttr->attr.typeDescriptor->properties.arrayProperties.elementType = ERROR_TYPE;
                getArrayDimensionAndSize(symbolAttr, idNode, 0);
            }
            // both typeNode and idNode are scalar type -> scalar
            if((typeEntry == NULL || typeEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR) && dimensionNode == NULL){
                symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                symbolAttr->attr.typeDescriptor->properties.dataType = dataType;
            }
            // typeNode is array type, idNode is scalar type -> array
            else if(typeEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR && dimensionNode == NULL){
                ArrayProperties *symbolProperty = &(symbolAttr->attr.typeDescriptor->properties.arrayProperties);
                ArrayProperties *typeProperty = &(typeEntry->attribute->attr.typeDescriptor->properties.arrayProperties);
                symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                symbolProperty->elementType = dataType;
                symbolProperty->dimension = typeProperty->dimension;
                for(int i = 0; i < typeProperty->dimension; i++)
                    symbolProperty->sizeInEachDimension[i] = typeProperty->sizeInEachDimension[i];
            }
            // typeNode is scalar type, idNode is array type -> array
            else if(typeEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR && dimensionNode != NULL){
                ArrayProperties *symbolProperty = &(symbolAttr->attr.typeDescriptor->properties.arrayProperties);
                ArrayProperties *typeProperty = &(typeEntry->attribute->attr.typeDescriptor->properties.arrayProperties);
                if(typeProperty->elementType == VOID_TYPE){
                    printErrorMsg(declarationNode, TYPEDEF_VOID_ARRAY);
                    symbolProperty->elementType = ERROR_TYPE;
                }
                else
                    symbolProperty->elementType = dataType;
                symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                getArrayDimensionAndSize(symbolAttr, idNode, 0);
            }
            // typeNode is array type, idNode is array type -> array
            else{
                ArrayProperties *symbolProperty = &(symbolAttr->attr.typeDescriptor->properties.arrayProperties);
                ArrayProperties *typeProperty = &(typeEntry->attribute->attr.typeDescriptor->properties.arrayProperties);
                symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                symbolProperty->elementType = dataType;
                getArrayDimensionAndSize(symbolAttr, idNode, 0);
                for(int i = 0; i < typeProperty->dimension; i++)
                    symbolProperty->sizeInEachDimension[symbolProperty->dimension + i] = typeProperty->sizeInEachDimension[i];
                symbolProperty->dimension += typeProperty->dimension;
                idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, symbolAttr);
            }
            idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, symbolAttr);
        }
        idNode = idNode->rightSibling;
    }
    return;
}

void declareVariable(AST_NODE *declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    DATA_TYPE dataType;
    checkTypeNode(typeNode, &dataType);
    SymbolTableEntry *typeEntry = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    if(typeEntry != NULL && dataType != ERROR_TYPE){
        TypeDescriptor *typeDescriptor = typeEntry->attribute->attr.typeDescriptor;
        if(typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR && typeDescriptor->properties.dataType == VOID_TYPE){
            printErrorMsgSpecial(declarationNode, idName, VOID_VARIABLE);
            dataType = ERROR_TYPE;
        }
    }
    while(idNode != NULL){
        char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
        if(isDeclaredLocally(idName)){
            char errMsg[ERR_MSG_LEN];
            sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
            printErrorMsgSpecial(declarationNode, errMsg, SYMBOL_REDECLARE);
        }
        else{
            SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
            symbolAttr->attributeKind = VARIABLE_ATTRIBUTE;
            symbolAttr->attr.typeDescriptor = (TypeDescriptor *)malloc(sizeof(TypeDescriptor));
            AST_NODE *exprNode;
            // scalar type or error type
            if(dataType == ERROR_TYPE || typeEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
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
                        exprNode = idNode->child;
                        symbolAttr->attr.typeDescriptor->kind = SCALAR_TYPE_DESCRIPTOR;
                        symbolAttr->attr.typeDescriptor->properties.dataType = dataType;
                        checkExprNode(exprNode);
                        if(exprNode->nodeType == CONST_VALUE_NODE && exprNode->semantic_value.const1->const_type == STRINGC)
                            printErrorMsg(declarationNode, STRING_OPERATION);
                        // global declaration case
                        if(symbolTable.currentLevel == 0){
                            switch(exprNode->nodeType){
                                case CONST_VALUE_NODE:
                                    break;
                                case IDENTIFIER_NODE:
                                    printErrorMsg(declarationNode, NONCONSTANT_GLOBAL_DECLARATION);
                                    break;
                                case EXPR_NODE:
                                    if(exprNode->semantic_value.exprSemanticValue.isConstEval == 0)
                                        printErrorMsg(declarationNode, NONCONSTANT_GLOBAL_DECLARATION);
                                    break;
                                case STMT_NODE:
                                    if(exprNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT)
                                        printErrorMsg(declarationNode, NONCONSTANT_GLOBAL_DECLARATION);
                                    break;
                                default:
                                    break;
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
                switch (idNode->semantic_value.identifierSemanticValue.kind){
                    case NORMAL_ID:
                        symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                        symbolProperty->elementType = dataType;
                        for(int i = 0; i < typeProperty->dimension; i++)
                            symbolProperty->sizeInEachDimension[i] = typeProperty->sizeInEachDimension[i];
                        symbolProperty->dimension = typeProperty->dimension;
                        break;
                    case ARRAY_ID:
                        symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                        symbolProperty->elementType = dataType;
                        getArrayDimensionAndSize(symbolAttr, idNode, 0);
                        for(int i = 0; i < typeProperty->dimension; i++)
                            symbolProperty->sizeInEachDimension[symbolProperty->dimension + i] = typeProperty->sizeInEachDimension[i];
                        symbolProperty->dimension += typeProperty->dimension;
                        break;
                    case WITH_INIT_ID:
                        printErrorMsg(declarationNode, TRY_TO_INIT_ARRAY);
                        symbolAttr->attr.typeDescriptor->kind = ARRAY_TYPE_DESCRIPTOR;
                        symbolProperty->elementType = dataType;
                        exprNode = idNode->child;
                        getArrayDimensionAndSize(symbolAttr, idNode, 0);
                        for(int i = 0; i < typeProperty->dimension; i++)
                            symbolProperty->sizeInEachDimension[symbolProperty->dimension + i] = typeProperty->sizeInEachDimension[i];
                        symbolProperty->dimension += typeProperty->dimension;
                        checkExprNode(exprNode);
                        if(exprNode->nodeType == CONST_VALUE_NODE && exprNode->semantic_value.const1->const_type == STRINGC)
                            printErrorMsg(declarationNode, STRING_OPERATION);
                        break;
                }
            }
            idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, symbolAttr);
        }
        idNode = idNode->rightSibling;
    }
}

void getArrayDimensionAndSize(SymbolAttribute *symbolAttr, AST_NODE *idNode, int ignoreFirstDimSize){
    char *name = idNode->semantic_value.identifierSemanticValue.identifierName;
    AST_NODE *arrayDimension = idNode->child;
    int *size = symbolAttr->attr.typeDescriptor->properties.arrayProperties.sizeInEachDimension;
    int nowDim = 0;
    if(ignoreFirstDimSize){
        nowDim += 1;
        arrayDimension = arrayDimension->rightSibling;
        size[0] = IGNORE_DIMENSION;
    }
    while(arrayDimension != NULL){
        switch(arrayDimension->nodeType){
            case CONST_VALUE_NODE:
                if(arrayDimension->semantic_value.const1->const_type != INTEGERC){
                    printErrorMsgSpecial(idNode, name, ARRAY_SIZE_NOT_INT);
                    size[nowDim] = FLOAT_DIMENSION;
                }
                else if(arrayDimension->semantic_value.const1->const_u.intval < 0){
                    printErrorMsgSpecial(idNode, name, ARRAY_SIZE_NEGATIVE);
                    size[nowDim] = arrayDimension->semantic_value.const1->const_u.intval;
                }
                else
                    size[nowDim] = arrayDimension->semantic_value.const1->const_u.intval;
                break;
            case EXPR_NODE:
                checkExprNode(arrayDimension);
                if(arrayDimension->dataType == FLOAT_TYPE){
                    printErrorMsgSpecial(idNode, name, ARRAY_SIZE_NOT_INT);
                    size[nowDim] = FLOAT_DIMENSION;
                }
                else if(arrayDimension->semantic_value.exprSemanticValue.constEvalValue.iValue < 0){
                    printErrorMsgSpecial(idNode, name, ARRAY_SIZE_NEGATIVE);
                    size[nowDim] = arrayDimension->semantic_value.exprSemanticValue.constEvalValue.iValue;
                }
                else
                    size[nowDim] = arrayDimension->semantic_value.exprSemanticValue.constEvalValue.iValue;
                break;
            default:
                break;
        }
        nowDim++;
        arrayDimension = arrayDimension->rightSibling;
    }
    symbolAttr->attr.typeDescriptor->properties.arrayProperties.dimension = nowDim;
    return;
}

void declareFunction(AST_NODE* declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    AST_NODE *paramListNode = idNode->rightSibling;
    AST_NODE *blockNode = paramListNode->rightSibling;
    DATA_TYPE dataType;
    checkTypeNode(typeNode, &dataType);
    char *typeName = typeNode->semantic_value.identifierSemanticValue.identifierName;
    /*
    SymbolTableEntry *typeEntry = retrieveSymbol(typeName);
    */
    SymbolTableEntry *typeEntry = typeNode->semantic_value.identifierSemanticValue.symbolTableEntry;
    if(typeEntry != NULL && typeEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
        dataType = ERROR_TYPE;
        printErrorMsgSpecial(declarationNode, typeName, RETURN_ARRAY);
    }
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    if(isDeclaredLocally(idName)){
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s %s\'", typeNode->semantic_value.identifierSemanticValue.identifierName, idName);
        printErrorMsgSpecial(idNode, errMsg, SYMBOL_REDECLARE);
    }
    else{
        typeNode->dataType = dataType;
        SymbolAttribute *symbolAttr = (SymbolAttribute *)malloc(sizeof(SymbolAttribute));
        symbolAttr->attributeKind = FUNCTION_SIGNATURE;
        symbolAttr->attr.functionSignature = (FunctionSignature *)malloc(sizeof(FunctionSignature));
        symbolAttr->attr.functionSignature->returnType = dataType;
        symbolAttr->attr.functionSignature->parametersCount = 0;
        symbolAttr->attr.functionSignature->parameterList = NULL;
        // enter function symbol first
        // construct the paramList in funcSignature and declare param at the same time
        idNode->semantic_value.identifierSemanticValue.symbolTableEntry = enterSymbol(idName, symbolAttr);
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
            enterSymbol(idName, symbolAttr);
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


void checkTypeNode(AST_NODE* typeNode, DATA_TYPE *dataType)
{
    SymbolTableEntry *typeEntry = retrieveSymbol(typeNode->semantic_value.identifierSemanticValue.identifierName);
    if(typeEntry == NULL || typeEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
        // error : unknown type or not a type
        char errMsg[ERR_MSG_LEN];
        sprintf(errMsg, "\'%s\' is not a type", typeNode->semantic_value.identifierSemanticValue.identifierName);
        printErrorMsgSpecial(typeNode, errMsg, SYMBOL_IS_NOT_TYPE);
        *dataType = ERROR_TYPE;
        typeNode->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    }
    else if(typeEntry->attribute->attr.typeDescriptor->kind == SCALAR_TYPE_DESCRIPTOR){
        // scalar type
        *dataType = typeEntry->attribute->attr.typeDescriptor->properties.dataType;
        typeNode->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    }
    else{
        // array type
        *dataType = typeEntry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;
        typeNode->semantic_value.identifierSemanticValue.symbolTableEntry = typeEntry;
    }
}

/*
void declareIdList(AST_NODE* declarationNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize)
{
}


void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode)
{
}
*/

void checkWhileStmt(AST_NODE* whileNode)
{
    AST_NODE *testExprRoot = whileNode->child;
    AST_NODE *stmtNode = testExprRoot->rightSibling;
    checkExprNode(testExprRoot);
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
        checkExprNode(relopExpr);
        relopExpr = relopExpr->rightSibling;
    }
    while(updateAssignExpr){
        checkAssignmentStmt(updateAssignExpr);
        updateAssignExpr = updateAssignExpr->rightSibling;
    }
    checkStmtNode(stmtNode);
    return;
}

void checkArrayReference(AST_NODE *idNode, ArrayProperties property, int isLvalue)
{
    int nowDimension = 0;
    AST_NODE *arrayDimension = idNode->child;
    while(arrayDimension){
        // constant index
        if(arrayDimension->nodeType == CONST_VALUE_NODE && arrayDimension->semantic_value.const1->const_type != INTEGERC){
            idNode->dataType = ERROR_TYPE;
            printErrorMsg(arrayDimension, ARRAY_SUBSCRIPT_NOT_INT);
        }
        // expression index
        else if(arrayDimension->nodeType == EXPR_NODE){
            checkExprNode(arrayDimension);
            if(arrayDimension->dataType == FLOAT_TYPE){
                // index not an integer error
                idNode->dataType = ERROR_TYPE;
                printErrorMsg(arrayDimension, ARRAY_SUBSCRIPT_NOT_INT);
            }
            else if(arrayDimension->dataType == ERROR_TYPE)
                idNode->dataType = ERROR_TYPE;
        }
        arrayDimension = arrayDimension->rightSibling;
        nowDimension++;
    }
    if(nowDimension < property.dimension){
        // assign to an array address error    
        idNode->dataType = ERROR_TYPE;
        if(idNode->parent->nodeType == NONEMPTY_RELOP_EXPR_LIST_NODE && \
           idNode->parent->parent->nodeType == STMT_NODE && \
           idNode->parent->parent->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
            // in parameter list
            return;
        }

        if(isLvalue){
            printErrorMsg(idNode, NOT_ASSIGNABLE);
        }
        else {
            printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
        }
    }
    else if(nowDimension > property.dimension){
        // dimension error
        idNode->dataType = ERROR_TYPE;
        printErrorMsg(idNode, INCOMPATIBLE_ARRAY_DIMENSION);
    }
    else
        idNode->dataType = property.elementType;
    return;
}

void checkAssignmentStmt(AST_NODE* assignmentNode)
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
        checkArrayReference(leftNode, property, 1);
    }
    // available, name is a scalar
    else{
        leftNode->dataType = leftNodeSymbol->attribute->attr.typeDescriptor->properties.dataType;
    }

    // check relop on RHS of assignment
    checkExprNode(rightNode);

    // type conversion
    if(leftNode->dataType == ERROR_TYPE || rightNode->dataType == ERROR_TYPE)
        assignmentNode->dataType = ERROR_TYPE;
    else
        assignmentNode->dataType = leftNode->dataType;
    return;
}


void checkIfStmt(AST_NODE* ifNode)
{
    AST_NODE *ifTest = ifNode->child;
    AST_NODE *stmtNode = ifTest->rightSibling;
    checkExprNode(ifTest);
    checkStmtNode(stmtNode);
}

void checkWriteFunction(AST_NODE* functionCallNode)
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

    checkExprNode(toWrite);
    // write a constant, should be string
    if(toWrite->nodeType == CONST_VALUE_NODE && toWrite->dataType != CONST_STRING_TYPE)
        printErrorMsg(functionCallNode, NOT_WRITABLE);
    // write an constant expression, invalid
    else if(toWrite->nodeType == EXPR_NODE && toWrite->semantic_value.exprSemanticValue.isConstEval == 1)
        printErrorMsg(functionCallNode, NOT_WRITABLE);
    // others: write identifier & write function call(handled in checkExprNode)
    return;
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
    AST_NODE *idNode = functionCallNode->child;
    AST_NODE *paramNode = idNode->rightSibling->child;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    // special case: write function
    if(strcmp(idName, "write") == 0){
        checkWriteFunction(functionCallNode);
        return;
    }

    SymbolTableEntry *idEntry = retrieveSymbol(idName);
    if(idEntry == NULL){
        // warning : implicit declaration
        functionCallNode->dataType = ERROR_TYPE;
        printErrorMsgSpecial(functionCallNode, idName, SYMBOL_UNDECLARED);
        return;
    }
    else if(idEntry->attribute->attributeKind != FUNCTION_SIGNATURE){
        // error : not callable
        functionCallNode->dataType = ERROR_TYPE;
        printErrorMsgSpecial(functionCallNode, idName, NOT_FUNCTION_NAME);
        return;
    }
    while(paramNode != NULL){
        checkExprNode(paramNode);
        paramNode = paramNode->rightSibling;
    }
    paramNode = idNode->rightSibling->child;
    checkParameterPassing(idEntry->attribute->attr.functionSignature->parameterList, paramNode, idNode);
    functionCallNode->dataType = idEntry->attribute->attr.functionSignature->returnType;
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter, AST_NODE *idNode)
{
    while(formalParameter != NULL && actualParameter != NULL){
        if(formalParameter->type->kind == SCALAR_TYPE_DESCRIPTOR){
            char errMsg[ERR_MSG_LEN];
            char *formalParameterType = formalParameter->type->properties.dataType == INT_TYPE ? "\'int\'" : "\'float\'";
            switch(actualParameter->dataType){
                case INT_TYPE: case FLOAT_TYPE:
                    // valid
                    break;
                case CONST_STRING_TYPE:
                    // error : passing char* to a scalar
                    sprintf(errMsg, "\'char *\' to %s", formalParameterType);
                    printErrorMsgSpecial(idNode, errMsg, PASS_ARRAY_TO_SCALAR);
                    break;
                case ERROR_TYPE:
                    if(actualParameter->nodeType == IDENTIFIER_NODE){
                        SymbolTableEntry *idEntry = retrieveSymbol(actualParameter->semantic_value.identifierSemanticValue.identifierName);
                        if(idEntry != NULL && idEntry->attribute->attributeKind == VARIABLE_ATTRIBUTE && \
                            idEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
                            ArrayProperties arrayProperties = idEntry->attribute->attr.typeDescriptor->properties.arrayProperties;
                            AST_NODE *arrayDimension = actualParameter->child;
                            int cnt = 0;
                            while(arrayDimension != NULL){
                                cnt += 1;
                                arrayDimension = arrayDimension->rightSibling;
                            }
                            if(cnt < arrayProperties.dimension){
                                char *actualParameterType = arrayProperties.elementType == INT_TYPE ? "\'int (*)" : "\'float (*)";
                                strcpy(errMsg, actualParameterType);
                                cnt += 1; // it's a pointer
                                while(cnt < arrayProperties.dimension){
                                    char dimensionSize[MSG_LEN];
                                    sprintf(dimensionSize, "[%d]", arrayProperties.sizeInEachDimension[cnt]);
                                    strcat(errMsg, dimensionSize);
                                    cnt += 1;
                                }
                                strcat(errMsg, "\' to ");
                                strcat(errMsg, formalParameterType);
                                printErrorMsgSpecial(idNode, errMsg, PASS_ARRAY_TO_SCALAR);
                            }
                        }
                    }
                    break;
            }
        }
        else{
            char errMsg[ERR_MSG_LEN];
            char formalParameterType[MSG_LEN];
            ArrayProperties formalArrayProperties = formalParameter->type->properties.arrayProperties;
            strcpy(formalParameterType, formalArrayProperties.elementType == INT_TYPE ? "\'int (*)" : "\'float(*)");
            int formalDimCnt = 1;
            while(formalDimCnt < formalArrayProperties.dimension){
                char dimensionSize[MSG_LEN];
                sprintf(dimensionSize, "[%d]", formalArrayProperties.sizeInEachDimension[formalDimCnt]);
                strcat(formalParameterType, dimensionSize);
                formalDimCnt += 1;
            }
            strcat(formalParameterType, "\'");
            switch (actualParameter->dataType){
                case INT_TYPE:
                    sprintf(errMsg, "\'int\' to %s", formalParameterType);
                    printErrorMsgSpecial(idNode, errMsg, PASS_SCALAR_TO_ARRAY);
                    break;
                case FLOAT_TYPE:
                    sprintf(errMsg, "\'float\' to %s", formalParameterType);
                    printErrorMsgSpecial(idNode, errMsg, PASS_SCALAR_TO_ARRAY);
                    break;
                case CONST_STRING_TYPE:
                    // erro : passing char* to int* or float*
                    break;
                case ERROR_TYPE:
                    if(actualParameter->nodeType == IDENTIFIER_NODE){
                        SymbolTableEntry *idEntry = retrieveSymbol(actualParameter->semantic_value.identifierSemanticValue.identifierName);
                        if(idEntry != NULL && idEntry->attribute->attributeKind == VARIABLE_ATTRIBUTE && \
                            idEntry->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
                            ArrayProperties actualArrayProperties = idEntry->attribute->attr.typeDescriptor->properties.arrayProperties;
                            AST_NODE *actualArrayDimension = actualParameter->child;
                            int actualDimCnt = 0;
                            while(actualArrayDimension != NULL){
                                actualDimCnt += 1;
                                actualArrayDimension = actualArrayDimension->rightSibling;
                            }
                            if(actualDimCnt < actualArrayProperties.dimension){
                                char actualParameterType[MSG_LEN];
                                strcpy(actualParameterType, actualArrayProperties.elementType == INT_TYPE ? "\'int (*)" : "\'float (*)");
                                int dimNotMatch = 0;
                                actualDimCnt += 1; // it's a pointer
                                formalDimCnt = 1;
                                while(actualDimCnt < actualArrayProperties.dimension){
                                    char dimensionSize[MSG_LEN];
                                    sprintf(dimensionSize, "[%d]", actualArrayProperties.sizeInEachDimension[actualDimCnt]);
                                    strcat(actualParameterType, dimensionSize);
                                    if(formalDimCnt < formalArrayProperties.dimension){
                                        dimNotMatch = (actualArrayProperties.sizeInEachDimension[actualDimCnt] != formalArrayProperties.sizeInEachDimension[formalDimCnt]);
                                    }
                                    actualDimCnt += 1;
                                    formalDimCnt += 1;
                                }
                                strcat(actualParameterType, "\'");
                                if(dimNotMatch){
                                    sprintf(errMsg, "%s to %s", actualParameterType, formalParameterType);
                                    printErrorMsgSpecial(idNode, errMsg, PASS_INCOMPATIBLE_DIMENSION);
                                }
                            }
                        }
                    }
                    break;
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


void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, double* fValue)
{
    if(exprOrConstNode->nodeType == CONST_VALUE_NODE){
        if(exprOrConstNode->semantic_value.const1->const_type == INTEGERC){
            iValue = &(exprOrConstNode->semantic_value.const1->const_u.intval);
        }
        else{
            fValue = &(exprOrConstNode->semantic_value.const1->const_u.fval);
        }
    }
    else if(exprOrConstNode->semantic_value.exprSemanticValue.isConstEval == 1){
        if(exprOrConstNode->dataType == INT_TYPE){
            iValue = &(exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
        }
        else{
            fValue = &(exprOrConstNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
        }
    }
    return;
}



void evaluateExprValue(AST_NODE* exprNode)
{
    AST_NODE *leftNode = exprNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    // for unary operation
    if(rightNode == NULL){
       if(leftNode->nodeType == CONST_VALUE_NODE || leftNode->semantic_value.exprSemanticValue.isConstEval == 1){
            int *livalue = NULL;
            double *lfvalue = NULL;
            exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
            getExprOrConstValue(leftNode, livalue, lfvalue);
            if(livalue){
                if(leftNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION)
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (*livalue == 0);
                else if(leftNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_NEGATIVE)
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = -(*livalue);
                else
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = *livalue;
                exprNode->dataType = INT_TYPE;
            }
            else if(lfvalue){
                if(leftNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION){
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = (*lfvalue == 0);
                    exprNode->dataType = INT_TYPE;
                }
                else if(leftNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_NEGATIVE){
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = -(*lfvalue);
                    exprNode->dataType = FLOAT_TYPE;
                }
                else{
                    exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue;
                    exprNode->dataType = FLOAT_TYPE;
                }
            }
        }
        else{
            exprNode->semantic_value.exprSemanticValue.isConstEval = 0;
            if(leftNode->dataType == ERROR_TYPE)
                exprNode->dataType = ERROR_TYPE;
            else if(leftNode->semantic_value.exprSemanticValue.op.unaryOp == UNARY_OP_LOGICAL_NEGATION)
                exprNode->dataType = INT_TYPE;
            else
                exprNode->dataType = leftNode->dataType;
        }
    }
    // binary arithmetic opeartion, both side are constant evaluations
    else if((leftNode->nodeType == CONST_VALUE_NODE || leftNode->semantic_value.exprSemanticValue.isConstEval == 1) && 
            (rightNode->nodeType == CONST_VALUE_NODE || rightNode->semantic_value.exprSemanticValue.isConstEval == 1)){
        int *livalue = NULL;
        int *rivalue = NULL;
        double *lfvalue = NULL;
        double *rfvalue = NULL;
        exprNode->semantic_value.exprSemanticValue.isConstEval = 1;
        getExprOrConstValue(leftNode, livalue, lfvalue);
        getExprOrConstValue(rightNode, rivalue, rfvalue);
        // both integer
        if(livalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = *livalue + *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = *livalue - *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = *livalue * *rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue = *livalue / *rivalue;
            exprNode->dataType = INT_TYPE;
        }
        // left integer, right float
        else if(livalue && rfvalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = (double)*livalue + *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = (double)*livalue - *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = (double)*livalue * *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = (double)*livalue / *rfvalue;
            exprNode->dataType = FLOAT_TYPE;
        }
        // left float, right integer
        else if(lfvalue && rivalue){
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue + (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue - (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue * (double)*rivalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue / (double)*rivalue;
            exprNode->dataType = FLOAT_TYPE;
        }
        // both float
        else{
            if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_ADD)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue + *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_SUB)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue - *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_MUL)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue * *rfvalue;
            else if(exprNode->semantic_value.exprSemanticValue.op.binaryOp == BINARY_OP_DIV)
                exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue = *lfvalue / *rfvalue;
            exprNode->dataType = FLOAT_TYPE;
        }
    }
    else{
        exprNode->semantic_value.exprSemanticValue.isConstEval = 0;
        if(leftNode->dataType == ERROR_TYPE || rightNode->dataType == ERROR_TYPE)
            exprNode->dataType = ERROR_TYPE;
        else
            exprNode->dataType = getBiggerType(leftNode->dataType, rightNode->dataType);
    }
    return;
}


void checkExprNode(AST_NODE* exprNode)
{
    /* this function should put dataType in AST_NODE */
    // constant node
    if(exprNode->nodeType == NUL_NODE) return;
    if(exprNode->nodeType == CONST_VALUE_NODE){
        if(exprNode->semantic_value.const1->const_type == INTEGERC)
            exprNode->dataType = INT_TYPE;
        else if(exprNode->semantic_value.const1->const_type == FLOATC)
            exprNode->dataType = FLOAT_TYPE;
        else
            exprNode->dataType = CONST_STRING_TYPE;
        return;
    }
    // function call node
    else if(exprNode->nodeType == STMT_NODE && \
            exprNode->semantic_value.stmtSemanticValue.kind == FUNCTION_CALL_STMT){
        checkFunctionCall(exprNode);
        if(exprNode->dataType == VOID_TYPE){
            printErrorMsgSpecial(exprNode, exprNode->child->semantic_value.identifierSemanticValue.identifierName, INVALID_OPERAND);
            exprNode->dataType = ERROR_TYPE;
        }
        return;
    }
    // identifier node
    else if(exprNode->nodeType == IDENTIFIER_NODE){
        char *identifierName = exprNode->semantic_value.identifierSemanticValue.identifierName;
        SymbolTableEntry *identifier = retrieveSymbol(identifierName);
        // identifier not available
        if(identifier == NULL){
            // error
            exprNode->dataType = ERROR_TYPE;
            printErrorMsgSpecial(exprNode, identifierName, SYMBOL_UNDECLARED);
        }
        // identifier is a function
        else if(identifier->attribute->attributeKind == FUNCTION_SIGNATURE){
            // error
            exprNode->dataType = ERROR_TYPE;
            printErrorMsgSpecial(exprNode, identifierName, IS_FUNCTION_NOT_VARIABLE);
        }
        // identifier is a typedef
        else if(identifier->attribute->attributeKind == TYPE_ATTRIBUTE){
            // error
            exprNode->dataType = ERROR_TYPE;
            printErrorMsgSpecial(exprNode, identifierName, IS_TYPE_NOT_VARIABLE);
        }
        // identifier is an array
        else if(identifier->attribute->attr.typeDescriptor->kind == ARRAY_TYPE_DESCRIPTOR){
            ArrayProperties property = identifier->attribute->attr.typeDescriptor->properties.arrayProperties;
            checkArrayReference(exprNode, property, 0);
        }
        // identifier is a scalar
        else
            exprNode->dataType = identifier->attribute->attr.typeDescriptor->properties.dataType;
        return;
    }
    // nonterminal nodes
    AST_NODE *leftNode = exprNode->child;
    AST_NODE *rightNode = leftNode->rightSibling;
    // relative operation, binary, e.g, a > 0, (a > 0 || b < 0)
    if(isRelativeOperation(exprNode)){
        checkExprNode(leftNode);
        checkExprNode(rightNode);
        // value of relative expression: 0 or 1(integer)
        if(leftNode->dataType == CONST_STRING_TYPE || rightNode->dataType == CONST_STRING_TYPE){
            exprNode->dataType = ERROR_TYPE;
            printErrorMsg(exprNode, STRING_OPERATION);
        }
        else if(leftNode->dataType == ERROR_TYPE || rightNode->dataType == ERROR_TYPE)
            exprNode->dataType = ERROR_TYPE;
        else
            exprNode->dataType = INT_TYPE;
    }
    // unary operation
    else if(exprNode->semantic_value.exprSemanticValue.kind == UNARY_OPERATION){
        checkExprNode(leftNode);
        if(leftNode->dataType == CONST_STRING_TYPE){
            exprNode->dataType = ERROR_TYPE;
            printErrorMsg(exprNode, STRING_OPERATION);
        }
        else if(leftNode->dataType != ERROR_TYPE)
            evaluateExprValue(exprNode);
        else
            exprNode->dataType = ERROR_TYPE;
    }
    // binary arithmetic operation, e.g, +, -, *, /
    else{
        checkExprNode(leftNode);
        checkExprNode(rightNode);
        if(leftNode->dataType == CONST_STRING_TYPE || rightNode->dataType == CONST_STRING_TYPE){
            exprNode->dataType = ERROR_TYPE;
            printErrorMsg(exprNode, STRING_OPERATION);
        }
        else if(leftNode->dataType != ERROR_TYPE && rightNode->dataType != ERROR_TYPE)
            evaluateExprValue(exprNode);
        else
            exprNode->dataType = ERROR_TYPE;
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

void checkReturnStmt(AST_NODE* returnNode)
{
    AST_NODE *returnItem = returnNode->child;
    AST_NODE *parentNode = returnNode->parent;
    while((parentNode->nodeType != DECLARATION_NODE) || (parentNode->semantic_value.declSemanticValue.kind != FUNCTION_DECL)){
        parentNode = parentNode->parent;
    }
    AST_NODE *typeNode = parentNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    char *functionName = idNode->semantic_value.identifierSemanticValue.identifierName;
    checkExprNode(returnItem);
    if(typeNode->dataType != ERROR_TYPE){
        if(typeNode->dataType == VOID_TYPE && returnItem->nodeType != NUL_NODE){
            printErrorMsgSpecial(returnNode, functionName, RETURN_IN_VOID_FUNCTION);
        }
        else if(typeNode->dataType != VOID_TYPE && returnItem->nodeType == NUL_NODE){
            // warning : return value in non-void function
        }
    }
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
    switch(stmtNode->nodeType){
        case BLOCK_NODE:
            openScope();
            checkBlockNode(stmtNode);
            closeScope();
            break;
        case NUL_NODE:
            break;
        default:
            switch(stmtNode->semantic_value.stmtSemanticValue.kind){
                case IF_STMT:
                    openScope();
                    checkIfStmt(stmtNode);
                    closeScope();
                    break;
                case WHILE_STMT:
                    openScope();
                    checkWhileStmt(stmtNode);
                    closeScope();
                    break;
                case FOR_STMT:
                    openScope();
                    checkForStmt(stmtNode);
                    closeScope();
                    break;
                case RETURN_STMT:
                    checkReturnStmt(stmtNode);
                    break;
                case FUNCTION_CALL_STMT:
                    checkFunctionCall(stmtNode);
                    break;
                case ASSIGN_STMT:
                    checkAssignmentStmt(stmtNode);
                    break;
            }
            break;
    }

}

/*
void checkGeneralNode(AST_NODE *node)
{
}

void checkDeclDimList(AST_NODE* idNode, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize)
{
}
*/
