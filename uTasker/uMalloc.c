/**********************************************************************
    Mark Butcher    Bsc (Hons) MPhil MIET

    M.J.Butcher Consulting
    Birchstrasse 20f,    CH-5406, Rütihof
    Switzerland

    www.uTasker.com    Skype: M_J_Butcher

    ---------------------------------------------------------------------
    File:      uMalloc.c
    Project:   Single Chip Embedded Internet
    ---------------------------------------------------------------------
    Copyright (C) M.J.Butcher Consulting 2004..2020
    *********************************************************************
    12.10.2007 uMallocAlign() quantity extended to 64k                   {1}
    05.05.2009 Add conditional compile on UNUSED_STACK_PATTERN           {2}
    02.02.2010 Add optional uFree()                                      {3}
    28.05.2010 Add stack monitoring support in different memory to heap  {4}
    12.12.2011 Add optional UNUSED_HEAP_PATTERN to aid debugging         {5}
    29.05.2013 Add optional second heap in fixed sized memory area       {6}
    06.11.2015 Modify fnStackFree() to allow worst-case used stack to be returned {7}
    16.12.2017 Add optional uCalloc() and uCFree()                       {8}
    10.01.2018 pucBottomOfHeap made extern                               {9}
    17.08.2019 Limit the end of heap to physical RAM space               {10}
    19.02.2020 Add i.MX RT support                                       {11}
    17.08.2020 SUPPORT_CALLOC_ALIG opion support  and routine fn_uCallocReport()

*/


#include "config.h"


#if defined _WINDOWS
    static unsigned char ucHeap[OUR_HEAP_SIZE] = {0};                    // space for heap simulation
    extern void *fnGetHeapStart(void) { return ucHeap; }
#else
    #if defined (_GNU)
        extern unsigned char _end;                                       // variable at last location in RAM (GNU linker)
    #elif defined (COMPILE_KEIL)
        extern unsigned char __libspace_start;
    #endif
#endif

#if defined SECONDARY_UMALLOC                                            // {6}
    static void fnInitSecondaryArea(void);
#endif

// Minimalist heap management without free support
//
unsigned char *pucBottomOfHeap = 0;                                      // {9}
static HEAP_REQUIREMENTS present_HeapSize = 0;
static unsigned char *pucTopOfHeap = 0;

#if defined SECONDARY_UMALLOC                                            // {6}
static unsigned char *pucBottomOfHeap2 = 0;
static unsigned char *pucTopOfHeap2 = 0;
static unsigned long present_HeapSize2 = 0;
#endif

extern void fnInitialiseHeap(const HEAP_NEEDS *ctOurHeap, void *HeapStart)
{
#if defined UNUSED_STACK_PATTERN                                         // {2}
    unsigned char *ucPattern;
    #if !defined _WINDOWS
    unsigned char ucTopOfStack;                                          // put a temporary variable on stack
    #endif
#endif
#if defined UNUSED_HEAP_PATTERN
    unsigned char *ptrHeap;
#endif

    present_HeapSize = 0;                                                // initially no heap has been allocated
    pucTopOfHeap = pucBottomOfHeap = (unsigned char *)HeapStart;         // mark bottom of heap
#if !defined APPLICATION_WITHOUT_OS
    while (ctOurHeap->ConfigNr != OurConfigNr) {
        if (ctOurHeap->ConfigNr == 0) {
            return;                                                      // end of list - node not found
        }
        ctOurHeap++;
    }
#endif
    pucTopOfHeap += ctOurHeap->need_this_amount;                         // we need the following amount of memory for heap use
#if !defined _WINDOWS
    if (pucTopOfHeap >= (unsigned char *)((RAM_START_ADDRESS + (SIZE_OF_RAM - NON_INITIALISED_RAM_SIZE)))) { // {10} limit the heap end to the physical memory area
        pucTopOfHeap = (unsigned char *)((RAM_START_ADDRESS + (SIZE_OF_RAM - NON_INITIALISED_RAM_SIZE)));
    }
#endif
#if defined UNUSED_HEAP_PATTERN
    ptrHeap = pucBottomOfHeap;
    while (ptrHeap < pucTopOfHeap) {
        *ptrHeap++ = UNUSED_HEAP_PATTERN;                                // {5} initialise heap with pattern
    }
#endif
#if defined UNUSED_STACK_PATTERN                                         // {2}
    #if defined STACK_START_ADDRESS                                      // {4}
    ucPattern = (unsigned char *)STACK_START_ADDRESS;                    // fixed stack start address
    #else
    ucPattern = pucTopOfHeap;                                            // stack is located after heap
    #endif
    #if !defined _WINDOWS                                                // we now fill a pattern from the top of stack until the top of heap
    while (ucPattern <= &ucTopOfStack) {                                 // this pattern is used later for Stack use monitoring
        *ucPattern++ = UNUSED_STACK_PATTERN;
    }
    #endif
#endif
#if defined SECONDARY_UMALLOC                                            // {6}
    fnInitSecondaryArea();                                               // perform initialisation of the secondary area with fixed locatiosn and sizes
#endif
}


