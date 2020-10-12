#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "header.h"


int main( int argc, char *argv[] )
{
    FILE *source, *target;
    Program program;
    SymbolTable symtab;
    char debugger[1024] = "";

    if( argc == 3){
        source = fopen(argv[1], "r");
        target = fopen(argv[2], "w");
        if( !source ){
            printf("can't open the source file\n");
            exit(2);
        }
        else if( !target ){
            printf("can't open the target file\n");
            exit(2);
        }
        else{
            program = parser(source);
            fclose(source);
            symtab = build(program);
            //TraverseTable(&symtab, debugger);
            check(&program, &symtab);
            ConstantFold(&program);
            gencode(program, target, &symtab);
        }
    }
    else{
        printf("Usage: %s source_file target_file\n", argv[0]);
    }


    return 0;
}


/********************************************* 
  Scanning 
 *********************************************/
void PutTokenBack( FILE *source, char *token){
    int i;
    int len = strlen(token);
    for(i = len - 1; i >= 0; i--)
        ungetc(token[i], source);
}

Token getNumericToken( FILE *source, char c )
{
    Token token;
    int i = 0;

    while( isdigit(c) ) {
        token.tok[i++] = c;
        c = fgetc(source);
    }

    if( c != '.' ){
        ungetc(c, source);
        token.tok[i] = '\0';
        token.type = IntValue;
        return token;
    }

    token.tok[i++] = '.';

    c = fgetc(source);
    if( !isdigit(c) ){
        ungetc(c, source);
        printf("Expect a digit : %c\n", c);
        exit(1);
    }

    while( isdigit(c) ){
        token.tok[i++] = c;
        c = fgetc(source);
    }

    ungetc(c, source);
    token.tok[i] = '\0';
    token.type = FloatValue;
    return token;
}

Token scanner( FILE *source )
{
    char c;
    int len = 0;
    Token token;

    while( !feof(source) ){
        c = fgetc(source);
        //printf("[scanner] first read %c\n", c);

        while( isspace(c) ) c = fgetc(source);

        if( isdigit(c) )
            return getNumericToken(source, c);

        token.tok[len++] = c;
        token.tok[len] = '\0';
        //printf("[scanner] first token.tok = %s\n", token.tok);
        // token.tok[1] = '\0';
        if( islower(c) ){
            c = fgetc(source);
            //printf("[scanner] then read %c\n", c);
            while( !isspace(c) ){
                token.tok[len++] = c;
                c = fgetc(source);
                if( !islower(c) && !isspace(c) )
                    printf("invalid character for variable name: %c\n", c);
                //printf("[scanner] then read %c\n", c);
            }
            ungetc(c, source);
            token.tok[len] = '\0';
            //printf("[scanner] then token.tok = %s\n", token.tok);
            if( strcmp(token.tok, "f") == 0 )
                token.type = FloatDeclaration;
            else if( strcmp(token.tok, "i" ) == 0 )
                token.type = IntegerDeclaration;
            else if( strcmp(token.tok, "p") == 0 )
                token.type = PrintOp;
            else
                token.type = Alphabet;
            //printf("[scanner] token.tok = %s\n", token.tok);
            return token;
        }

        switch(c){
            case '=':
                token.type = AssignmentOp;
                return token;
            case '+':
                token.type = PlusOp;
                return token;
            case '-':
                token.type = MinusOp;
                return token;
            case '*':
                token.type = MulOp;
                return token;
            case '/':
                token.type = DivOp;
                return token;
            case '(':
                token.type = LeftPar;
                return token;
            case ')':
                token.type = RightPar;
                return token;
            case EOF:
                token.type = EOFsymbol;
                token.tok[0] = '\0';
                return token;
            default:
                printf("Invalid character : %c\n", c);
                exit(1);
        }
    }

    token.tok[0] = '\0';
    token.type = EOFsymbol;
    return token;
}


/********************************************************
  Parsing
 *********************************************************/
Declaration parseDeclaration( FILE *source, Token token )
{
    Token token2;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            token2 = scanner(source);
            if (strcmp(token2.tok, "f") == 0 ||
                strcmp(token2.tok, "i") == 0 ||
                strcmp(token2.tok, "p") == 0) {
                printf("Syntax Error: %s cannot be used as id\n", token2.tok);
                exit(1);
            }
            return makeDeclarationNode( token, token2 );
        default:
            printf("Syntax Error: Expect Declaration %s\n", token.tok);
            exit(1);
    }
}

