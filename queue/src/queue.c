// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
* @file    		queue.c
* @author  		Kovalchuk Alexander roux@yandex.ru
* @brief		Queue with fixed size.
*
* @warning 		This data structure is not thread-safe!
*/

/*
 * Copyright (c) year Alexander KOVALCHUK
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of library_name.
 *
 * Author:          Alexander KOVALCHUK <roux@yandex.ru>
 */
//_____ I N C L U D E S _______________________________________________________________________
#include "queue.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
//_____ C O N F I G S  ________________________________________________________________________
// If unit testing is enabled override assert with mock_assert().
#if defined(UNIT_TESTING)
extern void mock_assert(const int result, const char* const expression,
                        const char * const file, const int line);
#undef assert
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif // UNIT_TESTING
//_____ D E F I N I T I O N ___________________________________________________________________
/**
 * \brief Static queue structure
 */
struct Queue_t
{
#if defined(QUEUE_STATIC_MODE)
	uint8_t id;
#endif
	uint8_t* data;																				///< array of data
    size_t write;																				///< pointer to the write position
    size_t read;																				///< pointer to the read position
    size_t size;																				///< current size of queue
    size_t capacity;																			///< max size of queue
    size_t esize;																				///< size in bytes one element
};
//_____ M A C R O S ___________________________________________________________________________
//_____ V A R I A B L E   D E F I N I T I O N  ________________________________________________
#if defined(QUEUE_STATIC_MODE)
	void* queue_static_malloc(size_t sizemem);
	void queue_static_free(void * ptrmem);

	static void* (*mem_alloc_fn)(size_t sizemem) = queue_static_malloc;
	static void (*mem_free_fn) (void *ptrmem) = queue_static_free;

	static queue_t pool[MAX_QUEUES] = {0};
	static size_t counter = 0;
#else
	//!Pointer to the memory allocation function
	static void* (*mem_alloc_fn)(size_t sizemem) = NULL;
	//!Pointer to the memory free function
	static void (*mem_free_fn) (void *ptrmem) = NULL;
#endif
//_____ I N L I N E   F U N C T I O N   D E F I N I T I O N   _________________________________
inline static bool is_callbacks_valid(void)
{
	return ((mem_free_fn != NULL) && (mem_alloc_fn != NULL)) ? true : false;
}

#if defined(QUEUE_STATIC_MODE)
void* queue_static_malloc(size_t sizemem)
{
	void* queue = NULL;

	if(counter < MAX_QUEUES)
	{
		queue = &pool[counter++];
	}

	return queue;
}

void queue_static_free(void * ptrmem)
{
}
#endif
//_____ S T A T I C  F U N C T I O N   D E F I N I T I O N   __________________________________
//_____ F U N C T I O N   D E F I N I T I O N   _______________________________________________
/**
* This function used to register function for alloc memory.
*
* Public function defined in queue.h
*/
void queue_reg_mem_alloc_cb(void* (*custom_malloc)(size_t sizemem))
{
	assert(custom_malloc);
#if defined(QUEUE_STATIC_MODE)
	mem_alloc_fn = queue_static_malloc;
#else
	mem_alloc_fn = custom_malloc;
#endif
}

/**
* This function used to register function for free memory.
*
* Public function defined in queue.h
*/
void queue_reg_mem_free_cb(void (*custom_free)(void * ptrmem))
{
	assert(custom_free);
#if defined(QUEUE_STATIC_MODE)
	mem_free_fn = queue_static_free;
#else
	mem_free_fn = custom_free;
#endif
}

/**
* This function used to create new queue.
*
* Public function defined in queue.h
*/
queue_t* queue_create(size_t nbm, size_t esize, uint8_t* pBuf)
{
	queue_t* queue = NULL;

#if defined(QUEUE_STATIC_MODE)
	assert(pBuf);
#endif

	size_t rawSize = nbm * esize;

	if(!is_callbacks_valid()) {
		return false;
	}

	queue = mem_alloc_fn(sizeof(*queue));
	if (queue == NULL) {
		return NULL;
	}

#if !defined(QUEUE_STATIC_MODE)
	queue->data = mem_alloc_fn(rawSize);
	if (queue->data == NULL)
	{
		mem_free_fn((void*)queue);
		return NULL;
	}
#else
	queue->data = pBuf;
#endif

	queue->capacity = nbm;
	queue->write = 0;
	queue->read = 0;
	queue->esize = esize;
	queue->size = 0;

	for(size_t i = 0; i < rawSize; i++) {
		queue->data[i] = 0;
	}

	return queue;
}

