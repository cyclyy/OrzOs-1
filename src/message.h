#ifndef MESSAGE_H
#define MESSAGE_H

#include "sysdef.h"
#include "semaphore.h"
#include "waitqueue.h"

struct Client;
struct Server;
struct Message;
struct MessageQueue;

#define CLIENT_STATUS_READY             0
#define CLIENT_STATUS_SEND_BLOCKED      1
#define CLIENT_STATUS_REPLY_BLOCKED     2

#define SERVER_STATUS_READY             0
#define SERVER_STATUS_RECEIVE_BLOCKED   1

struct Server
{
    s64int port;
    struct Task *ownerTask;
    struct Client *clients;
    struct MessageQueue *receiveMQ, *replyMQ;
    struct WaitQueue *receiveWQ, *sendWQ;
    struct Server *next, *prev;
};

struct Client
{
    struct Task *ownerTask;
    struct Server *server;
    struct Client *next, *prev;
};

#define MESSAGE_TYPE_SEND   1
#define MESSAGE_TYPE_POST   2

struct Message
{
    s64int type;
    struct Task *task;
    struct Client *client;
    struct Server *server;
    char *src, *dest;
    u64int srcSize, destSize;
    struct Message *next, *prev;
};

struct MessageQueue
{
    u64int size;
    struct Message *head, *tail;
};

struct IPCManager
{
    struct Server *servers;
};

struct Server *kCreateServer(u64int port);

s64int kDestroyServer(struct Server *server);

struct Client *kConnect(u64int port);

s64int kDisconnect(struct Client *client);

s64int kPost(struct Client *client, char *src, u64int srcSize);

s64int kSend(struct Client *client, char *src, u64int srcSize, char *dest, u64int destSize);

struct Message *kReceive(struct Server *server, char *buf, u64int bufSize);

s64int kReply(struct Message *message, char *buf, u64int bufSize);

void initIPC();

#endif /* MESSAGE_H */
