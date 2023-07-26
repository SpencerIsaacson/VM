//The Legend of Zelda: Fair Use Demonstration
#include <math.h>

bool button_down(GamePad pad, ButtonName button_name)
{
	return (pad.buttons & button_name) != 0;
}

#define pad0 (mem.game_pads[0])
bool ButtonDown(ButtonName button_name)
{
	return button_down(pad0, button_name);
}

print_gamepad(int pad)
{
	printf(
		"pad %d:\n{\n UP: %d\nDOWN: %d\nLEFT: %d\nRIGHT: %d\nA: %d\nB: %d\nX: %d\nY: %d\nSELECT: %d\nSTART: %d\nL1: %d\nR1: %d\nL2: %d\nR2: %d\nL3: %d\nR3:%d\nleft_stick:{ x: %d, y: %d }\nright_stick:{ x: %d, y: %d }}\n\n",
		pad,
		ButtonDown(UP),
		ButtonDown(DOWN),
		ButtonDown(LEFT),
		ButtonDown(RIGHT),
		ButtonDown(A),
		ButtonDown(B),
		ButtonDown(X),
		ButtonDown(Y),
		ButtonDown(SELECT),
		ButtonDown(START),
		ButtonDown(L1),
		ButtonDown(R1),
		ButtonDown(L2),
		ButtonDown(R2),
		ButtonDown(L3),
		ButtonDown(R3),
		mem.game_pads[pad].sticks.left_stick.X,
		mem.game_pads[pad].sticks.left_stick.Y,
		mem.game_pads[pad].sticks.right_stick.X,
		mem.game_pads[pad].sticks.right_stick.Y
	);
}

typedef struct
{
	u32 width, height;
	Color pixels[0];
} InlineTexture;

typedef struct
{
	u32 width, height;
	Color *pixels;
} Texture;

#define sprite_size 22
typedef struct
{
	Color pixels[sprite_size*sprite_size];
} Sprite;



int clamp_int(int val, int min, int max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

void clamp_float(float *val, int min, int max)
{
	*val = (*val < min) ? min : ((*val > max) ? max : *val);
}

typedef enum
{
	Field,
	MeshTest,
	Face,
	SplashScreen,
	TitleScreen,
	FileSelect,
	Dungeon,
	Paused,
	scene_count,
} GameState;

typedef struct
{
	int x, y;
} v2i;

float v2i_mag(v2i v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y);
}

v2i v2i_sub(v2i a, v2i b)
{
	return (v2i){ a.x - b.x, a.y - b.y};
}

float v2i_dist(v2i a, v2i b)
{    return v2i_mag(v2i_sub(a,b));    }

float v2_mag(v2 v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y);
}

v2 v2_sub(v2 a, v2 b)
{
	return (v2){ a.x - b.x, a.y - b.y};
}

float v2_dist(v2 a, v2 b)
{    return v2_mag(v2_sub(a,b));    }


typedef struct
{
	int x, y, width, height;
} Rect;

typedef struct
{
	float 
	left, 
	bottom, 
	back,
	right, 
	top, 
	front; 
} Cube;

bool cube_intersect(Cube a, Cube b)
{
	return 
	a.left < b.right && 
	b.left < a.right &&
	a.bottom < b.top &&
	b.bottom < a.top &&
	a.back < b.front &&
	b.back < a.front;
}

typedef struct
{
	float x,y,z;
} v3;
#define v3_zero ((v3){0, 0, 0})
#define v3_forward ((v3){0, 0, 1})
typedef struct
{
	v3 position;
	v3 rotation;
	v3 scale;
} Transform;

typedef struct
{
	v3 a, b, c;
}
Triangle;

v3 v3_add(v3 a, v3 b)
{    return (v3){ a.x+b.x, a.y+b.y, a.z+b.z }; }

v3 v3_sub(v3 a, v3 b)
{    return (v3){ a.x-b.x, a.y-b.y, a.z-b.z }; }

v3 v3_mult(v3 a, v3 b)
{    return (v3){ a.x*b.x, a.y*b.y, a.z*b.z }; }

v3 v3_scale(v3 v, float s)
{    return (v3){ v.x*s, v.y*s, v.z*s }; }

v3 v3_div(v3 v, float s)
{    return (v3){ v.x/s, v.y/s, v.z/s }; }

float v3_mag(v3 v)
{    return sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }

v3 v3_normalized(v3 v)
{    return v3_div(v, v3_mag(v)); }

float v3_dist(v3 a, v3 b)
{    return v3_mag(v3_sub(a, b)); }

float v3_dot(v3 a, v3 b)
{    return a.x*b.x + a.y*b.y + a.z*b.z; }

float v3_project(v3 a, v3 b)
{    return v3_dot(a,b)/v3_mag(b); }

v3 v3_rotate_yz_plane(v3 v, float t)
{
	return (v3){v.x,(sin(t)*v.z)+(cos(t)*v.y), (cos(t)*v.z)-(sin(t)*v.y)};
}

v3 v3_rotate_xz_plane(v3 v, float t)
{
	return (v3){cos(t)*v.x-sin(t)*v.z, v.y, sin(t)*v.x+cos(t)*v.z};
}


v3 v3_rotate_xy_plane(v3 v, float t)
{
	return (v3){-((-cos(t))*v.x-sin(t)*v.y), ((-sin(t))*v.x+cos(t)*v.y), v.z};
}

typedef struct
{
	int vertex;
	v3 offset;
} Displacement;

typedef struct
{
	int disp_count;
	Displacement *disps;	
} MorphTarget;

//returns the screen space barycentric point represented by a cartesian coordinate and the associated triangle
v3 to_barycentric(Triangle tri, v2i cartesian)
{
	v3 cart = {cartesian.x, cartesian.y, 0};

	v3 bary;
	v3 edge1 = v3_sub(tri.a,tri.c);
	v3 edge2 = v3_sub(tri.b,tri.c);
	v3 p = edge1;
	v3 q = edge2;
	float det = p.x*q.y-p.y*q.x;
	p.x=edge2.y;
	q.y=edge1.x;
	p.y=-p.y;
	q.x=-q.x;
	
	if(abs(det) > 0)
	{
		p = v3_div(p,det);
		q = v3_div(q,det);
	}
	cart = v3_sub(cart,tri.c);
	bary.x = p.x*cart.x+q.x*cart.y;
	bary.y = p.y*cart.x+q.y*cart.y;
	bary.z = 1-(bary.x+bary.y);

	float tol = .000001f;
	if(bary.x  < 0 && bary.x >= -tol)
		bary.x = 0;
	if(bary.y < 0 && bary.y >= -tol)
		bary.y = 0;
	if(bary.z < 0 && bary.z >= -tol)
		bary.z = 0;
	return bary;
}

