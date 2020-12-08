#ifndef __SEMANTIC_ANALYSIS_H__
#define  __SEMANTIC_ANALYSIS_H__

#include "header.h"
#include "symbolTable.h"
#define ERR_MSG_LEN 128
#define MSG_LEN 128

DATA_TYPE getBiggerType(DATA_TYPE dataType1, DATA_TYPE dataType2);
void checkProgramNode(AST_NODE *programNode);
void initType();
void checkDeclarationNode(AST_NODE* declarationNode);
void declareIdList(AST_NODE* typeNode, SymbolAttributeKind isVariableOrTypeAttribute, int ignoreArrayFirstDimSize);
void declareType(AST_NODE *declarationNode);
void declareVariable(AST_NODE *declaratioNode);
void declareFunction(AST_NODE* returnTypeNode);
void declareFunctionParam(AST_NODE *declarationNode, FunctionSignature *functionSignature);
void getArrayDimensionAndSize(SymbolAttribute *symbolAttr, AST_NODE *idNode, int ignoreFirstDimSize);
void checkDeclDimList(AST_NODE* variableDeclDimList, TypeDescriptor* typeDescriptor, int ignoreFirstDimSize);
void checkTypeNode(AST_NODE* typeNode, DATA_TYPE *dataType);
void checkBlockNode(AST_NODE* blockNode);
void checkStmtNode(AST_NODE* stmtNode);
void checkGeneralNode(AST_NODE *node);
void checkAssignOrExpr(AST_NODE* assignOrExprRelatedNode);
void checkWhileStmt(AST_NODE* whileNode);
void checkForStmt(AST_NODE* forNode);
void checkAssignmentStmt(AST_NODE* assignmentNode);
void checkIfStmt(AST_NODE* ifNode);
void checkWriteFunction(AST_NODE* functionCallNode);
void checkFunctionCall(AST_NODE* functionCallNode);
void checkParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter, AST_NODE *idNode);
void checkReturnStmt(AST_NODE* returnNode);
void checkExprNode(AST_NODE* exprNode);
void checkVariableLValue(AST_NODE* idNode);
void checkVariableRValue(AST_NODE* idNode);
void checkConstValueNode(AST_NODE* constValueNode);
void getExprOrConstValue(AST_NODE* exprOrConstNode, int* iValue, double* fValue);
void evaluateExprValue(AST_NODE* exprNode);
int  isRelativeOperation(AST_NODE* exprRelatedNode);
void checkArrayReference(AST_NODE* arrayReference, ArrayProperties property, int isLvalue);
void initFunction();


#endif
