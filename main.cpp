#include <iostream>
#include "dns_headers.h"
#include <sys/socket.h> // socket
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // inet_ntoa, inet_aton
#include <unistd.h> // getpid()
#include <string.h>
#include <stdlib.h> // malloc
#define BUF_SIZE 65535
#include <stdio.h>
#include "queue.h"
#include <pthread.h>




int strfind(const char *p, const char c, size_t i = 0);
void change(const unsigned char *host, unsigned char *h);
char * resolve(char *url);
void * worker(void *);
void replace(char *l, char c, char t);
void print_usage();

int main(int argc, char **argv) {
    int opt, thread_count = 1;
    unsigned char flags = 0;
    char *url = NULL;
    char *filename = NULL;

    while((opt = getopt(argc, argv, "u:f:t:h")) != -1) {
        switch(opt) {
            case 'u':
                flags = 1;
                url = optarg;
                break;
            case 'h':
                print_usage();
                exit(0);
                break;
            case 'f':
                flags = 2;
                filename = optarg;
                break;
            case 't':
                sscanf(optarg, "%d", &thread_count);
                if(thread_count < 1) thread_count = 1;
                break;
            case ':':
                printf("option needs a value\n");
                break;
            default:
                exit(0);
                break;
        }
    }
    if( flags == 1 ) {
        char *ip = resolve(url);
        printf("IP: %s\n", ip);
    }
    else if(flags == 2) {
        printf("filename: %s\n", filename);
    }
    else {
        printf("Unknown flag..\n");
    }

    theader* handle;
    handle = tqueue.create();

    FILE *f = fopen(filename, "r");
    if(!f) {
        printf("File can not readed.\n");
        exit(0);
    }
    char *line = NULL;
    size_t len = 0;
    while( (getline(&line, &len, f) != -1)) {
        //printf("- %s - %lu", line, strlen(line));
        replace(line, '\n', '\0');;
        size_t s = strlen(line);
        char *url = (char *)malloc(s+1);
        memset(url, 0, s+1);
        strcpy(url, line);
        printf("%s\n", url);
        tqueue.push(handle, (void*)url);
    }

    char *p = (char *)malloc(sizeof(char));
    pthread_t *threads = (pthread_t *)malloc( sizeof(pthread_t)*thread_count );
    for(int i=0;i< thread_count; i++) {
        sprintf(p, "%d", i);
        pthread_create(&threads[i], NULL, worker, handle);
    }

    for(int i=0; i<thread_count;i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

int strfind(const char *p,const char c, size_t i) {
    int size = strlen(p);
    for( int j=i; j < size; j+=1) {
        if(p[j] == c) {
            return j;
        }
    }
    return -1;

}

void change(const unsigned char *host, unsigned char *out) {
    size_t host_size = strlen((char*)host);

    int last_pos = -1, curr_pos = 0, j = 0;
    curr_pos = strfind((char*)host, '.', last_pos);
    while(last_pos != host_size ) {
        out[j] = curr_pos-last_pos-1;
        j++;

        for(int i=last_pos+1; i < curr_pos; i++, j++) {
            out[j] = host[i];
        }

        last_pos = curr_pos;
        curr_pos = strfind((char*)host,'.',curr_pos+1);

        if(curr_pos == host_size) break;
        if(curr_pos == -1) { curr_pos = host_size; }
    }

}

void print_usage() {
    printf("usage: ./multidns -u example.com\n");
    printf("NOTE: You should write the url without http\n");
}

char * resolve(char *url) {
    int sockd;

    unsigned char buf[BUF_SIZE];

    struct sockaddr_in dest;

    struct dns_header dns2;
    query q;

    struct in_addr addr;
    sockd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53); // dns servers port
    inet_aton("8.8.8.8", &addr);
    dest.sin_addr.s_addr = addr.s_addr;

    memset(&dns2, 0, sizeof(struct dns_header));
    dns2.id = htons( getpid() );
    dns2.rd = 1;
    dns2.q_count = htons(1);

    const char *h = url;
    unsigned char *host = (unsigned char*)malloc(strlen(h)+2);
    memset(host, 0, strlen(h)+2);

    change( (unsigned char*)h, host );
    int host_size = strlen((char*)host);

    q.name = host;

    q.ques.qclass = htons(1);
    q.ques.qtype = htons(1); // dns que type A = 1,

    // create packet
    memset(buf, 0, BUF_SIZE);
    memcpy(&buf[0], &dns2, sizeof(struct dns_header));
    memcpy(&buf[sizeof(struct dns_header)], q.name, host_size+1);
    memcpy(&buf[sizeof(struct dns_header)+host_size+1 ], &q.ques, sizeof(struct question));

    size_t packet_size = sizeof(struct dns_header)+host_size+1+ sizeof(struct question);
    if ( sendto(sockd, (char*)&buf, packet_size, 0, (struct sockaddr*)&dest, sizeof(dest)) < 0 ) {
        std::cout << "sendto error" << std::endl;
    }
    memset(buf, 0, BUF_SIZE);
    int i = sizeof(dest), recv_size = 0;
    if( (recv_size = recvfrom (sockd,(char*)&buf , BUF_SIZE, 0 , (struct sockaddr*)&dest , (socklen_t*)&i )) < 0) {
        std::cout << "recv error" << std::endl;
    }
    // get ip from last 4 byte
    in_addr ip;
    uint32_t p = 0;
    for (int j = recv_size-4; j < recv_size; ++j) {
        ip.s_addr <<= 8;
        ip.s_addr |= (uint8_t)buf[j];
    }
    ip.s_addr = htonl(ip.s_addr);

    char *ip_addr = inet_ntoa(ip);

    free(host);
// todo: parse response
//    struct dns_header *res_header;
//    res_header = (struct dns_header *) &buf[0];
//    int ans_count = ntohs(res_header->ans_count);
//    printf("%d", ans_count);

    close(sockd);
    return ip_addr;
}

void * worker(void *arg) {
    theader *handle = (theader *)arg;
    char *url = NULL;
    while( handle != NULL ) {
        url = (char *)tqueue.pop(handle);

        if(url == NULL)
            break;

        printf("%s\n", resolve((char*)url) );
        free(url);
    }
}

void replace(char *l, char c, char t) {
    for(int i=0; i<strlen(l);i++) {
        if( l[i] == c) {
            l[i] = t;
        }
    }
}