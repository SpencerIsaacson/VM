#include <assert.h>
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

AST_Node* Ident(char *name)
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


int lookup_addr(char *varname)
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

AST_Node* Increment(char *name)
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

AST_Node* GoTo(char *label_name)
{	
	AST_Node* node = next_node();
	node->ast_node_type = nt_GoTo;
	node->name = label_name;
	return node;
}

AST_Node* BinOp(AST_Node* lhs, char *operator, AST_Node* rhs)
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

AST_Node* Const(char *name)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Const;
	node->name = name;
	return node;	
}

AST_Node* Type(char *name)
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

char* names[7] = {"x", "y", "width", "height", "_x", "_y", "white"};
int addr_look_up[7] = { 200, 201, 202, 203, 204, 205, 206 };

int lookup(char *varname)
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
		printf("   ");
}

print_cond(AST_Node *condition)
{
	//todo compound conditions
	if(condition->lhs->ast_node_type == nt_IntLiteral)
		printf("%d",condition->lhs->value);
	if(condition->lhs->ast_node_type == nt_Ident)
		printf("%s",condition->lhs->name);

	printf(" %s ", condition->operator);

	if(condition->rhs->ast_node_type == nt_IntLiteral)
		printf("%d",condition->rhs->value);
	if(condition->rhs->ast_node_type == nt_Ident)
		printf("%s",condition->rhs->name);
}

void print_while(AST_Node *_while, SourceLevel level)
{
	printf("while ");
	print_cond(_while->condition);
	printf("\n");
	indent();
	printf("{\n");
	indent_level++;
	for(int i = 0; i < _while->body->statement_count; i++)
	{
		print_statement( _while->body->statements[i], level);
	}
	indent_level--;
	indent();
	printf("}\n");
}

