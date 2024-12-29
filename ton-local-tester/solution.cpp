#include <iostream>
#include "td/utils/lz4.h"
#include "td/utils/base64.h"
#include "td/utils/buffer.h"
#include "td/utils/misc.h"
#include "vm/boc.h"

#define LZ4_STATIC_LINKING_ONLY
#define LZ4_HC_STATIC_LINKING_ONLY

/* -- lz4.h -- */

#if defined (__cplusplus)
extern "C" {
#endif

#ifndef LZ4_H_2983827168210
#define LZ4_H_2983827168210

/* --- Dependency --- */
#include <stddef.h>   /* size_t */


#ifndef LZ4LIB_VISIBILITY
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define LZ4LIB_VISIBILITY __attribute__ ((visibility ("default")))
#  else
#    define LZ4LIB_VISIBILITY
#  endif
#endif
#if defined(LZ4_DLL_EXPORT) && (LZ4_DLL_EXPORT==1)
#  define LZ4LIB_API __declspec(dllexport) LZ4LIB_VISIBILITY
#elif defined(LZ4_DLL_IMPORT) && (LZ4_DLL_IMPORT==1)
#  define LZ4LIB_API __declspec(dllimport) LZ4LIB_VISIBILITY /* It isn't required but allows to generate better code, saving a function pointer load from the IAT and an indirect jump.*/
#else
#  define LZ4LIB_API LZ4LIB_VISIBILITY
#endif

#if defined(LZ4_FREESTANDING) && (LZ4_FREESTANDING == 1)
#  define LZ4_HEAPMODE 0
#  define LZ4HC_HEAPMODE 0
#  define LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION 1
#  if !defined(LZ4_memcpy)
#    error "LZ4_FREESTANDING requires macro 'LZ4_memcpy'."
#  endif
#  if !defined(LZ4_memset)
#    error "LZ4_FREESTANDING requires macro 'LZ4_memset'."
#  endif
#  if !defined(LZ4_memmove)
#    error "LZ4_FREESTANDING requires macro 'LZ4_memmove'."
#  endif
#elif ! defined(LZ4_FREESTANDING)
#  define LZ4_FREESTANDING 0
#endif


/*------   Version   ------*/
#define LZ4_VERSION_MAJOR    1    /* for breaking interface changes  */
#define LZ4_VERSION_MINOR   10    /* for new (non-breaking) interface capabilities */
#define LZ4_VERSION_RELEASE  0    /* for tweaks, bug-fixes, or development */

#define LZ4_VERSION_NUMBER (LZ4_VERSION_MAJOR *100*100 + LZ4_VERSION_MINOR *100 + LZ4_VERSION_RELEASE)

#define LZ4_LIB_VERSION LZ4_VERSION_MAJOR.LZ4_VERSION_MINOR.LZ4_VERSION_RELEASE
#define LZ4_QUOTE(str) #str
#define LZ4_EXPAND_AND_QUOTE(str) LZ4_QUOTE(str)
#define LZ4_VERSION_STRING LZ4_EXPAND_AND_QUOTE(LZ4_LIB_VERSION)  /* requires v1.7.3+ */

LZ4LIB_API int LZ4_versionNumber (void);  /**< library version number; useful to check dll version; requires v1.3.0+ */
LZ4LIB_API const char* LZ4_versionString (void);   /**< library version string; useful to check dll version; requires v1.7.5+ */


#ifndef LZ4_MEMORY_USAGE
# define LZ4_MEMORY_USAGE LZ4_MEMORY_USAGE_DEFAULT
#endif

/* These are absolute limits, they should not be changed by users */
#define LZ4_MEMORY_USAGE_MIN 10
#define LZ4_MEMORY_USAGE_DEFAULT 14
#define LZ4_MEMORY_USAGE_MAX 20

#if (LZ4_MEMORY_USAGE < LZ4_MEMORY_USAGE_MIN)
#  error "LZ4_MEMORY_USAGE is too small !"
#endif

#if (LZ4_MEMORY_USAGE > LZ4_MEMORY_USAGE_MAX)
#  error "LZ4_MEMORY_USAGE is too large !"
#endif

LZ4LIB_API int LZ4_compress_default(const char* src, char* dst, int srcSize, int dstCapacity);
LZ4LIB_API int LZ4_decompress_safe (const char* src, char* dst, int compressedSize, int dstCapacity);

#define LZ4_MAX_INPUT_SIZE        0x7E000000   /* 2 113 929 216 bytes */
#define LZ4_COMPRESSBOUND(isize)  ((unsigned)(isize) > (unsigned)LZ4_MAX_INPUT_SIZE ? 0 : (isize) + ((isize)/255) + 16)
LZ4LIB_API int LZ4_compressBound(int inputSize);
LZ4LIB_API int LZ4_compress_fast (const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);
LZ4LIB_API int LZ4_sizeofState(void);
LZ4LIB_API int LZ4_compress_fast_extState (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);
LZ4LIB_API int LZ4_compress_destSize(const char* src, char* dst, int* srcSizePtr, int targetDstSize);

/*! LZ4_decompress_safe_partial() :
 *  Decompress an LZ4 compressed block, of size 'srcSize' at position 'src',
 *  into destination buffer 'dst' of size 'dstCapacity'.
 *  Up to 'targetOutputSize' bytes will be decoded.
 *  The function stops decoding on reaching this objective.
 *  This can be useful to boost performance
 *  whenever only the beginning of a block is required.
 *
 * @return : the number of bytes decoded in `dst` (necessarily <= targetOutputSize)
 *           If source stream is detected malformed, function returns a negative result.
 *
 *  Note 1 : @return can be < targetOutputSize, if compressed block contains less data.
 *
 *  Note 2 : targetOutputSize must be <= dstCapacity
 *
 *  Note 3 : this function effectively stops decoding on reaching targetOutputSize,
 *           so dstCapacity is kind of redundant.
 *           This is because in older versions of this function,
 *           decoding operation would still write complete sequences.
 *           Therefore, there was no guarantee that it would stop writing at exactly targetOutputSize,
 *           it could write more bytes, though only up to dstCapacity.
 *           Some "margin" used to be required for this operation to work properly.
 *           Thankfully, this is no longer necessary.
 *           The function nonetheless keeps the same signature, in an effort to preserve API compatibility.
 *
 *  Note 4 : If srcSize is the exact size of the block,
 *           then targetOutputSize can be any value,
 *           including larger than the block's decompressed size.
 *           The function will, at most, generate block's decompressed size.
 *
 *  Note 5 : If srcSize is _larger_ than block's compressed size,
 *           then targetOutputSize **MUST** be <= block's decompressed size.
 *           Otherwise, *silent corruption will occur*.
 */
LZ4LIB_API int LZ4_decompress_safe_partial (const char* src, char* dst, int srcSize, int targetOutputSize, int dstCapacity);


/*-*********************************************
*  Streaming Compression Functions
***********************************************/
typedef union LZ4_stream_u LZ4_stream_t;  /* incomplete type (defined later) */

/*!
 Note about RC_INVOKED

 - RC_INVOKED is predefined symbol of rc.exe (the resource compiler which is part of MSVC/Visual Studio).
   https://docs.microsoft.com/en-us/windows/win32/menurc/predefined-macros

 - Since rc.exe is a legacy compiler, it truncates long symbol (> 30 chars)
   and reports warning "RC4011: identifier truncated".

 - To eliminate the warning, we surround long preprocessor symbol with
   "#if !defined(RC_INVOKED) ... #endif" block that means
   "skip this block when rc.exe is trying to read it".
*/
#if !defined(RC_INVOKED) /* https://docs.microsoft.com/en-us/windows/win32/menurc/predefined-macros */
#if !defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION)
LZ4LIB_API LZ4_stream_t* LZ4_createStream(void);
LZ4LIB_API int           LZ4_freeStream (LZ4_stream_t* streamPtr);
#endif /* !defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION) */
#endif

/*! LZ4_resetStream_fast() : v1.9.0+
 *  Use this to prepare an LZ4_stream_t for a new chain of dependent blocks
 *  (e.g., LZ4_compress_fast_continue()).
 *
 *  An LZ4_stream_t must be initialized once before usage.
 *  This is automatically done when created by LZ4_createStream().
 *  However, should the LZ4_stream_t be simply declared on stack (for example),
 *  it's necessary to initialize it first, using LZ4_initStream().
 *
 *  After init, start any new stream with LZ4_resetStream_fast().
 *  A same LZ4_stream_t can be re-used multiple times consecutively
 *  and compress multiple streams,
 *  provided that it starts each new stream with LZ4_resetStream_fast().
 *
 *  LZ4_resetStream_fast() is much faster than LZ4_initStream(),
 *  but is not compatible with memory regions containing garbage data.
 *
 *  Note: it's only useful to call LZ4_resetStream_fast()
 *        in the context of streaming compression.
 *        The *extState* functions perform their own resets.
 *        Invoking LZ4_resetStream_fast() before is redundant, and even counterproductive.
 */
LZ4LIB_API void LZ4_resetStream_fast (LZ4_stream_t* streamPtr);

/*! LZ4_loadDict() :
 *  Use this function to reference a static dictionary into LZ4_stream_t.
 *  The dictionary must remain available during compression.
 *  LZ4_loadDict() triggers a reset, so any previous data will be forgotten.
 *  The same dictionary will have to be loaded on decompression side for successful decoding.
 *  Dictionary are useful for better compression of small data (KB range).
 *  While LZ4 itself accepts any input as dictionary, dictionary efficiency is also a topic.
 *  When in doubt, employ the Zstandard's Dictionary Builder.
 *  Loading a size of 0 is allowed, and is the same as reset.
 * @return : loaded dictionary size, in bytes (note: only the last 64 KB are loaded)
 */
LZ4LIB_API int LZ4_loadDict (LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);

/*! LZ4_loadDictSlow() : v1.10.0+
 *  Same as LZ4_loadDict(),
 *  but uses a bit more cpu to reference the dictionary content more thoroughly.
 *  This is expected to slightly improve compression ratio.
 *  The extra-cpu cost is likely worth it if the dictionary is re-used across multiple sessions.
 * @return : loaded dictionary size, in bytes (note: only the last 64 KB are loaded)
 */
LZ4LIB_API int LZ4_loadDictSlow(LZ4_stream_t* streamPtr, const char* dictionary, int dictSize);

/*! LZ4_attach_dictionary() : stable since v1.10.0
 *
 *  This allows efficient re-use of a static dictionary multiple times.
 *
 *  Rather than re-loading the dictionary buffer into a working context before
 *  each compression, or copying a pre-loaded dictionary's LZ4_stream_t into a
 *  working LZ4_stream_t, this function introduces a no-copy setup mechanism,
 *  in which the working stream references @dictionaryStream in-place.
 *
 *  Several assumptions are made about the state of @dictionaryStream.
 *  Currently, only states which have been prepared by LZ4_loadDict() or
 *  LZ4_loadDictSlow() should be expected to work.
 *
 *  Alternatively, the provided @dictionaryStream may be NULL,
 *  in which case any existing dictionary stream is unset.
 *
 *  If a dictionary is provided, it replaces any pre-existing stream history.
 *  The dictionary contents are the only history that can be referenced and
 *  logically immediately precede the data compressed in the first subsequent
 *  compression call.
 *
 *  The dictionary will only remain attached to the working stream through the
 *  first compression call, at the end of which it is cleared.
 * @dictionaryStream stream (and source buffer) must remain in-place / accessible / unchanged
 *  through the completion of the compression session.
 *
 *  Note: there is no equivalent LZ4_attach_*() method on the decompression side
 *  because there is no initialization cost, hence no need to share the cost across multiple sessions.
 *  To decompress LZ4 blocks using dictionary, attached or not,
 *  just employ the regular LZ4_setStreamDecode() for streaming,
 *  or the stateless LZ4_decompress_safe_usingDict() for one-shot decompression.
 */
LZ4LIB_API void
LZ4_attach_dictionary(LZ4_stream_t* workingStream,
                const LZ4_stream_t* dictionaryStream);

/*! LZ4_compress_fast_continue() :
 *  Compress 'src' content using data from previously compressed blocks, for better compression ratio.
 * 'dst' buffer must be already allocated.
 *  If dstCapacity >= LZ4_compressBound(srcSize), compression is guaranteed to succeed, and runs faster.
 *
 * @return : size of compressed block
 *           or 0 if there is an error (typically, cannot fit into 'dst').
 *
 *  Note 1 : Each invocation to LZ4_compress_fast_continue() generates a new block.
 *           Each block has precise boundaries.
 *           Each block must be decompressed separately, calling LZ4_decompress_*() with relevant metadata.
 *           It's not possible to append blocks together and expect a single invocation of LZ4_decompress_*() to decompress them together.
 *
 *  Note 2 : The previous 64KB of source data is __assumed__ to remain present, unmodified, at same address in memory !
 *
 *  Note 3 : When input is structured as a double-buffer, each buffer can have any size, including < 64 KB.
 *           Make sure that buffers are separated, by at least one byte.
 *           This construction ensures that each block only depends on previous block.
 *
 *  Note 4 : If input buffer is a ring-buffer, it can have any size, including < 64 KB.
 *
 *  Note 5 : After an error, the stream status is undefined (invalid), it can only be reset or freed.
 */
LZ4LIB_API int LZ4_compress_fast_continue (LZ4_stream_t* streamPtr, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_saveDict() :
 *  If last 64KB data cannot be guaranteed to remain available at its current memory location,
 *  save it into a safer place (char* safeBuffer).
 *  This is schematically equivalent to a memcpy() followed by LZ4_loadDict(),
 *  but is much faster, because LZ4_saveDict() doesn't need to rebuild tables.
 * @return : saved dictionary size in bytes (necessarily <= maxDictSize), or 0 if error.
 */
LZ4LIB_API int LZ4_saveDict (LZ4_stream_t* streamPtr, char* safeBuffer, int maxDictSize);


/*-**********************************************
*  Streaming Decompression Functions
*  Bufferless synchronous API
************************************************/
typedef union LZ4_streamDecode_u LZ4_streamDecode_t;   /* tracking context */

/*! LZ4_createStreamDecode() and LZ4_freeStreamDecode() :
 *  creation / destruction of streaming decompression tracking context.
 *  A tracking context can be re-used multiple times.
 */
#if !defined(RC_INVOKED) /* https://docs.microsoft.com/en-us/windows/win32/menurc/predefined-macros */
#if !defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION)
LZ4LIB_API LZ4_streamDecode_t* LZ4_createStreamDecode(void);
LZ4LIB_API int                 LZ4_freeStreamDecode (LZ4_streamDecode_t* LZ4_stream);
#endif /* !defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION) */
#endif

/*! LZ4_setStreamDecode() :
 *  An LZ4_streamDecode_t context can be allocated once and re-used multiple times.
 *  Use this function to start decompression of a new stream of blocks.
 *  A dictionary can optionally be set. Use NULL or size 0 for a reset order.
 *  Dictionary is presumed stable : it must remain accessible and unmodified during next decompression.
 * @return : 1 if OK, 0 if error
 */
LZ4LIB_API int LZ4_setStreamDecode (LZ4_streamDecode_t* LZ4_streamDecode, const char* dictionary, int dictSize);

/*! LZ4_decoderRingBufferSize() : v1.8.2+
 *  Note : in a ring buffer scenario (optional),
 *  blocks are presumed decompressed next to each other
 *  up to the moment there is not enough remaining space for next block (remainingSize < maxBlockSize),
 *  at which stage it resumes from beginning of ring buffer.
 *  When setting such a ring buffer for streaming decompression,
 *  provides the minimum size of this ring buffer
 *  to be compatible with any source respecting maxBlockSize condition.
 * @return : minimum ring buffer size,
 *           or 0 if there is an error (invalid maxBlockSize).
 */
LZ4LIB_API int LZ4_decoderRingBufferSize(int maxBlockSize);
#define LZ4_DECODER_RING_BUFFER_SIZE(maxBlockSize) (65536 + 14 + (maxBlockSize))  /* for static allocation; maxBlockSize presumed valid */

/*! LZ4_decompress_safe_continue() :
 *  This decoding function allows decompression of consecutive blocks in "streaming" mode.
 *  The difference with the usual independent blocks is that
 *  new blocks are allowed to find references into former blocks.
 *  A block is an unsplittable entity, and must be presented entirely to the decompression function.
 *  LZ4_decompress_safe_continue() only accepts one block at a time.
 *  It's modeled after `LZ4_decompress_safe()` and behaves similarly.
 *
 * @LZ4_streamDecode : decompression state, tracking the position in memory of past data
 * @compressedSize : exact complete size of one compressed block.
 * @dstCapacity : size of destination buffer (which must be already allocated),
 *                must be an upper bound of decompressed size.
 * @return : number of bytes decompressed into destination buffer (necessarily <= dstCapacity)
 *           If destination buffer is not large enough, decoding will stop and output an error code (negative value).
 *           If the source stream is detected malformed, the function will stop decoding and return a negative result.
 *
 *  The last 64KB of previously decoded data *must* remain available and unmodified
 *  at the memory position where they were previously decoded.
 *  If less than 64KB of data has been decoded, all the data must be present.
 *
 *  Special : if decompression side sets a ring buffer, it must respect one of the following conditions :
 *  - Decompression buffer size is _at least_ LZ4_decoderRingBufferSize(maxBlockSize).
 *    maxBlockSize is the maximum size of any single block. It can have any value > 16 bytes.
 *    In which case, encoding and decoding buffers do not need to be synchronized.
 *    Actually, data can be produced by any source compliant with LZ4 format specification, and respecting maxBlockSize.
 *  - Synchronized mode :
 *    Decompression buffer size is _exactly_ the same as compression buffer size,
 *    and follows exactly same update rule (block boundaries at same positions),
 *    and decoding function is provided with exact decompressed size of each block (exception for last block of the stream),
 *    _then_ decoding & encoding ring buffer can have any size, including small ones ( < 64 KB).
 *  - Decompression buffer is larger than encoding buffer, by a minimum of maxBlockSize more bytes.
 *    In which case, encoding and decoding buffers do not need to be synchronized,
 *    and encoding ring buffer can have any size, including small ones ( < 64 KB).
 *
 *  Whenever these conditions are not possible,
 *  save the last 64KB of decoded data into a safe buffer where it can't be modified during decompression,
 *  then indicate where this data is saved using LZ4_setStreamDecode(), before decompressing next block.
*/
LZ4LIB_API int
LZ4_decompress_safe_continue (LZ4_streamDecode_t* LZ4_streamDecode,
                        const char* src, char* dst,
                        int srcSize, int dstCapacity);


/*! LZ4_decompress_safe_usingDict() :
 *  Works the same as
 *  a combination of LZ4_setStreamDecode() followed by LZ4_decompress_safe_continue()
 *  However, it's stateless: it doesn't need any LZ4_streamDecode_t state.
 *  Dictionary is presumed stable : it must remain accessible and unmodified during decompression.
 *  Performance tip : Decompression speed can be substantially increased
 *                    when dst == dictStart + dictSize.
 */
LZ4LIB_API int
LZ4_decompress_safe_usingDict(const char* src, char* dst,
                              int srcSize, int dstCapacity,
                              const char* dictStart, int dictSize);

/*! LZ4_decompress_safe_partial_usingDict() :
 *  Behaves the same as LZ4_decompress_safe_partial()
 *  with the added ability to specify a memory segment for past data.
 *  Performance tip : Decompression speed can be substantially increased
 *                    when dst == dictStart + dictSize.
 */
LZ4LIB_API int
LZ4_decompress_safe_partial_usingDict(const char* src, char* dst,
                                      int compressedSize,
                                      int targetOutputSize, int maxOutputSize,
                                      const char* dictStart, int dictSize);

#endif /* LZ4_H_2983827168210 */


/*^*************************************
 * !!!!!!   STATIC LINKING ONLY   !!!!!!
 ***************************************/

/*-****************************************************************************
 * Experimental section
 *
 * Symbols declared in this section must be considered unstable. Their
 * signatures or semantics may change, or they may be removed altogether in the
 * future. They are therefore only safe to depend on when the caller is
 * statically linked against the library.
 *
 * To protect against unsafe usage, not only are the declarations guarded,
 * the definitions are hidden by default
 * when building LZ4 as a shared/dynamic library.
 *
 * In order to access these declarations,
 * define LZ4_STATIC_LINKING_ONLY in your application
 * before including LZ4's headers.
 *
 * In order to make their implementations accessible dynamically, you must
 * define LZ4_PUBLISH_STATIC_FUNCTIONS when building the LZ4 library.
 ******************************************************************************/

#ifdef LZ4_STATIC_LINKING_ONLY

#ifndef LZ4_STATIC_3504398509
#define LZ4_STATIC_3504398509

#ifdef LZ4_PUBLISH_STATIC_FUNCTIONS
# define LZ4LIB_STATIC_API LZ4LIB_API
#else
# define LZ4LIB_STATIC_API
#endif


/*! LZ4_compress_fast_extState_fastReset() :
 *  A variant of LZ4_compress_fast_extState().
 *
 *  Using this variant avoids an expensive initialization step.
 *  It is only safe to call if the state buffer is known to be correctly initialized already
 *  (see above comment on LZ4_resetStream_fast() for a definition of "correctly initialized").
 *  From a high level, the difference is that
 *  this function initializes the provided state with a call to something like LZ4_resetStream_fast()
 *  while LZ4_compress_fast_extState() starts with a call to LZ4_resetStream().
 */
LZ4LIB_STATIC_API int LZ4_compress_fast_extState_fastReset (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int acceleration);

/*! LZ4_compress_destSize_extState() : introduced in v1.10.0
 *  Same as LZ4_compress_destSize(), but using an externally allocated state.
 *  Also: exposes @acceleration
 */
int LZ4_compress_destSize_extState(void* state, const char* src, char* dst, int* srcSizePtr, int targetDstSize, int acceleration);

/*! In-place compression and decompression
 *
 * It's possible to have input and output sharing the same buffer,
 * for highly constrained memory environments.
 * In both cases, it requires input to lay at the end of the buffer,
 * and decompression to start at beginning of the buffer.
 * Buffer size must feature some margin, hence be larger than final size.
 *
 * |<------------------------buffer--------------------------------->|
 *                             |<-----------compressed data--------->|
 * |<-----------decompressed size------------------>|
 *                                                  |<----margin---->|
 *
 * This technique is more useful for decompression,
 * since decompressed size is typically larger,
 * and margin is short.
 *
 * In-place decompression will work inside any buffer
 * which size is >= LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(decompressedSize).
 * This presumes that decompressedSize > compressedSize.
 * Otherwise, it means compression actually expanded data,
 * and it would be more efficient to store such data with a flag indicating it's not compressed.
 * This can happen when data is not compressible (already compressed, or encrypted).
 *
 * For in-place compression, margin is larger, as it must be able to cope with both
 * history preservation, requiring input data to remain unmodified up to LZ4_DISTANCE_MAX,
 * and data expansion, which can happen when input is not compressible.
 * As a consequence, buffer size requirements are much higher,
 * and memory savings offered by in-place compression are more limited.
 *
 * There are ways to limit this cost for compression :
 * - Reduce history size, by modifying LZ4_DISTANCE_MAX.
 *   Note that it is a compile-time constant, so all compressions will apply this limit.
 *   Lower values will reduce compression ratio, except when input_size < LZ4_DISTANCE_MAX,
 *   so it's a reasonable trick when inputs are known to be small.
 * - Require the compressor to deliver a "maximum compressed size".
 *   This is the `dstCapacity` parameter in `LZ4_compress*()`.
 *   When this size is < LZ4_COMPRESSBOUND(inputSize), then compression can fail,
 *   in which case, the return code will be 0 (zero).
 *   The caller must be ready for these cases to happen,
 *   and typically design a backup scheme to send data uncompressed.
 * The combination of both techniques can significantly reduce
 * the amount of margin required for in-place compression.
 *
 * In-place compression can work in any buffer
 * which size is >= (maxCompressedSize)
 * with maxCompressedSize == LZ4_COMPRESSBOUND(srcSize) for guaranteed compression success.
 * LZ4_COMPRESS_INPLACE_BUFFER_SIZE() depends on both maxCompressedSize and LZ4_DISTANCE_MAX,
 * so it's possible to reduce memory requirements by playing with them.
 */

#define LZ4_DECOMPRESS_INPLACE_MARGIN(compressedSize)          (((compressedSize) >> 8) + 32)
#define LZ4_DECOMPRESS_INPLACE_BUFFER_SIZE(decompressedSize)   ((decompressedSize) + LZ4_DECOMPRESS_INPLACE_MARGIN(decompressedSize))  /**< note: presumes that compressedSize < decompressedSize. note2: margin is overestimated a bit, since it could use compressedSize instead */

#ifndef LZ4_DISTANCE_MAX   /* history window size; can be user-defined at compile time */
#  define LZ4_DISTANCE_MAX 65535   /* set to maximum value by default */
#endif

#define LZ4_COMPRESS_INPLACE_MARGIN                           (LZ4_DISTANCE_MAX + 32)   /* LZ4_DISTANCE_MAX can be safely replaced by srcSize when it's smaller */
#define LZ4_COMPRESS_INPLACE_BUFFER_SIZE(maxCompressedSize)   ((maxCompressedSize) + LZ4_COMPRESS_INPLACE_MARGIN)  /**< maxCompressedSize is generally LZ4_COMPRESSBOUND(inputSize), but can be set to any lower value, with the risk that compression can fail (return code 0(zero)) */

#endif   /* LZ4_STATIC_3504398509 */
#endif   /* LZ4_STATIC_LINKING_ONLY */



#ifndef LZ4_H_98237428734687
#define LZ4_H_98237428734687

/*-************************************************************
 *  Private Definitions
 **************************************************************
 * Do not use these definitions directly.
 * They are only exposed to allow static allocation of `LZ4_stream_t` and `LZ4_streamDecode_t`.
 * Accessing members will expose user code to API and/or ABI break in future versions of the library.
 **************************************************************/
#define LZ4_HASHLOG   (LZ4_MEMORY_USAGE-2)
#define LZ4_HASHTABLESIZE (1 << LZ4_MEMORY_USAGE)
#define LZ4_HASH_SIZE_U32 (1 << LZ4_HASHLOG)       /* required as macro for static allocation */

#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
# include <stdint.h>
  typedef  int8_t  LZ4_i8;
  typedef uint8_t  LZ4_byte;
  typedef uint16_t LZ4_u16;
  typedef uint32_t LZ4_u32;
#else
  typedef   signed char  LZ4_i8;
  typedef unsigned char  LZ4_byte;
  typedef unsigned short LZ4_u16;
  typedef unsigned int   LZ4_u32;
#endif

/*! LZ4_stream_t :
 *  Never ever use below internal definitions directly !
 *  These definitions are not API/ABI safe, and may change in future versions.
 *  If you need static allocation, declare or allocate an LZ4_stream_t object.
**/

typedef struct LZ4_stream_t_internal LZ4_stream_t_internal;
struct LZ4_stream_t_internal {
    LZ4_u32 hashTable[LZ4_HASH_SIZE_U32];
    const LZ4_byte* dictionary;
    const LZ4_stream_t_internal* dictCtx;
    LZ4_u32 currentOffset;
    LZ4_u32 tableType;
    LZ4_u32 dictSize;
    /* Implicit padding to ensure structure is aligned */
};

#define LZ4_STREAM_MINSIZE  ((1UL << (LZ4_MEMORY_USAGE)) + 32)  /* static size, for inter-version compatibility */
union LZ4_stream_u {
    char minStateSize[LZ4_STREAM_MINSIZE];
    LZ4_stream_t_internal internal_donotuse;
}; /* previously typedef'd to LZ4_stream_t */


/*! LZ4_initStream() : v1.9.0+
 *  An LZ4_stream_t structure must be initialized at least once.
 *  This is automatically done when invoking LZ4_createStream(),
 *  but it's not when the structure is simply declared on stack (for example).
 *
 *  Use LZ4_initStream() to properly initialize a newly declared LZ4_stream_t.
 *  It can also initialize any arbitrary buffer of sufficient size,
 *  and will @return a pointer of proper type upon initialization.
 *
 *  Note : initialization fails if size and alignment conditions are not respected.
 *         In which case, the function will @return NULL.
 *  Note2: An LZ4_stream_t structure guarantees correct alignment and size.
 *  Note3: Before v1.9.0, use LZ4_resetStream() instead
**/
LZ4LIB_API LZ4_stream_t* LZ4_initStream (void* stateBuffer, size_t size);


/*! LZ4_streamDecode_t :
 *  Never ever use below internal definitions directly !
 *  These definitions are not API/ABI safe, and may change in future versions.
 *  If you need static allocation, declare or allocate an LZ4_streamDecode_t object.
**/
typedef struct {
    const LZ4_byte* externalDict;
    const LZ4_byte* prefixEnd;
    size_t extDictSize;
    size_t prefixSize;
} LZ4_streamDecode_t_internal;

#define LZ4_STREAMDECODE_MINSIZE 32
union LZ4_streamDecode_u {
    char minStateSize[LZ4_STREAMDECODE_MINSIZE];
    LZ4_streamDecode_t_internal internal_donotuse;
} ;   /* previously typedef'd to LZ4_streamDecode_t */



/*-************************************
*  Obsolete Functions
**************************************/

/*! Deprecation warnings
 *
 *  Deprecated functions make the compiler generate a warning when invoked.
 *  This is meant to invite users to update their source code.
 *  Should deprecation warnings be a problem, it is generally possible to disable them,
 *  typically with -Wno-deprecated-declarations for gcc
 *  or _CRT_SECURE_NO_WARNINGS in Visual.
 *
 *  Another method is to define LZ4_DISABLE_DEPRECATE_WARNINGS
 *  before including the header file.
 */
#ifdef LZ4_DISABLE_DEPRECATE_WARNINGS
#  define LZ4_DEPRECATED(message)   /* disable deprecation warnings */
#else
#  if defined (__cplusplus) && (__cplusplus >= 201402) /* C++14 or greater */
#    define LZ4_DEPRECATED(message) [[deprecated(message)]]
#  elif defined(_MSC_VER)
#    define LZ4_DEPRECATED(message) __declspec(deprecated(message))
#  elif defined(__clang__) || (defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__ >= 45))
#    define LZ4_DEPRECATED(message) __attribute__((deprecated(message)))
#  elif defined(__GNUC__) && (__GNUC__ * 10 + __GNUC_MINOR__ >= 31)
#    define LZ4_DEPRECATED(message) __attribute__((deprecated))
#  else
#    pragma message("WARNING: LZ4_DEPRECATED needs custom implementation for this compiler")
#    define LZ4_DEPRECATED(message)   /* disabled */
#  endif
#endif /* LZ4_DISABLE_DEPRECATE_WARNINGS */


/*! LZ4_resetStream() :
 *  An LZ4_stream_t structure must be initialized at least once.
 *  This is done with LZ4_initStream(), or LZ4_resetStream().
 *  Consider switching to LZ4_initStream(),
 *  invoking LZ4_resetStream() will trigger deprecation warnings in the future.
 */
LZ4LIB_API void LZ4_resetStream (LZ4_stream_t* streamPtr);


#endif /* LZ4_H_98237428734687 */


#if defined (__cplusplus)
}
#endif

/* -- lz4hc.h -- */

/*
   LZ4 HC - High Compression Mode of LZ4
   Header File
   Copyright (C) 2011-2020, Yann Collet.
   BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

       * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
       * Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following disclaimer
   in the documentation and/or other materials provided with the
   distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   You can contact the author at :
   - LZ4 source repository : https://github.com/lz4/lz4
   - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/
#ifndef LZ4_HC_H_19834876238432
#define LZ4_HC_H_19834876238432

#if defined (__cplusplus)
extern "C" {
#endif

/* --- Dependency --- */
/* note : lz4hc requires lz4.h/lz4.c for compilation */
// #include "lz4.h"   /* stddef, LZ4LIB_API, LZ4_DEPRECATED */


/* --- Useful constants --- */
#define LZ4HC_CLEVEL_MIN         2
#define LZ4HC_CLEVEL_DEFAULT     9
#define LZ4HC_CLEVEL_OPT_MIN    10
#define LZ4HC_CLEVEL_MAX        12


/*-************************************
 *  Block Compression
 **************************************/
/*! LZ4_compress_HC() :
 *  Compress data from `src` into `dst`, using the powerful but slower "HC" algorithm.
 * `dst` must be already allocated.
 *  Compression is guaranteed to succeed if `dstCapacity >= LZ4_compressBound(srcSize)` (see "lz4.h")
 *  Max supported `srcSize` value is LZ4_MAX_INPUT_SIZE (see "lz4.h")
 * `compressionLevel` : any value between 1 and LZ4HC_CLEVEL_MAX will work.
 *                      Values > LZ4HC_CLEVEL_MAX behave the same as LZ4HC_CLEVEL_MAX.
 * @return : the number of bytes written into 'dst'
 *           or 0 if compression fails.
 */
LZ4LIB_API int LZ4_compress_HC (const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel);


/* Note :
 *   Decompression functions are provided within "lz4.h" (BSD license)
 */


/*! LZ4_compress_HC_extStateHC() :
 *  Same as LZ4_compress_HC(), but using an externally allocated memory segment for `state`.
 * `state` size is provided by LZ4_sizeofStateHC().
 *  Memory segment must be aligned on 8-bytes boundaries (which a normal malloc() should do properly).
 */
LZ4LIB_API int LZ4_sizeofStateHC(void);
LZ4LIB_API int LZ4_compress_HC_extStateHC(void* stateHC, const char* src, char* dst, int srcSize, int maxDstSize, int compressionLevel);


/*! LZ4_compress_HC_destSize() : v1.9.0+
 *  Will compress as much data as possible from `src`
 *  to fit into `targetDstSize` budget.
 *  Result is provided in 2 parts :
 * @return : the number of bytes written into 'dst' (necessarily <= targetDstSize)
 *           or 0 if compression fails.
 * `srcSizePtr` : on success, *srcSizePtr is updated to indicate how much bytes were read from `src`
 */
LZ4LIB_API int LZ4_compress_HC_destSize(void* stateHC,
                                  const char* src, char* dst,
                                        int* srcSizePtr, int targetDstSize,
                                        int compressionLevel);


/*-************************************
 *  Streaming Compression
 *  Bufferless synchronous API
 **************************************/
 typedef union LZ4_streamHC_u LZ4_streamHC_t;   /* incomplete type (defined later) */

/*! LZ4_createStreamHC() and LZ4_freeStreamHC() :
 *  These functions create and release memory for LZ4 HC streaming state.
 *  Newly created states are automatically initialized.
 *  A same state can be used multiple times consecutively,
 *  starting with LZ4_resetStreamHC_fast() to start a new stream of blocks.
 */
LZ4LIB_API LZ4_streamHC_t* LZ4_createStreamHC(void);
LZ4LIB_API int             LZ4_freeStreamHC (LZ4_streamHC_t* streamHCPtr);

/*
  These functions compress data in successive blocks of any size,
  using previous blocks as dictionary, to improve compression ratio.
  One key assumption is that previous blocks (up to 64 KB) remain read-accessible while compressing next blocks.
  There is an exception for ring buffers, which can be smaller than 64 KB.
  Ring-buffer scenario is automatically detected and handled within LZ4_compress_HC_continue().

  Before starting compression, state must be allocated and properly initialized.
  LZ4_createStreamHC() does both, though compression level is set to LZ4HC_CLEVEL_DEFAULT.

  Selecting the compression level can be done with LZ4_resetStreamHC_fast() (starts a new stream)
  or LZ4_setCompressionLevel() (anytime, between blocks in the same stream) (experimental).
  LZ4_resetStreamHC_fast() only works on states which have been properly initialized at least once,
  which is automatically the case when state is created using LZ4_createStreamHC().

  After reset, a first "fictional block" can be designated as initial dictionary,
  using LZ4_loadDictHC() (Optional).
  Note: In order for LZ4_loadDictHC() to create the correct data structure,
  it is essential to set the compression level _before_ loading the dictionary.

  Invoke LZ4_compress_HC_continue() to compress each successive block.
  The number of blocks is unlimited.
  Previous input blocks, including initial dictionary when present,
  must remain accessible and unmodified during compression.

  It's allowed to update compression level anytime between blocks,
  using LZ4_setCompressionLevel() (experimental).

 @dst buffer should be sized to handle worst case scenarios
  (see LZ4_compressBound(), it ensures compression success).
  In case of failure, the API does not guarantee recovery,
  so the state _must_ be reset.
  To ensure compression success
  whenever @dst buffer size cannot be made >= LZ4_compressBound(),
  consider using LZ4_compress_HC_continue_destSize().

  Whenever previous input blocks can't be preserved unmodified in-place during compression of next blocks,
  it's possible to copy the last blocks into a more stable memory space, using LZ4_saveDictHC().
  Return value of LZ4_saveDictHC() is the size of dictionary effectively saved into 'safeBuffer' (<= 64 KB)

  After completing a streaming compression,
  it's possible to start a new stream of blocks, using the same LZ4_streamHC_t state,
  just by resetting it, using LZ4_resetStreamHC_fast().
*/

LZ4LIB_API void LZ4_resetStreamHC_fast(LZ4_streamHC_t* streamHCPtr, int compressionLevel);   /* v1.9.0+ */
LZ4LIB_API int  LZ4_loadDictHC (LZ4_streamHC_t* streamHCPtr, const char* dictionary, int dictSize);

LZ4LIB_API int LZ4_compress_HC_continue (LZ4_streamHC_t* streamHCPtr,
                                   const char* src, char* dst,
                                         int srcSize, int maxDstSize);

/*! LZ4_compress_HC_continue_destSize() : v1.9.0+
 *  Similar to LZ4_compress_HC_continue(),
 *  but will read as much data as possible from `src`
 *  to fit into `targetDstSize` budget.
 *  Result is provided into 2 parts :
 * @return : the number of bytes written into 'dst' (necessarily <= targetDstSize)
 *           or 0 if compression fails.
 * `srcSizePtr` : on success, *srcSizePtr will be updated to indicate how much bytes were read from `src`.
 *           Note that this function may not consume the entire input.
 */
LZ4LIB_API int LZ4_compress_HC_continue_destSize(LZ4_streamHC_t* LZ4_streamHCPtr,
                                           const char* src, char* dst,
                                                 int* srcSizePtr, int targetDstSize);

LZ4LIB_API int LZ4_saveDictHC (LZ4_streamHC_t* streamHCPtr, char* safeBuffer, int maxDictSize);


/*! LZ4_attach_HC_dictionary() : stable since v1.10.0
 *  This API allows for the efficient re-use of a static dictionary many times.
 *
 *  Rather than re-loading the dictionary buffer into a working context before
 *  each compression, or copying a pre-loaded dictionary's LZ4_streamHC_t into a
 *  working LZ4_streamHC_t, this function introduces a no-copy setup mechanism,
 *  in which the working stream references the dictionary stream in-place.
 *
 *  Several assumptions are made about the state of the dictionary stream.
 *  Currently, only streams which have been prepared by LZ4_loadDictHC() should
 *  be expected to work.
 *
 *  Alternatively, the provided dictionary stream pointer may be NULL, in which
 *  case any existing dictionary stream is unset.
 *
 *  A dictionary should only be attached to a stream without any history (i.e.,
 *  a stream that has just been reset).
 *
 *  The dictionary will remain attached to the working stream only for the
 *  current stream session. Calls to LZ4_resetStreamHC(_fast) will remove the
 *  dictionary context association from the working stream. The dictionary
 *  stream (and source buffer) must remain in-place / accessible / unchanged
 *  through the lifetime of the stream session.
 */
LZ4LIB_API void
LZ4_attach_HC_dictionary(LZ4_streamHC_t* working_stream,
                   const LZ4_streamHC_t* dictionary_stream);


/*^**********************************************
 * !!!!!!   STATIC LINKING ONLY   !!!!!!
 ***********************************************/

/*-******************************************************************
 * PRIVATE DEFINITIONS :
 * Do not use these definitions directly.
 * They are merely exposed to allow static allocation of `LZ4_streamHC_t`.
 * Declare an `LZ4_streamHC_t` directly, rather than any type below.
 * Even then, only do so in the context of static linking, as definitions may change between versions.
 ********************************************************************/

#define LZ4HC_DICTIONARY_LOGSIZE 16
#define LZ4HC_MAXD (1<<LZ4HC_DICTIONARY_LOGSIZE)
#define LZ4HC_MAXD_MASK (LZ4HC_MAXD - 1)

#define LZ4HC_HASH_LOG 15
#define LZ4HC_HASHTABLESIZE (1 << LZ4HC_HASH_LOG)
#define LZ4HC_HASH_MASK (LZ4HC_HASHTABLESIZE - 1)


/* Never ever use these definitions directly !
 * Declare or allocate an LZ4_streamHC_t instead.
**/
typedef struct LZ4HC_CCtx_internal LZ4HC_CCtx_internal;
struct LZ4HC_CCtx_internal
{
    LZ4_u32 hashTable[LZ4HC_HASHTABLESIZE];
    LZ4_u16 chainTable[LZ4HC_MAXD];
    const LZ4_byte* end;     /* next block here to continue on current prefix */
    const LZ4_byte* prefixStart;  /* Indexes relative to this position */
    const LZ4_byte* dictStart; /* alternate reference for extDict */
    LZ4_u32 dictLimit;       /* below that point, need extDict */
    LZ4_u32 lowLimit;        /* below that point, no more history */
    LZ4_u32 nextToUpdate;    /* index from which to continue dictionary update */
    short   compressionLevel;
    LZ4_i8  favorDecSpeed;   /* favor decompression speed if this flag set,
                                otherwise, favor compression ratio */
    LZ4_i8  dirty;           /* stream has to be fully reset if this flag is set */
    const LZ4HC_CCtx_internal* dictCtx;
};

#define LZ4_STREAMHC_MINSIZE  262200  /* static size, for inter-version compatibility */
union LZ4_streamHC_u {
    char minStateSize[LZ4_STREAMHC_MINSIZE];
    LZ4HC_CCtx_internal internal_donotuse;
}; /* previously typedef'd to LZ4_streamHC_t */

/* LZ4_streamHC_t :
 * This structure allows static allocation of LZ4 HC streaming state.
 * This can be used to allocate statically on stack, or as part of a larger structure.
 *
 * Such state **must** be initialized using LZ4_initStreamHC() before first use.
 *
 * Note that invoking LZ4_initStreamHC() is not required when
 * the state was created using LZ4_createStreamHC() (which is recommended).
 * Using the normal builder, a newly created state is automatically initialized.
 *
 * Static allocation shall only be used in combination with static linking.
 */

/* LZ4_initStreamHC() : v1.9.0+
 * Required before first use of a statically allocated LZ4_streamHC_t.
 * Before v1.9.0 : use LZ4_resetStreamHC() instead
 */
LZ4LIB_API LZ4_streamHC_t* LZ4_initStreamHC(void* buffer, size_t size);

LZ4LIB_API void LZ4_resetStreamHC (LZ4_streamHC_t* streamHCPtr, int compressionLevel);


#if defined (__cplusplus)
}
#endif

