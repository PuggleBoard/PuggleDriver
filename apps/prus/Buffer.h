/*
	 -------------------------------------------------------------------------

	 This file is part of the BoneClamp Data Conversion and Processing System
	 Copyright (C) 2013 BoneClamp

	 -------------------------------------------------------------------------

	Written in 2013 by: Yogi Patel <yapatel@gatech.edu>

	To the extent possible under law, the author(s) have dedicated all copyright
	and related and neighboring rights to this software to the public domain
	worldwide. This software is distributed without any warranty.

	You should have received a copy of the CC Public Domain Dedication along with
	this software. If not, see <http://creativecommons.org/licenses/by-sa/3.0/legalcode>.
 */

#ifndef Buffer_h
#define Buffer_h

#include <assert.h>
#include <libkern/OSAtomic.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct {
	void             *buffer;
  int32_t           length;
  int32_t           tail;
  int32_t           head;
  volatile int32_t  fillCount;
} Buffer;

//Initialise buffer
bool  BufferInit(Buffer *buffer, int32_t length);

//Cleanup buffer
void  BufferCleanup(Buffer *buffer);

//Clear buffer
void  BufferClear(Buffer *buffer);

// Reading

//Access end of buffer
static __inline__ __attribute__((always_inline)) void* BufferTail(Buffer *buffer, int32_t* availableBytes) {
    *availableBytes = buffer->fillCount;
    if ( *availableBytes == 0 ) return NULL;
    return (void*)((char*)buffer->buffer + buffer->tail);
}

//Read bytes
static __inline__ __attribute__((always_inline)) void BufferRead(Buffer *buffer, int32_t amount) {
    buffer->tail = (buffer->tail + amount) % buffer->length;
    OSAtomicAdd32Barrier(-amount, &buffer->fillCount);
    assert(buffer->fillCount >= 0);
}

//Access front of buffer
static __inline__ __attribute__((always_inline)) void* BufferHead(Buffer *buffer, int32_t* availableBytes) {
    *availableBytes = (buffer->length - buffer->fillCount);
    if ( *availableBytes == 0 ) return NULL;
    return (void*)((char*)buffer->buffer + buffer->head);
}
    
// Writing

//Is buffer memory spot ready?
static __inline__ __attribute__((always_inline)) void BufferWrite(Buffer *buffer, int amount) {
    buffer->head = (buffer->head + amount) % buffer->length;
    OSAtomicAdd32Barrier(amount, &buffer->fillCount);
    assert(buffer->fillCount <= buffer->length);
}

//Copy bytes to buffer
static __inline__ __attribute__((always_inline)) bool BufferWriteBytes(Buffer *buffer, const void* src, int32_t len) {
    int32_t space;
    void *ptr = BufferHead(buffer, &space);
    if ( space < len ) return false;
    memcpy(ptr, src, len);
    BufferWrite(buffer, len);
    return true;
}

#ifdef __cplusplus
}
#endif

#endif
