#include "DRAM.h"
#include "Performance.h"
#include <string.h>


// Define constants for memory sizes
#define TOTAL_MEMORY_SIZE (48 * 1024) // Total DRAM size in bytes (48KB)
#define WORD_SIZE 4 // Size of a word in bytes
#define CACHE_LINE_SIZE 32 // Size of a cache line in bytes

// Declare integer DRAM array to simulate memory
int intDram[TOTAL_MEMORY_SIZE / WORD_SIZE];

// Function to read a word from DRAM at the specified address
int readDram(Address addr) {
    int result;

    // Use memcpy to copy the word from DRAM to the result variable
    memcpy(&result, &intDram[addr / WORD_SIZE], WORD_SIZE);

    // Update performance metrics for DRAM read operation
    perfDramRead(addr, result);
    // Return the read value
    return result;
}

// Function to write a word to DRAM at the specified address
void writeDram(Address addr, int value) {
    // Use memcpy to copy the value to the specified address in DRAM
    memcpy(&intDram[addr / WORD_SIZE], &value, WORD_SIZE);
    // Update performance metrics for DRAM write operation
    perfDramWrite(addr, value);
}

// Function to read a cache line from DRAM at the specified address
void readDramCacheLine(Address addr, CacheLine line) {

    // Use memcpy to copy the cache line from DRAM to the provided buffer
    memcpy(line, &intDram[addr / WORD_SIZE], CACHE_LINE_SIZE);
    // Update performance metrics for DRAM cache line read operation
    perfDramCacheLineRead(addr, line);
}
// Function to write a cache line to DRAM at the specified address
void writeDramCacheLine(Address addr, CacheLine line) {
   
    // Use memcpy to copy the cache line data to the specified address in DRAM
    memcpy(&intDram[addr / WORD_SIZE], line, CACHE_LINE_SIZE);
    // Update performance metrics for DRAM cache line write operation
    perfDramCacheLineWrite(addr, line);
}