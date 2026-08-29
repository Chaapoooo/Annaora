#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>

#define BUFFER_SIZE 30

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

    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){
        printf("%s\n", entry->d_name);
    }

    closedir(dir);

    // ------------------------------------------------------------------


    // BUFFER & getcwd

    char buffer[BUFFER_SIZE];
    if(getcwd(buffer, BUFFER_SIZE) == NULL){
        printf("Cannot get current working directory path!\n");
        return 1;
    }

    printf("Current working directory: %s\n", buffer);

    // ------------------------------------------------------------------


    return 0;
}