Declarations *parseDeclarations( FILE *source )
{
    int i = 0;
    int len = 0;
    Token token = scanner(source);
    Declaration decl;
    Declarations *decls;
    switch(token.type){
        case FloatDeclaration:
        case IntegerDeclaration:
            decl = parseDeclaration(source, token);
            decls = parseDeclarations(source);
            return makeDeclarationTree( decl, decls );
        case PrintOp:
        case Alphabet:
            PutTokenBack(source, token.tok);
            return NULL;
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect declarations %s\n", token.tok);
            exit(1);
    }
}

Expression *parseValue( FILE *source )
{
    Token token = scanner(source);
    //printf("[parseValue] token.tok = %s\n", token.tok);
    Expression *value = (Expression *)malloc( sizeof(Expression) );
    value->leftOperand = value->rightOperand = NULL;
    Expression *parValue, *parExpr;

    switch(token.type){
        case Alphabet:
            (value->v).type = Identifier;
            strcpy((value->v).val.id, token.tok);
            //printf("id = %s\n", (value->v).val.id);
            break;
        case IntValue:
            (value->v).type = IntConst;
            (value->v).val.ivalue = atoi(token.tok);
            //printf("ival = %d\n", (value->v).val.ivalue);
            break;
        case FloatValue:
            (value->v).type = FloatConst;
            (value->v).val.fvalue = atof(token.tok);
            //printf("fval = %f\n", (value->v).val.fvalue);
            break;
        case LeftPar:
            parValue = parseValue(source);
            parExpr = parseExpression(source, parValue);
            Token tokenCheck = scanner(source);
            if(tokenCheck.type != RightPar){
                printf("Syntax Error: Expect a Right Parenthese %s\n", token.tok);
                exit(1);
            }
            return parExpr;
        default:
            printf("Syntax Error: Expect Identifier, a Number, or a Left Parenthese %s\n", token.tok);
            exit(1);
    }

    return value;
}

Expression *parseExpression( FILE *source, Expression *lvalue )
{
    int i, len;
    Token token = scanner(source);
    Expression *expr;
    //printf("[parseExpression] token.tok = %s\n", token.tok);

    switch(token.type){
        case PlusOp:
            if(lvalue == NULL){
                PutTokenBack(source, token.tok);
                return NULL;
            }
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = PlusNode;
            (expr->v).val.op = Plus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseExpression(source, expr);
        case MinusOp:
            if(lvalue == NULL){
                PutTokenBack(source, token.tok);
                return NULL;
            }
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MinusNode;
            (expr->v).val.op = Minus;
            expr->leftOperand = lvalue;
            expr->rightOperand = parseValue(source);
            return parseExpression(source, expr);
        case MulOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = MulNode;
            (expr->v).val.op = Mul;
            if(lvalue == NULL){
                expr->leftOperand = lvalue;
                expr->rightOperand = parseValue(source);
                return expr;
            }
            else if(lvalue->rightOperand == NULL){
                expr->leftOperand = lvalue;
                expr->rightOperand = parseValue(source);
                return parseExpression(source, expr);
            }
            else{
                expr->leftOperand = lvalue->rightOperand;
                expr->rightOperand = parseValue(source);
                lvalue->rightOperand = expr;
                return parseExpression(source, lvalue);
            }
        case DivOp:
            expr = (Expression *)malloc( sizeof(Expression) );
            (expr->v).type = DivNode;
            (expr->v).val.op = Div;
            if(lvalue == NULL){
                expr->leftOperand = lvalue;
                expr->rightOperand = parseValue(source);
                return expr;
            }
            else if(lvalue->rightOperand == NULL){
                expr->leftOperand = lvalue;
                expr->rightOperand = parseValue(source);
                return parseExpression(source, expr);
            }
            else{
                expr->leftOperand = lvalue->rightOperand;
                expr->rightOperand = parseValue(source);
                lvalue->rightOperand = expr;
                return parseExpression(source, lvalue);
            }
        case RightPar:
            if(lvalue == NULL){
                PutTokenBack(source, token.tok);
                return NULL;
            }
            expr = parseExpression(source, NULL);
            PutTokenBack(source, token.tok);
            if(expr == NULL){
                return lvalue;
            }
            else{
                expr->leftOperand = lvalue;
                return expr;
            }
        case Alphabet:
        case PrintOp:
            PutTokenBack(source, token.tok);
            return lvalue;
        case EOFsymbol:
            return lvalue;
        default:
            printf("Syntax Error: Expect a numeric value or an identifier %s\n", token.tok);
            exit(1);
    }
}

