#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

/* ================= QUEUE IMPLEMENTATION ================= */

typedef struct node {
    int client_fd;
    struct node *next;
} node_t;

node_t *front = NULL, *rear = NULL;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

/* enqueue client socket */
void enqueue(int client_fd) {
    node_t *new_node = malloc(sizeof(node_t));
    new_node->client_fd = client_fd;
    new_node->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    if (rear == NULL) {
        front = rear = new_node;
    } else {
        rear->next = new_node;
        rear = new_node;
    }
    pthread_cond_signal(&queue_cond);
    pthread_mutex_unlock(&queue_mutex);
}

/* dequeue client socket */
int dequeue() {
    pthread_mutex_lock(&queue_mutex);
    while (front == NULL) {
        pthread_cond_wait(&queue_cond, &queue_mutex);
    }

    node_t *temp = front;
    int client_fd = temp->client_fd;
    front = front->next;
    if (front == NULL)
        rear = NULL;

    free(temp);
    pthread_mutex_unlock(&queue_mutex);
    return client_fd;
}

/* ================= WORKER THREAD ================= */

void *worker_thread(void *arg) {
    (void)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        int client_fd = dequeue();
        printf("Thread %lu handling client %d\n",
               pthread_self(), client_fd);

        while (1) {
            int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) {
                printf("Client %d disconnected\n", client_fd);
                close(client_fd);
                break;
            }
            buffer[bytes] = '\0';
            printf("Client %d says: %s\n", client_fd, buffer);
        }
    }
    return NULL;
}

/* ================= MAIN ================= */

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    pthread_t threads[MAX_CLIENTS];

    /* Create thread pool */
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, MAX_CLIENTS);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        printf("Client connected: %d\n", client_fd);
        enqueue(client_fd);
    }

    close(server_fd);
    return 0;
}
