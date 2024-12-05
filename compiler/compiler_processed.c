#define c2s(n) ((string){.data = n, .length = strlen(n)})
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool str_equals(char* a, char* b)
{
	return strcmp(a,b) == 0;
}

typedef enum
{
	nt_Const,
	nt_IntLiteral,
	nt_Ident,
	nt_Assign,
	nt_AssignMem,
	nt_StatementList,
	nt_While,
	nt_Cond,
	nt_Increment,
	nt_BinOp,
	nt_If,
	nt_GoTo,
	nt_Decl,
	nt_Type,
} AST_NodeType;

typedef struct AST_Node
{
	AST_NodeType ast_node_type;
	int value;
	char* name;
	struct AST_Node* type;
	struct AST_Node* var;
	int statement_count;
	struct AST_Node** statements;
	struct AST_Node* body;
	struct AST_Node* condition;
	char* operator;
	struct AST_Node* lhs;
	struct AST_Node* rhs;
} AST_Node;

int node_count = 0;
AST_Node nodes[1000];

AST_Node* next_node()
{
	if(node_count < 1000)
		return &nodes[node_count++];
	return NULL;
}

AST_Node* Ident(char* name)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Ident;
	node->name = name;
	return node;
}

AST_Node* Assign(AST_Node* lhs, AST_Node *rhs)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Assign;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

AST_Node* AssignMem(AST_Node* lhs, AST_Node* addr)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_AssignMem;
	node->lhs = lhs;
	node->rhs = addr;
	return node;
}


#define countof(...) (sizeof(__VA_ARGS__)/sizeof(*(__VA_ARGS__)))
#define NodeList(x) x, countof(x)

#define decl_start 200
int var_decl = 0;
char* var_names[1000];


int lookup_addr(char* varname)
{
	for (int i = 0; i < countof(var_names); ++i)
	{
		if(str_equals(var_names[i], varname))
			return decl_start+i;
	}

	return -1; //error
}

AST_Node* StatementList(AST_Node** list, int count)
{
	AST_Node* node = next_node();
	node->statements = (AST_Node**)next_node();
	node_count+=count-1;
	node->ast_node_type  = nt_StatementList;

	for (int i = 0; i < count; ++i)
	 	node->statements[i] = list[i];

	node->statement_count = count;
	return node;
}

AST_Node* While(AST_Node* condition, AST_Node* while_body)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_While;
	node->condition = condition;
	node->body = while_body;
	return node;
}

AST_Node* Cond(AST_Node* lhs, char* operator, AST_Node* rhs)
{	
	AST_Node* node = next_node();
	node->ast_node_type = nt_Cond;
	node->lhs = lhs;
	node->operator = operator;
	node->rhs = rhs;
	return node;
}

AST_Node* Increment(char* name)
{	
	AST_Node* node = next_node();
	node->ast_node_type = nt_Increment;
	node->name = name;
	return node;
}

AST_Node* If(AST_Node* condition, AST_Node* body)
{	
	AST_Node* node = next_node();
	node->ast_node_type = nt_If;
	node->condition = condition;
	node->body = body;	
	return node;
}

AST_Node* GoTo(char* label_name)
{	
	AST_Node* node = next_node();
	node->ast_node_type = nt_GoTo;
	node->name = label_name;
	return node;
}

AST_Node* BinOp(AST_Node* lhs, char* operator, AST_Node* rhs)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_BinOp;
	node->lhs = lhs;
	node->operator = operator;
	node->rhs = rhs;
	return node;	
}

AST_Node* IntLiteral(int value)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_IntLiteral;
	node->value = value;
	return node;
}

AST_Node* Const(char* name)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Const;
	node->name = name;
	return node;	
}

AST_Node* Type(char* name)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Type;
	node->name = name;
	return node;	
}

AST_Node* Decl(AST_Node* type, AST_Node* var)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Decl;
	node->type = type;
	node->var = var;
	var_names[var_decl++] = var->name;
	return node;	
}

typedef struct
{
	int length;
	char *data;
} string;

string names[7] = {c2s("x"), c2s("y"), c2s("width"), c2s("height"), c2s("_x"), c2s("_y"), c2s("white")};
int addr_look_up[7] = { 200, 201, 202, 203, 204, 205, 206 };

