// udp_test.cpp: 定义应用程序的入口点。
//

#include <errno.h>
#include <string.h>
#ifdef _WIN32
#include <ws2tcpip.h>
#include <Windows.h>
#include <winsock2.h>
#include "getopt.h"
#else
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <getopt.h>
#endif
#include "udp_test.h"

#define udp_log_err printf
#define UDP_SOCKET_NUM 10

static void udp_test_print_usage(int exit_code)
{
    printf("Usage: udp test example [options]\n");
    printf(" -h --help                  help information.\n"
        " -v --version                  show program version.\n\n"
        " -c --connect xx.xx.xx.xx      udp client connect to xx.xx.xx.xx.\n"
        " -s --server xx                server mode and bind port.\n"
        " -p --port                     udp connect port.\n"
    );
    exit(exit_code);
}

static int udp_socket(unsigned port)
{
    int opt;
    int fd;
    struct sockaddr_in addr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        udp_log_err("can't create the socket.\n");
        return -1;
    }
    opt = 1;
#ifdef _WIN32
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, (int)sizeof(opt)) < 0) {
        udp_log_err("can't set the reuseaddr opt.\n");
        closesocket(fd);
        return -1;
    }
#else
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (void *)&opt, (socklen_t)sizeof(opt)) < 0) {
        udp_log_err("can't set the reuseport opt.\n");
        close(fd);
        return -1;
    }
#endif
    if (port) {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        if (bind(fd, (struct sockaddr *) & addr, sizeof(addr)) < 0) {
            udp_log_err("can't bind the udp socket to port(%d).\n", port);
#ifdef _WIN32
            closesocket(fd);
#else
            close(fd);
#endif
        }
    }
    udp_log_err("Create the udp socket(%d) success,port:%d.\n", fd, port);
    return fd;
}

int main(int argc, char *argv[])
{
    int i, j;
    int c;
    int cnt;
    int ret;
    int len;
	int max;
    int client_mode;
    int bind_port;
    char buf[128];
    struct timeval tv;
    fd_set set;
    int fd[UDP_SOCKET_NUM];
    int connect_port;
    int option_index = 0;
    struct sockaddr_in server, client;
    static struct option long_options[] = {
        {"help", 0, NULL, 'h'},
        {"version", 0, NULL, 'v'},
        {"connect", 1, NULL, 'c'},
        {"server", 1, NULL, 's'},
        {"port", 1, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    (void) WSAStartup(wVersionRequested, &wsaData);
#endif
    memset(&server, 0, sizeof(server));
    bind_port = 0;
    client_mode = 0;
    connect_port = UDP_TEST_PORT;
    bind_port = UDP_TEST_PORT;
    while (1) {
        c = getopt_long(argc, argv, "hvc:s:p:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
        case 'h':
            udp_test_print_usage(-1);
            break;
        case 'v':
            printf("UDP TEST VERSION:%s.\n", UDP_TEST_VERSION);
            return -1;
        case 'c':
            ret = inet_pton(AF_INET, optarg, &server.sin_addr);
            if (!ret) {
                printf("The connect address(%s) is error.\n", optarg);
                return -1;
            }
            server.sin_family = AF_INET;
            server.sin_port = htons(UDP_TEST_PORT);
            client_mode = 1;
            break;
        case 's':
            bind_port = atoi(optarg);
            if (bind_port <= 0 || bind_port > 65535) {
                printf("The bind port is error.port must in 1-65535.\n");
                return -1;
            }
            break;
        case 'p':
            connect_port = atoi(optarg);
            if (connect_port <= 0 || connect_port > 65535) {
                printf("The connect port is error.port must in 1-65535.\n");
                return -1;
            }
            server.sin_port = htons(connect_port);
            break;
        default:
            break;
        }
    }

    memset(fd, -1, sizeof(fd));
    i = 0;
    fd[i] = udp_socket(client_mode ? 0 : bind_port);
    if (fd[i] < 0) {
        return -1;
    }
    FD_ZERO(&set);
    FD_SET(fd[i], &set);
    if (client_mode) {
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        while (1) {
            //ret = select(1, &set, NULL, NULL, &tv);
            //if (ret < 0) {
            //    udp_log_err("Select error.\n");
            //    continue;
            //}

            ret = sendto(fd[i], "hello world.", strlen("hello world.") + 1, 0,
                (struct sockaddr *)&server, sizeof(server));
            udp_log_err("Client send data:%d to server success.\n", ret);
#ifdef _WIN32
            Sleep(5000);
#else
            sleep(5);
#endif
        }
    } else {
        while (1) {
            cnt = 0;
			max = 0;
			FD_ZERO(&set);
            for (i = 0; i < UDP_SOCKET_NUM; i++) {
                if (fd[i] > 0) {
					if (fd[i] > max) {
						max = fd[i];
					}
                    FD_SET(fd[i], &set);
                    cnt++;
                }
            }
            ret = select(max + 1, &set, NULL, NULL, NULL);
            if (ret < 0) {
                udp_log_err("Select error.\n");
                continue;
            }
			udp_log_err("select successs.\n");
            for (i = 0; i < UDP_SOCKET_NUM; i++) {
                if (fd[i] > 0) {
                    if (FD_ISSET(fd[i], &set)) {
                        len = sizeof(client);
                        memset(buf, 0, sizeof(buf));
                        ret = recvfrom(fd[i], buf, sizeof(buf), 0, &client, &len);
                        udp_log_err("fd(%d) receive the data:%s from %s.\n", 
                            fd[i], buf, inet_ntoa(*(struct in_addr *)&client.sin_addr));
                        if (i == 0) {
                            udp_log_err("##fd(%d) receive the data:%s from %s.\n", 
                                fd[i], buf, inet_ntoa(*(struct in_addr *)&client.sin_addr));
                            for (j = 1; j < UDP_SOCKET_NUM; j++) {
                                if (fd[j] > 0) {
                                    continue;
                                }
                                fd[j] = udp_socket(bind_port);
                                if (connect(fd[j], (struct sockaddr *) & client, len) < 0) {
                                    udp_log_err("fd(%d) connect to %s failed.\n", 
                                        fd[j], inet_ntoa(*(struct in_addr *)&client.sin_addr));
                                    break;
                                }
                                udp_log_err("connnect success .\n");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

	return 0;
}
