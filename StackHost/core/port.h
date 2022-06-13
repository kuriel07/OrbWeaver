#ifndef PORT__H
#if defined(WIN32) || defined(WIN64)
#include <winsock2.h>
#define net_recv(s, buf, len, flags)	recv(s, buf, len, flags)
#define net_send(s, buf, len, flags)	send(s, buf, len, flags)
#define net_close(x)	closesocket(x)
#define SHUT_RECV		SD_RECEIVE
#define SHUT_SEND		SD_SEND 
#define SHUT_ALL        SD_BOTH
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#ifdef __cplusplus
#include <cstdint>
#define SOCKET		intptr_t
#else
#define SOCKET		int
#endif
#define net_close(x)	close(x)
#define SHUT_RECV		SHUT_RD
#define SHUT_SEND		SHUT_WR 
#define SHUT_ALL        SHUT_RDWR
#define net_recv(s, buf, len, flags)	recv(s, buf, len, flags)
#define net_send(s, buf, len, flags)	send(s, buf, len, flags)
#endif
#define net_socket(af, type, protocol)	socket(af, type, protocol)
#define net_connect(s, addr, len)		connect(s, addr, len);
#define net_bind(s, name, namelen)		bind(s, name, namelen)
#define net_listen(s, backlog)					listen(s, backlog)
#define net_accept(s, addr, addrlen)	accept(s, addr, addrlen)
#define net_shutdown(s, how)			shutdown(s, how)

#define PORT__H
#endif
