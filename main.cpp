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

int strfind(const char *p, const char c, size_t i = 0);
void change(const unsigned char *host, unsigned char *h);


int main() {
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

    const char *h = "yahoo.com.tr";
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

    in_addr ip;
    uint32_t p = 0;
    for (int j = recv_size-4; j < recv_size; ++j) {
        ip.s_addr <<= 8;
        ip.s_addr |= (uint8_t)buf[j];
    }
    ip.s_addr = htonl(ip.s_addr);

    char *ip_addr = inet_ntoa(ip);
    std::cout << ip_addr << std::endl;

    free(host);
    close(sockd);
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