//returns the barycentric coordinate, but assumes you have performed the basic edge functions of the triangle already (so you can do them just once per triangle)
v3 to_barycentric_quick(v2 origin, v2 p, v2 q, v2 cart)
{
	cart.x-=origin.x;
	cart.y-=origin.y;

	v3 bary;
	bary.x = p.x*cart.x+q.x*cart.y;
	bary.y = p.y*cart.x+q.y*cart.y;
	bary.z = 1-(bary.x+bary.y);

	float tol = .000001f;
	if(bary.x  < 0 && bary.x >= -tol)
		bary.x = 0;
	if(bary.y < 0 && bary.y >= -tol)
		bary.y = 0;
	if(bary.z < 0 && bary.z >= -tol)
		bary.z = 0;
	return bary;
}

//returns the screen space cartesian point represented by a barycentric coordinate and the associated triangle
v2i to_cartesian(Triangle tri, v3 bary)
{
	v3 v = v3_add(v3_add(v3_scale(tri.a, bary.x), v3_scale(tri.b, bary.y)), v3_scale(tri.c, bary.z));
	return (v2i){(int)v.x,(int)v.y};
}

typedef struct
{
	int pixel_index;
	v3 bary;
}
RasterEntry;

typedef struct
{
	int entry_count;
	RasterEntry entries[10000];//todo deal with allocation
} RasterList;

int min_3(int a, int b, int c)
{
	int res = a;
	if(b < res)
		res = b;
	if(c < res)
		res = c;
	return res;
}
int max_3(int a, int b, int c)
{
	int res = a;
	if(b > res)
		res = b;
	if(c > res)
		res = c;
	return res;
}

typedef enum
{
	None,
	Player,
	Enemy,
	Block,
	Pickup,
	Trigger,
} Entity_Type;

typedef enum
{
	Heart,
	Rupee,
	Key,
	Fairy,
	HeartContainer,
	Item,
	Sword,
} PickupType;

typedef enum
{
	Bomb,
	Boomerang,

} ItemType;

typedef struct Entity
{
	Entity_Type entity_type;
	struct Entity* parent;
	Transform transform;
	int health;
	bool do_draw_rect;
	bool solid;
	Color color;
	bool draw_sprite;
	Sprite *sprite;
	PickupType pickup_type;
	int var;
} Entity;

typedef enum
{
	Idle,
	Walking,
	Running,
	RunningBackwards,
	Slashing, 
} PlayerAnimationState;

typedef struct
{
	char text[10];
	void (*action)(void);
} Command;

typedef struct
{
	int cur_health;
	int max_health;
	int cur_magic;
	int max_magic;
	int cur_rupees;
	int max_rupees;
	#define max_entitites 400
	int entity_count;
	Entity entities[max_entitites];
	GameState current_gamestate;
	GameState previous_gamestate;
	Transform camera;
	//assets
	Sprite heart;
	Sprite hearts[4];
	Sprite rupee;
	float delta_time;
	float time;
	GamePad previous_padstate;
	PlayerAnimationState player_animation_state;
	float anim_timer;
	Command *current_command;
	byte z_buffer[vm_width*vm_height];
	Texture global_texture;
	bool draw_hud;
	bool draw_camera_gizmo;
	bool render_scene;
	Triangle mesh_data[20000];
	Color texture_data[100*100];
} GameStatus;

float z_buffer[vm_width*vm_height];

GameStatus *g = (GameStatus *)&mem.RAM[start_address];

void delete_entity(int i)
{
	g->entities[i] = g->entities[--g->entity_count];
}
#define cur_tex (g->global_texture)
#define cur_pix (cur_tex.pixels)

fill(Color col)
{
	for (int i = 0; i < cur_tex.width*cur_tex.height; ++i)
		cur_pix[i] = col;
}

clear()
{
	for (int i = 0; i < cur_tex.width*cur_tex.height; ++i)
		cur_pix[i] = 0;
}

fill_rect(Color color, Rect rect)
{
	int x_min = clamp_int(rect.x, 0, cur_tex.width);
	int x_max = clamp_int(rect.x+rect.width, 0, cur_tex.width);
	int y_min = clamp_int(rect.y, 0, cur_tex.height);
	int y_max = clamp_int(rect.y+rect.height, 0, cur_tex.height);

	for (int _x = x_min; _x < x_max; ++_x)
	for (int _y = y_min; _y < y_max; ++_y)
	{
		cur_pix[_y*cur_tex.width+_x] = color;
	}
}

void fill_circle(Color color, v2i center, float radius)
{
	int x_min = (int)(center.x-radius);
	if(x_min < 0)
		x_min = 0;
	int x_max = (int)(center.x+radius);
	if(x_max > cur_tex.width)
		x_max = cur_tex.width;
	int y_min = (int)(center.y-radius);
	if(y_min < 0)
		y_min = 0;
	int y_max = (int)(center.y+radius);
	if(y_max > cur_tex.height)
		y_max = cur_tex.height;

	for (int x = x_min; x <= x_max; ++x)
	for (int y = y_min; y <= y_max; ++y)	
	{
		float dist_from_center = v2i_dist((v2i){x,y}, center);
		if(dist_from_center <= radius)
		{
			cur_pix[y*cur_tex.width+x] = color;
		}
	}
}

Color lerp_color(Color a, Color b, float t);
void fill_circle2(Color color, v2 center, float radius, float factor)
{
	float padding = 9;
	float pad_rad = radius+padding;
	int x_min = (int)(center.x-(pad_rad));
	if(x_min < 0)
		x_min = 0;
	int x_max = (int)(center.x+pad_rad);
	if(x_max > cur_tex.width)
		x_max = cur_tex.width;
	int y_min = (int)(center.y-pad_rad);
	if(y_min < 0)
		y_min = 0;
	int y_max = (int)(center.y+pad_rad);
	if(y_max > cur_tex.height)
		y_max = cur_tex.height;

	for (int x = x_min; x <= x_max; ++x)
	for (int y = y_min; y <= y_max; ++y)	
	{
		float dist_from_center = v2_dist((v2){x,y}, center);
		if(dist_from_center <= radius)
		{
			cur_pix[y*cur_tex.width+x] = color;
		}
		else
		{
			float t = (dist_from_center-radius)/factor;
			clamp_float(&t,0,1);
			cur_pix[y*cur_tex.width+x] = lerp_color(color, cur_pix[y*cur_tex.width+x], t);
		}
	}
}

draw_tex(Texture tex, int x, int y)
{
	for (int _x = 0; _x < tex.width; ++_x)
	for (int _y = 0; _y < tex.height; ++_y)
	{
		Color col = tex.pixels[_y*tex.width+_x];
		cur_pix[(y+_y)*cur_tex.width+(x+_x)] = col;
	}
}

draw_tex_t(Texture tex, int x, int y)
{
	for (int _x = 0; _x < tex.width; ++_x)
	for (int _y = 0; _y < tex.height; ++_y)
	{
		Color col = tex.pixels[_y*tex.width+_x];
		if(col != 0)
		cur_pix[(y+_y)*cur_tex.width+(x+_x)] = col;
	}
}

