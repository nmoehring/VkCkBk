// Single-header library that is unpacked when compiled
// Can be included as an INTERFACE library and linked, which
// could make it clear that this is a different project

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#include "vma/vk_mem_alloc.h"
