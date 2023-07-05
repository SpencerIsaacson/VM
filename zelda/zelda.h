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
} Texture;

#define sprite_size 16
typedef struct
{
	Color pixels[sprite_size*sprite_size];
} Sprite;

draw_tex_t(Texture *tex, int x, int y)
{
	for (int _x = 0; _x < tex->width; ++_x)
	for (int _y = 0; _y < tex->height; ++_y)
	{
		Color col = tex->pixels[_y*tex->width+_x];
		if(col != white)
		mem.frame_buffer.pixels[(y+_y)*vm_width+(x+_x)] = col;
	}
}

draw_sprite_t(Sprite s, int x, int y)
{
	for (int _x = 0; _x < sprite_size; ++_x)
	for (int _y = 0; _y < sprite_size; ++_y)
	{
		Color col = s.pixels[_y*sprite_size+_x];
		if(col != 0)
			mem.frame_buffer.pixels[(y+_y)*vm_width+(x+_x)] = col;
	}
}

int clamp_int(int val, int min, int max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

typedef enum
{
	SplashScreen,
	TitleScreen,
	FileSelect,
	Field,
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

v3 v3_rotate_xz_plane(v3 v, float t)
{
	return (v3){cos(t)*v.x-sin(t)*v.z, v.y, sin(t)*v.x+cos(t)*v.z};
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
	bool skip_draw;
	bool solid;
	Color color;
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
	float delta_time;
	GamePad previous_padstate;
	PlayerAnimationState player_animation_state;
	float anim_timer;
	Command *current_command;
} GameStatus;


GameStatus *g = (GameStatus *)&mem.RAM[start_address];

fill_rect(Color color, Rect rect)
{
	int x_min = clamp_int(rect.x, 0, vm_width);
	int x_max = clamp_int(rect.x+rect.width, 0, vm_width);
	int y_min = clamp_int(rect.y, 0, vm_height);
	int y_max = clamp_int(rect.y+rect.height, 0, vm_height);

	for (int _x = x_min; _x < x_max; ++_x)
	for (int _y = y_min; _y < y_max; ++_y)
	{
		mem.frame_buffer.pixels[_y*vm_width+_x] = color;
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

Entity block(Transform t)
{
	return 
	(Entity)
	{
		.entity_type = Block,
		.transform = t,
		.color = 0xAAAAAA,
		.solid = true,
	};
}

#define player (g->entities[0])
#define default_transform {0,0,0,0,0,0,1,1,1}
void generate_terrain(int subdivs_x, int subdivs_z);
void draw_terrain();

init()
{
	*g = (GameStatus)
	{
		.cur_health = 20,
		.max_health = 40,
		.cur_magic = 100,
		.max_magic = 100,
		.current_gamestate = Field,
		.entity_count = 11,
		.entities =
		{
			{
				.entity_type = Player,
				.transform = default_transform,
				.color = green,
			},
			block((Transform){-3,0,1,0,0,0,1,1,1}),
			block((Transform){-2,0,2,0,0,0,1,1,1}),
			block((Transform){-1,0,3,0,0,0,1,1,1}),
			block((Transform){-0,0,4,0,0,0,1,1,1}),
			block((Transform){-1,0,5,0,0,0,1,1,1}),
			block((Transform){-2,0,6,0,0,0,1,1,1}),
			{
				.entity_type = Enemy,
				.transform = (Transform){4,0,0,0,0,0,1,1,1},
				.color = red,
				.solid = true,
			},
			{
				.entity_type = Pickup,
				.pickup_type = Heart,
				.transform = (Transform){4,0,-2,0,0,0,.5f,.5f,.5f},
				.color = red,
				.solid = false,
			},
			{
				.entity_type = Pickup,
				.pickup_type = HeartContainer,
				.transform = (Transform){4,0,-3,0,0,0,1,1,1},
				.color = red,
				.solid = false,
			},
			{
				.entity_type = Pickup,
				.pickup_type = Rupee,
				.transform = (Transform){-2,0,-3,0,0,0,.5f,.5f,1},
				.color = green,
				.solid = false,
				.var = 1,
			},											
		},
	};


	g->camera = (Transform){0,0,-1,0,0,0,.2,.2,.2};
	for (int i = 0; i < sprite_size*sprite_size; ++i)
	{
		g->heart.pixels[i] = red;
		g->hearts[0].pixels[i] = white;
		g->hearts[1].pixels[i] = 0xFFAAAA;
		g->hearts[2].pixels[i] = 0xFF8888;
		g->hearts[3].pixels[i] = 0xFF5555;
	}

	generate_terrain(4,4);
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
		memset_u32_4wide(&mem.frame_buffer.pixels[y*vm_width], col, vm_width);
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

fill_circle(Color color, v2i center, float radius)
{

	int x_min = (int)(center.x-radius);
	if(x_min < 0)
		x_min = 0;
	int x_max = (int)(center.x+radius);
	if(x_max > vm_width)
		x_max = vm_width;
	int y_min = (int)(center.y-radius);
	if(y_min < 0)
		y_min = 0;
	int y_max = (int)(center.y+radius);
	if(y_max > vm_height)
		y_max = vm_height;
	//todo bounding box
	for (int x = x_min; x < x_max; ++x)
	for (int y = y_min; y < y_max; ++y)	
	{
		float dist_from_center = v2i_dist((v2i){x,y}, center);
		if(dist_from_center <= radius)
		{
			mem.frame_buffer.pixels[y*vm_width+x] = color;
		}
	}
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
	fill_circle(yellow, (v2i){64,64}, 32.3f);
}




splash_screen()
{
	//todo brushfire logo
	//fill_circle(red,(v2i){4,4},4.9f);
	//fill_circle(red,(v2i){12,4},4.9f);

	//fade up from black, show brushfire logo, fade to black, transition to title screen

}

title_screen()
{

	//draw game logo, press start text fading in and out, sword in stone in background in forest, animated glint on logo text, stretch goal fade to cut scene
	//draw_text("press start",)

}

file_select()
{

}

field()
{
	printf("%d\n",g->max_health);
	v3 forward;
	v3 right;
	v3 hand;
	v3 sword_tip;
	static v3 player_forward = v3_forward;
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
			//forward = (v3){0,0,1};
			//right = (v3){1,0,0};

			//transform move_vector to be camera relative
			v3 forward2 = v3_scale(forward,move_vector.z);
			v3 right2 = v3_scale(right, move_vector.x);
			move_vector = v3_add(forward2,right2);

			//for now
			if(v3_mag(move_vector) > 0) player_forward = move_vector;

			//todo actually base delta time on clock
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
							} break;
							case Fairy:
							{
								g->cur_health = g->max_health;
							} break;
							case HeartContainer:
							{
								g->max_health += 4;
								if(g->max_health > 80)
									g->max_health = 80;
							} break;
							case Heart:
							{
								g->cur_health += 4;
								if(g->cur_health > g->max_health)
									g->cur_health = g->max_health;
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
		//environment
		{
			sky();
			mountains();
			sun();
		}

		for (int i = 0; i < g->entity_count; ++i)
		{
			if(!g->entities[i].skip_draw)
			{
				render_rect(g->entities[i].color, g->entities[i].transform);
			}
		}

		render_rect(green, g->camera);
		render_rect(blue, t_from_v_and_s(forward,.2f));
		render_rect(red, t_from_v_and_s(right,.2f));

		if(g->player_animation_state == Slashing)
		{
			render_rect(0x555555, t_from_v_and_s(hand,.3f));
			render_rect(0x555555, t_from_v_and_s(sword_tip,.3f));
		}

		//HUD
		if(false)
		{
			//todo get heart sprites to replace these squares with
			if(true)
			//health bar
			{
				int full_hearts = g->cur_health / 4; 
				int total_hearts = g->max_health / 4;
				int partial =  g->cur_health % 4;

				int heart_width = 16;
				int x_offset = 6;
				int y_offset = 6;
				int x_padding = 2;
				int y_padding = 2;
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
				int bar_width = 100;
				int bar_height = 10;
				int border_size = 2;
				int x = 8;
				int y = 48;
				fill_rect(black, (Rect){x-border_size, y-border_size, bar_width+(2*border_size), bar_height+(2*border_size)});
				fill_rect(red, (Rect){x, y, bar_width, bar_height});
				fill_rect(green, (Rect){x, y, (int)((g->cur_magic/(float)g->max_magic)*bar_width), bar_height});
			}

			//wallet
			{
				//todo grab fontset file from platfighter project for text drawing
			}

			//command buttons
			{
				fill_circle(blue, (v2i){vm_width/2+32,48}, 32);

				//todo render
				{
					if(g->current_command != NULL)
					printf(g->current_command->text);
				}
			}

			//item buttons
			{
				fill_circle(yellow,(v2i){vm_width-(32+32+32+32),32}, 16);
				fill_circle(yellow,(v2i){vm_width-(32+32+32), 64}, 16);
				fill_circle(yellow,(v2i){vm_width-(32+32),32}, 16);
			}
		}

		//triangle r&d
		if(false)
		{
			Triangle tri = (Triangle)
			{
				{100,100,0},
				{200,100,0},
				{100,200,0},
			};

			Sleep(100);

			v3 edge1 = v3_sub(tri.b,tri.a);
			v3 edge2 = v3_sub(tri.c,tri.a);

			printf("1:{%f,%f,%f}\n", edge1.x, edge1.y, edge1.z);
			printf("2:{%f,%f,%f}\n", edge2.x, edge2.y, edge2.z);		
			fill_rect(red, (Rect){tri.a.x,tri.a.y,5,5});
			fill_rect(red, (Rect){tri.b.x,tri.b.y,5,5});
			fill_rect(red, (Rect){tri.c.x,tri.c.y,5,5});

			v3 bary = {.5f,.25f,.25f};
			v3 d = v3_add(v3_add(v3_scale(tri.a,bary.x),v3_scale(tri.b,bary.y)), v3_scale(tri.c,bary.z));
			printf("d: {%f,%f}", d.x,d.y);
			fill_rect(red, (Rect){(int)d.x,(int)d.y,5,5});
		}

		draw_terrain();
	}
}

dungeon()
{

}

paused()
{
	fill_rect(red,(Rect){10,10,vm_width-20,vm_height-20});
	if(ButtonDown(START) && !button_down(g->previous_padstate,START)){
		printf("foo!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		g->current_gamestate = g->previous_gamestate;
		g->previous_gamestate = 0;
	}

}

void (*scenes[scene_count])(void) = 
{
	[SplashScreen] = &splash_screen,
	[TitleScreen] = &title_screen,
	[FileSelect] = &file_select,
	[Dungeon] = &dungeon,
	[Field] = &field,
	[Paused] = &paused,
};

void _tick()
{
	Sleep(10);
	//print_gamepad(0);
	(scenes[g->current_gamestate])();
	g->previous_padstate = mem.game_pads[0];
}

int vertex_count = 0;
v3 vertices[1000];
int index_count;
int indices[1000];
void generate_terrain(int subdivs_x, int subdivs_z)
{
	int verts_wide = subdivs_x+1;
	int verts_deep = subdivs_z+1;
	vertex_count = (verts_wide)*(verts_deep);
	index_count = 6*subdivs_x*subdivs_z;
	printf("vertex count: %d", vertex_count);

	for (int z = 0; z < verts_deep; ++z)
	for (int x = 0; x < verts_wide; ++x)
	{
		vertices[z*verts_wide+x] = (v3){x,0,z};
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

	printf("vertices: %d\n", vertex_count);
	for (int i = 0; i < vertex_count; ++i)
	{
		printf("%d: {%f,%f,%f}\n", i, vertices[i].x,vertices[i].y,vertices[i].z);
	}
	printf("indices: %d\n", index_count);
	for (int i = 0; i < index_count; ++i)
	{
		printf("\t%d: %d\n", i, indices[i]);
	}
}

void draw_terrain()
{
	static float foo=0;
	// for (int i = 0; i < index_count; i++)
	// {
	// 	v3 vertex = vertices[indices[i]];
	// 	fill_rect(white, (Rect){vertex.x*10,vertex.z*10,5,5});
	// }
	static int offset=0;
	v3 a = vertices[indices[offset+0]];
	v3 b = vertices[indices[offset+1]];
	v3 c = vertices[indices[offset+2]];
	fill_rect(red, (Rect){a.x*10,a.z*10,5,5});
	fill_rect(green, (Rect){b.x*10,b.z*10,5,5});
	fill_rect(blue, (Rect){c.x*10,c.z*10,5,5});

	foo+=g->delta_time;
	if(foo>.5f)
	{
		foo = 0;
		offset+=3;
	}
}

//notes
/*
things to implement:
push blocks, torches,
doors, locked doors, shop, signs, octorocks, shield, jumping over gaps, climbing up ledges
you will need triangle rendering, meshes, particles, skeletal animation, 
7/3/2023
After work today, I need to implement a) basic triangle rasterization b) generalize drawing to have a "global texture"
so that you can switch from drawing to the screen to drawing into sprites, texture maps, etc.

Once you have basic triangle rasteriztion in place, you need to add a basic 3d transform pipeline.

Then do terrain (including collision). Then do animation (you don't necessarily need a complete animation system, possibly just some basic lerps and easing functions),

Then start working on making prettier assets
*/
