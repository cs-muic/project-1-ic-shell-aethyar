/* ICCS227: Project 1: icsh
 * Name: Adrian Jay Ang
 * StudentID: 6380989
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define MAX_CMD_BUFFER 255

int main() {
    printf("Starting IC shell\n");
    char buffer[MAX_CMD_BUFFER];
    while (1) {
        printf("icsh $ ");
        fgets(buffer, 255, stdin);
        // printf("you said: %s\n", buffer);
        if (strncmp(buffer, "echo ", 5) == 0) {
            printf("%s", buffer + 5);
        }
    }
}
