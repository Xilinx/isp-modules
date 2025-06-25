/* SPDX-License-Identifier: MIT*/
/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************
 *
 * The GPL License (GPL)
 *
 * Copyright (c) 2025 VeriSilicon Holdings Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 *****************************************************************************
 *
 * Note: This software is released under dual MIT and GPL licenses. A
 * recipient may use this file under the terms of either the MIT license or
 * GPL License. If you wish to use only one license not the other, you can
 * indicate your decision by deleting one of the above license notices in your
 * version of this file.
 *
 *****************************************************************************/

#ifndef __LIST_H__
#define __LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/list.h>
#include <linux/types.h>

struct ListHead_s {
	struct ListHead_s *prev;
	struct ListHead_s *next;
};

#define ContainerOf(Ptr, type, Member)                                         \
	((type *)(((char *)Ptr) - (size_t)(&((type *)0)->Member)))

static inline void InitListHead(struct ListHead_s *list)
{
	list->prev = list;
	list->next = list;
}

static inline void __ListAdd(struct ListHead_s *new, struct ListHead_s *prev,
			     struct ListHead_s *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void ListAdd(struct ListHead_s *new, struct ListHead_s *head)
{
	__ListAdd(new, head, head->next);
}

static inline void ListAddTail(struct ListHead_s *new,
			       struct ListHead_s *head)
{
	__ListAdd(new, head->prev, head);
}

static inline void __ListDel(struct ListHead_s *prev, struct ListHead_s *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __ListDelEntry(struct ListHead_s *entry)
{
	__ListDel(entry->prev, entry->next);
}

static inline void ListDel(struct ListHead_s *entry)
{
	__ListDelEntry(entry);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline int ListEmpty(const struct ListHead_s *head)
{
	return head->next == head;
}

#define ListEntry(Ptr, type, Member) ContainerOf(Ptr, type, Member)

#define ListFirstEntry(Ptr, type, Member) ListEntry((Ptr)->next, type, Member)

#define ListLastEntry(Ptr, type, Member) ListEntry((Ptr)->prev, type, Member)

#define ListNextEntry(Pos, Member)                                             \
	ListEntry((Pos)->Member.next, typeof(*(Pos)), Member)

#define ListPrivEntry(Pos, Member)                                             \
	ListEntry((Pos)->Member.prev, typeof(*(Pos)), Member)

#define ListForEachEntry(Pos, head, Member)                                    \
	for (Pos = ListFirstEntry(head, typeof(*Pos), Member);                 \
	     &Pos->Member != (head); Pos = ListNextEntry(Pos, Member))

#define ListForEachEntrySafe(Pos, N, head, Member)                             \
	for (Pos = ListFirstEntry(head, typeof(*Pos), Member),                 \
	    N = ListNextEntry(Pos, Member);                                    \
	     &Pos->Member != (head); Pos = N, N = ListNextEntry(N, Member))

#ifdef __cplusplus
}
#endif

#endif
