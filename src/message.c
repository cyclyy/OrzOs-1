#include "message.h"
#include "task.h"
#include "util.h"
#include "schedule.h"
#include "kmm.h"
#include "vmm.h"

struct IPCManager *ipcManager = 0;

struct MessageQueue *mqCreate()
{
    struct MessageQueue *mq;

    mq = (struct MessageQueue*)kMalloc(sizeof(struct MessageQueue));
    memset(mq,0,sizeof(struct MessageQueue));

    return mq;
}

void mqAppend(struct MessageQueue *mq,struct Message *msg)
{
    msg->next = 0;
    msg->prev = mq->tail;
    if (mq->tail) {
        mq->tail->next = msg;
    }
    mq->tail = msg;
    if (!mq->head) {
        mq->head = msg;
    }
    ++mq->size;
}

struct Message *mqTakeFirst(struct MessageQueue *mq)
{
    struct Message *msg;

    if (!mq->size) {
        return 0;
    }
    mq->size--;
    msg = mq->head;
    if (msg->next) {
        msg->next->prev = 0;
    }
    mq->head = msg->next;
    if (mq->tail == msg) {
        mq->tail = 0;
    }
    msg->next = msg->prev = 0;
    return msg;
}

void mqRemove(struct MessageQueue *mq, struct Message *msg)
{
    --mq->size;
    if (msg->prev) {
        msg->prev->next = msg->next;
    }
    if (msg->next) {
        msg->next->prev = msg->prev;
    }
    if (mq->head == msg) {
        mq->head = msg->next;
    }
    if (mq->tail == msg) {
        mq->tail = msg->prev;
    }

}

struct Server *findServer(u64int port)
{
    struct Server *server;

    if (!ipcManager) {
        return 0;
    }

    server = ipcManager->servers;
    while (server && (server->port != port)) {
        server = server->next;
    }
    return server;
};

struct Message *findReplyMessage(struct Client *client)
{
    struct MessageQueue *mq;
    struct Message *msg;

    mq = client->server->replyMQ;
    msg = mq->head;
    while (msg && (msg->client != client)) {
        msg = msg->next;
    }

    return msg;
}

void initIPC()
{
    ipcManager = (struct IPCManager *)kMalloc(sizeof(struct IPCManager));
    memset(ipcManager, 0, sizeof(ipcManager));
}

struct Server *kCreateServer(u64int port)
{
    struct Server *server;

    server = findServer(port);
    if (server) {
        return 0;
    }

    server = (struct Server*)kMalloc(sizeof(struct Server));
    memset(server,0,sizeof(struct Server));
    server->port = port;
    server->ownerTask = currentTask;
    server->next = ipcManager->servers;
    server->receiveMQ = mqCreate();
    server->replyMQ = mqCreate();
    server->receiveWQ = wqCreate();
    server->sendWQ = wqCreate();
    if (ipcManager->servers) {
        ipcManager->servers->prev = server;
    }
    ipcManager->servers = server;
    return server;
}

s64int kDestroyServer(struct Server *server)
{
    if (server->prev) {
        server->prev->next = server->next;
    }
    if (server->next) {
        server->next->prev = server->prev;
    }
    if (ipcManager->servers == server) {
        ipcManager->servers = server->next;
    }
    kFree(server);
    return 0;
}

struct Client *kConnect(u64int port)
{
    struct Client *client;
    struct Server *server;

    server = findServer(port);
    if (!server) {
        return 0;
    }
    client = (struct Client *)kMalloc(sizeof(struct Client));
    memset(client,0,sizeof(struct Client));
    client->server = server;
    client->ownerTask = currentTask;
    client->next = server->clients;
    if (server->clients) {
        server->clients->prev = client;
    }
    server->clients = client;

    return client;
}

s64int kDisconnect(struct Client *client)
{
    if (client->next) {
        client->next->prev = client->prev;
    }
    if (client->prev) {
        client->prev->next = client->next;
    }
    if (client->server->clients == client) {
        client->server->clients = client->next;
    }

    kFree(client);
    return 0;
}

s64int kSend(struct Client *client, char *src, u64int srcSize, char *dest, u64int destSize)
{
    struct Message *msg;

    msg = (struct Message *)kMalloc(sizeof(struct Message));
    memset(msg,0,sizeof(struct Message));
    msg->task = currentTask;
    msg->client = client;
    msg->server = client->server;
    msg->src = src;
    msg->dest = dest;
    msg->srcSize = srcSize;
    msg->destSize = destSize;
    mqAppend(client->server->receiveMQ,msg);
    wakeUpOne(msg->server->receiveWQ);
    sleepOn(msg->server->sendWQ);
    return 0;
}

struct Client *kReceive(struct Server *server, char *buf, u64int bufSize)
{
    struct Client *client;
    struct Message *msg;
    struct Task *t;

    while (!server->receiveMQ->size)
        sleepOn(server->receiveWQ);

    msg = mqTakeFirst(server->receiveMQ);
    mqAppend(server->replyMQ, msg);
    client = msg->client;

    t = msg->task;
    vmemcpy(currentTask->vm, buf, t->vm, msg->src, MIN(bufSize,msg->srcSize));

    return client;
}

s64int kReply(struct Client *client, char *buf, u64int bufSize)
{
    struct Message *msg;
    struct Task *t;

    msg = findReplyMessage(client);
    if (!msg) {
        return -1;
    }
    mqRemove(client->server->replyMQ, msg);
    wakeUp(client->server->sendWQ, msg->task);

    t = msg->task;
    vmemcpy(t->vm, msg->dest, currentTask->vm, buf, MIN(bufSize,msg->destSize));

    return 0;
}

