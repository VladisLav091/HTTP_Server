#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 10000
#define BUF_SIZE 1024

double op1 = 0.0, op2 = 0.0;

void error_handling(const char *message) {
    perror(message);
    exit(1);
}

void send_response(int client_sock, const char *header, const char *body) {
    char response[BUF_SIZE];
    sprintf(response, "%s\r\nContent-Length: %lu\r\n\r\n%s", header, strlen(body), body);
    write(client_sock, response, strlen(response));
}

void handle_request(int client_sock) {
    char buf[BUF_SIZE];
    int read_len = read(client_sock, buf, BUF_SIZE - 1);
    buf[read_len] = '\0';

    char method[16], resource[64];
    sscanf(buf, "%s %s", method, resource);
    if (strcmp(method, "PUT") == 0 && (strcmp(resource, "/op1") == 0 || strcmp(resource, "/op2") == 0)) {
        char *data = strstr(buf, "\r\n\r\n") + 4;  
        double value = atof(data);

        if (strcmp(resource, "/op1") == 0) {
            op1 = value;
        } else {
            op2 = value;
        }

        send_response(client_sock, "HTTP/1.1 200 OK", "");
    }
    else if (strcmp(method, "GET") == 0 && (strcmp(resource, "/op1") == 0 || strcmp(resource, "/op2") == 0)) {
        char body[32];
        if (strcmp(resource, "/op1") == 0) {
            sprintf(body, "%f", op1);
        } else {
            sprintf(body, "%f", op2);
        }

        send_response(client_sock, "HTTP/1.1 200 OK", body);
    }
    else if (strcmp(method, "POST") == 0 && strcmp(resource, "/calculate") == 0) {
        char *operation = strstr(buf, "Operation:");
        char op = '+';
        if (operation) {
            sscanf(operation, "Operation: %c", &op);
        }

        double result;
        switch (op) {
            case '+': result = op1 + op2; break;
            case '-': result = op1 - op2; break;
            case '*': result = op1 * op2; break;
            case '/': result = (op2 != 0) ? (op1 / op2) : 0.0; break;
            default: result = op1 + op2; break; 
        }

        char body[64];
        sprintf(body, "%f", result);

        send_response(client_sock, "HTTP/1.1 200 OK", body);
    }
    else if ((strcmp(resource, "/op1") == 0 || strcmp(resource, "/op2") == 0 || strcmp(resource, "/calculate") == 0)) {
        send_response(client_sock, "HTTP/1.1 405 Method Not Allowed", "");
    }
    else {
        send_response(client_sock, "HTTP/1.1 404 Not Found", "");
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;

    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1)
        error_handling("socket() error");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error_handling("bind() error");

    if (listen(server_sock, 5) == -1)
        error_handling("listen() error");

    printf("Server started on port %d\n", PORT);

    while (1) {
        client_addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
        if (client_sock == -1)
            error_handling("accept() error");

        handle_request(client_sock);

        close(client_sock);
    }

    close(server_sock);
    return 0;
}
