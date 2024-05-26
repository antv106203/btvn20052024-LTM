#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void *client_thread(void *);

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed\n");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed\n");
        return 1;
    }
    printf("Waiting for connection...\n");

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed\n");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    close(listener);
    return 0;
}

void *client_thread(void *args)
{
    int client = *(int *)args;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);

    char get_post[5], arg[256]; // GET /?a=1&b=2&cmd=add
    sscanf(buf, "%s%s", get_post, arg);
    printf("GET/POST: %s\n", get_post);
    printf("Arguments: %s\n", arg);

    double a, b;
    int cmd = 0;

    char *p = strtok(arg, "?");
    while (p != NULL)
    {
        if (strncmp(p, "a=", 2) == 0)
            a = atof(p + 2);
        else if (strncmp(p, "b=", 2) == 0)
            b = atof(p + 2);
        else if (strncmp(p, "cmd=", 4) == 0)
        {
            if (strncmp(p + 4, "add", 3) == 0)
                cmd = 1;
            else if (strncmp(p + 4, "sub", 3) == 0)
                cmd = 2;
            else if (strncmp(p + 4, "mul", 3) == 0)
                cmd = 3;
            else if (strncmp(p + 4, "div", 3) == 0)
                cmd = 4;
        }
        p = strtok(NULL, "&");
    }

    if (cmd == 1)
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 {font-weight: bold; }</style></head><body><h1>%f + %f = %f</h1></body></html>", a, b, a + b);
    else if (cmd == 2)
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 {font-weight: bold; }</style></head><body><h1>%f - %f = %f</h1></body></html>", a, b, a - b);
    else if (cmd == 3)
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body {display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 { font-weight: bold; }</style></head><body><h1>%f x %f = %f</h1></body></html>", a, b, a * b);
    else if (cmd == 4)
    {
        if (b == 0)
            strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 {font-weight: bold; }</style></head><body><h1>Can not devide to 0</h1></body></html>");
        else
            sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 {font-weight: bold; }</style></head><body><h1>%f : %f = %f</h1></body></html>", a, b, a / b);
    }
    else
        strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><head><style>body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; } h1 {font-weight: bold; }</style></head><body><h1>Syntax Error</h1></body></html>");

    send(client, buf, strlen(buf), 0);
    close(client);
}