int lookup(char* varname)
{
	for (int i = 0; i < countof(names); ++i)
	{
		if(str_equals(names[i], varname))
			return addr_look_up[i];
	}

	return -1; //error
}

typedef enum
{
	LOW,
	HIGH,
} SourceLevel;
void print_while(AST_Node *_while, SourceLevel level);
void print_statement(AST_Node *statement, SourceLevel level);
int indent_level = 0;
void indent()
{
	for (int i = 0; i < indent_level; ++i)
		printf(c2s("   "));
}

print_cond(AST_Node *condition)
{
	//todo compound conditions
	if(condition->lhs->ast_node_type == nt_IntLiteral)
		printf(c2s("%d"),condition->lhs->value);
	if(condition->lhs->ast_node_type == nt_Ident)
		printf(c2s("%s"),condition->lhs->name);

	printf(c2s(" %s "), condition->operator);

	if(condition->rhs->ast_node_type == nt_IntLiteral)
		printf(c2s("%d"),condition->rhs->value);
	if(condition->rhs->ast_node_type == nt_Ident)
		printf(c2s("%s"),condition->rhs->name);
}

void print_while(AST_Node *_while, SourceLevel level)
{
	printf(c2s("while "));
	print_cond(_while->condition);
	printf(c2s("\n"));
	indent();
	printf(c2s("{\n"));
	indent_level++;
	for(int i = 0; i < _while->body->statement_count; i++)
	{
		print_statement( _while->body->statements[i], level);
	}
	indent_level--;
	indent();
	printf(c2s("}\n"));
}

void print_if(AST_Node *_if, SourceLevel level)
{
	printf(c2s("if "));
	print_cond(_if->condition);
	printf(c2s("\n"));
	indent();
	printf(c2s("{\n"));
	indent_level++;
	for(int i = 0; i < _if->body->statement_count; i++)
	{
		print_statement( _if->body->statements[i], level);
	}
	indent_level--;
	indent();
	printf(c2s("}\n"));
}

void print_statement(AST_Node *statement, SourceLevel level)
{
	indent();
	switch(statement->ast_node_type)
	{
		case nt_Decl:
		{
			switch(level)
			{
				case LOW:
					printf(c2s("decl %s %s\n"), statement->type->name, statement->var->name);
					break;
				case HIGH:
					printf(c2s("%s %s\n"), statement->type->name, statement->var->name);
			}
		} break;			
		case nt_Assign:
		{
			switch(level)
			{
				case LOW:
					if(statement->rhs->ast_node_type == nt_IntLiteral)
						printf(c2s("assign %s %d\n"), statement->lhs->name, statement->rhs->value);
					else if(statement->rhs->ast_node_type == nt_Ident)
						printf(c2s("assign %s %s\n"), statement->lhs->name, statement->rhs->name);
					break;
				case HIGH:
					if(statement->rhs->ast_node_type == nt_IntLiteral)
						printf(c2s("%s = %d\n"), statement->lhs->name, statement->rhs->value);
					else if(statement->rhs->ast_node_type == nt_Ident)
						printf(c2s("%s = %s\n"), statement->lhs->name, statement->rhs->name);
					break;
			}
		} break;
		case nt_AssignMem:
		{
			switch(level)
			{
				case LOW:
					printf(c2s("assign [%d] %s//todo print address/expression correctly\n"), statement->value, statement->var->name);
					break;
				case HIGH:
					printf(c2s("[%d] = %s//todo print address/expression correctly\n"), statement->value, statement->var->name);
					break;
			}
		} break;			
		case nt_Increment:
			switch(level)
			{
				case LOW:
					printf(c2s("inc %s\n"), statement->name);
					break;
				case HIGH:
					printf(c2s("%s++\n"), statement->name);
					break;
			}
			break;
		case nt_If:
		{
			print_if(statement, level);
		} break;			
		case nt_While:
		{
			print_while(statement, level);
		} break;
	}
}

print_source(AST_Node	*root, SourceLevel level)
{
	for(int i = 0; i < root->statement_count; i++)
	{
		print_statement(root->statements[i], level);
	}
}


int instr_index = 0;

