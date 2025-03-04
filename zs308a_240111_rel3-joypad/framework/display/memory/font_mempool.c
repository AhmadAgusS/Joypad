#include <os_common_api.h>
#include <mem_manager.h>
#include <string.h>
#include "bitmap_font_api.h"
#include "res_mempool.h"

#ifdef CONFIG_SIMULATOR
#include <kheap.h>
#define ALLOC_NO_WAIT OS_NO_WAIT
#else
#define ALLOC_NO_WAIT K_NO_WAIT
#endif

typedef struct _cache_mem_info
{
	void* ptr;
	size_t size;
	struct _cache_mem_info* next;
}cache_mem_info_t;


#define CMAP_CACHE_SIZE							1024*8

#ifdef CONFIG_BITMAP_FONT_USE_HIGH_FREQ_CACHE
#ifdef CONFIG_BITMAP_FONT_HIGH_FREQ_CACHE_SIZE
#define BITMAP_FONT_HIGH_FREQ_CACHE_SIZE		CONFIG_BITMAP_FONT_HIGH_FREQ_CACHE_SIZE
#else
#define BITMAP_FONT_HIGH_FREQ_CACHE_SIZE		1500*1024
#endif //CONFIG_BITMAP_FONT_HIGH_FREQ_CACHE_SIZE
#else
#define BITMAP_FONT_HIGH_FREQ_CACHE_SIZE		0
#endif //CONFIG_BITMAP_FONT_USE_HIGH_FREQ_CACHE

#ifdef CONFIG_BITMAP_PER_FONT_CACHE_SIZE
#define BITMAP_FONT_CACHE_SIZE				CONFIG_BITMAP_PER_FONT_CACHE_SIZE
#else
#define BITMAP_FONT_CACHE_SIZE				1024*64
#endif //CONFIG_BITMAP_PER_FONT_CACHE_SIZE

#ifdef CONFIG_BITMAP_FONT_MAX_OPENED_FONT
#define MAX_OPEND_FONT						CONFIG_BITMAP_FONT_MAX_OPENED_FONT
#else
#define MAX_OPEND_FONT						2
#endif //CONFIG_BITMAP_FONT_MAX_OPENED_FONT

#ifdef CONFIG_BITMAP_FONT_SUPPORT_EMOJI
#define MAX_EMOJI_FONT						1
#define MAX_EMOJI_NUM						100
#else
#define MAX_EMOJI_FONT						0
#define MAX_EMOJI_NUM						0
#endif  //CONFIG_BITMAP_FONT_SUPPORT_EMOJI

//#define BITMAP_FONT_PSRAM_SIZE				(MAX_OPEND_FONT+MAX_EMOJI_FONT)*(BITMAP_FONT_CACHE_SIZE+CMAP_CACHE_SIZE)+MAX_EMOJI_NUM*sizeof(emoji_font_entry_t)+BITMAP_FONT_HIGH_FREQ_CACHE_SIZE

#ifdef CONFIG_BITMAP_FONT_CACHE_POOL_SIZE
#define BITMAP_FONT_PSRAM_SIZE				CONFIG_BITMAP_FONT_CACHE_POOL_SIZE
#else
#define BITMAP_FONT_PSRAM_SIZE				360*1024
#endif

#define DEBUG_FONT_GLYPH					0
#define PRINT_FONT_GLYPH_ERR				0


static char __aligned(4) bmp_font_cache_buffer[BITMAP_FONT_PSRAM_SIZE];
static struct k_heap font_mem_pool = {
    .heap = {
        .init_mem = bmp_font_cache_buffer,
        .init_bytes = BITMAP_FONT_PSRAM_SIZE,
    },
};

static int font_cache_total_size;
static int font_cache_used_size;
static cache_mem_info_t* cache_mem_info;


void bitmap_font_cache_init(void)
{
    font_cache_total_size = BITMAP_FONT_PSRAM_SIZE;
    font_cache_used_size = 0;

    k_heap_init(&font_mem_pool, font_mem_pool.heap.init_mem, font_mem_pool.heap.init_bytes);
}

static void _add_cache_mem_info(void* ptr, uint32_t size)
{
    cache_mem_info_t* item=NULL;

    item = mem_malloc(sizeof(cache_mem_info_t));
    if(item == NULL)
    {
        SYS_LOG_INF("cache mem info malloc failed");
        return;
    }

    item->ptr = ptr;
    item->size = size;
    item->next = NULL;
    if(cache_mem_info==NULL)
    {
        cache_mem_info = item;
    }
    else
    {
        cache_mem_info_t* pos = cache_mem_info;
        cache_mem_info_t* prev = cache_mem_info;
        while(pos)
        {
            prev = pos;
            pos = pos->next;
        }
        prev->next = item;
    }

    font_cache_used_size += size;

    SYS_LOG_INF("font cache mem add %d, total %d\n", size, font_cache_used_size);
}

static void _remove_cache_mem_info(void* ptr)
{
    cache_mem_info_t* item;
    cache_mem_info_t* prev;

    item = cache_mem_info;
    prev = NULL;
    while(item)
    {
        if(item->ptr == ptr)
        {
            break;
        }

        prev = item;
        item = item->next;
    }

    if(item == NULL)
    {
        SYS_LOG_INF("cache mem info not found %p\n", ptr);
        return;
    }

    if(prev)
    {
        prev->next = item->next;
    }

    if(item == cache_mem_info)
    {
        cache_mem_info = item->next;
    }

    font_cache_used_size -= item->size;
    SYS_LOG_INF("font cache mem remove %d, total %d\n", item->size, font_cache_used_size);
    mem_free(item);
}

void* bitmap_font_cache_malloc(uint32_t size)
{
	void *ptr;

	if(size % 4 != 0)
	{
		size = (size/4 + 1)*4;
	}

    ptr = k_heap_alloc(&font_mem_pool, size, ALLOC_NO_WAIT);
	if (ptr == NULL) {
		SYS_LOG_ERR("font cache heap alloc failed, size %d\n", size);
		return ptr;
	}

	_add_cache_mem_info(ptr, size);
	return ptr;
}

void bitmap_font_cache_free(void* ptr)
{
	k_heap_free(&font_mem_pool, ptr);
	_remove_cache_mem_info(ptr);
}

uint32_t bitmap_font_get_max_fonts_num(void)
{
	return (MAX_OPEND_FONT+MAX_EMOJI_FONT);
}

//use as default size if not configd for current font size
uint32_t bitmap_font_get_font_cache_size(void)
{
	return BITMAP_FONT_CACHE_SIZE;
}

uint32_t bitmap_font_get_max_emoji_num(void)
{
	return MAX_EMOJI_NUM;
}

uint32_t bitmap_font_get_cmap_cache_size(void)
{
	return CMAP_CACHE_SIZE;
}

void bitmap_font_cache_dump_info(void)
{
    SYS_LOG_INF("bitmap font cache info dump: total used %d, max size %d\n", font_cache_used_size, font_cache_total_size);
}

int bitmap_font_glyph_debug_is_on(void)
{
#if DEBUG_FONT_GLYPH==1
	return 1;
#else
	return 0;
#endif
}

int bitmap_font_glyph_err_print_is_on(void)
{
#if PRINT_FONT_GLYPH_ERR==1
	return 1;
#else
	return 0;
#endif
}

bool bitmap_font_get_high_freq_enabled(void)
{
#ifdef CONFIG_BITMAP_FONT_USE_HIGH_FREQ_CACHE
	return true;
#else
	return false;
#endif
}
