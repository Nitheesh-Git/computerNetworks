#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5000

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    printf("[CLIENT] Connected to server.\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int valread = read(sock, buffer, 1024);
        if (valread <= 0) {
            printf("Disconnected from server.\n");
            break;
        }
        buffer[valread] = '\0';
        printf("%s\n", buffer);

        // If message contains "Your turn", take input
        if (strstr(buffer, "Your turn")) {
            char move[10];
            printf("Enter your move (1-9): ");
            fgets(move, sizeof(move), stdin);
            send(sock, move, strlen(move), 0);
        }
    }

    close(sock);
    return 0;
}