#endif /* LZ4_HC_H_19834876238432 */


/*-**************************************************
 * !!!!!     STATIC LINKING ONLY     !!!!!
 * Following definitions are considered experimental.
 * They should not be linked from DLL,
 * as there is no guarantee of API stability yet.
 * Prototypes will be promoted to "stable" status
 * after successful usage in real-life scenarios.
 ***************************************************/
#ifdef LZ4_HC_STATIC_LINKING_ONLY   /* protection macro */
#ifndef LZ4_HC_SLO_098092834
#define LZ4_HC_SLO_098092834

#if defined (__cplusplus)
extern "C" {
#endif

/*! LZ4_setCompressionLevel() : v1.8.0+ (experimental)
 *  It's possible to change compression level
 *  between successive invocations of LZ4_compress_HC_continue*()
 *  for dynamic adaptation.
 */
LZ4LIB_STATIC_API void LZ4_setCompressionLevel(
    LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel);

/*! LZ4_favorDecompressionSpeed() : v1.8.2+ (experimental)
 *  Opt. Parser will favor decompression speed over compression ratio.
 *  Only applicable to levels >= LZ4HC_CLEVEL_OPT_MIN.
 */
LZ4LIB_STATIC_API void LZ4_favorDecompressionSpeed(
    LZ4_streamHC_t* LZ4_streamHCPtr, int favor);

/*! LZ4_resetStreamHC_fast() : v1.9.0+
 *  When an LZ4_streamHC_t is known to be in a internally coherent state,
 *  it can often be prepared for a new compression with almost no work, only
 *  sometimes falling back to the full, expensive reset that is always required
 *  when the stream is in an indeterminate state (i.e., the reset performed by
 *  LZ4_resetStreamHC()).
 *
 *  LZ4_streamHCs are guaranteed to be in a valid state when:
 *  - returned from LZ4_createStreamHC()
 *  - reset by LZ4_resetStreamHC()
 *  - memset(stream, 0, sizeof(LZ4_streamHC_t))
 *  - the stream was in a valid state and was reset by LZ4_resetStreamHC_fast()
 *  - the stream was in a valid state and was then used in any compression call
 *    that returned success
 *  - the stream was in an indeterminate state and was used in a compression
 *    call that fully reset the state (LZ4_compress_HC_extStateHC()) and that
 *    returned success
 *
 *  Note:
 *  A stream that was last used in a compression call that returned an error
 *  may be passed to this function. However, it will be fully reset, which will
 *  clear any existing history and settings from the context.
 */
LZ4LIB_STATIC_API void LZ4_resetStreamHC_fast(
    LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel);

/*! LZ4_compress_HC_extStateHC_fastReset() :
 *  A variant of LZ4_compress_HC_extStateHC().
 *
 *  Using this variant avoids an expensive initialization step. It is only safe
 *  to call if the state buffer is known to be correctly initialized already
 *  (see above comment on LZ4_resetStreamHC_fast() for a definition of
 *  "correctly initialized"). From a high level, the difference is that this
 *  function initializes the provided state with a call to
 *  LZ4_resetStreamHC_fast() while LZ4_compress_HC_extStateHC() starts with a
 *  call to LZ4_resetStreamHC().
 */
LZ4LIB_STATIC_API int LZ4_compress_HC_extStateHC_fastReset (
    void* state,
    const char* src, char* dst,
    int srcSize, int dstCapacity,
    int compressionLevel);

#if defined (__cplusplus)
}
#endif