draw_sprite_t(Sprite s, int x, int y)
{
	for (int _x = 0; _x < sprite_size; ++_x)
	for (int _y = 0; _y < sprite_size; ++_y)
	{
		Color col = s.pixels[_y*sprite_size+_x];
		if(col != 0)
			cur_pix[(y+_y)*cur_tex.width+(x+_x)] = col;
	}
}

typedef Color (*Shader)(Triangle tri, void *state);

Color foo_colors[9] = {red, green, blue, cyan, magenta, yellow, white, brown,black};
Color test_shader(Triangle tri, void *state)
{
	int i = (int)*state;
	return foo_colors[i%9];
}

v3 centroid(Triangle tri)
{
	return (v3)
	{
		(tri.a.x+tri.b.x+tri.c.x)/3,
		(tri.a.y+tri.b.y+tri.c.y)/3,
		(tri.a.z+tri.b.z+tri.c.z)/3,
	};
}

v3 v3_cross(v3 a, v3 b)
{
	return (v3)
	{
		a.y*b.z-a.z*b.y,
		a.z*b.x-a.x*b.z,
		a.x*b.y-a.y*b.x,
	};
}

v3 light = {0,0,-1};
Color flat_shaded(Triangle tri, void *state)
{
	//float theta =M_PI/4;
	//light = (v3){cos(theta),0,sin(theta)};
	v3 p = v3_normalized(v3_sub(tri.a,tri.c));
	v3 q = v3_normalized(v3_sub(tri.b,tri.c));

	v3 normal = v3_normalized(v3_cross(p,q));
	float d = v3_dot(light, normal);
	d = d < 0 ? 0 : d;
	byte lighting = (byte)(255*d);
	Color col = lighting<<16|lighting<<8|lighting;
	return col;
}

Color terrain_shader(Triangle tri, void *state)
{
	float elevation = centroid(tri).z;

	return ((int)(elevation))<<8;
}
typedef struct
{
	Shader shader;
	void *state;	
} Material;

void shade_triangle(Triangle tri, Material mat)
{
	//bounding box
	int x_min = min_3(tri.a.x, tri.b.x, tri.c.x);
	int x_max = max_3(tri.a.x, tri.b.x, tri.c.x);
	int y_min = min_3(tri.a.y, tri.b.y, tri.c.y);
	int y_max = max_3(tri.a.y, tri.b.y, tri.c.y);
	
	x_min = clamp_int(x_min, 0, cur_tex.width-1);
	x_max = clamp_int(x_max, 0, cur_tex.width-1);
	y_min = clamp_int(y_min, 0, cur_tex.height-1);
	y_max = clamp_int(y_max, 0, cur_tex.height-1);

	v3 edge1 = v3_sub(tri.a,tri.c);
	v3 edge2 = v3_sub(tri.b,tri.c);
	v3 p = edge1;
	v3 q = edge2;
	float det = p.x*q.y-p.y*q.x;
	p.x=edge2.y;
	q.y=edge1.x;
	p.y=-p.y;
	q.x=-q.x;
	if(abs(det) > 0)
	{
		p = v3_div(p,det);
		q = v3_div(q,det);
	}

	for (int y = y_min; y <= y_max; ++y)
	for (int x = x_min; x <= x_max; ++x)
	{
		//v3 bary = to_barycentric(tri,(v2i){x,y});
		v3 bary = to_barycentric_quick((v2){tri.c.x,tri.c.y},(v2){p.x,p.y},(v2){q.x,q.y},(v2){x,y});

		if(bary.x >= 0 && bary.y >= 0 && bary.z >= 0)
		{
			float z = ((bary.x*tri.a.z+bary.y*tri.b.z+bary.z*tri.c.z));

			if(z > z_buffer[y*cur_tex.width+x])
			{
				cur_pix[y*cur_tex.width+x] = mat.shader(tri, mat.state);
				z_buffer[y*cur_tex.width+x] = z;
			};
		}
	}	
}

void fill_triangle(Triangle tri, Color col)
{
	//bounding box
	int x_min = min_3(tri.a.x, tri.b.x, tri.c.x);
	int x_max = max_3(tri.a.x, tri.b.x, tri.c.x);
	int y_min = min_3(tri.a.y, tri.b.y, tri.c.y);
	int y_max = max_3(tri.a.y, tri.b.y, tri.c.y);
	
	x_min = clamp_int(x_min, 0, cur_tex.width-1);
	x_max = clamp_int(x_max, 0, cur_tex.width-1);
	y_min = clamp_int(y_min, 0, cur_tex.height-1);
	y_max = clamp_int(y_max, 0, cur_tex.height-1);

	v3 edge1 = v3_sub(tri.a,tri.c);
	v3 edge2 = v3_sub(tri.b,tri.c);
	v3 p = edge1;
	v3 q = edge2;
	float det = p.x*q.y-p.y*q.x;
	p.x=edge2.y;
	q.y=edge1.x;
	p.y=-p.y;
	q.x=-q.x;
	p = v3_div(p,det);
	q = v3_div(q,det);

	for (int y = y_min; y <= y_max; ++y)
	for (int x = x_min; x <= x_max; ++x)
	{
		//v3 bary = to_barycentric(tri,(v2i){x,y});
		v3 bary = to_barycentric_quick((v2){tri.c.x,tri.c.y},(v2){p.x,p.y},(v2){q.x,q.y},(v2){x,y});

		if(bary.x >= 0 && bary.y >= 0 && bary.z >= 0)
		{
			cur_pix[y*cur_tex.width+x] = col;
		}
	}	
}

RasterList triangle_to_rasterlist(Triangle tri)
{
	//todo bounding box
	RasterList list = {.entry_count = 0};
	for (int y = 0; y < vm_height; ++y)
	for (int x = 0; x < vm_width; ++x)
	{
		v2i v = (v2i){x,y};
		v3 bary = to_barycentric(tri,v);

		if(bary.x >= 0 && bary.y >= 0 && bary.z >= 0)
		{
			list.entries[list.entry_count++] = (RasterEntry){.pixel_index = y*vm_width+x, .bary = bary};
		}
	}

	return list;
}

fill_rasterlist(RasterList list, Color col)
{
	for (int i = 0; i < list.entry_count; ++i)
	{
		mem.frame_buffer.pixels[list.entries[i].pixel_index] = col;
	}
}

