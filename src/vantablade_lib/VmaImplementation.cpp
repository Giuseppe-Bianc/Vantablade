// src/vantablade_lib/VmaImplementation.cpp
// NOLINTBEGIN(*-include-cleaner, *-macro-usage)
#include "Vantablade/headers.hpp"

DISABLE_WARNINGS_PUSH(4100 4127 4189 4201 4324 4505 4820 26812)

#define VMA_IMPLEMENTATION

// Se vuoi caricamento dinamico, dichiaralo in modo non ambiguo.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

// Compile-time target Vulkan per VMA (A BBB CCC).
#define VMA_VULKAN_VERSION 1004000

#ifndef NDEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY 1
#endif

#include <vk_mem_alloc.h>

DISABLE_WARNINGS_POP()

// NOLINTEND(*-include-cleaner, *-macro-usage)