#endif   /* LZ4_HC_SLO_098092834 */
#endif   /* LZ4_HC_STATIC_LINKING_ONLY */


/* -- lz4hc.c -- */

/*
    LZ4 HC - High Compression Mode of LZ4
    Copyright (C) 2011-2020, Yann Collet.

    BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    You can contact the author at :
       - LZ4 source repository : https://github.com/lz4/lz4
       - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/
/* note : lz4hc is not an independent module, it requires lz4.h/lz4.c for proper compilation */


/* *************************************
*  Tuning Parameter
***************************************/

/*! HEAPMODE :
 *  Select how stateless HC compression functions like `LZ4_compress_HC()`
 *  allocate memory for their workspace:
 *  in stack (0:fastest), or in heap (1:default, requires malloc()).
 *  Since workspace is rather large, heap mode is recommended.
**/
#ifndef LZ4HC_HEAPMODE
#  define LZ4HC_HEAPMODE 1
#endif


/*===    Dependency    ===*/
#include <limits.h>


/*===   Shared lz4.c code   ===*/
// #ifndef LZ4_SRC_INCLUDED
// # if defined(__GNUC__)
// #  pragma GCC diagnostic ignored "-Wunused-function"
// # endif
// # if defined (__clang__)
// #  pragma clang diagnostic ignored "-Wunused-function"
// # endif
// # define LZ4_COMMONDEFS_ONLY
// # include "lz4.c"   /* LZ4_count, constants, mem */
// #endif

#include <limits.h>


/*-************************************
*  Tuning parameters
**************************************/
/*
 * LZ4_HEAPMODE :
 * Select how stateless compression functions like `LZ4_compress_default()`
 * allocate memory for their hash table,
 * in memory stack (0:default, fastest), or in memory heap (1:requires malloc()).
 */
#ifndef LZ4_HEAPMODE
#  define LZ4_HEAPMODE 0
#endif

/*
 * LZ4_ACCELERATION_DEFAULT :
 * Select "acceleration" for LZ4_compress_fast() when parameter value <= 0
 */
#define LZ4_ACCELERATION_DEFAULT 1
/*
 * LZ4_ACCELERATION_MAX :
 * Any "acceleration" value higher than this threshold
 * get treated as LZ4_ACCELERATION_MAX instead (fix #876)
 */
#define LZ4_ACCELERATION_MAX 65537


/*-************************************
*  CPU Feature Detection
**************************************/
/* LZ4_FORCE_MEMORY_ACCESS
 * By default, access to unaligned memory is controlled by `memcpy()`, which is safe and portable.
 * Unfortunately, on some target/compiler combinations, the generated assembly is sub-optimal.
 * The below switch allow to select different access method for improved performance.
 * Method 0 (default) : use `memcpy()`. Safe and portable.
 * Method 1 : `__packed` statement. It depends on compiler extension (ie, not portable).
 *            This method is safe if your compiler supports it, and *generally* as fast or faster than `memcpy`.
 * Method 2 : direct access. This method is portable but violate C standard.
 *            It can generate buggy code on targets which assembly generation depends on alignment.
 *            But in some circumstances, it's the only known way to get the most performance (ie GCC + ARMv6)
 * See https://fastcompression.blogspot.fr/2015/08/accessing-unaligned-memory.html for details.
 * Prefer these methods in priority order (0 > 1 > 2)
 */
#ifndef LZ4_FORCE_MEMORY_ACCESS   /* can be defined externally */
#  if defined(__GNUC__) && \
  ( defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) \
  || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__) )
#    define LZ4_FORCE_MEMORY_ACCESS 2
#  elif (defined(__INTEL_COMPILER) && !defined(_WIN32)) || defined(__GNUC__) || defined(_MSC_VER)
#    define LZ4_FORCE_MEMORY_ACCESS 1
#  endif
#endif

/*
 * LZ4_FORCE_SW_BITCOUNT
 * Define this parameter if your target system or compiler does not support hardware bit count
 */
#if defined(_MSC_VER) && defined(_WIN32_WCE)   /* Visual Studio for WinCE doesn't support Hardware bit count */
#  undef  LZ4_FORCE_SW_BITCOUNT  /* avoid double def */
#  define LZ4_FORCE_SW_BITCOUNT
#endif



/*-************************************
*  Dependency
**************************************/
/*
 * LZ4_SRC_INCLUDED:
 * Amalgamation flag, whether lz4.c is included
 */
#ifndef LZ4_SRC_INCLUDED
#  define LZ4_SRC_INCLUDED 1
#endif

#ifndef LZ4_DISABLE_DEPRECATE_WARNINGS
#  define LZ4_DISABLE_DEPRECATE_WARNINGS /* due to LZ4_decompress_safe_withPrefix64k */
#endif

#ifndef LZ4_STATIC_LINKING_ONLY
#  define LZ4_STATIC_LINKING_ONLY
#endif
/* see also "memory routines" below */


/*-************************************
*  Compiler Options
**************************************/
#if defined(_MSC_VER) && (_MSC_VER >= 1400)  /* Visual Studio 2005+ */
#  include <intrin.h>               /* only present in VS2005+ */
#  pragma warning(disable : 4127)   /* disable: C4127: conditional expression is constant */
#  pragma warning(disable : 6237)   /* disable: C6237: conditional expression is always 0 */
#  pragma warning(disable : 6239)   /* disable: C6239: (<non-zero constant> && <expression>) always evaluates to the result of <expression> */
#  pragma warning(disable : 6240)   /* disable: C6240: (<expression> && <non-zero constant>) always evaluates to the result of <expression> */
#  pragma warning(disable : 6326)   /* disable: C6326: Potential comparison of a constant with another constant */
#endif  /* _MSC_VER */

#ifndef LZ4_FORCE_INLINE
#  if defined (_MSC_VER) && !defined (__clang__)    /* MSVC */
#    define LZ4_FORCE_INLINE static __forceinline
#  else
#    if defined (__cplusplus) || defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L   /* C99 */
#      if defined (__GNUC__) || defined (__clang__)
#        define LZ4_FORCE_INLINE static inline __attribute__((always_inline))
#      else
#        define LZ4_FORCE_INLINE static inline
#      endif
#    else
#      define LZ4_FORCE_INLINE static
#    endif /* __STDC_VERSION__ */
#  endif  /* _MSC_VER */
#endif /* LZ4_FORCE_INLINE */

/* LZ4_FORCE_O2 and LZ4_FORCE_INLINE
 * gcc on ppc64le generates an unrolled SIMDized loop for LZ4_wildCopy8,
 * together with a simple 8-byte copy loop as a fall-back path.
 * However, this optimization hurts the decompression speed by >30%,
 * because the execution does not go to the optimized loop
 * for typical compressible data, and all of the preamble checks
 * before going to the fall-back path become useless overhead.
 * This optimization happens only with the -O3 flag, and -O2 generates
 * a simple 8-byte copy loop.
 * With gcc on ppc64le, all of the LZ4_decompress_* and LZ4_wildCopy8
 * functions are annotated with __attribute__((optimize("O2"))),
 * and also LZ4_wildCopy8 is forcibly inlined, so that the O2 attribute
 * of LZ4_wildCopy8 does not affect the compression speed.
 */
#if defined(__PPC64__) && defined(__LITTLE_ENDIAN__) && defined(__GNUC__) && !defined(__clang__)
#  define LZ4_FORCE_O2  __attribute__((optimize("O2")))
#  undef LZ4_FORCE_INLINE
#  define LZ4_FORCE_INLINE  static __inline __attribute__((optimize("O2"),always_inline))
#else
#  define LZ4_FORCE_O2
#endif

#if (defined(__GNUC__) && (__GNUC__ >= 3)) || (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 800)) || defined(__clang__)
#  define expect(expr,value)    (__builtin_expect ((expr),(value)) )
#else
#  define expect(expr,value)    (expr)
#endif

#ifndef likely
#define likely(expr)     expect((expr) != 0, 1)
#endif
#ifndef unlikely
#define unlikely(expr)   expect((expr) != 0, 0)
#endif

/* Should the alignment test prove unreliable, for some reason,
 * it can be disabled by setting LZ4_ALIGN_TEST to 0 */
#ifndef LZ4_ALIGN_TEST  /* can be externally provided */
# define LZ4_ALIGN_TEST 1
#endif


/*-************************************
*  Memory routines
**************************************/

/*! LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION :
 *  Disable relatively high-level LZ4/HC functions that use dynamic memory
 *  allocation functions (malloc(), calloc(), free()).
 *
 *  Note that this is a compile-time switch. And since it disables
 *  public/stable LZ4 v1 API functions, we don't recommend using this
 *  symbol to generate a library for distribution.
 *
 *  The following public functions are removed when this symbol is defined.
 *  - lz4   : LZ4_createStream, LZ4_freeStream,
 *            LZ4_createStreamDecode, LZ4_freeStreamDecode, LZ4_create (deprecated)
 *  - lz4hc : LZ4_createStreamHC, LZ4_freeStreamHC,
 *            LZ4_createHC (deprecated), LZ4_freeHC  (deprecated)
 *  - lz4frame, lz4file : All LZ4F_* functions
 */
#if defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION)
#  define ALLOC(s)          lz4_error_memory_allocation_is_disabled
#  define ALLOC_AND_ZERO(s) lz4_error_memory_allocation_is_disabled
#  define FREEMEM(p)        lz4_error_memory_allocation_is_disabled
#elif defined(LZ4_USER_MEMORY_FUNCTIONS)
/* memory management functions can be customized by user project.
 * Below functions must exist somewhere in the Project
 * and be available at link time */
void* LZ4_malloc(size_t s);
void* LZ4_calloc(size_t n, size_t s);
void  LZ4_free(void* p);
# define ALLOC(s)          LZ4_malloc(s)
# define ALLOC_AND_ZERO(s) LZ4_calloc(1,s)
# define FREEMEM(p)        LZ4_free(p)
#else
# include <stdlib.h>   /* malloc, calloc, free */
# define ALLOC(s)          malloc(s)
# define ALLOC_AND_ZERO(s) calloc(1,s)
# define FREEMEM(p)        free(p)
#endif

#if ! LZ4_FREESTANDING
#  include <string.h>   /* memset, memcpy */
#endif
#if !defined(LZ4_memset)
#  define LZ4_memset(p,v,s) memset((p),(v),(s))
#endif
#define MEM_INIT(p,v,s)   LZ4_memset((p),(v),(s))


/*-************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define WILDCOPYLENGTH 8
#define LASTLITERALS   5   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MFLIMIT       12   /* see ../doc/lz4_Block_format.md#parsing-restrictions */
#define MATCH_SAFEGUARD_DISTANCE  ((2*WILDCOPYLENGTH) - MINMATCH)   /* ensure it's possible to write 2 x wildcopyLength without overflowing output buffer */
#define FASTLOOP_SAFE_DISTANCE 64
static const int LZ4_minLength = (MFLIMIT+1);

#define KB *(1 <<10)
#define MB *(1 <<20)
#define GB *(1U<<30)

#define LZ4_DISTANCE_ABSOLUTE_MAX 65535
#if (LZ4_DISTANCE_MAX > LZ4_DISTANCE_ABSOLUTE_MAX)   /* max supported by LZ4 format */
#  error "LZ4_DISTANCE_MAX is too big : must be <= 65535"
#endif

#define ML_BITS  4
#define ML_MASK  ((1U<<ML_BITS)-1)
#define RUN_BITS (8-ML_BITS)
#define RUN_MASK ((1U<<RUN_BITS)-1)


/*-************************************
*  Error detection
**************************************/
#if defined(LZ4_DEBUG) && (LZ4_DEBUG>=1)
#  include <assert.h>
#else
#  ifndef assert
#    define assert(condition) ((void)0)
#  endif
#endif

#define LZ4_STATIC_ASSERT(c)   { enum { LZ4_static_assert = 1/(int)(!!(c)) }; }   /* use after variable declarations */

#if defined(LZ4_DEBUG) && (LZ4_DEBUG>=2)
#  include <stdio.h>
   static int g_debuglog_enable = 1;
#  define DEBUGLOG(l, ...) {                          \
        if ((g_debuglog_enable) && (l<=LZ4_DEBUG)) {  \
            fprintf(stderr, __FILE__  " %i: ", __LINE__); \
            fprintf(stderr, __VA_ARGS__);             \
            fprintf(stderr, " \n");                   \
    }   }
#else
#  define DEBUGLOG(l, ...) {}    /* disabled */
#endif

static int LZ4_isAligned(const void* ptr, size_t alignment)
{
    return ((size_t)ptr & (alignment -1)) == 0;
}


/*-************************************
*  Types
**************************************/
#include <limits.h>
#if defined(__cplusplus) || (defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* C99 */)
# include <stdint.h>
  typedef  uint8_t BYTE;
  typedef uint16_t U16;
  typedef uint32_t U32;
  typedef  int32_t S32;
  typedef uint64_t U64;
  typedef uintptr_t uptrval;
#else
# if UINT_MAX != 4294967295UL
#   error "LZ4 code (when not C++ or C99) assumes that sizeof(int) == 4"
# endif
  typedef unsigned char       BYTE;
  typedef unsigned short      U16;
  typedef unsigned int        U32;
  typedef   signed int        S32;
  typedef unsigned long long  U64;
  typedef size_t              uptrval;   /* generally true, except OpenVMS-64 */
#endif

#if defined(__x86_64__)
  typedef U64    reg_t;   /* 64-bits in x32 mode */
#else
  typedef size_t reg_t;   /* 32-bits in x32 mode */
#endif

typedef enum {
    notLimited = 0,
    limitedOutput = 1,
    fillOutput = 2
} limitedOutput_directive;


/*-************************************
*  Reading and writing into memory
**************************************/

/**
 * LZ4 relies on memcpy with a constant size being inlined. In freestanding
 * environments, the compiler can't assume the implementation of memcpy() is
 * standard compliant, so it can't apply its specialized memcpy() inlining
 * logic. When possible, use __builtin_memcpy() to tell the compiler to analyze
 * memcpy() as if it were standard compliant, so it can inline it in freestanding
 * environments. This is needed when decompressing the Linux Kernel, for example.
 */
#if !defined(LZ4_memcpy)
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define LZ4_memcpy(dst, src, size) __builtin_memcpy(dst, src, size)
#  else
#    define LZ4_memcpy(dst, src, size) memcpy(dst, src, size)
#  endif
#endif

#if !defined(LZ4_memmove)
#  if defined(__GNUC__) && (__GNUC__ >= 4)
#    define LZ4_memmove __builtin_memmove
#  else
#    define LZ4_memmove memmove
#  endif
#endif

static unsigned LZ4_isLittleEndian(void)
{
    const union { U32 u; BYTE c[4]; } one = { 1 };   /* don't use static : performance detrimental */
    return one.c[0];
}

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
#define LZ4_PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#elif defined(_MSC_VER)
#define LZ4_PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

#if defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==2)
/* lie to the compiler about data alignment; use with caution */

static U16 LZ4_read16(const void* memPtr) { return *(const U16*) memPtr; }
static U32 LZ4_read32(const void* memPtr) { return *(const U32*) memPtr; }
static reg_t LZ4_read_ARCH(const void* memPtr) { return *(const reg_t*) memPtr; }

static void LZ4_write16(void* memPtr, U16 value) { *(U16*)memPtr = value; }

#elif defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==1)

/* __pack instructions are safer, but compiler specific, hence potentially problematic for some compilers */
/* currently only defined for gcc and icc */
LZ4_PACK(typedef struct { U16 u16; }) LZ4_unalign16;
LZ4_PACK(typedef struct { U32 u32; }) LZ4_unalign32;
LZ4_PACK(typedef struct { reg_t uArch; }) LZ4_unalignST;

static U16 LZ4_read16(const void* ptr) { return ((const LZ4_unalign16*)ptr)->u16; }
static U32 LZ4_read32(const void* ptr) { return ((const LZ4_unalign32*)ptr)->u32; }
static reg_t LZ4_read_ARCH(const void* ptr) { return ((const LZ4_unalignST*)ptr)->uArch; }

static void LZ4_write16(void* memPtr, U16 value) { ((LZ4_unalign16*)memPtr)->u16 = value; }

#else  /* safe and portable access using memcpy() */

