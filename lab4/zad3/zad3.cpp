#include <stdio.h>


int zero = 0;

void fun() {
        char buffer[5] = {1, 2, 3, 4, 5};
        fread(buffer, 100, 1, stdin);

        if (zero == 0) return;

        printf("Code never reached\n");
}

int main(int argc, char *argv[]) {
    fun();
    return 0;
}