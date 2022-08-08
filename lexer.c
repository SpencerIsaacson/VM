#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_file_size(FILE* fp)
{
	fseek(fp, 0, SEEK_END);
	int file_size = ftell(fp);
	rewind(fp);
	return file_size;
}

typedef struct
{
	int length;
	char* data;
} string;

typedef struct
{
	int count;
	string strs[100];
} StrArray;

#define megabyte 1024*1024
int scratch_index = 0;
char scratch[8 * megabyte];

StrArray lex(char* source, int length)
{
	int i = 0;
	int token_count = 0;
	static char* tokens[5000];

	while (i < length)
	{
		tokens[token_count] = (char*)&scratch[scratch_index];

		while (!isspace(source[i]) && source[i] != 0)
			scratch[scratch_index++] = source[i++];

		scratch[scratch_index++] = 0; //null terminate string
		token_count++;

		while (isspace(source[i]) && source[i] != 0)
			i++;
	}

	StrArray token_array;
	token_array.count = token_count;
	for (int i = 0; i < token_count; ++i)
	{
		token_array.strs[i] = (string){ .length = strlen(tokens[i]), .data = tokens[i] };
	}

	return token_array;
}


char* push_chars(char* text, char delimiter)
{
	char* result = (char*)&scratch[scratch_index];
	for (int i = 0; i < strlen(text) && text[i] != delimiter; ++i)
	{
		scratch[scratch_index++] = text[i];
	}

	scratch[scratch_index++] = 0;
	return result;
}

StrArray split(char* source, int length)
{
	int i = 0;
	StrArray lines;
	while (i < length)
	{
		char* next_line = push_chars(source, '\n');
		printf("%s\n", next_line);
		lines.strs[i] = (string){ .data = next_line, .length = strlen(next_line) };
		lines.count++;
		while (i++ != '\n' && i < strlen(next_line));
	}

	return lines;

}

int main(int argc, char** argv)
{
	FILE* fp = fopen("test.lang_low", "rb");
	int file_size = get_file_size(fp);
	char* source = (char*)malloc(file_size);
	fread(source, file_size, 1, fp);
	fclose(fp);

	source[file_size] = 0;
	//StrArray lines = split(source, file_size);
	StrArray tokens = lex(source, file_size);

	fp = fopen("lexed", "wb");
	for (int i = 0; i < tokens.count; ++i)
	{
		char text[100];
		sprintf(text, "%s\n", tokens.strs[i].data);
		fwrite(text, strlen(text), 1, fp);
	}


	//for (int i = 0; i < lines.count; ++i)
	//{
		//printf("%.*s", lines.strs[i].length, lines.strs[i].data);
	//}
}