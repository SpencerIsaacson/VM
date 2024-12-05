AST_Node* decl_x = Decl(Type("int"), Ident("x"));
AST_Node* decl_y = Decl(Type("int"), Ident("y"));
AST_Node* decl_width = Decl(Type("int"), Ident("width"));
AST_Node* decl_height = Decl(Type("int"), Ident("height"));
AST_Node* decl__x = Decl(Type("int"), Ident("_x"));
AST_Node* decl__y = Decl(Type("int"), Ident("_y"));
AST_Node* decl_pixel_address = Decl(Type("int"), Ident("pixel_address"));
AST_Node* decl_white = Decl(Type("int"), Ident("white"));

AST_Node* x_init = Assign(Ident("x"), IntLiteral(10));
AST_Node* y_init = Assign(Ident("y"), IntLiteral(15));
AST_Node* width_init = Assign(Ident("width"), IntLiteral(150));
AST_Node* height_init = Assign(Ident("height"), IntLiteral(75));
AST_Node* _x_init = Assign(Ident("_x"), IntLiteral(0));
AST_Node* _y_init = Assign(Ident("_y"), IntLiteral(0));
AST_Node* white_init = Assign(Ident("white"), IntLiteral(0xFFFFFF));

AST_Node* y_plus__y = BinOp(Ident("y"), "+", Ident("_y"));
AST_Node* y_times_screen = BinOp(y_plus__y, "*", Const("screen_width"));
AST_Node* x_plus__x = BinOp(Ident("x"), "+", Ident("_x"));
AST_Node* y_screen_x = BinOp(y_times_screen, "+", x_plus__x);
AST_Node* calculate_pixel_address = BinOp(y_screen_x, "+", Const("screen"));
AST_Node* set_pixel = AssignMem(Ident("white"), calculate_pixel_address);
AST_Node* _x_inc = Increment(Ident("_x"));

AST_Node* while2_body[2] =
{
	set_pixel,
	_x_inc,
};

AST_Node* while2 = While(Cond(Ident("_x"), "<", Ident("width")), StatementList(NodeList(while2_body)));

AST_Node* _y_inc = Increment(Ident("_y"));

AST_Node* while1_body[2] = { while2, _y_inc};

AST_Node* while1 = While(Cond(Ident("_y"), "<", Ident("height")), StatementList(NodeList(while1_body)));


AST_Node* list[] =
{
	decl_x, decl_y, decl_width, decl_height, decl__x, decl__y, decl_pixel_address, decl_white, x_init,y_init,width_init,height_init,_x_init,_y_init,white_init, while1,
};

AST_Node* h = StatementList(list, countof(list));
