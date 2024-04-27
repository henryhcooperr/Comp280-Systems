#include <stdio.h>
#include "VirtualMemory.h"
#include "Performance.h"
#include "DRAM_Cache.h"

// TLB table
#define TLB_SIZE 2
struct PageTableEntry tlb[TLB_SIZE];
int tlbRoundRobin = 0;

// vmEnabled flag
int vmEnabled = 0;

// Page table address
Address pageTableAddress;

// Function declarations
int lookupTLB(int pageNumber);
int loadPageTableEntry(int pageNumber);

int vmRead(Address addr) {
    if (!vmEnabled) {
        return readWithCache(addr);
    }

    perfStartAddressTranslation(addr);

    int pageNumber = addr >> 10;
    int offset = addr & 0x3FF;

    int tlbIndex = lookupTLB(pageNumber);
    if (tlbIndex == -1) {
        perfTlbMiss(pageNumber);
        tlbIndex = loadPageTableEntry(pageNumber);
    } else {
        perfTlbHit(pageNumber);
    }

    struct PageTableEntry pte = tlb[tlbIndex];
    if (!pte.valid) {
        // Page fault
        return -1;
    }

    Address physicalAddress = (pte.physicalPageNumber << 10) | offset;
    perfEndAddressTranslation(physicalAddress);

    return readWithCache(physicalAddress);
}

void vmWrite(Address addr, int value) {
    if (!vmEnabled) {
        writeWithCache(addr, value);
        return;
    }

    perfStartAddressTranslation(addr);

    int pageNumber = addr >> 10;
    int offset = addr & 0x3FF;

    int tlbIndex = lookupTLB(pageNumber);
    if (tlbIndex == -1) {
        perfTlbMiss(pageNumber);
        tlbIndex = loadPageTableEntry(pageNumber);
    } else {
        perfTlbHit(pageNumber);
    }

    struct PageTableEntry pte = tlb[tlbIndex];
    if (!pte.valid) {
        // Page fault
        return;
    }

    Address physicalAddress = (pte.physicalPageNumber << 10) | offset;
    perfEndAddressTranslation(physicalAddress);

    writeWithCache(physicalAddress, value);
}

void vmDisable() {
    vmEnabled = 0;
}

void vmEnable(Address pageTable) {
    pageTableAddress = pageTable;
    vmEnabled = 1;
}

int lookupTLB(int pageNumber) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].physicalPageNumber == pageNumber) {
            perfTlbHit(pageNumber);
            return i;
        }
    }
    perfTlbMiss(pageNumber);
    return -1;
}

int loadPageTableEntry(int pageNumber) {
    Address pteAddress = pageTableAddress + pageNumber * BYTES_PER_PTE;
    struct PageTableEntry pte;
    int pteData = readWithCache(pteAddress);

    pte.physicalPageNumber = pteData & 0x3F;
    pte.valid = (pteData >> 6) & 1;

    int tlbIndex = tlbRoundRobin;
    tlb[tlbIndex] = pte;
    tlbRoundRobin = (tlbRoundRobin + 1) % TLB_SIZE;

    return tlbIndex;
}
