#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>


int main(int argc, char **argv){
    if (argc != 2){
        printf("Usage ./main <fisier_intrare>");
        exit(-1);
    }

    DIR *d = opendir(argv[1]);
    if (d == NULL){
        perror("Directory couldn't be open");
        exit(-1);
    }

    int fd_out = creat("statistica.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (fd_out < 0){
        perror("New file couldn't be created");
        exit(-1);
    }

    char filePath[64] = "";
    struct stat fst;

    struct dirent *dnt;
    char buffer[1024] = "";
    while (dnt = readdir(d)) {
        if (strcmp(dnt -> d_name, ".") == 0 || strcmp(dnt -> d_name, "..") == 0) {
            continue;
        }
        printf("%s\n", dnt -> d_name);
        strcpy(filePath, "");
        strcat(filePath, argv[1]);
        strcat(filePath, "/");
        strcat(filePath, dnt -> d_name);
        if (lstat(filePath, &fst) < 0) {
            perror("Could not use lstat");
            continue;
        }
        if (S_ISDIR(fst.st_mode)) {
            snprintf(buffer, 1024, "nume director: %s\n"
            "identificatorul utilizatorului(uid): %d\n"
            "drepturi de acces user: %c%c%c\n"
            "drepturi de acces grup: %c%c%c\n"
            "drepturi de acces altii: %c%c%c\n\n\n", filePath, fst.st_uid, 
            (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
            (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
            (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
            if (write(fd_out, buffer, strlen(buffer)) < 0) {
                perror("Failed");
                continue;
            }
        }

        if(S_ISLNK(fst.st_mode)) {
            struct stat target;
            if (stat(filePath, &target) < 0) {
                perror("Could not use stat");
                continue;
            }
            snprintf(buffer, 1024, "nume legatura: %s\ndimensiune: %ld\ndimensiunea fisier: %ld\n"
            "drepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\n"
            "drepturi de acces altii: %c%c%c\n\n",
            filePath, fst.st_size, target.st_size,
            (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
            (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
            (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
            if (write(fd_out, buffer, strlen(buffer)) < 0) {
                perror("Failed");
                continue;
            }
        }

        if (S_ISREG(fst.st_mode)) {
            int sizefp = strlen(filePath);
            time_t time = fst.st_mtime;
            struct tm *ltime = localtime(&time);
            char timeStr[32] = "";
            strftime(timeStr, 32, "%c", ltime);

            if (filePath[sizefp - 1] == 'p' && filePath[sizefp - 2] == 'm' && filePath[sizefp - 3] == 'b' 
            && filePath[sizefp - 4] == '.') {
                int fd = open(filePath, O_RDONLY);
                if (fd < 0) {
                    perror("");
                    continue;
                }
                if (lseek(fd, 18, SEEK_SET) < 0) {
                    perror("");
                    continue;
                }
                int width = 0, height = 0;
                if (read(fd, &width, 4) < 0) {
                    perror("");
                    continue;
                }
                if (read(fd, &height, 4) < 0) {
                    perror("");
                    continue;
                }
                 snprintf(buffer, 1024, 
                "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\n"
                "timpul ultimei moldificari:%s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\n"
                "drepturi de acces grup: %c%c%c\n"
                "drepturi de acces altii: %c%c%c\n\n",
                filePath, height, width, fst.st_size, fst.st_uid, timeStr, fst.st_nlink,
                (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
                (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
                (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
                close(fd);
            }
            else {
                snprintf(buffer, 1024, 
                "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\n"
                "timpul ultimei moldificari:%s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\n"
                "drepturi de acces grup: %c%c%c\n"
                "drepturi de acces altii: %c%c%c\n\n",
                filePath, fst.st_size, fst.st_uid, timeStr, fst.st_nlink,
                (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
                (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
                (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
            }
            if (write(fd_out, buffer, strlen(buffer)) < 0) {
                perror("Failed");
                continue;
            }
        }
    }
    if (close(fd_out) < 0) {
            perror("Error closing output file");
            exit(-1);
        }
         if (closedir(d) < 0) {
            perror("Error closing output file");
             exit(-1);
         }
}




