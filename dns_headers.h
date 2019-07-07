#ifndef UNTITLED_DNS_HEADERS_H
#define UNTITLED_DNS_HEADERS_H

struct dns_header {
    unsigned short id; // identity

    unsigned char rd: 1;
    unsigned char tc: 1;
    unsigned char aa: 1;
    unsigned char opcode: 4;
    unsigned char qr: 1;

    unsigned char rcode: 4;
    unsigned char cd: 1;
    unsigned char ad: 1;
    unsigned char z: 1;
    unsigned char ra: 1;

    unsigned short q_count;
    unsigned short ans_count;
    unsigned short auth_count;
    unsigned short add_count;

};

struct question {
    unsigned short qtype;
    unsigned short qclass;
};

struct r_data {
    unsigned short type;
    unsigned short _class;
    unsigned int ttl;
    unsigned short data_len;
};

struct res_record {
        unsigned char *name;
        struct r_data *resource;
        unsigned char *rdata;
};

typedef struct {
    unsigned char *name;
    struct question ques;
} query;

#endif //UNTITLED_DNS_HEADERS_H
