#include <linux/limits.h>
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
#include <sys/vfs.h>
#include <math.h>
#include <fcntl.h>
#include <limits.h>
#include <fcntl.h>

#define BUFFER_SIZE 30

typedef struct{
    char *name;
    unsigned int type;
    off_t size;
    time_t modified;
    uid_t owner;
    bool isSymlink;
} File;

typedef struct {
    char relPath[PATH_MAX];
    bool isDir;
} CopyNode;

void listFiles(File *files, int number, int currentSelect);
void getCurrentWorkingDirectory();
void refreshListFile(File **files, int *number, int *capacity);

void printPermissions(mode_t mode);
void printDate(time_t timestamp);
void printOwner(uid_t uid);
off_t getFolderSize(const char *path, dev_t filesystem, unsigned long *fileCount, unsigned long *dirCount);
dev_t getFilesystemDevice(const char *path);
void convertSize(off_t size);

void copyFile(const char *source, const char *destination);
void copyFolder(const char *source, const char *destination);
void scanFolder(const char *baseSource, const char *currentSubPath, CopyNode **nodes, int *count, int *capacity);

void moveFile(const char *source, const char *destination);

void deleteFile(const char *path);
void deleteFolder(const char *path);

bool validName(const char *name);

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
            if(S_ISDIR(files[number].type)) {
                printf("   -->%39s/ - FOLD\n", files[number].name);
            }
            else if(S_ISREG(files[number].type)) {
                printf("   -->%40s - FILE \n", files[number].name);
            }
            printf("\x1b[0m");

        } else {
            if(S_ISDIR(files[number].type)) {
                printf("%45s/ - FOLD\n", files[number].name);
            }
            else if(S_ISREG(files[number].type)) {
                printf("%46s - FILE \n", files[number].name);
            }
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

    bool copyMode = false;
    char *copySource = NULL;

    bool moveMode = false;
    char *moveSource = NULL;

    bool deleteMode = false;
    bool deleteKey = false;
    char *deleteSource = NULL;

    isFolder ? printf("\n%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name) : printf("\n%03d %44s FILE \n", currentSelect, files[currentSelect].name);

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
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                } else if(S_ISREG(files[currentSelect].type)){
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number, currentSelect);
                    printf("\n\r%03d %44s FILE\n", currentSelect, files[currentSelect].name);
                }
            }

            if(c2 == '[' && c3 == '3'){
                char c4 = getchar();

                if(c4 == '~'){
                    deleteKey = true;
                }
            }
        }

        if(key == '\n' && S_ISDIR(files[currentSelect].type)){
            printf("\rEntering folder %s\n", files[currentSelect].name);

            if (chdir(files[currentSelect].name) == -1) {
                perror("chdir");
            } else{
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
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
            }
        }

        if(key == 127 || key == 8){
            if (chdir("..") == -1) {
                perror("chdir");
            } else{
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
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
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
                printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
            }
        }

        if((key == 'o' || key == 'O') && S_ISDIR(files[currentSelect].type)){
            system("clear");

            printf("Informations for FOLDER %s\n", files[currentSelect].name);
            printf("\n");

            unsigned long fileCount = 0;
            unsigned long dirCount = 0;

            printf("Size: ");
            if (S_ISDIR(files[currentSelect].type)) {
                off_t folderSize = getFolderSize(files[currentSelect].name, getFilesystemDevice(files[currentSelect].name), &fileCount, &dirCount);
                convertSize(folderSize);
            } else {
                convertSize(files[currentSelect].size);
            }

            printf("\n");
            printf("Contains: %lu file(s), %lu folder(s)\n", fileCount, dirCount);

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
                printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
            }
        }

        if(key == 'c' || key == 'C'){
            char currentPath[PATH_MAX];

            if(getcwd(currentPath, sizeof(currentPath)) == NULL){
                perror("getcwd");
                exit(1);
            }

            copySource = malloc(strlen(currentPath) + 1 + strlen(files[currentSelect].name) + 1);

            if(copySource == NULL){
                perror("malloc");
                exit(1);
            }

            sprintf(copySource, "%s/%s", currentPath, files[currentSelect].name);
            copyMode = true;

            while(copyMode){
                system("clear");
                printf("COPY MODE\n");
                printf("Copy: %s\n\n", copySource);
                printf("[Y-CONFIRM / Q-CANCEL]\n");
                listFiles(files, number, currentSelect);
                int copyKey = getchar();

                if(copyKey == 'q' || copyKey == 'Q'){
                    copyMode = false;
                }

                if(copyKey == 'y' || copyKey == 'Y'){
                    char destinationPath[PATH_MAX];

                    if(getcwd(destinationPath, sizeof(destinationPath)) == NULL){
                        perror("getcwd");
                        exit(1);
                    }

                    char *sourceName = strrchr(copySource, '/');

                    if(sourceName == NULL){
                        printf("Invalid source path.\n");
                        copyMode = false;
                        continue;
                    }

                    sourceName++;
                    char destination[PATH_MAX];
                    snprintf(destination, sizeof(destination), "%s/%s", destinationPath, sourceName);
                    struct stat sourceInfo;

                    if(lstat(copySource, &sourceInfo) == -1){
                        perror("lstat");
                    }
                    else if(S_ISREG(sourceInfo.st_mode)){
                        copyFile(copySource, destination);
                    }
                    else if(S_ISDIR(sourceInfo.st_mode)){
                        copyFolder(copySource, destination);
                    }

                    copyMode = false;
                    refreshListFile(&files, &number, &capacity);
                    currentSelect = 0;

                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number, currentSelect);

                    if(S_ISDIR(files[currentSelect].type)){
                        printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                    }
                    else if(S_ISREG(files[currentSelect].type)){
                        printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                    }
                }

                if(copyKey == '\033'){
                    char c2 = getchar();
                    char c3 = getchar();
                    if(c2 == '['){
                        if(c3 == 'A'){
                            currentSelect == 0 ? currentSelect = number - 1 : currentSelect--;
                        }

                        if(c3 == 'B'){
                            currentSelect == number - 1 ? currentSelect = 0 : currentSelect++;
                        }
                    }
                }

                if(copyKey == '\n' && S_ISDIR(files[currentSelect].type)){
                    if(chdir(files[currentSelect].name) == -1){
                        perror("chdir");
                    } else{
                        closedir(dir);
                        dir = opendir(".");

                        if(dir == NULL){
                            perror("opendir");
                            exit(1);
                        }

                        refreshListFile(&files, &number, &capacity);
                        currentSelect = 0;
                    }
                }
            }

            free(copySource);
            copySource = NULL;

            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);
            if(S_ISDIR(files[currentSelect].type)){
                printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
            }
        
            continue;
        }
    
        if(key == 'm' || key == 'M'){
            char currentPath[PATH_MAX];
                
            if(getcwd(currentPath, sizeof(currentPath)) == NULL){
                perror("getcwd"); exit(1);
            }
        
            moveSource = malloc(strlen(currentPath) + 1 + strlen(files[currentSelect].name) + 1);
        
            if(moveSource == NULL){
                perror("malloc");
                exit(1);
            }
        
            sprintf(moveSource, "%s/%s", currentPath, files[currentSelect].name);
            moveMode = true;
        
            while(moveMode){
                system("clear");
                printf("MOVE MODE\n");
                printf("Move: %s\n\n", moveSource);
                printf("[Y-CONFIRM / Q-CANCEL]\n");
                listFiles(files, number, currentSelect);
            
                int moveKey = getchar();
            
                if(moveKey == 'q' || moveKey == 'Q'){
                    moveMode = false;
                }
            
                if(moveKey == 'y' || moveKey == 'Y'){
                    char destinationPath[PATH_MAX];
                
                    if(getcwd(destinationPath, sizeof(destinationPath)) == NULL){
                        perror("getcwd");
                        exit(1);
                    }

                    char *sourceName = strrchr(moveSource, '/');
                    printf("%c", *sourceName);
                
                    if(sourceName == NULL){
                        printf("Invalid source path.\n");
                        moveMode = false;
                        continue;
                    }
                
                    sourceName++;
                    char destination[PATH_MAX];
                    snprintf(destination, sizeof(destination), "%s/%s", destinationPath, sourceName);
                    moveFile(moveSource, destination);
                    moveMode = false;
                    refreshListFile(&files, &number, &capacity);
                    currentSelect = 0;
                
                    system("clear");
                    printf("Annaora file manager!\n");
                    listFiles(files, number, currentSelect);
                
                    if(S_ISDIR(files[currentSelect].type)){
                        printf(
                            "\n\r%03d %43s/ FOLD\n",
                            currentSelect,
                            files[currentSelect].name
                        );
                    }
                    else if(S_ISREG(files[currentSelect].type)){
                        printf(
                            "\n\r%03d %44s FILE \n",
                            currentSelect,
                            files[currentSelect].name
                        );
                    }
                }

                if(moveKey == '\033'){
                    char c2 = getchar();
                    char c3 = getchar();

                    if(c2 == '['){
                        if(c3 == 'A'){
                            currentSelect == 0
                                ? currentSelect = number - 1
                                : currentSelect--;
                        }

                        if(c3 == 'B'){
                            currentSelect == number - 1
                                ? currentSelect = 0
                                : currentSelect++;
                        }
                    }
                }

                if(moveKey == '\n' && S_ISDIR(files[currentSelect].type)){
                    if(chdir(files[currentSelect].name) == -1){
                        perror("chdir");
                    }
                    else{
                        closedir(dir);
                        dir = opendir(".");

                        if(dir == NULL){
                            perror("opendir");
                            exit(1);
                        }

                        refreshListFile(&files, &number, &capacity);
                        currentSelect = 0;
                    }
                }
            }

            free(moveSource);
            moveSource = NULL;

            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);

            if(S_ISDIR(files[currentSelect].type)){
                printf(
                    "\n\r%03d %43s/ FOLD\n",
                    currentSelect,
                    files[currentSelect].name
                );
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf(
                    "\n\r%03d %44s FILE \n",
                    currentSelect,
                    files[currentSelect].name
                );
            }

            continue;
        }

        if(key == 'd' || key == 'D' || deleteKey){
            char currentPath[PATH_MAX];
                
            if(getcwd(currentPath, sizeof(currentPath)) == NULL){
                perror("getcwd"); exit(1);
            }
        
            deleteSource = malloc(strlen(currentPath) + 1 + strlen(files[currentSelect].name) + 1);
        
            if(deleteSource == NULL){
                perror("malloc");
                exit(1);
            }
        
            sprintf(deleteSource, "%s/%s", currentPath, files[currentSelect].name);
            deleteMode = true;
        
            while(deleteMode){
                system("clear");
                printf("DELETE MODE\n");
                printf("Delete: %s ?\n\n", deleteSource);
                printf("[Y-CONFIRM / Q-CANCEL] (y by default)\n");
            
                int confirmKey = getchar();
            
                if(confirmKey == 'q' || confirmKey == 'Q'){
                    deleteKey = false;
                    deleteMode = false;
                }

                if(confirmKey == 'y' || confirmKey == 'Y' || confirmKey == '\n'){
                    char *selectedName = NULL;
                    if(number > 0 && currentSelect < number){
                        selectedName = strdup(files[currentSelect].name);
                    }

                    deleteFile(deleteSource);
                    refreshListFile(&files, &number, &capacity);
                    if (selectedName != NULL) {
                        for (int i = 0; i < number; i++) {
                            if (strcmp(files[i].name, selectedName) == 0) {
                                currentSelect = i;
                                break;
                            }
                        }
                        free(selectedName);
                    }

                    if (currentSelect >= number && number > 0) {
                        currentSelect = number - 1;
                    }
                    
                    deleteKey = false;
                    deleteMode = false;
                }
                else if (confirmKey != 'y' && confirmKey != 'q' && confirmKey != '\n'){
                    system("clear");
                }
            }

            free(deleteSource);
            deleteSource = NULL;

            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);

            if (number > 0) {
                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
            }

            continue;
        }

        if(key == 'r' || key == 'R'){
            system("clear");

            printf("RENAME MENU FOR %s\n", files[currentSelect].name);
            printf("New name: ");

            fflush(stdout);
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

            char newName[NAME_MAX];

            if(fgets(newName, sizeof(newName), stdin) == NULL){
                tcsetattr(STDIN_FILENO, TCSANOW, &newt); continue;
            }

            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            newName[strcspn(newName, "\n")] = '\0';

            if(newName[0] == '\0'){
                system("clear");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);
                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                } else {
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            if(!validName(newName)){
                printf("Invalid Name: '/' or '\\0' are not allowed.\n");
                printf("Press any arrow key to exit.\n");
                continue;
            }

            char currentPath[PATH_MAX];

            if(getcwd(currentPath, sizeof(currentPath)) == NULL){
                perror("getcwd");
                continue;
            }

            char destination[PATH_MAX];
            snprintf(destination, sizeof(destination), "%s/%s", currentPath, newName);

            if(access(destination, F_OK) == 0){
                printf("\nA file or folder with this name already exists.\n"); 
                printf("Press any arrow key to exit.\n");
                continue;
            }

            char oldPath[PATH_MAX];
            snprintf(oldPath, sizeof(oldPath), "%s/%s", currentPath, files[currentSelect].name);

            if(rename(oldPath, destination) == -1){
                perror("rename");
                getchar();
                continue;
            }

            refreshListFile(&files, &number, &capacity);

            for(int i = 0; i < number; i++){
                if(strcmp(files[i].name, newName) == 0){ 
                    currentSelect = i; 
                    break; 
                }
            }

            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);

            if(S_ISDIR(files[currentSelect].type)){
                printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
            }
        }

        if(key == 't' || key == 'T'){
            system("clear");

            printf("MKDIR / TOUCH MENU\n\n");
            printf("F - File\n");
            printf("D - Directory\n");
            printf("Q - Cancel\n");

            printf("Choose: ");
            fflush(stdout);

            char type;
            type = getchar();

            if(type == 'q' || type == 'Q'){
                refreshListFile(&files, &number, &capacity);
                system("clear");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);

                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }
            
            if(type == '\n'){
                refreshListFile(&files, &number, &capacity);
                system("clear");
                printf("Invalid choice.\n");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);

                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            if(type != 'f' && type != 'F' && type != 'd' && type != 'D'){ 
                refreshListFile(&files, &number, &capacity);
                system("clear");
                printf("Invalid choice.\n");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);

                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

            char newName[CHAR_MAX];
            printf("\nName: ");
            fflush(stdout);


            if(fgets(newName, sizeof(newName), stdin) == NULL){
                tcsetattr(STDIN_FILENO, TCSANOW, &newt);
                continue;
            }

            tcsetattr(STDIN_FILENO, TCSANOW, &newt);
            newName[strcspn(newName, "\n")] = '\0';

            if(newName[0] == '\0'){
                system("clear");
                printf("A file or folder name can't be empty !\n");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);

                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            if(!validName(newName)){
                refreshListFile(&files, &number, &capacity);
                system("clear");
                printf("Invalid Name: '/' or '\\0' are not allowed.");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);
                
                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            char currentPath[PATH_MAX];

            if(getcwd(currentPath, sizeof(currentPath)) == NULL){
                perror("getcwd");
                continue;
            }

            char newPath[PATH_MAX];
            snprintf(newPath, sizeof(newPath), "%s/%s", currentPath, newName);

            if(access(newPath, F_OK) == 0){
                refreshListFile(&files, &number, &capacity);
                system("clear");
                printf("A file or folder with this name already exists !\n");
                printf("Annaora file manager!\n");
                listFiles(files, number, currentSelect);

                if(S_ISDIR(files[currentSelect].type)){
                    printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
                }
                else if(S_ISREG(files[currentSelect].type)){
                    printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
                }
                continue;
            }

            if(type == 'd' || type == 'D'){
                if(mkdir(newPath, 0755) == -1){
                    perror("mkdir");
                    continue;
                }

                printf("\nDirectory successfully created !\n");
            } else {
                int fd = open(newPath, O_WRONLY | O_CREAT | O_EXCL, 0644);
                if(fd == -1){
                    perror("touch");
                    continue;
                }

                close(fd);
                printf("\nFile successfully created !\n");
            }

            refreshListFile(&files, &number, &capacity);

            for(int i = 0; i < number; i++){
                if(strcmp(files[i].name, newName) == 0){ 
                    currentSelect = i; 
                    break; 
                }
            }

            system("clear");
            printf("Annaora file manager!\n");
            listFiles(files, number, currentSelect);

            if(S_ISDIR(files[currentSelect].type)){
                printf("\n\r%03d %43s/ FOLD\n", currentSelect, files[currentSelect].name);
            }
            else if(S_ISREG(files[currentSelect].type)){
                printf("\n\r%03d %44s FILE \n", currentSelect, files[currentSelect].name);
            }

        }
    }

    // -----------------------------------------------------------------

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    free(copySource);
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

            if(S_ISDIR(files[i].type)) {
                printf("   -->%39s/ - FOLD\n", files[i].name);
            }
            else if(S_ISREG(files[i].type)) {
                printf("   -->%40s - FILE \n", files[i].name);
            }
            printf("\x1b[0m");
        } else {
            if(S_ISDIR(files[i].type)) {
                printf("%45s/ - FOLD\n", files[i].name);
            }
            else if(S_ISREG(files[i].type)) {
                printf("%46s - FILE \n", files[i].name);
            }
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

        struct stat linkInfo;

        if (lstat(entry->d_name, &linkInfo) == -1) {
            continue;
        }

        strcpy((*files)[*number].name, entry->d_name);
        (*files)[*number].type = info.st_mode;
        (*files)[*number].size = info.st_size;
        (*files)[*number].modified = info.st_mtime;
        (*files)[*number].owner = info.st_uid;
        (*files)[*number].isSymlink = S_ISLNK(linkInfo.st_mode);
        (*number)++;
    }
    closedir(dir);
}

