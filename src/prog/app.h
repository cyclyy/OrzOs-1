#ifndef APP_H
#define APP_H

#include <os/list.h>

struct App
{
    int pid;
    int nMissingEvents;
    struct ListHead windowList;
    struct ListHead bufferList;
    struct ListHead link;
};

extern struct ListHead appList;

struct App *createApp(int pid);

void destroyApp(struct App *app);

struct App *getApp(int pid);

void notifyApp(struct App *app, void *data, int size);

void pollAppEvent(struct App *app);

#endif /* APP_H */
