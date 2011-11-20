#ifndef APP_H
#define APP_H

#include <os/list.h>

struct App
{
    int pid;
    int needEvent;
    struct ListHead windowList;
    struct ListHead link;
};

extern struct ListHead appList;

struct App *createApp(int pid);

void destroyApp(struct App *app);

struct App *getApp(int pid);


#endif /* APP_H */
