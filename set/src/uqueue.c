/**
* \file    		template.c
* \author  		Kovalchuk Alexander
* \email  		roux@yandex.ru
* \brief   		This file contains the prototypes functions which use for...
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
//_____ I N C L U D E S _______________________________________________________
#include "uqueue.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
//_____ C O N F I G S  ________________________________________________________
//_____ D E F I N I T I O N ___________________________________________________
/**
 * \brief Static queue structure
 */
struct UQueue_t
{
	uint8_t* data;																				///< array of data
    size_t write;																				///< pointer to the write position
    size_t read;																				///< pointer to the read position
    size_t size;																				///< current size of queue
    size_t capacity;																			///< max size of queue
    size_t esize;																				///< size in bytes one element
    uqueue_is_equal_fn_t compare_cb;
};
//_____ M A C R O S ___________________________________________________________
//_____ V A R I A B L E   D E F I N I T I O N  ________________________________
//!Pointer to the memory allocation function
static void* (*mem_alloc_fn)(size_t sizemem) = NULL;

//!Pointer to the memory free function
static void (*mem_free_fn) (void *ptrmem) = NULL;
//_____ I N L I N E   F U N C T I O N   D E F I N I T I O N   _________________
//_____ S T A T I C  F U N C T I O N   D E F I N I T I O N   __________________
//_____ F U N C T I O N   D E F I N I T I O N   _______________________________
/**
* This function used to register function for alloc memory.
*
* Public function defined in queue.h
*/
void uqueue_reg_mem_alloc_cb(void* (*custom_malloc)(size_t sizemem))
{
	if(custom_malloc == NULL) {
		return;
	}

	mem_alloc_fn = custom_malloc;
}

/**
* This function used to register function for free memory.
*
* Public function defined in queue.h
*/
void uqueue_reg_mem_free_cb(void (*custom_free)(void * ptrmem))
{
	if(custom_free == NULL) {
		return;
	}

	mem_free_fn = custom_free;
}


void uqueue_reg_compare_cb(uqueue_t *uqueue, uqueue_is_equal_fn_t custom_compare)
{
    if(custom_compare == NULL) {
        return;
    }

    uqueue->compare_cb = custom_compare;
}


/**
* This function used to create new queue.
*
* Public function defined in queue.h
*/
uqueue_t* uqueue_create(size_t capacity, size_t esize, const uqueue_is_equal_fn_t custom_compare)
{
	if(custom_compare == NULL) {
		return NULL;
	}

	if(mem_free_fn == NULL || mem_alloc_fn == NULL) {
		return NULL;
	}

	uqueue_t * uqueue = (uqueue_t*) mem_alloc_fn(sizeof(uqueue_t));
	if (uqueue == NULL) {
		return NULL;
	}

	uqueue->data = (uint8_t*) mem_alloc_fn(capacity * esize);
	if (uqueue->data == NULL)
	{
		mem_free_fn((void*)uqueue);
		return NULL;
	}

	uqueue->write = 0;
	uqueue->read = 0;
	uqueue->capacity = capacity;
	uqueue->esize = esize;
	uqueue->size = 0;
	uqueue->compare_cb = custom_compare;

	for(size_t i = 0; i < uqueue->capacity; i++) {
		uqueue->data[i] = 0;
	}

	return uqueue;
}

/**
* This function used to delete queue.
*
* Public function defined in queue.h
*/
void uqueue_delete(uqueue_t **uqueue)
{
	if(*uqueue == NULL) {
		return;
	}

	mem_free_fn((*uqueue)->data);
	mem_free_fn(*uqueue);
	*uqueue = NULL;
}

/**
* This function used to check the queue for empty.
*
* Public function defined in queue.h
*/
bool uqueue_is_empty(const uqueue_t *uqueue)
{
	if(uqueue->size == 0) {
		return true;
	}

	return (((uqueue->capacity - uqueue->size) >= uqueue->capacity)) ? true : false;
}

/**
* This function used to check the queue for full.
*
* Public function defined in queue.h
*/
bool uqueue_is_full(const uqueue_t *uqueue)
{
	if(uqueue->size == uqueue->capacity) {
		return true;
	}

	return (uqueue->size + uqueue->esize > uqueue->capacity) ? true : false;
}

/**
* This function return size of queue.
*
* Public function defined in queue.h
*/
size_t uqueue_size(const uqueue_t *uqueue)
{
	return uqueue->size/uqueue->esize;
}



/**
* This function return free size of queue.
*
* Public function defined in queue.h
*/
size_t uqueue_free_space(const uqueue_t *uqueue)
{
	return uqueue->capacity - uqueue->size/uqueue->esize;
}

/**
* This function add data into queue.
*
* Public function defined in queue.h
*/
bool uqueue_enqueue(uqueue_t *uqueue, const void *data)
{
	size_t size = 0;
	uint8_t* pData = (uint8_t*)data;

	if(uqueue == NULL || data == NULL) {
		return false;
	}

	if(mem_free_fn == NULL || mem_alloc_fn == NULL) {
		return false;
	}

	if(uqueue->compare_cb == NULL) {
		return false;
	}

	if(uqueue_is_full(uqueue)) {
		return false;
	}

	size = uqueue_size(uqueue);

	for(size_t i = 0; i < size; i++)
	{
		if(uqueue->compare_cb((void*)&uqueue->data[i * uqueue->esize], data)) {
			return true;
		}
	}

	for(size_t i = 0; i < uqueue->esize; i++)
	{
		uqueue->data[uqueue->write] = pData[i];
		uqueue->size++;
		uqueue->write = (uqueue->write == uqueue->capacity - 1ul) ? 0ul: (uqueue->write + 1ul);
	}

	return true;
}


/**
* This function used to get data from queue.
*
* Public function defined in queue.h
*/
bool uqueue_denqueue(uqueue_t *uqueue, void *data)
{
	uint8_t* pData = (uint8_t*)data;

	if(uqueue == NULL || data == NULL) {
		return false;
	}

	if(mem_free_fn == NULL || mem_alloc_fn == NULL) {
		return false;
	}

	if(uqueue->compare_cb == NULL) {
		return false;
	}

	if(uqueue_is_empty(uqueue)) {
		return false;
	}

	for(size_t i = 0; i < uqueue->esize; i++)
	{
		pData[i] = uqueue->data[uqueue->read];
		uqueue->size--;
		uqueue->read = (uqueue->read == uqueue->capacity - 1ul) ? 0ul : (uqueue->read + 1ul);
	}

	return true;
}

