#include <cstdio>
#include <cstring>

char secret[] = "This is top secret!";

int main(int argc, char *argv[]) {
    char name[64] = {0};

    printf("Enter your name: ");
    if (fgets(name, 64, stdin) != NULL) {
        printf(name);
    }
}

