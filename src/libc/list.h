#ifndef LIST_H
#define LIST_H

#include "listtypes.h"
#include "prefetch.h"
#include "poison.h"

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("_Xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct ListHead name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct ListHead *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void _listAdd(struct ListHead *new,
			      struct ListHead *prev,
			      struct ListHead *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

/**
 * listAdd - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void listAdd(struct ListHead *new, struct ListHead *head)
{
	_listAdd(new, head, head->next);
}


/**
 * listAddTail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void listAddTail(struct ListHead *new, struct ListHead *head)
{
	_listAdd(new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void _listDel(struct ListHead * prev, struct ListHead * next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * listDel - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: listEmpty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void _listDelEntry(struct ListHead *entry)
{
	_listDel(entry->prev, entry->next);
}

static inline void listDel(struct ListHead *entry)
{
	_listDel(entry->prev, entry->next);
	entry->next = LIST_POISON1;
	entry->prev = LIST_POISON2;
}

/**
 * listReplace - replace old entry by new one
 * @old : the element to be replaced
 * @new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void listReplace(struct ListHead *old,
				struct ListHead *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void listReplaceInit(struct ListHead *old,
					struct ListHead *new)
{
	listReplace(old, new);
	INIT_LIST_HEAD(old);
}

/**
 * listDelInit - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
static inline void listDelInit(struct ListHead *entry)
{
	_listDelEntry(entry);
	INIT_LIST_HEAD(entry);
}

/**
 * listMove - delete from one list and add as another's head
 * @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void listMove(struct ListHead *list, struct ListHead *head)
{
	_listDelEntry(list);
	listAdd(list, head);
}

/**
 * listMoveTail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void listMoveTail(struct ListHead *list,
				  struct ListHead *head)
{
	_listDelEntry(list);
	listAddTail(list, head);
}

/**
 * listIsLast - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int listIsLast(const struct ListHead *list,
				const struct ListHead *head)
{
	return list->next == head;
}

/**
 * listEmpty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int listEmpty(const struct ListHead *head)
{
	return head->next == head;
}

/**
 * listEmptyCareful - tests whether a list is empty and not being modified
 * @head: the list to test
 *
 * Description:
 * tests whether a list is empty And_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using listEmptyCareful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is listDelInit(). Eg. it cannot be used
 * if another CPU could re-listAdd() it.
 */
static inline int listEmptyCareful(const struct ListHead *head)
{
	struct ListHead *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * listRotateLeft - rotate the list to the left
 * @head: the head of the list
 */
static inline void listRotateLeft(struct ListHead *head)
{
	struct ListHead *first;

	if (!listEmpty(head)) {
		first = head->next;
		listMoveTail(first, head);
	}
}

/**
 * listIsSingular - tests whether a list has just one entry.
 * @head: the list to test.
 */
static inline int listIsSingular(const struct ListHead *head)
{
	return !listEmpty(head) && (head->next == head->prev);
}

static inline void _listCutPosition(struct ListHead *list,
		struct ListHead *head, struct ListHead *entry)
{
	struct ListHead *newFirst = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = newFirst;
	newFirst->prev = head;
}

/**
 * listCutPosition - cut a list into two
 * @list: a new list to add all removed entries
 * @head: a list with entries
 * @entry: an entry within head, could be the head itself
 *	and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static inline void listCutPosition(struct ListHead *list,
		struct ListHead *head, struct ListHead *entry)
{
	if (listEmpty(head))
		return;
	if (listIsSingular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		_listCutPosition(list, head, entry);
}

static inline void _listSplice(const struct ListHead *list,
				 struct ListHead *prev,
				 struct ListHead *next)
{
	struct ListHead *first = list->next;
	struct ListHead *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * listSplice - join two lists, this is designed for stacks
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void listSplice(const struct ListHead *list,
				struct ListHead *head)
{
	if (!listEmpty(list))
		_listSplice(list, head, head->next);
}

/**
 * listSpliceTail - join two lists, each list being a queue
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
static inline void listSpliceTail(struct ListHead *list,
				struct ListHead *head)
{
	if (!listEmpty(list))
		_listSplice(list, head->prev, head);
}

/**
 * listSpliceInit - join two lists and reinitialise the emptied list.
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static inline void listSpliceInit(struct ListHead *list,
				    struct ListHead *head)
{
	if (!listEmpty(list)) {
		_listSplice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}

/**
 * listSpliceTailInit - join two lists and reinitialise the emptied list
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static inline void listSpliceTailInit(struct ListHead *list,
					 struct ListHead *head)
{
	if (!listEmpty(list)) {
		_listSplice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}

/**
 * listEntry - get the struct for this entry
 * @ptr:	the &struct ListHead pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the listStruct within the struct.
 */
#define listEntry(ptr, type, member) \
	containerOf(ptr, type, member)

/**
 * listFirstEntry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the listStruct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define listFirstEntry(ptr, type, member) \
	listEntry((ptr)->next, type, member)

/**
 * listForEach	-	iterate over a list
 * @pos:	the &struct ListHead to use as a loop cursor.
 * @head:	the head for your list.
 */
#define listForEach(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * _listForEach	-	iterate over a list
 * @pos:	the &struct ListHead to use as a loop cursor.
 * @head:	the head for your list.
 *
 * This variant doesn't differ from listForEach() any more.
 * We don't do prefetching in either case.
 */
#define _listForEach(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * listForEachPrev	-	iterate over a list backwards
 * @pos:	the &struct ListHead to use as a loop cursor.
 * @head:	the head for your list.
 */
#define listForEachPrev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * listForEachSafe - iterate over a list safe against removal of list entry
 * @pos:	the &struct ListHead to use as a loop cursor.
 * @n:		another &struct ListHead to use as temporary storage
 * @head:	the head for your list.
 */
#define listForEachSafe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * listForEachPrevSafe - iterate over a list backwards safe against removal of list entry
 * @pos:	the &struct ListHead to use as a loop cursor.
 * @n:		another &struct ListHead to use as temporary storage
 * @head:	the head for your list.
 */
#define listForEachPrevSafe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)

/**
 * listForEachEntry	-	iterate over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 */
#define listForEachEntry(pos, head, member)				\
	for (pos = listEntry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = listEntry(pos->member.next, typeof(*pos), member))

/**
 * listForEachEntryReverse - iterate backwards over list of given type.
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 */
#define listForEachEntryReverse(pos, head, member)			\
	for (pos = listEntry((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = listEntry(pos->member.prev, typeof(*pos), member))

/**
 * listPrepareEntry - prepare a pos entry for use in listForEachEntryContinue()
 * @pos:	the type * to use as a start point
 * @head:	the head of the list
 * @member:	the name of the listStruct within the struct.
 *
 * Prepares a pos entry for use as a start point in listForEachEntryContinue().
 */
#define listPrepareEntry(pos, head, member) \
	((pos) ? : listEntry(head, typeof(*pos), member))

/**
 * listForEachEntryContinue - continue iteration over list of given type
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define listForEachEntryContinue(pos, head, member) 		\
	for (pos = listEntry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = listEntry(pos->member.next, typeof(*pos), member))

/**
 * listForEachEntryContinueReverse - iterate backwards from the given point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define listForEachEntryContinueReverse(pos, head, member)		\
	for (pos = listEntry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = listEntry(pos->member.prev, typeof(*pos), member))

/**
 * listForEachEntryFrom - iterate over list of given type from the current point
 * @pos:	the type * to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define listForEachEntryFrom(pos, head, member) 			\
	for (; &pos->member != (head);	\
	     pos = listEntry(pos->member.next, typeof(*pos), member))

/**
 * listForEachEntrySafe - iterate over list of given type safe against removal of list entry
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 */
#define listForEachEntrySafe(pos, n, head, member)			\
	for (pos = listEntry((head)->next, typeof(*pos), member),	\
		n = listEntry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = listEntry(n->member.next, typeof(*n), member))

/**
 * listForEachEntrySafeContinue - continue list iteration safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define listForEachEntrySafeContinue(pos, n, head, member) 		\
	for (pos = listEntry(pos->member.next, typeof(*pos), member), 		\
		n = listEntry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = listEntry(n->member.next, typeof(*n), member))

/**
 * listForEachEntrySafeFrom - iterate over list from current point safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define listForEachEntrySafeFrom(pos, n, head, member) 			\
	for (n = listEntry(pos->member.next, typeof(*pos), member);		\
	     &pos->member != (head);						\
	     pos = n, n = listEntry(n->member.next, typeof(*n), member))

/**
 * listForEachEntrySafeReverse - iterate backwards over list safe against removal
 * @pos:	the type * to use as a loop cursor.
 * @n:		another type * to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the listStruct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define listForEachEntrySafeReverse(pos, n, head, member)		\
	for (pos = listEntry((head)->prev, typeof(*pos), member),	\
		n = listEntry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = listEntry(n->member.prev, typeof(*n), member))

/**
 * listSafeResetNext - reset a stale listForEachEntrySafe loop
 * @pos:	the loop cursor used in the listForEachEntrySafe loop
 * @n:		temporary storage used in listForEachEntrySafe
 * @member:	the name of the listStruct within the struct.
 *
 * listSafeResetNext is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and listSafeResetNext is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define listSafeResetNext(pos, n, member)				\
	n = listEntry(pos->member.next, typeof(*pos), member)

/*
 * Double linked lists with a single pointer list head.
 * Mostly useful for hash tables where the two pointer list head is
 * too wasteful.
 * You lose the ability to access the tail in O(1).
 */

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct HListHead name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct HListNode *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlistUnhashed(const struct HListNode *h)
{
	return !h->pprev;
}

static inline int hlistEmpty(const struct HListHead *h)
{
	return !h->first;
}

static inline void _HlistDel(struct HListNode *n)
{
	struct HListNode *next = n->next;
	struct HListNode **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlistDel(struct HListNode *n)
{
	_HlistDel(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlistDelInit(struct HListNode *n)
{
	if (!hlistUnhashed(n)) {
		_HlistDel(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlistAddHead(struct HListNode *n, struct HListHead *h)
{
	struct HListNode *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlistAddBefore(struct HListNode *n,
					struct HListNode *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static inline void hlistAddAfter(struct HListNode *n,
					struct HListNode *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

/* after that we'll appear to be on some hlist and hlistDel will work */
static inline void hlistAddFake(struct HListNode *n)
{
	n->pprev = &n->next;
}

/*
 * Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists.
 */
static inline void hlistMoveList(struct HListHead *old,
				   struct HListHead *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define hlistEntry(ptr, type, member) containerOf(ptr,type,member)

#define hlistForEach(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define hlistForEachSafe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)

/**
 * hlistForEachEntry	- iterate over list of given type
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct HListNode to use as a loop cursor.
 * @head:	the head for your list.
 * @member:	the name of the hlistNode within the struct.
 */
#define hlistForEachEntry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos &&							 \
		({ tpos = hlistEntry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlistForEachEntryContinue - iterate over a hlist continuing after current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct HListNode to use as a loop cursor.
 * @member:	the name of the hlistNode within the struct.
 */
#define hlistForEachEntryContinue(tpos, pos, member)		 \
	for (pos = (pos)->next;						 \
	     pos &&							 \
		({ tpos = hlistEntry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlistForEachEntryFrom - iterate over a hlist continuing from current point
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct HListNode to use as a loop cursor.
 * @member:	the name of the hlistNode within the struct.
 */
#define hlistForEachEntryFrom(tpos, pos, member)			 \
	for (; pos &&							 \
		({ tpos = hlistEntry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)

/**
 * hlistForEachEntrySafe - iterate over list of given type safe against removal of list entry
 * @tpos:	the type * to use as a loop cursor.
 * @pos:	the &struct HListNode to use as a loop cursor.
 * @n:		another &struct HListNode to use as temporary storage
 * @head:	the head for your list.
 * @member:	the name of the hlistNode within the struct.
 */
#define hlistForEachEntrySafe(tpos, pos, n, head, member) 		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) && 				 \
		({ tpos = hlistEntry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)

#endif /* LIST_H */
