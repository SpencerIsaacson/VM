typedef struct 
{
	float x, y;
} v2;

typedef struct MouseState
{
	v2 position;
	v2 delta;
	bool leftbutton_down;
	bool leftbutton_was_down;
} MouseState;

typedef struct ShortStick //16 bit (short) representation of analog stick state
{
	signed char X;
	signed char Y;
} ShortStick;

typedef enum
{
	UP =     1 << 0,
	DOWN =   1 << 1,
	LEFT =   1 << 2,
	RIGHT =  1 << 3,
	A =      1 << 4,
	B =      1 << 5,
	X =      1 << 6,
	Y =      1 << 7,
	SELECT = 1 << 8,
	START =  1 << 9,
	L1 =     1 << 10,
	R1 =     1 << 11,
	L2 =     1 << 12,
	R2 =     1 << 13,
	L3 =     1 << 14,
	R3 =     1 << 15,

} ButtonName;

typedef unsigned short Buttons;

typedef struct Triggers //sizeof short
{
	unsigned char l_trigger;
	unsigned char r_trigger;
} Triggers;

typedef struct Sticks
{
	ShortStick left_stick;
	ShortStick right_stick;
} Sticks;

typedef struct GamePad //total size of 8 bytes (64 bits)
{
	Buttons buttons;
	Triggers triggers;
	/*boundary between first 32-bit word and second word ----------*/
	Sticks sticks;
} GamePad;

MouseState mousestate;

enum Keys
{
	Keys_Backspace = 8,
	Keys_Tab = 9,
	Keys_Enter = 13,
	Keys_Escape = 27,
	Keys_Space = 32,
	Keys_Left = 37, Keys_Up, Keys_Right, Keys_Down,
	Keys_Delete = 46,
	Keys_A = 65, Keys_B, Keys_C, Keys_D, Keys_E, Keys_F, Keys_G, Keys_H, Keys_I, Keys_J, Keys_K, Keys_L, Keys_M, Keys_N, Keys_O, Keys_P, Keys_Q, Keys_R, Keys_S, Keys_T, Keys_U, Keys_V, Keys_W, Keys_X, Keys_Y, Keys_Z,
	Keys_Add = 107,
	Keys_Subtract = 109,
	Keys_NumPad0 = 96, Keys_NumPad1, Keys_NumPad2, Keys_NumPad3, Keys_NumPad4, Keys_NumPad5, Keys_NumPad6, Keys_NumPad7, Keys_NumPad8, Keys_NumPad9,
	Keys_F4 = 115,
	Keys_LCtrl = 162,
};

bool keys_down[256];
bool keys_stale[256]; //whether a key has been pressed for more than one consecutive frame
bool keyboard_state[256];


bool KeyDownFresh(enum Keys key)
{	return keys_down[key] && !keys_stale[key];	}

bool KeyDown(enum Keys key)
{	return keys_down[key];	}


// void PollKeyboard()
// {
// 	for (int i = 0; i < 256; i++)
// 		keys_stale[i] = keys_down[i];

// 	GetKeyboardState((PBYTE)keyboard_state);

// 	for (int i = 0; i < 256; i++)
// 		keys_down[i] = keyboard_state[i] & 128;

// 	for (int i = 0; i < 256; i++)
// 	{
// 		if (!keys_down[i])
// 			keys_stale[i] = false;
// 	}
// }