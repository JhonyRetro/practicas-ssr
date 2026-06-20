#include <stdio.h>
#include <unistd.h>

void smash() {
    char buffer[500];
    int count;

    count = read(0, buffer, 700);
    printf("User provided %d bytes. Buffer contains %s\n", count, buffer);
}

int main(void) {
    smash();

    return 0;
}
