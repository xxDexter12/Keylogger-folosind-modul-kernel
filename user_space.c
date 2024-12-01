#include <stdio.h>
#include <stdlib.h>
#include<fcntl.h>
#include <string.h>
#include <unistd.h>

#define CIPBOARD_FILE "/proc/keyboard_clipboard"
#define DATA_SIZE 1024

void send_to_kernel(const char *data)
{
    int fd=open(CIPBOARD_FILE,O_WRONLY);
    if(fd<0)
    {
        perror("Error when trying to open the /proc/keyboard_clipboard from user_space \n");
        return ;
    }
    write(fd,data,strlen(data));
    close(fd);
}

int main()
{
    char *clipboard_data=malloc(sizeof(char)*DATA_SIZE);

    while(1)
    {
        FILE *file=popen("xclip -o -selection clipboard 2>/dev/null", "r");
        if(!file)
        {
            perror("Failed to read clipboard\n");
            sleep(1);
            continue;
        }

        if(fgets(clipboard_data,DATA_SIZE,file)!=NULL)
        {
            send_to_kernel(clipboard_data);
        }
        pclose(file);
        sleep(1);
    }
    return 0;
}