Statement parseStatement( FILE *source, Token token )
{
    Token next_token;
    Expression *value, *expr;
    //printf("token.tok = %s\n", token.tok);
    switch(token.type){
        case Alphabet:
            next_token = scanner(source);
            if(next_token.type == AssignmentOp){
                //printf(" = ");
                value = parseValue(source);
                /*
                if(value->v.type == IntConst)
                    printf("[parseStatement] ivalue = %d\n", value->v.val.ivalue);
                else if(value->v.type == FloatConst)
                    printf("[parseStatement] fvalue = %f\n", value->v.val.fvalue);
                */
                expr = parseExpression(source, value);
                return makeAssignmentNode(token.tok, expr);
            }
            else{
                printf("Syntax Error: Expect an assignment op %s\n", next_token.tok);
                exit(1);
            }
        case PrintOp:
            next_token = scanner(source);
            if(next_token.type == Alphabet)
                return makePrintNode(next_token.tok);
            else{
                printf("Syntax Error: Expect an identifier %s\n", next_token.tok);
                exit(1);
            }
            break;
        default:
            printf("Syntax Error: Expect a statement %s\n", token.tok);
            exit(1);
    }
}

Statements *parseStatements( FILE * source )
{

    Token token = scanner(source);
    //printf("[parseStatements] token.tok = %s\n", token.tok);
    Statement stmt;
    Statements *stmts;

    switch(token.type){
        case Alphabet:
        case PrintOp:
            stmt = parseStatement(source, token);
            stmts = parseStatements(source);
            return makeStatementTree(stmt , stmts);
        case EOFsymbol:
            return NULL;
        default:
            printf("Syntax Error: Expect statements %s\n", token.tok);
            exit(1);
    }
}


/*********************************************************************
  Build AST
 **********************************************************************/
Declaration makeDeclarationNode( Token declare_type, Token identifier )
{
    Declaration tree_node;

    switch(declare_type.type){
        case FloatDeclaration:
            tree_node.type = Float;
            break;
        case IntegerDeclaration:
            tree_node.type = Int;
            break;
        default:
            break;
    }
    strcpy(tree_node.name, identifier.tok);

    return tree_node;
}

Declarations *makeDeclarationTree( Declaration decl, Declarations *decls )
{
    Declarations *new_tree = (Declarations *)malloc( sizeof(Declarations) );
    new_tree->first = decl;
    new_tree->rest = decls;

    return new_tree;
}


Statement makeAssignmentNode( char *id, Expression *expr)
{
    Statement stmt;
    AssignmentStatement assign;

    stmt.type = Assignment;
    //printf("read id = %s\n", id);
    strcpy(assign.id, id);
    assign.expr = expr;
    stmt.stmt.assign = assign;

    return stmt;
}

Statement makePrintNode( char *id )
{
    Statement stmt;
    stmt.type = Print;
    strcpy(stmt.stmt.variable, id);

    return stmt;
}

Statements *makeStatementTree( Statement stmt, Statements *stmts )
{
    Statements *new_tree = (Statements *)malloc( sizeof(Statements) );
    new_tree->first = stmt;
    new_tree->rest = stmts;

    return new_tree;
}

/* parser */
Program parser( FILE *source )
{
    Program program;

    program.declarations = parseDeclarations(source);
    program.statements = parseStatements(source);

    return program;
}


/********************************************************
  Build symbol table
 *********************************************************/