static U16 LZ4_read16(const void* memPtr)
{
    U16 val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static U32 LZ4_read32(const void* memPtr)
{
    U32 val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static reg_t LZ4_read_ARCH(const void* memPtr)
{
    reg_t val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

static void LZ4_write16(void* memPtr, U16 value)
{
    LZ4_memcpy(memPtr, &value, sizeof(value));
}

#endif /* LZ4_FORCE_MEMORY_ACCESS */


/* customized variant of memcpy, which can overwrite up to 8 bytes beyond dstEnd */
LZ4_FORCE_INLINE
void LZ4_wildCopy8(void* dstPtr, const void* srcPtr, void* dstEnd)
{
    BYTE* d = (BYTE*)dstPtr;
    const BYTE* s = (const BYTE*)srcPtr;
    BYTE* const e = (BYTE*)dstEnd;

    do { LZ4_memcpy(d,s,8); d+=8; s+=8; } while (d<e);
}


/*-************************************
*  Common functions
**************************************/
static unsigned LZ4_NbCommonBytes (reg_t val)
{
    assert(val != 0);
    if (LZ4_isLittleEndian()) {
        if (sizeof(val) == 8) {
#       if defined(_MSC_VER) && (_MSC_VER >= 1800) && (defined(_M_AMD64) && !defined(_M_ARM64EC)) && !defined(LZ4_FORCE_SW_BITCOUNT)
/*-*************************************************************************************************
* ARM64EC is a Microsoft-designed ARM64 ABI compatible with AMD64 applications on ARM64 Windows 11.
* The ARM64EC ABI does not support AVX/AVX2/AVX512 instructions, nor their relevant intrinsics
* including _tzcnt_u64. Therefore, we need to neuter the _tzcnt_u64 code path for ARM64EC.
****************************************************************************************************/
#         if defined(__clang__) && (__clang_major__ < 10)
            /* Avoid undefined clang-cl intrinsics issue.
             * See https://github.com/lz4/lz4/pull/1017 for details. */
            return (unsigned)__builtin_ia32_tzcnt_u64(val) >> 3;
#         else
            /* x64 CPUS without BMI support interpret `TZCNT` as `REP BSF` */
            return (unsigned)_tzcnt_u64(val) >> 3;
#         endif
#       elif defined(_MSC_VER) && defined(_WIN64) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r = 0;
            _BitScanForward64(&r, (U64)val);
            return (unsigned)r >> 3;
#       elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_ctzll((U64)val) >> 3;
#       else
            const U64 m = 0x0101010101010101ULL;
            val ^= val - 1;
            return (unsigned)(((U64)((val & (m - 1)) * m)) >> 56);
#       endif
        } else /* 32 bits */ {
#       if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(LZ4_FORCE_SW_BITCOUNT)
            unsigned long r;
            _BitScanForward(&r, (U32)val);
            return (unsigned)r >> 3;
#       elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                        !defined(__TINYC__) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_ctz((U32)val) >> 3;
#       else
            const U32 m = 0x01010101;
            return (unsigned)((((val - 1) ^ val) & (m - 1)) * m) >> 24;
#       endif
        }
    } else   /* Big Endian CPU */ {
        if (sizeof(val)==8) {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                        !defined(__TINYC__) && !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clzll((U64)val) >> 3;
#       else
#if 1
            /* this method is probably faster,
             * but adds a 128 bytes lookup table */
            static const unsigned char ctz7_tab[128] = {
                7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
                4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            };
            U64 const mask = 0x0101010101010101ULL;
            U64 const t = (((val >> 8) - mask) | val) & mask;
            return ctz7_tab[(t * 0x0080402010080402ULL) >> 57];
#else
            /* this method doesn't consume memory space like the previous one,
             * but it contains several branches,
             * that may end up slowing execution */
            static const U32 by32 = sizeof(val)*4;  /* 32 on 64 bits (goal), 16 on 32 bits.
            Just to avoid some static analyzer complaining about shift by 32 on 32-bits target.
            Note that this code path is never triggered in 32-bits mode. */
            unsigned r;
            if (!(val>>by32)) { r=4; } else { r=0; val>>=by32; }
            if (!(val>>16)) { r+=2; val>>=8; } else { val>>=24; }
            r += (!val);
            return r;
#endif
#       endif
        } else /* 32 bits */ {
#       if (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
            return (unsigned)__builtin_clz((U32)val) >> 3;
#       else
            val >>= 8;
            val = ((((val + 0x00FFFF00) | 0x00FFFFFF) + val) |
              (val + 0x00FF0000)) >> 24;
            return (unsigned)val ^ 3;
#       endif
        }
    }
}


#define STEPSIZE sizeof(reg_t)
LZ4_FORCE_INLINE
unsigned LZ4_count(const BYTE* pIn, const BYTE* pMatch, const BYTE* pInLimit)
{
    const BYTE* const pStart = pIn;

    if (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) {
            pIn+=STEPSIZE; pMatch+=STEPSIZE;
        } else {
            return LZ4_NbCommonBytes(diff);
    }   }

    while (likely(pIn < pInLimit-(STEPSIZE-1))) {
        reg_t const diff = LZ4_read_ARCH(pMatch) ^ LZ4_read_ARCH(pIn);
        if (!diff) { pIn+=STEPSIZE; pMatch+=STEPSIZE; continue; }
        pIn += LZ4_NbCommonBytes(diff);
        return (unsigned)(pIn - pStart);
    }

    if ((STEPSIZE==8) && (pIn<(pInLimit-3)) && (LZ4_read32(pMatch) == LZ4_read32(pIn))) { pIn+=4; pMatch+=4; }
    if ((pIn<(pInLimit-1)) && (LZ4_read16(pMatch) == LZ4_read16(pIn))) { pIn+=2; pMatch+=2; }
    if ((pIn<pInLimit) && (*pMatch == *pIn)) pIn++;
    return (unsigned)(pIn - pStart);
}


static void LZ4_writeLE16(void* memPtr, U16 value)
{
    if (LZ4_isLittleEndian()) {
        LZ4_write16(memPtr, value);
    } else {
        BYTE* p = (BYTE*)memPtr;
        p[0] = (BYTE) value;
        p[1] = (BYTE)(value>>8);
    }
}

/*===   Enums   ===*/
typedef enum { noDictCtx, usingDictCtxHc } dictCtx_directive;


/*===   Constants   ===*/
#define OPTIMAL_ML (int)((ML_MASK-1)+MINMATCH)
#define LZ4_OPT_NUM   (1<<12)


/*===   Macros   ===*/
#define MIN(a,b)   ( (a) < (b) ? (a) : (b) )
#define MAX(a,b)   ( (a) > (b) ? (a) : (b) )


/*===   Levels definition   ===*/
typedef enum { lz4mid, lz4hc, lz4opt } lz4hc_strat_e;
typedef struct {
    lz4hc_strat_e strat;
    int nbSearches;
    U32 targetLength;
} cParams_t;
static const cParams_t k_clTable[LZ4HC_CLEVEL_MAX+1] = {
    { lz4mid,    2, 16 },  /* 0, unused */
    { lz4mid,    2, 16 },  /* 1, unused */
    { lz4mid,    2, 16 },  /* 2 */
    { lz4hc,     4, 16 },  /* 3 */
    { lz4hc,     8, 16 },  /* 4 */
    { lz4hc,    16, 16 },  /* 5 */
    { lz4hc,    32, 16 },  /* 6 */
    { lz4hc,    64, 16 },  /* 7 */
    { lz4hc,   128, 16 },  /* 8 */
    { lz4hc,   256, 16 },  /* 9 */
    { lz4opt,   96, 64 },  /*10==LZ4HC_CLEVEL_OPT_MIN*/
    { lz4opt,  512,128 },  /*11 */
    { lz4opt,16384,LZ4_OPT_NUM },  /* 12==LZ4HC_CLEVEL_MAX */
};

static cParams_t LZ4HC_getCLevelParams(int cLevel)
{
    /* note : clevel convention is a bit different from lz4frame,
     * possibly something worth revisiting for consistency */
    if (cLevel < 1)
        cLevel = LZ4HC_CLEVEL_DEFAULT;
    cLevel = MIN(LZ4HC_CLEVEL_MAX, cLevel);
    return k_clTable[cLevel];
}


/*===   Hashing   ===*/
#define LZ4HC_HASHSIZE 4
#define HASH_FUNCTION(i)      (((i) * 2654435761U) >> ((MINMATCH*8)-LZ4HC_HASH_LOG))
static U32 LZ4HC_hashPtr(const void* ptr) { return HASH_FUNCTION(LZ4_read32(ptr)); }

#if defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==2)
/* lie to the compiler about data alignment; use with caution */
static U64 LZ4_read64(const void* memPtr) { return *(const U64*) memPtr; }

#elif defined(LZ4_FORCE_MEMORY_ACCESS) && (LZ4_FORCE_MEMORY_ACCESS==1)
/* __pack instructions are safer, but compiler specific */
LZ4_PACK(typedef struct { U64 u64; }) LZ4_unalign64;
static U64 LZ4_read64(const void* ptr) { return ((const LZ4_unalign64*)ptr)->u64; }

#else  /* safe and portable access using memcpy() */
static U64 LZ4_read64(const void* memPtr)
{
    U64 val; LZ4_memcpy(&val, memPtr, sizeof(val)); return val;
}

#endif /* LZ4_FORCE_MEMORY_ACCESS */

#define LZ4MID_HASHSIZE 8
#define LZ4MID_HASHLOG (LZ4HC_HASH_LOG-1)
#define LZ4MID_HASHTABLESIZE (1 << LZ4MID_HASHLOG)

static U32 LZ4MID_hash4(U32 v) { return (v * 2654435761U) >> (32-LZ4MID_HASHLOG); }
static U32 LZ4MID_hash4Ptr(const void* ptr) { return LZ4MID_hash4(LZ4_read32(ptr)); }
/* note: hash7 hashes the lower 56-bits.
 * It presumes input was read using little endian.*/
static U32 LZ4MID_hash7(U64 v) { return (U32)(((v  << (64-56)) * 58295818150454627ULL) >> (64-LZ4MID_HASHLOG)) ; }
static U64 LZ4_readLE64(const void* memPtr);
static U32 LZ4MID_hash8Ptr(const void* ptr) { return LZ4MID_hash7(LZ4_readLE64(ptr)); }

static U64 LZ4_readLE64(const void* memPtr)
{
    if (LZ4_isLittleEndian()) {
        return LZ4_read64(memPtr);
    } else {
        const BYTE* p = (const BYTE*)memPtr;
        /* note: relies on the compiler to simplify this expression */
        return (U64)p[0] | ((U64)p[1]<<8) | ((U64)p[2]<<16) | ((U64)p[3]<<24)
            | ((U64)p[4]<<32) | ((U64)p[5]<<40) | ((U64)p[6]<<48) | ((U64)p[7]<<56);
    }
}


/*===   Count match length   ===*/
LZ4_FORCE_INLINE
unsigned LZ4HC_NbCommonBytes32(U32 val)
{
    assert(val != 0);
    if (LZ4_isLittleEndian()) {
#     if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(LZ4_FORCE_SW_BITCOUNT)
        unsigned long r;
        _BitScanReverse(&r, val);
        return (unsigned)((31 - r) >> 3);
#     elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
        return (unsigned)__builtin_clz(val) >> 3;
#     else
        val >>= 8;
        val = ((((val + 0x00FFFF00) | 0x00FFFFFF) + val) |
              (val + 0x00FF0000)) >> 24;
        return (unsigned)val ^ 3;
#     endif
    } else {
#     if defined(_MSC_VER) && (_MSC_VER >= 1400) && !defined(LZ4_FORCE_SW_BITCOUNT)
        unsigned long r;
        _BitScanForward(&r, val);
        return (unsigned)(r >> 3);
#     elif (defined(__clang__) || (defined(__GNUC__) && ((__GNUC__ > 3) || \
                            ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4))))) && \
                                        !defined(LZ4_FORCE_SW_BITCOUNT)
        return (unsigned)__builtin_ctz(val) >> 3;
#     else
        const U32 m = 0x01010101;
        return (unsigned)((((val - 1) ^ val) & (m - 1)) * m) >> 24;
#     endif
    }
}

/** LZ4HC_countBack() :
 * @return : negative value, nb of common bytes before ip/match */
LZ4_FORCE_INLINE
int LZ4HC_countBack(const BYTE* const ip, const BYTE* const match,
                    const BYTE* const iMin, const BYTE* const mMin)
{
    int back = 0;
    int const min = (int)MAX(iMin - ip, mMin - match);
    assert(min <= 0);
    assert(ip >= iMin); assert((size_t)(ip-iMin) < (1U<<31));
    assert(match >= mMin); assert((size_t)(match - mMin) < (1U<<31));

    while ((back - min) > 3) {
        U32 const v = LZ4_read32(ip + back - 4) ^ LZ4_read32(match + back - 4);
        if (v) {
            return (back - (int)LZ4HC_NbCommonBytes32(v));
        } else back -= 4; /* 4-byte step */
    }
    /* check remainder if any */
    while ( (back > min)
         && (ip[back-1] == match[back-1]) )
            back--;
    return back;
}

/*===   Chain table updates   ===*/
#define DELTANEXTU16(table, pos) table[(U16)(pos)]   /* faster */
/* Make fields passed to, and updated by LZ4HC_encodeSequence explicit */
#define UPDATABLE(ip, op, anchor) &ip, &op, &anchor


/**************************************
*  Init
**************************************/
static void LZ4HC_clearTables (LZ4HC_CCtx_internal* hc4)
{
    MEM_INIT(hc4->hashTable, 0, sizeof(hc4->hashTable));
    MEM_INIT(hc4->chainTable, 0xFF, sizeof(hc4->chainTable));
}

static void LZ4HC_init_internal (LZ4HC_CCtx_internal* hc4, const BYTE* start)
{
    size_t const bufferSize = (size_t)(hc4->end - hc4->prefixStart);
    size_t newStartingOffset = bufferSize + hc4->dictLimit;
    DEBUGLOG(5, "LZ4HC_init_internal");
    assert(newStartingOffset >= bufferSize);  /* check overflow */
    if (newStartingOffset > 1 GB) {
        LZ4HC_clearTables(hc4);
        newStartingOffset = 0;
    }
    newStartingOffset += 64 KB;
    hc4->nextToUpdate = (U32)newStartingOffset;
    hc4->prefixStart = start;
    hc4->end = start;
    hc4->dictStart = start;
    hc4->dictLimit = (U32)newStartingOffset;
    hc4->lowLimit = (U32)newStartingOffset;
}


/**************************************
*  Encode
**************************************/
/* LZ4HC_encodeSequence() :
 * @return : 0 if ok,
 *           1 if buffer issue detected */
LZ4_FORCE_INLINE int LZ4HC_encodeSequence (
    const BYTE** _ip,
    BYTE** _op,
    const BYTE** _anchor,
    int matchLength,
    int offset,
    limitedOutput_directive limit,
    BYTE* oend)
{
#define ip      (*_ip)
#define op      (*_op)
#define anchor  (*_anchor)

    size_t length;
    BYTE* const token = op++;

#if defined(LZ4_DEBUG) && (LZ4_DEBUG >= 6)
    static const BYTE* start = NULL;
    static U32 totalCost = 0;
    U32 const pos = (start==NULL) ? 0 : (U32)(anchor - start);
    U32 const ll = (U32)(ip - anchor);
    U32 const llAdd = (ll>=15) ? ((ll-15) / 255) + 1 : 0;
    U32 const mlAdd = (matchLength>=19) ? ((matchLength-19) / 255) + 1 : 0;
    U32 const cost = 1 + llAdd + ll + 2 + mlAdd;
    if (start==NULL) start = anchor;  /* only works for single segment */
    /* g_debuglog_enable = (pos >= 2228) & (pos <= 2262); */
    DEBUGLOG(6, "pos:%7u -- literals:%4u, match:%4i, offset:%5i, cost:%4u + %5u",
                pos,
                (U32)(ip - anchor), matchLength, offset,
                cost, totalCost);
    totalCost += cost;
#endif

    /* Encode Literal length */
    length = (size_t)(ip - anchor);
    LZ4_STATIC_ASSERT(notLimited == 0);
    /* Check output limit */
    if (limit && ((op + (length / 255) + length + (2 + 1 + LASTLITERALS)) > oend)) {
        DEBUGLOG(6, "Not enough room to write %i literals (%i bytes remaining)",
                (int)length, (int)(oend - op));
        return 1;
    }
    if (length >= RUN_MASK) {
        size_t len = length - RUN_MASK;
        *token = (RUN_MASK << ML_BITS);
        for(; len >= 255 ; len -= 255) *op++ = 255;
        *op++ = (BYTE)len;
    } else {
        *token = (BYTE)(length << ML_BITS);
    }

    /* Copy Literals */
    LZ4_wildCopy8(op, anchor, op + length);
    op += length;

    /* Encode Offset */
    assert(offset <= LZ4_DISTANCE_MAX );
    assert(offset > 0);
    LZ4_writeLE16(op, (U16)(offset)); op += 2;

    /* Encode MatchLength */
    assert(matchLength >= MINMATCH);
    length = (size_t)matchLength - MINMATCH;
    if (limit && (op + (length / 255) + (1 + LASTLITERALS) > oend)) {
        DEBUGLOG(6, "Not enough room to write match length");
        return 1;   /* Check output limit */
    }
    if (length >= ML_MASK) {
        *token += ML_MASK;
        length -= ML_MASK;
        for(; length >= 510 ; length -= 510) { *op++ = 255; *op++ = 255; }
        if (length >= 255) { length -= 255; *op++ = 255; }
        *op++ = (BYTE)length;
    } else {
        *token += (BYTE)(length);
    }

    /* Prepare next loop */
    ip += matchLength;
    anchor = ip;

    return 0;

#undef ip
#undef op
#undef anchor
}


typedef struct {
    int off;
    int len;
    int back;  /* negative value */
} LZ4HC_match_t;

LZ4HC_match_t LZ4HC_searchExtDict(const BYTE* ip, U32 ipIndex,
        const BYTE* const iLowLimit, const BYTE* const iHighLimit,
        const LZ4HC_CCtx_internal* dictCtx, U32 gDictEndIndex,
        int currentBestML, int nbAttempts)
{
    size_t const lDictEndIndex = (size_t)(dictCtx->end - dictCtx->prefixStart) + dictCtx->dictLimit;
    U32 lDictMatchIndex = dictCtx->hashTable[LZ4HC_hashPtr(ip)];
    U32 matchIndex = lDictMatchIndex + gDictEndIndex - (U32)lDictEndIndex;
    int offset = 0, sBack = 0;
    assert(lDictEndIndex <= 1 GB);
    if (lDictMatchIndex>0)
        DEBUGLOG(7, "lDictEndIndex = %zu, lDictMatchIndex = %u", lDictEndIndex, lDictMatchIndex);
    while (ipIndex - matchIndex <= LZ4_DISTANCE_MAX && nbAttempts--) {
        const BYTE* const matchPtr = dictCtx->prefixStart - dictCtx->dictLimit + lDictMatchIndex;

        if (LZ4_read32(matchPtr) == LZ4_read32(ip)) {
            int mlt;
            int back = 0;
            const BYTE* vLimit = ip + (lDictEndIndex - lDictMatchIndex);
            if (vLimit > iHighLimit) vLimit = iHighLimit;
            mlt = (int)LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, vLimit) + MINMATCH;
            back = (ip > iLowLimit) ? LZ4HC_countBack(ip, matchPtr, iLowLimit, dictCtx->prefixStart) : 0;
            mlt -= back;
            if (mlt > currentBestML) {
                currentBestML = mlt;
                offset = (int)(ipIndex - matchIndex);
                sBack = back;
                DEBUGLOG(7, "found match of length %i within extDictCtx", currentBestML);
        }   }

        {   U32 const nextOffset = DELTANEXTU16(dictCtx->chainTable, lDictMatchIndex);
            lDictMatchIndex -= nextOffset;
            matchIndex -= nextOffset;
    }   }

    {   LZ4HC_match_t md;
        md.len = currentBestML;
        md.off = offset;
        md.back = sBack;
        return md;
    }
}

typedef LZ4HC_match_t (*LZ4MID_searchIntoDict_f)(const BYTE* ip, U32 ipIndex,
        const BYTE* const iHighLimit,
        const LZ4HC_CCtx_internal* dictCtx, U32 gDictEndIndex);

static LZ4HC_match_t LZ4MID_searchHCDict(const BYTE* ip, U32 ipIndex,
        const BYTE* const iHighLimit,
        const LZ4HC_CCtx_internal* dictCtx, U32 gDictEndIndex)
{
    return LZ4HC_searchExtDict(ip,ipIndex,
                            ip, iHighLimit,
                            dictCtx, gDictEndIndex,
                            MINMATCH-1, 2);
}

