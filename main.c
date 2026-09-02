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

typedef struct{
    char *name;
    unsigned char type;
} File;

void listFiles(File *files, int number);
void getCurrentWorkingDirectory();

int main() {

    // TITLE

    system("clear");
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
            printf("%25s - FOLD\n", files[number].name);
        } else if (files[number].type == 8){
            printf("%25s - FILE\n", files[number].name);
        }

        number++;
    }

    // ------------------------------------------------------------------

    // BUFFER & getcwd

    //getCurrentWorkingDirectory(); -- Don't need it rn

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
    bool isFolder = files[0].type == 4;

    isFolder ? printf("\n%03d %30s FOLD\n", currentSelect, files[currentSelect].name) : printf("\n%03d %30s FILE\n", currentSelect, files[currentSelect].name);

    while(isRunning) {
        char c = getchar();
        usleep(1000000/60);

        if (c == '\033') {
            char c2 = getchar();
            char c3 = getchar();

            if (c2 == '[') {
                if(c3 == 'A')
                    currentSelect == number - 1 ? currentSelect = 0 : currentSelect++;
                if(c3 == 'B')
                    currentSelect == 0 ? currentSelect = number - 1 : currentSelect--;
                
                isFolder = files[currentSelect].type == 4;

                if(isFolder){
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number);
                    printf("\n\r%03d %30s FOLD\n", currentSelect, files[currentSelect].name);
                } else {
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number);
                    printf("\n\r%03d %30s FILE\n", currentSelect, files[currentSelect].name);
                }
            }
        }
    }

    // -----------------------------------------------------------------

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    free(files);
    closedir(dir);
    return 0;
}

void listFiles(File *files, int number){
    for (int i = 0; i < number; i++) {

        if (files[i].type == 4) {
            printf("%25s - FOLD\n", files[i].name);
        } 
        else if (files[i].type == 8) {
            printf("%25s - FILE\n", files[i].name);
        }
    }
}

void getCurrentWorkingDirectory(){
    char buffer[BUFFER_SIZE];
    if(getcwd(buffer, BUFFER_SIZE) == NULL){
        printf("Cannot get current working directory path!\n");
        exit(1);
    }

    printf("Current working directory: %s\n", buffer);
}