void TraverseTable( SymbolTable *table, char *current ){
    int i;
    char buf[1024];
    int len = strlen(current);
    printf("now len = %d\n", len);
    strncpy(buf, current, len);
    if(table->end)
        printf("in buf: %s\n", buf);
    for(i = 0; i < 26; i++){
        if(table->next[i]){
            buf[len] = (char)('a' + i);
            buf[len + 1] = '\0';
            TraverseTable(table->next[i], buf);
        }    
    }
    
}
void InitializeTable( SymbolTable *table )
{
    int i;
    table->type = Notype;
    for ( i = 0; i < 26; i++ )
        table->next[i] = NULL;
    table->end = 0;
    table->index = -1;
}

void add_table( SymbolTable *table, char *c, DataType t, int nowindex )
{
    int i, index;
    int len = strlen(c);
    //printf("c = %s, len = %d\n", c, len);
    SymbolTable *current = table;

    for( i = 0; i < len; i++){
        index = (int)(c[i] - 'a');
        //printf("now char is %c, index is %d\n", c[i], index);
        if(current->next[index] == NULL){
            current->next[index] = (SymbolTable *)malloc(sizeof(SymbolTable));
            InitializeTable(current->next[index]);
        }
        current = current->next[index];
    }

    if(current->end || current->type != Notype)
        printf("Error : id %s has been declared\n", c);//error
    current->end = 1;
    current->type = t;
    current->index = nowindex;
}

SymbolTable build( Program program )
{
    SymbolTable table;
    Declarations *decls = program.declarations;
    Declaration current;
    int index = 0;

    InitializeTable(&table);

    while(decls != NULL){
        current = decls->first;
        add_table(&table, current.name, current.type, index++);
        decls = decls->rest;
    }

    return table;
}


/********************************************************************
  Type checking
 *********************************************************************/

void convertType( Expression * old, DataType type )
{
    if(old->type == Float && type == Int){
        printf("error : can't convert float to integer\n");
        return;
    }
    if(old->type == Int && type == Float){
        Expression *tmp = (Expression *)malloc( sizeof(Expression) );
        if(old->v.type == Identifier)
            printf("convert to float %s \n",old->v.val.id);
        else
            printf("convert to float %d \n", old->v.val.ivalue);
        tmp->v = old->v;
        tmp->leftOperand = old->leftOperand;
        tmp->rightOperand = old->rightOperand;
        tmp->type = old->type;

        Value v;
        v.type = IntToFloatConvertNode;
        v.val.op = IntToFloatConvert;
        old->v = v;
        old->type = Int;
        old->leftOperand = tmp;
        old->rightOperand = NULL;
    }
}

DataType generalize( Expression *left, Expression *right )
{
    if(left->type == Float || right->type == Float){
        printf("generalize : float\n");
        return Float;
    }
    printf("generalize : int\n");
    return Int;
}

DataType lookup_table( SymbolTable *table, char *c )
{
    int i, id;
    int len = strlen(c);
    SymbolTable *current = table;
    for( i = 0; i < len; i++ ){
        if(current == NULL){
            printf("Error: variable %s is not declared\n", c);
            exit(1);
        }
        id = (int)(c[i] - 'a');
        current = current->next[id];
    }
    if(!current->end){
        printf("Error: variable %s is not declared\n", c);
        //exit(1);
    }
    return current->type;
}

void checkexpression( Expression * expr, SymbolTable * table )
{
    char c[64];
    if(expr->leftOperand == NULL && expr->rightOperand == NULL){
        switch(expr->v.type){
            case Identifier:
                strcpy(c, expr->v.val.id);
                printf("identifier : %s\n", c);
                expr->type = lookup_table(table, c);
                break;
            case IntConst:
                printf("constant : int\n");
                expr->type = Int;
                break;
            case FloatConst:
                printf("constant : float\n");
                expr->type = Float;
                break;
                //case PlusNode: case MinusNode: case MulNode: case DivNode:
            default:
                break;
        }
    }
    else{
        Expression *left = expr->leftOperand;
        Expression *right = expr->rightOperand;

        checkexpression(left, table);
        checkexpression(right, table);

        DataType type = generalize(left, right);
        convertType(left, type);//left->type = type;//converto
        convertType(right, type);//right->type = type;//converto
        expr->type = type;
    }
}