static LZ4HC_match_t LZ4MID_searchExtDict(const BYTE* ip, U32 ipIndex,
        const BYTE* const iHighLimit,
        const LZ4HC_CCtx_internal* dictCtx, U32 gDictEndIndex)
{
    size_t const lDictEndIndex = (size_t)(dictCtx->end - dictCtx->prefixStart) + dictCtx->dictLimit;
    const U32* const hash4Table = dictCtx->hashTable;
    const U32* const hash8Table = hash4Table + LZ4MID_HASHTABLESIZE;
    DEBUGLOG(7, "LZ4MID_searchExtDict (ipIdx=%u)", ipIndex);

    /* search long match first */
    {   U32 l8DictMatchIndex = hash8Table[LZ4MID_hash8Ptr(ip)];
        U32 m8Index = l8DictMatchIndex + gDictEndIndex - (U32)lDictEndIndex;
        assert(lDictEndIndex <= 1 GB);
        if (ipIndex - m8Index <= LZ4_DISTANCE_MAX) {
            const BYTE* const matchPtr = dictCtx->prefixStart - dictCtx->dictLimit + l8DictMatchIndex;
            const size_t safeLen = MIN(lDictEndIndex - l8DictMatchIndex, (size_t)(iHighLimit - ip));
            int mlt = (int)LZ4_count(ip, matchPtr, ip + safeLen);
            if (mlt >= MINMATCH) {
                LZ4HC_match_t md;
                DEBUGLOG(7, "Found long ExtDict match of len=%u", mlt);
                md.len = mlt;
                md.off = (int)(ipIndex - m8Index);
                md.back = 0;
                return md;
            }
        }
    }

    /* search for short match second */
    {   U32 l4DictMatchIndex = hash4Table[LZ4MID_hash4Ptr(ip)];
        U32 m4Index = l4DictMatchIndex + gDictEndIndex - (U32)lDictEndIndex;
        if (ipIndex - m4Index <= LZ4_DISTANCE_MAX) {
            const BYTE* const matchPtr = dictCtx->prefixStart - dictCtx->dictLimit + l4DictMatchIndex;
            const size_t safeLen = MIN(lDictEndIndex - l4DictMatchIndex, (size_t)(iHighLimit - ip));
            int mlt = (int)LZ4_count(ip, matchPtr, ip + safeLen);
            if (mlt >= MINMATCH) {
                LZ4HC_match_t md;
                DEBUGLOG(7, "Found short ExtDict match of len=%u", mlt);
                md.len = mlt;
                md.off = (int)(ipIndex - m4Index);
                md.back = 0;
                return md;
            }
        }
    }

    /* nothing found */
    {   LZ4HC_match_t const md = {0, 0, 0 };
        return md;
    }
}

/**************************************
*  Mid Compression (level 2)
**************************************/

LZ4_FORCE_INLINE void
LZ4MID_addPosition(U32* hTable, U32 hValue, U32 index)
{
    hTable[hValue] = index;
}

#define ADDPOS8(_p, _idx) LZ4MID_addPosition(hash8Table, LZ4MID_hash8Ptr(_p), _idx)
#define ADDPOS4(_p, _idx) LZ4MID_addPosition(hash4Table, LZ4MID_hash4Ptr(_p), _idx)

/* Fill hash tables with references into dictionary.
 * The resulting table is only exploitable by LZ4MID (level 2) */
static void
LZ4MID_fillHTable (LZ4HC_CCtx_internal* cctx, const void* dict, size_t size)
{
    U32* const hash4Table = cctx->hashTable;
    U32* const hash8Table = hash4Table + LZ4MID_HASHTABLESIZE;
    const BYTE* const prefixPtr = (const BYTE*)dict;
    U32 const prefixIdx = cctx->dictLimit;
    U32 const target = prefixIdx + (U32)size - LZ4MID_HASHSIZE;
    U32 idx = cctx->nextToUpdate;
    assert(dict == cctx->prefixStart);
    DEBUGLOG(4, "LZ4MID_fillHTable (size:%zu)", size);
    if (size <= LZ4MID_HASHSIZE)
        return;

    for (; idx < target; idx += 3) {
        ADDPOS4(prefixPtr+idx-prefixIdx, idx);
        ADDPOS8(prefixPtr+idx+1-prefixIdx, idx+1);
    }

    idx = (size > 32 KB + LZ4MID_HASHSIZE) ? target - 32 KB : cctx->nextToUpdate;
    for (; idx < target; idx += 1) {
        ADDPOS8(prefixPtr+idx-prefixIdx, idx);
    }

    cctx->nextToUpdate = target;
}

static LZ4MID_searchIntoDict_f select_searchDict_function(const LZ4HC_CCtx_internal* dictCtx)
{
    if (dictCtx == NULL) return NULL;
    if (LZ4HC_getCLevelParams(dictCtx->compressionLevel).strat == lz4mid)
        return LZ4MID_searchExtDict;
    return LZ4MID_searchHCDict;
}

/* preconditions:
 * - *srcSizePtr within [1, LZ4_MAX_INPUT_SIZE]
 * - src is valid
 * - maxOutputSize >= 1
 * - dst is valid
 */
static int LZ4MID_compress (
    LZ4HC_CCtx_internal* const ctx,
    const char* const src,
    char* const dst,
    int* srcSizePtr,
    int const maxOutputSize,
    const limitedOutput_directive limit,
    const dictCtx_directive dict
    )
{
    U32* const hash4Table = ctx->hashTable;
    U32* const hash8Table = hash4Table + LZ4MID_HASHTABLESIZE;
    const BYTE* ip = (const BYTE*)src;
    const BYTE* anchor = ip;
    const BYTE* const iend = ip + *srcSizePtr;
    const BYTE* const mflimit = iend - MFLIMIT;
    const BYTE* const matchlimit = (iend - LASTLITERALS);
    const BYTE* const ilimit = (iend - LZ4MID_HASHSIZE);
    BYTE* op = (BYTE*)dst;
    BYTE* oend = op + maxOutputSize;

    const BYTE* const prefixPtr = ctx->prefixStart;
    const U32 prefixIdx = ctx->dictLimit;
    const U32 ilimitIdx = (U32)(ilimit - prefixPtr) + prefixIdx;
    const BYTE* const dictStart = ctx->dictStart;
    const U32 dictIdx = ctx->lowLimit;
    const U32 gDictEndIndex = ctx->lowLimit;
    const LZ4MID_searchIntoDict_f searchIntoDict = (dict == usingDictCtxHc) ? select_searchDict_function(ctx->dictCtx) : NULL;
    unsigned matchLength;
    unsigned matchDistance;

    DEBUGLOG(5, "LZ4MID_compress (%i bytes)", *srcSizePtr);

    /* preconditions verifications */
    if (dict == usingDictCtxHc) DEBUGLOG(5, "usingDictCtxHc");
    assert(*srcSizePtr > 0);
    assert(*srcSizePtr <= LZ4_MAX_INPUT_SIZE);
    assert(src != NULL);
    assert(maxOutputSize >= 1);
    assert(dst != NULL);

    if (limit == fillOutput) oend -= LASTLITERALS;  /* Hack for support LZ4 format restriction */
    if (*srcSizePtr < LZ4_minLength)
        goto _lz4mid_last_literals;  /* Input too small, no compression (all literals) */

    /* main loop */
    while (ip <= mflimit) {
        const U32 ipIndex = (U32)(ip - prefixPtr) + prefixIdx;
        /* search long match */
        {   U32 const h8 = LZ4MID_hash8Ptr(ip);
            U32 const pos8 = hash8Table[h8];
            assert(h8 < LZ4MID_HASHTABLESIZE);
            assert(pos8 < ipIndex);
            LZ4MID_addPosition(hash8Table, h8, ipIndex);
            if (ipIndex - pos8 <= LZ4_DISTANCE_MAX) {
                /* match candidate found */
                if (pos8 >= prefixIdx) {
                    const BYTE* const matchPtr = prefixPtr + pos8 - prefixIdx;
                    assert(matchPtr < ip);
                    matchLength = LZ4_count(ip, matchPtr, matchlimit);
                    if (matchLength >= MINMATCH) {
                        DEBUGLOG(7, "found long match at pos %u (len=%u)", pos8, matchLength);
                        matchDistance = ipIndex - pos8;
                        goto _lz4mid_encode_sequence;
                    }
                } else {
                    if (pos8 >= dictIdx) {
                        /* extDict match candidate */
                        const BYTE* const matchPtr = dictStart + (pos8 - dictIdx);
                        const size_t safeLen = MIN(prefixIdx - pos8, (size_t)(matchlimit - ip));
                        matchLength = LZ4_count(ip, matchPtr, ip + safeLen);
                        if (matchLength >= MINMATCH) {
                            DEBUGLOG(7, "found long match at ExtDict pos %u (len=%u)", pos8, matchLength);
                            matchDistance = ipIndex - pos8;
                            goto _lz4mid_encode_sequence;
                        }
                    }
                }
        }   }
        /* search short match */
        {   U32 const h4 = LZ4MID_hash4Ptr(ip);
            U32 const pos4 = hash4Table[h4];
            assert(h4 < LZ4MID_HASHTABLESIZE);
            assert(pos4 < ipIndex);
            LZ4MID_addPosition(hash4Table, h4, ipIndex);
            if (ipIndex - pos4 <= LZ4_DISTANCE_MAX) {
                /* match candidate found */
                if (pos4 >= prefixIdx) {
                /* only search within prefix */
                    const BYTE* const matchPtr = prefixPtr + (pos4 - prefixIdx);
                    assert(matchPtr < ip);
                    assert(matchPtr >= prefixPtr);
                    matchLength = LZ4_count(ip, matchPtr, matchlimit);
                    if (matchLength >= MINMATCH) {
                        /* short match found, let's just check ip+1 for longer */
                        U32 const h8 = LZ4MID_hash8Ptr(ip+1);
                        U32 const pos8 = hash8Table[h8];
                        U32 const m2Distance = ipIndex + 1 - pos8;
                        matchDistance = ipIndex - pos4;
                        if ( m2Distance <= LZ4_DISTANCE_MAX
                        && pos8 >= prefixIdx /* only search within prefix */
                        && likely(ip < mflimit)
                        ) {
                            const BYTE* const m2Ptr = prefixPtr + (pos8 - prefixIdx);
                            unsigned ml2 = LZ4_count(ip+1, m2Ptr, matchlimit);
                            if (ml2 > matchLength) {
                                LZ4MID_addPosition(hash8Table, h8, ipIndex+1);
                                ip++;
                                matchLength = ml2;
                                matchDistance = m2Distance;
                        }   }
                        goto _lz4mid_encode_sequence;
                    }
                } else {
                    if (pos4 >= dictIdx) {
                        /* extDict match candidate */
                        const BYTE* const matchPtr = dictStart + (pos4 - dictIdx);
                        const size_t safeLen = MIN(prefixIdx - pos4, (size_t)(matchlimit - ip));
                        matchLength = LZ4_count(ip, matchPtr, ip + safeLen);
                        if (matchLength >= MINMATCH) {
                            DEBUGLOG(7, "found match at ExtDict pos %u (len=%u)", pos4, matchLength);
                            matchDistance = ipIndex - pos4;
                            goto _lz4mid_encode_sequence;
                        }
                    }
                }
        }   }
        /* no match found in prefix */
        if ( (dict == usingDictCtxHc)
          && (ipIndex - gDictEndIndex < LZ4_DISTANCE_MAX - 8) ) {
            /* search a match into external dictionary */
            LZ4HC_match_t dMatch = searchIntoDict(ip, ipIndex,
                    matchlimit,
                    ctx->dictCtx, gDictEndIndex);
            if (dMatch.len >= MINMATCH) {
                DEBUGLOG(7, "found Dictionary match (offset=%i)", dMatch.off);
                assert(dMatch.back == 0);
                matchLength = (unsigned)dMatch.len;
                matchDistance = (unsigned)dMatch.off;
                goto _lz4mid_encode_sequence;
            }
        }
        /* no match found */
        ip += 1 + ((ip-anchor) >> 9);  /* skip faster over incompressible data */
        continue;

_lz4mid_encode_sequence:
        /* catch back */
        while (((ip > anchor) & ((U32)(ip-prefixPtr) > matchDistance)) && (unlikely(ip[-1] == ip[-(int)matchDistance-1]))) {
            ip--;  matchLength++;
        };

        /* fill table with beginning of match */
        ADDPOS8(ip+1, ipIndex+1);
        ADDPOS8(ip+2, ipIndex+2);
        ADDPOS4(ip+1, ipIndex+1);

        /* encode */
        {   BYTE* const saved_op = op;
            /* LZ4HC_encodeSequence always updates @op; on success, it updates @ip and @anchor */
            if (LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                    (int)matchLength, (int)matchDistance,
                    limit, oend) ) {
                op = saved_op;  /* restore @op value before failed LZ4HC_encodeSequence */
                goto _lz4mid_dest_overflow;
            }
        }

        /* fill table with end of match */
        {   U32 endMatchIdx = (U32)(ip-prefixPtr) + prefixIdx;
            U32 pos_m2 = endMatchIdx - 2;
            if (pos_m2 < ilimitIdx) {
                if (likely(ip - prefixPtr > 5)) {
                    ADDPOS8(ip-5, endMatchIdx - 5);
                }
                ADDPOS8(ip-3, endMatchIdx - 3);
                ADDPOS8(ip-2, endMatchIdx - 2);
                ADDPOS4(ip-2, endMatchIdx - 2);
                ADDPOS4(ip-1, endMatchIdx - 1);
            }
        }
    }

_lz4mid_last_literals:
    /* Encode Last Literals */
    {   size_t lastRunSize = (size_t)(iend - anchor);  /* literals */
        size_t llAdd = (lastRunSize + 255 - RUN_MASK) / 255;
        size_t const totalSize = 1 + llAdd + lastRunSize;
        if (limit == fillOutput) oend += LASTLITERALS;  /* restore correct value */
        if (limit && (op + totalSize > oend)) {
            if (limit == limitedOutput) return 0;  /* not enough space in @dst */
            /* adapt lastRunSize to fill 'dest' */
            lastRunSize  = (size_t)(oend - op) - 1 /*token*/;
            llAdd = (lastRunSize + 256 - RUN_MASK) / 256;
            lastRunSize -= llAdd;
        }
        DEBUGLOG(6, "Final literal run : %i literals", (int)lastRunSize);
        ip = anchor + lastRunSize;  /* can be != iend if limit==fillOutput */

        if (lastRunSize >= RUN_MASK) {
            size_t accumulator = lastRunSize - RUN_MASK;
            *op++ = (RUN_MASK << ML_BITS);
            for(; accumulator >= 255 ; accumulator -= 255)
                *op++ = 255;
            *op++ = (BYTE) accumulator;
        } else {
            *op++ = (BYTE)(lastRunSize << ML_BITS);
        }
        assert(lastRunSize <= (size_t)(oend - op));
        LZ4_memcpy(op, anchor, lastRunSize);
        op += lastRunSize;
    }

    /* End */
    DEBUGLOG(5, "compressed %i bytes into %i bytes", *srcSizePtr, (int)((char*)op - dst));
    assert(ip >= (const BYTE*)src);
    assert(ip <= iend);
    *srcSizePtr = (int)(ip - (const BYTE*)src);
    assert((char*)op >= dst);
    assert(op <= oend);
    assert((char*)op - dst < INT_MAX);
    return (int)((char*)op - dst);

_lz4mid_dest_overflow:
    if (limit == fillOutput) {
        /* Assumption : @ip, @anchor, @optr and @matchLength must be set correctly */
        size_t const ll = (size_t)(ip - anchor);
        size_t const ll_addbytes = (ll + 240) / 255;
        size_t const ll_totalCost = 1 + ll_addbytes + ll;
        BYTE* const maxLitPos = oend - 3; /* 2 for offset, 1 for token */
        DEBUGLOG(6, "Last sequence is overflowing : %u literals, %u remaining space",
                (unsigned)ll, (unsigned)(oend-op));
        if (op + ll_totalCost <= maxLitPos) {
            /* ll validated; now adjust match length */
            size_t const bytesLeftForMl = (size_t)(maxLitPos - (op+ll_totalCost));
            size_t const maxMlSize = MINMATCH + (ML_MASK-1) + (bytesLeftForMl * 255);
            assert(maxMlSize < INT_MAX);
            if ((size_t)matchLength > maxMlSize) matchLength= (unsigned)maxMlSize;
            if ((oend + LASTLITERALS) - (op + ll_totalCost + 2) - 1 + matchLength >= MFLIMIT) {
            DEBUGLOG(6, "Let's encode a last sequence (ll=%u, ml=%u)", (unsigned)ll, matchLength);
                LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                        (int)matchLength, (int)matchDistance,
                        notLimited, oend);
        }   }
        DEBUGLOG(6, "Let's finish with a run of literals (%u bytes left)", (unsigned)(oend-op));
        goto _lz4mid_last_literals;
    }
    /* compression failed */
    return 0;
}


/**************************************
*  HC Compression - Search
**************************************/

/* Update chains up to ip (excluded) */
LZ4_FORCE_INLINE void LZ4HC_Insert (LZ4HC_CCtx_internal* hc4, const BYTE* ip)
{
    U16* const chainTable = hc4->chainTable;
    U32* const hashTable  = hc4->hashTable;
    const BYTE* const prefixPtr = hc4->prefixStart;
    U32 const prefixIdx = hc4->dictLimit;
    U32 const target = (U32)(ip - prefixPtr) + prefixIdx;
    U32 idx = hc4->nextToUpdate;
    assert(ip >= prefixPtr);
    assert(target >= prefixIdx);

    while (idx < target) {
        U32 const h = LZ4HC_hashPtr(prefixPtr+idx-prefixIdx);
        size_t delta = idx - hashTable[h];
        if (delta>LZ4_DISTANCE_MAX) delta = LZ4_DISTANCE_MAX;
        DELTANEXTU16(chainTable, idx) = (U16)delta;
        hashTable[h] = idx;
        idx++;
    }

    hc4->nextToUpdate = target;
}

#if defined(_MSC_VER)
#  define LZ4HC_rotl32(x,r) _rotl(x,r)
#else
#  define LZ4HC_rotl32(x,r) ((x << r) | (x >> (32 - r)))
#endif


static U32 LZ4HC_rotatePattern(size_t const rotate, U32 const pattern)
{
    size_t const bitsToRotate = (rotate & (sizeof(pattern) - 1)) << 3;
    if (bitsToRotate == 0) return pattern;
    return LZ4HC_rotl32(pattern, (int)bitsToRotate);
}

/* LZ4HC_countPattern() :
 * pattern32 must be a sample of repetitive pattern of length 1, 2 or 4 (but not 3!) */
static unsigned
LZ4HC_countPattern(const BYTE* ip, const BYTE* const iEnd, U32 const pattern32)
{
    const BYTE* const iStart = ip;
    reg_t const pattern = (sizeof(pattern)==8) ?
        (reg_t)pattern32 + (((reg_t)pattern32) << (sizeof(pattern)*4)) : pattern32;

    while (likely(ip < iEnd-(sizeof(pattern)-1))) {
        reg_t const diff = LZ4_read_ARCH(ip) ^ pattern;
        if (!diff) { ip+=sizeof(pattern); continue; }
        ip += LZ4_NbCommonBytes(diff);
        return (unsigned)(ip - iStart);
    }

    if (LZ4_isLittleEndian()) {
        reg_t patternByte = pattern;
        while ((ip<iEnd) && (*ip == (BYTE)patternByte)) {
            ip++; patternByte >>= 8;
        }
    } else {  /* big endian */
        U32 bitOffset = (sizeof(pattern)*8) - 8;
        while (ip < iEnd) {
            BYTE const byte = (BYTE)(pattern >> bitOffset);
            if (*ip != byte) break;
            ip ++; bitOffset -= 8;
    }   }

    return (unsigned)(ip - iStart);
}

/* LZ4HC_reverseCountPattern() :
 * pattern must be a sample of repetitive pattern of length 1, 2 or 4 (but not 3!)
 * read using natural platform endianness */
static unsigned
LZ4HC_reverseCountPattern(const BYTE* ip, const BYTE* const iLow, U32 pattern)
{
    const BYTE* const iStart = ip;

    while (likely(ip >= iLow+4)) {
        if (LZ4_read32(ip-4) != pattern) break;
        ip -= 4;
    }
    {   const BYTE* bytePtr = (const BYTE*)(&pattern) + 3; /* works for any endianness */
        while (likely(ip>iLow)) {
            if (ip[-1] != *bytePtr) break;
            ip--; bytePtr--;
    }   }
    return (unsigned)(iStart - ip);
}

/* LZ4HC_protectDictEnd() :
 * Checks if the match is in the last 3 bytes of the dictionary, so reading the
 * 4 byte MINMATCH would overflow.
 * @returns true if the match index is okay.
 */
static int LZ4HC_protectDictEnd(U32 const dictLimit, U32 const matchIndex)
{
    return ((U32)((dictLimit - 1) - matchIndex) >= 3);
}

