// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <thread>
#include <iostream>
#include <winsock2.h>
#include <winerror.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

static char is_init = false;
int fd[2];

int SocketPair(int type, int fd[2]) 
{
    int listener = -1;
    int connector = -1;
    int acceptor = -1;
    struct sockaddr_in listen_addr;
    struct sockaddr_in connect_addr;
    socklen_t size;
    int saved_errno = -1;

    if (!fd) {
        return -1;
    }

    listener = socket(AF_INET, type, 0);
    if (listener < 0)
        return -1;
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    listen_addr.sin_port = 0;	/* kernel chooses port.	 */
    if (bind(listener, (struct sockaddr *) &listen_addr, sizeof(listen_addr)) == -1)
        return -1;
    if (listen(listener, 10) == -1)
        return -1;
    connector = socket(AF_INET, type, 0);
    if (connector < 0)
        return -1;

    memset(&connect_addr, 0, sizeof(connect_addr));

    /* We want to find out the port number to connect to.  */
    size = sizeof(connect_addr);
    if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1) {
        return -1;
    }
    if (size != sizeof(connect_addr))
        return -1;
    if (connect(connector, (struct sockaddr *) &connect_addr,
        sizeof(connect_addr)) == -1)
        return -1;

    size = sizeof(listen_addr);
    acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
    if (acceptor < 0)
        return -1;
    if (size != sizeof(listen_addr))
        return -1;
    /* Now check we are talking to ourself by matching port and host on the
    two sockets.	 */
    if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
        return -1;
    if (size != sizeof(connect_addr)
        || listen_addr.sin_family != connect_addr.sin_family
        || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
        || listen_addr.sin_port != connect_addr.sin_port)
        return -1;
    closesocket(listener);
    fd[0] = connector;
    fd[1] = acceptor;
    int optval = 1;
    setsockopt(fd[0], IPPROTO_TCP, TCP_NODELAY,
        (char *)&optval, sizeof(optval));
    setsockopt(fd[1], IPPROTO_TCP, TCP_NODELAY,
        (char *)&optval, sizeof(optval));

    return 0;
}

static void thread_main(void)
{
    char buf[128];
    int ret;

    if (SocketPair(SOCK_STREAM, fd) < 0) {
        std::cout << "create socket pair failed.\n";
        return;
    }

    ret = send(fd[1], (char *)buf, 8, 0);
    if (ret <= 0) {
        std::cout << "Can't send the data,fd[0]:" << fd[0] << ",fd[1]:"
            << fd[1] << ",return:" << ret << ".\n";
        return;
    }
    std::cout << "Send the data,fd[0]:" << fd[0] << ",fd[1]:"
        << fd[1] << ",return:" << ret << ".\n";
    is_init = true;

    //while (1) {
    //    Sleep(500);
    //}
}

int main()
{
    int ret;
    DWORD index;
    WSADATA wsadata;
    WSAEVENT NewEvent;
    char buf[128];
    HANDLE hFile;

    WSAStartup(MAKEWORD(2, 2), &wsadata);
    memset(fd, 0, sizeof(fd));
#if 0
    NewEvent = WSACreateEvent();
    std::thread thread(thread_main);

    while (!is_init) {
        Sleep(100);
    }

    while (1) {
        std::cout << "Select fd:" << fd[0] << ".\n";
        WSAEventSelect(fd[0], NewEvent, FD_READ);
        index = WaitForMultipleObjects(1, &NewEvent, FALSE, WSA_INFINITE);
        switch (index) {
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            std::cout << "Wait objects failed.\n";
            break;
        default:
            std::cout << "1.It's OK.\n";
            break;
        }

        WSAEventSelect(fd[0], NewEvent, FD_READ);
        index = WaitForMultipleObjects(1, &NewEvent, FALSE, WSA_INFINITE);
        switch (index) {
        case WAIT_TIMEOUT:
        case WAIT_FAILED:
            std::cout << "Wait objects failed.\n";
            break;
        default:
            std::cout << "2.It's OK.\n";
            break;
        }
        Sleep(100);
    }

    std::cout << "Hello World!\n";
    thread.join();
#endif
    hFile = CreateFile(TEXT("two.txt"), // open Two.txt
        FILE_GENERIC_WRITE,         // open for writing
        FILE_SHARE_WRITE,          // allow multiple readers
        NULL,                     // no security
        OPEN_ALWAYS,              // open or create
        FILE_ATTRIBUTE_NORMAL,    // normal file
        NULL);                    // no attr. template
    if (hFile == INVALID_HANDLE_VALUE) {
        std::cout << "Can't open the file.\n";
        return -1;
    }
    if (LockFile(hFile, 0, 0, 100, 0) == FALSE) {
        std::cout << "Lock the file failed:" << GetLastError() << ".\n";
        return -1;
    }
    while (1) {
        Sleep(5000);
        std::cout << "Hello World!\n";
    }

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
