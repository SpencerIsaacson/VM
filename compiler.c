#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool str_equals (char* a, char* b)
{
	return strcmp(a,b) == 0;
}

typedef enum
{
	nt_Const,
	nt_Var,
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

AST_Node* Var(char* name)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Var;
	node->name = name;
	return node;
}

AST_Node* Assign(AST_Node* var, int value)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_Assign;
	node->var = var;
	node->value = value;
	return node;
}

AST_Node* AssignMem(AST_Node* var, AST_Node* addr)
{
	AST_Node* node = next_node();
	node->ast_node_type = nt_AssignMem;
	node->var = var;
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

char* names[7] = {"x", "y", "width", "height", "_x", "_y", "white"};
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


void test_program()
{
	AST_Node* decl_x = Decl(Type("int"), Var("x"));
	AST_Node* decl_y = Decl(Type("int"), Var("y"));
	AST_Node* decl_width = Decl(Type("int"), Var("width"));
	AST_Node* decl_height = Decl(Type("int"), Var("height"));
	AST_Node* decl__x = Decl(Type("int"), Var("_x"));
	AST_Node* decl__y = Decl(Type("int"), Var("_y"));
	AST_Node* decl_pixel_address = Decl(Type("int"), Var("pixel_address"));
	AST_Node* decl_white = Decl(Type("int"), Var("white"));

	AST_Node* x_init = Assign(Var("x"), 10);
	AST_Node* y_init = Assign(Var("y"), 15);
	AST_Node* width_init = Assign(Var("width"), 150);
	AST_Node* height_init = Assign(Var("height"), 75);
	AST_Node* _x_init = Assign(Var("_x"), 0);
	AST_Node* _y_init = Assign(Var("_y"), 0);
	AST_Node* white_init = Assign(Var("white"), 0xFFFFFF);

	AST_Node* y_plus__y = BinOp(Var("y"), "+", Var("_y"));
	AST_Node* y_times_screen = BinOp(y_plus__y, "*", Const("screen_width"));
	AST_Node* x_plus__x = BinOp(Var("x"), "+", Var("_x"));
	AST_Node* y_screen_x = BinOp(y_times_screen, "+", x_plus__x);
	AST_Node* calculate_pixel_address = BinOp(y_screen_x, "+", Const("screen"));
	AST_Node* set_pixel = AssignMem(Var("white"), calculate_pixel_address);
	AST_Node* _x_inc = Increment("_x");

	AST_Node* while2_body[] =
	{
		set_pixel,
		_x_inc,
	};

	AST_Node* while2 = While(Cond(Var("_x"), "<", Var("width")), StatementList(NodeList(while2_body)));

	AST_Node* _y_inc = Increment("_y");

	AST_Node* while1_body[] = { while2, _y_inc};

	AST_Node* while1 = While(Cond(Var("_y"), "<", Var("height")), StatementList(NodeList(while1_body)));


	AST_Node* list[] =
	{
		x_init,y_init,width_init,height_init,_x_init,_y_init,white_init, while1,
	};

	AST_Node* h = StatementList(list, countof(list));


	for (int i = 0; i < h->statement_count; ++i)
	{
		switch(h->statements[i]->ast_node_type)
		{
			case nt_Decl:
				printf("declare %s %d\n", h->statements[i]->type, h->statements[i]->var->name);
				break;			
			case nt_Assign:
				printf("assign %s %d\n", h->statements[i]->var->name, h->statements[i]->value);
				break;
			case nt_While:
				printf("while %s %s %s\n{\t", h->statements[i]->condition->lhs->name, h->statements[i]->condition->operator, h->statements[i]->condition->rhs->name);
				printf("\n}\n");
				break;

		}
	}

	for (int i = 0; i < h->statement_count; ++i)
	{
		switch(h->statements[i]->ast_node_type)
		{
			case nt_Assign:
				printf("loadi A %d\nloadi B 0\nor\nstore %d\n", h->statements[i]->value, lookup_addr(h->statements[i]->var->name));
				break;
			case nt_While:
				//printf("while %s %s %s\n", h->statements[i]->condition->lhs->name, h->statements[i]->condition->operator, h->statements[i]->condition->rhs->name);			
				break;
		}
	}
}



int main(int argc, char** argv)
{
	test_program();
	// AST_Node* decl_x = Decl(Type("int"), Var("x"));
	// AST_Node* x_init = Assign(Var("x"), 10);

	// printf("loadi A %d\nloadi B 0\nor\nstore %d\n", x_init->value, lookup_addr(x_init->var->name));
}