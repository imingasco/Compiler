#ifndef __CODE_GEN_H__
#define  __CODE_GEN_H__

#include "header.h"
#include "symbolTable.h"
#define ERR_MSG_LEN 128
#define MSG_LEN 128

int  get_t_reg();
void free_t_reg(int t_reg_num);
int  get_ft_reg();
void free_ft_reg(int ft_reg_num);
void genProgramNode(AST_NODE *programNode);
void genDeclarationNode(AST_NODE* declarationNode);
void genDeclareVariable(AST_NODE *declaratioNode);
void genDeclareFunction(AST_NODE* returnTypeNode);
void declareFunctionParam(AST_NODE *declarationNode, FunctionSignature *functionSignature);
void genBlockNode(AST_NODE* blockNode);
void genStmtNode(AST_NODE* stmtNode);
void genWhileStmt(AST_NODE* whileNode);
void genForStmt(AST_NODE* forNode);
void genAssignmentStmt(AST_NODE* assignmentNode);
void genIfStmt(AST_NODE* ifNode);
void genWriteFunction(AST_NODE* functionCallNode);
void genFunctionCall(AST_NODE* functionCallNode);
void genParameterPassing(Parameter* formalParameter, AST_NODE* actualParameter, AST_NODE *idNode);
void genReturnStmt(AST_NODE* returnNode);
void genExprNode(AST_NODE* exprNode);
void getFormalParameterType(Parameter *formalParameter, char *formalParameterType);
void getActualParameterType(AST_NODE *actualParameter, char *actualParameterType);
int  getArraySpace(SymbolTableEntry *idEntry);
int  isGlobal(SymbolTableEntry *entry);
void loadConst(int constVal, int reg_num);
void loadFloat(float constVal, int reg_num);
void prologue(char *functionName);
void epilogue(char *functionName);
void genArrayElement(AST_NODE *idNode, int offset_reg);

#endif