void emit_statement(AST_Node *stmt)
{
	switch(stmt->ast_node_type)
	{
		case nt_Increment:
		{
			printf(c2s("inc %d\n"), lookup_addr(stmt->name));
			instr_index++;
		} break;
		case nt_Assign:
			if(stmt->rhs->ast_node_type == nt_IntLiteral)
				printf(c2s("set %d %d\n"), lookup_addr(stmt->lhs->name), stmt->rhs->value);
			else if(stmt->rhs->ast_node_type == nt_Ident)
				printf(c2s("set %d @%d\n"), lookup_addr(stmt->lhs->name), lookup_addr(stmt->rhs->name));
				instr_index++;			
			break;
		case nt_While:
			//printf(c2s("while %s %s %s\n"), stmt->condition->lhs->name, stmt->condition->operator, stmt->condition->rhs->name);			
			break;
		case nt_If:
		{
			char c = stmt->condition->operator[0];
			switch(c)
			{
				case '<':
				{
					printf(c2s("jlt "));
					unsigned int jump_address = instr_index+2;
					printf(c2s("%d "), jump_address);
					AST_Node *cond = stmt->condition;

					//swap lhs and rhs
					if(cond->rhs->ast_node_type == nt_IntLiteral)
						printf(c2s("%d "), cond->rhs->value);
					else if(stmt->lhs->ast_node_type == nt_Ident)
						printf(c2s("@%d "), lookup_addr(cond->rhs->name));

					if(cond->lhs->ast_node_type == nt_IntLiteral)
						printf(c2s("%d "), cond->lhs->value);
					else if(cond->lhs->ast_node_type == nt_Ident)
						printf(c2s("@%d "), lookup_addr(cond->lhs->name));
					printf(c2s("\n"));
					for (int i = 0; i < stmt->body->statement_count; ++i)
					{
						emit_statement(stmt->body->statements[i]);
					}
					instr_index++;
				} break;
				default:
					printf(c2s("operator `%c` not implemented!\n"), c);
					exit(-1);
			}
		} break;
	}
}

void emit_code(AST_Node	*root)
{
	if(root)
	for (int i = 0; i < root->statement_count; ++i)
	{
		AST_Node *stmt = root->statements[i];
		emit_statement(stmt);

	}
	else
		printf(c2s("cannot parse null AST"));
}

void test_program()
{
	#include c2s("basic_ast.h")

	print_source(h,HIGH);

	printf(c2s("\n"));

	emit_code(h);
}

bool is_number(char c)
{
	return (c >= '0' && c <= '9');
}

