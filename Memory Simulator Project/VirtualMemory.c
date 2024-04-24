#include <stddef.h> // Include for NULL definition
#include "VirtualMemory.h"
#include "Performance.h"

#define PAGE_SIZE 1024
#define NUM_PAGES 64
#define TLB_SIZE 2
#define PAGE_TABLE_SIZE 256 // Total size of page table in bytes
#define BYTES_PER_PTE 4

static PageTable* pageTablePtr = NULL;
static int vmEnabled = 0; // Virtual memory initially disabled

// TLB entries
static int tlb[TLB_SIZE];
static int tlbIndex = 0;

// Helper function to perform TLB lookup
static int tlbLookup(int pageNumber) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i] == pageNumber) {
            // TLB hit
            perfTlbHit(pageNumber);
            return 1;
        }
    }
    // TLB miss
    perfTlbMiss(pageNumber);
    return 0;
}

// Helper function to update TLB
static void updateTLB(int pageNumber) {
    tlb[tlbIndex] = pageNumber;
    tlbIndex = (tlbIndex + 1) % TLB_SIZE; // Implementing round robin replacement strategy
}

// Helper function to translate virtual address to physical address
static int translateAddress(Address addr) {
    if (vmEnabled && pageTablePtr != NULL) {
        int pageNumber = addr / PAGE_SIZE;

        // Check TLB for the page number
        if (tlbLookup(pageNumber)) {
            // TLB hit
            int physicalPageNumber = (*pageTablePtr)[pageNumber].physicalPageNumber;
            return (physicalPageNumber * PAGE_SIZE) + (addr % PAGE_SIZE);
        } else {
            // TLB miss, perform full translation
            if ((*pageTablePtr)[pageNumber].valid) {
                int physicalPageNumber = (*pageTablePtr)[pageNumber].physicalPageNumber;
                // Update TLB
                updateTLB(pageNumber);
                return (physicalPageNumber * PAGE_SIZE) + (addr % PAGE_SIZE);
            } else {
                // Page is not valid in virtual memory
                return -1;
            }
        }
    } else {
        // If virtual memory is disabled or page table not initialized, no translation needed
        return addr;
    }
}

int vmRead(Address addr) {
    int physicalAddr = translateAddress(addr);
    if (physicalAddr != -1) {
        // Report address translation
        perfStartAddressTranslation(addr);
        perfEndAddressTranslation(physicalAddr);
        // Perform read operation on physical address
        // Assuming DRAMRead is declared and defined somewhere
        return readDram(physicalAddr);
    } else {
        // Handle case where page is not valid in virtual memory
        return -1;
    }
}

void vmWrite(Address addr, int value) {
    int physicalAddr = translateAddress(addr);
    if (physicalAddr != -1) {
        // Report address translation
        perfStartAddressTranslation(addr);
        perfEndAddressTranslation(physicalAddr);
        // Perform write operation on physical address
        // Assuming DRAMWrite is declared and defined somewhere
        writeDram(physicalAddr, value);
    } else {
        // Handle case where page is not valid in virtual memory
    }
}

void vmDisable() {
    vmEnabled = 0;
}

void vmEnable(Address pageTable) {


    pageTablePtr = (PageTable*)&pageTable;
    vmEnabled = 1;
}