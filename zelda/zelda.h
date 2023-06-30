//The Legend of Zelda: Fair Use Demonstration
#include <math.h>

bool ButtonDown(int pad, ButtonName button_name)
{
	return ( (mem.game_pads[pad].buttons & button_name) != 0);
}

print_gamepad(int pad)
{
	printf(
		"pad %d:\n{\n UP: %d\nDOWN: %d\nLEFT: %d\nRIGHT: %d\nA: %d\nB: %d\nX: %d\nY: %d\nSELECT: %d\nSTART: %d\nL1: %d\nR1: %d\nL2: %d\nR2: %d\nL3: %d\nR3:%d\nleft_stick:{ x: %d, y: %d }\nright_stick:{ x: %d, y: %d }}\n\n",
		pad,
		ButtonDown(pad, UP),
		ButtonDown(pad, DOWN),
		ButtonDown(pad, LEFT),
		ButtonDown(pad, RIGHT),
		ButtonDown(pad, A),
		ButtonDown(pad, B),
		ButtonDown(pad, X),
		ButtonDown(pad, Y),
		ButtonDown(pad, SELECT),
		ButtonDown(pad, START),
		ButtonDown(pad, L1),
		ButtonDown(pad, R1),
		ButtonDown(pad, L2),
		ButtonDown(pad, R2),
		ButtonDown(pad, L3),
		ButtonDown(pad, R3),
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
	scene_count,
} Scene;

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
} Entity_Type;
typedef struct Entity
{
	Entity_Type entity_type;
	struct Entity* parent;
	Transform transform;
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
	int cur_health;
	int max_health;
	int cur_magic;
	int max_magic;
	int cur_rupees;
	int max_rupees;
	#define max_entitites 400
	int entity_count;
	Entity entities[max_entitites];
	Scene current_scene;
	Transform camera;
	//assets
	Sprite heart;
	Sprite hearts[4];
} GameState;


GameState *g = (GameState *)&mem.RAM[start_address];

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

#define player (g->entities[0])
#define default_transform {0,0,0,0,0,0,1,1,1}
init()
{
	*g = (GameState)
	{
		.cur_health = 25,
		.max_health = 40,
		.cur_magic = 80,
		.max_magic = 100,
		.current_scene = Field,
		.entity_count = 1,
	};

	g->entities[0] = (Entity)
	{
		.entity_type = Player,
		.parent = 0,
		.transform = default_transform,
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
	for (int x = 0; x < vm_width; ++x)
	for (int y = 0; y < vm_height; ++y)	
	{
		float dist_from_center = v2i_dist((v2i){x,y}, center);
		if(dist_from_center <= radius)
		{
			mem.frame_buffer.pixels[y*vm_width+x] = color;
		}
	}
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
	v3 forward;
	v3 right;
	v3 hand;
	v3 sword_tip;
	static v3 player_forward;
	//update
	{
		//player motion
		{
			//note:for now assume player is entity 0, this may change
			float speed = 7;
			v3 move_vector = v3_zero;

			if(ButtonDown(0, UP))
				move_vector.z = 1;
			else if (ButtonDown(0,DOWN))
				move_vector.z = -1;
			if (ButtonDown(0,LEFT))
				move_vector.x = -1;
			else if(ButtonDown(0,RIGHT))
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

			//transform move_vector to be camera relative
			v3 forward2 = v3_scale(forward,move_vector.z);
			v3 right2 = v3_scale(right, move_vector.x);
			move_vector = v3_add(forward2,right2);

			//for now
			if(v3_mag(move_vector) > 0) player_forward = move_vector;

			//todo actually base delta time on clock
			float delta_time = 0.01f;
			move_vector = v3_scale(move_vector, speed);
			move_vector = v3_scale(move_vector, delta_time);
			player.transform.position =  v3_add(player.transform.position, move_vector);

			//lerp camera to target position
			v3 target = v3_sub(player.transform.position, v3_scale(forward,follow_distance));
			g->camera.position = v3_lerp(g->camera.position, target,.05f);

			right = v3_add(right, g->camera.position);
			forward = v3_add(forward, g->camera.position);

			player.transform.rotation.y = atan2(player_forward.z, player_forward.x)-M_PI/2;
			hand = (v3){-.5,0,1};
			hand = v3_rotate_xz_plane(hand, player.transform.rotation.y);
			hand = v3_add(player.transform.position,hand);
			sword_tip = (v3){0,0,1};
			sword_tip = v3_rotate_xz_plane(sword_tip, player.transform.rotation.y);
			sword_tip = v3_add(sword_tip,hand);

			float theta = player.transform.rotation.y;
			printf("theta:%f\n", theta);
			//transform hand_direction;
			//sword_tip = v3_add(player.transform.position, sword_tip);
		}
	}

	//render
	{
		//environment
		{
			sky();
			mountains();
			sun();
		}

		render_rect(green, player.transform);

		render_rect(red, g->camera);
		Transform temp_t = (Transform){forward.x,forward.y,forward.z, 0,0,0,.2,.2,.2};
		render_rect(blue, temp_t);
		temp_t = (Transform){right.x,right.y,right.z, 0,0,0,.2,.2,.2};
		render_rect(red, temp_t);

		temp_t = (Transform){ hand.x, hand.y, hand.z, 0, 0, 0, .3f, .3f, .3f};
		render_rect(0x555555, temp_t);
		temp_t = (Transform){ sword_tip.x, sword_tip.y, sword_tip.z, 0, 0, 0, .3f, .3f, .3f};
		render_rect(0x555555, temp_t);

		//HUD
		{
			//todo get heart sprites to replace these squares with
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
				int hearts_per_row = 5;

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

			//items
			{

			}
		}
	}
}

dungeon()
{

}

void (*scenes[scene_count])(void) = 
{
	[SplashScreen] = &splash_screen,
	[TitleScreen] = &title_screen,
	[FileSelect] = &file_select,
	[Dungeon] = &dungeon,
	[Field] = &field,
};

void _tick()
{
	(scenes[g->current_scene])();
}


generate_terrain()
{
	//todo (first get basic triangle rendering up and running)

}