typedef enum { rep_untested, rep_not, rep_confirmed } repeat_state_e;
typedef enum { favorCompressionRatio=0, favorDecompressionSpeed } HCfavor_e;


LZ4_FORCE_INLINE LZ4HC_match_t
LZ4HC_InsertAndGetWiderMatch (
        LZ4HC_CCtx_internal* const hc4,
        const BYTE* const ip,
        const BYTE* const iLowLimit, const BYTE* const iHighLimit,
        int longest,
        const int maxNbAttempts,
        const int patternAnalysis, const int chainSwap,
        const dictCtx_directive dict,
        const HCfavor_e favorDecSpeed)
{
    U16* const chainTable = hc4->chainTable;
    U32* const hashTable = hc4->hashTable;
    const LZ4HC_CCtx_internal* const dictCtx = hc4->dictCtx;
    const BYTE* const prefixPtr = hc4->prefixStart;
    const U32 prefixIdx = hc4->dictLimit;
    const U32 ipIndex = (U32)(ip - prefixPtr) + prefixIdx;
    const int withinStartDistance = (hc4->lowLimit + (LZ4_DISTANCE_MAX + 1) > ipIndex);
    const U32 lowestMatchIndex = (withinStartDistance) ? hc4->lowLimit : ipIndex - LZ4_DISTANCE_MAX;
    const BYTE* const dictStart = hc4->dictStart;
    const U32 dictIdx = hc4->lowLimit;
    const BYTE* const dictEnd = dictStart + prefixIdx - dictIdx;
    int const lookBackLength = (int)(ip-iLowLimit);
    int nbAttempts = maxNbAttempts;
    U32 matchChainPos = 0;
    U32 const pattern = LZ4_read32(ip);
    U32 matchIndex;
    repeat_state_e repeat = rep_untested;
    size_t srcPatternLength = 0;
    int offset = 0, sBack = 0;

    DEBUGLOG(7, "LZ4HC_InsertAndGetWiderMatch");
    /* First Match */
    LZ4HC_Insert(hc4, ip);  /* insert all prior positions up to ip (excluded) */
    matchIndex = hashTable[LZ4HC_hashPtr(ip)];
    DEBUGLOG(7, "First candidate match for pos %u found at index %u / %u (lowestMatchIndex)",
                ipIndex, matchIndex, lowestMatchIndex);

    while ((matchIndex>=lowestMatchIndex) && (nbAttempts>0)) {
        int matchLength=0;
        nbAttempts--;
        assert(matchIndex < ipIndex);
        if (favorDecSpeed && (ipIndex - matchIndex < 8)) {
            /* do nothing:
             * favorDecSpeed intentionally skips matches with offset < 8 */
        } else if (matchIndex >= prefixIdx) {   /* within current Prefix */
            const BYTE* const matchPtr = prefixPtr + (matchIndex - prefixIdx);
            assert(matchPtr < ip);
            assert(longest >= 1);
            if (LZ4_read16(iLowLimit + longest - 1) == LZ4_read16(matchPtr - lookBackLength + longest - 1)) {
                if (LZ4_read32(matchPtr) == pattern) {
                    int const back = lookBackLength ? LZ4HC_countBack(ip, matchPtr, iLowLimit, prefixPtr) : 0;
                    matchLength = MINMATCH + (int)LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, iHighLimit);
                    matchLength -= back;
                    if (matchLength > longest) {
                        longest = matchLength;
                        offset = (int)(ipIndex - matchIndex);
                        sBack = back;
                        DEBUGLOG(7, "Found match of len=%i within prefix, offset=%i, back=%i", longest, offset, -back);
            }   }   }
        } else {   /* lowestMatchIndex <= matchIndex < dictLimit : within Ext Dict */
            const BYTE* const matchPtr = dictStart + (matchIndex - dictIdx);
            assert(matchIndex >= dictIdx);
            if ( likely(matchIndex <= prefixIdx - 4)
              && (LZ4_read32(matchPtr) == pattern) ) {
                int back = 0;
                const BYTE* vLimit = ip + (prefixIdx - matchIndex);
                if (vLimit > iHighLimit) vLimit = iHighLimit;
                matchLength = (int)LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, vLimit) + MINMATCH;
                if ((ip+matchLength == vLimit) && (vLimit < iHighLimit))
                    matchLength += LZ4_count(ip+matchLength, prefixPtr, iHighLimit);
                back = lookBackLength ? LZ4HC_countBack(ip, matchPtr, iLowLimit, dictStart) : 0;
                matchLength -= back;
                if (matchLength > longest) {
                    longest = matchLength;
                    offset = (int)(ipIndex - matchIndex);
                    sBack = back;
                    DEBUGLOG(7, "Found match of len=%i within dict, offset=%i, back=%i", longest, offset, -back);
        }   }   }

        if (chainSwap && matchLength==longest) {   /* better match => select a better chain */
            assert(lookBackLength==0);   /* search forward only */
            if (matchIndex + (U32)longest <= ipIndex) {
                int const kTrigger = 4;
                U32 distanceToNextMatch = 1;
                int const end = longest - MINMATCH + 1;
                int step = 1;
                int accel = 1 << kTrigger;
                int pos;
                for (pos = 0; pos < end; pos += step) {
                    U32 const candidateDist = DELTANEXTU16(chainTable, matchIndex + (U32)pos);
                    step = (accel++ >> kTrigger);
                    if (candidateDist > distanceToNextMatch) {
                        distanceToNextMatch = candidateDist;
                        matchChainPos = (U32)pos;
                        accel = 1 << kTrigger;
                }   }
                if (distanceToNextMatch > 1) {
                    if (distanceToNextMatch > matchIndex) break;   /* avoid overflow */
                    matchIndex -= distanceToNextMatch;
                    continue;
        }   }   }

        {   U32 const distNextMatch = DELTANEXTU16(chainTable, matchIndex);
            if (patternAnalysis && distNextMatch==1 && matchChainPos==0) {
                U32 const matchCandidateIdx = matchIndex-1;
                /* may be a repeated pattern */
                if (repeat == rep_untested) {
                    if ( ((pattern & 0xFFFF) == (pattern >> 16))
                      &  ((pattern & 0xFF)   == (pattern >> 24)) ) {
                        DEBUGLOG(7, "Repeat pattern detected, char %02X", pattern >> 24);
                        repeat = rep_confirmed;
                        srcPatternLength = LZ4HC_countPattern(ip+sizeof(pattern), iHighLimit, pattern) + sizeof(pattern);
                    } else {
                        repeat = rep_not;
                }   }
                if ( (repeat == rep_confirmed) && (matchCandidateIdx >= lowestMatchIndex)
                  && LZ4HC_protectDictEnd(prefixIdx, matchCandidateIdx) ) {
                    const int extDict = matchCandidateIdx < prefixIdx;
                    const BYTE* const matchPtr = extDict ? dictStart + (matchCandidateIdx - dictIdx) : prefixPtr + (matchCandidateIdx - prefixIdx);
                    if (LZ4_read32(matchPtr) == pattern) {  /* good candidate */
                        const BYTE* const iLimit = extDict ? dictEnd : iHighLimit;
                        size_t forwardPatternLength = LZ4HC_countPattern(matchPtr+sizeof(pattern), iLimit, pattern) + sizeof(pattern);
                        if (extDict && matchPtr + forwardPatternLength == iLimit) {
                            U32 const rotatedPattern = LZ4HC_rotatePattern(forwardPatternLength, pattern);
                            forwardPatternLength += LZ4HC_countPattern(prefixPtr, iHighLimit, rotatedPattern);
                        }
                        {   const BYTE* const lowestMatchPtr = extDict ? dictStart : prefixPtr;
                            size_t backLength = LZ4HC_reverseCountPattern(matchPtr, lowestMatchPtr, pattern);
                            size_t currentSegmentLength;
                            if (!extDict
                              && matchPtr - backLength == prefixPtr
                              && dictIdx < prefixIdx) {
                                U32 const rotatedPattern = LZ4HC_rotatePattern((U32)(-(int)backLength), pattern);
                                backLength += LZ4HC_reverseCountPattern(dictEnd, dictStart, rotatedPattern);
                            }
                            /* Limit backLength not go further than lowestMatchIndex */
                            backLength = matchCandidateIdx - MAX(matchCandidateIdx - (U32)backLength, lowestMatchIndex);
                            assert(matchCandidateIdx - backLength >= lowestMatchIndex);
                            currentSegmentLength = backLength + forwardPatternLength;
                            /* Adjust to end of pattern if the source pattern fits, otherwise the beginning of the pattern */
                            if ( (currentSegmentLength >= srcPatternLength)   /* current pattern segment large enough to contain full srcPatternLength */
                              && (forwardPatternLength <= srcPatternLength) ) { /* haven't reached this position yet */
                                U32 const newMatchIndex = matchCandidateIdx + (U32)forwardPatternLength - (U32)srcPatternLength;  /* best position, full pattern, might be followed by more match */
                                if (LZ4HC_protectDictEnd(prefixIdx, newMatchIndex))
                                    matchIndex = newMatchIndex;
                                else {
                                    /* Can only happen if started in the prefix */
                                    assert(newMatchIndex >= prefixIdx - 3 && newMatchIndex < prefixIdx && !extDict);
                                    matchIndex = prefixIdx;
                                }
                            } else {
                                U32 const newMatchIndex = matchCandidateIdx - (U32)backLength;   /* farthest position in current segment, will find a match of length currentSegmentLength + maybe some back */
                                if (!LZ4HC_protectDictEnd(prefixIdx, newMatchIndex)) {
                                    assert(newMatchIndex >= prefixIdx - 3 && newMatchIndex < prefixIdx && !extDict);
                                    matchIndex = prefixIdx;
                                } else {
                                    matchIndex = newMatchIndex;
                                    if (lookBackLength==0) {  /* no back possible */
                                        size_t const maxML = MIN(currentSegmentLength, srcPatternLength);
                                        if ((size_t)longest < maxML) {
                                            assert(prefixPtr - prefixIdx + matchIndex != ip);
                                            if ((size_t)(ip - prefixPtr) + prefixIdx - matchIndex > LZ4_DISTANCE_MAX) break;
                                            assert(maxML < 2 GB);
                                            longest = (int)maxML;
                                            offset = (int)(ipIndex - matchIndex);
                                            assert(sBack == 0);
                                            DEBUGLOG(7, "Found repeat pattern match of len=%i, offset=%i", longest, offset);
                                        }
                                        {   U32 const distToNextPattern = DELTANEXTU16(chainTable, matchIndex);
                                            if (distToNextPattern > matchIndex) break;  /* avoid overflow */
                                            matchIndex -= distToNextPattern;
                        }   }   }   }   }
                        continue;
                }   }
        }   }   /* PA optimization */

        /* follow current chain */
        matchIndex -= DELTANEXTU16(chainTable, matchIndex + matchChainPos);

    }  /* while ((matchIndex>=lowestMatchIndex) && (nbAttempts)) */

    if ( dict == usingDictCtxHc
      && nbAttempts > 0
      && withinStartDistance) {
        size_t const dictEndOffset = (size_t)(dictCtx->end - dictCtx->prefixStart) + dictCtx->dictLimit;
        U32 dictMatchIndex = dictCtx->hashTable[LZ4HC_hashPtr(ip)];
        assert(dictEndOffset <= 1 GB);
        matchIndex = dictMatchIndex + lowestMatchIndex - (U32)dictEndOffset;
        if (dictMatchIndex>0) DEBUGLOG(7, "dictEndOffset = %zu, dictMatchIndex = %u => relative matchIndex = %i", dictEndOffset, dictMatchIndex, (int)dictMatchIndex - (int)dictEndOffset);
        while (ipIndex - matchIndex <= LZ4_DISTANCE_MAX && nbAttempts--) {
            const BYTE* const matchPtr = dictCtx->prefixStart - dictCtx->dictLimit + dictMatchIndex;

            if (LZ4_read32(matchPtr) == pattern) {
                int mlt;
                int back = 0;
                const BYTE* vLimit = ip + (dictEndOffset - dictMatchIndex);
                if (vLimit > iHighLimit) vLimit = iHighLimit;
                mlt = (int)LZ4_count(ip+MINMATCH, matchPtr+MINMATCH, vLimit) + MINMATCH;
                back = lookBackLength ? LZ4HC_countBack(ip, matchPtr, iLowLimit, dictCtx->prefixStart) : 0;
                mlt -= back;
                if (mlt > longest) {
                    longest = mlt;
                    offset = (int)(ipIndex - matchIndex);
                    sBack = back;
                    DEBUGLOG(7, "found match of length %i within extDictCtx", longest);
            }   }

            {   U32 const nextOffset = DELTANEXTU16(dictCtx->chainTable, dictMatchIndex);
                dictMatchIndex -= nextOffset;
                matchIndex -= nextOffset;
    }   }   }

    {   LZ4HC_match_t md;
        assert(longest >= 0);
        md.len = longest;
        md.off = offset;
        md.back = sBack;
        return md;
    }
}

LZ4_FORCE_INLINE LZ4HC_match_t
LZ4HC_InsertAndFindBestMatch(LZ4HC_CCtx_internal* const hc4,   /* Index table will be updated */
                       const BYTE* const ip, const BYTE* const iLimit,
                       const int maxNbAttempts,
                       const int patternAnalysis,
                       const dictCtx_directive dict)
{
    DEBUGLOG(7, "LZ4HC_InsertAndFindBestMatch");
    /* note : LZ4HC_InsertAndGetWiderMatch() is able to modify the starting position of a match (*startpos),
     * but this won't be the case here, as we define iLowLimit==ip,
     * so LZ4HC_InsertAndGetWiderMatch() won't be allowed to search past ip */
    return LZ4HC_InsertAndGetWiderMatch(hc4, ip, ip, iLimit, MINMATCH-1, maxNbAttempts, patternAnalysis, 0 /*chainSwap*/, dict, favorCompressionRatio);
}


/* preconditions:
 * - *srcSizePtr within [1, LZ4_MAX_INPUT_SIZE]
 * - src is valid
 * - maxOutputSize >= 1
 * - dst is valid
 */
LZ4_FORCE_INLINE int LZ4HC_compress_hashChain (
    LZ4HC_CCtx_internal* const ctx,
    const char* const src,
    char* const dst,
    int* srcSizePtr,
    int const maxOutputSize,
    int maxNbAttempts,
    const limitedOutput_directive limit,
    const dictCtx_directive dict
    )
{
    const int inputSize = *srcSizePtr;
    const int patternAnalysis = (maxNbAttempts > 128);   /* levels 9+ */

    const BYTE* ip = (const BYTE*)src;
    const BYTE* anchor = ip;
    const BYTE* const iend = ip + inputSize;
    const BYTE* const mflimit = iend - MFLIMIT;
    const BYTE* const matchlimit = (iend - LASTLITERALS);

    BYTE* optr = (BYTE*) dst;
    BYTE* op = (BYTE*) dst;
    BYTE* oend = op + maxOutputSize;

    const BYTE* start0;
    const BYTE* start2 = NULL;
    const BYTE* start3 = NULL;
    LZ4HC_match_t m0, m1, m2, m3;
    const LZ4HC_match_t nomatch = {0, 0, 0};

    /* init */
    DEBUGLOG(5, "LZ4HC_compress_hashChain (dict?=>%i)", dict);

    /* preconditions verifications */
    assert(*srcSizePtr >= 1);
    assert(src != NULL);
    assert(maxOutputSize >= 1);
    assert(dst != NULL);

    *srcSizePtr = 0;
    if (limit == fillOutput) oend -= LASTLITERALS;                  /* Hack for support LZ4 format restriction */
    if (inputSize < LZ4_minLength) goto _last_literals;             /* Input too small, no compression (all literals) */

    /* Main Loop */
    while (ip <= mflimit) {
        m1 = LZ4HC_InsertAndFindBestMatch(ctx, ip, matchlimit, maxNbAttempts, patternAnalysis, dict);
        if (m1.len<MINMATCH) { ip++; continue; }

        /* saved, in case we would skip too much */
        start0 = ip; m0 = m1;

_Search2:
        DEBUGLOG(7, "_Search2 (currently found match of size %i)", m1.len);
        if (ip+m1.len <= mflimit) {
            start2 = ip + m1.len - 2;
            m2 = LZ4HC_InsertAndGetWiderMatch(ctx,
                            start2, ip + 0, matchlimit, m1.len,
                            maxNbAttempts, patternAnalysis, 0, dict, favorCompressionRatio);
            start2 += m2.back;
        } else {
            m2 = nomatch;  /* do not search further */
        }

        if (m2.len <= m1.len) { /* No better match => encode ML1 immediately */
            optr = op;
            if (LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                    m1.len, m1.off,
                    limit, oend) )
                goto _dest_overflow;
            continue;
        }

        if (start0 < ip) {   /* first match was skipped at least once */
            if (start2 < ip + m0.len) {  /* squeezing ML1 between ML0(original ML1) and ML2 */
                ip = start0; m1 = m0;  /* restore initial Match1 */
        }   }

        /* Here, start0==ip */
        if ((start2 - ip) < 3) {  /* First Match too small : removed */
            ip = start2;
            m1 = m2;
            goto _Search2;
        }

_Search3:
        if ((start2 - ip) < OPTIMAL_ML) {
            int correction;
            int new_ml = m1.len;
            if (new_ml > OPTIMAL_ML) new_ml = OPTIMAL_ML;
            if (ip+new_ml > start2 + m2.len - MINMATCH)
                new_ml = (int)(start2 - ip) + m2.len - MINMATCH;
            correction = new_ml - (int)(start2 - ip);
            if (correction > 0) {
                start2 += correction;
                m2.len -= correction;
            }
        }

        if (start2 + m2.len <= mflimit) {
            start3 = start2 + m2.len - 3;
            m3 = LZ4HC_InsertAndGetWiderMatch(ctx,
                            start3, start2, matchlimit, m2.len,
                            maxNbAttempts, patternAnalysis, 0, dict, favorCompressionRatio);
            start3 += m3.back;
        } else {
            m3 = nomatch;  /* do not search further */
        }

        if (m3.len <= m2.len) {  /* No better match => encode ML1 and ML2 */
            /* ip & ref are known; Now for ml */
            if (start2 < ip+m1.len) m1.len = (int)(start2 - ip);
            /* Now, encode 2 sequences */
            optr = op;
            if (LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                    m1.len, m1.off,
                    limit, oend) )
                goto _dest_overflow;
            ip = start2;
            optr = op;
            if (LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                    m2.len, m2.off,
                    limit, oend) ) {
                m1 = m2;
                goto _dest_overflow;
            }
            continue;
        }

        if (start3 < ip+m1.len+3) {  /* Not enough space for match 2 : remove it */
            if (start3 >= (ip+m1.len)) {  /* can write Seq1 immediately ==> Seq2 is removed, so Seq3 becomes Seq1 */
                if (start2 < ip+m1.len) {
                    int correction = (int)(ip+m1.len - start2);
                    start2 += correction;
                    m2.len -= correction;
                    if (m2.len < MINMATCH) {
                        start2 = start3;
                        m2 = m3;
                    }
                }

                optr = op;
                if (LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                        m1.len, m1.off,
                        limit, oend) )
                    goto _dest_overflow;
                ip  = start3;
                m1 = m3;

                start0 = start2;
                m0 = m2;
                goto _Search2;
            }

            start2 = start3;
            m2 = m3;
            goto _Search3;
        }

        /*
        * OK, now we have 3 ascending matches;
        * let's write the first one ML1.
        * ip & ref are known; Now decide ml.
        */
        if (start2 < ip+m1.len) {
            if ((start2 - ip) < OPTIMAL_ML) {
                int correction;
                if (m1.len > OPTIMAL_ML) m1.len = OPTIMAL_ML;
                if (ip + m1.len > start2 + m2.len - MINMATCH)
                    m1.len = (int)(start2 - ip) + m2.len - MINMATCH;
                correction = m1.len - (int)(start2 - ip);
                if (correction > 0) {
                    start2 += correction;
                    m2.len -= correction;
                }
            } else {
                m1.len = (int)(start2 - ip);
            }
        }
        optr = op;
        if ( LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor),
                m1.len, m1.off,
                limit, oend) )
            goto _dest_overflow;

        /* ML2 becomes ML1 */
        ip = start2; m1 = m2;

        /* ML3 becomes ML2 */
        start2 = start3; m2 = m3;

        /* let's find a new ML3 */
        goto _Search3;
    }