void printPermissions(mode_t mode){
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

off_t getFolderSize(const char *path, dev_t filesystem, unsigned long *fileCount, unsigned long *dirCount){
    DIR *dir = opendir(path);
    if (dir == NULL) {
        return 0;
    }

    off_t totalSize = 0;
    struct dirent *entry;
    int dir_fd = dirfd(dir);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        struct stat info;

        if (fstatat(dir_fd, entry->d_name, &info, AT_SYMLINK_NOFOLLOW) == -1) {
            continue;
        }

        if (info.st_dev != filesystem) {
            continue;
        }

        if (S_ISREG(info.st_mode)) {
            totalSize += info.st_size;
            if (fileCount) (*fileCount)++;
        }
        else if (S_ISDIR(info.st_mode)) {
            if (dirCount) (*dirCount)++;
            char fullPath[PATH_MAX]; 
            snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
            
            totalSize += getFolderSize(fullPath, filesystem, fileCount, dirCount);
        }
    }

    closedir(dir);
    return totalSize;
}

dev_t getFilesystemDevice(const char *path){

    struct stat info;

    if (stat(path, &info) == -1) {
        perror("stat");
        return 0;
    }

    return info.st_dev;
}

void convertSize(off_t size){
    double convertedSize = size;

    if (size >= pow(1024, 4)) {
        convertedSize /= pow(1024, 4);
        printf("%.2f TiB", convertedSize);
    } else if (size >= pow(1024, 3)) {
        convertedSize /= pow(1024, 3);
        printf("%.2f GiB", convertedSize);
    } else if (size >= pow(1024, 2)) {
        convertedSize /= pow(1024, 2);
        printf("%.2f MiB", convertedSize);
    } else if (size >= 1024) {
        convertedSize /= 1024;
        printf("%.2f KiB", convertedSize);
    } else {
        printf("%.0f B", convertedSize);
    }
}

