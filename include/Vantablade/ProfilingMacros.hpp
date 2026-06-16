#pragma once

// ============================================================
// Inclusione condizionale di Tracy.
// VANTABLADE_PROFILING è definito dal sistema di build (CMake) quando
// ENABLE_TRACY=ON. Quando non definito, tutti i macro sotto
// sono no-op a costo zero (espansioni a espressione vuota).
// ============================================================
#ifdef VANTABLADE_PROFILING
#include <tracy/Tracy.hpp>
#endif

// ============================================================
// FRAME MARKERS
// Posizionare VND_FRAME_MARK dopo vkQueuePresentKHR nel loop
// principale. VND_FRAME_MARK_NAMED per timeline multiple.
// ============================================================
#ifdef VANTABLADE_PROFILING
#define VND_FRAME_MARK FrameMark
#define VND_FRAME_MARK_NAMED(name) FrameMarkNamed(name)
#define VND_FRAME_MARK_START(name) FrameMarkStart(name)
#define VND_FRAME_MARK_END(name) FrameMarkEnd(name)
#else
#define VND_FRAME_MARK
#define VND_FRAME_MARK_NAMED(name)
#define VND_FRAME_MARK_START(name)
#define VND_FRAME_MARK_END(name)
#endif

// ============================================================
// ZONE CPU
// VND_ZONE_SCOPED        — nome dalla firma della funzione
// VND_ZONE(name)         — nome stringa esplicito
// VND_ZONE_C(name, rgb)  — nome + colore 0xRRGGBB
// VND_ZONE_S(name, depth)— nome + callstack depth
// ============================================================
#ifdef VANTABLADE_PROFILING
#define VND_ZONE_SCOPED ZoneScoped
#define VND_ZONE(name) ZoneScopedN(name)
#define VND_ZONE_C(name, rgb) ZoneScopedNC(name, rgb)
#define VND_ZONE_S(name, depth) ZoneScopedNS(name, depth)
#define VND_ZONE_TEXT(str, len) ZoneText(str, len)
#define VND_ZONE_VALUE(val) ZoneValue(static_cast<uint64_t>(val))
#else
#define VND_ZONE_SCOPED
#define VND_ZONE(name)
#define VND_ZONE_C(name, rgb)
#define VND_ZONE_S(name, depth)
#define VND_ZONE_TEXT(str, len)
#define VND_ZONE_VALUE(val)
#endif

// ============================================================
// MEMORY TRACKING
// Ogni sito di allocazione/deallocazione custom deve chiamare
// questi macro. Tracy NON intercetta malloc/free automaticamente.
// ============================================================
#ifdef VANTABLADE_PROFILING
#define VND_ALLOC(ptr, size) TracyAlloc(ptr, size)
#define VND_FREE(ptr) TracyFree(ptr)
#define VND_ALLOC_N(ptr, size, pool) TracyAllocN(ptr, size, pool)
#define VND_FREE_N(ptr, pool) TracyFreeN(ptr, pool)
#else
#define VND_ALLOC(ptr, size)
#define VND_FREE(ptr)
#define VND_ALLOC_N(ptr, size, pool)
#define VND_FREE_N(ptr, pool)
#endif

// ============================================================
// PLOT E MESSAGGI
// Il parametro 'name' dei plot DEVE essere un string literal
// con storage duration statica.
// ============================================================
#ifdef VANTABLADE_PROFILING
#define VND_PLOT(name, value) TracyPlot(name, value)
#define VND_MESSAGE(str, len) TracyMessage(str, len)
#define VND_MESSAGE_L(literal) TracyMessageL(literal)
#define VND_MESSAGE_C(str, len, rgb) TracyMessageC(str, len, rgb)
#else
#define VND_PLOT(name, value)
#define VND_MESSAGE(str, len)
#define VND_MESSAGE_L(literal)
#define VND_MESSAGE_C(str, len, rgb)
#endif

// ============================================================
// LOCK TRACKING
// Usare VND_LOCKABLE al posto di std::mutex nella dichiarazione.
// VND_LOCK_BASE fornisce il tipo per std::lock_guard/unique_lock.
// ============================================================
#ifdef VANTABLADE_PROFILING
#define VND_LOCKABLE(type, name) TracyLockable(type, name)
#define VND_SHARED_LOCKABLE(type, name) TracySharedLockable(type, name)
#define VND_LOCK_BASE(type) LockableBase(type)
#define VND_LOCK_MARK(name) LockMark(name)
#else
#define VND_LOCKABLE(type, name) type name
#define VND_SHARED_LOCKABLE(type, name) type name
#define VND_LOCK_BASE(type) type
#define VND_LOCK_MARK(name)
#endif