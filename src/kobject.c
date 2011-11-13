#include "kobject.h"
#include "util.h"
#include "kmm.h"

#define KDIR_OP_FINDDIR         1
#define KDIR_OP_READDIR         2
#define KDIR_OP_ADDOBJECT       4
#define KDIR_OP_REMOVEOBJECT    8 

struct KDirectoryObjectData
{
    u64int count;
    struct KObjectListNode *children;
};

struct KDirectoryProcessData
{
    u64int op;
    union {
        struct {
            u64int startIndex;
            char *buf;
            u64int bufSize;
        } readDir;
        struct {
            const char *name;
            struct KObject *object;
        } findDir;
        struct {
            struct KObject *object;
        } addObject;
        struct {
            const char *name;
        } removeObject;
    } arg;
};

struct KObject *RootObject = 0;
struct Klass *klassList = 0;

struct Klass *directoryKlass = 0;
struct KObject *constructKDirectory(struct Klass *self, struct KObject *parent, const char *name, void *initData);
void destructKDirectory(struct Klass *self, struct KObject *object);
void dumpKDirectory(struct KObject *object);

struct KObjectListNode
{
    struct KObject *object;
    struct KObjectListNode *next, *prev;
};

s64int koRegisterKlass(struct Klass *klass)
{
    if (!klass)
        return -1;
    if (klassList)
        klassList->prev = klass;
    klass->next = klassList;
    klassList = klass;
    return 0;
}

s64int koUnregisterKlass(struct Klass *klass)
{
    struct Klass *p = klassList;

    while (p && (p != klass))
        p = p->next;

    if (p == klass) {
        if (p->prev)
            p->prev->next = p->next;
        if (p->next)
            p->next->prev = p->prev;
        if (p==klassList)
            klassList = p->next;
    }
    return 0;
}

s64int koCreateDirectory(const char *path)
{
    koCreateObject(path, directoryKlass, 0);
    return 0;
}

s64int koRemoveDirectory(const char *path)
{
    koRemoveObject(path);
    return 0;
}

s64int koReadDirectory(struct KOD *kod, char *buf, u64int bufSize)
{
    u64int ret, startIndex;
    struct KDirectoryProcessData *pdata;

    if (kod->object->klass != directoryKlass) {
        DBG("Not a directory");
        return -1;
    } else {
        startIndex = (u64int)(kod->data);
        pdata = (struct KDirectoryProcessData *)kMalloc(sizeof(struct KDirectoryProcessData));
        memset(pdata, 0, sizeof(struct KDirectoryProcessData));
        pdata->op = KDIR_OP_READDIR;
        pdata->arg.readDir.startIndex = startIndex;
        pdata->arg.readDir.buf = buf;
        pdata->arg.readDir.bufSize = bufSize;
        ret = koProcess(kod, pdata);
        if (ret > 0) {
            startIndex += ret;
            kod->data = (void*)startIndex;
        }
        kFree(pdata);
        return ret;
    }
}

s64int koCreateObject(const char *path, struct Klass *klass, void *data)
{
    char *dirName, *baseName;
    struct KObject *object, *dobject;
    struct KOD *kod;
    u64int len;
    s64int ret;

    len = strlen(path) + 1;
    dirName = (char*)kMalloc(len);
    dirname(dirName, path);
    baseName = (char*)kMalloc(len);
    basename(baseName, path);

    kod = koOpen(dirName);
    if (!kod) {
        kFree(dirName);
        kFree(baseName);
        return -1;
    }
    dobject = kod->object;
    if (dobject->klass != directoryKlass)
        ret = -1;
    else {
        object = klass->construct(klass, dobject, baseName, data);
        struct KDirectoryProcessData *pdata;
        pdata= (struct KDirectoryProcessData *)kMalloc(sizeof(struct KDirectoryProcessData));
        memset(pdata,0,sizeof(struct KDirectoryProcessData));
        pdata->op = KDIR_OP_ADDOBJECT;
        pdata->arg.addObject.object = object;
        ret = koProcess(kod, pdata);
        kFree(pdata);
    }

    koClose(kod);
    kFree(dirName);
    kFree(baseName);
    return ret;
}