extern void *uMalloc(MAX_MALLOC __size)
{
#if defined _ALIGN_HEAP_4
    unsigned char *ptr = (pucBottomOfHeap + present_HeapSize);
    if (((unsigned long)ptr & 0x3) != 0) {                               // ensure all memory is aligned on 4 byte boundary
        ptr = (unsigned char *)((unsigned long)ptr & ~0x3);
        ptr += 4;
    }
#else
    unsigned char *ptr = (pucBottomOfHeap + present_HeapSize);           // packs with maximum density for processors with no restrictions
#endif

    if ((ptr + __size) >= pucTopOfHeap) {
#if defined _WINDOWS
        _EXCEPTION("uMalloc failed!!");                                  // generate exception so that the problem can be immediately seen and solved
#endif
        return NO_MEMORY;                                                // no more room - it will be necessary to increase heap size
    }

    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
#if defined _ALIGN_HEAP_4
    present_HeapSize = ((ptr - pucBottomOfHeap) + __size);
#else
    present_HeapSize += __size;                                          // new heap size
#endif
    return ((void *)ptr);
}

#if defined _MALLOC_ALIGN
extern void *uMallocAlign(MAX_MALLOC __size, unsigned short usAlign)     // {1}
{
    unsigned char *ptr = (pucBottomOfHeap + present_HeapSize);
    if ((unsigned long)ptr & (usAlign - 1)) {                            // ensure all memory is aligned on specified byte boundary
        ptr = (unsigned char *)((unsigned long)ptr & ~(usAlign - 1));
        ptr += usAlign;
    }

    if ((ptr + __size) >= pucTopOfHeap) {
    #if defined _WINDOWS
         _EXCEPTION("uMallocAlign failed!!");                            // generate exception so that the problem can be immediately seen and cured
    #endif
        return NO_MEMORY;                                                // no more room - it will be necessary to increase heap size
    }

    #if defined RUN_LOOPS_IN_RAM
        #if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA
    if (uMemset) {                                                       // don't do this before the routine has been installed
        uMemset(ptr, 0, __size);                                         // ensure new memory chunk is always zeroed
    }
        #else
    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
        #endif
    #else
    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
    #endif

    present_HeapSize = (ptr - pucBottomOfHeap) + __size;
    return ((void *)ptr);
}
#endif

#if defined SUPPORT_UFREE                                                // {3}
// This function allows the present malloc() state to be saved and later restored, in order to free allocated memory regions for reuse
// This only supports a single region which must be on the top of the uMalloc() area - this means that it should usually only be used once all fixed
// resources have already been fully allocated
//
extern MAX_MALLOC uFree(int iFreeRegion)
{
    static MAX_MALLOC free_start = 0;                                    // invalid
    if (iFreeRegion == 0) {                                              // mark the start of a single region
        free_start = present_HeapSize;
    }
    else {                                                               // free the region
        if (free_start != 0) {
            MAX_MALLOC freed_heap = (present_HeapSize - free_start);
            present_HeapSize = free_start;                               // set the uMalloc() size back to when the free region was originally marked
            free_start = 0;                                              // invalidate
            return freed_heap;                                           // the size of heap that was freed
        }
    }
    return NO_MEMORY;
}
#endif