bool is_letter(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool is_alphanumeric(char c)
{
	return is_letter(c) || is_number(c);
}



char *copy_to_c_string(string t)
{
	char *c = malloc(t.length+1);
	for (int i = 0; i < t.length; ++i)
	{
		c[i] = t.data[i];
	}

	c[t.length] = 0;
	return c;
}


#define keyword_count 3

char * keywords[keyword_count] =
{
	c2s("if"),
	c2s("while"),
	c2s("int"),
};

bool string_equals(string a, char *b)
{
	int b_len = strlen(b); 
	for (int i = 0; (i < a.length) && (i < b_len); ++i)
	{
		if(a.data[i] != b[i])
			return false;
	}

	return true;
}
typedef enum
{
	tok_NONE,
	tok_OpenBrace,
	tok_CloseBrace,
	tok_Equals,
	tok_LessThan,
	tok_Plus,
	tok_Divide,
	tok_Increment,
	tok_Keyword,
	tok_Ident,
	tok_IntLiteral,
	tok_typecount
} TokenType;

char *tok_typenames[tok_typecount] = 
{
	      [tok_NONE] = c2s("NONE"),	
	 [tok_OpenBrace] = c2s("OpenBrace"),
	[tok_CloseBrace] = c2s("CloseBrace"),
	    [tok_Equals] = c2s("Equals"),
	  [tok_LessThan] = c2s("LessThan"),
	      [tok_Plus] = c2s("Plus"),
	    [tok_Divide] = c2s("Divide"),
	 [tok_Increment] = c2s("Increment"),
	   [tok_Keyword] = c2s("Keyword"),
	     [tok_Ident] = c2s("Ident"),
	[tok_IntLiteral] = c2s("IntLiteral"),
};
typedef struct
{
	TokenType type;
	string text;
} Token;

int token_count = 0;
Token tokens[7000]; //todo handle proper allocation
void lex(string source)
{
#define new_tok(n) (Token){.type = n, .text = {.data = &source.data[i], .length = 1 } }
	for (int i = 0; i < source.length; ++i)
	{
		char c = source.data[i];
		switch(c)
		{
			case ' ':
			case '\t':
			case '\r':
			case '\n':
				break;
			case '/':
			{
				if(source.data[i+1] == '/')
				{
					while(source.data[i++] != '\n');
				}
				else
				{
					tokens[token_count++] = new_tok(tok_Divide);
				}
			} break;				
			case '{':
				tokens[token_count++] = new_tok(tok_OpenBrace);
				break;
			case '}':
				tokens[token_count++] = new_tok(tok_CloseBrace);
				break;				
			case '=':
				tokens[token_count++] = new_tok(tok_Equals);
				break;				
			case '<':
				tokens[token_count++] = new_tok(tok_LessThan);
				break;
			case '+':
				if(source.data[i+1] == '+'){
					tokens[token_count++] = (Token){.type = tok_Increment, .text = {.data = &source.data[i], .length = 2 } };
					i++;
				}
				else
					tokens[token_count++] = new_tok(tok_Plus);
				break;
			case '"':
				//switch mode, begin string
				break;
			//else idents and such
			default:
			{
				int start = i;
				TokenType t = tok_NONE;
				string text;
				if(is_letter(source.data[i]) || source.data[i] == '_')
				{
					while(is_alphanumeric(source.data[i]) || source.data[i] == '_')
					{
						i++;
					}

					text = (string){.data = &source.data[start], .length = i-start };

					for (int o = 0; o < keyword_count; ++o)
					{
						if(string_equals(text, keywords[o]))
						{
							t = tok_Keyword;
							break;
						}
					}
					if(t == tok_NONE)
						t = tok_Ident;
				}
				else if(is_number(source.data[i]))
				{
					t = tok_IntLiteral;
					while(is_number(source.data[i]))
					{
						i++;
					}

					text = (string){.data = &source.data[start], .length = i-start };
				}

				tokens[token_count++] = (Token){.type = t, .text = text };
				i--;
			}
		}
	}
}

AST_Node *parse(Token *tokens)
{
	int t_i = 0;
	Token t = tokens[t_i];

	AST_Node *n = NULL;

	switch(t.type)
	{
		case tok_IntLiteral:
		{
		 	n = IntLiteral(0);
		} break;
		case tok_Keyword:
		{
			if(string_equals(t.text, c2s("int")))
			{
		 		n = Type(c2s("int"));
		 		t_i++;
		 		t = tokens[t_i];
		 		if(t.type == tok_Ident)
		 		{
		 			char *d = copy_to_c_string(t.text); //this is fucking dumb and I hate it and we need to come up with a more general strategy for string handling.
		 			n = Decl(n, Ident(d));
		 		}
			}
		} break;			
	}

	AST_Node *sts[1] = {n};
	AST_Node *list = StatementList(NodeList(sts));
	return list;
}
void print_tokens(int detail)
{
	for (int i = 0; i < token_count; ++i)
	{
		printf(c2s("%.*s"), tokens[i].text.length, tokens[i].text.data);
		if(detail)
		{
			printf(c2s(":\n"));
			printf(c2s("    %s\n"), tok_typenames[tokens[i].type]);
		}
		else
			printf(c2s("\n"));
	}
}

char *read_file(char *path)
{
	FILE *file = fopen(path, c2s("rb"));
	fseek(file, 0, SEEK_END);
	int source_size = ftell(file);
	rewind(file);
	char * source = malloc(source_size+1);
	source[source_size] = 0;
	fread(source, 1, source_size, file);
	fclose(file);
	return source;	
}

int main(int argc, char** argv)
{
 	char *source = read_file(c2s("basic.hum"));

 	string source_string = (string){.length = strlen(source), .data = source };
 	lex(source_string);
 	//print_tokens(1);
 	AST_Node *ast = parse(tokens);
 	print_source(ast, HIGH);
 	emit_code(ast);
	// AST_Node* decl_x = Decl(Type(c2s("int")), Var(c2s("x")));
	// AST_Node* x_init = Assign(Var(c2s("x")), 10);

	// printf(c2s("loadi A %d\nloadi B 0\nor\nstore %d\n"), x_init->value, lookup_addr(x_init->var->name));
}