void print_if(AST_Node *_if, SourceLevel level)
{
	printf("if ");
	print_cond(_if->condition);
	printf("\n");
	indent();
	printf("{\n");
	indent_level++;
	for(int i = 0; i < _if->body->statement_count; i++)
	{
		print_statement( _if->body->statements[i], level);
	}
	indent_level--;
	indent();
	printf("}\n");
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
					printf("decl %s %s\n", statement->type->name, statement->var->name);
					break;
				case HIGH:
					printf("%s %s\n", statement->type->name, statement->var->name);
			}
		} break;			
		case nt_Assign:
		{
			switch(level)
			{
				case LOW:
					if(statement->rhs->ast_node_type == nt_IntLiteral)
						printf("assign %s %d\n", statement->lhs->name, statement->rhs->value);
					else if(statement->rhs->ast_node_type == nt_Ident)
						printf("assign %s %s\n", statement->lhs->name, statement->rhs->name);
					break;
				case HIGH:
					if(statement->rhs->ast_node_type == nt_IntLiteral)
						printf("%s = %d\n", statement->lhs->name, statement->rhs->value);
					else if(statement->rhs->ast_node_type == nt_Ident)
						printf("%s = %s\n", statement->lhs->name, statement->rhs->name);
					break;
			}
		} break;
		case nt_AssignMem:
		{
			switch(level)
			{
				case LOW:
					printf("assign [%d] %s//todo print address/expression correctly\n", statement->value, statement->var->name);
					break;
				case HIGH:
					printf("[%d] = %s//todo print address/expression correctly\n", statement->value, statement->var->name);
					break;
			}
		} break;			
		case nt_Increment:
			switch(level)
			{
				case LOW:
					printf("inc %s\n", statement->name);
					break;
				case HIGH:
					printf("%s++\n", statement->name);
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
			printf("inc %d\n", lookup_addr(stmt->name));
			instr_index++;
		} break;
		case nt_Assign:
			if(stmt->rhs->ast_node_type == nt_IntLiteral)
				printf("set %d %d\n", lookup_addr(stmt->lhs->name), stmt->rhs->value);
			else if(stmt->rhs->ast_node_type == nt_Ident)
				printf("set %d @%d\n", lookup_addr(stmt->lhs->name), lookup_addr(stmt->rhs->name));
				instr_index++;			
			break;
		case nt_While:
			//printf("while %s %s %s\n", stmt->condition->lhs->name, stmt->condition->operator, stmt->condition->rhs->name);			
			break;
		case nt_If:
		{
			char c = stmt->condition->operator[0];
			switch(c)
			{
				case '<':
				{
					printf("jlt ");
					unsigned int jump_address = instr_index+2;
					printf("%d ", jump_address);
					AST_Node *cond = stmt->condition;

					//swap lhs and rhs
					if(cond->rhs->ast_node_type == nt_IntLiteral)
						printf("%d ", cond->rhs->value);
					else if(stmt->lhs->ast_node_type == nt_Ident)
						printf("@%d ", lookup_addr(cond->rhs->name));

					if(cond->lhs->ast_node_type == nt_IntLiteral)
						printf("%d ", cond->lhs->value);
					else if(cond->lhs->ast_node_type == nt_Ident)
						printf("@%d ", lookup_addr(cond->lhs->name));
					printf("\n");
					for (int i = 0; i < stmt->body->statement_count; ++i)
					{
						emit_statement(stmt->body->statements[i]);
					}
					instr_index++;
				} break;
				default:
					printf("operator `%c` not implemented!\n", c);
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
		printf("cannot parse null AST");
}

void test_program()
{
	#include "basic_ast.h"

	print_source(h,HIGH);

	printf("\n");

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

typedef struct
{
	int length;
	char *data;
} string;

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
	"if",
	"while",
	"int",
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
	      [tok_NONE] = "NONE",	
	 [tok_OpenBrace] = "OpenBrace",
	[tok_CloseBrace] = "CloseBrace",
	    [tok_Equals] = "Equals",
	  [tok_LessThan] = "LessThan",
	      [tok_Plus] = "Plus",
	    [tok_Divide] = "Divide",
	 [tok_Increment] = "Increment",
	   [tok_Keyword] = "Keyword",
	     [tok_Ident] = "Ident",
	[tok_IntLiteral] = "IntLiteral",
};
typedef struct
{
	TokenType type;
	string text;
} Token;

int token_count = 0;
Token *tokens = NULL; //todo handle proper allocation
void lex(string source)
{
	tokens = realloc(tokens, sizeof(Token)*source.length);
	token_count	= 0;

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

AST_Node *parse_statement(Token *tokens)
{
	Token t = *tokens;
	AST_Node *n = NULL;

	switch(t.type)
	{
		case tok_IntLiteral:
		{
		 	n = IntLiteral(0);
		} break;
		case tok_Ident:
		{
			char *d = copy_to_c_string(t.text);
			n = Ident(d);
	 		t = *(++tokens);

	 		if(t.type == tok_Equals)
	 		{
		 		t = *(++tokens);
				return Assign(n, IntLiteral(10));
	 		}

			return NULL;	
		} break;
		case tok_Keyword:
		{
			if(string_equals(t.text, "int"))
			{
		 		n = Type("int");
		 		t = *(++tokens);
		 		if(t.type == tok_Ident)
		 		{
		 			char *d = copy_to_c_string(t.text); //this is fucking dumb and I hate it and we need to come up with a more general strategy for string handling.
		 			return Decl(n, Ident(d));
		 		}
			}
			else if(string_equals(t.text, "if"))
			{
		 		t = *(++tokens);
				char *d = copy_to_c_string(t.text);
				n = Ident(d);

		 		t = *(++tokens);
		 		if(t.type == tok_LessThan)
		 		{
		 			AST_Node *if_body[0];
					return If(Cond(n, "<", IntLiteral(11)), StatementList(NodeList(if_body)));
		 		}

		 		return NULL;


			}			
		} break;
	}
}

AST_Node* parse_statement_list(Token *tokens)
{
	int statement_count = 0;


	AST_Node *a = parse_statement(tokens);
	tokens++;
	tokens++;
	AST_Node *b = parse_statement(tokens);
	tokens++;
	tokens++;
	tokens++;
	AST_Node *c = parse_statement(tokens);
	tokens++;
	tokens++;	
	AST_Node *d = parse_statement(tokens);
	tokens++;
	tokens++;
	tokens++;	
	AST_Node *e = parse_statement(tokens);
	AST_Node *sts[5] = {a,b,c,d,e};
	AST_Node *list = StatementList(NodeList(sts));
	return list;
}

void print_tokens(int detail)
{
	for (int i = 0; i < token_count; ++i)
	{
		printf("%.*s", tokens[i].text.length, tokens[i].text.data);
		if(detail)
		{
			printf(":\n");
			printf("    %s\n", tok_typenames[tokens[i].type]);
		}
		else
			printf("\n");
	}
}

char *read_file(char *path)
{
	FILE *file = fopen(path, "rb");
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
 	char *source = read_file("basic.hum");

 	string source_string = (string){.length = strlen(source), .data = source };
 	lex(source_string);
 	//print_tokens(1);
 	AST_Node *ast = parse_statement_list(tokens);
 	print_source(ast, HIGH);
 	//emit_code(ast);
}