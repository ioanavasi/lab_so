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
#include <sys/wait.h>
#include <stdint.h>

void grayscale(const char *path) {
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("New file couldn't be created");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, 18, SEEK_CUR) < 0) {
        perror("lseek error");
        exit(EXIT_FAILURE);
    }

    int width = 0, height = 0;
    if (read(fd, &width, sizeof(width)) < 0 || read(fd, &height, sizeof(height)) < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    uint16_t bits_per_pixel;
    if (lseek(fd, 28, SEEK_SET) < 0) {
        perror("lseek error");
        exit(EXIT_FAILURE);
    }

    if (read(fd, &bits_per_pixel, 2) < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    }

    bits_per_pixel = (bits_per_pixel >> 8) | (bits_per_pixel << 8);

    if (bits_per_pixel <= 8) {
        fprintf(stderr, "Unsupported format\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *pixel_arr = malloc(width * height * 3);
    if (pixel_arr == NULL) {
        perror("Malloc error");
        exit(EXIT_FAILURE);
    }

    if (lseek(fd, 54, SEEK_SET) < 0) {
        perror("lseek error");
        exit(EXIT_FAILURE);
    }

    ssize_t bytes_read = read(fd, pixel_arr, width * height * 3);
    if (bytes_read < 0) {
        perror("Read error");
        exit(EXIT_FAILURE);
    } else if (bytes_read != width * height * 3) {
        fprintf(stderr, "Unexpected number of bytes read: %zd\n", bytes_read);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            uint8_t *pixel = &pixel_arr[((i * width) + j) * 3];
            uint8_t gray = (uint8_t)(0.299 * pixel[2] + 0.587 * pixel[1] + 0.114 * pixel[0]);

            pixel[0] = pixel[1] = pixel[2] = gray;
        }
    }

    if (lseek(fd, 54, SEEK_SET) < 0) {
        perror("lseek error");
        exit(EXIT_FAILURE);
    }

    if (write(fd, pixel_arr, width * height * 3) < 0) {
        perror("Write error");
        exit(EXIT_FAILURE);
    }

    free(pixel_arr);
    close(fd);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <director_intrare> <director_iesire> <c>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *d = opendir(argv[1]);
    if (d == NULL) {
        perror("Directory couldn't be open");
        exit(EXIT_FAILURE);
    }

    char filePath[64] = "";
    char outpath[64] = "";
    struct stat fst;

    struct dirent *dnt;
    char buffer[1024] = "";
    int counter = 0;
    
    while ((dnt = readdir(d)) != NULL) {
        if (strcmp(dnt->d_name, ".") == 0 || strcmp(dnt->d_name, "..") == 0) {
            continue;
        }

        strcpy(outpath, "");
        strcat(outpath, argv[2]);
        strcat(outpath, "/");
        strcat(outpath, dnt->d_name);
        strcat(outpath, "_statistica.txt");

        int fd_out = creat(outpath, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd_out < 0) {
            perror("New file couldn't be created");
            exit(EXIT_FAILURE);
        }

        strcpy(filePath, "");
        strcat(filePath, argv[1]);
        strcat(filePath, "/");
        strcat(filePath, dnt->d_name);

        if (lstat(filePath, &fst) < 0) {
            perror("Could not use lstat");
            continue;
        }

        int pipe1_fd[2], pipe2_fd[2];
        if (pipe(pipe1_fd) < 0 || pipe(pipe2_fd) < 0) {
            perror("Pipe error");
            exit(EXIT_FAILURE);
        }

        int pid = fork();
        if (pid < 0) {
            perror("fork error");
            exit(EXIT_FAILURE);
        }

        if (pid == 0 && S_ISDIR(fst.st_mode)) { // child process
            snprintf(buffer, 1024, "nume director: %s\n"
            "identificatorul utilizatorului(uid): %d\n"
            "drepturi de acces user: %c%c%c\n"
            "drepturi de acces grup: %c%c%c\n"
            "drepturi de acces altii: %c%c%c\n\n\n", dnt -> d_name, fst.st_uid, 
            (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
            (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
            (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
            if (write(fd_out, buffer, strlen(buffer)) < 0) {
                perror("Failed");
                exit(-1);
            }
            exit(0);
        }

        if(pid == 0 && S_ISLNK(fst.st_mode)) { // child process
            struct stat target;
            if (stat(filePath, &target) < 0) {
                perror("Could not use stat");
                exit(-1);
            }
            snprintf(buffer, 1024, "nume legatura: %s\ndimensiune: %ld\ndimensiunea fisier: %ld\n"
            "drepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\n"
            "drepturi de acces altii: %c%c%c\n\n",
            dnt -> d_name, fst.st_size, target.st_size,
            (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
            (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
            (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
            if (write(fd_out, buffer, strlen(buffer)) < 0) {
                perror("Failed");
                exit(-1);
            }
            exit(0);
            
        }

        if (S_ISREG(fst.st_mode)) {
            int sizefp = strlen(filePath);
            char timeStr[32] = "";
            if (pid == 0) {
                time_t time = fst.st_mtime;
                struct tm *ltime = localtime(&time);
                strftime(timeStr, 32, "%c", ltime);
            }

            if (filePath[sizefp - 1] == 'p' && filePath[sizefp - 2] == 'm' && filePath[sizefp - 3] == 'b' 
            && filePath[sizefp - 4] == '.') {
                if (pid > 0) { // parent
                    int pid2 = fork();
                    if (pid2 < 0) {
                        perror("");
                        exit(-1);
                    }

                    if (pid2 == 0) {
                       grayscale(filePath);
                       exit(0);
                    }

                }
                else { // 1st child
                    int fd = open(filePath, O_RDONLY);
                    if (fd < 0) {
                        perror("");
                        exit(-1);
                    }
                    if (lseek(fd, 18, SEEK_SET) < 0) {
                        perror("");
                        exit(-1);
                    }
                    int width = 0, height = 0;
                    if (read(fd, &width, 4) < 0) {
                        perror("");
                        exit(-1);
                    }
                    if (read(fd, &height, 4) < 0) {
                        perror("");
                        exit(-1);
                    }
                    snprintf(buffer, 1024, 
                    "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\n"
                    "timpul ultimei modificari:%s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\n"
                    "drepturi de acces grup: %c%c%c\n"
                    "drepturi de acces altii: %c%c%c\n\n",
                    dnt -> d_name, height, width, fst.st_size, fst.st_uid, timeStr, fst.st_nlink,
                    (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
                    (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
                    (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
                    if (write(fd_out, buffer, strlen(buffer)) < 0) {
                        perror("Failed");
                        exit(-1);
                    }
                    close(fd);
                    exit(0);
                }
            }
            else { 
                if (pid == 0) {
                snprintf(buffer, 1024, 
                "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\n"
                "timpul ultimei modificari:%s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\n"
                "drepturi de acces grup: %c%c%c\n"
                "drepturi de acces altii: %c%c%c\n\n",
                dnt -> d_name, fst.st_size, fst.st_uid, timeStr, fst.st_nlink,
                (fst.st_mode & S_IRUSR) ? 'R' : '-', (fst.st_mode & S_IWUSR) ? 'W' : '-', (fst.st_mode & S_IXUSR) ? 'X' : '-',
                (fst.st_mode & S_IRGRP) ? 'R' : '-', (fst.st_mode & S_IWGRP) ? 'W' : '-', (fst.st_mode & S_IXGRP) ? 'X' : '-',
                (fst.st_mode & S_IROTH) ? 'R' : '-', (fst.st_mode & S_IWOTH) ? 'W' : '-', (fst.st_mode & S_IXOTH) ? 'X' : '-');
                if (write(fd_out, buffer, strlen(buffer)) < 0) {
                    perror("Failed");
                    exit(-1);
                }
                close(pipe2_fd[0]);
                close(pipe2_fd[1]);
                close(pipe1_fd[0]);
                if (dup2(pipe1_fd[1], 1) < 0) {
                    perror("Error redirecting stdout");
                    exit(-1);
                }
                execlp("cat", "cat", filePath, NULL);
                close(pipe1_fd[1]);
                exit(-1);
                }
                else {
                    int pid2 = fork();
                    if (pid2 < 0) {
                        perror("Fork error");
                        exit(-1);
                    }
                    if (pid2 == 0) {
                        close(pipe1_fd[1]);
                        if (dup2(pipe1_fd[0], 0) < 0) {
                            perror("Error redirecting stdout");
                            exit(-1);
                        }
                        close(pipe2_fd[0]);
                        if (dup2(pipe2_fd[1], 1) < 0) {
                            perror("Error redirecting stdout");
                            exit(-1);
                        }
                        execlp("sh", "sh", "./script.sh", argv[3], NULL);
                        exit(-1);
                    }
                    else {
                        close(pipe2_fd[1]);
                        char readbuff[16];
                        if (read(pipe2_fd[0], readbuff, 16) < 0) {
                            perror("Read error");
                            exit(-1);
                        }
                        close(pipe1_fd[0]);
                        close(pipe1_fd[1]);
                        counter += atoi(readbuff);
                    }
                }
            }
        }
        if (close(fd_out) < 0) {
            perror("Error closing output file");
            exit(EXIT_FAILURE);
        }

        close(pipe2_fd[0]);
        close(pipe2_fd[1]);
        close(pipe1_fd[0]);
        close(pipe1_fd[1]);
    }

    int pid_child, status;
    while ((pid_child = wait(&status)) > 0) {
        printf("S-a incheiat procesul cu pid-ul %d si codul %d\n", pid_child, status);
    }

    if (closedir(d) < 0) {
        perror("Error closing output file");
        exit(EXIT_FAILURE);
    }

    printf("Au fost identificate in total %d propozitii corecte care contin caracterul %c\n", counter, argv[3][0]);
    return 0;
}