void outline(Color fill_color, Color detect_color)
{
	for (int y = 0; y < cur_tex.height; ++y)
	for (int x = 0; x < cur_tex.width; ++x)
	{
		int i = y*cur_tex.width+x;
		bool neighbor_test = false;
		if(x > 0)
			neighbor_test |= cur_pix[i-1] == detect_color;
		if(x < cur_tex.width-1)
			neighbor_test |= cur_pix[i+1] == detect_color;
		if(y > 0)
			neighbor_test |= cur_pix[i-cur_tex.width] == detect_color;
		if(y < cur_tex.height-1)
			neighbor_test |= cur_pix[i+cur_tex.width] == detect_color;

		if(cur_pix[i] == 0 && neighbor_test)
			cur_pix[i] = fill_color;
	}
}

Transform t_from_v_and_s(v3 v, float s)
{
	return (Transform){v.x,v.y,v.z, 0,0,0,s,s,s};
}

#define unit_size 16
void render_rect(Color color, Transform t)
{
	int width = unit_size*t.scale.x;
	int height = unit_size*t.scale.z;
	int half_width = width/2;
	int half_height = height/2;
	fill_rect(color, (Rect){
		vm_width/2-half_width+(t.position.x)*unit_size,
		vm_height/2-half_height-(t.position.z)*unit_size,
		width,
		height});
}

Cube transform_to_cube(Transform t)
{
	Cube c =  (Cube)
	{
		t.position.x-t.scale.x/2,
		t.position.y-t.scale.y/2,
		t.position.z-t.scale.z/2,
		t.position.x+t.scale.x/2,
		t.position.y+t.scale.y/2,
		t.position.z+t.scale.z/2,
	};
	return c;
}

print_cube(Cube c)
{
	printf("cube: { ");
	float *p = (float*)&c;
	for (int i = 0; i < 6; ++i)
	{
		printf("%f, ", p[i]);
	}
	printf(" }\n");	
}

print_v3(char* label, v3 v)
{
	printf("%s: { %f, %f, %f }\n", label, v.x, v.y, v.z);
}
u32 mesh_cursor = 0;

typedef struct
{
	u32 index;
	u32 count;
} Mesh;

Mesh terrain_mesh;

print_triangle(char* label, Triangle t)
{
	printf("%s\n", label);
	print_v3("\ta", t.a);
	print_v3("\tb", t.b);
	print_v3("\tc", t.c);
}
Entity block(Transform t)
{
	return 
	(Entity)
	{
		.entity_type = Block,
		.transform = t,
		.do_draw_rect = true,
		.color = 0xAAAAAA,
		.solid = true,
	};
}

Entity rupee(v3 position)
{
	return (Entity)
	{
		.entity_type = Pickup,
		.pickup_type = Rupee,
		.transform = (Transform){.position = position,0,0,0,.5f,.5f,1},
		.draw_sprite = true,
		.color = blue,
		.sprite = &g->rupee,
		.var = 1,
	};	
}

#define player (g->entities[0])
#define default_transform {0,0,0,0,0,0,1,1,1}
Mesh generate_terrain(float scale_x, float scale_z, int subdivs_x, int subdivs_z);
void draw_terrain();

#define screen_texture ((Texture) { vm_width, vm_height, mem.frame_buffer.pixels })
#define heart_texture ((Texture){sprite_size,sprite_size, g->heart.pixels})
void regenerate_heart_sprites()
{
	cur_tex = heart_texture;

	for (int o = 0; o < 3; ++o)
	for (int i = 0; i < sprite_size*sprite_size; ++i)
	{
		g->heart.pixels[i] = 0;
		g->hearts[0].pixels[i] = 0;
		g->hearts[1].pixels[i] = 0;
		g->hearts[2].pixels[i] = 0;
		g->hearts[3].pixels[i] = 0;
		//next_sprite();
	}
	cur_tex = heart_texture;
	int x_offset = 3;
	int y_offset = 3;
	Triangle tri = (Triangle)
	{
		.a = {x_offset+1, y_offset+7,0},
		.b = {x_offset+8, y_offset+14,0},
		.c = {x_offset+15,y_offset+7,0},
	};
	fill_triangle(tri,red);
	fill_rect(red,(Rect){x_offset+4,y_offset+5,8,4});
	float pulse_radius = (sin(g->time*6)+1)*.55f;
	fill_circle(red,(v2i){x_offset+4,y_offset+4}, 4+pulse_radius);
	fill_circle(red,(v2i){x_offset+12,y_offset+4}, 4+pulse_radius);
	outline(0xFF880000, red);
	outline(black, 0xFF880000);
	fill_circle(0xFFFF4444,(v2i){x_offset+4,y_offset+4}, 3.5f);
	fill_circle(0xFFFF8888,(v2i){x_offset+4,y_offset+4}, 2);

	#define next_sprite() cur_pix += sizeof(Sprite)/sizeof(Color)
	next_sprite();
	draw_sprite_t(g->heart,0,0);
	next_sprite();
	draw_sprite_t(g->heart,0,0);
	next_sprite();
	draw_sprite_t(g->heart,0,0);
	next_sprite();
	draw_sprite_t(g->heart,0,0);

	for (int y = 0; y < sprite_size; ++y)
	for (int x = 0; x < sprite_size; ++x)
	{
		int i = y*sprite_size+x;
		if((g->hearts[0].pixels[i] & red) == red)
			g->hearts[0].pixels[i] = white;

		if((g->hearts[1].pixels[i] & red) == red && (x > x_offset+16/2 || y > y_offset+16/2))
			g->hearts[1].pixels[i] = white;

		if((g->hearts[2].pixels[i] & red) == red && x > x_offset+16/2)
			g->hearts[2].pixels[i] = white;

		if(((g->hearts[3].pixels[i] & red) == red) && (x > x_offset+16/2 && y < y_offset+16/2))
			g->hearts[3].pixels[i] = white;
	}

	cur_tex = screen_texture;
}

