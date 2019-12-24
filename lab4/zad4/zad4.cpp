#include <cstdio>

int main(int argc, char *argv[]) {
    char name[100];

    printf("Enter your name: ");
    if (fgets(name, 100, stdin) != NULL) {
        printf(name);
    }
}
