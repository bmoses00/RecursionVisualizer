#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

void execute(char* command, int pipe[]) {
    write(pipe[1], command, strlen(command));
}

void print_output(int pipe[]) {
    char buffer[1024];
    read(pipe[0], buffer, sizeof buffer);
    printf("%s\n", buffer);
}

int main() {
    system("gcc -g sum.c -o output.o");

    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);
    int id = fork();

    if (id == 0) {
        dup2(fd1[1], 1);
        dup2(fd2[0], 0);
        system("gdb output.o -q");
        close(fd1[0]);
        close(fd1[1]);

    }
    else {
        int waiting_for_gdb = 0;
        int flags;
        flags = fcntl(fd1[0], F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(fd1[0], F_SETFL, flags);

        FILE* gdb_out = fdopen(fd1[0], "r");
        char* terminate_message1 = "\"finish\" not meaningful in the outermost frame.\n";
        char *terminate_message2 = "(gdb) \"finish\" not meaningful in the outermost frame.\n";
        char buffer[128];
        char old_buffer[128];

        execute("set logging on\n", fd2);
        execute("rbreak sum.c:.\n", fd2);
        execute("run\n", fd2);
        execute("c\n", fd2);

        while(strcmp(buffer, terminate_message1) != 0 && strcmp(buffer, terminate_message2) != 0) {
            strcpy(old_buffer, buffer);
            char* read = fgets(buffer, sizeof buffer, gdb_out);
            if (read == NULL && !waiting_for_gdb) {
                printf("%s", "-----------EXECUTING FINISH-----------------\n");
                execute("finish\n", fd2);
                waiting_for_gdb = 1;
            }
            if (read != NULL && waiting_for_gdb) {
                waiting_for_gdb = 0;
            }
            if (strcmp(old_buffer, buffer) != 0) {
                printf("%s", buffer);
            }
        }
        
    }
    return 0;
}