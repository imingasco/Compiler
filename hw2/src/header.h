struct symtab{
	char lexeme[32];
	struct symtab *front;
	struct symtab *back;
	int line;
	int counter;
    short reserved;
};

typedef struct symtab symtab;
symtab * lookup(char *name);
void insert(char *name);
