#include <stdio.h>
#include <unistd.h>

#define BUFFER_SIZE 30

int main(int argc, char *argv[]) {

    printf("Annaora file manager!\n");
    printf("%s\n%s\n", argv[0], argv[1]);

    char buffer[BUFFER_SIZE];
    if(getcwd(buffer, BUFFER_SIZE) == NULL){
        printf("Cannot get current working directory path!\n");
        return 1;
    }

    printf("Current working directory: %s\n", buffer);
    
    return 0;
}