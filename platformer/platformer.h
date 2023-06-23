#include <math.h>
typedef struct
{
	u32 width, height;
	Color pixels[0];
} Texture;

typedef struct
{
	int x, y;
} v2i;

typedef struct
{
	int x, y, width, height;
} Rect;

typedef enum
{
	DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
} Direction;

int clamp_int(int val, int min, int max)
{
	return (val < min) ? min : ((val > max) ? max : val);
}

float v2_mag(v2i v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y);
}

v2i v2_sub(v2i a, v2i b)
{
	return (v2i){ a.x - b.x, a.y - b.y};
}

float v2_dist(v2i a, v2i b)
{
	return v2_mag(v2_sub(a,b));
}

typedef struct
{
	float x, y, z;
} v3;
float v3_mag(v3 v)
{
	return (float)sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

#define mag(v) _Generic(v,v2i:v2_mag(v),v3:v3_mag(v))

v3 v3_Divide(v3 v, float s)
{
	return (v3){ v.x / s, v.y / s, v.z / s };
}
v3 normalized(v3 v)
{
	return v3_Divide(v, mag(v));
}

normalize(v3 *v)
{
	*v = v3_Divide(*v, mag(*v));
}

float v2_dot(v2 a, v2 b) 
{   return a.x * b.x + a.y * b.y;   }
float v3_dot(v3 a, v3 b) 
{   return a.x * b.x + a.y * b.y + a.z * b.z;   }
#define dot(a,b) _Generic(a,v2i:v2_dot(a,b),v3:v3_dot(a,b))

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

clear()
{
	for (int i = 0; i < vm_width*vm_height; ++i)
	{
		mem.frame_buffer.pixels[i] = 0;
	}
}

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

gradient(Color a, Color b)
{
	Color col = a;

	for (int y = 0; y < vm_height; ++y)
	{
		float t = y/(float)vm_height;
		col = lerp_color(a, b, t);
		memset_u32_4wide(&mem.frame_buffer.pixels[y*vm_width], col, vm_width);
	}
}

typedef struct
{
	v2i start_position;
	int block_count;
	Rect blocks[10]; 
	Rect level_flag;
} Level;

bool ButtonDown(int pad, ButtonName button_name)
{
	return ( (mem.game_pads[pad].buttons & button_name) != 0);
}

bool rect_intersect(Rect a, Rect b)
{
	return a.x < (b.x+b.width) && (a.x+a.width) > b.x && a.y < (b.y+b.height) && (a.y+a.height) > b.y;
}

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

blit_background_offset(FrameBuffer *tex, int x_offset)
{
	for (int _x = 0; _x < vm_width; ++_x)
	for (int _y = 0; _y < vm_height; ++_y)
	{
		Color col = tex->pixels[_y*vm_width+((_x+x_offset)%vm_width)];
		if(col != white)
		mem.frame_buffer.pixels[_y*vm_width+_x] = col;
	}
}

typedef struct
{
	int health;
	int max_health;
	Rect rect;
} Enemy;

typedef struct
{
	v2i spawn;
	v2i vel;
} Move;


Move bullet_move_lut[4] = 
{
	[DIR_LEFT] = {-10,0, -10, 0},
	[DIR_RIGHT] = {16,0, 10, 0},
};

typedef struct
{
	Direction player_direction;
	v2i vel;
	bool bullet_out;
	Rect player_rect;
	Move bullet_move;
	Rect bullet;
	int enemy_count;
	Enemy enemies[10];
	Level levels[10];
	int current_level;
	int health;
	int max_health;
	v2i camera;
	//TEXTURE DATA
	// flag
	Texture flag_texture;
	Color flag_texture_pixels[16*16];
	//player
	Texture player_texture;
	Color player_texture_pixels[16*32];
	//bullet
	Texture bullet_texture;
	Color bullet_texture_pixels[8*8];
	//background
	Texture background_texture;
	Color background_texture_pixels[vm_width*vm_height];

} GameState; 


GameState *g = (GameState *)&mem.RAM[start_address];//todo auto calculate beginning of user memory in case it changes

void render_rect(Color col, Rect rect)
{
	rect.x -= g->camera.x;
	rect.y -= g->camera.y;
	fill_rect(col, rect);
}

void mountains(int r[vm_width])
{	
	Color rock = 0xFF888888;

	for (int x = 0; x < vm_width; ++x)
	for (int y = r[x]; y < vm_height; ++y)
	{
		mem.frame_buffer.pixels[y*vm_width+x] = rock;
	}
}

init()
{
	*g = (GameState)
	{
		.player_direction = DIR_RIGHT,
		.player_rect = { 64, vm_height-64, 16, 32 },
		.vel = { 0, 0 },
		.bullet = {0,0,10,10},
		.levels =
		{
			{	
				.block_count = 10,
				.blocks = 
				{
					{ 0, 0, 32, vm_height},
					{ 0, vm_height-32, 1000, 32 },
					{ 1032, vm_height-32, 650, 32 },
					{ 128, vm_height-64, 32, 32 },
					{ 32*20, vm_height-64, 32, 32 },
					{ 32*22, vm_height-32*3, 64, 32 },
					{ 32*25, vm_height-32*3, 32, 32*3 },
					{ 32*26, vm_height-32*2, 32, 32*2 },
					{ 1032+650, 3*vm_height/4+1, 128, vm_height/4 },
					{ 1032+650+128, 0, 256, vm_height },
				},
				.level_flag = { 1032+650+128-16, 3*vm_height/4+1-32, 16, 32 },
			},
			{
				.start_position = {128,128},		
				.block_count = 4,
				.blocks = 
				{
					{ 0, vm_height-64, vm_width, 64 },
					{ 0, 0, vm_width, 64 },
					{ 0, 64, 32, vm_height-128 },
					{ vm_width-32, 64, 32, vm_height-128 },
				},
				.level_flag = { 300, 0, 20, 20 },
			}			
		},
		.enemy_count = 1,
		.enemies =
		{
			{10, 10, 600,vm_height-64, 32, 32},
		},
		.health = 100,
		.max_health = 100,
#define w white
#define r red
#define b 0xFF996600
		.flag_texture = {16, 16},
		.flag_texture_pixels = 
		{
			#include "flag.sprite"	
		},
		.player_texture = {16,32},
		.player_texture_pixels =
		{
			#include "monkey.sprite"
		},
		.bullet_texture = {8,8},
		.bullet_texture_pixels =
		{
			w,w,w,w,w,w,w,w,
			w,0,0,0,0,0,w,w,
			w,0,0,0,0xFFFFFFFE,0xFFFFFFFE,0,w,
			w,0,0,0,0,0,0,0,
			w,0,0,0,0,0,0,0,
			w,0,0,0,0,0,0,w,
			w,0,0,0,0,0,w,w,
			w,w,w,w,w,w,w,w,									
		}
#undef w
#undef r
#undef b
	};

	g->current_level = 0;

	g->background_texture = (Texture){vm_width,vm_height};
	FILE* fp = fopen("background.sprite","rb");
	fread(g->background_texture_pixels,1,sizeof(FrameBuffer),fp);
	fclose(fp);

}


int abs(int v)
{
   int t = v >> 31;
   return (v^t) - t;
}



_tick()
{

#define l g->levels[g->current_level] 
	if(ButtonDown(0, B))
		init();

	//update
	{		
		//movement
		{
			bool grounded = false;
			g->vel.x = 0;
			int speed = 5;
			
			if(ButtonDown(0, RIGHT)){
				g->vel.x = speed;
				g->player_direction = DIR_RIGHT;
			}

			if(ButtonDown(0, LEFT)){
				g->vel.x = -speed;
				g->player_direction = DIR_LEFT;
			}

			int old_y = g->player_rect.y;
			g->player_rect.y += g->vel.y;

			for (int i = 0; i < l.block_count; ++i)
			{
				if(rect_intersect(g->player_rect, l.blocks[i]))
				{
					if(old_y <= l.blocks[i].y){
						g->player_rect.y = l.blocks[i].y-g->player_rect.height;
						grounded = true;
					}
					else if(old_y >= l.blocks[i].y+l.blocks[i].height){
						g->player_rect.y = l.blocks[i].y+l.blocks[i].height;
					}
					g->vel.y = 0;
				}
			}
			
			int old_x = g->player_rect.x;
			g->player_rect.x += g->vel.x;
			for (int i = 0; i < l.block_count; ++i)
			{

				if(rect_intersect(g->player_rect, l.blocks[i]))
				{
					if(old_x <= l.blocks[i].x){
						g->player_rect.x = l.blocks[i].x-g->player_rect.width;
					}
					else if(old_x >= (l.blocks[i].x+l.blocks[i].width)){
						g->player_rect.x = l.blocks[i].x+l.blocks[i].width;
					}
					g->vel.x = 0;
				}
			}

		
			if(grounded)
			{
				if(ButtonDown(0, A))
					g->vel.y = -10;
			}
			else
			{
				g->vel.y++;
				
				#define terminal_velocity 5
				
				if(g->vel.y > terminal_velocity)
					g->vel.y = terminal_velocity;
			}
		}

		static int start_shot;
		if(ButtonDown(0, X) && !g->bullet_out)
		{
			g->bullet_out = true;
			g->bullet = (Rect){ g->player_rect.x, g->player_rect.y, 10, 10 };
			g->bullet_move = bullet_move_lut[g->player_direction];
			g->bullet.x += g->bullet_move.spawn.x;
			g->bullet.y += g->bullet_move.spawn.y;
			start_shot = g->bullet.x;
		}
		else if(g->bullet_out)
		{
			g->bullet.x += g->bullet_move.vel.x;
			if(abs(g->bullet.x-start_shot)>150){
				g->bullet_out = false;
			}
		}

		for (int i = 0; i < g->enemy_count; ++i)
		{
			if(rect_intersect(g->bullet,g->enemies[i].rect))
			{
				g->enemies[i].health--;
				if(g->enemies[i].health == 0)
					g->enemy_count--;
			}
		}

		if(rect_intersect(g->player_rect,l.level_flag))
		{
			g->current_level++;
			v2i *p = &g->player_rect;
			*p = l.start_position;
		}

		g->camera.x = g->player_rect.x-(vm_width/2-g->player_rect.width/2);

		g->camera.x = clamp_int(g->camera.x,0, 4000);
		if(g->player_rect.y > vm_height)
			init();
	}

	if(false)
	//sphere
	{
		float sphere_size = 120.1f;
		v2i center = (v2i){ vm_width/2, vm_height/2 };
		static float t = 0;
		t+=.05f;
		byte ambient = 10;

		for (int x = 0; x < vm_width; ++x)
		for (int y = 0; y < vm_height; ++y)	
		{
			float dist_from_center = v2_dist((v2i){x,y}, center);
			if(dist_from_center < sphere_size)
			{
				v2i relative = {x-center.x,y-center.y};
  				v3 tangent_normal = (v3)
  				{
  					(relative.x / sphere_size)/2. + 0.5,
                  	(relative.y / sphere_size)/2. + 0.5,
                  	(float)cos(dist_from_center / sphere_size)
              	};


              	v3 light = (v3){cos(t),0,sin(t)};

              	normalize(&light);
              	normalize(&tangent_normal);
              	float lit = dot(light,tangent_normal);
              	if(lit < 0)
              		lit = 0;

          		lit = (float)pow(lit,2.2f);

              	lit *= 255;
              	float chance = lit - floor(lit);
              	byte lit_byte = 0;
              	if((rand()/(float)RAND_MAX) < chance)
              		lit_byte = (byte)ceil(lit);
              	else
              		lit_byte = (byte)floor(lit);
              	lit_byte = (lit_byte < ambient) ? ambient : lit_byte;
				mem.frame_buffer.pixels[y*vm_width+x] = (lit_byte << 16)|(lit_byte<<8)| lit_byte;
			}
		}
	}

	if(true)
	//render
	{
		blit_background_offset((FrameBuffer*)g->background_texture_pixels,g->camera.x/8);

		//draw blocks
		{
			for (int i = 0; i < l.block_count; ++i)
			{
				render_rect(brown,l.blocks[i]);
			}
		}

		//draw enemies
		{
			for (int i = 0; i < g->enemy_count; ++i)
			{
				render_rect(red, g->enemies[i].rect);

				render_rect(red, (Rect){ g->enemies[i].rect.x, g->enemies[i].rect.y+g->enemies[i].rect.height+3, 32, 6 });
				render_rect(green, (Rect){ g->enemies[i].rect.x, g->enemies[i].rect.y+g->enemies[i].rect.height+3, (int)(g->enemies[i].health/(float)g->enemies[i].max_health*32), 6 });
			}
		}

		render_rect(green, g->player_rect);
		draw_tex_t(&g->player_texture,g->player_rect.x-g->camera.x,g->player_rect.y-g->camera.y);
		if(g->bullet_out){
		//	render_rect(red, g->bullet);
			draw_tex_t(&g->bullet_texture,g->bullet.x-g->camera.x,g->bullet.y-g->camera.y);
		}

		render_rect(magenta, l.level_flag);

		//draw health bar
		{
			fill_rect(0, (Rect){8, 8, 204, 24});
			fill_rect(red, (Rect){10, 10, 200, 20});
			fill_rect(green, (Rect){10, 10, (int)((g->health/(float)g->max_health)*200), 20});
		}
	}
}