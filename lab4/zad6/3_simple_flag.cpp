#include <cstdio>

bool check_user(const char* name) { return false; }

int is_root = false;

int main(int argc, char *argv[]) {
    char name[64] = {0};

    printf("Enter your name: ");
    if (fgets(name, 64, stdin) != NULL) {
        printf(name);
    }

    if (is_root || check_user(name)) {
        printf("Valid user\n");
    } else {
        printf("Invalid user\n");
    }
}