init()
{
	*g = (GameStatus)
	{
		.cur_health = 20,
		.max_health = 40,
		.cur_magic = 100,
		.max_magic = 100,
		.entity_count = 11,
		.entities =
		{
			(Entity)
			{
				.entity_type = Player,
				.transform = (Transform){1,0,1,0,0,0,1,1,1},
				.do_draw_rect = true,
				.color = green,
			},
			block((Transform){4.5f,0,0,0,0,0,10,1,1}),
			block((Transform){0,0,5.5f,0,0,0,1,1,10}),
			(Entity)
			{
				.entity_type = Enemy,
				.transform = (Transform){ {4,0,-1},.scale = {1,1,1}},
				.do_draw_rect = true,
				.color = red,
				.solid = true,
			},
			(Entity)
			{
				.entity_type = Pickup,
				.pickup_type = Heart,
				.transform = (Transform){4,0,-2,0,0,0,.5f,.5f,.5f},
				//.do_draw_rect = true,
				.draw_sprite = true,
				.sprite = &g->heart,
				.color = 0xFFAA00,
			},
			(Entity)
			{
				.entity_type = Pickup,
				.pickup_type = HeartContainer,
				.transform = (Transform){6,0,-3,0,0,0,1,1,1},
				.do_draw_rect = true,
				.color = red,
			},
			rupee((v3){-2,0,-3}),
			rupee((v3){-3,0,-3}),
			rupee((v3){-4,0,-3}),
			rupee((v3){-5,0,-3}),
			rupee((v3){-6,0,-3}),										
		},
		.global_texture = screen_texture,
		.draw_hud = true,
		.render_scene = true,
		.draw_camera_gizmo = false,
		.camera = (Transform){0,0,-1,0,0,0,.2,.2,.2},
	};

	#define rupee_texture ((Texture){sprite_size,sprite_size, g->rupee.pixels})
	int x = 2;
	int y = 2;
	cur_tex = rupee_texture;

	Triangle tri = (Triangle)
	{
		.a = {x+16/2, y+0,0},
		.b = {x+4,    y+4,0},
		.c = {x+16-4, y+4,0},
	};

	fill_triangle(tri,green);
	fill_rect(green,(Rect){x+4,y+4,9,8});
	tri = (Triangle)
	{
		.a = {x+16/2,y+16-0,0},
		.b = {x+16-4,y+16-4,0},
		.c = {x+4,y+16-4,0},
	};
	fill_triangle(tri,green);
	outline(0x007700, green);
	outline(1,0x007700);
	regenerate_heart_sprites();
	cur_tex = screen_texture;


	terrain_mesh = generate_terrain(16,16,16,16);
	mesh_cursor = 0;
}

memset_u32_4wide(u32 *p, int value, int count)
{
	for (int i = 0; i < count; i+=4)
	{
		p[i]   = value;
		p[i+1] = value;
		p[i+2] = value;
		p[i+3] = value;
	}
}

float lerp(float a, float b, float t)
{
	return a+t*(b-a);
}

v3 v3_lerp(v3 a, v3 b, float t)
{
	 return (v3)
	 {
	 	lerp(a.x, b.x, t),
	 	lerp(a.y, b.y, t),
 		lerp(a.z, b.z, t),
	 };
}

Color lerp_color(Color a, Color b, float t)
{
	//separate out channels
	u32 a_a = (0xFF000000 & a) >> 24;
	u32 a_r = (0x00FF0000 & a) >> 16;
	u32 a_g = (0x0000FF00 & a) >> 8;
	u32 a_b = (0x000000FF & a);
	u32 b_a = (0xFF000000 & b) >> 24;
	u32 b_r = (0x00FF0000 & b) >> 16;
	u32 b_g = (0x0000FF00 & b) >> 8;
	u32 b_b = (0x000000FF & b);

	//lerp per channel
	u32 l_a = (u32)(((float)a_a) + (t * ((float)b_a - (float)a_a)));
	u32 l_r = (u32)(((float)a_r) + (t * ((float)b_r - (float)a_r)));
	u32 l_g = (u32)(((float)a_g) + (t * ((float)b_g - (float)a_g)));
	u32 l_b = (u32)(((float)a_b) + (t * ((float)b_b - (float)a_b)));


	//align lerped channels
	l_a <<= 24;
	l_r <<= 16;
	l_g <<= 8;


	//reassemble channels
	u32 l = l_a | l_r | l_g | l_b;
	return l;
}

gradient(Color a, Color b, int h)
{
	Color col = a;

	for (int y = 0; y < h; ++y)
	{
		float t = y/(float)h;
		col = lerp_color(a, b, t);
		memset_u32_4wide(&cur_pix[y*cur_tex.width], col, cur_tex.width);
	}
}

sky()
{
	gradient(0x0055FF, white,100);
}

mountains()
{
	fill_rect(brown,(Rect){0,100,vm_width,vm_height-100});
}



bool line_circle_intersect(v3 start, v3 end, v3 center, float radius)
{
	bool ret = false;

	v3 a = v3_sub(center,start);
	v3 b = v3_sub(end,start);
	v3 c = v3_scale(v3_normalized(b), v3_project(a,b));
	c = v3_add(c,start);

	if(
	(v3_dist(start,center) < radius) ||
	(v3_dist(end,center) < radius) ||
	((v3_dist(center,c) < radius) && (c.x >= start.x && c.x <= end.x)))
		return true;

	return false;
}

sun()
{
	int div= 12;
	fill_circle2(yellow, (v2){(vm_width/2+cos(3.14159f+g->time/div)*((vm_width-40)/2)),120-sin(g->time/div)*100}, 19.9f, 8);
}




splash_screen()
{
	//todo brushfire logo
	//fill_circle(red,(v2i){4,4},4.9f);
	//fill_circle(red,(v2i){12,4},4.9f);

	//fade up from black, show brushfire logo, fade to black, transition to title screen
	g->current_gamestate++;
}

title_screen()
{

	//draw game logo, press start text fading in and out, sword in stone in background in forest, animated glint on logo text, stretch goal fade to cut scene
	//draw_text("press start",)
	g->current_gamestate++;

}

file_select()
{
	g->current_gamestate++;
}