s64int koRemoveObject(const char *path)
{
    char *dirName, *baseName;
    struct KObject *object, *dobject;
    struct KOD *kod;
    u64int len;
    s64int ret;

    len = strlen(path) + 1;
    dirName = (char*)kMalloc(len);
    dirname(dirName, path);
    baseName = (char*)kMalloc(len);
    basename(baseName, path);

    kod = koOpen(dirName);
    dobject = kod->object;
    if (dobject->klass != directoryKlass)
        ret = -1;
    else {
        struct KDirectoryProcessData *pdata;
        pdata= (struct KDirectoryProcessData *)kMalloc(sizeof(struct KDirectoryProcessData));
        memset(pdata,0,sizeof(struct KDirectoryProcessData));
        pdata->op = KDIR_OP_REMOVEOBJECT;
        pdata->arg.removeObject.name = baseName;
        ret = koProcess(kod, pdata);
        kFree(pdata);
    }

    koClose(kod);
    kFree(dirName);
    kFree(baseName);
    return ret;
}

void initObjectTree()
{
    directoryKlass = (struct Klass*)kMalloc(sizeof(struct Klass));
    memset(directoryKlass, 0, sizeof(struct Klass));
    strcpy(directoryKlass->name, "Directory");
    directoryKlass->construct = constructKDirectory;
    directoryKlass->destruct = destructKDirectory;
    koRegisterKlass(directoryKlass);
    RootObject = directoryKlass->construct(directoryKlass, 0, "Root", 0);
    RootObject->ref = 1;
}

s64int processKDirectory(struct KObject *object, void *data)
{
    struct KDirectoryProcessData *pdata = (struct KDirectoryProcessData *)data;
    struct KDirectoryObjectData *odata = (struct KDirectoryObjectData *)(object->data);
    u64int i, startIndex, bufSize, len;
    const char *name;
    char *buf;
    struct KObjectListNode *node;
    struct KObject *cobject;

    if (!object) {
        DBG("nil argument");
        return -1;
    }

    if (object->klass != directoryKlass) {
        DBG("object not directory type");
        return -2;
    }

    if (pdata->op == KDIR_OP_READDIR) {
        startIndex = pdata->arg.readDir.startIndex;
        buf = pdata->arg.readDir.buf;
        bufSize = pdata->arg.readDir.bufSize;
        if (startIndex >= odata->count)
            return 0;
        else {
            node = odata->children;
            for (i = 0; i < startIndex; i++)
                node = node->next;
            i = startIndex;
            while (i < odata->count) {
                len = strlen(node->object->name);
                if (bufSize >= len + 1) {
                    strcpy(buf, node->object->name);
                    bufSize -= len + 1;
                    buf += len + 1;
                    i++;
                } else 
                    break;
                node = node->next;
            }
            return i - startIndex;
        }
    } else if (pdata->op == KDIR_OP_FINDDIR) {
        name = pdata->arg.findDir.name;
        pdata->arg.findDir.object = 0;
        node = odata->children;
        for (i = 0; i < odata->count; i++) {
            if (strcmp(name,node->object->name)==0) {
                pdata->arg.findDir.object = node->object;
                break;
            }
            node = node->next;
        }
        if (pdata->arg.findDir.object)
            return 0;
        else 
            return -1;
    } else if (pdata->op == KDIR_OP_ADDOBJECT) {
        cobject = pdata->arg.addObject.object;
        if (odata->children) {
            node = odata->children;
            for (i=0; i<odata->count; i++) {
                if (strcmp(node->object->name, cobject->name)==0) 
                    return -1;
                node = node->next;
            }
        }
        odata->count++;
        node = (struct KObjectListNode *)kMalloc(sizeof(struct KObjectListNode));
        node->object = cobject;
        node->prev = 0;
        node->next = odata->children;
        if (odata->children)
            odata->children->prev = node;
        odata->children = node;
        
        return 0;
    } else if (pdata->op == KDIR_OP_REMOVEOBJECT) {
        name = pdata->arg.removeObject.name;
        if (!odata->count)
            return -1;
        node = odata->children;
        for (i=0; i<odata->count; i++) {
            if (strcmp(node->object->name,name)==0) {
                if (node->prev)
                    node->prev->next = node->next;
                if (node->next)
                    node->next->prev = node->prev;
                if (odata->children == node)
                    odata->children = node->next;
                --odata->count;
                kFree(node);
                return 0;
            }
            node = node->next;
        }
    }
    return 0;
}
struct KObject *constructKDirectory(struct Klass *self, struct KObject *parent, const char *name, void *initData)
{
    struct KObject *object;
    struct KDirectoryObjectData *objData;

