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
#include <sys/stat.h>

#define BUFFER_SIZE 30

typedef struct{
    char *name;
    unsigned int type;
} File;

void listFiles(File *files, int number);
void getCurrentWorkingDirectory();
void refreshListFile(File **files, int *number, int *capacity);

DIR *enterFile(char basePath[], char followingPath[], char *dirPath){
    if (dirPath == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    strcpy(dirPath, basePath);
    strcat(dirPath, followingPath);

    DIR *dir = opendir(dirPath);

    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du dossier");
        exit(1);
    }

    return dir;
}

int main() {

    // TITLE

    system("clear");
    printf("Annaora file manager!\n");

    // ------------------------------------------------------------------

    // dirent.h

    char basePath[100];
    strcpy(basePath, getenv("HOME"));
    char followingPath[] = "";
    char *dirPath = malloc(strlen(basePath) + strlen(followingPath) + 1);

    DIR *dir = enterFile(basePath, followingPath, dirPath);

    if (chdir(dirPath) == -1) {
        perror("chdir");
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

        struct stat info;

        if (lstat(entry->d_name, &info) == -1) {
            perror("stat");
            continue;
        }

        files[number].type = info.st_mode;

        if (S_ISDIR(files[number].type)) {
            printf("%25s - FOLD\n", files[number].name);
        }
        else if (S_ISREG(files[number].type)) {
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
    bool isFolder = S_ISDIR(files[0].type);

    isFolder ? printf("\n%03d %23s FOLD\n", currentSelect, files[currentSelect].name) : printf("\n%03d %30s FILE\n", currentSelect, files[currentSelect].name);

    while(isRunning) {
        char key = getchar();
        usleep(1000000/60);

        if(key == '\033') {
            char c2 = getchar();
            char c3 = getchar();

            if(c2 == '[') {
                if(c3 == 'A')
                    currentSelect == 0 ? currentSelect = number - 1 : currentSelect--;
                if(c3 == 'B')
                    currentSelect == number - 1 ? currentSelect = 0 : currentSelect++;
                
                isFolder = S_ISDIR(files[currentSelect].type);

                if(isFolder){
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number);
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
                } else {
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number);
                    printf("\n\r%03d %23s FILE\n", currentSelect, files[currentSelect].name);
                }
            }
        }
        if(key == '\n' && S_ISDIR(files[currentSelect].type)){
            printf("\rEntering folder %s\n", files[currentSelect].name);

            if (chdir(files[currentSelect].name) == -1) {
                perror("chdir");
            } 
            else{
                closedir(dir);
                dir = opendir(".");
            
                if (dir == NULL) {
                    perror("opendir");
                    exit(1);
                }
            
                refreshListFile(&files, &number, &capacity);
                currentSelect = 0;
                system("clear");
                printf("Annaora file manager!\n");
                listFiles(files, number);
                isFolder = S_ISDIR(files[currentSelect].type);
            
                if(isFolder) {
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
                } 
                else {
                    printf("\n\r%03d %30s FILE\n", currentSelect, files[currentSelect].name);
                }
            }
        }
    }

    // -----------------------------------------------------------------

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    free(dirPath);
    for(int i = 0; i < number; i++){free(files[i].name);}
    free(files);
    closedir(dir);
    return 0;
}

void listFiles(File *files, int number){
    for (int i = 0; i < number; i++) {

        if (S_ISDIR(files[i].type)) {
            printf("%25s - FOLD\n", files[i].name);
        } 
        else if (S_ISREG(files[i].type)) {
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

void refreshListFile(File **files, int *number, int *capacity) {
    DIR *dir = opendir(".");

    if (dir == NULL) {
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;

    for (int i = 0; i < *number; i++) {
        free((*files)[i].name);
    }

    *number = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (*number >= *capacity) {
            *capacity *= 2;
            File *temp = realloc(*files, *capacity * sizeof(**files));

            if (temp == NULL) {
                perror("realloc");
                closedir(dir);
                exit(1);
            }
            *files = temp;
        }

        struct stat info;

        if (lstat(entry->d_name, &info) == -1) {
            continue;
        }

        (*files)[*number].name = malloc(strlen(entry->d_name) + 1);

        if ((*files)[*number].name == NULL) {
            perror("malloc");
            closedir(dir);
            exit(1);
        }

        strcpy((*files)[*number].name, entry->d_name);
        (*files)[*number].type = info.st_mode;
        (*number)++;
    }
    closedir(dir);
}