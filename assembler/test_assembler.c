#include <string.h>

void test_func()
{
	char *instruction = "set $15 $5";
}

char look_up[]  = {'0', '1', '2', '3', '4', '5', '6', '7','8', '9', 'A', 'B', 'C','D','E','F'};
int parse_hex(char *string)
{
	int res = 0;

	for (int o = 0; o < strlen(string); ++o)
	{
		if(o > 0)
			res*=0x10;
		char b = string[o];
		for (int i = 0; i < 0xF; ++i)
		{
			if(b == look_up[i])
			{
				res += i;
				break;
			}
		}
	}

	return res;
}

int parse_int(char *s)
{
	int ret = 0;
	while(*s != 0)
	{
		ret += (*(s++)-'0');
		if(*s != 0)
			ret *= 10;
	}

	return ret;
}

char is_digit(char c)
{   return c >= '0' && c <= '9';   }

char is_letter(char c)
{
	c &= ~0x20;
	return c >= 65 && c <= 90;
}

unsigned int parse_hex2(char *s)
{
	int ret = 0;
	while(*s != 0)
	{
		if(is_digit(*s))
			ret += (*(s++)-'0');
		else if(is_letter(*s)){
			*s &= ~0x20;
			ret += (*(s++)-('A'-0xA));		
		}
		if(*s != 0)
			ret *= 0x10;
	}

	return ret;
}