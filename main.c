#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <sys/select.h>
#include <termios.h>

#define BUFFER_SIZE 30

int getMin(int a, int b);

typedef struct{
    char *name;
    unsigned char type;
} File;

int main() {

    // TITLE

    printf("Annaora file manager!\n");

    // ------------------------------------------------------------------

    // dirent.h

    DIR *dir = opendir(".");
    if(dir == NULL){
        perror("Erreur lors de l'ouverture du dossier!\n");
        exit(1);
    }

    int capacity = 10;
    int number = 0;
    File *files = malloc(capacity * sizeof(*files));
    if (files == NULL) {
        perror("malloc");
        return 1;
    }

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){

        if(number >= capacity){
            capacity *= 2;
            files = realloc(files, capacity * sizeof(*files));
        }

        files[number].name = malloc(strlen(entry->d_name) + 1);
        strcpy(files[number].name, entry->d_name);
        files[number].type = entry->d_type;

        if(files[number].type == 4){
            printf("FOLDER - %s\n", files[number].name);
        } else if (files[number].type == 8){
            printf("FILE   - %s\n", files[number].name);
        }

        number++;
    }

    // ------------------------------------------------------------------


    // BUFFER & getcwd

    char buffer[BUFFER_SIZE];
    if(getcwd(buffer, BUFFER_SIZE) == NULL){
        printf("Cannot get current working directory path!\n");
        return 1;
    }

    printf("Current working directory: %s\n", buffer);

    // ------------------------------------------------------------------

    // while loop

    struct termios oldt;
    struct termios newt;

    tcgetattr(STDIN_FILENO, &oldt);

    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    bool isRunning = true;
    int currentSelect = 0;
    int selected = getMin(0, number);

    while(isRunning) {
        char c = getchar();
        usleep(1000000/60);
        
        if (c == '\033') {
            char c2 = getchar();
            char c3 = getchar();

            if (c2 == '[') {
                if (c3 == 'A')
                    currentSelect == number - 1 ? currentSelect = 0 : currentSelect++;
                if (c3 == 'B')
                    currentSelect == 0 ? currentSelect = number - 1 : currentSelect--;

                printf("%d, %s\n", currentSelect, files[currentSelect].name);
            }
        }
    }

    
    // -----------------------------------------------------------------
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    free(files);
    closedir(dir);
    return 0;
}

int getMin(int a, int b){
    if(a>b){
        return b;
    }
    return a;
}