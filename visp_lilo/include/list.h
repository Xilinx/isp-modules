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
extern "C"
{
#endif

#include <linux/list.h>
#include <linux/types.h>

	struct ListHead_s
	{
		struct ListHead_s *Prev;
		struct ListHead_s *Next;
	};

#define ContainerOf(Ptr, Type, Member) \
	((Type *)(((char *)Ptr) - (size_t)(&((Type *)0)->Member)))

	static inline void InitListHead(struct ListHead_s *List)
	{
		List->Prev = List;
		List->Next = List;
	}

	static inline void __ListAdd(struct ListHead_s *New,
								 struct ListHead_s *Prev,
								 struct ListHead_s *Next)
	{
		Next->Prev = New;
		New->Next = Next;
		New->Prev = Prev;
		Prev->Next = New;
	}

	static inline void ListAdd(struct ListHead_s *New, struct ListHead_s *Head)
	{
		__ListAdd(New, Head, Head->Next);
	}

	static inline void ListAddTail(struct ListHead_s *New,
								   struct ListHead_s *Head)
	{
		__ListAdd(New, Head->Prev, Head);
	}

	static inline void __ListDel(struct ListHead_s *Prev,
								 struct ListHead_s *Next)
	{
		Next->Prev = Prev;
		Prev->Next = Next;
	}

	static inline void __ListDelEntry(struct ListHead_s *Entry)
	{
		__ListDel(Entry->Prev, Entry->Next);
	}

	static inline void ListDel(struct ListHead_s *Entry)
	{
		__ListDelEntry(Entry);
		Entry->Next = NULL;
		Entry->Prev = NULL;
	}

	static inline int ListEmpty(const struct ListHead_s *Head)
	{
		return Head->Next == Head;
	}

#define ListEntry(Ptr, Type, Member) ContainerOf(Ptr, Type, Member)

#define ListFirstEntry(Ptr, Type, Member) ListEntry((Ptr)->Next, Type, Member)

#define ListLastEntry(Ptr, Type, Member) ListEntry((Ptr)->Prev, Type, Member)

#define ListNextEntry(Pos, Member) \
	ListEntry((Pos)->Member.Next, typeof(*(Pos)), Member)

#define ListPrivEntry(Pos, Member) \
	ListEntry((Pos)->Member.Prev, typeof(*(Pos)), Member)

#define ListForEachEntry(Pos, Head, Member)                \
	for (Pos = ListFirstEntry(Head, typeof(*Pos), Member); \
		 &Pos->Member != (Head); Pos = ListNextEntry(Pos, Member))

#define ListForEachEntrySafe(Pos, N, Head, Member)         \
	for (Pos = ListFirstEntry(Head, typeof(*Pos), Member), \
		N = ListNextEntry(Pos, Member);                    \
		 &Pos->Member != (Head); Pos = N, N = ListNextEntry(N, Member))

#ifdef __cplusplus
}
#endif

#endif