extern STACK_REQUIREMENTS fnStackFree(STACK_REQUIREMENTS *stackUsed)     // {7}
{
#if defined UNUSED_STACK_PATTERN && !defined _WINDOWS                    // {2}
    #if defined STACK_START_ADDRESS                                      // {4}
    unsigned char *ptrStack = (unsigned char *)STACK_START_ADDRESS;      // fixed stack start address
    #else
    unsigned char *ptrStack = pucTopOfHeap;                              // stack begins after heap
    #endif
    STACK_REQUIREMENTS FreeStack = 0;
    #if defined _iMX && defined iMX_BOOT                                 // {11}
    unsigned char ucInstructionBanks;
    unsigned char ucDataBanks;
    fnGetFlexRAM_banks(&ucInstructionBanks, &ucDataBanks);               // request the FlexRAM layout
    #endif

    while (*(ptrStack++) == UNUSED_STACK_PATTERN) {
        FreeStack++;                                                     // count free locations in RAM never used until now
    }
  //if (FreeStack != 0) {
  //    ptrStack = pucTopOfHeap;
  //}
    #if defined _iMX && defined iMX_BOOT                                 // {11}
    *stackUsed = ((unsigned char *)((RAM_START_ADDRESS + ((ucDataBanks * FLEX_RAM_GRANULARITY) - NON_INITIALISED_RAM_SIZE))) - ptrStack);
    #else
    *stackUsed = ((unsigned char *)((RAM_START_ADDRESS + (SIZE_OF_RAM - NON_INITIALISED_RAM_SIZE))) - ptrStack); // {7}
    #endif
    return (FreeStack);
#else
    *stackUsed = 0;
    return NO_MEMORY;
#endif
}

extern HEAP_REQUIREMENTS fnHeapAvailable(void)
{
    return (pucTopOfHeap - pucBottomOfHeap);                             // total heap space available
}

extern HEAP_REQUIREMENTS fnHeapFree(void)
{
    return ((pucTopOfHeap - pucBottomOfHeap) - present_HeapSize);        // total still free
}

#if defined SECONDARY_UMALLOC                                            // {6}
    #if defined _WINDOWS
static unsigned char ucHeap2[HEAP2_SIZE] = {0};                          // heap memory used by the simulator
    #endif

// The secondary uMalloc area is usually a free area of fixed sized memory - such as large external SRAM or SDRAM - that is not used for variable storage.
// An equivalent uMalloc interface is used but the user can define which dynamic variables are in which area
//
static void fnInitSecondaryArea(void)
{
    present_HeapSize2 = 0;                                               // initially no heap has been allocated
    pucBottomOfHeap2 = (unsigned char *)HEAP2_ADDRESS;                   // mark bottom of heap
    pucTopOfHeap2 = (pucBottomOfHeap2 + HEAP2_SIZE);                     // mark top of heap
}