_last_literals:
    /* Encode Last Literals */
    {   size_t lastRunSize = (size_t)(iend - anchor);  /* literals */
        size_t llAdd = (lastRunSize + 255 - RUN_MASK) / 255;
        size_t const totalSize = 1 + llAdd + lastRunSize;
        if (limit == fillOutput) oend += LASTLITERALS;  /* restore correct value */
        if (limit && (op + totalSize > oend)) {
            if (limit == limitedOutput) return 0;
            /* adapt lastRunSize to fill 'dest' */
            lastRunSize  = (size_t)(oend - op) - 1 /*token*/;
            llAdd = (lastRunSize + 256 - RUN_MASK) / 256;
            lastRunSize -= llAdd;
        }
        DEBUGLOG(6, "Final literal run : %i literals", (int)lastRunSize);
        ip = anchor + lastRunSize;  /* can be != iend if limit==fillOutput */

        if (lastRunSize >= RUN_MASK) {
            size_t accumulator = lastRunSize - RUN_MASK;
            *op++ = (RUN_MASK << ML_BITS);
            for(; accumulator >= 255 ; accumulator -= 255) *op++ = 255;
            *op++ = (BYTE) accumulator;
        } else {
            *op++ = (BYTE)(lastRunSize << ML_BITS);
        }
        LZ4_memcpy(op, anchor, lastRunSize);
        op += lastRunSize;
    }

    /* End */
    *srcSizePtr = (int) (((const char*)ip) - src);
    return (int) (((char*)op)-dst);

_dest_overflow:
    if (limit == fillOutput) {
        /* Assumption : @ip, @anchor, @optr and @m1 must be set correctly */
        size_t const ll = (size_t)(ip - anchor);
        size_t const ll_addbytes = (ll + 240) / 255;
        size_t const ll_totalCost = 1 + ll_addbytes + ll;
        BYTE* const maxLitPos = oend - 3; /* 2 for offset, 1 for token */
        DEBUGLOG(6, "Last sequence overflowing");
        op = optr;  /* restore correct out pointer */
        if (op + ll_totalCost <= maxLitPos) {
            /* ll validated; now adjust match length */
            size_t const bytesLeftForMl = (size_t)(maxLitPos - (op+ll_totalCost));
            size_t const maxMlSize = MINMATCH + (ML_MASK-1) + (bytesLeftForMl * 255);
            assert(maxMlSize < INT_MAX); assert(m1.len >= 0);
            if ((size_t)m1.len > maxMlSize) m1.len = (int)maxMlSize;
            if ((oend + LASTLITERALS) - (op + ll_totalCost + 2) - 1 + m1.len >= MFLIMIT) {
                LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor), m1.len, m1.off, notLimited, oend);
        }   }
        goto _last_literals;
    }
    /* compression failed */
    return 0;
}


static int LZ4HC_compress_optimal( LZ4HC_CCtx_internal* ctx,
    const char* const source, char* dst,
    int* srcSizePtr, int dstCapacity,
    int const nbSearches, size_t sufficient_len,
    const limitedOutput_directive limit, int const fullUpdate,
    const dictCtx_directive dict,
    const HCfavor_e favorDecSpeed);

static int
LZ4HC_compress_generic_internal (
            LZ4HC_CCtx_internal* const ctx,
            const char* const src,
            char* const dst,
            int* const srcSizePtr,
            int const dstCapacity,
            int cLevel,
            const limitedOutput_directive limit,
            const dictCtx_directive dict
            )
{
    DEBUGLOG(5, "LZ4HC_compress_generic_internal(src=%p, srcSize=%d)",
                src, *srcSizePtr);

    /* input sanitization */
    if ((U32)*srcSizePtr > (U32)LZ4_MAX_INPUT_SIZE) return 0;  /* Unsupported input size (too large or negative) */
    if (dstCapacity < 1) return 0;   /* Invalid: impossible to store anything */
    assert(dst); /* since dstCapacity >= 1, dst must be valid */
    if (*srcSizePtr == 0) { *dst = 0; return 1; }
    assert(src != NULL); /* since *srcSizePtr >= 1, src must be valid */

    ctx->end += *srcSizePtr;
    {   cParams_t const cParam = LZ4HC_getCLevelParams(cLevel);
        HCfavor_e const favor = ctx->favorDecSpeed ? favorDecompressionSpeed : favorCompressionRatio;
        int result;

        if (cParam.strat == lz4mid) {
            result = LZ4MID_compress(ctx,
                                src, dst, srcSizePtr, dstCapacity,
                                limit, dict);
        } else if (cParam.strat == lz4hc) {
            result = LZ4HC_compress_hashChain(ctx,
                                src, dst, srcSizePtr, dstCapacity,
                                cParam.nbSearches, limit, dict);
        } else {
            assert(cParam.strat == lz4opt);
            result = LZ4HC_compress_optimal(ctx,
                                src, dst, srcSizePtr, dstCapacity,
                                cParam.nbSearches, cParam.targetLength, limit,
                                cLevel >= LZ4HC_CLEVEL_MAX,   /* ultra mode */
                                dict, favor);
        }
        if (result <= 0) ctx->dirty = 1;
        return result;
    }
}

static void LZ4HC_setExternalDict(LZ4HC_CCtx_internal* ctxPtr, const BYTE* newBlock);

static int
LZ4HC_compress_generic_noDictCtx (
        LZ4HC_CCtx_internal* const ctx,
        const char* const src,
        char* const dst,
        int* const srcSizePtr,
        int const dstCapacity,
        int cLevel,
        limitedOutput_directive limit
        )
{
    assert(ctx->dictCtx == NULL);
    return LZ4HC_compress_generic_internal(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit, noDictCtx);
}

static int isStateCompatible(const LZ4HC_CCtx_internal* ctx1, const LZ4HC_CCtx_internal* ctx2)
{
    int const isMid1 = LZ4HC_getCLevelParams(ctx1->compressionLevel).strat == lz4mid;
    int const isMid2 = LZ4HC_getCLevelParams(ctx2->compressionLevel).strat == lz4mid;
    return !(isMid1 ^ isMid2);
}

static int
LZ4HC_compress_generic_dictCtx (
        LZ4HC_CCtx_internal* const ctx,
        const char* const src,
        char* const dst,
        int* const srcSizePtr,
        int const dstCapacity,
        int cLevel,
        limitedOutput_directive limit
        )
{
    const size_t position = (size_t)(ctx->end - ctx->prefixStart) + (ctx->dictLimit - ctx->lowLimit);
    assert(ctx->dictCtx != NULL);
    if (position >= 64 KB) {
        ctx->dictCtx = NULL;
        return LZ4HC_compress_generic_noDictCtx(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit);
    } else if (position == 0 && *srcSizePtr > 4 KB && isStateCompatible(ctx, ctx->dictCtx)) {
        LZ4_memcpy(ctx, ctx->dictCtx, sizeof(LZ4HC_CCtx_internal));
        LZ4HC_setExternalDict(ctx, (const BYTE *)src);
        ctx->compressionLevel = (short)cLevel;
        return LZ4HC_compress_generic_noDictCtx(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit);
    } else {
        return LZ4HC_compress_generic_internal(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit, usingDictCtxHc);
    }
}

static int
LZ4HC_compress_generic (
        LZ4HC_CCtx_internal* const ctx,
        const char* const src,
        char* const dst,
        int* const srcSizePtr,
        int const dstCapacity,
        int cLevel,
        limitedOutput_directive limit
        )
{
    if (ctx->dictCtx == NULL) {
        return LZ4HC_compress_generic_noDictCtx(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit);
    } else {
        return LZ4HC_compress_generic_dictCtx(ctx, src, dst, srcSizePtr, dstCapacity, cLevel, limit);
    }
}


int LZ4_sizeofStateHC(void) { return (int)sizeof(LZ4_streamHC_t); }

static size_t LZ4_streamHC_t_alignment(void)
{
#if LZ4_ALIGN_TEST
    typedef struct { char c; LZ4_streamHC_t t; } t_a;
    return sizeof(t_a) - sizeof(LZ4_streamHC_t);
#else
    return 1;  /* effectively disabled */
#endif
}

/* state is presumed correctly initialized,
 * in which case its size and alignment have already been validate */
int LZ4_compress_HC_extStateHC_fastReset (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel)
{
    LZ4HC_CCtx_internal* const ctx = &((LZ4_streamHC_t*)state)->internal_donotuse;
    if (!LZ4_isAligned(state, LZ4_streamHC_t_alignment())) return 0;
    LZ4_resetStreamHC_fast((LZ4_streamHC_t*)state, compressionLevel);
    LZ4HC_init_internal (ctx, (const BYTE*)src);
    if (dstCapacity < LZ4_compressBound(srcSize))
        return LZ4HC_compress_generic (ctx, src, dst, &srcSize, dstCapacity, compressionLevel, limitedOutput);
    else
        return LZ4HC_compress_generic (ctx, src, dst, &srcSize, dstCapacity, compressionLevel, notLimited);
}

int LZ4_compress_HC_extStateHC (void* state, const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel)
{
    LZ4_streamHC_t* const ctx = LZ4_initStreamHC(state, sizeof(*ctx));
    if (ctx==NULL) return 0;   /* init failure */
    return LZ4_compress_HC_extStateHC_fastReset(state, src, dst, srcSize, dstCapacity, compressionLevel);
}

int LZ4_compress_HC(const char* src, char* dst, int srcSize, int dstCapacity, int compressionLevel)
{
    int cSize;
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    LZ4_streamHC_t* const statePtr = (LZ4_streamHC_t*)ALLOC(sizeof(LZ4_streamHC_t));
    if (statePtr==NULL) return 0;
#else
    LZ4_streamHC_t state;
    LZ4_streamHC_t* const statePtr = &state;
#endif
    DEBUGLOG(5, "LZ4_compress_HC")
    cSize = LZ4_compress_HC_extStateHC(statePtr, src, dst, srcSize, dstCapacity, compressionLevel);
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    FREEMEM(statePtr);
#endif
    return cSize;
}

/* state is presumed sized correctly (>= sizeof(LZ4_streamHC_t)) */
int LZ4_compress_HC_destSize(void* state, const char* source, char* dest, int* sourceSizePtr, int targetDestSize, int cLevel)
{
    LZ4_streamHC_t* const ctx = LZ4_initStreamHC(state, sizeof(*ctx));
    if (ctx==NULL) return 0;   /* init failure */
    LZ4HC_init_internal(&ctx->internal_donotuse, (const BYTE*) source);
    LZ4_setCompressionLevel(ctx, cLevel);
    return LZ4HC_compress_generic(&ctx->internal_donotuse, source, dest, sourceSizePtr, targetDestSize, cLevel, fillOutput);
}



/**************************************
*  Streaming Functions
**************************************/
/* allocation */
#if !defined(LZ4_STATIC_LINKING_ONLY_DISABLE_MEMORY_ALLOCATION)
LZ4_streamHC_t* LZ4_createStreamHC(void)
{
    LZ4_streamHC_t* const state =
        (LZ4_streamHC_t*)ALLOC_AND_ZERO(sizeof(LZ4_streamHC_t));
    if (state == NULL) return NULL;
    LZ4_setCompressionLevel(state, LZ4HC_CLEVEL_DEFAULT);
    return state;
}

int LZ4_freeStreamHC (LZ4_streamHC_t* LZ4_streamHCPtr)
{
    DEBUGLOG(4, "LZ4_freeStreamHC(%p)", LZ4_streamHCPtr);
    if (!LZ4_streamHCPtr) return 0;  /* support free on NULL */
    FREEMEM(LZ4_streamHCPtr);
    return 0;
}
#endif


LZ4_streamHC_t* LZ4_initStreamHC (void* buffer, size_t size)
{
    LZ4_streamHC_t* const LZ4_streamHCPtr = (LZ4_streamHC_t*)buffer;
    DEBUGLOG(4, "LZ4_initStreamHC(%p, %u)", buffer, (unsigned)size);
    /* check conditions */
    if (buffer == NULL) return NULL;
    if (size < sizeof(LZ4_streamHC_t)) return NULL;
    if (!LZ4_isAligned(buffer, LZ4_streamHC_t_alignment())) return NULL;
    /* init */
    { LZ4HC_CCtx_internal* const hcstate = &(LZ4_streamHCPtr->internal_donotuse);
      MEM_INIT(hcstate, 0, sizeof(*hcstate)); }
    LZ4_setCompressionLevel(LZ4_streamHCPtr, LZ4HC_CLEVEL_DEFAULT);
    return LZ4_streamHCPtr;
}

/* just a stub */
void LZ4_resetStreamHC (LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel)
{
    LZ4_initStreamHC(LZ4_streamHCPtr, sizeof(*LZ4_streamHCPtr));
    LZ4_setCompressionLevel(LZ4_streamHCPtr, compressionLevel);
}

void LZ4_resetStreamHC_fast (LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel)
{
    LZ4HC_CCtx_internal* const s = &LZ4_streamHCPtr->internal_donotuse;
    DEBUGLOG(5, "LZ4_resetStreamHC_fast(%p, %d)", LZ4_streamHCPtr, compressionLevel);
    if (s->dirty) {
        LZ4_initStreamHC(LZ4_streamHCPtr, sizeof(*LZ4_streamHCPtr));
    } else {
        assert(s->end >= s->prefixStart);
        s->dictLimit += (U32)(s->end - s->prefixStart);
        s->prefixStart = NULL;
        s->end = NULL;
        s->dictCtx = NULL;
    }
    LZ4_setCompressionLevel(LZ4_streamHCPtr, compressionLevel);
}

void LZ4_setCompressionLevel(LZ4_streamHC_t* LZ4_streamHCPtr, int compressionLevel)
{
    DEBUGLOG(5, "LZ4_setCompressionLevel(%p, %d)", LZ4_streamHCPtr, compressionLevel);
    if (compressionLevel < 1) compressionLevel = LZ4HC_CLEVEL_DEFAULT;
    if (compressionLevel > LZ4HC_CLEVEL_MAX) compressionLevel = LZ4HC_CLEVEL_MAX;
    LZ4_streamHCPtr->internal_donotuse.compressionLevel = (short)compressionLevel;
}

void LZ4_favorDecompressionSpeed(LZ4_streamHC_t* LZ4_streamHCPtr, int favor)
{
    LZ4_streamHCPtr->internal_donotuse.favorDecSpeed = (favor!=0);
}

/* LZ4_loadDictHC() :
 * LZ4_streamHCPtr is presumed properly initialized */
int LZ4_loadDictHC (LZ4_streamHC_t* LZ4_streamHCPtr,
              const char* dictionary, int dictSize)
{
    LZ4HC_CCtx_internal* const ctxPtr = &LZ4_streamHCPtr->internal_donotuse;
    cParams_t cp;
    DEBUGLOG(4, "LZ4_loadDictHC(ctx:%p, dict:%p, dictSize:%d, clevel=%d)", LZ4_streamHCPtr, dictionary, dictSize, ctxPtr->compressionLevel);
    assert(dictSize >= 0);
    assert(LZ4_streamHCPtr != NULL);
    if (dictSize > 64 KB) {
        dictionary += (size_t)dictSize - 64 KB;
        dictSize = 64 KB;
    }
    /* need a full initialization, there are bad side-effects when using resetFast() */
    {   int const cLevel = ctxPtr->compressionLevel;
        LZ4_initStreamHC(LZ4_streamHCPtr, sizeof(*LZ4_streamHCPtr));
        LZ4_setCompressionLevel(LZ4_streamHCPtr, cLevel);
        cp = LZ4HC_getCLevelParams(cLevel);
    }
    LZ4HC_init_internal (ctxPtr, (const BYTE*)dictionary);
    ctxPtr->end = (const BYTE*)dictionary + dictSize;
    if (cp.strat == lz4mid) {
        LZ4MID_fillHTable (ctxPtr, dictionary, (size_t)dictSize);
    } else {
        if (dictSize >= LZ4HC_HASHSIZE) LZ4HC_Insert (ctxPtr, ctxPtr->end-3);
    }
    return dictSize;
}

void LZ4_attach_HC_dictionary(LZ4_streamHC_t *working_stream, const LZ4_streamHC_t *dictionary_stream) {
    working_stream->internal_donotuse.dictCtx = dictionary_stream != NULL ? &(dictionary_stream->internal_donotuse) : NULL;
}

/* compression */

static void LZ4HC_setExternalDict(LZ4HC_CCtx_internal* ctxPtr, const BYTE* newBlock)
{
    DEBUGLOG(4, "LZ4HC_setExternalDict(%p, %p)", ctxPtr, newBlock);
    if ( (ctxPtr->end >= ctxPtr->prefixStart + 4)
      && (LZ4HC_getCLevelParams(ctxPtr->compressionLevel).strat != lz4mid) ) {
        LZ4HC_Insert (ctxPtr, ctxPtr->end-3);  /* Referencing remaining dictionary content */
    }

    /* Only one memory segment for extDict, so any previous extDict is lost at this stage */
    ctxPtr->lowLimit  = ctxPtr->dictLimit;
    ctxPtr->dictStart  = ctxPtr->prefixStart;
    ctxPtr->dictLimit += (U32)(ctxPtr->end - ctxPtr->prefixStart);
    ctxPtr->prefixStart = newBlock;
    ctxPtr->end  = newBlock;
    ctxPtr->nextToUpdate = ctxPtr->dictLimit;   /* match referencing will resume from there */

    /* cannot reference an extDict and a dictCtx at the same time */
    ctxPtr->dictCtx = NULL;
}

static int
LZ4_compressHC_continue_generic (LZ4_streamHC_t* LZ4_streamHCPtr,
                                 const char* src, char* dst,
                                 int* srcSizePtr, int dstCapacity,
                                 limitedOutput_directive limit)
{
    LZ4HC_CCtx_internal* const ctxPtr = &LZ4_streamHCPtr->internal_donotuse;
    DEBUGLOG(5, "LZ4_compressHC_continue_generic(ctx=%p, src=%p, srcSize=%d, limit=%d)",
                LZ4_streamHCPtr, src, *srcSizePtr, limit);
    assert(ctxPtr != NULL);
    /* auto-init if forgotten */
    if (ctxPtr->prefixStart == NULL)
        LZ4HC_init_internal (ctxPtr, (const BYTE*) src);

    /* Check overflow */
    if ((size_t)(ctxPtr->end - ctxPtr->prefixStart) + ctxPtr->dictLimit > 2 GB) {
        size_t dictSize = (size_t)(ctxPtr->end - ctxPtr->prefixStart);
        if (dictSize > 64 KB) dictSize = 64 KB;
        LZ4_loadDictHC(LZ4_streamHCPtr, (const char*)(ctxPtr->end) - dictSize, (int)dictSize);
    }

    /* Check if blocks follow each other */
    if ((const BYTE*)src != ctxPtr->end)
        LZ4HC_setExternalDict(ctxPtr, (const BYTE*)src);

    /* Check overlapping input/dictionary space */
    {   const BYTE* sourceEnd = (const BYTE*) src + *srcSizePtr;
        const BYTE* const dictBegin = ctxPtr->dictStart;
        const BYTE* const dictEnd   = ctxPtr->dictStart + (ctxPtr->dictLimit - ctxPtr->lowLimit);
        if ((sourceEnd > dictBegin) && ((const BYTE*)src < dictEnd)) {
            if (sourceEnd > dictEnd) sourceEnd = dictEnd;
            ctxPtr->lowLimit += (U32)(sourceEnd - ctxPtr->dictStart);
            ctxPtr->dictStart += (U32)(sourceEnd - ctxPtr->dictStart);
            /* invalidate dictionary is it's too small */
            if (ctxPtr->dictLimit - ctxPtr->lowLimit < LZ4HC_HASHSIZE) {
                ctxPtr->lowLimit = ctxPtr->dictLimit;
                ctxPtr->dictStart = ctxPtr->prefixStart;
    }   }   }

    return LZ4HC_compress_generic (ctxPtr, src, dst, srcSizePtr, dstCapacity, ctxPtr->compressionLevel, limit);
}

int LZ4_compress_HC_continue (LZ4_streamHC_t* LZ4_streamHCPtr, const char* src, char* dst, int srcSize, int dstCapacity)
{
    DEBUGLOG(5, "LZ4_compress_HC_continue");
    if (dstCapacity < LZ4_compressBound(srcSize))
        return LZ4_compressHC_continue_generic (LZ4_streamHCPtr, src, dst, &srcSize, dstCapacity, limitedOutput);
    else
        return LZ4_compressHC_continue_generic (LZ4_streamHCPtr, src, dst, &srcSize, dstCapacity, notLimited);
}

int LZ4_compress_HC_continue_destSize (LZ4_streamHC_t* LZ4_streamHCPtr, const char* src, char* dst, int* srcSizePtr, int targetDestSize)
{
    return LZ4_compressHC_continue_generic(LZ4_streamHCPtr, src, dst, srcSizePtr, targetDestSize, fillOutput);
}


/* LZ4_saveDictHC :
 * save history content
 * into a user-provided buffer
 * which is then used to continue compression
 */