field()
{

	v3 forward;
	v3 right;
	v3 hand;
	v3 sword_tip;
	static v3 player_forward = v3_forward;

	terrain_mesh = generate_terrain(16,16,16,16);
	mesh_cursor = 0;
	regenerate_heart_sprites();
	cur_tex=screen_texture;
	//update
	{
		if(ButtonDown(START) && !button_down(g->previous_padstate,START))
		{
			g->previous_gamestate = Field;
			g->current_gamestate = Paused;
			return;
		}

		//player motion
		{
			//note:for now assume player is entity 0, this may change
			float speed = 7;
			v3 move_vector = v3_zero;

			if(ButtonDown(UP))
				move_vector.z = 1;
			else if (ButtonDown(DOWN))
				move_vector.z = -1;
			if (ButtonDown(LEFT))
				move_vector.x = -1;
			else if(ButtonDown(RIGHT))
				move_vector.x = 1;

			char deadzone = 20;
			if(abs(mem.game_pads[0].sticks.left_stick.X) > deadzone || abs(mem.game_pads[0].sticks.left_stick.Y) > deadzone)
				move_vector = (v3){mem.game_pads[0].sticks.left_stick.X/128.0f, 0, mem.game_pads[0].sticks.left_stick.Y/128.0f};

			if(v3_mag(move_vector) > 1)
			{
				move_vector = v3_normalized(move_vector);
			}

			float follow_distance = 5;
			forward = v3_sub(player.transform.position, g->camera.position);
			if(v3_mag(forward) > 0)
				forward = v3_normalized(forward);
			right = (v3){forward.z, 0, -forward.x};

			//temporarily align with axes while in 2D as a bodge, delete these 2 lines when returning to 3d 
			forward = (v3){0,0,1};
			right = (v3){1,0,0};

			//transform move_vector to be camera relative
			v3 forward2 = v3_scale(forward,move_vector.z);
			v3 right2 = v3_scale(right, move_vector.x);
			move_vector = v3_add(forward2,right2);

			//for now
			if(v3_mag(move_vector) > 0) player_forward = move_vector;

			move_vector = v3_scale(move_vector, speed);
			move_vector = v3_scale(move_vector, g->delta_time);


			//apply motion and handle collision
			{
				#define t1 player.transform
				#define t2 g->entities[i].transform
				v3 old = t1.position;
				t1.position.x += move_vector.x;

				for (int i = 1; i < g->entity_count; ++i)
				{
					if(g->entities[i].solid)
					if(cube_intersect(transform_to_cube(t1), transform_to_cube(t2)))
					{
						if(old.x < t2.position.x)
							t1.position.x = t2.position.x-((t1.scale.x+t2.scale.x)/2);
						else
							t1.position.x = t2.position.x+((t1.scale.x+t2.scale.x)/2);
					}
				}


				old = t1.position;
				t1.position.y += move_vector.y;
				
				for (int i = 1; i < g->entity_count; ++i)
				{
					if(g->entities[i].solid)
					if(cube_intersect(transform_to_cube(t1), transform_to_cube(t2)))
					{
						if(old.y < t2.position.y)
							t1.position.y = t2.position.y-((t1.scale.y+t2.scale.y)/2);
						else
							t1.position.y = t2.position.y+((t1.scale.y+t2.scale.y)/2);
					}
				}

				old = t1.position;
				t1.position.z += move_vector.z;
				
				for (int i = 1; i < g->entity_count; ++i)
				{
					if(g->entities[i].solid)
					if(cube_intersect(transform_to_cube(t1), transform_to_cube(t2)))
					{
						if(old.z < t2.position.z)
							t1.position.z = t2.position.z-((t1.scale.z+t2.scale.z)/2);
						else
							t1.position.z = t2.position.z+((t1.scale.z+t2.scale.z)/2);
					}
				}
				
				#undef t1
				#undef t2
			}
			
			//lerp camera to target position
			v3 target = v3_sub(player.transform.position, v3_scale(forward,follow_distance));
			g->camera.position = v3_lerp(g->camera.position, target,g->delta_time);

			right = v3_add(right, g->camera.position);
			forward = v3_add(forward, g->camera.position);

			player.transform.rotation.y = atan2(player_forward.z, player_forward.x)-M_PI/2;
			hand = (v3){-.5,0,1};
			hand = v3_rotate_xz_plane(hand, player.transform.rotation.y);
			hand = v3_add(player.transform.position,hand);
			sword_tip = (v3){0,0,1};
			sword_tip = v3_rotate_xz_plane(sword_tip, player.transform.rotation.y);
			sword_tip = v3_add(sword_tip,hand);

			for (int i = 1; i < g->entity_count; ++i)
			{
				if(cube_intersect(transform_to_cube(player.transform),transform_to_cube(g->entities[i].transform)))
				{
					if(g->entities[i].entity_type == Pickup)
					{
						switch(g->entities[i].pickup_type)
						{
							case Rupee:
							{
								g->cur_rupees+=g->entities[i].var;
								delete_entity(i);
								continue;
							} break;
							case Fairy:
							{
								g->cur_health = g->max_health;
								delete_entity(i);
								continue;
							} break;
							case HeartContainer:
							{
								g->max_health += 4;
								if(g->max_health > 80)
									g->max_health = 80;
								delete_entity(i);
								continue;
							} break;
							case Heart:
							{
								g->cur_health ++;
								if(g->cur_health > g->max_health)
									g->cur_health = g->max_health;
								delete_entity(i);
								continue;
							} break;
						}
					}
				}

				if(g->entities[i].entity_type == Enemy)
				if(g->player_animation_state == Slashing && line_circle_intersect(hand, sword_tip, g->entities[i].transform.position, g->entities[i].transform.scale.x/2))
				{
					g->entities[i].health--;
					if(g->entities[i].health<=0)
						g->entities[i] = g->entities[--(g->entity_count)];
				}
			}
		}

		if(ButtonDown(X) && g->player_animation_state != Slashing)
		{
			g->player_animation_state = Slashing;
			g->anim_timer = .25f;
		}
		
		if(ButtonDown(A))
		{
			if(g->current_command != NULL)
				g->current_command->action();
		}

		if(g->anim_timer > 0)
			g->anim_timer-=g->delta_time;
		else 
			g->player_animation_state = Idle;
	}

	//render
	{
		for (int i = 0; i < vm_width*vm_height; ++i)
		{
			z_buffer[i] = -1000;
		}
		//scene
		if(g->render_scene)
		{
			//environment
			{
				sky();
				sun();
				mountains();
				draw_terrain();
				//render_mesh(terrain_mesh, (Material){terrain_shader},(Transform)default_transform);
			}

			for (int i = 0; i < g->entity_count; ++i)
			{
				if(g->entities[i].draw_sprite)
					draw_sprite_t(*(g->entities[i].sprite),(vm_width-sprite_size)/2+g->entities[i].transform.position.x*unit_size,(vm_height-sprite_size)/2-g->entities[i].transform.position.z*unit_size);
				if(g->entities[i].do_draw_rect)
					render_rect(g->entities[i].color, g->entities[i].transform);
			}

			if(g->draw_camera_gizmo)
			{
				render_rect(green, g->camera);
				render_rect(blue, t_from_v_and_s(forward,.2f));
				render_rect(red, t_from_v_and_s(right,.2f));
			}

			if(g->player_animation_state == Slashing)
			{
				render_rect(0x555555, t_from_v_and_s(hand,.3f));
				render_rect(0x555555, t_from_v_and_s(sword_tip,.3f));
			}
		}
		else
			fill(blue);

		//HUD
		if(g->draw_hud)
		{
			static int prev_health = -1;
			if(true)//(prev_health != g->cur_health)
			{
				//health bar
				{
					int full_hearts = g->cur_health / 4; 
					int total_hearts = g->max_health / 4;
					int partial =  g->cur_health % 4;

					int heart_width = sprite_size;
					int x_offset = 6;
					int y_offset = 6;
					int x_padding = 2;
					int y_padding = -3;
					int hearts_per_row = 10;

					int i;
					for (i = 0; i < full_hearts; ++i)
					{
						draw_sprite_t(g->heart, x_offset+(i%hearts_per_row)*(heart_width+x_padding),y_offset+(i/hearts_per_row)*(heart_width+y_padding));
					}

					if(partial){
						draw_sprite_t(g->hearts[partial], x_offset+(i%hearts_per_row)*(heart_width+x_padding),y_offset+(i/hearts_per_row)*(heart_width+y_padding));
						i++;
					}
					for (; i < total_hearts; ++i)
					{
						draw_sprite_t(g->hearts[0], x_offset+(i%hearts_per_row)*(heart_width+x_padding),y_offset+(i/hearts_per_row)*(heart_width+y_padding));
					}
				}

				//magic bar
				{
					int bar_width = 150;
					int bar_height = 12;
					int border_size = 2;
					int x = 8;
					int y = 52;
					fill_rect(0xFF151515, (Rect){x-3, y-3, bar_width+(2*3), bar_height+(2*3)});
					fill_rect(0xFF444444, (Rect){x-border_size, y-border_size, bar_width+(2*border_size), bar_height+(2*border_size)});
					fill_rect(0xFF222222, (Rect){x-1, y-1, bar_width+(2), bar_height+(2)});
					fill_rect(red, (Rect){x, y, bar_width, bar_height});
					//fill_rect(green, (Rect){x, y, (int)((g->cur_magic/(float)g->max_magic)*bar_width), bar_height});
					for (int _y = y; _y < y+bar_height; ++_y)
					for (int _x = x; _x < x+(int)((g->cur_magic/(float)g->max_magic)*bar_width); ++_x)
					{
						cur_pix[_y*cur_tex.width+_x] = (int)(255-((sin(_x/12.5f)+1)*64))<<8;
					}			
				}

				//wallet
				{
					fill_rect(green, (Rect){0,vm_height-5,g->cur_rupees,5});
					//todo grab fontset file from platfighter project for text drawing
				}

				//command buttons
				{
					fill_circle2(black, (v2){vm_width/2+32,48}, 35, 1.5f);
					fill_circle2(blue, (v2){vm_width/2+32,48}, 33, 1);

					//todo render
					{
						if(g->current_command != NULL)
						printf(g->current_command->text);
					}
				}

				//item buttons
				{
					float rad = 16;
					fill_circle2(yellow,(v2){vm_width-(32+32+32+32),32}, rad, 1);
					fill_circle2(yellow,(v2){vm_width-(32+32+32), 64}, rad, 1);
					fill_circle2(yellow,(v2){vm_width-(32+32),32}, rad, 1);
				}
				
				cur_tex = screen_texture;
			}

			prev_health = g->cur_health;
		}
	}
}