extern void *uMalloc2(unsigned long __size)
{
    #if defined _ALIGN_HEAP_4
        #if defined _WINDOWS
    unsigned char *_ptr;
        #endif
    unsigned char *ptr = (pucBottomOfHeap2 + present_HeapSize2);
    if ((unsigned long)ptr & 0x3) {                                      // ensure all memory is aligned on 4 byte boundary
        ptr = (unsigned char *)((unsigned long)ptr & ~0x3);
        ptr += 4;
    }
    #else
    unsigned char *ptr = (pucBottomOfHeap2 + present_HeapSize2);         // packs with maximum density for processors with no restrictions
    #endif

    if ((ptr + __size) >= pucTopOfHeap2) {
    #if defined _WINDOWS
        _EXCEPTION("uMalloc2 failed!!");                                 // generate exception so that the problem can be immediately seen and cured
    #endif
        return NO_MEMORY;                                                // no more room - it will be necessary to increase heap size
    }
    #if defined _WINDOWS
        #if defined _ALIGN_HEAP_4
    _ptr = ptr;                                                          // backup original pointer
        #endif
    ptr = (ucHeap2 + (ptr - (unsigned char *)HEAP2_ADDRESS));            // move to simulated heap space
    #endif

    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
    #if defined _ALIGN_HEAP_4
        #if defined _WINDOWS
    present_HeapSize2 = (_ptr - pucBottomOfHeap2) + __size;
        #else
    present_HeapSize2 = (ptr - pucBottomOfHeap2) + __size;
        #endif
    #else
    present_HeapSize2 += __size;                                         // new heap size
    #endif
    return ((void *)ptr);
}

    #if defined _MALLOC_ALIGN
extern void *uMallocAlign2(unsigned long __size, unsigned short usAlign)
{
        #if defined _WINDOWS
    unsigned char *_ptr;
        #endif
    unsigned char *ptr = (pucBottomOfHeap2 + present_HeapSize2);
    if ((unsigned long)ptr & (usAlign - 1)) {                            // ensure all memory is aligned on specified byte boundary
        ptr = (unsigned char *)((unsigned long)ptr & ~(usAlign - 1));
        ptr += usAlign;
    }

    if ((ptr + __size) >= pucTopOfHeap2) {
        #if defined _WINDOWS
         _EXCEPTION("uMallocAlign2 failed!!");                           // generate exception so that the problem can be immediately seen and cured
        #endif
        return NO_MEMORY;                                                // no more room - it will be necessary to increase heap size
    }

        #if defined _WINDOWS
    _ptr = ptr;                                                          // backup original pointer
    ptr = (ucHeap2 + (ptr - (unsigned char *)HEAP2_ADDRESS));            // move to simulated heap space
        #endif

        #if defined RUN_LOOPS_IN_RAM
            #if !defined DMA_MEMCPY_SET || defined DEVICE_WITHOUT_DMA
    if (uMemset != 0) {                                                  // don't do this before the routine has been installed
        uMemset(ptr, 0, __size);                                         // ensure new memory chunk is always zeroed
    }
            #else
    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
            #endif
        #else
    uMemset(ptr, 0, __size);                                             // ensure new memory chunk is always zeroed
        #endif

        #if defined _WINDOWS
    present_HeapSize2 = (_ptr - pucBottomOfHeap2) + __size;
        #else
    present_HeapSize2 = (ptr - pucBottomOfHeap2) + __size;
        #endif
    return ((void *)ptr);
}
    #endif

    #if defined SUPPORT_UFREE
extern unsigned long uFree2(int iFreeRegion)
{
    static unsigned long free_start2 = 0;                                // invalid
    if (iFreeRegion == 0) {                                              // mark the start of a single region
        free_start2 = present_HeapSize2;
    }
    else {                                                               // free the region
        if (free_start2 != 0) {
            unsigned long freed_heap = (present_HeapSize2 - free_start2);
            present_HeapSize2 = free_start2;                             // set the uMalloc2() size back to when the free region was originally marked
            free_start2 = 0;                                             // invalidate
            return freed_heap;                                           // the size of heap that was freed
        }
    }
    return NO_MEMORY;
}
    #endif
#endif

#if defined SUPPORT_UCALLOC                                              // {8}
#if !defined UCALLOC_HEAP_SIZE
    #define UCALLOC_HEAP_SIZE        (24 * 1024)                         // default uCalloc heap size suitable for TLS handshake
    #define BREAK_DOWN_BLOCKS_LIMIT  (4 * 1024)                          // break down holes of this size and larger by reallocating them in preference to filling smallest possibly hole
    #define HEAP_OBJECTS             200
