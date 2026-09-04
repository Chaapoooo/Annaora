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
#include <inttypes.h>
#include <pwd.h>

#define BUFFER_SIZE 30

typedef struct{
    char *name;
    unsigned int type;
    off_t size;
    time_t modified;
    uid_t owner;
    bool isSymlink;
} File;

void listFiles(File *files, int number, int currentSelect);
void getCurrentWorkingDirectory();
void refreshListFile(File **files, int *number, int *capacity);
void printPermissions(mode_t mode);
void printDate(time_t timestamp);
void printOwner(uid_t uid);

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

        if (stat(entry->d_name, &info) == -1) {
            continue;
        }

        struct stat linkInfo;

        if (lstat(entry->d_name, &linkInfo) == -1) {
            continue;
        }

        files[number].type = info.st_mode;
        files[number].size = info.st_size;
        files[number].modified = info.st_mtime;
        files[number].owner = info.st_uid;
        files[number].isSymlink = S_ISLNK(linkInfo.st_mode);

        if(number == 0){
            printf("\x1b[1;30;47m");
        }

        if (S_ISDIR(files[number].type)) {
            printf("%25s - FOLD\n", files[number].name);
        }
        else if (S_ISREG(files[number].type)) {
            printf("%25s - FILE\n", files[number].name);
        }

        if(number == 0){
            printf("\x1b[0m");
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

                if(S_ISDIR(files[currentSelect].type)){
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number, currentSelect);
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
                } else if(S_ISREG(files[currentSelect].type)){
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number, currentSelect);
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
                listFiles(files, number, currentSelect);
                isFolder = S_ISDIR(files[currentSelect].type);
            
                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
                } 
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %30s FILE\n", currentSelect, files[currentSelect].name);
                }
            }
        }

        if((key == 'O' || key == 'o') && S_ISREG(files[currentSelect].type)){
            system("clear");

            printf("Informations for FILE %s\n", files[currentSelect].name);
            printf("\n");

            printf("Size: %" PRIdMAX " bytes.\n",(intmax_t)files[currentSelect].size);

            printf("Permissions: ");
            printPermissions(files[currentSelect].type);
            printf("\n");

            printf("Modified: ");
            printDate(files[currentSelect].modified);
            printf("\n");

            printf("Owner: ");
            printOwner(files[currentSelect].owner);
            printf("\n");

            printf("Symlink: %s\n", files[currentSelect].isSymlink ? "Y" : "N");

            printf("\n");
            printf("Press 'Q' to exit !");

            while(1){
                key = getchar();
            
                if(key == 'Q' || key == 'q'){
                    break;
                }
            }
        
            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);
            if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
            } 
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %30s FILE\n", currentSelect, files[currentSelect].name);
            }
        }

        if((key == 'O' || key == 'o') && S_ISDIR(files[currentSelect].type)){
            system("clear");

            printf("Informations for FOLDER %s\n", files[currentSelect].name);
            printf("\n");

            printf("Size: %" PRIdMAX " bytes.\n",(intmax_t)files[currentSelect].size);

            printf("Permissions: ");
            printPermissions(files[currentSelect].type);
            printf("\n");

            printf("Modified: ");
            printDate(files[currentSelect].modified);
            printf("\n");

            printf("Owner: ");
            printOwner(files[currentSelect].owner);
            printf("\n");

            printf("Symlink: %s\n", files[currentSelect].isSymlink ? "Y" : "N");

            printf("\n");
            printf("Press 'Q' to exit !");

            while(1){
                key = getchar();
            
                if(key == 'Q' || key == 'q'){
                    break;
                }
            }
        
            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);
            if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %23s FOLD\n", currentSelect, files[currentSelect].name);
            } 
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %30s FILE\n", currentSelect, files[currentSelect].name);
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

void listFiles(File *files, int number, int currentSelect){
    for (int i = 0; i < number; i++) {

        if(i == currentSelect){
            printf("\x1b[1;30;47m");
        }

        if (S_ISDIR(files[i].type)) {
            printf("%25s - FOLD\n", files[i].name);
        } 
        else if (S_ISREG(files[i].type)) {
            printf("%25s - FILE\n", files[i].name);
        }

        if(i == currentSelect){
            printf("\x1b[0m");
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
        if (stat(entry->d_name, &info) == -1) {
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
        (*files)[*number].size = info.st_size;
        (*number)++;
    }
    closedir(dir);
}

void printPermissions(mode_t mode)
{
    mode & S_IRUSR ? printf("r") : printf("-");

    mode & S_IWUSR ? printf("w") : printf("-");

    mode & S_IXUSR ? printf("x") : printf("-");

    printf("/");

    mode & S_IRGRP ? printf("r") : printf("-");

    mode & S_IWGRP ? printf("w") : printf("-");

    mode & S_IXGRP ? printf("x") : printf("-");

    printf("/");

    mode & S_IROTH ? printf("r") : printf("-");

    mode & S_IWOTH ? printf("w") : printf("-");

    mode & S_IXOTH ? printf("x") : printf("-");
}

void printDate(time_t timestamp){
    char buffer[100];
    struct tm *timeinfo = localtime(&timestamp);

    strftime(buffer, sizeof(buffer), "%d/%m/%Y -- %H:%M:%S", timeinfo);

    printf("%s", buffer);
}

void printOwner(uid_t uid){
    struct passwd *user = getpwuid(uid);

    user != NULL ? printf("%s", user->pw_name) : printf("%d", uid);
}