    object = (struct KObject*)kMalloc(sizeof(struct KObject));
    memset(object, 0, sizeof(struct KObject));
    strcpy(object->name, name);
    object->klass = self;
    object->parent = parent;
    object->process = processKDirectory;
    object->dump = dumpKDirectory;
    objData = (struct KDirectoryObjectData*)kMalloc(sizeof(struct KDirectoryObjectData));
    memset(objData, 0, sizeof(struct KDirectoryObjectData));
    object->data = objData;

    return object;
}

void destructKDirectory(struct Klass *self, struct KObject *object)
{
    kFree(object->data);
    kFree(object);
}

void dumpKDirectory(struct KObject *object)
{
    struct KDirectoryObjectData *odata;
    struct KObjectListNode *node;
    u64int i;

    if (object->klass != directoryKlass) {
        printk("Not valid directory\n");
    } else {
        odata = (struct KDirectoryObjectData *)(object->data);
        printk("Directory, Name:%s, Subdirs:%d\n", object->name, odata->count);
        node = odata->children;
        for (i=0; i<odata->count; i++) {
            if (node->object->dump);
                node->object->dump(node->object);
            node = node->next;
        }
    }

}

struct KObject *koLookup(const char *path)
{
    struct KObject *object;
    struct KOD *kod;
    u64int i, j, len;
    char *buf, *buf2, *p;
    struct KDirectoryProcessData *pdata;

    if (strcmp(path,"/")==0)
        return RootObject;

    object = RootObject;
    len = strlen(path);
    if (len == 1)
        return 0;
    buf = (char*)kMalloc(len+1);
    buf2 = (char*)kMalloc(len+1);
    pdata= (struct KDirectoryProcessData *)kMalloc(sizeof(struct KDirectoryProcessData));
    memset(pdata,0,sizeof(struct KDirectoryProcessData));
    pdata->op = KDIR_OP_FINDDIR;
    pdata->arg.findDir.name = buf;
    i = 1;
    while (i<len) {
        strcpy(buf, path+i);
        p = strchr(buf, '/');
        if (p)
            j = p - buf;
        else 
            j = strlen(buf);
        buf[j] = 0;
        strcpy(buf2, path);
        buf2[i] = 0;
        kod = koOpen(buf2);
        if (!kod) {
            object = 0;
            break;
        }
        koProcess(kod, pdata);
        object = pdata->arg.findDir.object;
        koClose(kod);
        if (!object)
            break;
        i += j+1;
    }
    kFree(pdata);
    kFree(buf2);
    kFree(buf);
    return object;
}

struct KOD *koOpen(const char *path)
{
    struct KOD *kod;
    struct KObject *object;

    object = koLookup(path);
    if (object) {
        kod = (struct KOD *)kMalloc(sizeof(struct KOD));
        memset(kod,0,sizeof(struct KOD));
        kod->object = object;
        return kod;
    } else 
        return 0;

}

s64int koClose(struct KOD *kod)
{
    if (!kod)
        return 0;

    --kod->object->ref;
    kFree(kod);

    return 0;
}

s64int koProcess(struct KOD *kod, void *data)
{
    return kod->object->process(kod->object,data);
}

void koDump(struct KOD *kod)
{
    if (kod->object->dump)
        kod->object->dump(kod->object);
}

