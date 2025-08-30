#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5000

char board[9] = {'-', '-', '-', '-', '-', '-', '-', '-', '-'};
int turn = 1;  // 1 = Client1 (X), 2 = Client2 (O)

// -------------------------
// Print board (server debug)
// -------------------------
void printBoard() {
    printf("\nCurrent Board:\n");
    for (int i = 0; i < 9; i++) {
        printf(" %c ", board[i]);
        if ((i + 1) % 3 == 0) printf("\n");
    }
    printf("\n");
}

// -------------------------
// Convert board to string
// -------------------------
void boardToString(char *str) {
    int idx = 0;
    for (int i = 0; i < 9; i++) {
        str[idx++] = board[i];
        if ((i + 1) % 3 == 0) str[idx++] = '\n';
        else str[idx++] = ' ';
    }
    str[idx] = '\0';
}

// -------------------------
// Check if someone has won
// -------------------------
int checkWin() {
    int wins[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8}, // rows
        {0,3,6}, {1,4,7}, {2,5,8}, // cols
        {0,4,8}, {2,4,6}            // diagonals
    };

    for (int i = 0; i < 8; i++) {
        if (board[wins[i][0]] != '-' &&
            board[wins[i][0]] == board[wins[i][1]] &&
            board[wins[i][1]] == board[wins[i][2]]) {
            return 1; // win found
        }
    }
    return 0;
}

// -------------------------
// Check if board is full (draw)
// -------------------------
int checkDraw() {
    for (int i = 0; i < 9; i++) {
        if (board[i] == '-') return 0;
    }
    return 1;
}

// -------------------------
// Main
// -------------------------
int main() {
    int server_fd, client1, client2;
    struct sockaddr_in address;
    char buffer[1024];
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    listen(server_fd, 2);
    printf("[SERVER] Waiting for 2 clients...\n");

    // Accept clients
    client1 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    printf("[SERVER] Client 1 connected\n");

    client2 = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    printf("[SERVER] Client 2 connected\n");

    printf("[SERVER] Game starting. Client1 = X, Client2 = O\n");
    printBoard();

    // Game loop
    while (1) {
        int currentClient = (turn == 1) ? client1 : client2;
        char mark = (turn == 1) ? 'X' : 'O';
        int validMove = 0;

        while (!validMove) {
            // Ask for move
            char msg[64];
            sprintf(msg, "Your turn (Player %d - %c). Enter position (1-9): ", turn, mark);
            send(currentClient, msg, strlen(msg), 0);

            // Get move
            memset(buffer, 0, sizeof(buffer));
            int valread = read(currentClient, buffer, sizeof(buffer) - 1);
            if (valread <= 0) {
                printf("[SERVER] Client disconnected\n");
                goto cleanup;
            }

            buffer[valread] = '\0';
            int move = atoi(buffer) - 1;

            // Validate
            if (move < 0 || move >= 9 || board[move] != '-') {
                char *err = "Invalid move. Try again.\n";
                send(currentClient, err, strlen(err), 0);
            } else {
                board[move] = mark;
                validMove = 1;
            }
        }

        printBoard();

        // Send board to both clients
        char boardStr[128];
        boardToString(boardStr);
        send(client1, boardStr, strlen(boardStr), 0);
        send(client2, boardStr, strlen(boardStr), 0);

        // Check win
        if (checkWin()) {
            char msg[64];
            sprintf(msg, "Player %d (%c) wins!\n", turn, mark);
            send(client1, msg, strlen(msg), 0);
            send(client2, msg, strlen(msg), 0);
            break;
        }

        // Check draw
        if (checkDraw()) {
            char *msg = "It's a draw!\n";
            send(client1, msg, strlen(msg), 0);
            send(client2, msg, strlen(msg), 0);
            break;
        }

        // Switch turn
        turn = (turn == 1) ? 2 : 1;
    }

cleanup:
    close(client1);
    close(client2);
    close(server_fd);
    return 0;
}