int LZ4_saveDictHC (LZ4_streamHC_t* LZ4_streamHCPtr, char* safeBuffer, int dictSize)
{
    LZ4HC_CCtx_internal* const streamPtr = &LZ4_streamHCPtr->internal_donotuse;
    int const prefixSize = (int)(streamPtr->end - streamPtr->prefixStart);
    DEBUGLOG(5, "LZ4_saveDictHC(%p, %p, %d)", LZ4_streamHCPtr, safeBuffer, dictSize);
    assert(prefixSize >= 0);
    if (dictSize > 64 KB) dictSize = 64 KB;
    if (dictSize < 4) dictSize = 0;
    if (dictSize > prefixSize) dictSize = prefixSize;
    if (safeBuffer == NULL) assert(dictSize == 0);
    if (dictSize > 0)
        LZ4_memmove(safeBuffer, streamPtr->end - dictSize, (size_t)dictSize);
    {   U32 const endIndex = (U32)(streamPtr->end - streamPtr->prefixStart) + streamPtr->dictLimit;
        streamPtr->end = (safeBuffer == NULL) ? NULL : (const BYTE*)safeBuffer + dictSize;
        streamPtr->prefixStart = (const BYTE*)safeBuffer;
        streamPtr->dictLimit = endIndex - (U32)dictSize;
        streamPtr->lowLimit = endIndex - (U32)dictSize;
        streamPtr->dictStart = streamPtr->prefixStart;
        if (streamPtr->nextToUpdate < streamPtr->dictLimit)
            streamPtr->nextToUpdate = streamPtr->dictLimit;
    }
    return dictSize;
}


/* ================================================
 *  LZ4 Optimal parser (levels [LZ4HC_CLEVEL_OPT_MIN - LZ4HC_CLEVEL_MAX])
 * ===============================================*/
typedef struct {
    int price;
    int off;
    int mlen;
    int litlen;
} LZ4HC_optimal_t;

/* price in bytes */
LZ4_FORCE_INLINE int LZ4HC_literalsPrice(int const litlen)
{
    int price = litlen;
    assert(litlen >= 0);
    if (litlen >= (int)RUN_MASK)
        price += 1 + ((litlen-(int)RUN_MASK) / 255);
    return price;
}

/* requires mlen >= MINMATCH */
LZ4_FORCE_INLINE int LZ4HC_sequencePrice(int litlen, int mlen)
{
    int price = 1 + 2 ; /* token + 16-bit offset */
    assert(litlen >= 0);
    assert(mlen >= MINMATCH);

    price += LZ4HC_literalsPrice(litlen);

    if (mlen >= (int)(ML_MASK+MINMATCH))
        price += 1 + ((mlen-(int)(ML_MASK+MINMATCH)) / 255);

    return price;
}

LZ4_FORCE_INLINE LZ4HC_match_t
LZ4HC_FindLongerMatch(LZ4HC_CCtx_internal* const ctx,
                      const BYTE* ip, const BYTE* const iHighLimit,
                      int minLen, int nbSearches,
                      const dictCtx_directive dict,
                      const HCfavor_e favorDecSpeed)
{
    LZ4HC_match_t const match0 = { 0 , 0, 0 };
    /* note : LZ4HC_InsertAndGetWiderMatch() is able to modify the starting position of a match (*startpos),
     * but this won't be the case here, as we define iLowLimit==ip,
    ** so LZ4HC_InsertAndGetWiderMatch() won't be allowed to search past ip */
    LZ4HC_match_t md = LZ4HC_InsertAndGetWiderMatch(ctx, ip, ip, iHighLimit, minLen, nbSearches, 1 /*patternAnalysis*/, 1 /*chainSwap*/, dict, favorDecSpeed);
    assert(md.back == 0);
    if (md.len <= minLen) return match0;
    if (favorDecSpeed) {
        if ((md.len>18) & (md.len<=36)) md.len=18;   /* favor dec.speed (shortcut) */
    }
    return md;
}



/* preconditions:
 * - *srcSizePtr within [1, LZ4_MAX_INPUT_SIZE]
 * - src is valid
 * - maxOutputSize >= 1
 * - dst is valid
 */
static int LZ4HC_compress_optimal ( LZ4HC_CCtx_internal* ctx,
                                    const char* const source,
                                    char* dst,
                                    int* srcSizePtr,
                                    int dstCapacity,
                                    int const nbSearches,
                                    size_t sufficient_len,
                                    const limitedOutput_directive limit,
                                    int const fullUpdate,
                                    const dictCtx_directive dict,
                                    const HCfavor_e favorDecSpeed)
{
    int retval = 0;
#define TRAILING_LITERALS 3
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    LZ4HC_optimal_t* const opt = (LZ4HC_optimal_t*)ALLOC(sizeof(LZ4HC_optimal_t) * (LZ4_OPT_NUM + TRAILING_LITERALS));
#else
    LZ4HC_optimal_t opt[LZ4_OPT_NUM + TRAILING_LITERALS];   /* ~64 KB, which can be a bit large for some stacks... */
#endif

    const BYTE* ip = (const BYTE*) source;
    const BYTE* anchor = ip;
    const BYTE* const iend = ip + *srcSizePtr;
    const BYTE* const mflimit = iend - MFLIMIT;
    const BYTE* const matchlimit = iend - LASTLITERALS;
    BYTE* op = (BYTE*) dst;
    BYTE* opSaved = (BYTE*) dst;
    BYTE* oend = op + dstCapacity;
    int ovml = MINMATCH;  /* overflow - last sequence */
    int ovoff = 0;

    /* init */
    DEBUGLOG(5, "LZ4HC_compress_optimal(dst=%p, dstCapa=%u)", dst, (unsigned)dstCapacity);
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
    if (opt == NULL) goto _return_label;
#endif

    /* preconditions verifications */
    assert(dstCapacity > 0);
    assert(dst != NULL);
    assert(*srcSizePtr > 0);
    assert(source != NULL);

    *srcSizePtr = 0;
    if (limit == fillOutput) oend -= LASTLITERALS;   /* Hack for support LZ4 format restriction */
    if (sufficient_len >= LZ4_OPT_NUM) sufficient_len = LZ4_OPT_NUM-1;

    /* Main Loop */
    while (ip <= mflimit) {
         int const llen = (int)(ip - anchor);
         int best_mlen, best_off;
         int cur, last_match_pos = 0;

         LZ4HC_match_t const firstMatch = LZ4HC_FindLongerMatch(ctx, ip, matchlimit, MINMATCH-1, nbSearches, dict, favorDecSpeed);
         if (firstMatch.len==0) { ip++; continue; }

         if ((size_t)firstMatch.len > sufficient_len) {
             /* good enough solution : immediate encoding */
             int const firstML = firstMatch.len;
             opSaved = op;
             if ( LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor), firstML, firstMatch.off, limit, oend) ) {  /* updates ip, op and anchor */
                 ovml = firstML;
                 ovoff = firstMatch.off;
                 goto _dest_overflow;
             }
             continue;
         }

         /* set prices for first positions (literals) */
         {   int rPos;
             for (rPos = 0 ; rPos < MINMATCH ; rPos++) {
                 int const cost = LZ4HC_literalsPrice(llen + rPos);
                 opt[rPos].mlen = 1;
                 opt[rPos].off = 0;
                 opt[rPos].litlen = llen + rPos;
                 opt[rPos].price = cost;
                 DEBUGLOG(7, "rPos:%3i => price:%3i (litlen=%i) -- initial setup",
                             rPos, cost, opt[rPos].litlen);
         }   }
         /* set prices using initial match */
         {   int const matchML = firstMatch.len;   /* necessarily < sufficient_len < LZ4_OPT_NUM */
             int const offset = firstMatch.off;
             int mlen;
             assert(matchML < LZ4_OPT_NUM);
             for (mlen = MINMATCH ; mlen <= matchML ; mlen++) {
                 int const cost = LZ4HC_sequencePrice(llen, mlen);
                 opt[mlen].mlen = mlen;
                 opt[mlen].off = offset;
                 opt[mlen].litlen = llen;
                 opt[mlen].price = cost;
                 DEBUGLOG(7, "rPos:%3i => price:%3i (matchlen=%i) -- initial setup",
                             mlen, cost, mlen);
         }   }
         last_match_pos = firstMatch.len;
         {   int addLit;
             for (addLit = 1; addLit <= TRAILING_LITERALS; addLit ++) {
                 opt[last_match_pos+addLit].mlen = 1; /* literal */
                 opt[last_match_pos+addLit].off = 0;
                 opt[last_match_pos+addLit].litlen = addLit;
                 opt[last_match_pos+addLit].price = opt[last_match_pos].price + LZ4HC_literalsPrice(addLit);
                 DEBUGLOG(7, "rPos:%3i => price:%3i (litlen=%i) -- initial setup",
                             last_match_pos+addLit, opt[last_match_pos+addLit].price, addLit);
         }   }

         /* check further positions */
         for (cur = 1; cur < last_match_pos; cur++) {
             const BYTE* const curPtr = ip + cur;
             LZ4HC_match_t newMatch;

             if (curPtr > mflimit) break;
             DEBUGLOG(7, "rPos:%u[%u] vs [%u]%u",
                     cur, opt[cur].price, opt[cur+1].price, cur+1);
             if (fullUpdate) {
                 /* not useful to search here if next position has same (or lower) cost */
                 if ( (opt[cur+1].price <= opt[cur].price)
                   /* in some cases, next position has same cost, but cost rises sharply after, so a small match would still be beneficial */
                   && (opt[cur+MINMATCH].price < opt[cur].price + 3/*min seq price*/) )
                     continue;
             } else {
                 /* not useful to search here if next position has same (or lower) cost */
                 if (opt[cur+1].price <= opt[cur].price) continue;
             }

             DEBUGLOG(7, "search at rPos:%u", cur);
             if (fullUpdate)
                 newMatch = LZ4HC_FindLongerMatch(ctx, curPtr, matchlimit, MINMATCH-1, nbSearches, dict, favorDecSpeed);
             else
                 /* only test matches of minimum length; slightly faster, but misses a few bytes */
                 newMatch = LZ4HC_FindLongerMatch(ctx, curPtr, matchlimit, last_match_pos - cur, nbSearches, dict, favorDecSpeed);
             if (!newMatch.len) continue;

             if ( ((size_t)newMatch.len > sufficient_len)
               || (newMatch.len + cur >= LZ4_OPT_NUM) ) {
                 /* immediate encoding */
                 best_mlen = newMatch.len;
                 best_off = newMatch.off;
                 last_match_pos = cur + 1;
                 goto encode;
             }

             /* before match : set price with literals at beginning */
             {   int const baseLitlen = opt[cur].litlen;
                 int litlen;
                 for (litlen = 1; litlen < MINMATCH; litlen++) {
                     int const price = opt[cur].price - LZ4HC_literalsPrice(baseLitlen) + LZ4HC_literalsPrice(baseLitlen+litlen);
                     int const pos = cur + litlen;
                     if (price < opt[pos].price) {
                         opt[pos].mlen = 1; /* literal */
                         opt[pos].off = 0;
                         opt[pos].litlen = baseLitlen+litlen;
                         opt[pos].price = price;
                         DEBUGLOG(7, "rPos:%3i => price:%3i (litlen=%i)",
                                     pos, price, opt[pos].litlen);
             }   }   }

             /* set prices using match at position = cur */
             {   int const matchML = newMatch.len;
                 int ml = MINMATCH;

                 assert(cur + newMatch.len < LZ4_OPT_NUM);
                 for ( ; ml <= matchML ; ml++) {
                     int const pos = cur + ml;
                     int const offset = newMatch.off;
                     int price;
                     int ll;
                     DEBUGLOG(7, "testing price rPos %i (last_match_pos=%i)",
                                 pos, last_match_pos);
                     if (opt[cur].mlen == 1) {
                         ll = opt[cur].litlen;
                         price = ((cur > ll) ? opt[cur - ll].price : 0)
                               + LZ4HC_sequencePrice(ll, ml);
                     } else {
                         ll = 0;
                         price = opt[cur].price + LZ4HC_sequencePrice(0, ml);
                     }

                    assert((U32)favorDecSpeed <= 1);
                     if (pos > last_match_pos+TRAILING_LITERALS
                      || price <= opt[pos].price - (int)favorDecSpeed) {
                         DEBUGLOG(7, "rPos:%3i => price:%3i (matchlen=%i)",
                                     pos, price, ml);
                         assert(pos < LZ4_OPT_NUM);
                         if ( (ml == matchML)  /* last pos of last match */
                           && (last_match_pos < pos) )
                             last_match_pos = pos;
                         opt[pos].mlen = ml;
                         opt[pos].off = offset;
                         opt[pos].litlen = ll;
                         opt[pos].price = price;
             }   }   }
             /* complete following positions with literals */
             {   int addLit;
                 for (addLit = 1; addLit <= TRAILING_LITERALS; addLit ++) {
                     opt[last_match_pos+addLit].mlen = 1; /* literal */
                     opt[last_match_pos+addLit].off = 0;
                     opt[last_match_pos+addLit].litlen = addLit;
                     opt[last_match_pos+addLit].price = opt[last_match_pos].price + LZ4HC_literalsPrice(addLit);
                     DEBUGLOG(7, "rPos:%3i => price:%3i (litlen=%i)", last_match_pos+addLit, opt[last_match_pos+addLit].price, addLit);
             }   }
         }  /* for (cur = 1; cur <= last_match_pos; cur++) */

         assert(last_match_pos < LZ4_OPT_NUM + TRAILING_LITERALS);
         best_mlen = opt[last_match_pos].mlen;
         best_off = opt[last_match_pos].off;
         cur = last_match_pos - best_mlen;

encode: /* cur, last_match_pos, best_mlen, best_off must be set */
         assert(cur < LZ4_OPT_NUM);
         assert(last_match_pos >= 1);  /* == 1 when only one candidate */
         DEBUGLOG(6, "reverse traversal, looking for shortest path (last_match_pos=%i)", last_match_pos);
         {   int candidate_pos = cur;
             int selected_matchLength = best_mlen;
             int selected_offset = best_off;
             while (1) {  /* from end to beginning */
                 int const next_matchLength = opt[candidate_pos].mlen;  /* can be 1, means literal */
                 int const next_offset = opt[candidate_pos].off;
                 DEBUGLOG(7, "pos %i: sequence length %i", candidate_pos, selected_matchLength);
                 opt[candidate_pos].mlen = selected_matchLength;
                 opt[candidate_pos].off = selected_offset;
                 selected_matchLength = next_matchLength;
                 selected_offset = next_offset;
                 if (next_matchLength > candidate_pos) break; /* last match elected, first match to encode */
                 assert(next_matchLength > 0);  /* can be 1, means literal */
                 candidate_pos -= next_matchLength;
         }   }

         /* encode all recorded sequences in order */
         {   int rPos = 0;  /* relative position (to ip) */
             while (rPos < last_match_pos) {
                 int const ml = opt[rPos].mlen;
                 int const offset = opt[rPos].off;
                 if (ml == 1) { ip++; rPos++; continue; }  /* literal; note: can end up with several literals, in which case, skip them */
                 rPos += ml;
                 assert(ml >= MINMATCH);
                 assert((offset >= 1) && (offset <= LZ4_DISTANCE_MAX));
                 opSaved = op;
                 if ( LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor), ml, offset, limit, oend) ) {  /* updates ip, op and anchor */
                     ovml = ml;
                     ovoff = offset;
                     goto _dest_overflow;
         }   }   }
     }  /* while (ip <= mflimit) */

_last_literals:
     /* Encode Last Literals */
     {   size_t lastRunSize = (size_t)(iend - anchor);  /* literals */
         size_t llAdd = (lastRunSize + 255 - RUN_MASK) / 255;
         size_t const totalSize = 1 + llAdd + lastRunSize;
         if (limit == fillOutput) oend += LASTLITERALS;  /* restore correct value */
         if (limit && (op + totalSize > oend)) {
             if (limit == limitedOutput) { /* Check output limit */
                retval = 0;
                goto _return_label;
             }
             /* adapt lastRunSize to fill 'dst' */
             lastRunSize  = (size_t)(oend - op) - 1 /*token*/;
             llAdd = (lastRunSize + 256 - RUN_MASK) / 256;
             lastRunSize -= llAdd;
         }
         DEBUGLOG(6, "Final literal run : %i literals", (int)lastRunSize);
         ip = anchor + lastRunSize; /* can be != iend if limit==fillOutput */

         if (lastRunSize >= RUN_MASK) {
             size_t accumulator = lastRunSize - RUN_MASK;
             *op++ = (RUN_MASK << ML_BITS);
             for(; accumulator >= 255 ; accumulator -= 255) *op++ = 255;
             *op++ = (BYTE) accumulator;
         } else {
             *op++ = (BYTE)(lastRunSize << ML_BITS);
         }
         LZ4_memcpy(op, anchor, lastRunSize);
         op += lastRunSize;
     }

     /* End */
     *srcSizePtr = (int) (((const char*)ip) - source);
     retval = (int) ((char*)op-dst);
     goto _return_label;

_dest_overflow:
if (limit == fillOutput) {
     /* Assumption : ip, anchor, ovml and ovref must be set correctly */
     size_t const ll = (size_t)(ip - anchor);
     size_t const ll_addbytes = (ll + 240) / 255;
     size_t const ll_totalCost = 1 + ll_addbytes + ll;
     BYTE* const maxLitPos = oend - 3; /* 2 for offset, 1 for token */
     DEBUGLOG(6, "Last sequence overflowing (only %i bytes remaining)", (int)(oend-1-opSaved));
     op = opSaved;  /* restore correct out pointer */
     if (op + ll_totalCost <= maxLitPos) {
         /* ll validated; now adjust match length */
         size_t const bytesLeftForMl = (size_t)(maxLitPos - (op+ll_totalCost));
         size_t const maxMlSize = MINMATCH + (ML_MASK-1) + (bytesLeftForMl * 255);
         assert(maxMlSize < INT_MAX); assert(ovml >= 0);
         if ((size_t)ovml > maxMlSize) ovml = (int)maxMlSize;
         if ((oend + LASTLITERALS) - (op + ll_totalCost + 2) - 1 + ovml >= MFLIMIT) {
             DEBUGLOG(6, "Space to end : %i + ml (%i)", (int)((oend + LASTLITERALS) - (op + ll_totalCost + 2) - 1), ovml);
             DEBUGLOG(6, "Before : ip = %p, anchor = %p", ip, anchor);
             LZ4HC_encodeSequence(UPDATABLE(ip, op, anchor), ovml, ovoff, notLimited, oend);
             DEBUGLOG(6, "After : ip = %p, anchor = %p", ip, anchor);
     }   }
     goto _last_literals;
}
_return_label:
#if defined(LZ4HC_HEAPMODE) && LZ4HC_HEAPMODE==1
     if (opt) FREEMEM(opt);
#endif
     return retval;
}


/***************************************************
*  Deprecated Functions
***************************************************/

/* -- end -- */

/*
 * solution.cpp
 *
 * Example solution.
 * This is (almost) how blocks are actually compressed in TON.
 * Normally, blocks are stored using vm::std_boc_serialize with mode=31.
 * Compression algorithm takes a block, converts it to mode=2 (which has less extra information) and compresses it using lz4.
 */


namespace td {

td::BufferSlice lz4_compress_2(td::Slice data) {
  int size = narrow_cast<int>(data.size());
  int buf_size = LZ4_compressBound(size);
  td::BufferSlice compressed(buf_size);

  int compression_level = 9;
  int compressed_size = LZ4_compress_HC(data.data(), compressed.data(), size, buf_size, compression_level);
  CHECK(compressed_size > 0);
  return td::BufferSlice{compressed.as_slice().substr(0, compressed_size)};
}

td::Result<td::BufferSlice> lz4_decompress_2(td::Slice data, int max_decompressed_size) {
  TRY_RESULT(size, narrow_cast_safe<int>(data.size()));
  if (max_decompressed_size < 0) {
    return td::Status::Error("invalid max_decompressed_size");
  }
  td::BufferSlice decompressed(max_decompressed_size);
  int result = LZ4_decompress_safe(data.data(), decompressed.data(), size, max_decompressed_size);
  if (result < 0) {
    return td::Status::Error(PSTRING() << "lz4 decompression failed, error code: " << result);
  }
  if (result == max_decompressed_size) {
    return decompressed;
  }
  return td::BufferSlice{decompressed.as_slice().substr(0, result)};
}

}  // namespace td

td::BufferSlice compress(td::Slice data) {
  td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();
  td::BufferSlice serialized = vm::std_boc_serialize(root, 2).move_as_ok();
//   return td::lz4_compress(serialized);
  return td::lz4_compress_2(serialized);
}

td::BufferSlice decompress(td::Slice data) {
//   td::BufferSlice serialized = td::lz4_decompress(data, 2 << 20).move_as_ok();
  td::BufferSlice serialized = td::lz4_decompress_2(data, 2 << 20).move_as_ok();
  auto root = vm::std_boc_deserialize(serialized).move_as_ok();
  return vm::std_boc_serialize(root, 31).move_as_ok();
}

int main() {
  std::string mode;
  std::cin >> mode;
  CHECK(mode == "compress" || mode == "decompress");

  std::string base64_data;
  std::cin >> base64_data;
  CHECK(!base64_data.empty());

  td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());

  if (mode == "compress") {
    data = compress(data);
  } else {
    data = decompress(data);
  }

  std::cout << td::base64_encode(data) << std::endl;
}
