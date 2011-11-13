#ifndef TYPES_H
#define TYPES_H

#define NULL 0

#define offsetOf(TYPE, MEMBER) ((unsigned long) &((TYPE *)0)->MEMBER)

/**
 * * containerOf - cast a member of a structure out to the containing structure
 * * @ptr:    the pointer to the member.
 * * @type:   the type of the container struct this is embedded in.
 * * @member: the name of the member within the struct.
 * * */
#define containerOf(ptr, type, member) ({                      \
                const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
                (type *)( (char *)__mptr - offsetOf(type,member) );})

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

struct ListHead {
	struct ListHead *next, *prev;
};

struct HListHead {
	struct HListNode *first;
};

struct HListNode {
	struct HListNode *next, **pprev;
};

#endif /* TYPES_H */