dungeon()
{

}

paused()
{
	fill_rect(red,(Rect){10,10,vm_width-20,vm_height-20});
	if(ButtonDown(START) && !button_down(g->previous_padstate,START)){
		g->current_gamestate = g->previous_gamestate;
		g->previous_gamestate = 0;
	}

}

eye(v2 eye_pos)
{
	fill_circle2(0xFF555555, eye_pos, 15.5f, .8f);	
	fill_circle2(white, eye_pos, 12, 4);	
	fill_circle2(0xFF000044, eye_pos, 7.5f, 1);
	fill_circle2(blue, eye_pos, 6,1);
	fill_circle2(black, eye_pos, 3,1.5f);
	fill_circle2(white, (v2){eye_pos.x-3,eye_pos.y-2}, 1.5f, 1.6f);
}

face()
{
	v2 eye_pos = {82,40};
	for (int i = 0; i < cur_tex.width*cur_tex.height; ++i)
	{
		cur_pix[i] = 0;
	}
	
	fill_circle2(0xFFFFCCAA,(v2){100,41}, 38.5f, 2);
	
	fill_circle2(0xFFFFCCAA,(v2){100,65}, 19.9f, 2);
	fill_circle(0xFFFFAA88, (v2i){100,60}, 12.9f);

	eye(eye_pos);
	eye_pos.x+=37;
	eye(eye_pos);

	fill_rect(0xFFFFCCAA,(Rect){67,24,67,11});
	fill_rect(0xFFFFCCAA,(Rect){68,50,65,10});
	fill_rect(black,(Rect){66,28,19,6});
	fill_rect(black,(Rect){65,30,30,5});
	fill_rect(black,(Rect){65+42+10,28,19,6});
	fill_rect(black,(Rect){65+42,31,30,4});
}

int vertex_count = 0;
v3 vertices[20000];
int index_count;
int indices[100000];
int triangle_count = 0;
Triangle triangles[100000];



Mesh cap(v3 origin, float radius, int divisions)
{
	#define TAU (M_PI*2)
	u32 old_cursor = mesh_cursor;	
	for (int i = 0; i < divisions; ++i)
	{
		float theta = i/(float)divisions*TAU;
		float theta2 = (i+1)/(float)divisions*TAU;

		g->mesh_data[mesh_cursor++] = (Triangle)
		{
			origin,
			{ origin.x + cos(theta)*radius,  origin.y, origin.z + sin(theta)*radius },
			{ origin.x + cos(theta2)*radius, origin.y, origin.z + sin(theta2)*radius },
		};
	}

	return (Mesh){.index = old_cursor, .count = divisions};
}

void rot_mesh(Mesh mesh, v3 euler);

Mesh cone(v3 origin, float radius, int height, int divisions)
{
	Mesh mesh = cap(origin, radius,divisions);
	Mesh mesh_2 = cap(v3_zero, radius,divisions);
	rot_mesh(mesh_2,(v3){M_PI,0,0});
	//todo used indexed mesh rather than triangle triplet to avoid having to iterate multiple triangles
	for (int i = 0; i < mesh.count; ++i)
	{
		g->mesh_data[mesh.index+i].a.y += height;
	}

	for (int i = 0; i < mesh_2.count; ++i)
	{
		g->mesh_data[mesh_2.index+i].a.y += origin.y;
		g->mesh_data[mesh_2.index+i].b.y += origin.y;
		g->mesh_data[mesh_2.index+i].c.y += origin.y;
	}
	
	mesh.count+=mesh_2.count;
	return mesh;
}

v3 v3_rotate(v3 v, v3 euler)
{
	v = v3_rotate_yz_plane(v,euler.x);
	v = v3_rotate_xz_plane(v,euler.y);
	v = v3_rotate_xy_plane(v,euler.z);
	return v;
}

void rot_mesh(Mesh mesh, v3 euler)
{
	v3 *v= (v3*)&g->mesh_data[mesh.index];
	
	for (int i = 0; i < mesh.count*3; ++i)
	{
		v[i] = v3_rotate(v[i], euler);
	}
}

v3 transform_point(v3 v, Transform t)
{
	v = v3_mult(v,t.scale);
	v = v3_rotate(v,t.rotation);
	v = v3_add(v, t.position);
	return v;
}

render_mesh(Mesh mesh, Material mat, Transform tr)
{
	//transform
	{

		float *f = (float*)&g->mesh_data[mesh.index];

		for (int i = 0; i < mesh.count*9; ++i)
		{
			f[i] *= unit_size;
		}

		v3 *v = (v3*)&g->mesh_data[mesh.index];
		for (int i = 0; i < mesh.count*3; ++i)
		{
			v[i] = transform_point(v[i], tr);
		}

		for (int i = 0; i < mesh.count*3; ++i)
		{

			float temp = v[i].y;
			v[i].y = v[i].z;
			v[i].z = temp;
			
			v[i].y = -v[i].y;
			v[i].x+= vm_width/2;
			v[i].y+= vm_height/2;

			v[i].z+=3;
			v[i].z/=2;
		}
	}

	//rasterize
	{
		for (int i = 0; i < mesh.count; ++i)
		{
			shade_triangle(g->mesh_data[mesh.index+i], mat);
		}
	}
}

