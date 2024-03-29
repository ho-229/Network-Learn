﻿#ifndef DEFINE_H
#define DEFINE_H

#define ANY_HOST "0.0.0.0"
#define LISTENQ 1024

#ifdef _WIN32
typedef unsigned int Socket;    // Socket handle
typedef void* File;             // File handle
# define ssize_t SSIZE_T
# define OS_WINDOWS
#else   // *nix
typedef int Socket;             // Socket descriptor
typedef int File;               // File descriptor

# define INVALID_SOCKET -1
# ifdef __linux
#  define OS_LINUX
# else
#  define OS_UNIX
# endif
#endif

/**********************
 *  User definitions  *
 **********************/
#define SOCKET_BUF_SIZE 4096
#define SOCKET_INFO_ENABLE 0

#define EPOLL_WAIT_TIMEOUT 500
#define EPOLL_MAX_EVENTS 256

#define TCP_CORK_ENABLE 0       // Only for Linux
/**********************
 *  User definitions  *
 **********************/

#endif // DEFINE_H