void checkstmt( Statement *stmt, SymbolTable * table )
{
    if(stmt->type == Assignment){
        AssignmentStatement assign = stmt->stmt.assign;
        printf("assignment : %s \n", assign.id);
        checkexpression(assign.expr, table);
        stmt->stmt.assign.type = lookup_table(table, assign.id);
        if (assign.expr->type == Float && stmt->stmt.assign.type == Int) {
            printf("error : can't convert float to integer\n");
        } else {
            convertType(assign.expr, stmt->stmt.assign.type);
        }
    }
    else if (stmt->type == Print){
        printf("print : %s \n",stmt->stmt.variable);
        lookup_table(table, stmt->stmt.variable);
    }
    else printf("error : statement error\n");//error
}

void check( Program *program, SymbolTable * table )
{
    Statements *stmts = program->statements;
    while(stmts != NULL){
        checkstmt(&stmts->first,table);
        stmts = stmts->rest;
    }
}

/***********************************************************************
 Constant Folding
 ************************************************************************/
Expression *ReplaceConvertNode( Expression *expr ){
    Expression *newNode = (Expression *)malloc(sizeof(Expression));
    Value v;
    v.type = FloatConst;
    v.val.fvalue = (float)(expr->leftOperand->v.val.ivalue);
    newNode->v = v;
    newNode->leftOperand = newNode->rightOperand = NULL;
    newNode->type = Float;
    return newNode; 
}

Expression *FoldFloat( Expression *expr, float lvalue, float rvalue ){
    Expression *newNode = (Expression *)malloc(sizeof(Expression));
    Value v;
    v.type = FloatConst;
    switch(expr->v.type){
        case PlusNode:
            v.val.fvalue = lvalue + rvalue;
            break;
        case MinusNode:
            v.val.fvalue = lvalue - rvalue;
            break;
        case MulNode:
            v.val.fvalue = lvalue * rvalue;
            break;
        case DivNode:
            v.val.fvalue = lvalue / rvalue;
            break;
        default:
            break;
    }
    newNode->v = v;
    newNode->leftOperand = newNode->rightOperand = NULL;
    newNode->type = Float;
    return newNode;
}

Expression *FoldInt( Expression *expr, int lvalue, int rvalue ){
    Expression *newNode = (Expression *)malloc(sizeof(Expression));
    Value v;
    v.type = IntConst;
    switch(expr->v.type){
        case PlusNode:
            v.val.ivalue = lvalue + rvalue;
            break;
        case MinusNode:
            v.val.ivalue = lvalue - rvalue;
            break;
        case MulNode:
            v.val.ivalue = lvalue * rvalue;
            break;
        case DivNode:
            v.val.ivalue = lvalue / rvalue;
            break;
        default:
            break;
    }
    newNode->v = v;
    newNode->leftOperand = newNode->rightOperand = NULL;
    newNode->type = Int;
    return newNode;
}

Expression *TraverseExpressionTree( Expression *expr ){
    if(!expr->leftOperand && !expr->rightOperand)
        return expr;
    if(expr->leftOperand)
        expr->leftOperand = TraverseExpressionTree( expr->leftOperand );
    if(expr->rightOperand)
        expr->rightOperand = TraverseExpressionTree( expr->rightOperand );
    Expression *left = expr->leftOperand;
    Expression *right = expr->rightOperand;
    // left is not NULL, right is NULL: convertNode
    if(left && !right && left->v.type == IntConst){
        return ReplaceConvertNode( expr ); 
    }
    // operatorNode
    else if(left && right){
        if(left->v.type == FloatConst && right->v.type == FloatConst){
            float lvalue = left->v.val.fvalue;
            float rvalue = right->v.val.fvalue;
            return FoldFloat( expr, lvalue, rvalue );
        }       
        else if(left->v.type == IntConst && right->v.type == IntConst){
            int lvalue = left->v.val.ivalue;
            int rvalue = right->v.val.ivalue;
            return FoldInt( expr, lvalue, rvalue );
        }
    }
    return expr;
}

void ConstantFold( Program *program ){
    Statements *stmts = program->statements;
    while(stmts != NULL){
        Statement stmt = stmts->first;
        if(stmt.type == Assignment){
            stmt.stmt.assign.expr = TraverseExpressionTree( stmt.stmt.assign.expr );
            stmts->first = stmt;
        }
        stmts = stmts->rest;
    }
}

/***********************************************************************
  Code generation
 ************************************************************************/
