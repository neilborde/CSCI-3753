#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdbool.h>

#define DEVICENAME "/dev/simple_character_device"
#define Buffer_size 1024

int main(){
	char command;
	int length, whence, new_offset;
	char buffer[BUFF_SIZE];
    
	int file = open(DEVICENAME, O_RDWR);
	bool running = true;
	while(running){
		printf("COMMANDS:\n");
		printf("	'r' to read from device\n");
		printf("	'w' to write to device\n");
		printf("	's' to seek from device\n");
		printf("	'e' to exit from device\n");
		printf("	anything else brings up main menu\n");
		printf("/SCD/command$> ");
		scanf("%c", &command);

		switch(command){
			case 'r':
				printf("/SCD/read$> How many bytes to read?: ");
				scanf("%d", &length);
				read(file, buffer, length);
				printf("/SCD/read$> %s\n", buffer); // print that buffer
				while(getchar() != '\n'); // check for end line
				break;
			case 'w':
				printf("/SCD/write$> ");
				scanf("%s", buffer);
				write(file, buffer, BUFF_SIZE); // write the buffer to file
				while (getchar() != '\n'); // check for end line
				break;
			case 's':
				printf("SEEK COMMANDS:\n");
				printf("	'0' seek set\n");
				printf("	'1' seek cur\n");
				printf("	'2' seek end\n");
				printf("	anything else brings up main menu\n");
				printf("/SCD/seek$> Enter whence: ");
				scanf("%d", &whence);
				printf("\n/SCD/write$> Enter an offset value: ");
				scanf("%d", &new_offset);
				llseek(file, new_offset, whence);
				break;
			case 'e':
				printf("/SCD/exit$> Exiting\n");
				running = false;
				break;
			default:
				printf("\n/SCD/error$> error: not a valid command\n");
				break;
		}
	}
	close(file);
	return 0;
}

