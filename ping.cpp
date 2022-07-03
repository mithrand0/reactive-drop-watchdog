#include "ping.h"

#ifndef SD_SEND
#define SD_SEND 1
#endif

// Many thanks to:
// 
// Forlix SrcDS Manager
// http://forlix.org/, df@forlix.org
//
// Copyright (c) 2010-2013 Dominik Friedrichs

#define PING_QUERY      "\xFF\xFF\xFF\xFFTSource Engine Query\0"
#define PING_RESPONSE   "\xFF\xFF\xFF\xFFI"


int Query(const char* str_host, const char* str_port, unsigned int timeout)
{
    struct sockaddr_in sock_addr;
    struct hostent* hostp = NULL;

    memset(&sock_addr, 0, sizeof(sock_addr));

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(atoi(str_port));

    // resolve and convert given hostname/ip address
    if (!(hostp = gethostbyname(str_host)))
        return(-1);
    else
        memcpy(&sock_addr.sin_addr, hostp->h_addr_list[0], hostp->h_length);

    SOCKET a2aping = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (a2aping == INVALID_SOCKET)
        return(-1);

    unsigned long FIONBIOarg = 1;

    if (ioctlsocket(a2aping, FIONBIO, &FIONBIOarg) == SOCKET_ERROR
        || connect(a2aping, (PSOCKADDR)&sock_addr, sizeof(SOCKADDR)) == SOCKET_ERROR
        || send(a2aping, PING_QUERY, sizeof(PING_QUERY) - 1, 0) == SOCKET_ERROR
        || shutdown(a2aping, SD_SEND) == SOCKET_ERROR)
    {
        closesocket(a2aping);
        return(-1);
    }

    fd_set fd;
    timeval tv;

    fd.fd_count = 1;
    fd.fd_array[0] = a2aping;
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;

    if (!select(0, &fd, NULL, NULL, &tv))
        // receive timeout
    {
        closesocket(a2aping);
        return(0);
    }

    char recvbuf[32];
    int r = recv(a2aping, recvbuf, sizeof(recvbuf), 0);
    closesocket(a2aping);

    if (r < sizeof(PING_RESPONSE) - 1
        || memcmp(recvbuf, PING_RESPONSE, sizeof(PING_RESPONSE) - 1))
        // empty or incorrect response
        return(0);

    return(1);
}
