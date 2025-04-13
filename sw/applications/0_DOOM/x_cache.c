#include "x_spi.h"
#include "x_cache.h"

/****************************************************************************/
/**                                                                        **/
/**                       TYPEDEFS AND STRUCTURES                          **/
/**                                                                        **/
/****************************************************************************/

//Might need to increase this if i have many cache entries  
#define HASH_TABLE_SIZE 16

// Cache entry structure, with pointers for a doubly-linked LRU list.
typedef struct cache_entry {
    uintptr_t key;              // Unique identifier 
    void *data;             // Pointer to the cached data (allocated dynamically)
    size_t size;            // Size in bytes of the cached data
    struct cache_entry *next;  // Next entry in hash bucket (separate chaining)
    // For LRU list:
    struct cache_entry *lru_prev;
    struct cache_entry *lru_next;
} cache_entry_t;

// The overall cache structure:
typedef struct {
    cache_entry_t *buckets[HASH_TABLE_SIZE];  // Hash table buckets
    size_t cache_size;      // Maximum allowed cache size (bytes)
    size_t used;            // Current bytes used in the cache
    // LRU list pointers:
    cache_entry_t *lru_head; // Most recently used entry
    cache_entry_t *lru_tail; // Least recently used entry
} cache_t;

/****************************************************************************/
/**                                                                        **/
/*                            GLOBAL VARIABLES                              */
/**                                                                        **/
/****************************************************************************/

cache_t my_cache;

/****************************************************************************/
/**                                                                        **/
/*                      PROTOTYPES OF LOCAL FUNCTIONS                       */
/**                                                                        **/
/****************************************************************************/

static unsigned int hash_function_addr(uintptr_t addr); 

// Remove an entry from the LRU list.
static void lru_remove(cache_t *cache, cache_entry_t *entry); 

// Insert an entry at the head (most recently used) of the LRU list.
static void lru_insert_head(cache_t *cache, cache_entry_t *entry); 

// Move an entry to the head of the LRU list.
static void lru_move_to_head(cache_t *cache, cache_entry_t *entry); 

// Lookup a cache entry by key.
// If found, move the entry to the head of the LRU list and return the data.
void *cache_get(cache_t *cache, uintptr_t addr); 

// Evict entries from the cache (starting from the LRU tail) until at least "needed" bytes are free.
static void cache_evict(cache_t *cache, size_t needed); 

// Insert a new entry into the cache.
// On success, the entry is added to the hash table and LRU list.
int cache_put(cache_t *cache, uintptr_t addr, void *data, size_t size); 


/****************************************************************************/
/**                                                                        **/
/**                          EXPORTED FUNCTIONS                            **/
/**                                                                        **/
/****************************************************************************/

// Initialize the cache.
void cache_init(size_t cache_size) {
    memset(my_cache.buckets, 0, sizeof(my_cache.buckets));
    my_cache.cache_size = cache_size;
    my_cache.used = 0;
    my_cache.lru_head = NULL;
    my_cache.lru_tail = NULL;
}

// Free all cache entries.
void cache_free() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        cache_entry_t *entry = my_cache.buckets[i];
        while (entry) {
            cache_entry_t *next = entry->next;
            free(entry->key);
            free(entry->data);
            free(entry);
            entry = next;
        }
        my_cache.buckets[i] = NULL;
    }
    my_cache.lru_head = NULL;
    my_cache.lru_tail = NULL;
    my_cache.used = 0;
}

// Reads `len` bytes from the flash address using the cache.
// If the data is not cached, it fetches from flash, inserts into cache, and returns it.
void *X_cache_read(uint32_t flash_addr, uint32_t len)
{
    void *cached_data = cache_get(&my_cache, flash_addr);
    if (cached_data == NULL)
    {   
        if (len <= my_cache.cache_size && len != 0)
        { 
            void *buffer = malloc(len);
            if (len%4 == 0)
            {
                X_spi_read(flash_addr, buffer, len/4);        
            }
            else
            {
                uint32_t size = (len + 3)/4; 
                uint8_t raw_buffer[size];
                X_spi_read(flash_addr, &raw_buffer, size); 
                memcpy(buffer, &raw_buffer, len); 
                cache_put(&my_cache, flash_addr, buffer, len); 
            }
            if (cache_put(&my_cache, flash_addr, buffer, len) != 0)
            {
                //printf("Failed to insert data into cache.\n");
                free(buffer);
                return NULL;
            }
            return buffer; 
        }
        else
        {
            //printf("Data length bigger than the cache size, failed to insert data into cache\n");
            return NULL;
        }
    }
    return cached_data;
}

/****************************************************************************/
/**                                                                        **/
/*                            LOCAL FUNCTIONS                               */
/**                                                                        **/
/****************************************************************************/

static unsigned int hash_function_addr(uintptr_t addr) {
    // Simple hashing for pointer/address
    return (addr >> 4) ^ (addr >> 12);
}

static void lru_remove(cache_t *cache, cache_entry_t *entry) {
    if (entry->lru_prev)
        entry->lru_prev->lru_next = entry->lru_next;
    else
        cache->lru_head = entry->lru_next;

    if (entry->lru_next)
        entry->lru_next->lru_prev = entry->lru_prev;
    else
        cache->lru_tail = entry->lru_prev;

    entry->lru_prev = entry->lru_next = NULL;
}

static void lru_insert_head(cache_t *cache, cache_entry_t *entry) {
    entry->lru_prev = NULL;
    entry->lru_next = cache->lru_head;
    if (cache->lru_head)
        cache->lru_head->lru_prev = entry;
    cache->lru_head = entry;
    if (cache->lru_tail == NULL)
        cache->lru_tail = entry;
}

static void lru_move_to_head(cache_t *cache, cache_entry_t *entry) {
    if (cache->lru_head == entry)
        return;
    lru_remove(cache, entry);
    lru_insert_head(cache, entry);
}

void *cache_get(cache_t *cache, uintptr_t addr) {
    unsigned int hash = hash_function_addr(addr) % HASH_TABLE_SIZE;
    cache_entry_t *entry = cache->buckets[hash];
    while (entry) {
        if (entry->key == addr) {
            lru_move_to_head(cache, entry);
            return entry->data;
        }
        entry = entry->next;
    }
    return NULL;
}

static void cache_evict(cache_t *cache, size_t needed) {
    while (cache->used + needed > cache->cache_size && cache->lru_tail) {
        cache_entry_t *evict = cache->lru_tail;
        unsigned int hash = hash_function_addr(evict->key) % HASH_TABLE_SIZE;
        cache_entry_t **prev = &cache->buckets[hash];
        while (*prev && *prev != evict) {
            prev = &((*prev)->next);
        }
        if (*prev == evict) {
            *prev = evict->next;
        }
        
        lru_remove(cache, evict);
        
        cache->used -= evict->size;
    
        free(evict->data);
        free(evict);
    }
}

int cache_put(cache_t *cache, uintptr_t addr, void *data, size_t size) {
    cache_evict(cache, size);
    if (cache->used + size > cache->cache_size)
        return -1;

    unsigned int hash = hash_function_addr(addr) % HASH_TABLE_SIZE;
    cache_entry_t *entry = malloc(sizeof(cache_entry_t));
    if (!entry)
        return -1;

    entry->key = addr;
    entry->data = data;
    entry->size = size;
    entry->next = cache->buckets[hash];
    cache->buckets[hash] = entry;

    lru_insert_head(cache, entry);
    cache->used += size;
    return 0;
}