/**
* This function used to delete queue.
*
* Public function defined in queue.h
*/
void queue_delete(queue_t **queue)
{
#if !defined(QUEUE_STATIC_MODE)
	assert(*queue);

	mem_free_fn((*queue)->data);
	mem_free_fn(*queue);
	*queue = NULL;
#else
	counter = 0;
#endif
}

/**
* This function used to check the queue for empty.
*
* Public function defined in queue.h
*/
bool queue_is_empty(const queue_t *queue)
{
	assert(queue);

	if(queue->size == 0) {
		return true;
	}

	return false;
}

/**
* This function used to check the queue for full.
*
* Public function defined in queue.h
*/
bool queue_is_full(const queue_t *queue)
{
	assert(queue);

	if(queue->size == queue->capacity) {
		return true;
	}

	return false;
}

/**
* This function return size of queue.
*
* Public function defined in queue.h
*/
size_t queue_size(const queue_t *queue)
{
	assert(queue);
	return queue->size;
}

/**
* This function return free size of queue.
*
* Public function defined in queue.h
*/
size_t queue_free_space(const queue_t *queue)
{
	assert(queue);
	return queue->capacity - queue->size;
}

/**
* This function add data into queue.
*
* Public function defined in queue.h
*/
bool queue_enqueue(queue_t *queue, const void *data)
{
	assert(queue);
	assert(data);

	uint8_t* pData = (uint8_t*)data;
	size_t rawSize = queue->capacity * queue->esize;

	if(queue_is_full(queue)) {
		return false;
	}

	for(size_t i = 0; i < queue->esize; i++)
	{
		queue->data[queue->write] = pData[i];
		queue->write = (queue->write == rawSize - 1ul) ? 0ul: (queue->write + 1ul);
	}

	queue->size++;

	return true;
}

/**
* This function used to get data from queue.
*
* Public function defined in queue.h
*/
bool queue_denqueue(queue_t *queue, void *data)
{
	assert(queue);
	assert(data);

	uint8_t* pData = (uint8_t*)data;
	size_t rawSize = queue->capacity * queue->esize;

	if(queue_is_empty(queue)) {
		return false;
	}

	for(size_t i = 0; i < queue->esize; i++)
	{
		pData[i] = queue->data[queue->read];
		queue->read = (queue->read == rawSize - 1ul) ? 0ul : (queue->read + 1ul);
	}

	queue->size--;

	return true;
}

/**
* This function used to get data from queue without deleting.
*
* Public function defined in queue.h
*/
bool queue_peek(const queue_t *queue, void *data)
{
	assert(queue);
	assert(data);

	uint8_t* pData = (uint8_t*)data;
	uint8_t* pBuffer = &queue->data[queue->read];

	if(queue_is_empty(queue)) {
		return false;
	}

	for(size_t i = 0; i < queue->esize; i++)
	{
		pData[i] = *pBuffer++;
	}

	return true;
}

/**
* This function search data in the queue.
*
* Public function defined in queue.h
*/
bool queue_find(const queue_t *queue, const void *data, bool (*is_equal)(const void*, const void*))
{
	assert(queue);
	assert(data);
	assert(is_equal);

	size_t size = queue_size(queue);

	for(size_t i = 0; i < size; i++)
	{
		if(is_equal((void*)&queue->data[i * queue->esize], data)) {
			return true;
		}
	}

	return false;
}

/**
* This function used to reset queue.
*
* Public function defined in queue.h
*/
void queue_flush(queue_t *queue)
{
	assert(queue);

	size_t rawSize = queue->capacity * queue->esize;

	queue->write = 0;
	queue->read = 0;
	queue->size = 0;

	for(size_t i = 0; i < rawSize; i++) {
		queue->data[i] = 0;
	}
}
