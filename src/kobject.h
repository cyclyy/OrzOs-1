#ifndef KOBJECT_H
#define KOBJECT_H 

#include "sysdef.h"

struct KObject;

struct Klass
{
    char name[MAX_NAME_LEN];
    struct KObject *(*construct)(struct Klass *klass, struct KObject *parent, const char *name, void *data);
    void (*destruct)(struct Klass *klass, struct KObject *object);
    void *data;
    struct Klass *next, *prev;
    //struct KObject *objects;
};

struct KObject
{
    char name[MAX_NAME_LEN];
    struct Klass *klass;
    struct KObject *parent;
    u64int ref;
    void *data;

    s64int (*process)(struct KObject *object, void *data);
    void (*dump)(struct KObject *);
};

struct KOD
{
    struct KObject *object;
    void *data;
};

extern struct KObject *RootObject;

s64int koRegisterKlass(struct Klass *klass);
s64int koUnregisterKlass(struct Klass *klass);
struct Klass *koGetKlass(const char *name);

s64int koCreateDirectory(const char *path);
s64int koRemoveDirectory(const char *path);
s64int koReadDirectory(struct KOD *kod, char *buf, u64int bufSize);

s64int koCreateObject(const char *path, struct Klass *klass, void *data);
s64int koRemoveObject(const char *path);

struct KOD *koOpen(const char *path);
s64int koClose(struct KOD *kod);
s64int koProcess(struct KOD *kod, void *data);
void koDump(struct KOD *kod);

void initObjectTree();

#endif

