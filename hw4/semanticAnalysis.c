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
        case FUNCTION_PARAMETER_DECL:
            declareFunctionParam(declarationNode);
            break;
    }
}

void declareType(AST_NODE *declarationNode){
    AST_NODE *typeNode = declarationNode->child;
    AST_NODE *idNode = typeNode->rightSibling;
    SymbolTableEntry *typeEntry = retrieveSymbol(typeNode->semantic_value.identifierSemanticValue.identifierName);
    DATA_TYPE dataType;
    if(typeEntry == NULL){
        // error :  unknown type name
        dataType = ERROR_TYPE;
    }
    else if(typeEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
        // error : not a type
        dataType = ERROR_TYPE;
    }
    else{
        // ignore the ARRAY_TYPE ( which is defined as array e.g. typedef int II[2])
        dataType = typeEntry->attribute->attr.typeDescriptor->properties.dataType;
    }
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
    SymbolTableEntry *typeEntry = retrieveSymbol(typeNode->semantic_value.identifierSemanticValue.identifierName);
    DATA_TYPE dataType;
    if(typeEntry == NULL){
        // error :  unknown type name
        dataType = ERROR_TYPE;
    }
    else if(typeEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
        // error : not a type
        dataType = ERROR_TYPE;
    }
    else{
        // ignore the ARRAY_TYPE ( which is defined as array e.g. typedef int II[2])
        dataType = typeEntry->attribute->attr.typeDescriptor->properties.dataType;
    }
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
                    getArrayDimensionAndSize(symbolAttr, idNode, ignoreFirstDimSize);
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
    AST_NODE *paramNode = idNode->rightSibling;
    AST_NODE *blockNode = paramNode->rightSibling;
    DATA_TYPE dataType;
    char *idName = idNode->semantic_value.identifierSemanticValue.identifierName;
    SymbolTableEntry *typeEntry = retrieveSymbol(typeNode->semantic_value.identifierSemanticValue.identifierName);
    if(typeEntry == NULL){
        // error :  unknown type name
        dataType = ERROR_TYPE;
    }
    else if(typeEntry->attribute->attributeKind != TYPE_ATTRIBUTE){
        // error : not a type
        dataType = ERROR_TYPE;
    }
    else{
        // ignore the ARRAY_TYPE ( which is defined as array e.g. typedef int II[2])
        dataType = typeEntry->attribute->attr.typeDescriptor->properties.dataType;
    }
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

    }
}

void declareFunctionParam(AST_NODE *declarationNode){

}


void checkTypeNode(AST_NODE* idNodeAsType)
{
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
    checkExprRelatedNode
    checkStmtNode(stmtNode);
}


void checkForStmt(AST_NODE* forNode)
{
}


void checkAssignmentStmt(AST_NODE* assignmentNode)
{
}


void checkIfStmt(AST_NODE* ifNode)
{
}

void checkWriteFunction(AST_NODE* functionCallNode)
{
}

void checkFunctionCall(AST_NODE* functionCallNode)
{
}

void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter)
{
}


void checkExprRelatedNode(AST_NODE* exprRelatedNode)
{
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