void mesh_test()
{
	clear();
	for (int i = 0; i < vm_width*vm_height; ++i)
	{
		z_buffer[i] = -1000;
	}
	static int foo = 8;
	static float t =0;
	t+=g->delta_time;

	if(t>1.5f){
		t=0;
		foo++;
	}
	float height = 2;
	float half_height = height/2;
	static float rad= 3;
	Mesh cylinder_cap = cap((v3){0,-half_height, 0}, rad, foo);
	Mesh cylinder_cap2 = cone((v3){0, half_height, 0}, rad, height, foo);
	static float fleep;

	fleep+=g->delta_time;
	cylinder_cap.count+=cylinder_cap2.count;
	render_mesh(cylinder_cap, (Material){flat_shaded},(Transform){cos(fleep),0,0,0,0,fleep,1,4,1});
	if(GetAsyncKeyState('F'))
	for (int i = 0; i < vm_width*vm_height; ++i)
	{
		if(z_buffer[i] != -1000){
			z_buffer[i]+=55;
			cur_pix[i]=(int)z_buffer[i];

		}
	}

	if(fleep > TAU)
		fleep-=TAU;

	mesh_cursor = 0;
}

void (*scenes[scene_count])(void) = 
{
	[MeshTest] = &mesh_test,
	[Face] = &face,
	[SplashScreen] = &splash_screen,
	[TitleScreen] = &title_screen,
	[FileSelect] = &file_select,
	[Dungeon] = &dungeon,
	[Field] = &field,
	[Paused] = &paused,
};

void _tick()
{
	static bool was = false;
	if(GetAsyncKeyState(VK_TAB))
	{
		if(!was)
		{
			was = true;
			g->current_gamestate = (g->current_gamestate + 1) % scene_count;
		}
	}
	else
	{
		was = false;
	}
	(scenes[g->current_gamestate])();
	g->previous_padstate = mem.game_pads[0];
	g->time += g->delta_time;
}

Mesh generate_terrain(float scale_x, float scale_z, int subdivs_x, int subdivs_z)
{
	int verts_wide = subdivs_x+1;
	int verts_deep = subdivs_z+1;
	vertex_count = (verts_wide)*(verts_deep);
	index_count = 6*subdivs_x*subdivs_z;


	//todo delete this
	float t = 2.5f;

	v2i floop = {(int)(sin(t)*10),6};
	float rad = t;

	float height = (cos(t/10)+1)*100;
	for (int z = 0; z < verts_deep; ++z)
	for (int x = 0; x < verts_wide; ++x)
	{
		float sc_x = (x/(float)subdivs_x)*scale_x;
		float sc_z = (z/(float)subdivs_z)*scale_z;
		float first_octave = .01f;
		float off =.2f;
		float second_octave = 0;
		float dist = v2i_dist((v2i){x,z}, floop);
		if(dist < rad){
			if(dist > 0)
				first_octave += height/dist;
			else
				first_octave+=height;
		}
		vertices[z*verts_wide+x] = (v3){sc_x,first_octave+second_octave, sc_z};
	}

	int origin = 0;
	for (int i = 0; i < index_count; i+=6)
	{
		//if origin vertex at end of row, skip and start next row
		if(origin > 0 && (((i/6) % subdivs_x) == 0))
			origin++;
		indices[i] = origin;
		indices[i+1] = origin+1;
		indices[i+2] = origin+subdivs_x+1;
		indices[i+3] = origin+subdivs_x+2;
		indices[i+4] = origin+subdivs_x+1;
		indices[i+5] = origin+1;

		origin++;
	}

	triangle_count = index_count/3;

	for (int i = 0; i < index_count; i+=3)
	{
		v3 a = vertices[indices[i+0]];
		v3 b = vertices[indices[i+1]];
		v3 c = vertices[indices[i+2]];

		g->mesh_data[mesh_cursor+i/3] = (Triangle)
		{
			{a.x,a.y,a.z},
			{b.x,b.y,b.z},
			{c.x,c.y,c.z},
		};
	}

	Mesh result = (Mesh){.index=mesh_cursor,.count=triangle_count};
	mesh_cursor+=triangle_count;
	return result;
}

Triangle render_tris[10000];
void draw_terrain()
{
	v3 foo = (v3){player.transform.position.x,player.transform.position.z, 0};
	int column = floor(player.transform.position.x);
	int row = floor(player.transform.position.z);

	int index = -1;
	if(!(row < 0 || column < 0))
	{
		float foo_x = foo.x-(int)foo.x;//fractional part
		float foo_y = foo.y-(int)foo.y;//fractional part
		index = (row*32+column*2)*1;
		if(foo_x+foo_y > 1)
			index++;
	}

	if(index >= 0)
	//move player
	{
		Triangle tri = g->mesh_data[terrain_mesh.index+index];
		tri = (Triangle)
		{
			{vm_width/2+tri.b.x*unit_size,vm_height/2-tri.b.z*unit_size, tri.b.y},
			{vm_width/2+tri.c.x*unit_size,vm_height/2-tri.c.z*unit_size, tri.c.y},
			{vm_width/2+tri.a.x*unit_size,vm_height/2-tri.a.z*unit_size, tri.a.y},
		};

		v2i foo = (v2i)
		{
			vm_width/2+player.transform.position.x*unit_size,
			vm_height/2-player.transform.position.z*unit_size
		};

		v3 bary = to_barycentric(tri,foo);
		print_v3("bary",bary);

		float y = bary.x*tri.a.z+bary.y*tri.c.z+bary.z*tri.b.z;
		player.transform.position.y = y;
		printf("%f\n",y);
	}

	for (int i = 0; i < triangle_count; ++i)
	{
		Triangle tri = g->mesh_data[terrain_mesh.index+i];
		render_tris[i] = (Triangle)
		{
			{vm_width/2+tri.a.x*unit_size,vm_height/2-tri.a.z*unit_size, tri.a.y},
			{vm_width/2+tri.b.x*unit_size,vm_height/2-tri.b.z*unit_size, tri.b.y},
			{vm_width/2+tri.c.x*unit_size,vm_height/2-tri.c.z*unit_size, tri.c.y},
		};
	}

	for (int i = 0; i < triangle_count; ++i)
	{
		Triangle tri = g->mesh_data[terrain_mesh.index+i];
		bool player_above = (index == i);
		Color col = (player_above) ? red : ((int)((tri.a.y+tri.b.y+tri.c.y)/3) << 8);
		fill_triangle(render_tris[i], col);
	}
}