char getRegister( char *name, SymbolTable *symtab ){
    int len = strlen(name);
    SymbolTable *current = symtab;
    for(int i = 0; i < len; i++)
        current = current->next[(int)(name[i] - 'a')];
    return (char)('a' + current->index);
}

void fprint_op( FILE *target, ValueType op )
{
    switch(op){
        case MinusNode:
            fprintf(target,"-\n");
            break;
        case PlusNode:
            fprintf(target,"+\n");
            break;
        case MulNode:
            fprintf(target, "*\n");
            break;
        case DivNode:
            fprintf(target, "/\n");
            break;
        default:
            fprintf(target,"Error in fprintf_op ValueType = %d\n",op);
            break;
    }
}

void fprint_expr( FILE *target, Expression *expr, SymbolTable *symtab )
{

    if(expr->leftOperand == NULL){
        switch( (expr->v).type ){
            case Identifier:
                fprintf(target,"l%c\n",getRegister((expr->v).val.id, symtab));
                break;
            case IntConst:
                fprintf(target,"%d\n",(expr->v).val.ivalue);
                break;
            case FloatConst:
                fprintf(target,"%f\n", (expr->v).val.fvalue);
                break;
            default:
                fprintf(target,"Error In fprint_left_expr. (expr->v).type=%d\n",(expr->v).type);
                break;
        }
    }
    else{
        fprint_expr(target, expr->leftOperand, symtab);
        if(expr->rightOperand == NULL){
            fprintf(target,"5k\n");
        }
        else{
            //	fprint_right_expr(expr->rightOperand);
            fprint_expr(target, expr->rightOperand, symtab);
            fprint_op(target, (expr->v).type);
        }
    }
}

void gencode(Program prog, FILE * target, SymbolTable *symtab)
{
    Statements *stmts = prog.statements;
    Statement stmt;
    int used[26];

    while(stmts != NULL){
        stmt = stmts->first;
        switch(stmt.type){
            case Print:
                fprintf(target,"l%c\n",getRegister(stmt.stmt.variable, symtab));
                fprintf(target,"p\n");
                break;
            case Assignment:
                //print_expr(stmt.stmt.assign.expr);
                fprint_expr(target, stmt.stmt.assign.expr, symtab);
                /*
                   if(stmt.stmt.assign.type == Int){
                   fprintf(target,"0 k\n");
                   }
                   else if(stmt.stmt.assign.type == Float){
                   fprintf(target,"5 k\n");
                   }*/
                fprintf(target,"s%c\n",getRegister(stmt.stmt.assign.id, symtab));
                fprintf(target,"0 k\n");
                break;
        }
        stmts=stmts->rest;
    }

}


/***************************************
  For our debug,
  you can omit them.
 ****************************************/
void print_expr(Expression *expr)
{
    if(expr == NULL)
        return;
    else{
        print_expr(expr->leftOperand);
        switch((expr->v).type){
            case Identifier:
                printf("%s ", (expr->v).val.id);
                break;
            case IntConst:
                printf("%d ", (expr->v).val.ivalue);
                break;
            case FloatConst:
                printf("%f ", (expr->v).val.fvalue);
                break;
            case PlusNode:
                printf("+ ");
                break;
            case MinusNode:
                printf("- ");
                break;
            case MulNode:
                printf("* ");
                break;
            case DivNode:
                printf("/ ");
                break;
            case IntToFloatConvertNode:
                printf("(float) ");
                break;
            default:
                printf("error ");
                break;
        }
        print_expr(expr->rightOperand);
    }
    printf("\n");
}

void test_parser( FILE *source )
{
    Declarations *decls;
    Statements *stmts;
    Declaration decl;
    Statement stmt;
    Program program = parser(source);

    decls = program.declarations;

    while(decls != NULL){
        decl = decls->first;
        if(decl.type == Int)
            printf("i ");
        if(decl.type == Float)
            printf("f ");
        printf("%s ",decl.name);
        decls = decls->rest;
    }

    stmts = program.statements;

    while(stmts != NULL){
        stmt = stmts->first;
        if(stmt.type == Print){
            printf("p %s ", stmt.stmt.variable);
        }

        if(stmt.type == Assignment){
            printf("%s = ", stmt.stmt.assign.id);
            print_expr(stmt.stmt.assign.expr);
        }
        stmts = stmts->rest;
    }

}
