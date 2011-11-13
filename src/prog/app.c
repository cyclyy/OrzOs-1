#include "app.h"
#include <stdlib.h>

LIST_HEAD(appList);

struct App *createApp(int pid)
{
    struct App *app;
    app = (struct App*)malloc(sizeof(struct App));
    app->pid = pid;
    app->needEvent = 0;
    INIT_LIST_HEAD(&app->windowList);
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
