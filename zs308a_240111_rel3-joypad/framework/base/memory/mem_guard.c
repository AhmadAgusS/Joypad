/*
 * Copyright (c) 2019 Actions Semi Co., Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */


/**
 * @file
 * @brief mem leak check.
*/
#include <errno.h>
#include "mem_guard.h"

static int mem_guard_iterator(struct mem_guard_obj* node, void* context) {
    int * total = (int *)context;
    PRINTF("$Memory <0x%p %6u> (%s) %d \n", node->ptr, node->len - MEM_STAKE_SIZE, node->caller, node->user_info);
	*total += node->len;
	return 0;
}

static int mem_guard_check_overwirte(struct mem_guard_obj* node, void* context) {
    int *check_memory = (int *)((uint8_t *)node->ptr + node->len - 4);
	if(node == context){
		if (*check_memory != MEM_STAKE) {
	    	PRINTF("$Memory overwrite <0x%p %6u> (%s) view id %d \n", node->ptr, node->len - MEM_STAKE_SIZE, node->caller, node->user_info);
		}
		return 1;
	} else {
		return 0;
	}
}

static struct mem_guard_obj* mem_guard_obj_alloc(struct mem_guard_head* head) {
    char* free_node = head->free_list;
    if (free_node) {
        head->free_list = *(char**)free_node;
        head->free_blocks--;
        return (struct mem_guard_obj*)free_node;
    } else {
		PRINTF("mem guard node not enough free %d max %d \n", head->free_blocks, MAX_MOBJ_NR);
        mem_guard_dump(head);
        MM_ASSERT(0);
    }
    return NULL;
}

static void mem_guard_obj_free(struct mem_guard_head* head, void* node) {
    MM_ASSERT(node != NULL);
    if (node) {
        *(char**)node = head->free_list;
        head->free_list = node;
        head->free_blocks++;
    }
}

int mem_guard_add(struct mem_guard_head* head, const char* name,
    const void* obj, size_t len) {
    MUTEX_LOCK_DECLARE
    MUTEX_LOCK();
    struct mem_guard_obj* node = mem_guard_obj_alloc(head);
    int index = MM_HASH(obj);
    node->caller = name;
    node->ptr = obj;
    node->len = len;
#ifdef CONFIG_UI_MANAGER
	node->user_info = view_manager_get_current_view_id();
#endif
    node->next = head->heads[index];
    head->heads[index] = node;
    MUTEX_UNLOCK();
    return 0;
}

struct mem_guard_obj* mem_guard_find(struct mem_guard_head* head, const void* obj) {
    MUTEX_LOCK_DECLARE
        MUTEX_LOCK();
    int index = MM_HASH(obj);
    for (struct mem_guard_obj** node = &head->heads[index];
        *node != NULL;
        node = &(*node)->next) {
        if ((*node)->ptr == obj) {
            MUTEX_UNLOCK();
            return *node;
        }
    }
    MUTEX_UNLOCK();
    return NULL;
}

int mem_guard_del_locked(struct mem_guard_head* head, const void* obj) {
    struct mem_guard_obj** node, * tmp;
    int index = MM_HASH(obj);
    for (node = &head->heads[index]; *node != NULL;
        node = &(*node)->next) {
        if ((*node)->ptr == obj) {
            tmp = *node;
            *node = tmp->next;
            tmp->next = NULL;
            mem_guard_obj_free(head, tmp);
            return 0;
        }
    }
    return -EINVAL;
}

int mem_guard_del(struct mem_guard_head* head, const void* obj,
    void (*mfree)(void* obj)) {
    //MM_ASSERT(mfree != NULL);
    MUTEX_LOCK_DECLARE
        if (obj == NULL)
            return -EINVAL;
    MUTEX_LOCK();
    head->free_ptr = (void*)obj;
	if (mfree) {
		mem_guard_iterate(head, mem_guard_check_overwirte, (void*)obj);
    	mfree((void*)obj);
	}
    mem_guard_del_locked(head, obj);
    MUTEX_UNLOCK();
    return 0;
}

void mem_guard_iterate(struct mem_guard_head* head,
    int (*iterator)(struct mem_guard_obj*, void*),
    void* context) {
    MUTEX_LOCK_DECLARE
    if (iterator == NULL)
        iterator = mem_guard_iterator;
    MUTEX_LOCK();
    for (int i = 0; i < (int)MAX_MOBJ_HASH; i++) {
        for (struct mem_guard_obj* node = head->heads[i];
            node != NULL; node = node->next) {
            MUTEX_UNLOCK();
            if(iterator(node, context)) {
            	MUTEX_LOCK();
				break;
			}
			MUTEX_LOCK();
        }
    }
    MUTEX_UNLOCK();
}

void mem_guard_leak_check(struct mem_guard_head* head, int user_info)
{
    MUTEX_LOCK_DECLARE
    MUTEX_LOCK();
    for (int i = 0; i < (int)MAX_MOBJ_HASH; i++) {
        for (struct mem_guard_obj* node = head->heads[i];
            node != NULL; node = node->next) {
            MUTEX_UNLOCK();
			if (node->user_info == user_info) {
				  PRINTF("$Memory leak <0x%p %6u> (%s)\n", node->ptr, node->len, node->caller);
			}
            MUTEX_LOCK();
        }
    }
    MUTEX_UNLOCK();
}

void mem_guard_init(struct mem_guard_head* head) {
    char* p = (char*)head->nodes;
    head->free_blocks = MAX_MOBJ_NR;
    head->free_list = NULL;
    for (int i = 0; i < head->free_blocks; i++) {
        *(char**)p = head->free_list;
        head->free_list = p;
        p += sizeof(struct mem_guard_obj);
    }
}
