
/**
 * Run these files:
 *
 * $ gcc -o semsrv semsrv.c
 * $ gcc -o semcli semcli.c
 * $ ./semsrv
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

#include "common.h"

#define CLIENT_PATH_BUFSIZE 255

int
main(int argc, char *argv[]) {
    key_t sem_key;
    int sem_id;
    int sem_fd;
    char client_exe[CLIENT_PATH_BUFSIZE];
    int dir_len;
    int i;
    struct sembuf sop;
    int pid;
    int status;

    sem_key = ftok("./semsrv.c", 42);

    // Write the key to a file for children to pick it up
    sem_fd = open(SEM_KEY_FILE, O_WRONLY | O_TRUNC | O_EXCL | O_CREAT, 0644);
    if (sem_fd < 0) {
        perror("Could not open sem.key");
        exit(1);
    }

    // Actual write of the key
    if (write(sem_fd, &sem_key, sizeof(key_t)) < 0) {
        perror("Could not write key to file");
        exit(2);
    }

    // Done with the key
    close(sem_fd);

    // Create the semaphore
    sem_id = semget(sem_key, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (sem_id < 0) {
        perror("Could not create sem");
        unlink(SEM_KEY_FILE);
        exit(3);
    }

    if (semctl(sem_id, 0, SETVAL, 0) < 0) {
        perror("Could not set value of semaphore");
        exit(4);
    }

    // Now create some clients
    // First create the path to the client exec
    getcwd(client_exe, CLIENT_PATH_BUFSIZE);
    dir_len = strlen(client_exe);
    strcpy(client_exe + dir_len, "/semcli");
    printf("%s\n", client_exe);

    for (i = 0; i < 5; ++i) {
        if ((pid = fork()) < 0) {
            perror("Could not fork, please create clients manually");
        } else if (pid == 0) {
            // We're in the child process, start a client
            execl(client_exe, "semcli", (char *)0);
            _exit(127);
        }
    }

    printf("Done creating clients, sleeping for a while\n");
    sleep(5);
    printf("Increasing sem count\n");
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;
    if (semop(sem_id, &sop, 1)) {
        perror("Could not increment semaphore");
        exit(5);
    }
    // Wait for all children to finish
    for (;;) {
        // Remove the zombie process, and get the pid and return code
        pid = wait(&status);
        if (pid < 0) {
            if (errno == ECHILD) {
                printf("All children have exited\n");
                break;
            } else {
                perror("Could not wait");
            }
        } else {
            printf("Child %d exited with status %d\n", pid, status);
        }
    }

    // Delete semaphore and file
    if (unlink(SEM_KEY_FILE) < 0) {
        perror("Could not unlink key file");
    }

    if (semctl(sem_id, 0, IPC_RMID) < 0) {
        perror("Could not delete semaphore");
    }

    exit(0);
}