void copyFile(const char *source, const char *destination){
    int src = open(source, O_RDONLY);

    if(src == -1){
        perror("open source");
        return;
    }

    int dest = open(destination, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(dest == -1){
        perror("open destination");
        close(src);
        return;
    }

    char buffer[4096];
    ssize_t bytesRead;

    while((bytesRead = read(src, buffer, sizeof(buffer))) > 0){
        if(write(dest, buffer, bytesRead) == -1){
            perror("write");
            break;
        }
    }

    if(bytesRead == -1){
        perror("read");
    }

    close(src);
    close(dest);
}

void copyFolder(const char *source, const char *destination){
    int capacity = 16;
    int count = 0;
    CopyNode *nodes = malloc(capacity * sizeof(CopyNode));

    scanFolder(source, "", &nodes, &count, &capacity);
    mkdir(destination, 0755);

    for(int i=0; i<count; i++){
        char srcPath[PATH_MAX];
        char dstPath[PATH_MAX];

        snprintf(srcPath, sizeof(srcPath), "%s/%s", source, nodes[i].relPath);
        snprintf(dstPath, sizeof(dstPath), "%s/%s", destination, nodes[i].relPath);

        if(nodes[i].isDir){
            mkdir(dstPath, 0755);
        } else {
            copyFile(srcPath, dstPath);
        }
    }

    free(nodes);
}

void scanFolder(const char *baseSource, const char *currentSubPath, CopyNode **nodes, int *count, int *capacity){
    char fullPath[PATH_MAX];
    if(strlen(currentSubPath) == 0){
        snprintf(fullPath, sizeof(fullPath), "%s", baseSource);
    } else {
        snprintf(fullPath, sizeof(fullPath), "%s/%s", baseSource, currentSubPath);
    }

    DIR *dir = opendir(fullPath);
    if(!dir) return;

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char entryRelPath[PATH_MAX];
        if(strlen(currentSubPath) == 0){
            snprintf(entryRelPath, sizeof(entryRelPath), "%s", entry->d_name);
        } else {
            snprintf(entryRelPath, sizeof(entryRelPath), "%s/%s", currentSubPath, entry->d_name);
        }

        char entryFullPath[PATH_MAX];
        snprintf(entryFullPath, sizeof(entryRelPath), "%s/%s", baseSource, entryRelPath);

        struct stat info;
        if(lstat(entryFullPath, &info) == -1) continue;

        if(*count >= *capacity){
            *capacity *= 2;
            *nodes = realloc(*nodes, *capacity * sizeof(CopyNode));
        }

        strcpy((*nodes)[*count].relPath, entryRelPath);
        (*nodes)[*count].isDir = S_ISDIR(info.st_mode);
        (*count) ++;

        if(S_ISDIR(info.st_mode)){
            scanFolder(baseSource, entryRelPath, nodes, count, capacity);
        }
    }
    closedir(dir);
}

void moveFile(const char *source, const char *destination){
    if(rename(source, destination) == -1){
        perror("rename");
    }
}

void deleteFile(const char *path){

    struct stat info;

    if(lstat(path, &info) == -1){
        perror("lstat");
        return;
    }

    if(S_ISDIR(info.st_mode)){
        deleteFolder(path);
    } else {
        if(remove(path)){
            perror("remove");
        }
    }
}

void deleteFolder(const char *path){
    DIR *dir = opendir(path);
    if(dir == NULL){
        perror("opendir");
        return;
    }

    struct dirent *entry;

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }

        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);

        struct stat info;

        if(lstat(fullPath, &info) == -1){
            perror("lstat");
            continue;
        }

        if(S_ISDIR(info.st_mode)){
            deleteFolder(fullPath);
            
        } else {
            if(remove(fullPath) == -1){
                perror("remove");
            }
        }
    }

    closedir(dir);
    if(rmdir(path) == -1){
        perror("rmdir");
    }
}

bool validName(const char *name){
    for(int i = 0; name[i] != '\0'; i++){
        if(name[i] == '/'){
            return false;
        }
    }
    return true;
}