#endif
#define MONITOR_PEAK_MANAGEMENT_BLOCK_USE                                // check the maximum block use during operation

typedef struct stHEAP_MANAGEMENT_BLOCK
{
    unsigned char *ptrMemory;                                            // pointer to the allocated memory
    MAX_MALLOC memorySize;                                               // the size of the allocated memory
    unsigned char ucStatus;
    #if defined SUPPORT_CALLOC_ALIGN
    unsigned char ucOffset;                                              // alignment offset
    #endif
} HEAP_MANAGEMENT_BLOCK;

static unsigned long ulMemoryAllocated = 0;                              // present size of allocated hep memory
static unsigned long ulTopMargin = 0xffffffff;                           // the worst case of heap margin detected
static unsigned long ulHoleCount = 0;                                    // the number of holes that presently exist
static unsigned long ulAllocateCount = 0;                                // the number of times memory allocation has been requested
static unsigned long ulDeallocateCount = 0;                              // the number of times memory has been freed
static unsigned long ulAccumulatedHoleSize = 0;                          // the accumulated hole memory size
#if defined MONITOR_PEAK_MANAGEMENT_BLOCK_USE
    static unsigned long ulMaxUsedManagementBlocks = 0;                  // the maximum utilisation of management blocks detected
#endif

static unsigned char *ptrSecurityHeapTop = 0;                            // present top of allocated heap memory
static HEAP_MANAGEMENT_BLOCK *ptrHeapManager = 0;                        // the location of the heap management entries

#if defined SUPPORT_CALLOC_ALIGN
static int iAlignMemory = 0;
#endif

extern void fn_uCallocReport(void)
{
    fnDebugMsg("\r\ncuCalloc:\r\n");
    fnDebugMsg("Size:");
    fnDebugHex(UCALLOC_HEAP_SIZE, (WITH_SPACE | WITH_LEADIN | sizeof(unsigned long) | WITH_CR_LF));
    fnDebugMsg("Max. objects:");
    fnDebugDec(HEAP_OBJECTS, (WITH_SPACE | WITH_CR_LF));
    fnDebugMsg("Hole size limit:");
    fnDebugHex(BREAK_DOWN_BLOCKS_LIMIT, (WITH_SPACE | WITH_LEADIN | sizeof(unsigned short) | WITH_CR_LF));
    fnDebugMsg("Allocated memory:");
    fnDebugHex(ulMemoryAllocated, (WITH_SPACE | WITH_LEADIN | sizeof(ulMemoryAllocated)));
    fnDebugMsg(" bytes with");
    fnDebugDec(ulHoleCount, (WITH_SPACE));
    fnDebugMsg(" holes");
    if (ulHoleCount != 0) {
        fnDebugMsg(" - Ac cumulated hole size:");
        fnDebugHex(ulAccumulatedHoleSize, (WITH_SPACE | WITH_LEADIN | sizeof(ulAccumulatedHoleSize)));
        fnDebugMsg(" Bytes");
    }
    fnDebugMsg("\r\n");
    fnDebugMsg("Allocs:");
    fnDebugDec(ulAllocateCount, (WITH_SPACE));
    fnDebugMsg(" / De-Allocs:");
    fnDebugDec(ulDeallocateCount, (WITH_SPACE | WITH_CR_LF));
    if (ulTopMargin != 0xffffffff) {
        fnDebugMsg("Memory margin:");
        fnDebugHex(ulTopMargin, (WITH_SPACE | WITH_LEADIN | sizeof(ulTopMargin) | WITH_CR_LF));
#if defined MONITOR_PEAK_MANAGEMENT_BLOCK_USE
        fnDebugMsg("Max. used objects:");
        fnDebugDec(ulMaxUsedManagementBlocks, (WITH_SPACE | WITH_CR_LF));
#endif
    }
}

