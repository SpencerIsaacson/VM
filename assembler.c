#include <stdio.h>

int main(int argc, char** argv)
{
	if(argc == 2)
		printf("%s\n", argv[1]);

	FILE* file = fopen(argv[1],"rb");
	if(file != NULL)
	{
		fseek(file,0,SEEK_END);
		int bytes = ftell(file);
		fseek(file,0,0);

		char text[bytes+1];
		text[bytes] = 0;
		fread(text, bytes,1,file);
		fclose(file);

		int i = 0;
		while(i < bytes)
		{
			char mneumonic[50];
			int o = 0;
			while(text[o] != ' ')
				mneumonic[o] = text[o++];


			printf("%s\n",mneumonic);
			return;	
			printf("%c",text[i++]);
			sleep(100);
		}
	}

}