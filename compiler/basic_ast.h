
AST_Node* decl_x = Decl(Type("int"), Ident("x"));
AST_Node* x_init = Assign(Ident("x"), IntLiteral(10));

AST_Node* decl_y = Decl(Type("int"), Ident("y"));
AST_Node* y_init = Assign(Ident("y"), IntLiteral(17));

AST_Node* x_inc = Increment("x");
AST_Node* if_body[1] = { x_inc };
AST_Node* if1 = If(Cond(Ident("x"), "<", IntLiteral(11)), StatementList(NodeList(if_body)));
AST_Node* x_assign = Assign(Ident("x"), IntLiteral(30));
AST_Node* if_body2[1] = { x_assign };
AST_Node* if2 = If(Cond(Ident("x"), "<", IntLiteral(11)), StatementList(NodeList(if_body2)));
AST_Node* x_as_y = Assign(Ident("x"), Ident("y"));

AST_Node* list[] =
{ decl_x, x_init, decl_y, y_init, if1, if2, x_as_y };

AST_Node* h = StatementList(list, countof(list));