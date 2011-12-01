#include "uidef.h"
#include "app.h"
#include "buffer.h"
#include <stdlib.h>
#include <os/list.h>
#include <os/syscall.h>

LIST_HEAD(appList);

static void compactAppBufferList(struct App *app)
{
    long currentTicks;
    struct Buffer *buffer, *tmp;
    if (listEmpty(&app->bufferList))
        return;
    currentTicks = OzGetTicks();
    listForEachEntrySafe(buffer, tmp, &app->bufferList, link) {
        if (currentTicks > 2000 + buffer->tstamp) {
            listDel(&buffer->link);
            destroyBuffer(buffer);
        } else 
            break;
    }
}

struct App *createApp(int pid)
{
    struct App *app;
    app = (struct App*)malloc(sizeof(struct App));
    app->pid = pid;
    app->nMissingEvents = 0;
    INIT_LIST_HEAD(&app->windowList);
    INIT_LIST_HEAD(&app->bufferList);
    listAdd(&app->link, &appList);
    return app;
}

void destroyApp(struct App *app)
{
    listDel(&app->link);

    free(app);
}

struct App *getApp(int pid)
{
    struct App *app;
    listForEachEntry(app, &appList, link) {
        if (app->pid == pid)
            return app;
    }
    return 0;
}

void notifyApp(struct App *app, void *data, int size)
{
    struct Buffer *buffer;
    /*
    UIDBG("notifyApp pid:%d,misses:%d\n", app->pid,app->nMissingEvents);
    */
    compactAppBufferList(app);
    if (app->nMissingEvents) {
        OzPost(app->pid, data, size);
        --app->nMissingEvents;
    } else {
        buffer = createBuffer(data, size);
        listAddTail(&buffer->link, &app->bufferList);
    }
}

void pollAppEvent(struct App *app)
{
    struct Buffer *buffer;
    compactAppBufferList(app);
    if (listEmpty(&app->bufferList)) {
        ++app->nMissingEvents;
    } else {
        buffer = listFirstEntry(&app->bufferList, struct Buffer, link);
        OzPost(app->pid, buffer->data, buffer->size);
        listDel(&buffer->link);
        destroyBuffer(buffer);
    }
}

