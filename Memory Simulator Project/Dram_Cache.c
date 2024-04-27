#include "DRAM_Cache.h"
#include "Performance.h"
#include <string.h>
#include <stdio.h>
// Define cache constants
#define CACHE_SETS 4
#define CACHE_ENTRIES_PER_SET 2
#define CACHE_LINE_SIZE 32
#define CACHE_TAG_BITS 22
#define CACHE_SET_BITS 2
#define CACHE_OFFSET_BITS 5

// Cache entry structure
typedef struct {
    int valid;
    int dirty;
    Address tag;
    CacheLine data;
    int lru;
} CacheEntry;

// Cache structure
CacheEntry cache[CACHE_SETS][CACHE_ENTRIES_PER_SET];

// Initialize the cache
void initCache() {
    for (int set = 0; set < CACHE_SETS; set++) {
    for (int entry = 0; entry < CACHE_ENTRIES_PER_SET; entry++) {
        cache[set][entry].valid = 0;
        cache[set][entry].dirty = 0;
        cache[set][entry].tag = 0;
        cache[set][entry].lru = 0;
        }
    }
}
// Read a word from the cache
int readWithCache(Address addr) {
    int tag = addr >> (CACHE_SET_BITS + CACHE_OFFSET_BITS);
    int set = (addr >> CACHE_OFFSET_BITS) & 0x3;
    int offset = addr & 0x1F;

    printf("Reading from Cache - Addr: %08X, Tag: %d, Set: %d, Offset: %d\n", addr, tag, set, offset);

    for (int entry = 0; entry < CACHE_ENTRIES_PER_SET; entry++) {
        printf("Entry %d - Valid: %d, Tag: %d, Expected Tag: %d\n", entry, cache[set][entry].valid, cache[set][entry].tag, tag);
        if (cache[set][entry].valid && cache[set][entry].tag == tag) {
            printf("Cache Hit at Entry %d\n", entry);

            perfCacheHit(addr, set, entry);
            cache[set][entry].lru = 1;
            cache[set][1 - entry].lru = 0;
            return *((int *)&cache[set][entry].data[offset]);
    }
}
    printf("Cache Miss for Addr: %08X\n", addr);

// Cache miss
    int victimEntry = cache[set][0].lru ? 0 : 1;
    perfCacheMiss(addr, set, victimEntry, cache[set][victimEntry].valid == 0);
    if (cache[set][victimEntry].dirty) {
    // Write back dirty cache line to DRAM
        Address victimAddr = (cache[set][victimEntry].tag << (CACHE_SET_BITS + CACHE_OFFSET_BITS)) | (set << CACHE_OFFSET_BITS);
        writeDramCacheLine(victimAddr, cache[set][victimEntry].data);
}
// Read cache line from DRAM
    readDramCacheLine(addr & ~0x1F, cache[set][victimEntry].data);
    cache[set][victimEntry].valid = 1;
    cache[set][victimEntry].dirty = 0;
    cache[set][victimEntry].tag = tag;
    cache[set][victimEntry].lru = 1;
    cache[set][1 - victimEntry].lru = 0;
    return *((int *)&cache[set][victimEntry].data[offset]);
}

// Write a word to the cache
void writeWithCache(Address addr, int value) {
    int tag = addr >> (CACHE_SET_BITS + CACHE_OFFSET_BITS);
    int set = (addr >> CACHE_OFFSET_BITS) & 0x3;
    int offset = addr & 0x1F;

    for (int entry = 0; entry < CACHE_ENTRIES_PER_SET; entry++) {
        if (cache[set][entry].valid && cache[set][entry].tag == tag) {

        // Cache hit
        perfCacheHit(addr, set, entry);
        *((int *)&cache[set][entry].data[offset]) = value;
        cache[set][entry].dirty = 1;
        cache[set][entry].lru = 1;
        cache[set][1 - entry].lru = 0;
        return;
    }
}
// Cache miss
    int victimEntry = cache[set][0].lru ? 0 : 1;
    perfCacheMiss(addr, set, victimEntry, cache[set][victimEntry].valid == 0);
    if (cache[set][victimEntry].dirty) {
        // Write back dirty cache line to DRAM
        Address victimAddr = (cache[set][victimEntry].tag << (CACHE_SET_BITS + CACHE_OFFSET_BITS)) | (set << CACHE_OFFSET_BITS);
        writeDramCacheLine(victimAddr, cache[set][victimEntry].data);
    }
    // Read cache line from DRAM
    readDramCacheLine(addr & ~0x1F, cache[set][victimEntry].data);
    *((int *)&cache[set][victimEntry].data[offset]) = value;
    cache[set][victimEntry].valid = 1;
    cache[set][victimEntry].dirty = 1;
    cache[set][victimEntry].tag = tag;
    cache[set][victimEntry].lru = 1;
    cache[set][1 - victimEntry].lru = 0;
}
// Flush the cache
void flushCache() {
    for (int set = 0; set < CACHE_SETS; set++) {
        for (int entry = 0; entry < CACHE_ENTRIES_PER_SET; entry++) {
            if (cache[set][entry].valid && cache[set][entry].dirty) {
                // Write back dirty cache line to DRAM
                Address addr = (cache[set][entry].tag << (CACHE_SET_BITS + CACHE_OFFSET_BITS)) | (set << CACHE_OFFSET_BITS);

                writeDramCacheLine(addr, cache[set][entry].data);
                cache[set][entry].dirty = 0;
        }

            cache[set][entry].valid = 0;
        }
    }
    perfCacheFlush();
}