// Return always zeroed memory that is long word aligned
//
extern void *uCalloc(size_t n, size_t size)
{
    static unsigned char *ptrSecurityHeapStart = 0;
    static unsigned char *ptrSecurityHeapEnd = 0;
    register HEAP_MANAGEMENT_BLOCK *ptrHeapEntry;
    int iEntry = 0;
    MAX_MALLOC thisSize = (MAX_MALLOC)(n * size);                        // the size of zeroed memory requested
    thisSize += 3;
    thisSize &= ~0x3;                                                    // always long word size
    if (ptrHeapManager == 0) {                                           // on first utilisation
        ptrHeapManager = uMalloc((HEAP_OBJECTS * sizeof(HEAP_MANAGEMENT_BLOCK))); // allocate management memory in uMalloc() heap
        ptrSecurityHeapStart = ptrSecurityHeapTop = uMalloc(UCALLOC_HEAP_SIZE); // allocate uCalloc() memory in uMalloc() heap
        if ((ptrHeapManager == 0) || (ptrSecurityHeapTop == 0)) {
            _EXCEPTION("No Heap for uCalloc!!");
            return NO_MEMORY;
        }
        ptrSecurityHeapEnd = (ptrSecurityHeapStart + UCALLOC_HEAP_SIZE);
    }
    ptrHeapEntry = ptrHeapManager;
    ulAllocateCount++;
    FOREVER_LOOP() {
        if (ptrHeapEntry->ptrMemory == 0) {                              // free entry found (neither in use nor a hole)
            // If we can find a hole that can be re-used we do this as preference, rather than letting the heap grow unnecessarily and not being able to accept a large block later
            //
            register HEAP_MANAGEMENT_BLOCK *ptrReuseEntry = ptrHeapManager;
            register HEAP_MANAGEMENT_BLOCK *ptrPreferredHole = 0;
            register unsigned long ulSpaceOnTopOfStack;
            register unsigned long ulHolesToCheck = ulHoleCount;
#if defined MONITOR_PEAK_MANAGEMENT_BLOCK_USE
            unsigned long ulUsedManagementBlockCount = ++iEntry;
            ptrPreferredHole = ptrHeapEntry;
            while (iEntry++ < HEAP_OBJECTS) {
                ptrPreferredHole++;
                if (ptrHeapEntry->ptrMemory != 0) {                      // either in use or a hole
                    ulUsedManagementBlockCount++;
                }
            }
            if (ulMaxUsedManagementBlocks < ulUsedManagementBlockCount) {
                ulMaxUsedManagementBlocks = ulUsedManagementBlockCount;  // the peak in use count seen
            }
            ptrPreferredHole = 0;
#endif
            iEntry = 0;
            while (ulHolesToCheck != 0) {                                // we scan for holes
                if (ptrReuseEntry->ucStatus != 0) {                      // this is a hole
                    ulHolesToCheck--;
                    if (ptrReuseEntry->memorySize == thisSize) {         // this hole has the perfect size for us
                        ptrReuseEntry->ucStatus = 0;                     // the hole is in use again (with original size)
                        ulHoleCount--;
                        ulAccumulatedHoleSize -= thisSize;
                        ulMemoryAllocated += thisSize;
                        return (void *)(ptrReuseEntry->ptrMemory);
                    }
                    else if (ptrReuseEntry->memorySize > thisSize) {     // this hole has adequate size for us
                        if (ptrPreferredHole == 0) {                     // the first hole that we have found
                            ptrPreferredHole = ptrReuseEntry;            // presently it is the one that will be re-used
                        }
                        else {
                            if ((ptrReuseEntry->memorySize >= BREAK_DOWN_BLOCKS_LIMIT) || (ptrPreferredHole->memorySize >= BREAK_DOWN_BLOCKS_LIMIT)) { // if this hole is large
                                if (ptrPreferredHole->memorySize < ptrReuseEntry->memorySize) { // we prefer to break down larger blocks rather than fill smaller ones
                                    ptrPreferredHole = ptrReuseEntry;    // larger hole has been found which we will re-use instead
                                }
                            }
                            else {
                                if (ptrPreferredHole->memorySize > ptrReuseEntry->memorySize) { // we prefer to use the smallest that fits
                                    ptrPreferredHole = ptrReuseEntry;    // smaller hole has been found which we will re-use instead
                                }
                            }
                        }
                    }
                }
                if (++iEntry >= HEAP_OBJECTS) {                          // complete list checked
                    break;
                }
                ptrReuseEntry++;                                         // move to next entry
            }
            if (ptrPreferredHole != 0) {                                 // we reuse a part of a hole
                ptrHeapEntry->memorySize = thisSize;                     // the entry size
                ptrHeapEntry->ptrMemory = ptrPreferredHole->ptrMemory;   // insert at start of hole
                ptrPreferredHole->memorySize -= thisSize;                // the original hole is now smaller but is still a hole
                ptrPreferredHole->ptrMemory += thisSize;
                ulAccumulatedHoleSize -= thisSize;
                ulMemoryAllocated += thisSize;
                break;
            }
            // No memory re-use was possible so it must be put to the top of heap
            //
            ulSpaceOnTopOfStack = (ptrSecurityHeapEnd - ptrSecurityHeapTop); // the remaining memory at the top of the heap
            if (thisSize > ulSpaceOnTopOfStack) {
                _EXCEPTION("Heap exhausted!!");
                return NO_MEMORY;
            }
            if (ulTopMargin > ulSpaceOnTopOfStack) {
                ulTopMargin = ulSpaceOnTopOfStack;
            }
            ptrHeapEntry->ptrMemory = ptrSecurityHeapTop;
            ptrHeapEntry->memorySize = thisSize;
            ptrSecurityHeapTop += thisSize;
            ulMemoryAllocated += thisSize;
            break;
        }
        if (++iEntry >= HEAP_OBJECTS) {
            _EXCEPTION("Not enough heap management objects!");
            return NO_MEMORY;
        }
        ptrHeapEntry++;
    }
#if defined SUPPORT_CALLOC_ALIGN
    if (iAlignMemory != 0) {
        return (void *)(ptrHeapEntry);
    }
#endif
    return (void *)(ptrHeapEntry->ptrMemory);
}

