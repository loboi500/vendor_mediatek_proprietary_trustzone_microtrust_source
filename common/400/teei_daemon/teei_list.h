/*
 * Copyright (c) 2015-2020 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */


#ifndef __TEEI_LIST_H__
#define __TEEI_LIST_H__

#define LIST_POISON_PREV    0xDEADBEEF
#define LIST_POISON_NEXT    0xFADEBABE

struct teei_list {
	struct teei_list *next, *prev;
};

#define INIT_HEAD(__lname)  { &(__lname), &(__lname) }
#define LIST_HEAD(_lname)   struct teei_list _lname = INIT_HEAD(_lname)
#define INIT_LIST_HEAD(ptr)  do { \
	(ptr)->next = ptr; (ptr)->prev = ptr;   \
	} while (0)

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

#define list_for_each(curr, head) \
	for (curr = (head)->next; curr != head; curr = (curr)->next)

#define list_for_each_entry(ptr, head, member) \
	for (ptr = list_entry((head)->next, typeof(*ptr), member); \
		&ptr->member != (head); \
		ptr = list_entry(ptr->member.next, typeof(*ptr), member))

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_for_each_entry_safe(pos, n, head, member)          \
	for (pos = list_entry((head)->next, typeof(*pos), member),  \
		n = list_entry(pos->member.next, typeof(*pos), member); \
		&pos->member != (head);                    \
		pos = n, n = list_entry(n->member.next, typeof(*n), member))

static inline void __teei_list_add(struct teei_list *prev,
			struct teei_list *next, struct teei_list *entry)
{
	entry->prev = prev;
	entry->next = next;
	prev->next = entry;
	next->prev = entry;
}

static inline void teei_list_add(struct teei_list *head,
					struct teei_list *entry)
{
	__teei_list_add(head, head->next, entry);
}

static inline void teei_list_add_tail(struct teei_list *tnode,
					struct teei_list *entry)
{
	__teei_list_add(tnode->prev, tnode, entry);
}

static inline void __teei_list_del(struct teei_list *entry,
			struct teei_list *prev, struct teei_list *next)
{
	prev->next = entry->next;
	next->prev = entry->prev;
	entry->next = (void *)LIST_POISON_NEXT;
	entry->prev = (void *)LIST_POISON_PREV;
}

static inline void teei_list_del(struct teei_list *entry)
{
	__teei_list_del(entry, entry->prev, entry->next);
}

static inline struct teei_list *teei_list_pop_tail(struct teei_list *head)
{
	struct teei_list *entry = head->prev;

	teei_list_del(head->prev);
	return entry;
}

static inline struct teei_list *teei_list_pop(struct teei_list *head)
{
	struct teei_list *entry = head->next;

	teei_list_del(head->next);
	return entry;
}

static inline int teei_list_empty(struct teei_list *head)
{
	if (head->next == head)
		return 1;
	else
		return 0;
}

#endif /* __TEEI_LIST_H__ */
