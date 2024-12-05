#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct
{
	int length;
	char *data;
} string;

string read_file(char *path)
{
	FILE *file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	int source_size = ftell(file);
	rewind(file);
	char *source = malloc(source_size+1);
	source[source_size] = 0;
	fread(source, 1, source_size, file);
	fclose(file);
	return (string){ .length = source_size, .data = source };
}

main()
{
	string text = read_file("compiler.c");

	FILE* file = fopen("compiler_processed.c", "wb+");

	char *processed_text = malloc(text.length + 1024);

	char* foo = "#define c2s(n) ((string){.data = n, .length = strlen(n)})\r\n";
	fwrite(foo, 1, strlen(foo), file);

	int o = 0;
	bool toggle = false;
	for (int i = 0; i < text.length; ++i, ++o)
	{
		if(text.data[i] == '"' && text.data[i-1] != '\'')
		{
			if(!toggle)
				o += sprintf(&processed_text[o], "c2s(");
			else{
				o += sprintf(&processed_text[o], "\")");
				i++;
			}
			toggle = !toggle;
		}
		processed_text[o] = text.data[i];
	}

	printf("%d\n", o);
	processed_text[o] = 0;
	fwrite(processed_text, 1, o, file);
	fclose(file);
}