extern void *_uCalloc(MAX_MALLOC size)
{
    return uCalloc(1, (size_t)size);
}

#if defined SUPPORT_CALLOC_ALIGN
extern void *uCallocAlign(MAX_MALLOC __size, unsigned short usAlign)
{
    HEAP_MANAGEMENT_BLOCK *ptrHeapEntry;
    unsigned long ulMemory;
    iAlignMemory = 1;
        ptrHeapEntry = (HEAP_MANAGEMENT_BLOCK *)uCalloc(1, (size_t)(__size + usAlign - 1)); // allocate more memory to be sure to be able to align it
    ulMemory = (unsigned long)ptrHeapEntry->ptrMemory;
    iAlignMemory = 0;
    if ((ulMemory & (usAlign - 1)) != 0) {                               // ensure all memory is aligned on specified byte boundary
        unsigned long ulAlignedMemory = (ulMemory & ~(usAlign - 1));
        ulAlignedMemory += usAlign;
        ptrHeapEntry->ucOffset = (unsigned char)(ulAlignedMemory - ulMemory);
        return (void *)ulAlignedMemory;
    }
    return (void *)ulMemory;
}
#endif

extern void uCFree(void *ptr)
{
    if (ptr == 0) {
      //_EXCEPTION("Ignoring free of zero pointer!");
        return;
    }
    if (ptrHeapManager != 0) {
        register int iEntry = 0;
        register HEAP_MANAGEMENT_BLOCK *ptrHeapEntry = ptrHeapManager;
        ulDeallocateCount++;
        FOREVER_LOOP() {
#if defined SUPPORT_CALLOC_ALIGN
            if ((ptrHeapEntry->ptrMemory + ptrHeapEntry->ucOffset) == ptr)
#else
            if (ptrHeapEntry->ptrMemory == ptr)
#endif
            {                                                            // entry found
                register HEAP_MANAGEMENT_BLOCK *ptrHeapScan = ptrHeapManager;
                register HEAP_MANAGEMENT_BLOCK *ptrHeapEntryBefore = 0;
                register HEAP_MANAGEMENT_BLOCK *ptrHeapEntryAfter = 0;
                void *ptrMemoryBefore = ptrHeapEntry->ptrMemory;
                void *ptrMemoryAfter = (ptrHeapEntry->ptrMemory + ptrHeapEntry->memorySize);
#if defined _WINDOWS
                if (ptrHeapEntry->ucStatus != 0) {
                    _EXCEPTION("Freeing a pointer that has already been freed!!");
                    return;
                }
#endif
                uMemset(ptrHeapEntry->ptrMemory, 0, ptrHeapEntry->memorySize); // zero freed memory (so that subsequent allocation doesn't need to do it and also to cover up previous use in security applications)
                ulMemoryAllocated -= ptrHeapEntry->memorySize;
#if defined SUPPORT_CALLOC_ALIGN
                ptrHeapEntry->ucOffset = 0;
#endif
                ptrHeapEntry->ucStatus = 1;                              // this is now a hole
                ulHoleCount++;
                ulAccumulatedHoleSize += ptrHeapEntry->memorySize;
                iEntry = 0;
                FOREVER_LOOP() {
                    if (ptrHeapScan->ucStatus != 0) {                    // for each hole found
                        if (ptrHeapScan->ptrMemory == ptrMemoryAfter) {
                            ptrHeapEntryAfter = ptrHeapScan;             // found following hole
                            if (ptrHeapEntryBefore != 0) {
                                break;                                   // since we have found both a preceeding and following hole we can stop a continued search
                            }
                        }
                        else if ((ptrHeapScan->ptrMemory + ptrHeapScan->memorySize) == ptrMemoryBefore) {
                            ptrHeapEntryBefore = ptrHeapScan;            // found preceeding hole
                            if (ptrHeapEntryAfter != 0) {
                                break;                                   // since we have found both a preceeding and following hole we can stop a continued search
                            }
                        }
                    }
                    if (++iEntry >= HEAP_OBJECTS) {
                        break;                                           // all blocks have been scanned for possible combination
                    }
                    ptrHeapScan++;
                }
                if (ptrHeapEntryBefore != 0) {                           // if a hole was found before the freed chunk
                    ptrHeapEntryBefore->memorySize += ptrHeapEntry->memorySize; // combine previous hole with ours
                    ptrHeapEntry->ptrMemory = 0;
                    ptrHeapEntry->ucStatus = 0;                          // this entry is free
                    ptrHeapEntry = ptrHeapEntryBefore;
                    ulHoleCount--;
                }
                if (ptrHeapEntryAfter != 0) {                            // if a hole was found after the freed chunk
                    ptrHeapEntry->memorySize += ptrHeapEntryAfter->memorySize; // combine with following hole
                    ptrHeapEntryAfter->ptrMemory = 0;
                    ptrHeapEntryAfter->ucStatus = 0;                     // this entry is free           
                    ulHoleCount--;
                }
                if ((ptrHeapEntry->ptrMemory + ptrHeapEntry->memorySize) == ptrSecurityHeapTop) { // being removed from the top of heap
                    ulAccumulatedHoleSize -= ptrHeapEntry->memorySize;
                    ptrSecurityHeapTop = ptrHeapEntry->ptrMemory;        // remove the freed memory from the top of heap
                    ulHoleCount--;                                       // it is no longer a hole
                    ptrHeapEntry->ucStatus = 0;
                    ptrHeapEntry->ptrMemory = 0;
                }
                break;
            }
            if (++iEntry >= HEAP_OBJECTS) {
                _EXCEPTION("Trying to free non-allocated memory!");
                break;
            }
            ptrHeapEntry++;
        }
    }
}
#endif
