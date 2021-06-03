#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <string.h>
#include <dlfcn.h>

int main(void) {
    int fd;
    void *lib;
    void (*funct)();
    char code[] = "#include <stdio.h>\nvoid hello() { printf(\"hello, world\\n\"); }\n";

    /* Create hello.c */
    if ((fd = open("./hello.c", O_WRONLY | O_CREAT, 0640)) < 1) {
        perror("cannot open \"hello.c\"");
        return 1;
    }
    write(fd, code, strlen(code));
    close(fd);

    /* compile hello.c */
    if (system("gcc -shared -fPIC -G hello.c -o hello.so") < 0) 
        return 2;
         
    /* read hello.so */
    if (!((lib = dlopen("./hello.so", RTLD_LAZY)))) 
        return 3;

    /* retrieve "hello" symbol pointer */
    if (!(funct = (void (*)()) dlsym(lib, "hello"))) {
        dlclose(lib);
        return 4;
    }

    /* call funct() */
    funct();

    dlclose(lib);
    return 0;
}
