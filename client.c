#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in server;
    char msg[100];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);

    connect(sock, (struct sockaddr *)&server, sizeof(server));

    while (1) {
        printf("Enter message (type exit to disconnect): ");
        fgets(msg, sizeof(msg), stdin);

        if (strncmp(msg, "exit", 4) == 0)
            break;

        send(sock, msg, strlen(msg), 0);
    }

    close(sock);
    return 0;
}
