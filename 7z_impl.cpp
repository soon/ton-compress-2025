
// minify-remove:start

#include"td/utils/buffer.h"
#include"td/utils/misc.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
// minify-remove:end

#define SZ_OK 0

#define SZ_ERROR_DATA 1
#define SZ_ERROR_MEM 2
#define SZ_ERROR_UNSUPPORTED 4
#define SZ_ERROR_PARAM 5
#define SZ_ERROR_INPUT_EOF 6
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_FAIL 11

#define RINOK(x) { const int _result_ = (x); if (_result_ != 0) return _result_; }

typedef unsigned char Byte;
typedef unsigned short UInt16;
typedef unsigned int Uint;

struct ISeqOutStream {
    size_t (*Write)(ISeqOutStream* p, const void *buf, size_t size);
};

#define MY_offsetof(type, m) offsetof(type, m)

#define Z7_container_of(ptr, type, m)   ((type *)(void *)((char *)(void *)   (1 ? (ptr) : &((type *)0)->m) - MY_offsetof(type, m)))

#define Z7_CONTAINER_FROM_VTBL(ptr, type, m) Z7_container_of(ptr, type, m)

#define Z7_CONTAINER_FROM_VTBL_TO_DECL_VAR(ptr, type, m, p) type *p = Z7_CONTAINER_FROM_VTBL(ptr, type, m); 

#define UNUSED_VAR(x) (void)x;

void *SzAlloc(size_t size) { return size == 0 ? 0 : malloc(size); }

#define GetUi16(p) (*(const UInt16 *)(const void *)(p))
#define GetUi32(p) (*(const Uint *)(const void *)(p))

#define SetUi32(p, v) { *(Uint *)(void *)(p) = (v); }

#define UNUSED_VAR(x) (void)x;

typedef Uint CLzRef;

typedef struct
{
  const Byte *buffer;
  Uint pos, posLimit, streamPos, lenLimit, cyclicBufferPos, cyclicBufferSize; 

  Byte streamEndWasReached, btMode, bigHash, directInput;

  Uint matchMaxLen;
  CLzRef *hash, *son;
  Uint hashMask, cutValue;

  Byte *bufBase;
  
  Uint blockSize, keepSizeBefore, keepSizeAfter;

  Uint numHashBytes;
  size_t directInputRem;
  Uint historySize, fixedHashSize;
  Byte numHashBytes_Min, numHashOutBits, _pad2_[2];
  int result;
  Uint crc[256];
  size_t numRefs;

  uint64_t expectedDataSize;
} CMatchFinder;


#define Inline_MatchFinder_GetNumAvailableBytes(p) ((Uint)((p)->streamPos - (p)->pos))

#define MatchFinder_SET_DIRECT_INPUT_BUF(p, a, b) { (p)->directInput = 1; (p)->buffer = (a); (p)->directInputRem = (b); }
  
#define MatchFinder_REDUCE_OFFSETS(p, a)     (p)->pos -= (a);     (p)->streamPos -= (a);

typedef void (*Mf_Init_Func)(void *object);
typedef Uint (*Mf_GetNumAvailableBytes_Func)(void *object);
typedef const Byte * (*Mf_GetPointerToCurrentPos_Func)(void *object);
typedef Uint * (*Mf_GetMatches_Func)(void *object, Uint *distances);
typedef void (*Mf_Skip_Func)(void *object, Uint);

struct IMatchFinder2
{
  Mf_Init_Func Init;
  Mf_GetNumAvailableBytes_Func GetNumAvailableBytes;
  Mf_GetPointerToCurrentPos_Func GetPointerToCurrentPos;
  Mf_GetMatches_Func GetMatches;
  Mf_Skip_Func Skip;
};

#define kHash2Size (1 << 10)
#define kHash3Size (1 << 16)

#define kFix3HashSize (kHash2Size)
#define kFix4HashSize (kHash2Size + kHash3Size)

#define kLzHash_CrcShift_1 5
#define kLzHash_CrcShift_2 10

#define kBlockMoveAlign       (1 << 7)  
#define kBlockSizeAlign       (1 << 16) 
#define kBlockSizeReserveMin  (1 << 24) 

#define kEmptyHashValue 0

#define kMaxValForNormalize ((Uint)0)

#define GET_AVAIL_BYTES(p) Inline_MatchFinder_GetNumAvailableBytes(p)

#define kFix5HashSize kFix4HashSize

#define HASH2_CALC hv = GetUi16(cur);

#define HASH3_CALC { Uint temp = p->crc[cur[0]] ^ cur[1]; h2 = temp & (kHash2Size - 1); hv = (temp ^ ((Uint)cur[2] << 8)) & p->hashMask; }

#define HASH4_CALC { Uint temp = p->crc[cur[0]] ^ cur[1]; h2 = temp & (kHash2Size - 1); temp ^= ((Uint)cur[2] << 8); h3 = temp & (kHash3Size - 1); hv = (temp ^ (p->crc[cur[3]] << kLzHash_CrcShift_1)) & p->hashMask; }

#define HASH5_CALC { Uint temp = p->crc[cur[0]] ^ cur[1]; h2 = temp & (kHash2Size - 1); temp ^= ((Uint)cur[2] << 8); h3 = temp & (kHash3Size - 1); temp ^= (p->crc[cur[3]] << kLzHash_CrcShift_1); hv = (temp ^ (p->crc[cur[4]] << kLzHash_CrcShift_2)) & p->hashMask; }


static void LzInWindow_Free(CMatchFinder *p)
{
  
  {
    free(p->bufBase);
    p->bufBase = 0;
  }
}

static int LzInWindow_Create2(CMatchFinder *p, Uint blockSize)
{
  if (blockSize == 0)
    return 0;
  if (!p->bufBase || p->blockSize != blockSize)
  {
    LzInWindow_Free(p);
    p->blockSize = blockSize;
    p->bufBase = (Byte *)SzAlloc(blockSize);
  }
  return (p->bufBase != 0);
}

static const Byte *MatchFinder_GetPointerToCurrentPos(void *p)
{
  return ((CMatchFinder *)p)->buffer;
}

static Uint MatchFinder_GetNumAvailableBytes(void *p)
{
  return GET_AVAIL_BYTES((CMatchFinder *)p);
}


static void MatchFinder_ReadBlock(CMatchFinder *p)
{
  if (p->streamEndWasReached || p->result != SZ_OK)
    return;

  Uint curSize = 0xFFFFFFFF - GET_AVAIL_BYTES(p);
  if (curSize > p->directInputRem)
    curSize = (Uint)p->directInputRem;
  p->streamPos += curSize;
  p->directInputRem -= curSize;
  if (p->directInputRem == 0)
    p->streamEndWasReached = 1;
}

void MatchFinder_MoveBlock(CMatchFinder *p)
{
  const size_t offset = (size_t)(p->buffer - p->bufBase) - p->keepSizeBefore;
  const size_t keepBefore = (offset & (kBlockMoveAlign - 1)) + p->keepSizeBefore;
  p->buffer = p->bufBase + keepBefore;
  memmove(p->bufBase,
      p->bufBase + (offset & ~((size_t)kBlockMoveAlign - 1)),
      keepBefore + (size_t)GET_AVAIL_BYTES(p));
}

int MatchFinder_NeedMove(CMatchFinder *p)
{
  if (p->directInput)
    return 0;
  if (p->streamEndWasReached || p->result != SZ_OK)
    return 0;
  return ((size_t)(p->bufBase + p->blockSize - p->buffer) <= p->keepSizeAfter);
}

static void MatchFinder_SetDefaultSettings(CMatchFinder *p)
{
  p->cutValue = 32;
  p->btMode = 1;
  p->numHashBytes = 4;
  p->numHashBytes_Min = 2;
  p->numHashOutBits = 0;
  p->bigHash = 0;
}

#define kCrcPoly 0xEDB88320

void MatchFinder_Construct(CMatchFinder *p)
{
  unsigned i;
  p->buffer = 0;
  p->bufBase = 0;
  p->directInput = 0;
  // p->stream = 0;
  p->hash = 0;
  p->expectedDataSize = (uint64_t)(int64_t)-1;
  MatchFinder_SetDefaultSettings(p);

  for (i = 0; i < 256; i++)
  {
    Uint r = (Uint)i;
    unsigned j;
    for (j = 0; j < 8; j++)
      r = (r >> 1) ^ (kCrcPoly & ((Uint)0 - (r & 1)));
    p->crc[i] = r;
  }
}

#undef kCrcPoly

static void MatchFinder_FreeThisClassMemory(CMatchFinder *p)
{
  free(p->hash);
  p->hash = 0;
}

void MatchFinder_Free(CMatchFinder *p)
{
  MatchFinder_FreeThisClassMemory(p);
  LzInWindow_Free(p);
}

static CLzRef* AllocRefs(size_t num)
{
  const size_t sizeInBytes = (size_t)num * sizeof(CLzRef);
  if (sizeInBytes / sizeof(CLzRef) != num)
    return 0;
  return (CLzRef *)SzAlloc(sizeInBytes);
}


static Uint GetBlockSize(CMatchFinder *p, Uint historySize)
{
  Uint blockSize = (p->keepSizeBefore + p->keepSizeAfter);

  if (p->keepSizeBefore < historySize || blockSize < p->keepSizeBefore)  
    return 0;
  
  {
    const Uint kBlockSizeMax = (Uint)0 - (Uint)kBlockSizeAlign;
    const Uint rem = kBlockSizeMax - blockSize;
    const Uint reserve = (blockSize >> (blockSize < ((Uint)1 << 30) ? 1 : 2))
        + (1 << 12) + kBlockMoveAlign + kBlockSizeAlign; 
    if (blockSize >= kBlockSizeMax
        || rem < kBlockSizeReserveMin) 
      return 0;
    if (reserve >= rem)
      blockSize = kBlockSizeMax;
    else
    {
      blockSize += reserve;
      blockSize &= ~(Uint)(kBlockSizeAlign - 1);
    }
  }
  
  
  return blockSize;
}



static Uint MatchFinder_GetHashMask2(CMatchFinder *p, Uint hs)
{
  if (p->numHashBytes == 2)
    return (1 << 16) - 1;
  if (hs != 0)
    hs--;
  hs |= (hs >> 1);
  hs |= (hs >> 2);
  hs |= (hs >> 4);
  hs |= (hs >> 8);
  
  if (hs >= (1 << 24))
  {
    if (p->numHashBytes == 3)
      hs = (1 << 24) - 1;
    
  }
  
  hs |= (1 << 16) - 1; 
  
  if (p->numHashBytes >= 5)
    hs |= (256 << kLzHash_CrcShift_2) - 1;
  return hs;
}


static Uint MatchFinder_GetHashMask(CMatchFinder *p, Uint hs)
{
  if (p->numHashBytes == 2)
    return (1 << 16) - 1;
  if (hs != 0)
    hs--;
  hs |= (hs >> 1);
  hs |= (hs >> 2);
  hs |= (hs >> 4);
  hs |= (hs >> 8);
  
  hs >>= 1;
  if (hs >= (1 << 24))
  {
    if (p->numHashBytes == 3)
      hs = (1 << 24) - 1;
    else
      hs >>= 1;
    
  }
  
  hs |= (1 << 16) - 1; 
  
  if (p->numHashBytes >= 5)
    hs |= (256 << kLzHash_CrcShift_2) - 1;
  return hs;
}


int MatchFinder_Create(CMatchFinder *p, Uint historySize,
    Uint keepAddBufferBefore, Uint matchMaxLen, Uint keepAddBufferAfter)
{
  
  
  p->keepSizeBefore = historySize + keepAddBufferBefore + 1;

  keepAddBufferAfter += matchMaxLen;
  
  if (keepAddBufferAfter < p->numHashBytes)
    keepAddBufferAfter = p->numHashBytes;
  
  p->keepSizeAfter = keepAddBufferAfter;

  if (p->directInput)
    p->blockSize = 0;
  if (p->directInput || LzInWindow_Create2(p, GetBlockSize(p, historySize)))
  {
    size_t hashSizeSum;
    {
      Uint hs;
      Uint hsCur;
      
      if (p->numHashOutBits != 0)
      {
        unsigned numBits = p->numHashOutBits;
        const unsigned nbMax =
            (p->numHashBytes == 2 ? 16 :
            (p->numHashBytes == 3 ? 24 : 32));
        if (numBits > nbMax)
          numBits = nbMax;
        if (numBits >= 32)
          hs = (Uint)0 - 1;
        else
          hs = ((Uint)1 << numBits) - 1;
        
        hs |= (1 << 16) - 1; 
        if (p->numHashBytes >= 5)
          hs |= (256 << kLzHash_CrcShift_2) - 1;
        {
          const Uint hs2 = MatchFinder_GetHashMask2(p, historySize);
          if (hs > hs2)
            hs = hs2;
        }
        hsCur = hs;
        if (p->expectedDataSize < historySize)
        {
          const Uint hs2 = MatchFinder_GetHashMask2(p, (Uint)p->expectedDataSize);
          if (hsCur > hs2)
            hsCur = hs2;
        }
      }
      else
      {
        hs = MatchFinder_GetHashMask(p, historySize);
        hsCur = hs;
        if (p->expectedDataSize < historySize)
        {
          hsCur = MatchFinder_GetHashMask(p, (Uint)p->expectedDataSize);
          if (hsCur > hs) 
            hsCur = hs;
        }
      }

      p->hashMask = hsCur;

      hashSizeSum = hs;
      hashSizeSum++;
      if (hashSizeSum < hs)
        return 0;
      {
        Uint fixedHashSize = 0;
        if (p->numHashBytes > 2 && p->numHashBytes_Min <= 2) fixedHashSize += kHash2Size;
        if (p->numHashBytes > 3 && p->numHashBytes_Min <= 3) fixedHashSize += kHash3Size;
        
        hashSizeSum += fixedHashSize;
        p->fixedHashSize = fixedHashSize;
      }
    }

    p->matchMaxLen = matchMaxLen;

    {
      size_t newSize;
      size_t numSons;
      const Uint newCyclicBufferSize = historySize + 1; 
      p->historySize = historySize;
      p->cyclicBufferSize = newCyclicBufferSize; 
      
      numSons = newCyclicBufferSize;
      if (p->btMode)
        numSons <<= 1;
      newSize = hashSizeSum + numSons;

      if (numSons < newCyclicBufferSize || newSize < numSons)
        return 0;

      
      #define NUM_REFS_ALIGN_MASK 0xF
      newSize = (newSize + NUM_REFS_ALIGN_MASK) & ~(size_t)NUM_REFS_ALIGN_MASK;

      
      if (p->hash && p->numRefs >= newSize)
        return 1;
      
      MatchFinder_FreeThisClassMemory(p);
      p->numRefs = newSize;
      p->hash = AllocRefs(newSize);
      
      if (p->hash)
      {
        p->son = p->hash + hashSizeSum;
        return 1;
      }
    }
  }

  MatchFinder_Free(p);
  return 0;
}


static void MatchFinder_SetLimits(CMatchFinder *p)
{
  Uint k;
  Uint n = kMaxValForNormalize - p->pos;
  if (n == 0)
    n = (Uint)(int)-1;  
  
  k = p->cyclicBufferSize - p->cyclicBufferPos;
  if (k < n)
    n = k;

  k = GET_AVAIL_BYTES(p);
  {
    const Uint ksa = p->keepSizeAfter;
    Uint mm = p->matchMaxLen;
    if (k > ksa)
      k -= ksa; 
    else if (k >= mm)
    {
      
      k -= mm;   
      k++;
      
    }
    else
    {
      mm = k;
      if (k != 0)
        k = 1;
    }
    p->lenLimit = mm;
  }
  if (k < n)
    n = k;
  
  p->posLimit = p->pos + n;
}


void MatchFinder_Init_LowHash(CMatchFinder *p)
{
  size_t i;
  CLzRef *items = p->hash;
  const size_t numItems = p->fixedHashSize;
  for (i = 0; i < numItems; i++)
    items[i] = kEmptyHashValue;
}


void MatchFinder_Init_HighHash(CMatchFinder *p)
{
  size_t i;
  CLzRef *items = p->hash + p->fixedHashSize;
  const size_t numItems = (size_t)p->hashMask + 1;
  for (i = 0; i < numItems; i++)
    items[i] = kEmptyHashValue;
}


void MatchFinder_Init_4(CMatchFinder *p)
{
  if (!p->directInput)
    p->buffer = p->bufBase;
  {
    p->pos = p->streamPos = 1; 
  }
  p->result = SZ_OK;
  p->streamEndWasReached = 0;
}



#define CYC_TO_POS_OFFSET 0


void MatchFinder_Init(void *_p)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  MatchFinder_Init_HighHash(p);
  MatchFinder_Init_LowHash(p);
  MatchFinder_Init_4(p);
  
  MatchFinder_ReadBlock(p);

  
  p->cyclicBufferPos = (p->pos - CYC_TO_POS_OFFSET); 
  
  
  MatchFinder_SetLimits(p);
}


#define SASUB_32(i)  { Uint v = items[i];  if (v < subValue) v = subValue; items[i] = v - subValue; }

#define DEFAULT_SaturSub LzFind_SaturSub_32

static
void

LzFind_SaturSub_32(Uint subValue, CLzRef *items, const CLzRef *lim)
{
  do
  {
    SASUB_32(0)  SASUB_32(1)  items += 2;
    SASUB_32(0)  SASUB_32(1)  items += 2;
    SASUB_32(0)  SASUB_32(1)  items += 2;
    SASUB_32(0)  SASUB_32(1)  items += 2;
  }
  while (items != lim);
}


void MatchFinder_Normalize3(Uint subValue, CLzRef *items, size_t numItems)
{
  #define LZFIND_NORM_ALIGN_BLOCK_SIZE (1 << 7)
  for (; numItems != 0 && ((unsigned)(ptrdiff_t)items & (LZFIND_NORM_ALIGN_BLOCK_SIZE - 1)) != 0; numItems--)
  {
    SASUB_32(0)
    items++;
  }
  {
    const size_t k_Align_Mask = (LZFIND_NORM_ALIGN_BLOCK_SIZE / 4 - 1);
    CLzRef *lim = items + (numItems & ~(size_t)k_Align_Mask);
    numItems &= k_Align_Mask;
    if (items != lim)
    {
        DEFAULT_SaturSub(subValue, items, lim);
    }
    items = lim;
  }
  for (; numItems != 0; numItems--)
  {
    SASUB_32(0)
    items++;
  }
}

static void MatchFinder_CheckLimits(CMatchFinder *p)
{
  if (
      p->keepSizeAfter == GET_AVAIL_BYTES(p))
  {
    
    if (MatchFinder_NeedMove(p))
      MatchFinder_MoveBlock(p);
    MatchFinder_ReadBlock(p);
  }

  if (p->pos == kMaxValForNormalize)
  if (GET_AVAIL_BYTES(p) >= p->numHashBytes) 
    
  {
    
    
    
    const Uint subValue = (p->pos - p->historySize - 1) ;
    
    
    MatchFinder_REDUCE_OFFSETS(p, subValue)
    MatchFinder_Normalize3(subValue, p->hash, (size_t)p->hashMask + 1 + p->fixedHashSize);
    {
      size_t numSonRefs = p->cyclicBufferSize;
      if (p->btMode)
        numSonRefs <<= 1;
      MatchFinder_Normalize3(subValue, p->son, numSonRefs);
    }
  }

  if (p->cyclicBufferPos == p->cyclicBufferSize)
    p->cyclicBufferPos = 0;
  
  MatchFinder_SetLimits(p);
}


static Uint * Hc_GetMatchesSpec(size_t lenLimit, Uint curMatch, Uint pos, const Byte *cur, CLzRef *son,
    size_t _cyclicBufferPos, Uint _cyclicBufferSize, Uint cutValue,
    Uint *d, unsigned maxLen)
{
  const Byte *lim = cur + lenLimit;
  son[_cyclicBufferPos] = curMatch;

  do
  {
    Uint delta;

    if (curMatch == 0)
      break;
    
    delta = pos - curMatch;
    if (delta >= _cyclicBufferSize)
      break;
    {
      ptrdiff_t diff;
      curMatch = son[_cyclicBufferPos - delta + ((delta > _cyclicBufferPos) ? _cyclicBufferSize : 0)];
      diff = (ptrdiff_t)0 - (ptrdiff_t)delta;
      if (cur[maxLen] == cur[(ptrdiff_t)maxLen + diff])
      {
        const Byte *c = cur;
        while (*c == c[diff])
        {
          if (++c == lim)
          {
            d[0] = (Uint)(lim - cur);
            d[1] = delta - 1;
            return d + 2;
          }
        }
        {
          const unsigned len = (unsigned)(c - cur);
          if (maxLen < len)
          {
            maxLen = len;
            d[0] = (Uint)len;
            d[1] = delta - 1;
            d += 2;
          }
        }
      }
    }
  }
  while (--cutValue);
  
  return d;
}



Uint * GetMatchesSpec1(Uint lenLimit, Uint curMatch, Uint pos, const Byte *cur, CLzRef *son,
    size_t _cyclicBufferPos, Uint _cyclicBufferSize, Uint cutValue,
    Uint *d, Uint maxLen)
{
  CLzRef *ptr0 = son + ((size_t)_cyclicBufferPos << 1) + 1;
  CLzRef *ptr1 = son + ((size_t)_cyclicBufferPos << 1);
  unsigned len0 = 0, len1 = 0;

  Uint cmCheck;

  

  cmCheck = (Uint)(pos - _cyclicBufferSize);
  if ((Uint)pos <= _cyclicBufferSize)
    cmCheck = 0;

  if (cmCheck < curMatch)
  do
  {
    const Uint delta = pos - curMatch;
    {
      CLzRef *pair = son + ((size_t)(_cyclicBufferPos - delta + ((delta > _cyclicBufferPos) ? _cyclicBufferSize : 0)) << 1);
      const Byte *pb = cur - delta;
      unsigned len = (len0 < len1 ? len0 : len1);
      const Uint pair0 = pair[0];
      if (pb[len] == cur[len])
      {
        if (++len != lenLimit && pb[len] == cur[len])
          while (++len != lenLimit)
            if (pb[len] != cur[len])
              break;
        if (maxLen < len)
        {
          maxLen = (Uint)len;
          *d++ = (Uint)len;
          *d++ = delta - 1;
          if (len == lenLimit)
          {
            *ptr1 = pair0;
            *ptr0 = pair[1];
            return d;
          }
        }
      }
      if (pb[len] < cur[len])
      {
        *ptr1 = curMatch;
        
        
        
        curMatch = pair[1];
        ptr1 = pair + 1;
        len1 = len;
      }
      else
      {
        *ptr0 = curMatch;
        curMatch = pair[0];
        ptr0 = pair;
        len0 = len;
      }
    }
  }
  while(--cutValue && cmCheck < curMatch);

  *ptr0 = *ptr1 = kEmptyHashValue;
  return d;
}


static void SkipMatchesSpec(Uint lenLimit, Uint curMatch, Uint pos, const Byte *cur, CLzRef *son,
    size_t _cyclicBufferPos, Uint _cyclicBufferSize, Uint cutValue)
{
  CLzRef *ptr0 = son + ((size_t)_cyclicBufferPos << 1) + 1;
  CLzRef *ptr1 = son + ((size_t)_cyclicBufferPos << 1);
  unsigned len0 = 0, len1 = 0;

  Uint cmCheck;

  cmCheck = (Uint)(pos - _cyclicBufferSize);
  if ((Uint)pos <= _cyclicBufferSize)
    cmCheck = 0;

  if (
      cmCheck < curMatch)
  do
  {
    const Uint delta = pos - curMatch;
    {
      CLzRef *pair = son + ((size_t)(_cyclicBufferPos - delta + ((delta > _cyclicBufferPos) ? _cyclicBufferSize : 0)) << 1);
      const Byte *pb = cur - delta;
      unsigned len = (len0 < len1 ? len0 : len1);
      if (pb[len] == cur[len])
      {
        while (++len != lenLimit)
          if (pb[len] != cur[len])
            break;
        {
          if (len == lenLimit)
          {
            *ptr1 = pair[0];
            *ptr0 = pair[1];
            return;
          }
        }
      }
      if (pb[len] < cur[len])
      {
        *ptr1 = curMatch;
        curMatch = pair[1];
        ptr1 = pair + 1;
        len1 = len;
      }
      else
      {
        *ptr0 = curMatch;
        curMatch = pair[0];
        ptr0 = pair;
        len0 = len;
      }
    }
  }
  while(--cutValue && cmCheck < curMatch);
  
  *ptr0 = *ptr1 = kEmptyHashValue;
  return;
}


#define MOVE_POS   p->cyclicBufferPos++;   p->buffer++;   { const Uint pos1 = p->pos + 1;     p->pos = pos1;     if (pos1 == p->posLimit) MatchFinder_CheckLimits(p); }

#define MOVE_POS_RET MOVE_POS return distances;

static void MatchFinder_MovePos(CMatchFinder *p)
{
  
  MOVE_POS
}

#define GET_MATCHES_HEADER2(minLen, ret_op)   Uint hv; const Byte *cur; Uint curMatch;   Uint lenLimit = p->lenLimit;   if (lenLimit < minLen) { MatchFinder_MovePos(p);  ret_op; }   cur = p->buffer;

#define GET_MATCHES_HEADER(minLen) GET_MATCHES_HEADER2(minLen, return distances)
#define SKIP_HEADER(minLen)    do { GET_MATCHES_HEADER2(minLen, continue)

#define MF_PARAMS(p)  lenLimit, curMatch, p->pos, p->buffer, p->son,     p->cyclicBufferPos, p->cyclicBufferSize, p->cutValue

#define SKIP_FOOTER      SkipMatchesSpec(MF_PARAMS(p));     MOVE_POS   } while (--num);

#define GET_MATCHES_FOOTER_BASE(_maxLen_, func)   distances = func(MF_PARAMS(p), distances, (Uint)_maxLen_);   MOVE_POS_RET

#define GET_MATCHES_FOOTER_BT(_maxLen_)   GET_MATCHES_FOOTER_BASE(_maxLen_, GetMatchesSpec1)

#define GET_MATCHES_FOOTER_HC(_maxLen_)   GET_MATCHES_FOOTER_BASE(_maxLen_, Hc_GetMatchesSpec)



#define UPDATE_maxLen {     const ptrdiff_t diff = (ptrdiff_t)0 - (ptrdiff_t)d2;     const Byte *c = cur + maxLen;     const Byte *lim = cur + lenLimit;     for (; c != lim; c++) if (*(c + diff) != *c) break;     maxLen = (unsigned)(c - cur); }

static Uint* Bt2_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  GET_MATCHES_HEADER(2)
  HASH2_CALC
  curMatch = p->hash[hv];
  p->hash[hv] = p->pos;
  GET_MATCHES_FOOTER_BT(1)
}

#define SET_mmm    mmm = p->cyclicBufferSize;   if (pos < mmm)     mmm = pos;


static Uint* Bt3_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  Uint mmm;
  Uint h2, d2, pos;
  unsigned maxLen;
  Uint *hash;
  GET_MATCHES_HEADER(3)

  HASH3_CALC

  hash = p->hash;
  pos = p->pos;

  d2 = pos - hash[h2];

  curMatch = (hash + kFix3HashSize)[hv];
  
  hash[h2] = pos;
  (hash + kFix3HashSize)[hv] = pos;

  SET_mmm

  maxLen = 2;

  if (d2 < mmm && *(cur - d2) == *cur)
  {
    UPDATE_maxLen
    distances[0] = (Uint)maxLen;
    distances[1] = d2 - 1;
    distances += 2;
    if (maxLen == lenLimit)
    {
      SkipMatchesSpec(MF_PARAMS(p));
      MOVE_POS_RET
    }
  }
  
  GET_MATCHES_FOOTER_BT(maxLen)
}


static Uint* Bt4_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  Uint mmm;
  Uint h2, h3, d2, d3, pos;
  unsigned maxLen;
  Uint *hash;
  GET_MATCHES_HEADER(4)

  HASH4_CALC

  hash = p->hash;
  pos = p->pos;

  d2 = pos - hash                  [h2];
  d3 = pos - (hash + kFix3HashSize)[h3];
  curMatch = (hash + kFix4HashSize)[hv];

  hash                  [h2] = pos;
  (hash + kFix3HashSize)[h3] = pos;
  (hash + kFix4HashSize)[hv] = pos;

  SET_mmm

  maxLen = 3;
  
  for (;;)
  {
    if (d2 < mmm && *(cur - d2) == *cur)
    {
      distances[0] = 2;
      distances[1] = d2 - 1;
      distances += 2;
      if (*(cur - d2 + 2) == cur[2])
      {
        
      }
      else if (d3 < mmm && *(cur - d3) == *cur)
      {
        d2 = d3;
        distances[1] = d3 - 1;
        distances += 2;
      }
      else
        break;
    }
    else if (d3 < mmm && *(cur - d3) == *cur)
    {
      d2 = d3;
      distances[1] = d3 - 1;
      distances += 2;
    }
    else
      break;
  
    UPDATE_maxLen
    distances[-2] = (Uint)maxLen;
    if (maxLen == lenLimit)
    {
      SkipMatchesSpec(MF_PARAMS(p));
      MOVE_POS_RET
    }
    break;
  }
  
  GET_MATCHES_FOOTER_BT(maxLen)
}


static Uint* Bt5_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  Uint mmm;
  Uint h2, h3, d2, d3, pos;
  unsigned maxLen;
  Uint *hash;
  GET_MATCHES_HEADER(5)

  HASH5_CALC

  hash = p->hash;
  pos = p->pos;

  d2 = pos - hash                  [h2];
  d3 = pos - (hash + kFix3HashSize)[h3];
  

  curMatch = (hash + kFix5HashSize)[hv];

  hash                  [h2] = pos;
  (hash + kFix3HashSize)[h3] = pos;
  
  (hash + kFix5HashSize)[hv] = pos;

  SET_mmm

  maxLen = 4;

  for (;;)
  {
    if (d2 < mmm && *(cur - d2) == *cur)
    {
      distances[0] = 2;
      distances[1] = d2 - 1;
      distances += 2;
      if (*(cur - d2 + 2) == cur[2])
      {
      }
      else if (d3 < mmm && *(cur - d3) == *cur)
      {
        distances[1] = d3 - 1;
        distances += 2;
        d2 = d3;
      }
      else
        break;
    }
    else if (d3 < mmm && *(cur - d3) == *cur)
    {
      distances[1] = d3 - 1;
      distances += 2;
      d2 = d3;
    }
    else
      break;

    distances[-2] = 3;
    if (*(cur - d2 + 3) != cur[3])
      break;
    UPDATE_maxLen
    distances[-2] = (Uint)maxLen;
    if (maxLen == lenLimit)
    {
      SkipMatchesSpec(MF_PARAMS(p));
      MOVE_POS_RET
    }
    break;
  }
  
  GET_MATCHES_FOOTER_BT(maxLen)
}


static Uint* Hc4_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  Uint mmm;
  Uint h2, h3, d2, d3, pos;
  unsigned maxLen;
  Uint *hash;
  GET_MATCHES_HEADER(4)

  HASH4_CALC

  hash = p->hash;
  pos = p->pos;
  
  d2 = pos - hash                  [h2];
  d3 = pos - (hash + kFix3HashSize)[h3];
  curMatch = (hash + kFix4HashSize)[hv];

  hash                  [h2] = pos;
  (hash + kFix3HashSize)[h3] = pos;
  (hash + kFix4HashSize)[hv] = pos;

  SET_mmm

  maxLen = 3;

  for (;;)
  {
    if (d2 < mmm && *(cur - d2) == *cur)
    {
      distances[0] = 2;
      distances[1] = d2 - 1;
      distances += 2;
      if (*(cur - d2 + 2) == cur[2])
      {
        
      }
      else if (d3 < mmm && *(cur - d3) == *cur)
      {
        d2 = d3;
        distances[1] = d3 - 1;
        distances += 2;
      }
      else
        break;
    }
    else if (d3 < mmm && *(cur - d3) == *cur)
    {
      d2 = d3;
      distances[1] = d3 - 1;
      distances += 2;
    }
    else
      break;

    UPDATE_maxLen
    distances[-2] = (Uint)maxLen;
    if (maxLen == lenLimit)
    {
      p->son[p->cyclicBufferPos] = curMatch;
      MOVE_POS_RET
    }
    break;
  }
  
  GET_MATCHES_FOOTER_HC(maxLen)
}


static Uint * Hc5_MatchFinder_GetMatches(void *_p, Uint *distances)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  Uint mmm;
  Uint h2, h3, d2, d3, pos;
  unsigned maxLen;
  Uint *hash;
  GET_MATCHES_HEADER(5)

  HASH5_CALC

  hash = p->hash;
  pos = p->pos;

  d2 = pos - hash                  [h2];
  d3 = pos - (hash + kFix3HashSize)[h3];
  

  curMatch = (hash + kFix5HashSize)[hv];

  hash                  [h2] = pos;
  (hash + kFix3HashSize)[h3] = pos;
  
  (hash + kFix5HashSize)[hv] = pos;

  SET_mmm
  
  maxLen = 4;

  for (;;)
  {
    if (d2 < mmm && *(cur - d2) == *cur)
    {
      distances[0] = 2;
      distances[1] = d2 - 1;
      distances += 2;
      if (*(cur - d2 + 2) == cur[2])
      {
      }
      else if (d3 < mmm && *(cur - d3) == *cur)
      {
        distances[1] = d3 - 1;
        distances += 2;
        d2 = d3;
      }
      else
        break;
    }
    else if (d3 < mmm && *(cur - d3) == *cur)
    {
      distances[1] = d3 - 1;
      distances += 2;
      d2 = d3;
    }
    else
      break;

    distances[-2] = 3;
    if (*(cur - d2 + 3) != cur[3])
      break;
    UPDATE_maxLen
    distances[-2] = (Uint)maxLen;
    if (maxLen == lenLimit)
    {
      p->son[p->cyclicBufferPos] = curMatch;
      MOVE_POS_RET
    }
    break;
  }
  
  GET_MATCHES_FOOTER_HC(maxLen)
}

static void Bt2_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  SKIP_HEADER(2)
  {
    HASH2_CALC
    curMatch = p->hash[hv];
    p->hash[hv] = p->pos;
  }
  SKIP_FOOTER
}

static void Bt3_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  SKIP_HEADER(3)
  {
    Uint h2;
    Uint *hash;
    HASH3_CALC
    hash = p->hash;
    curMatch = (hash + kFix3HashSize)[hv];
    hash[h2] =
    (hash + kFix3HashSize)[hv] = p->pos;
  }
  SKIP_FOOTER
}

static void Bt4_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  SKIP_HEADER(4)
  {
    Uint h2, h3;
    Uint *hash;
    HASH4_CALC
    hash = p->hash;
    curMatch = (hash + kFix4HashSize)[hv];
    hash                  [h2] =
    (hash + kFix3HashSize)[h3] =
    (hash + kFix4HashSize)[hv] = p->pos;
  }
  SKIP_FOOTER
}

static void Bt5_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  SKIP_HEADER(5)
  {
    Uint h2, h3;
    Uint *hash;
    HASH5_CALC
    hash = p->hash;
    curMatch = (hash + kFix5HashSize)[hv];
    hash                  [h2] =
    (hash + kFix3HashSize)[h3] =
    
    (hash + kFix5HashSize)[hv] = p->pos;
  }
  SKIP_FOOTER
}


#define HC_SKIP_HEADER(minLen)     do { if (p->lenLimit < minLen) { MatchFinder_MovePos(p); num--; continue; } {     const Byte *cur;     Uint *hash;     Uint *son;     Uint pos = p->pos;     Uint num2 = num;          { const Uint rem = p->posLimit - pos; if (num2 > rem) num2 = rem; }     num -= num2;     { const Uint cycPos = p->cyclicBufferPos;       son = p->son + cycPos;       p->cyclicBufferPos = cycPos + num2; }     cur = p->buffer;     hash = p->hash;     do {     Uint curMatch;     Uint hv;


#define HC_SKIP_FOOTER     cur++;  pos++;  *son++ = curMatch;     } while (--num2);     p->buffer = cur;     p->pos = pos;     if (pos == p->posLimit) MatchFinder_CheckLimits(p);     }} while(num); 

static void Hc4_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  HC_SKIP_HEADER(4)

    Uint h2, h3;
    HASH4_CALC
    curMatch = (hash + kFix4HashSize)[hv];
    hash                  [h2] =
    (hash + kFix3HashSize)[h3] =
    (hash + kFix4HashSize)[hv] = pos;
  
  HC_SKIP_FOOTER
}


static void Hc5_MatchFinder_Skip(void *_p, Uint num)
{
  CMatchFinder *p = (CMatchFinder *)_p;
  HC_SKIP_HEADER(5)
  
    Uint h2, h3;
    HASH5_CALC
    curMatch = (hash + kFix5HashSize)[hv];
    hash                  [h2] =
    (hash + kFix3HashSize)[h3] =
    
    (hash + kFix5HashSize)[hv] = pos;
  
  HC_SKIP_FOOTER
}

void MatchFinder_CreateVTable(CMatchFinder *p, IMatchFinder2 *vTable)
{
  vTable->Init = MatchFinder_Init;
  vTable->GetNumAvailableBytes = MatchFinder_GetNumAvailableBytes;
  vTable->GetPointerToCurrentPos = MatchFinder_GetPointerToCurrentPos;
  if (!p->btMode)
  {
    if (p->numHashBytes <= 4)
    {
      vTable->GetMatches = Hc4_MatchFinder_GetMatches;
      vTable->Skip = Hc4_MatchFinder_Skip;
    }
    else
    {
      vTable->GetMatches = Hc5_MatchFinder_GetMatches;
      vTable->Skip = Hc5_MatchFinder_Skip;
    }
  }
  else if (p->numHashBytes == 2)
  {
    vTable->GetMatches = Bt2_MatchFinder_GetMatches;
    vTable->Skip = Bt2_MatchFinder_Skip;
  }
  else if (p->numHashBytes == 3)
  {
    vTable->GetMatches = Bt3_MatchFinder_GetMatches;
    vTable->Skip = Bt3_MatchFinder_Skip;
  }
  else if (p->numHashBytes == 4)
  {
    vTable->GetMatches = Bt4_MatchFinder_GetMatches;
    vTable->Skip = Bt4_MatchFinder_Skip;
  }
  else
  {
    vTable->GetMatches = Bt5_MatchFinder_GetMatches;
    vTable->Skip = Bt5_MatchFinder_Skip;
  }
}


#undef MOVE_POS
#undef MOVE_POS_RET

typedef UInt16 CLzmaProb;

#define LZMA_PROPS_SIZE 5

struct CLzmaProps
{
  Byte lc;
  Byte lp;
  Byte pb;
  Byte _pad_;
  Uint dicSize;
};

#define LZMA_REQUIRED_INPUT_MAX 20

struct CLzmaDec
{
  
  CLzmaProps prop;
  CLzmaProb *probs;
  CLzmaProb *probs_1664;
  Byte *dic;
  size_t dicBufSize;
  size_t dicPos;
  const Byte *buf;
  Uint range;
  Uint code;
  Uint processedPos;
  Uint checkDicSize;
  Uint reps[4];
  Uint state;
  Uint remainLen;

  Uint numProbs;
  unsigned tempBufSize;
  Byte tempBuf[LZMA_REQUIRED_INPUT_MAX];
};

#define LzmaDec_CONSTRUCT(p) { (p)->dic = 0; (p)->probs = 0; }

typedef enum
{
  LZMA_FINISH_ANY,   
} ELzmaFinishMode;



typedef enum
{
  LZMA_STATUS_NOT_SPECIFIED,               
  LZMA_STATUS_FINISHED_WITH_MARK,          
  LZMA_STATUS_NOT_FINISHED,                
  LZMA_STATUS_NEEDS_MORE_INPUT,            
  LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK  
} ELzmaStatus;




#define kTopValue ((Uint)1 << 24)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)

#define RC_INIT_SIZE 5


#define kNumMoveBits 5
#define NORMALIZE if (range < kTopValue) { range <<= 8; code = (code << 8) | (*buf++); }

#define IF_BIT_0(p) ttt = *(p); NORMALIZE; bound = (range >> kNumBitModelTotalBits) * (Uint)ttt; if (code < bound)
#define UPDATE_0(p) range = bound; *(p) = (CLzmaProb)(ttt + ((kBitModelTotal - ttt) >> kNumMoveBits));
#define UPDATE_1(p) range -= bound; code -= bound; *(p) = (CLzmaProb)(ttt - (ttt >> kNumMoveBits));
#define GET_BIT2(p, i, A0, A1) IF_BIT_0(p)   { UPDATE_0(p)  i = (i + i); A0; } else   { UPDATE_1(p)  i = (i + i) + 1; A1; }

#define TREE_GET_BIT(probs, i) { GET_BIT2(probs + i, i, ;, ;); }

#define REV_BIT(p, i, A0, A1) IF_BIT_0(p + i)   { UPDATE_0(p + i)  A0; } else   { UPDATE_1(p + i)  A1; }
#define REV_BIT_VAR(  p, i, m) REV_BIT(p, i, i += m; m += m, m += m; i += m; )
#define REV_BIT_CONST(p, i, m) REV_BIT(p, i, i += m;       , i += m * 2; )
#define REV_BIT_LAST( p, i, m) REV_BIT(p, i, i -= m        , ; )

#define TREE_DECODE(probs, limit, i)   { i = 1; do { TREE_GET_BIT(probs, i); } while (i < limit); i -= limit; }



#define TREE_6_DECODE(probs, i)   { i = 1;   TREE_GET_BIT(probs, i)   TREE_GET_BIT(probs, i)   TREE_GET_BIT(probs, i)   TREE_GET_BIT(probs, i)   TREE_GET_BIT(probs, i)   TREE_GET_BIT(probs, i)   i -= 0x40; }

#define NORMAL_LITER_DEC TREE_GET_BIT(prob, symbol)
#define MATCHED_LITER_DEC   matchByte += matchByte;   bit = offs;   offs &= matchByte;   probLit = prob + (offs + bit + symbol);   GET_BIT2(probLit, symbol, offs ^= bit; , ;)



#define NORMALIZE_CHECK if (range < kTopValue) { if (buf >= bufLimit) return DUMMY_INPUT_EOF; range <<= 8; code = (code << 8) | (*buf++); }

#define IF_BIT_0_CHECK(p) ttt = *(p); NORMALIZE_CHECK bound = (range >> kNumBitModelTotalBits) * (Uint)ttt; if (code < bound)
#define UPDATE_0_CHECK range = bound;
#define UPDATE_1_CHECK range -= bound; code -= bound;
#define GET_BIT2_CHECK(p, i, A0, A1) IF_BIT_0_CHECK(p)   { UPDATE_0_CHECK  i = (i + i); A0; } else   { UPDATE_1_CHECK  i = (i + i) + 1; A1; }
#define GET_BIT_CHECK(p, i) GET_BIT2_CHECK(p, i, ; , ;)
#define TREE_DECODE_CHECK(probs, limit, i)   { i = 1; do { GET_BIT_CHECK(probs + i, i) } while (i < limit); i -= limit; }


#define REV_BIT_CHECK(p, i, m) IF_BIT_0_CHECK(p + i)   { UPDATE_0_CHECK  i += m; m += m; } else   { UPDATE_1_CHECK  m += m; i += m; }


#define kNumPosBitsMax 4
#define kNumPosStatesMax (1 << kNumPosBitsMax)

#define kLenNumLowBits 3
#define kLenNumLowSymbols (1 << kLenNumLowBits)
#define kLenNumHighBits 8
#define kLenNumHighSymbols (1 << kLenNumHighBits)

#define LenLow 0
#define LenHigh (LenLow + 2 * (kNumPosStatesMax << kLenNumLowBits))
#define kNumLenProbs (LenHigh + kLenNumHighSymbols)

#define LenChoice LenLow
#define LenChoice2 (LenLow + (1 << kLenNumLowBits))

#define kNumStates 12
#define kNumStates2 16
#define kNumLitStates 7

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumFullDistances (1 << (kEndPosModelIndex >> 1))

#define kNumPosSlotBits 6
#define kNumLenToPosStates 4

#define kNumAlignBits 4
#define kAlignTableSize (1 << kNumAlignBits)

#define kMatchMinLen 2
#define kMatchSpecLenStart (kMatchMinLen + kLenNumLowSymbols * 2 + kLenNumHighSymbols)

#define kMatchSpecLen_Error_Data (1 << 9)
#define kMatchSpecLen_Error_Fail (kMatchSpecLen_Error_Data - 1)

#define kStartOffset 1664
#define GET_PROBS p->probs_1664

#define SpecPos (-kStartOffset)
#define IsRep0Long (SpecPos + kNumFullDistances)
#define RepLenCoder (IsRep0Long + (kNumStates2 << kNumPosBitsMax))
#define LenCoder (RepLenCoder + kNumLenProbs)
#define IsMatch (LenCoder + kNumLenProbs)
#define Align (IsMatch + (kNumStates2 << kNumPosBitsMax))
#define IsRep (Align + kAlignTableSize)
#define IsRepG0 (IsRep + kNumStates)
#define IsRepG1 (IsRepG0 + kNumStates)
#define IsRepG2 (IsRepG1 + kNumStates)
#define PosSlot (IsRepG2 + kNumStates)
#define Literal (PosSlot + (kNumLenToPosStates << kNumPosSlotBits))
#define NUM_BASE_PROBS (Literal + kStartOffset)

#define LZMA_LIT_SIZE 0x300

#define LzmaProps_GetNumProbs(p) (NUM_BASE_PROBS + ((Uint)LZMA_LIT_SIZE << ((p)->lc + (p)->lp)))


#define CALC_POS_STATE(processedPos, pbMask) (((processedPos) & (pbMask)) << 4)
#define COMBINED_PS_STATE (posState + state)
#define GET_LEN_STATE (posState)

#define LZMA_DIC_MIN (1 << 12)

static
int  LzmaDec_DecodeReal_3(CLzmaDec *p, size_t limit, const Byte *bufLimit)
{
  CLzmaProb *probs = GET_PROBS;
  unsigned state = (unsigned)p->state;
  Uint rep0 = p->reps[0], rep1 = p->reps[1], rep2 = p->reps[2], rep3 = p->reps[3];
  unsigned pbMask = ((unsigned)1 << (p->prop.pb)) - 1;
  unsigned lc = p->prop.lc;
  unsigned lpMask = ((unsigned)0x100 << p->prop.lp) - ((unsigned)0x100 >> lc);

  Byte *dic = p->dic;
  size_t dicBufSize = p->dicBufSize;
  size_t dicPos = p->dicPos;
  
  Uint processedPos = p->processedPos;
  Uint checkDicSize = p->checkDicSize;
  unsigned len = 0;

  const Byte *buf = p->buf;
  Uint range = p->range;
  Uint code = p->code;

  do
  {
    CLzmaProb *prob;
    Uint bound;
    unsigned ttt;
    unsigned posState = CALC_POS_STATE(processedPos, pbMask);

    prob = probs + IsMatch + COMBINED_PS_STATE;
    IF_BIT_0(prob)
    {
      unsigned symbol;
      UPDATE_0(prob)
      prob = probs + Literal;
      if (processedPos != 0 || checkDicSize != 0)
        prob += (Uint)3 * ((((processedPos << 8) + dic[(dicPos == 0 ? dicBufSize : dicPos) - 1]) & lpMask) << lc);
      processedPos++;

      if (state < kNumLitStates)
      {
        state -= (state < 4) ? state : 3;
        symbol = 1;
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
        NORMAL_LITER_DEC
      }
      else
      {
        unsigned matchByte = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
        unsigned offs = 0x100;
        state -= (state < 10) ? 3 : 6;
        symbol = 1;
        {
          unsigned bit;
          CLzmaProb *probLit;
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
          MATCHED_LITER_DEC
        }
      }

      dic[dicPos++] = (Byte)symbol;
      continue;
    }
    
    {
      UPDATE_1(prob)
      prob = probs + IsRep + state;
      IF_BIT_0(prob)
      {
        UPDATE_0(prob)
        state += kNumStates;
        prob = probs + LenCoder;
      }
      else
      {
        UPDATE_1(prob)
        prob = probs + IsRepG0 + state;
        IF_BIT_0(prob)
        {
          UPDATE_0(prob)
          prob = probs + IsRep0Long + COMBINED_PS_STATE;
          IF_BIT_0(prob)
          {
            UPDATE_0(prob)
  
            
            
            
            
            
            
            dic[dicPos] = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
            dicPos++;
            processedPos++;
            state = state < kNumLitStates ? 9 : 11;
            continue;
          }
          UPDATE_1(prob)
        }
        else
        {
          Uint distance;
          UPDATE_1(prob)
          prob = probs + IsRepG1 + state;
          IF_BIT_0(prob)
          {
            UPDATE_0(prob)
            distance = rep1;
          }
          else
          {
            UPDATE_1(prob)
            prob = probs + IsRepG2 + state;
            IF_BIT_0(prob)
            {
              UPDATE_0(prob)
              distance = rep2;
            }
            else
            {
              UPDATE_1(prob)
              distance = rep3;
              rep3 = rep2;
            }
            rep2 = rep1;
          }
          rep1 = rep0;
          rep0 = distance;
        }
        state = state < kNumLitStates ? 8 : 11;
        prob = probs + RepLenCoder;
      }
      
      {
        CLzmaProb *probLen = prob + LenChoice;
        IF_BIT_0(probLen)
        {
          UPDATE_0(probLen)
          probLen = prob + LenLow + GET_LEN_STATE;
          len = 1;
          TREE_GET_BIT(probLen, len)
          TREE_GET_BIT(probLen, len)
          TREE_GET_BIT(probLen, len)
          len -= 8;
        }
        else
        {
          UPDATE_1(probLen)
          probLen = prob + LenChoice2;
          IF_BIT_0(probLen)
          {
            UPDATE_0(probLen)
            probLen = prob + LenLow + GET_LEN_STATE + (1 << kLenNumLowBits);
            len = 1;
            TREE_GET_BIT(probLen, len)
            TREE_GET_BIT(probLen, len)
            TREE_GET_BIT(probLen, len)
          }
          else
          {
            UPDATE_1(probLen)
            probLen = prob + LenHigh;
            TREE_DECODE(probLen, (1 << kLenNumHighBits), len)
            len += kLenNumLowSymbols * 2;
          }
        }
      }

      if (state >= kNumStates)
      {
        Uint distance;
        prob = probs + PosSlot +
            ((len < kNumLenToPosStates ? len : kNumLenToPosStates - 1) << kNumPosSlotBits);
        TREE_6_DECODE(prob, distance)
        if (distance >= kStartPosModelIndex)
        {
          unsigned posSlot = (unsigned)distance;
          unsigned numDirectBits = (unsigned)(((distance >> 1) - 1));
          distance = (2 | (distance & 1));
          if (posSlot < kEndPosModelIndex)
          {
            distance <<= numDirectBits;
            prob = probs + SpecPos;
            {
              Uint m = 1;
              distance++;
              do
              {
                REV_BIT_VAR(prob, distance, m)
              }
              while (--numDirectBits);
              distance -= m;
            }
          }
          else
          {
            numDirectBits -= kNumAlignBits;
            do
            {
              NORMALIZE
              range >>= 1;
              
              {
                Uint t;
                code -= range;
                t = (0 - ((Uint)code >> 31)); 
                distance = (distance << 1) + (t + 1);
                code += range & t;
              }
              
            }
            while (--numDirectBits);
            prob = probs + Align;
            distance <<= kNumAlignBits;
            {
              unsigned i = 1;
              REV_BIT_CONST(prob, i, 1)
              REV_BIT_CONST(prob, i, 2)
              REV_BIT_CONST(prob, i, 4)
              REV_BIT_LAST (prob, i, 8)
              distance |= i;
            }
            if (distance == (Uint)0xFFFFFFFF)
            {
              len = kMatchSpecLenStart;
              state -= kNumStates;
              break;
            }
          }
        }
        
        rep3 = rep2;
        rep2 = rep1;
        rep1 = rep0;
        rep0 = distance + 1;
        state = (state < kNumStates + kNumLitStates) ? kNumLitStates : kNumLitStates + 3;
        if (distance >= (checkDicSize == 0 ? processedPos: checkDicSize))
        {
          len += kMatchSpecLen_Error_Data + kMatchMinLen;
          
          
          break;
        }
      }

      len += kMatchMinLen;

      {
        size_t rem;
        unsigned curLen;
        size_t pos;
        
        if ((rem = limit - dicPos) == 0)
        {
          
          break;
        }
        
        curLen = ((rem < len) ? (unsigned)rem : len);
        pos = dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0);

        processedPos += (Uint)curLen;

        len -= curLen;
        if (curLen <= dicBufSize - pos)
        {
          Byte *dest = dic + dicPos;
          ptrdiff_t src = (ptrdiff_t)pos - (ptrdiff_t)dicPos;
          const Byte *lim = dest + curLen;
          dicPos += (size_t)curLen;
          do
            *(dest) = (Byte)*(dest + src);
          while (++dest != lim);
        }
        else
        {
          do
          {
            dic[dicPos++] = dic[pos];
            if (++pos == dicBufSize)
              pos = 0;
          }
          while (--curLen != 0);
        }
      }
    }
  }
  while (dicPos < limit && buf < bufLimit);

  NORMALIZE
  
  p->buf = buf;
  p->range = range;
  p->code = code;
  p->remainLen = (Uint)len; 
  p->dicPos = dicPos;
  p->processedPos = processedPos;
  p->reps[0] = rep0;
  p->reps[1] = rep1;
  p->reps[2] = rep2;
  p->reps[3] = rep3;
  p->state = (Uint)state;
  if (len >= kMatchSpecLen_Error_Data)
    return SZ_ERROR_DATA;
  return SZ_OK;
}



static void  LzmaDec_WriteRem(CLzmaDec *p, size_t limit)
{
  unsigned len = (unsigned)p->remainLen;
  if (len == 0 )
    return;
  {
    size_t dicPos = p->dicPos;
    Byte *dic;
    size_t dicBufSize;
    size_t rep0;   
    {
      size_t rem = limit - dicPos;
      if (rem < len)
      {
        len = (unsigned)(rem);
        if (len == 0)
          return;
      }
    }

    if (p->checkDicSize == 0 && p->prop.dicSize - p->processedPos <= len)
      p->checkDicSize = p->prop.dicSize;

    p->processedPos += (Uint)len;
    p->remainLen -= (Uint)len;
    dic = p->dic;
    rep0 = p->reps[0];
    dicBufSize = p->dicBufSize;
    do
    {
      dic[dicPos] = dic[dicPos - rep0 + (dicPos < rep0 ? dicBufSize : 0)];
      dicPos++;
    }
    while (--len);
    p->dicPos = dicPos;
  }
}


#define kRange0 0xFFFFFFFF
#define kBound0 ((kRange0 >> kNumBitModelTotalBits) << (kNumBitModelTotalBits - 1))
#define kBadRepCode (kBound0 + (((kRange0 - kBound0) >> kNumBitModelTotalBits) << (kNumBitModelTotalBits - 1)))

static int  LzmaDec_DecodeReal2(CLzmaDec *p, size_t limit, const Byte *bufLimit)
{
  if (p->checkDicSize == 0)
  {
    Uint rem = p->prop.dicSize - p->processedPos;
    if (limit - p->dicPos > rem)
      limit = p->dicPos + rem;
  }
  {
    int res = LzmaDec_DecodeReal_3(p, limit, bufLimit);
    if (p->checkDicSize == 0 && p->processedPos >= p->prop.dicSize)
      p->checkDicSize = p->prop.dicSize;
    return res;
  }
}



typedef enum
{
  DUMMY_INPUT_EOF, 
  DUMMY_LIT,
  DUMMY_MATCH,
  DUMMY_REP
} ELzmaDummy;


#define IS_DUMMY_END_MARKER_POSSIBLE(dummyRes) ((dummyRes) == DUMMY_MATCH)

static ELzmaDummy LzmaDec_TryDummy(const CLzmaDec *p, const Byte *buf, const Byte **bufOut)
{
  Uint range = p->range;
  Uint code = p->code;
  const Byte *bufLimit = *bufOut;
  const CLzmaProb *probs = GET_PROBS;
  unsigned state = (unsigned)p->state;
  ELzmaDummy res;

  for (;;)
  {
    const CLzmaProb *prob;
    Uint bound;
    unsigned ttt;
    unsigned posState = CALC_POS_STATE(p->processedPos, ((unsigned)1 << p->prop.pb) - 1);

    prob = probs + IsMatch + COMBINED_PS_STATE;
    IF_BIT_0_CHECK(prob)
    {
      UPDATE_0_CHECK

      prob = probs + Literal;
      if (p->checkDicSize != 0 || p->processedPos != 0)
        prob += ((Uint)LZMA_LIT_SIZE *
            ((((p->processedPos) & (((unsigned)1 << (p->prop.lp)) - 1)) << p->prop.lc) +
            ((unsigned)p->dic[(p->dicPos == 0 ? p->dicBufSize : p->dicPos) - 1] >> (8 - p->prop.lc))));

      if (state < kNumLitStates)
      {
        unsigned symbol = 1;
        do { GET_BIT_CHECK(prob + symbol, symbol) } while (symbol < 0x100);
      }
      else
      {
        unsigned matchByte = p->dic[p->dicPos - p->reps[0] +
            (p->dicPos < p->reps[0] ? p->dicBufSize : 0)];
        unsigned offs = 0x100;
        unsigned symbol = 1;
        do
        {
          unsigned bit;
          const CLzmaProb *probLit;
          matchByte += matchByte;
          bit = offs;
          offs &= matchByte;
          probLit = prob + (offs + bit + symbol);
          GET_BIT2_CHECK(probLit, symbol, offs ^= bit; , ; )
        }
        while (symbol < 0x100);
      }
      res = DUMMY_LIT;
    }
    else
    {
      unsigned len;
      UPDATE_1_CHECK

      prob = probs + IsRep + state;
      IF_BIT_0_CHECK(prob)
      {
        UPDATE_0_CHECK
        state = 0;
        prob = probs + LenCoder;
        res = DUMMY_MATCH;
      }
      else
      {
        UPDATE_1_CHECK
        res = DUMMY_REP;
        prob = probs + IsRepG0 + state;
        IF_BIT_0_CHECK(prob)
        {
          UPDATE_0_CHECK
          prob = probs + IsRep0Long + COMBINED_PS_STATE;
          IF_BIT_0_CHECK(prob)
          {
            UPDATE_0_CHECK
            break;
          }
          else
          {
            UPDATE_1_CHECK
          }
        }
        else
        {
          UPDATE_1_CHECK
          prob = probs + IsRepG1 + state;
          IF_BIT_0_CHECK(prob)
          {
            UPDATE_0_CHECK
          }
          else
          {
            UPDATE_1_CHECK
            prob = probs + IsRepG2 + state;
            IF_BIT_0_CHECK(prob)
            {
              UPDATE_0_CHECK
            }
            else
            {
              UPDATE_1_CHECK
            }
          }
        }
        state = kNumStates;
        prob = probs + RepLenCoder;
      }
      {
        unsigned limit, offset;
        const CLzmaProb *probLen = prob + LenChoice;
        IF_BIT_0_CHECK(probLen)
        {
          UPDATE_0_CHECK
          probLen = prob + LenLow + GET_LEN_STATE;
          offset = 0;
          limit = 1 << kLenNumLowBits;
        }
        else
        {
          UPDATE_1_CHECK
          probLen = prob + LenChoice2;
          IF_BIT_0_CHECK(probLen)
          {
            UPDATE_0_CHECK
            probLen = prob + LenLow + GET_LEN_STATE + (1 << kLenNumLowBits);
            offset = kLenNumLowSymbols;
            limit = 1 << kLenNumLowBits;
          }
          else
          {
            UPDATE_1_CHECK
            probLen = prob + LenHigh;
            offset = kLenNumLowSymbols * 2;
            limit = 1 << kLenNumHighBits;
          }
        }
        TREE_DECODE_CHECK(probLen, limit, len)
        len += offset;
      }

      if (state < 4)
      {
        unsigned posSlot;
        prob = probs + PosSlot +
            ((len < kNumLenToPosStates - 1 ? len : kNumLenToPosStates - 1) <<
            kNumPosSlotBits);
        TREE_DECODE_CHECK(prob, 1 << kNumPosSlotBits, posSlot)
        if (posSlot >= kStartPosModelIndex)
        {
          unsigned numDirectBits = ((posSlot >> 1) - 1);

          if (posSlot < kEndPosModelIndex)
          {
            prob = probs + SpecPos + ((2 | (posSlot & 1)) << numDirectBits);
          }
          else
          {
            numDirectBits -= kNumAlignBits;
            do
            {
              NORMALIZE_CHECK
              range >>= 1;
              code -= range & (((code - range) >> 31) - 1);
              
            }
            while (--numDirectBits);
            prob = probs + Align;
            numDirectBits = kNumAlignBits;
          }
          {
            unsigned i = 1;
            unsigned m = 1;
            do
            {
              REV_BIT_CHECK(prob, i, m)
            }
            while (--numDirectBits);
          }
        }
      }
    }
    break;
  }
  NORMALIZE_CHECK

  *bufOut = buf;
  return res;
}

void LzmaDec_InitDicAndState(CLzmaDec *p, int initDic, int initState)
{
  p->remainLen = kMatchSpecLenStart + 1;
  p->tempBufSize = 0;

  if (initDic)
  {
    p->processedPos = 0;
    p->checkDicSize = 0;
    p->remainLen = kMatchSpecLenStart + 2;
  }
  if (initState)
    p->remainLen = kMatchSpecLenStart + 2;
}

void LzmaDec_Init(CLzmaDec *p)
{
  p->dicPos = 0;
  LzmaDec_InitDicAndState(p, 1, 1);
}





#define RETURN_NOT_FINISHED_FOR_FINISH   *status = LZMA_STATUS_NOT_FINISHED;   return SZ_ERROR_DATA; 
  


int LzmaDec_DecodeToDic(CLzmaDec *p, size_t dicLimit, const Byte *src, size_t *srcLen,
    ELzmaFinishMode finishMode, ELzmaStatus *status)
{
  size_t inSize = *srcLen;
  (*srcLen) = 0;
  *status = LZMA_STATUS_NOT_SPECIFIED;

  if (p->remainLen > kMatchSpecLenStart)
  {
    if (p->remainLen > kMatchSpecLenStart + 2)
      return p->remainLen == kMatchSpecLen_Error_Fail ? SZ_ERROR_FAIL : SZ_ERROR_DATA;

    for (; inSize > 0 && p->tempBufSize < RC_INIT_SIZE; (*srcLen)++, inSize--)
      p->tempBuf[p->tempBufSize++] = *src++;
    if (p->tempBufSize != 0 && p->tempBuf[0] != 0)
      return SZ_ERROR_DATA;
    if (p->tempBufSize < RC_INIT_SIZE)
    {
      *status = LZMA_STATUS_NEEDS_MORE_INPUT;
      return SZ_OK;
    }
    p->code =
        ((Uint)p->tempBuf[1] << 24)
      | ((Uint)p->tempBuf[2] << 16)
      | ((Uint)p->tempBuf[3] << 8)
      | ((Uint)p->tempBuf[4]);

    if (p->checkDicSize == 0
        && p->processedPos == 0
        && p->code >= kBadRepCode)
      return SZ_ERROR_DATA;

    p->range = 0xFFFFFFFF;
    p->tempBufSize = 0;

    if (p->remainLen > kMatchSpecLenStart + 1)
    {
      size_t numProbs = LzmaProps_GetNumProbs(&p->prop);
      size_t i;
      CLzmaProb *probs = p->probs;
      for (i = 0; i < numProbs; i++)
        probs[i] = kBitModelTotal >> 1;
      p->reps[0] = p->reps[1] = p->reps[2] = p->reps[3] = 1;
      p->state = 0;
    }

    p->remainLen = 0;
  }

  for (;;)
  {
    if (p->remainLen == kMatchSpecLenStart)
    {
      if (p->code != 0)
        return SZ_ERROR_DATA;
      *status = LZMA_STATUS_FINISHED_WITH_MARK;
      return SZ_OK;
    }

    LzmaDec_WriteRem(p, dicLimit);

    {
      

      int checkEndMarkNow = 0;

      if (p->dicPos >= dicLimit)
      {
        if (p->remainLen == 0 && p->code == 0)
        {
          *status = LZMA_STATUS_MAYBE_FINISHED_WITHOUT_MARK;
          return SZ_OK;
        }
        if (finishMode == LZMA_FINISH_ANY)
        {
          *status = LZMA_STATUS_NOT_FINISHED;
          return SZ_OK;
        }
        if (p->remainLen != 0)
        {
          RETURN_NOT_FINISHED_FOR_FINISH
        }
        checkEndMarkNow = 1;
      }

      

      if (p->tempBufSize == 0)
      {
        const Byte *bufLimit;
        int dummyProcessed = -1;
        
        if (inSize < LZMA_REQUIRED_INPUT_MAX || checkEndMarkNow)
        {
          const Byte *bufOut = src + inSize;
          
          ELzmaDummy dummyRes = LzmaDec_TryDummy(p, src, &bufOut);
          
          if (dummyRes == DUMMY_INPUT_EOF)
          {
            size_t i;
            if (inSize >= LZMA_REQUIRED_INPUT_MAX)
              break;
            (*srcLen) += inSize;
            p->tempBufSize = (unsigned)inSize;
            for (i = 0; i < inSize; i++)
              p->tempBuf[i] = src[i];
            *status = LZMA_STATUS_NEEDS_MORE_INPUT;
            return SZ_OK;
          }
 
          dummyProcessed = (int)(bufOut - src);
          if ((unsigned)dummyProcessed > LZMA_REQUIRED_INPUT_MAX)
            break;
          
          if (checkEndMarkNow && !IS_DUMMY_END_MARKER_POSSIBLE(dummyRes))
          {
            unsigned i;
            (*srcLen) += (unsigned)dummyProcessed;
            p->tempBufSize = (unsigned)dummyProcessed;
            for (i = 0; i < (unsigned)dummyProcessed; i++)
              p->tempBuf[i] = src[i];
            
            RETURN_NOT_FINISHED_FOR_FINISH
          }
          
          bufLimit = src;
          
        }
        else
          bufLimit = src + inSize - LZMA_REQUIRED_INPUT_MAX;

        p->buf = src;
        
        {
          int res = LzmaDec_DecodeReal2(p, dicLimit, bufLimit);
          
          size_t processed = (size_t)(p->buf - src);

          if (dummyProcessed < 0)
          {
            if (processed > inSize)
              break;
          }
          else if ((unsigned)dummyProcessed != processed)
            break;

          src += processed;
          inSize -= processed;
          (*srcLen) += processed;

          if (res != SZ_OK)
          {
            p->remainLen = kMatchSpecLen_Error_Data;
            return SZ_ERROR_DATA;
          }
        }
        continue;
      }

      {
        
        
        

        unsigned rem = p->tempBufSize;
        unsigned ahead = 0;
        int dummyProcessed = -1;
        
        while (rem < LZMA_REQUIRED_INPUT_MAX && ahead < inSize)
          p->tempBuf[rem++] = src[ahead++];
        
        
        
        
        if (rem < LZMA_REQUIRED_INPUT_MAX || checkEndMarkNow)
        {
          const Byte *bufOut = p->tempBuf + rem;
        
          ELzmaDummy dummyRes = LzmaDec_TryDummy(p, p->tempBuf, &bufOut);
          
          if (dummyRes == DUMMY_INPUT_EOF)
          {
            if (rem >= LZMA_REQUIRED_INPUT_MAX)
              break;
            p->tempBufSize = rem;
            (*srcLen) += (size_t)ahead;
            *status = LZMA_STATUS_NEEDS_MORE_INPUT;
            return SZ_OK;
          }
          
          dummyProcessed = (int)(bufOut - p->tempBuf);

          if ((unsigned)dummyProcessed < p->tempBufSize)
            break;

          if (checkEndMarkNow && !IS_DUMMY_END_MARKER_POSSIBLE(dummyRes))
          {
            (*srcLen) += (unsigned)dummyProcessed - p->tempBufSize;
            p->tempBufSize = (unsigned)dummyProcessed;
            
            RETURN_NOT_FINISHED_FOR_FINISH
          }
        }

        p->buf = p->tempBuf;
        
        {
          
          int res = LzmaDec_DecodeReal2(p, dicLimit, p->buf);

          size_t processed = (size_t)(p->buf - p->tempBuf);
          rem = p->tempBufSize;
          
          if (dummyProcessed < 0)
          {
            if (processed > LZMA_REQUIRED_INPUT_MAX)
              break;
            if (processed < rem)
              break;
          }
          else if ((unsigned)dummyProcessed != processed)
            break;
          
          processed -= rem;

          src += processed;
          inSize -= processed;
          (*srcLen) += processed;
          p->tempBufSize = 0;
          
          if (res != SZ_OK)
          {
            p->remainLen = kMatchSpecLen_Error_Data;
            return SZ_ERROR_DATA;
          }
        }
      }
    }
  }

  
  p->remainLen = kMatchSpecLen_Error_Fail;
  return SZ_ERROR_FAIL;
}

void LzmaDec_FreeProbs(CLzmaDec *p)
{
  free(p->probs);
  p->probs = 0;
}

int LzmaProps_Decode(CLzmaProps *p, const Byte *data, unsigned size)
{
  Uint dicSize;
  Byte d;
  
  if (size < LZMA_PROPS_SIZE)
    return SZ_ERROR_UNSUPPORTED;
  else
    dicSize = data[1] | ((Uint)data[2] << 8) | ((Uint)data[3] << 16) | ((Uint)data[4] << 24);
 
  if (dicSize < LZMA_DIC_MIN)
    dicSize = LZMA_DIC_MIN;
  p->dicSize = dicSize;

  d = data[0];
  if (d >= (9 * 5 * 5))
    return SZ_ERROR_UNSUPPORTED;

  p->lc = (Byte)(d % 9);
  d /= 9;
  p->pb = (Byte)(d / 5);
  p->lp = (Byte)(d % 5);

  return SZ_OK;
}

static int LzmaDec_AllocateProbs2(CLzmaDec *p, const CLzmaProps *propNew)
{
  Uint numProbs = LzmaProps_GetNumProbs(propNew);
  if (!p->probs || numProbs != p->numProbs)
  {
    LzmaDec_FreeProbs(p);
    p->probs = (CLzmaProb *)SzAlloc(numProbs * sizeof(CLzmaProb));
    if (!p->probs)
      return SZ_ERROR_MEM;
    p->probs_1664 = p->probs + 1664;
    p->numProbs = numProbs;
  }
  return SZ_OK;
}

int LzmaDec_AllocateProbs(CLzmaDec *p, const Byte *props, unsigned propsSize)
{
  CLzmaProps propNew;
  RINOK(LzmaProps_Decode(&propNew, props, propsSize))
  RINOK(LzmaDec_AllocateProbs2(p, &propNew))
  p->prop = propNew;
  return SZ_OK;
}

int LzmaDecode(Byte *dest, size_t *destLen, const Byte *src, size_t *srcLen,
    const Byte *propData, unsigned propSize, ELzmaFinishMode finishMode,
    ELzmaStatus *status)
{
  CLzmaDec p;
  int res;
  size_t outSize = *destLen, inSize = *srcLen;
  *destLen = *srcLen = 0;
  *status = LZMA_STATUS_NOT_SPECIFIED;
  if (inSize < RC_INIT_SIZE)
    return SZ_ERROR_INPUT_EOF;
  LzmaDec_CONSTRUCT(&p)
  RINOK(LzmaDec_AllocateProbs(&p, propData, propSize))
  p.dic = dest;
  p.dicBufSize = outSize;
  LzmaDec_Init(&p);
  *srcLen = inSize;
  res = LzmaDec_DecodeToDic(&p, outSize, src, srcLen, finishMode, status);
  *destLen = p.dicPos;
  if (res == SZ_OK && *status == LZMA_STATUS_NEEDS_MORE_INPUT)
    res = SZ_ERROR_INPUT_EOF;
  LzmaDec_FreeProbs(&p);
  return res;
}

struct CLzmaEncProps
{
  int level;       
  Uint dictSize; 
  int lc, lp, pb, algo, fb, btMode, numHashBytes; 
  unsigned numHashOutBits;  
  Uint mc;       
  unsigned writeEndMark;  
  int numThreads;  

  uint64_t reduceSize, affinity;
};


struct CLzmaEnc;

#define kLzmaMaxHistorySize ((Uint)15 << 28)


#define kTopValue ((Uint)1 << 24)

#define kNumBitModelTotalBits 11
#define kBitModelTotal (1 << kNumBitModelTotalBits)
#define kNumMoveBits 5
#define kProbInitValue (kBitModelTotal >> 1)

#define kNumMoveReducingBits 4
#define kNumBitPriceShiftBits 4


#define REP_LEN_COUNT 64

void LzmaEncProps_Init(CLzmaEncProps *p)
{
  p->level = 5;
  p->dictSize = p->mc = 0;
  p->reduceSize = (uint64_t)(int64_t)-1;
  p->lc = p->lp = p->pb = p->algo = p->fb = p->btMode = p->numHashBytes = p->numThreads = -1;
  p->numHashOutBits = 0;
  p->writeEndMark = 0;
  p->affinity = 0;
}

void LzmaEncProps_Normalize(CLzmaEncProps *p)
{
  int level = p->level;
  if (level < 0) level = 5;
  p->level = level;
  
  if (p->dictSize == 0)
    p->dictSize = (unsigned)level <= 4 ?
        (Uint)1 << (level * 2 + 16) :
        (unsigned)level <= sizeof(size_t) / 2 + 4 ?
          (Uint)1 << (level + 20) :
          (Uint)1 << (sizeof(size_t) / 2 + 24);

  if (p->dictSize > p->reduceSize)
  {
    Uint v = (Uint)p->reduceSize;
    const Uint kReduceMin = ((Uint)1 << 12);
    if (v < kReduceMin)
      v = kReduceMin;
    if (p->dictSize > v)
      p->dictSize = v;
  }

  if (p->lc < 0) p->lc = 3;
  if (p->lp < 0) p->lp = 0;
  if (p->pb < 0) p->pb = 2;

  if (p->algo < 0) p->algo = (unsigned)level < 5 ? 0 : 1;
  if (p->fb < 0) p->fb = (unsigned)level < 7 ? 32 : 64;
  if (p->btMode < 0) p->btMode = (p->algo == 0 ? 0 : 1);
  if (p->numHashBytes < 0) p->numHashBytes = (p->btMode ? 4 : 5);
  if (p->mc == 0) p->mc = (16 + ((unsigned)p->fb >> 1)) >> (p->btMode ? 0 : 1);
  
  if (p->numThreads < 0)
    p->numThreads = 1;
}


#define kNumLogBits (11 + sizeof(size_t) / 8 * 3)

#define kDicLogSizeMaxCompress ((kNumLogBits - 1) * 2 + 7)

static void LzmaEnc_FastPosInit(Byte *g_FastPos)
{
  unsigned slot;
  g_FastPos[0] = 0;
  g_FastPos[1] = 1;
  g_FastPos += 2;
  
  for (slot = 2; slot < kNumLogBits * 2; slot++)
  {
    size_t k = ((size_t)1 << ((slot >> 1) - 1));
    size_t j;
    for (j = 0; j < k; j++)
      g_FastPos[j] = (Byte)slot;
    g_FastPos += k;
  }
}


#define BSR2_RET(pos, res) { unsigned zz = (pos < (1 << (kNumLogBits + 6))) ? 6 : 6 + kNumLogBits - 1;   res = p->g_FastPos[pos >> zz] + (zz * 2); }

#define GetPosSlot1(pos) p->g_FastPos[pos]
#define GetPosSlot2(pos, res) { BSR2_RET(pos, res); }
#define GetPosSlot(pos, res) { if (pos < kNumFullDistances) res = p->g_FastPos[pos & (kNumFullDistances - 1)]; else BSR2_RET(pos, res); }


#define LZMA_NUM_REPS 4

typedef UInt16 CState;
typedef UInt16 CExtra;

struct COptimal
{
  Uint price;
  CState state, extra;
  Uint len, dist, reps[LZMA_NUM_REPS];
};

#define kNumOpts (1 << 11)
#define kPackReserve (kNumOpts * 8)

#define kNumLenToPosStates 4
#define kNumPosSlotBits 6

#define kDicLogSizeMax 32
#define kDistTableSizeMax (kDicLogSizeMax * 2)

#define kNumAlignBits 4
#define kAlignTableSize (1 << kNumAlignBits)
#define kAlignMask (kAlignTableSize - 1)

#define kStartPosModelIndex 4
#define kEndPosModelIndex 14
#define kNumFullDistances (1 << (kEndPosModelIndex >> 1))


#define LZMA_PB_MAX 4
#define LZMA_LC_MAX 8
#define LZMA_LP_MAX 4

#define LZMA_NUM_PB_STATES_MAX (1 << LZMA_PB_MAX)

#define kLenNumLowBits 3
#define kLenNumLowSymbols (1 << kLenNumLowBits)
#define kLenNumHighBits 8
#define kLenNumHighSymbols (1 << kLenNumHighBits)
#define kLenNumSymbolsTotal (kLenNumLowSymbols * 2 + kLenNumHighSymbols)

#define LZMA_MATCH_LEN_MIN 2
#define LZMA_MATCH_LEN_MAX (LZMA_MATCH_LEN_MIN + kLenNumSymbolsTotal - 1)

#define kNumStates 12

struct CLenEnc
{
  CLzmaProb low[LZMA_NUM_PB_STATES_MAX << (kLenNumLowBits + 1)], high[kLenNumHighSymbols];
};

struct CLenPriceEnc
{
  unsigned tableSize;
  Uint prices[LZMA_NUM_PB_STATES_MAX][kLenNumSymbolsTotal];
};

#define GET_PRICE_LEN(p, posState, len)     ((p)->prices[posState][(size_t)(len) - LZMA_MATCH_LEN_MIN])



struct CRangeEnc
{
  Uint range;
  unsigned cache;
  uint64_t low, cacheSize;
  Byte *buf, *bufLim, *bufBase;
  ISeqOutStream* outStream;
  uint64_t processed;
  int res;
};


struct CSaveState
{
  CLzmaProb *litProbs;

  unsigned state;
  Uint reps[LZMA_NUM_REPS];

  CLzmaProb posAlignEncoder[1 << kNumAlignBits], isRep[kNumStates], isRepG0[kNumStates], isRepG1[kNumStates], isRepG2[kNumStates], isMatch[kNumStates][LZMA_NUM_PB_STATES_MAX], isRep0Long[kNumStates][LZMA_NUM_PB_STATES_MAX], posSlotEncoder[kNumLenToPosStates][1 << kNumPosSlotBits], posEncoders[kNumFullDistances];
  CLenEnc lenProbs, repLenProbs;
};


struct CLzmaEnc
{
  void *matchFinderObj;
  IMatchFinder2 matchFinder;

  unsigned optCur, optEnd, longestMatchLen, numPairs;
  Uint numAvail;

  unsigned state, numFastBytes, additionalOffset;
  Uint reps[LZMA_NUM_REPS];
  unsigned lpMask, pbMask;
  CLzmaProb *litProbs;
  CRangeEnc rc;

  Uint backRes;

  unsigned lc, lp, pb, lclp;

  int fastMode, writeEndMark, finished, needInit;

  uint64_t nowPos64;
  
  unsigned matchPriceCount;
  
  int repLenEncCounter;

  unsigned distTableSize;

  Uint dictSize;
  int result;

  CMatchFinder matchFinderBase;
  
  Uint ProbPrices[kBitModelTotal >> kNumMoveReducingBits], matches[LZMA_MATCH_LEN_MAX * 2 + 2];

  Uint alignPrices[kAlignTableSize], posSlotPrices[kNumLenToPosStates][kDistTableSizeMax], distancesPrices[kNumLenToPosStates][kNumFullDistances];

  CLzmaProb posAlignEncoder[1 << kNumAlignBits], isRep[kNumStates], isRepG0[kNumStates], isRepG1[kNumStates], isRepG2[kNumStates], isMatch[kNumStates][LZMA_NUM_PB_STATES_MAX], isRep0Long[kNumStates][LZMA_NUM_PB_STATES_MAX], posSlotEncoder[kNumLenToPosStates][1 << kNumPosSlotBits], posEncoders[kNumFullDistances];
  
  CLenEnc lenProbs, repLenProbs;

  Byte g_FastPos[1 << kNumLogBits];

  CLenPriceEnc lenEnc, repLenEnc;

  COptimal opt[kNumOpts];

  CSaveState saveState;
};


#define MFB (p->matchFinderBase)

int LzmaEnc_SetProps(CLzmaEnc* p, const CLzmaEncProps *props2)
{
  
  CLzmaEncProps props = *props2;
  LzmaEncProps_Normalize(&props);

  if (props.lc > LZMA_LC_MAX
      || props.lp > LZMA_LP_MAX
      || props.pb > LZMA_PB_MAX)
    return SZ_ERROR_PARAM;


  if (props.dictSize > kLzmaMaxHistorySize)
    props.dictSize = kLzmaMaxHistorySize;

  {
    const uint64_t dict64 = props.dictSize;
    if (dict64 > ((uint64_t)1 << kDicLogSizeMaxCompress))
      return SZ_ERROR_PARAM;
  }

  p->dictSize = props.dictSize;
  {
    unsigned fb = (unsigned)props.fb;
    if (fb < 5)
      fb = 5;
    if (fb > LZMA_MATCH_LEN_MAX)
      fb = LZMA_MATCH_LEN_MAX;
    p->numFastBytes = fb;
  }
  p->lc = (unsigned)props.lc;
  p->lp = (unsigned)props.lp;
  p->pb = (unsigned)props.pb;
  p->fastMode = (props.algo == 0);
  
  MFB.btMode = (Byte)(props.btMode ? 1 : 0);
  
  {
    unsigned numHashBytes = 4;
    if (props.btMode)
    {
           if (props.numHashBytes <  2) numHashBytes = 2;
      else if (props.numHashBytes <  4) numHashBytes = (unsigned)props.numHashBytes;
    }
    if (props.numHashBytes >= 5) numHashBytes = 5;

    MFB.numHashBytes = numHashBytes;
    
    MFB.numHashOutBits = (Byte)props.numHashOutBits;
  }

  MFB.cutValue = props.mc;

  p->writeEndMark = (int)props.writeEndMark;

  return SZ_OK;
}


void LzmaEnc_SetDataSize(CLzmaEnc* p, uint64_t expectedDataSiize)
{ 
  MFB.expectedDataSize = expectedDataSiize;
}

#define kState_Start 0
#define kState_LitAfterMatch 4
#define kState_LitAfterRep   5
#define kState_MatchAfterLit 7
#define kState_RepAfterLit   8

static const Byte kLiteralNextStates[kNumStates] = {0, 0, 0, 0, 1, 2, 3, 4,  5,  6,   4, 5};
static const Byte kMatchNextStates[kNumStates]   = {7, 7, 7, 7, 7, 7, 7, 10, 10, 10, 10, 10};
static const Byte kRepNextStates[kNumStates]     = {8, 8, 8, 8, 8, 8, 8, 11, 11, 11, 11, 11};
static const Byte kShortRepNextStates[kNumStates]= {9, 9, 9, 9, 9, 9, 9, 11, 11, 11, 11, 11};

#define IsLitState(s) ((s) < 7)
#define GetLenToPosState2(len) (((len) < kNumLenToPosStates - 1) ? (len) : kNumLenToPosStates - 1)
#define GetLenToPosState(len) (((len) < kNumLenToPosStates + 1) ? (len) - 2 : kNumLenToPosStates - 1)

#define kInfinityPrice (1 << 30)

static void RangeEnc_Construct(CRangeEnc *p)
{
  p->outStream = 0;
  p->bufBase = 0;
}

#define RangeEnc_GetProcessed_sizet(p) ((size_t)(p)->processed + (size_t)((p)->buf - (p)->bufBase) + (size_t)(p)->cacheSize)

#define RC_BUF_SIZE (1 << 16)

static int RangeEnc_Alloc(CRangeEnc *p)
{
  if (!p->bufBase)
  {
    p->bufBase = (Byte *)SzAlloc(RC_BUF_SIZE);
    if (!p->bufBase)
      return 0;
    p->bufLim = p->bufBase + RC_BUF_SIZE;
  }
  return 1;
}

static void RangeEnc_Free(CRangeEnc *p)
{
  free(p->bufBase);
  p->bufBase = 0;
}

static void RangeEnc_Init(CRangeEnc *p)
{
  p->range = 0xFFFFFFFF;
  p->cache = 0;
  p->low = 0;
  p->cacheSize = 0;

  p->buf = p->bufBase;

  p->processed = 0;
  p->res = SZ_OK;
}

static void RangeEnc_FlushStream(CRangeEnc *p)
{
  const size_t num = (size_t)(p->buf - p->bufBase);
  if (p->res == SZ_OK)
  {
    if (num != p->outStream->Write(p->outStream, p->bufBase, num))
      p->res = SZ_ERROR_WRITE;
  }
  p->processed += num;
  p->buf = p->bufBase;
}

static void  RangeEnc_ShiftLow(CRangeEnc *p)
{
  Uint low = (Uint)p->low;
  unsigned high = (unsigned)(p->low >> 32);
  p->low = (Uint)(low << 8);
  if (low < (Uint)0xFF000000 || high != 0)
  {
    {
      Byte *buf = p->buf;
      *buf++ = (Byte)(p->cache + high);
      p->cache = (unsigned)(low >> 24);
      p->buf = buf;
      if (buf == p->bufLim)
        RangeEnc_FlushStream(p);
      if (p->cacheSize == 0)
        return;
    }
    high += 0xFF;
    for (;;)
    {
      Byte *buf = p->buf;
      *buf++ = (Byte)(high);
      p->buf = buf;
      if (buf == p->bufLim)
        RangeEnc_FlushStream(p);
      if (--p->cacheSize == 0)
        return;
    }
  }
  p->cacheSize++;
}

static void RangeEnc_FlushData(CRangeEnc *p)
{
  int i;
  for (i = 0; i < 5; i++)
    RangeEnc_ShiftLow(p);
}

#define RC_NORM(p) if (range < kTopValue) { range <<= 8; RangeEnc_ShiftLow(p); }

#define RC_BIT_PRE(p, prob)   ttt = *(prob);   newBound = (range >> kNumBitModelTotalBits) * ttt;



#define RC_BIT(p, prob, bit) {   Uint mask;   RC_BIT_PRE(p, prob)   mask = 0 - (Uint)bit;   range &= mask;   mask &= newBound;   range -= mask;   (p)->low += mask;   mask = (Uint)bit - 1;   range += newBound & mask;   mask &= (kBitModelTotal - ((1 << kNumMoveBits) - 1));   mask += ((1 << kNumMoveBits) - 1);   ttt += (Uint)((int)(mask - ttt) >> kNumMoveBits);   *(prob) = (CLzmaProb)ttt;   RC_NORM(p)   }




#define RC_BIT_0_BASE(p, prob)   range = newBound; *(prob) = (CLzmaProb)(ttt + ((kBitModelTotal - ttt) >> kNumMoveBits));

#define RC_BIT_1_BASE(p, prob)   range -= newBound; (p)->low += newBound; *(prob) = (CLzmaProb)(ttt - (ttt >> kNumMoveBits)); 
#define RC_BIT_0(p, prob)   RC_BIT_0_BASE(p, prob)   RC_NORM(p)

#define RC_BIT_1(p, prob)   RC_BIT_1_BASE(p, prob)   RC_NORM(p)

static void RangeEnc_EncodeBit_0(CRangeEnc *p, CLzmaProb *prob)
{
  Uint range, ttt, newBound;
  range = p->range;
  RC_BIT_PRE(p, prob)
  RC_BIT_0(p, prob)
  p->range = range;
}

static void LitEnc_Encode(CRangeEnc *p, CLzmaProb *probs, Uint sym)
{
  Uint range = p->range;
  sym |= 0x100;
  do
  {
    Uint ttt, newBound;
    
    CLzmaProb *prob = probs + (sym >> 8);
    Uint bit = (sym >> 7) & 1;
    sym <<= 1;
    RC_BIT(p, prob, bit)
  }
  while (sym < 0x10000);
  p->range = range;
}

static void LitEnc_EncodeMatched(CRangeEnc *p, CLzmaProb *probs, Uint sym, Uint matchByte)
{
  Uint range = p->range;
  Uint offs = 0x100;
  sym |= 0x100;
  do
  {
    Uint ttt, newBound;
    CLzmaProb *prob;
    Uint bit;
    matchByte <<= 1;
    
    prob = probs + (offs + (matchByte & offs) + (sym >> 8));
    bit = (sym >> 7) & 1;
    sym <<= 1;
    offs &= ~(matchByte ^ sym);
    RC_BIT(p, prob, bit)
  }
  while (sym < 0x10000);
  p->range = range;
}



static void LzmaEnc_InitPriceTables(Uint *ProbPrices)
{
  Uint i;
  for (i = 0; i < (kBitModelTotal >> kNumMoveReducingBits); i++)
  {
    const unsigned kCyclesBits = kNumBitPriceShiftBits;
    Uint w = (i << kNumMoveReducingBits) + (1 << (kNumMoveReducingBits - 1));
    unsigned bitCount = 0;
    unsigned j;
    for (j = 0; j < kCyclesBits; j++)
    {
      w = w * w;
      bitCount <<= 1;
      while (w >= ((Uint)1 << 16))
      {
        w >>= 1;
        bitCount++;
      }
    }
    ProbPrices[i] = (Uint)(((unsigned)kNumBitModelTotalBits << kCyclesBits) - 15 - bitCount);
    
  }
}


#define GET_PRICE(prob, bit)   p->ProbPrices[((prob) ^ (unsigned)(((-(int)(bit))) & (kBitModelTotal - 1))) >> kNumMoveReducingBits]

#define GET_PRICEa(prob, bit)      ProbPrices[((prob) ^ (unsigned)((-((int)(bit))) & (kBitModelTotal - 1))) >> kNumMoveReducingBits]

#define GET_PRICE_0(prob) p->ProbPrices[(prob) >> kNumMoveReducingBits]
#define GET_PRICE_1(prob) p->ProbPrices[((prob) ^ (kBitModelTotal - 1)) >> kNumMoveReducingBits]

#define GET_PRICEa_0(prob) ProbPrices[(prob) >> kNumMoveReducingBits]
#define GET_PRICEa_1(prob) ProbPrices[((prob) ^ (kBitModelTotal - 1)) >> kNumMoveReducingBits]


static Uint LitEnc_GetPrice(const CLzmaProb *probs, Uint sym, const Uint *ProbPrices)
{
  Uint price = 0;
  sym |= 0x100;
  do
  {
    unsigned bit = sym & 1;
    sym >>= 1;
    price += GET_PRICEa(probs[sym], bit);
  }
  while (sym >= 2);
  return price;
}


static Uint LitEnc_Matched_GetPrice(const CLzmaProb *probs, Uint sym, Uint matchByte, const Uint *ProbPrices)
{
  Uint price = 0;
  Uint offs = 0x100;
  sym |= 0x100;
  do
  {
    matchByte <<= 1;
    price += GET_PRICEa(probs[offs + (matchByte & offs) + (sym >> 8)], (sym >> 7) & 1);
    sym <<= 1;
    offs &= ~(matchByte ^ sym);
  }
  while (sym < 0x10000);
  return price;
}


static void RcTree_ReverseEncode(CRangeEnc *rc, CLzmaProb *probs, unsigned numBits, unsigned sym)
{
  Uint range = rc->range;
  unsigned m = 1;
  do
  {
    Uint ttt, newBound;
    unsigned bit = sym & 1;
    
    sym >>= 1;
    RC_BIT(rc, probs + m, bit)
    m = (m << 1) | bit;
  }
  while (--numBits);
  rc->range = range;
}



static void LenEnc_Init(CLenEnc *p)
{
  unsigned i;
  for (i = 0; i < (LZMA_NUM_PB_STATES_MAX << (kLenNumLowBits + 1)); i++)
    p->low[i] = kProbInitValue;
  for (i = 0; i < kLenNumHighSymbols; i++)
    p->high[i] = kProbInitValue;
}

static void LenEnc_Encode(CLenEnc *p, CRangeEnc *rc, unsigned sym, unsigned posState)
{
  Uint range, ttt, newBound;
  CLzmaProb *probs = p->low;
  range = rc->range;
  RC_BIT_PRE(rc, probs)
  if (sym >= kLenNumLowSymbols)
  {
    RC_BIT_1(rc, probs)
    probs += kLenNumLowSymbols;
    RC_BIT_PRE(rc, probs)
    if (sym >= kLenNumLowSymbols * 2)
    {
      RC_BIT_1(rc, probs)
      rc->range = range;
      
      LitEnc_Encode(rc, p->high, sym - kLenNumLowSymbols * 2);
      return;
    }
    sym -= kLenNumLowSymbols;
  }

  
  {
    unsigned m;
    unsigned bit;
    RC_BIT_0(rc, probs)
    probs += (posState << (1 + kLenNumLowBits));
    bit = (sym >> 2)    ; RC_BIT(rc, probs + 1, bit)  m = (1 << 1) + bit;
    bit = (sym >> 1) & 1; RC_BIT(rc, probs + m, bit)  m = (m << 1) + bit;
    bit =  sym       & 1; RC_BIT(rc, probs + m, bit)
    rc->range = range;
  }
}

static void SetPrices_3(const CLzmaProb *probs, Uint startPrice, Uint *prices, const Uint *ProbPrices)
{
  unsigned i;
  for (i = 0; i < 8; i += 2)
  {
    Uint price = startPrice;
    Uint prob;
    price += GET_PRICEa(probs[1           ], (i >> 2));
    price += GET_PRICEa(probs[2 + (i >> 2)], (i >> 1) & 1);
    prob = probs[4 + (i >> 1)];
    prices[i    ] = price + GET_PRICEa_0(prob);
    prices[i + 1] = price + GET_PRICEa_1(prob);
  }
}


static void  LenPriceEnc_UpdateTables(
    CLenPriceEnc *p,
    unsigned numPosStates,
    const CLenEnc *enc,
    const Uint *ProbPrices)
{
  Uint b;
 
  {
    unsigned prob = enc->low[0];
    Uint a, c;
    unsigned posState;
    b = GET_PRICEa_1(prob);
    a = GET_PRICEa_0(prob);
    c = b + GET_PRICEa_0(enc->low[kLenNumLowSymbols]);
    for (posState = 0; posState < numPosStates; posState++)
    {
      Uint *prices = p->prices[posState];
      const CLzmaProb *probs = enc->low + (posState << (1 + kLenNumLowBits));
      SetPrices_3(probs, a, prices, ProbPrices);
      SetPrices_3(probs + kLenNumLowSymbols, c, prices + kLenNumLowSymbols, ProbPrices);
    }
  }

  
 
  
  

  {
    unsigned i = p->tableSize;
    
    if (i > kLenNumLowSymbols * 2)
    {
      const CLzmaProb *probs = enc->high;
      Uint *prices = p->prices[0] + kLenNumLowSymbols * 2;
      i -= kLenNumLowSymbols * 2 - 1;
      i >>= 1;
      b += GET_PRICEa_1(enc->low[kLenNumLowSymbols]);
      do
      {
        
        
        unsigned sym = --i + (1 << (kLenNumHighBits - 1));
        Uint price = b;
        do
        {
          const unsigned bit = sym & 1;
          sym >>= 1;
          price += GET_PRICEa(probs[sym], bit);
        }
        while (sym >= 2);

        {
          const unsigned prob = probs[(size_t)i + (1 << (kLenNumHighBits - 1))];
          prices[(size_t)i * 2    ] = price + GET_PRICEa_0(prob);
          prices[(size_t)i * 2 + 1] = price + GET_PRICEa_1(prob);
        }
      }
      while (i);

      {
        unsigned posState;
        const size_t num = (p->tableSize - kLenNumLowSymbols * 2) * sizeof(p->prices[0][0]);
        for (posState = 1; posState < numPosStates; posState++)
          memcpy(p->prices[posState] + kLenNumLowSymbols * 2, p->prices[0] + kLenNumLowSymbols * 2, num);
      }
    }
  }
}
  
#define MOVE_POS(p, num) {     p->additionalOffset += (num);     p->matchFinder.Skip(p->matchFinderObj, (Uint)(num)); }


static unsigned ReadMatchDistances(CLzmaEnc *p, unsigned *numPairsRes)
{
  unsigned numPairs;
  
  p->additionalOffset++;
  p->numAvail = p->matchFinder.GetNumAvailableBytes(p->matchFinderObj);
  {
    const Uint *d = p->matchFinder.GetMatches(p->matchFinderObj, p->matches);
    
    numPairs = (unsigned)(d - p->matches);
  }
  *numPairsRes = numPairs;

  
  if (numPairs == 0)
    return 0;
  {
    const unsigned len = p->matches[(size_t)numPairs - 2];
    if (len != p->numFastBytes)
      return len;
    {
      Uint numAvail = p->numAvail;
      if (numAvail > LZMA_MATCH_LEN_MAX)
        numAvail = LZMA_MATCH_LEN_MAX;
      {
        const Byte *p1 = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - 1;
        const Byte *p2 = p1 + len;
        const ptrdiff_t dif = (ptrdiff_t)-1 - (ptrdiff_t)p->matches[(size_t)numPairs - 1];
        const Byte *lim = p1 + numAvail;
        for (; p2 != lim && *p2 == p2[dif]; p2++)
        {}
        return (unsigned)(p2 - p1);
      }
    }
  }
}

#define MARK_LIT ((Uint)(int)-1)

#define MakeAs_Lit(p)       { (p)->dist = MARK_LIT; (p)->extra = 0; }
#define MakeAs_ShortRep(p)  { (p)->dist = 0; (p)->extra = 0; }
#define IsShortRep(p)       ((p)->dist == 0)


#define GetPrice_ShortRep(p, state, posState)   ( GET_PRICE_0(p->isRepG0[state]) + GET_PRICE_0(p->isRep0Long[state][posState]))

#define GetPrice_Rep_0(p, state, posState) (     GET_PRICE_1(p->isMatch[state][posState])   + GET_PRICE_1(p->isRep0Long[state][posState]))   + GET_PRICE_1(p->isRep[state])   + GET_PRICE_0(p->isRepG0[state])
  

static Uint GetPrice_PureRep(const CLzmaEnc *p, unsigned repIndex, size_t state, size_t posState)
{
  Uint price;
  Uint prob = p->isRepG0[state];
  if (repIndex == 0)
  {
    price = GET_PRICE_0(prob);
    price += GET_PRICE_1(p->isRep0Long[state][posState]);
  }
  else
  {
    price = GET_PRICE_1(prob);
    prob = p->isRepG1[state];
    if (repIndex == 1)
      price += GET_PRICE_0(prob);
    else
    {
      price += GET_PRICE_1(prob);
      price += GET_PRICE(p->isRepG2[state], repIndex - 2);
    }
  }
  return price;
}


static unsigned Backward(CLzmaEnc *p, unsigned cur)
{
  unsigned wr = cur + 1;
  p->optEnd = wr;

  for (;;)
  {
    Uint dist = p->opt[cur].dist;
    unsigned len = (unsigned)p->opt[cur].len;
    unsigned extra = (unsigned)p->opt[cur].extra;
    cur -= len;

    if (extra)
    {
      wr--;
      p->opt[wr].len = (Uint)len;
      cur -= extra;
      len = extra;
      if (extra == 1)
      {
        p->opt[wr].dist = dist;
        dist = MARK_LIT;
      }
      else
      {
        p->opt[wr].dist = 0;
        len--;
        wr--;
        p->opt[wr].dist = MARK_LIT;
        p->opt[wr].len = 1;
      }
    }

    if (cur == 0)
    {
      p->backRes = dist;
      p->optCur = wr;
      return len;
    }
    
    wr--;
    p->opt[wr].dist = dist;
    p->opt[wr].len = (Uint)len;
  }
}



#define LIT_PROBS(pos, prevByte)   (p->litProbs + (Uint)3 * (((((pos) << 8) + (prevByte)) & p->lpMask) << p->lc))


static unsigned GetOptimum(CLzmaEnc *p, Uint position)
{
  unsigned last, cur;
  Uint reps[LZMA_NUM_REPS];
  unsigned repLens[LZMA_NUM_REPS];
  Uint *matches;

  {
    Uint numAvail;
    unsigned numPairs, mainLen, repMaxIndex, i, posState;
    Uint matchPrice, repMatchPrice;
    const Byte *data;
    Byte curByte, matchByte;
    
    p->optCur = p->optEnd = 0;
    
    if (p->additionalOffset == 0)
      mainLen = ReadMatchDistances(p, &numPairs);
    else
    {
      mainLen = p->longestMatchLen;
      numPairs = p->numPairs;
    }
    
    numAvail = p->numAvail;
    if (numAvail < 2)
    {
      p->backRes = MARK_LIT;
      return 1;
    }
    if (numAvail > LZMA_MATCH_LEN_MAX)
      numAvail = LZMA_MATCH_LEN_MAX;
    
    data = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - 1;
    repMaxIndex = 0;
    
    for (i = 0; i < LZMA_NUM_REPS; i++)
    {
      unsigned len;
      const Byte *data2;
      reps[i] = p->reps[i];
      data2 = data - reps[i];
      if (data[0] != data2[0] || data[1] != data2[1])
      {
        repLens[i] = 0;
        continue;
      }
      for (len = 2; len < numAvail && data[len] == data2[len]; len++)
      {}
      repLens[i] = len;
      if (len > repLens[repMaxIndex])
        repMaxIndex = i;
      if (len == LZMA_MATCH_LEN_MAX) 
        break;
    }
    
    if (repLens[repMaxIndex] >= p->numFastBytes)
    {
      unsigned len;
      p->backRes = (Uint)repMaxIndex;
      len = repLens[repMaxIndex];
      MOVE_POS(p, len - 1)
      return len;
    }
    
    matches = p->matches;
    #define MATCHES  matches
    
    
    if (mainLen >= p->numFastBytes)
    {
      p->backRes = MATCHES[(size_t)numPairs - 1] + LZMA_NUM_REPS;
      MOVE_POS(p, mainLen - 1)
      return mainLen;
    }
    
    curByte = *data;
    matchByte = *(data - reps[0]);

    last = repLens[repMaxIndex];
    if (last <= mainLen)
      last = mainLen;
    
    if (last < 2 && curByte != matchByte)
    {
      p->backRes = MARK_LIT;
      return 1;
    }
    
    p->opt[0].state = (CState)p->state;
    
    posState = (position & p->pbMask);
    
    {
      const CLzmaProb *probs = LIT_PROBS(position, *(data - 1));
      p->opt[1].price = GET_PRICE_0(p->isMatch[p->state][posState]) +
        (!IsLitState(p->state) ?
          LitEnc_Matched_GetPrice(probs, curByte, matchByte, p->ProbPrices) :
          LitEnc_GetPrice(probs, curByte, p->ProbPrices));
    }

    MakeAs_Lit(&p->opt[1])
    
    matchPrice = GET_PRICE_1(p->isMatch[p->state][posState]);
    repMatchPrice = matchPrice + GET_PRICE_1(p->isRep[p->state]);
    
    
    if (matchByte == curByte && repLens[0] == 0)
    {
      Uint shortRepPrice = repMatchPrice + GetPrice_ShortRep(p, p->state, posState);
      if (shortRepPrice < p->opt[1].price)
      {
        p->opt[1].price = shortRepPrice;
        MakeAs_ShortRep(&p->opt[1])
      }
      if (last < 2)
      {
        p->backRes = p->opt[1].dist;
        return 1;
      }
    }
   
    p->opt[1].len = 1;
    
    p->opt[0].reps[0] = reps[0];
    p->opt[0].reps[1] = reps[1];
    p->opt[0].reps[2] = reps[2];
    p->opt[0].reps[3] = reps[3];
    
    
    
    for (i = 0; i < LZMA_NUM_REPS; i++)
    {
      unsigned repLen = repLens[i];
      Uint price;
      if (repLen < 2)
        continue;
      price = repMatchPrice + GetPrice_PureRep(p, i, p->state, posState);
      do
      {
        Uint price2 = price + GET_PRICE_LEN(&p->repLenEnc, posState, repLen);
        COptimal *opt = &p->opt[repLen];
        if (price2 < opt->price)
        {
          opt->price = price2;
          opt->len = (Uint)repLen;
          opt->dist = (Uint)i;
          opt->extra = 0;
        }
      }
      while (--repLen >= 2);
    }
    
    
    
    {
      unsigned len = repLens[0] + 1;
      if (len <= mainLen)
      {
        unsigned offs = 0;
        Uint normalMatchPrice = matchPrice + GET_PRICE_0(p->isRep[p->state]);

        if (len < 2)
          len = 2;
        else
          while (len > MATCHES[offs])
            offs += 2;
    
        for (; ; len++)
        {
          COptimal *opt;
          Uint dist = MATCHES[(size_t)offs + 1];
          Uint price = normalMatchPrice + GET_PRICE_LEN(&p->lenEnc, posState, len);
          unsigned lenToPosState = GetLenToPosState(len);
       
          if (dist < kNumFullDistances)
            price += p->distancesPrices[lenToPosState][dist & (kNumFullDistances - 1)];
          else
          {
            unsigned slot;
            GetPosSlot2(dist, slot)
            price += p->alignPrices[dist & kAlignMask];
            price += p->posSlotPrices[lenToPosState][slot];
          }
          
          opt = &p->opt[len];
          
          if (price < opt->price)
          {
            opt->price = price;
            opt->len = (Uint)len;
            opt->dist = dist + LZMA_NUM_REPS;
            opt->extra = 0;
          }
          
          if (len == MATCHES[offs])
          {
            offs += 2;
            if (offs == numPairs)
              break;
          }
        }
      }
    }
    

    cur = 0;
  }


  
  

  for (;;)
  {
    unsigned numAvail;
    Uint numAvailFull;
    unsigned newLen, numPairs, prev, state, posState, startLen;
    Uint litPrice, matchPrice, repMatchPrice;
    int nextIsLit;
    Byte curByte, matchByte;
    const Byte *data;
    COptimal *curOpt, *nextOpt;

    if (++cur == last)
      break;
    
    
    if (cur >= kNumOpts - 64)
    {
      unsigned j, best;
      Uint price = p->opt[cur].price;
      best = cur;
      for (j = cur + 1; j <= last; j++)
      {
        Uint price2 = p->opt[j].price;
        if (price >= price2)
        {
          price = price2;
          best = j;
        }
      }
      {
        unsigned delta = best - cur;
        if (delta != 0)
        {
          MOVE_POS(p, delta)
        }
      }
      cur = best;
      break;
    }

    newLen = ReadMatchDistances(p, &numPairs);
    
    if (newLen >= p->numFastBytes)
    {
      p->numPairs = numPairs;
      p->longestMatchLen = newLen;
      break;
    }
    
    curOpt = &p->opt[cur];

    position++;

    
    

    prev = cur - curOpt->len;

    if (curOpt->len == 1)
    {
      state = (unsigned)p->opt[prev].state;
      if (IsShortRep(curOpt))
        state = kShortRepNextStates[state];
      else
        state = kLiteralNextStates[state];
    }
    else
    {
      const COptimal *prevOpt;
      Uint b0;
      Uint dist = curOpt->dist;

      if (curOpt->extra)
      {
        prev -= (unsigned)curOpt->extra;
        state = kState_RepAfterLit;
        if (curOpt->extra == 1)
          state = (dist < LZMA_NUM_REPS ? kState_RepAfterLit : kState_MatchAfterLit);
      }
      else
      {
        state = (unsigned)p->opt[prev].state;
        if (dist < LZMA_NUM_REPS)
          state = kRepNextStates[state];
        else
          state = kMatchNextStates[state];
      }

      prevOpt = &p->opt[prev];
      b0 = prevOpt->reps[0];

      if (dist < LZMA_NUM_REPS)
      {
        if (dist == 0)
        {
          reps[0] = b0;
          reps[1] = prevOpt->reps[1];
          reps[2] = prevOpt->reps[2];
          reps[3] = prevOpt->reps[3];
        }
        else
        {
          reps[1] = b0;
          b0 = prevOpt->reps[1];
          if (dist == 1)
          {
            reps[0] = b0;
            reps[2] = prevOpt->reps[2];
            reps[3] = prevOpt->reps[3];
          }
          else
          {
            reps[2] = b0;
            reps[0] = prevOpt->reps[dist];
            reps[3] = prevOpt->reps[dist ^ 1];
          }
        }
      }
      else
      {
        reps[0] = (dist - LZMA_NUM_REPS + 1);
        reps[1] = b0;
        reps[2] = prevOpt->reps[1];
        reps[3] = prevOpt->reps[2];
      }
    }
    
    curOpt->state = (CState)state;
    curOpt->reps[0] = reps[0];
    curOpt->reps[1] = reps[1];
    curOpt->reps[2] = reps[2];
    curOpt->reps[3] = reps[3];

    data = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - 1;
    curByte = *data;
    matchByte = *(data - reps[0]);

    posState = (position & p->pbMask);

    

    {
      Uint curPrice = curOpt->price;
      unsigned prob = p->isMatch[state][posState];
      matchPrice = curPrice + GET_PRICE_1(prob);
      litPrice = curPrice + GET_PRICE_0(prob);
    }

    nextOpt = &p->opt[(size_t)cur + 1];
    nextIsLit = 0;

    
    
    if ((nextOpt->price < kInfinityPrice
        
        && matchByte == curByte)
        || litPrice > nextOpt->price
        )
      litPrice = 0;
    else
    {
      const CLzmaProb *probs = LIT_PROBS(position, *(data - 1));
      litPrice += (!IsLitState(state) ?
          LitEnc_Matched_GetPrice(probs, curByte, matchByte, p->ProbPrices) :
          LitEnc_GetPrice(probs, curByte, p->ProbPrices));
      
      if (litPrice < nextOpt->price)
      {
        nextOpt->price = litPrice;
        nextOpt->len = 1;
        MakeAs_Lit(nextOpt)
        nextIsLit = 1;
      }
    }

    repMatchPrice = matchPrice + GET_PRICE_1(p->isRep[state]);
    
    numAvailFull = p->numAvail;
    {
      unsigned temp = kNumOpts - 1 - cur;
      if (numAvailFull > temp)
        numAvailFull = (Uint)temp;
    }

    
    
    if (IsLitState(state)) 
    if (matchByte == curByte)
    if (repMatchPrice < nextOpt->price) 
    
    if (
        
        nextOpt->len < 2   
        || (nextOpt->dist != 0
            
            )
        )
    {
      Uint shortRepPrice = repMatchPrice + GetPrice_ShortRep(p, state, posState);
      
      if (shortRepPrice < nextOpt->price)  
      {
        nextOpt->price = shortRepPrice;
        nextOpt->len = 1;
        MakeAs_ShortRep(nextOpt)
        nextIsLit = 0;
      }
    }
    
    if (numAvailFull < 2)
      continue;
    numAvail = (numAvailFull <= p->numFastBytes ? numAvailFull : p->numFastBytes);

    

    

    if (!nextIsLit
        && litPrice != 0 
        && matchByte != curByte
        && numAvailFull > 2)
    {
      const Byte *data2 = data - reps[0];
      if (data[1] == data2[1] && data[2] == data2[2])
      {
        unsigned len;
        unsigned limit = p->numFastBytes + 1;
        if (limit > numAvailFull)
          limit = numAvailFull;
        for (len = 3; len < limit && data[len] == data2[len]; len++)
        {}
        
        {
          unsigned state2 = kLiteralNextStates[state];
          unsigned posState2 = (position + 1) & p->pbMask;
          Uint price = litPrice + GetPrice_Rep_0(p, state2, posState2);
          {
            unsigned offset = cur + len;

            if (last < offset)
              last = offset;
          
            
            {
              Uint price2;
              COptimal *opt;
              len--;
              
              price2 = price + GET_PRICE_LEN(&p->repLenEnc, posState2, len);

              opt = &p->opt[offset];
              
              if (price2 < opt->price)
              {
                opt->price = price2;
                opt->len = (Uint)len;
                opt->dist = 0;
                opt->extra = 1;
              }
            }
            
          }
        }
      }
    }
    
    startLen = 2; 

    {
      
      unsigned repIndex = 0; 
      
      for (; repIndex < LZMA_NUM_REPS; repIndex++)
      {
        unsigned len;
        Uint price;
        const Byte *data2 = data - reps[repIndex];
        if (data[0] != data2[0] || data[1] != data2[1])
          continue;
        
        for (len = 2; len < numAvail && data[len] == data2[len]; len++)
        {}
        
        

        {
          unsigned offset = cur + len;
          if (last < offset)
            last = offset;
        }
        {
          unsigned len2 = len;
          price = repMatchPrice + GetPrice_PureRep(p, repIndex, state, posState);
          do
          {
            Uint price2 = price + GET_PRICE_LEN(&p->repLenEnc, posState, len2);
            COptimal *opt = &p->opt[cur + len2];
            if (price2 < opt->price)
            {
              opt->price = price2;
              opt->len = (Uint)len2;
              opt->dist = (Uint)repIndex;
              opt->extra = 0;
            }
          }
          while (--len2 >= 2);
        }
        
        if (repIndex == 0) startLen = len + 1;  
        

        
        {
          
          

          unsigned len2 = len + 1;
          unsigned limit = len2 + p->numFastBytes;
          if (limit > numAvailFull)
            limit = numAvailFull;
          
          len2 += 2;
          if (len2 <= limit)
          if (data[len2 - 2] == data2[len2 - 2])
          if (data[len2 - 1] == data2[len2 - 1])
          {
            unsigned state2 = kRepNextStates[state];
            unsigned posState2 = (position + len) & p->pbMask;
            price += GET_PRICE_LEN(&p->repLenEnc, posState, len)
                + GET_PRICE_0(p->isMatch[state2][posState2])
                + LitEnc_Matched_GetPrice(LIT_PROBS(position + len, data[(size_t)len - 1]),
                    data[len], data2[len], p->ProbPrices);
            
            
            state2 = kState_LitAfterRep;
            posState2 = (posState2 + 1) & p->pbMask;


            price += GetPrice_Rep_0(p, state2, posState2);

          for (; len2 < limit && data[len2] == data2[len2]; len2++)
          {}
          
          len2 -= len;
          
          {
            {
              unsigned offset = cur + len + len2;

              if (last < offset)
                last = offset;
              
              {
                Uint price2;
                COptimal *opt;
                len2--;
                
                price2 = price + GET_PRICE_LEN(&p->repLenEnc, posState2, len2);

                opt = &p->opt[offset];
                
                if (price2 < opt->price)
                {
                  opt->price = price2;
                  opt->len = (Uint)len2;
                  opt->extra = (CExtra)(len + 1);
                  opt->dist = (Uint)repIndex;
                }
              }
              
            }
          }
          }
        }
      }
    }


    
    
    if (newLen > numAvail)
    {
      newLen = numAvail;
      for (numPairs = 0; newLen > MATCHES[numPairs]; numPairs += 2);
      MATCHES[numPairs] = (Uint)newLen;
      numPairs += 2;
    }
    
    

    if (newLen >= startLen)
    {
      Uint normalMatchPrice = matchPrice + GET_PRICE_0(p->isRep[state]);
      Uint dist;
      unsigned offs, posSlot, len;
      
      {
        unsigned offset = cur + newLen;
        if (last < offset)
          last = offset;
      }

      offs = 0;
      while (startLen > MATCHES[offs])
        offs += 2;
      dist = MATCHES[(size_t)offs + 1];
      
      
      GetPosSlot2(dist, posSlot)
      
      for (len =  startLen; ; len++)
      {
        Uint price = normalMatchPrice + GET_PRICE_LEN(&p->lenEnc, posState, len);
        {
          COptimal *opt;
          unsigned lenNorm = len - 2;
          lenNorm = GetLenToPosState2(lenNorm);
          if (dist < kNumFullDistances)
            price += p->distancesPrices[lenNorm][dist & (kNumFullDistances - 1)];
          else
            price += p->posSlotPrices[lenNorm][posSlot] + p->alignPrices[dist & kAlignMask];
          
          opt = &p->opt[cur + len];
          if (price < opt->price)
          {
            opt->price = price;
            opt->len = (Uint)len;
            opt->dist = dist + LZMA_NUM_REPS;
            opt->extra = 0;
          }
        }

        if (len == MATCHES[offs])
        {
          
          

          const Byte *data2 = data - dist - 1;
          unsigned len2 = len + 1;
          unsigned limit = len2 + p->numFastBytes;
          if (limit > numAvailFull)
            limit = numAvailFull;
          
          len2 += 2;
          if (len2 <= limit)
          if (data[len2 - 2] == data2[len2 - 2])
          if (data[len2 - 1] == data2[len2 - 1])
          {
          for (; len2 < limit && data[len2] == data2[len2]; len2++)
          {}
          
          len2 -= len;
          
          
          {
            unsigned state2 = kMatchNextStates[state];
            unsigned posState2 = (position + len) & p->pbMask;
            unsigned offset;
            price += GET_PRICE_0(p->isMatch[state2][posState2]);
            price += LitEnc_Matched_GetPrice(LIT_PROBS(position + len, data[(size_t)len - 1]),
                    data[len], data2[len], p->ProbPrices);

            
            state2 = kState_LitAfterMatch;

            posState2 = (posState2 + 1) & p->pbMask;
            price += GetPrice_Rep_0(p, state2, posState2);

            offset = cur + len + len2;

            if (last < offset)
              last = offset;
            
            {
              Uint price2;
              COptimal *opt;
              len2--;
              
              price2 = price + GET_PRICE_LEN(&p->repLenEnc, posState2, len2);
              opt = &p->opt[offset];
              
              if (price2 < opt->price)
              {
                opt->price = price2;
                opt->len = (Uint)len2;
                opt->extra = (CExtra)(len + 1);
                opt->dist = dist + LZMA_NUM_REPS;
              }
            }
            
          }

          }
        
          offs += 2;
          if (offs == numPairs)
            break;
          dist = MATCHES[(size_t)offs + 1];
          
            GetPosSlot2(dist, posSlot)
        }
      }
    }
  }

  do
    p->opt[last].price = kInfinityPrice;
  while (--last);

  return Backward(p, cur);
}



#define ChangePair(smallDist, bigDist) (((bigDist) >> 7) > (smallDist))



static unsigned GetOptimumFast(CLzmaEnc *p)
{
  Uint numAvail, mainDist;
  unsigned mainLen, numPairs, repIndex, repLen, i;
  const Byte *data;

  if (p->additionalOffset == 0)
    mainLen = ReadMatchDistances(p, &numPairs);
  else
  {
    mainLen = p->longestMatchLen;
    numPairs = p->numPairs;
  }

  numAvail = p->numAvail;
  p->backRes = MARK_LIT;
  if (numAvail < 2)
    return 1;
  
  if (numAvail > LZMA_MATCH_LEN_MAX)
    numAvail = LZMA_MATCH_LEN_MAX;
  data = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - 1;
  repLen = repIndex = 0;
  
  for (i = 0; i < LZMA_NUM_REPS; i++)
  {
    unsigned len;
    const Byte *data2 = data - p->reps[i];
    if (data[0] != data2[0] || data[1] != data2[1])
      continue;
    for (len = 2; len < numAvail && data[len] == data2[len]; len++)
    {}
    if (len >= p->numFastBytes)
    {
      p->backRes = (Uint)i;
      MOVE_POS(p, len - 1)
      return len;
    }
    if (len > repLen)
    {
      repIndex = i;
      repLen = len;
    }
  }

  if (mainLen >= p->numFastBytes)
  {
    p->backRes = p->matches[(size_t)numPairs - 1] + LZMA_NUM_REPS;
    MOVE_POS(p, mainLen - 1)
    return mainLen;
  }

  mainDist = 0; 
  
  if (mainLen >= 2)
  {
    mainDist = p->matches[(size_t)numPairs - 1];
    while (numPairs > 2)
    {
      Uint dist2;
      if (mainLen != p->matches[(size_t)numPairs - 4] + 1)
        break;
      dist2 = p->matches[(size_t)numPairs - 3];
      if (!ChangePair(dist2, mainDist))
        break;
      numPairs -= 2;
      mainLen--;
      mainDist = dist2;
    }
    if (mainLen == 2 && mainDist >= 0x80)
      mainLen = 1;
  }

  if (repLen >= 2)
    if (    repLen + 1 >= mainLen
        || (repLen + 2 >= mainLen && mainDist >= (1 << 9))
        || (repLen + 3 >= mainLen && mainDist >= (1 << 15)))
  {
    p->backRes = (Uint)repIndex;
    MOVE_POS(p, repLen - 1)
    return repLen;
  }
  
  if (mainLen < 2 || numAvail <= 2)
    return 1;

  {
    unsigned len1 = ReadMatchDistances(p, &p->numPairs);
    p->longestMatchLen = len1;
  
    if (len1 >= 2)
    {
      Uint newDist = p->matches[(size_t)p->numPairs - 1];
      if (   (len1 >= mainLen && newDist < mainDist)
          || (len1 == mainLen + 1 && !ChangePair(mainDist, newDist))
          || (len1 >  mainLen + 1)
          || (len1 + 1 >= mainLen && mainLen >= 3 && ChangePair(newDist, mainDist)))
        return 1;
    }
  }
  
  data = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - 1;
  
  for (i = 0; i < LZMA_NUM_REPS; i++)
  {
    unsigned len, limit;
    const Byte *data2 = data - p->reps[i];
    if (data[0] != data2[0] || data[1] != data2[1])
      continue;
    limit = mainLen - 1;
    for (len = 2;; len++)
    {
      if (len >= limit)
        return 1;
      if (data[len] != data2[len])
        break;
    }
  }
  
  p->backRes = mainDist + LZMA_NUM_REPS;
  if (mainLen != 2)
  {
    MOVE_POS(p, mainLen - 2)
  }
  return mainLen;
}




static void WriteEndMarker(CLzmaEnc *p, unsigned posState)
{
  Uint range;
  range = p->rc.range;
  {
    Uint ttt, newBound;
    CLzmaProb *prob = &p->isMatch[p->state][posState];
    RC_BIT_PRE(&p->rc, prob)
    RC_BIT_1(&p->rc, prob)
    prob = &p->isRep[p->state];
    RC_BIT_PRE(&p->rc, prob)
    RC_BIT_0(&p->rc, prob)
  }
  p->state = kMatchNextStates[p->state];
  
  p->rc.range = range;
  LenEnc_Encode(&p->lenProbs, &p->rc, 0, posState);
  range = p->rc.range;

  {
    
    CLzmaProb *probs = p->posSlotEncoder[0];
    unsigned m = 1;
    do
    {
      Uint ttt, newBound;
      RC_BIT_PRE(p, probs + m)
      RC_BIT_1(&p->rc, probs + m)
      m = (m << 1) + 1;
    }
    while (m < (1 << kNumPosSlotBits));
  }
  {
    
    unsigned numBits = 30 - kNumAlignBits;
    do
    {
      range >>= 1;
      p->rc.low += range;
      RC_NORM(&p->rc)
    }
    while (--numBits);
  }
   
  {
    
    CLzmaProb *probs = p->posAlignEncoder;
    unsigned m = 1;
    do
    {
      Uint ttt, newBound;
      RC_BIT_PRE(p, probs + m)
      RC_BIT_1(&p->rc, probs + m)
      m = (m << 1) + 1;
    }
    while (m < kAlignTableSize);
  }
  p->rc.range = range;
}


static int CheckErrors(CLzmaEnc *p)
{
  if (p->result != SZ_OK)
    return p->result;
  if (p->rc.res != SZ_OK)
    p->result = SZ_ERROR_WRITE;

  if (MFB.result != SZ_OK)
    p->result = 8;
  
  if (p->result != SZ_OK)
    p->finished = 1;
  return p->result;
}


static int Flush(CLzmaEnc *p, Uint nowPos)
{
  
  p->finished = 1;
  if (p->writeEndMark)
    WriteEndMarker(p, nowPos & p->pbMask);
  RangeEnc_FlushData(&p->rc);
  RangeEnc_FlushStream(&p->rc);
  return CheckErrors(p);
}


static void FillAlignPrices(CLzmaEnc *p)
{
  unsigned i;
  const Uint *ProbPrices = p->ProbPrices;
  const CLzmaProb *probs = p->posAlignEncoder;
  
  for (i = 0; i < kAlignTableSize / 2; i++)
  {
    Uint price = 0;
    unsigned sym = i;
    unsigned m = 1;
    unsigned bit;
    Uint prob;
    bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[m], bit); m = (m << 1) + bit;
    bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[m], bit); m = (m << 1) + bit;
    bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[m], bit); m = (m << 1) + bit;
    prob = probs[m];
    p->alignPrices[i    ] = price + GET_PRICEa_0(prob);
    p->alignPrices[i + 8] = price + GET_PRICEa_1(prob);
    
  }
}


static void FillDistancesPrices(CLzmaEnc *p)
{
  

  Uint tempPrices[kNumFullDistances];
  unsigned i, lps;

  const Uint *ProbPrices = p->ProbPrices;
  p->matchPriceCount = 0;

  for (i = kStartPosModelIndex / 2; i < kNumFullDistances / 2; i++)
  {
    unsigned posSlot = GetPosSlot1(i);
    unsigned footerBits = (posSlot >> 1) - 1;
    unsigned base = ((2 | (posSlot & 1)) << footerBits);
    const CLzmaProb *probs = p->posEncoders + (size_t)base * 2;
    
    Uint price = 0;
    unsigned m = 1;
    unsigned sym = i;
    unsigned offset = (unsigned)1 << footerBits;
    base += i;
    
    if (footerBits)
    do
    {
      unsigned bit = sym & 1;
      sym >>= 1;
      price += GET_PRICEa(probs[m], bit);
      m = (m << 1) + bit;
    }
    while (--footerBits);

    {
      unsigned prob = probs[m];
      tempPrices[base         ] = price + GET_PRICEa_0(prob);
      tempPrices[base + offset] = price + GET_PRICEa_1(prob);
    }
  }

  for (lps = 0; lps < kNumLenToPosStates; lps++)
  {
    unsigned slot;
    unsigned distTableSize2 = (p->distTableSize + 1) >> 1;
    Uint *posSlotPrices = p->posSlotPrices[lps];
    const CLzmaProb *probs = p->posSlotEncoder[lps];
    
    for (slot = 0; slot < distTableSize2; slot++)
    {
      
      Uint price;
      unsigned bit;
      unsigned sym = slot + (1 << (kNumPosSlotBits - 1));
      unsigned prob;
      bit = sym & 1; sym >>= 1; price  = GET_PRICEa(probs[sym], bit);
      bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[sym], bit);
      bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[sym], bit);
      bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[sym], bit);
      bit = sym & 1; sym >>= 1; price += GET_PRICEa(probs[sym], bit);
      prob = probs[(size_t)slot + (1 << (kNumPosSlotBits - 1))];
      posSlotPrices[(size_t)slot * 2    ] = price + GET_PRICEa_0(prob);
      posSlotPrices[(size_t)slot * 2 + 1] = price + GET_PRICEa_1(prob);
    }
    
    {
      Uint delta = ((Uint)((kEndPosModelIndex / 2 - 1) - kNumAlignBits) << kNumBitPriceShiftBits);
      for (slot = kEndPosModelIndex / 2; slot < distTableSize2; slot++)
      {
        posSlotPrices[(size_t)slot * 2    ] += delta;
        posSlotPrices[(size_t)slot * 2 + 1] += delta;
        delta += ((Uint)1 << kNumBitPriceShiftBits);
      }
    }

    {
      Uint *dp = p->distancesPrices[lps];
      
      dp[0] = posSlotPrices[0];
      dp[1] = posSlotPrices[1];
      dp[2] = posSlotPrices[2];
      dp[3] = posSlotPrices[3];

      for (i = 4; i < kNumFullDistances; i += 2)
      {
        Uint slotPrice = posSlotPrices[GetPosSlot1(i)];
        dp[i    ] = slotPrice + tempPrices[i];
        dp[i + 1] = slotPrice + tempPrices[i + 1];
      }
    }
  }
  
}



static void LzmaEnc_Construct(CLzmaEnc *p)
{
  RangeEnc_Construct(&p->rc);
  MatchFinder_Construct(&MFB);
  
  {
    CLzmaEncProps props;
    LzmaEncProps_Init(&props);
    LzmaEnc_SetProps((CLzmaEnc*)(void *)p, &props);
  }

  LzmaEnc_FastPosInit(p->g_FastPos);

  LzmaEnc_InitPriceTables(p->ProbPrices);
  p->litProbs = 0;
  p->saveState.litProbs = 0;
}

CLzmaEnc* LzmaEnc_Create()
{
  void *p;
  p = SzAlloc(sizeof(CLzmaEnc));
  if (p)
    LzmaEnc_Construct((CLzmaEnc *)p);
  return (CLzmaEnc*)p;
}

static void LzmaEnc_FreeLits(CLzmaEnc *p)
{
  free(p->litProbs);
  free(p->saveState.litProbs);
  p->litProbs = 0;
  p->saveState.litProbs = 0;
}

static void LzmaEnc_Destruct(CLzmaEnc *p)
{
  MatchFinder_Free(&MFB);
  LzmaEnc_FreeLits(p);
  RangeEnc_Free(&p->rc);
}

void LzmaEnc_Destroy(CLzmaEnc* p)
{
  
  LzmaEnc_Destruct(p);
  free(p);
}

static int LzmaEnc_CodeOneBlock(CLzmaEnc *p, Uint maxPackSize, Uint maxUnpackSize)
{
  Uint nowPos32, startPos32;
  if (p->needInit)
  {
    p->matchFinder.Init(p->matchFinderObj);
    p->needInit = 0;
  }

  if (p->finished)
    return p->result;
  RINOK(CheckErrors(p))

  nowPos32 = (Uint)p->nowPos64;
  startPos32 = nowPos32;

  if (p->nowPos64 == 0)
  {
    unsigned numPairs;
    Byte curByte;
    if (p->matchFinder.GetNumAvailableBytes(p->matchFinderObj) == 0)
      return Flush(p, nowPos32);
    ReadMatchDistances(p, &numPairs);
    RangeEnc_EncodeBit_0(&p->rc, &p->isMatch[kState_Start][0]);
    
    curByte = *(p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - p->additionalOffset);
    LitEnc_Encode(&p->rc, p->litProbs, curByte);
    p->additionalOffset--;
    nowPos32++;
  }

  if (p->matchFinder.GetNumAvailableBytes(p->matchFinderObj) != 0)
  
  for (;;)
  {
    Uint dist;
    unsigned len, posState;
    Uint range, ttt, newBound;
    CLzmaProb *probs;
  
    if (p->fastMode)
      len = GetOptimumFast(p);
    else
    {
      unsigned oci = p->optCur;
      if (p->optEnd == oci)
        len = GetOptimum(p, nowPos32);
      else
      {
        const COptimal *opt = &p->opt[oci];
        len = opt->len;
        p->backRes = opt->dist;
        p->optCur = oci + 1;
      }
    }

    posState = (unsigned)nowPos32 & p->pbMask;
    range = p->rc.range;
    probs = &p->isMatch[p->state][posState];
    
    RC_BIT_PRE(&p->rc, probs)
    
    dist = p->backRes;

    if (dist == MARK_LIT)
    {
      Byte curByte;
      const Byte *data;
      unsigned state;

      RC_BIT_0(&p->rc, probs)
      p->rc.range = range;
      data = p->matchFinder.GetPointerToCurrentPos(p->matchFinderObj) - p->additionalOffset;
      probs = LIT_PROBS(nowPos32, *(data - 1));
      curByte = *data;
      state = p->state;
      p->state = kLiteralNextStates[state];
      if (IsLitState(state))
        LitEnc_Encode(&p->rc, probs, curByte);
      else
        LitEnc_EncodeMatched(&p->rc, probs, curByte, *(data - p->reps[0]));
    }
    else
    {
      RC_BIT_1(&p->rc, probs)
      probs = &p->isRep[p->state];
      RC_BIT_PRE(&p->rc, probs)
      
      if (dist < LZMA_NUM_REPS)
      {
        RC_BIT_1(&p->rc, probs)
        probs = &p->isRepG0[p->state];
        RC_BIT_PRE(&p->rc, probs)
        if (dist == 0)
        {
          RC_BIT_0(&p->rc, probs)
          probs = &p->isRep0Long[p->state][posState];
          RC_BIT_PRE(&p->rc, probs)
          if (len != 1)
          {
            RC_BIT_1_BASE(&p->rc, probs)
          }
          else
          {
            RC_BIT_0_BASE(&p->rc, probs)
            p->state = kShortRepNextStates[p->state];
          }
        }
        else
        {
          RC_BIT_1(&p->rc, probs)
          probs = &p->isRepG1[p->state];
          RC_BIT_PRE(&p->rc, probs)
          if (dist == 1)
          {
            RC_BIT_0_BASE(&p->rc, probs)
            dist = p->reps[1];
          }
          else
          {
            RC_BIT_1(&p->rc, probs)
            probs = &p->isRepG2[p->state];
            RC_BIT_PRE(&p->rc, probs)
            if (dist == 2)
            {
              RC_BIT_0_BASE(&p->rc, probs)
              dist = p->reps[2];
            }
            else
            {
              RC_BIT_1_BASE(&p->rc, probs)
              dist = p->reps[3];
              p->reps[3] = p->reps[2];
            }
            p->reps[2] = p->reps[1];
          }
          p->reps[1] = p->reps[0];
          p->reps[0] = dist;
        }

        RC_NORM(&p->rc)

        p->rc.range = range;

        if (len != 1)
        {
          LenEnc_Encode(&p->repLenProbs, &p->rc, len - LZMA_MATCH_LEN_MIN, posState);
          --p->repLenEncCounter;
          p->state = kRepNextStates[p->state];
        }
      }
      else
      {
        unsigned posSlot;
        RC_BIT_0(&p->rc, probs)
        p->rc.range = range;
        p->state = kMatchNextStates[p->state];

        LenEnc_Encode(&p->lenProbs, &p->rc, len - LZMA_MATCH_LEN_MIN, posState);
        

        dist -= LZMA_NUM_REPS;
        p->reps[3] = p->reps[2];
        p->reps[2] = p->reps[1];
        p->reps[1] = p->reps[0];
        p->reps[0] = dist + 1;
        
        p->matchPriceCount++;
        GetPosSlot(dist, posSlot)
        
        {
          Uint sym = (Uint)posSlot + (1 << kNumPosSlotBits);
          range = p->rc.range;
          probs = p->posSlotEncoder[GetLenToPosState(len)];
          do
          {
            CLzmaProb *prob = probs + (sym >> kNumPosSlotBits);
            Uint bit = (sym >> (kNumPosSlotBits - 1)) & 1;
            sym <<= 1;
            RC_BIT(&p->rc, prob, bit)
          }
          while (sym < (1 << kNumPosSlotBits * 2));
          p->rc.range = range;
        }
        
        if (dist >= kStartPosModelIndex)
        {
          unsigned footerBits = ((posSlot >> 1) - 1);

          if (dist < kNumFullDistances)
          {
            unsigned base = ((2 | (posSlot & 1)) << footerBits);
            RcTree_ReverseEncode(&p->rc, p->posEncoders + base, footerBits, (unsigned)(dist ));
          }
          else
          {
            Uint pos2 = (dist | 0xF) << (32 - footerBits);
            range = p->rc.range;
            
            
            do
            {
              range >>= 1;
              p->rc.low += range & (0 - (pos2 >> 31));
              pos2 += pos2;
              RC_NORM(&p->rc)
            }
            while (pos2 != 0xF0000000);


            

            {
              unsigned m = 1;
              unsigned bit;
              bit = dist & 1; dist >>= 1; RC_BIT(&p->rc, p->posAlignEncoder + m, bit)  m = (m << 1) + bit;
              bit = dist & 1; dist >>= 1; RC_BIT(&p->rc, p->posAlignEncoder + m, bit)  m = (m << 1) + bit;
              bit = dist & 1; dist >>= 1; RC_BIT(&p->rc, p->posAlignEncoder + m, bit)  m = (m << 1) + bit;
              bit = dist & 1;             RC_BIT(&p->rc, p->posAlignEncoder + m, bit)
              p->rc.range = range;
              
            }
          }
        }
      }
    }

    nowPos32 += (Uint)len;
    p->additionalOffset -= len;
    
    if (p->additionalOffset == 0)
    {
      Uint processed;

      if (!p->fastMode)
      {
        
        if (p->matchPriceCount >= 64)
        {
          FillAlignPrices(p);
          
          FillDistancesPrices(p);
          
          LenPriceEnc_UpdateTables(&p->lenEnc, (unsigned)1 << p->pb, &p->lenProbs, p->ProbPrices);
        }
        if (p->repLenEncCounter <= 0)
        {
          p->repLenEncCounter = REP_LEN_COUNT;
          LenPriceEnc_UpdateTables(&p->repLenEnc, (unsigned)1 << p->pb, &p->repLenProbs, p->ProbPrices);
        }
      }
    
      if (p->matchFinder.GetNumAvailableBytes(p->matchFinderObj) == 0)
        break;
      processed = nowPos32 - startPos32;
      
      if (maxPackSize)
      {
        if (processed + kNumOpts + 300 >= maxUnpackSize
            || RangeEnc_GetProcessed_sizet(&p->rc) + kPackReserve >= maxPackSize)
          break;
      }
      else if (processed >= (1 << 17))
      {
        p->nowPos64 += nowPos32 - startPos32;
        return CheckErrors(p);
      }
    }
  }

  p->nowPos64 += nowPos32 - startPos32;
  return Flush(p, nowPos32);
}



#define kBigHashDicLimit ((Uint)1 << 24)

static int LzmaEnc_Alloc(CLzmaEnc *p, Uint keepWindowSize)
{
  Uint beforeSize = kNumOpts;
  Uint dictSize;

  if (!RangeEnc_Alloc(&p->rc))
    return SZ_ERROR_MEM;

  {
    const unsigned lclp = p->lc + p->lp;
    if (!p->litProbs || !p->saveState.litProbs || p->lclp != lclp)
    {
      LzmaEnc_FreeLits(p);
      p->litProbs =           (CLzmaProb *)SzAlloc(((size_t)0x300 * sizeof(CLzmaProb)) << lclp);
      p->saveState.litProbs = (CLzmaProb *)SzAlloc(((size_t)0x300 * sizeof(CLzmaProb)) << lclp);
      if (!p->litProbs || !p->saveState.litProbs)
      {
        LzmaEnc_FreeLits(p);
        return SZ_ERROR_MEM;
      }
      p->lclp = lclp;
    }
  }

  MFB.bigHash = (Byte)(p->dictSize > kBigHashDicLimit ? 1 : 0);


  dictSize = p->dictSize;
  if (dictSize == ((Uint)2 << 30) ||
      dictSize == ((Uint)3 << 30))
  {
    
    dictSize -= 1;
  }

  if (beforeSize + dictSize < keepWindowSize)
    beforeSize = keepWindowSize - dictSize;


  {
    if (!MatchFinder_Create(&MFB, dictSize, beforeSize,
        p->numFastBytes, LZMA_MATCH_LEN_MAX + 1))
      return SZ_ERROR_MEM;
    p->matchFinderObj = &MFB;
    MatchFinder_CreateVTable(&MFB, &p->matchFinder);
  }
  
  return SZ_OK;
}

static void LzmaEnc_Init(CLzmaEnc *p)
{
  unsigned i;
  p->state = 0;
  p->reps[0] =
  p->reps[1] =
  p->reps[2] =
  p->reps[3] = 1;

  RangeEnc_Init(&p->rc);

  for (i = 0; i < (1 << kNumAlignBits); i++)
    p->posAlignEncoder[i] = kProbInitValue;

  for (i = 0; i < kNumStates; i++)
  {
    unsigned j;
    for (j = 0; j < LZMA_NUM_PB_STATES_MAX; j++)
    {
      p->isMatch[i][j] = kProbInitValue;
      p->isRep0Long[i][j] = kProbInitValue;
    }
    p->isRep[i] = kProbInitValue;
    p->isRepG0[i] = kProbInitValue;
    p->isRepG1[i] = kProbInitValue;
    p->isRepG2[i] = kProbInitValue;
  }

  {
    for (i = 0; i < kNumLenToPosStates; i++)
    {
      CLzmaProb *probs = p->posSlotEncoder[i];
      unsigned j;
      for (j = 0; j < (1 << kNumPosSlotBits); j++)
        probs[j] = kProbInitValue;
    }
  }
  {
    for (i = 0; i < kNumFullDistances; i++)
      p->posEncoders[i] = kProbInitValue;
  }

  {
    const size_t num = (size_t)0x300 << (p->lp + p->lc);
    size_t k;
    CLzmaProb *probs = p->litProbs;
    for (k = 0; k < num; k++)
      probs[k] = kProbInitValue;
  }


  LenEnc_Init(&p->lenProbs);
  LenEnc_Init(&p->repLenProbs);

  p->optEnd = 0;
  p->optCur = 0;

  {
    for (i = 0; i < kNumOpts; i++)
      p->opt[i].price = kInfinityPrice;
  }

  p->additionalOffset = 0;

  p->pbMask = ((unsigned)1 << p->pb) - 1;
  p->lpMask = ((Uint)0x100 << p->lp) - ((unsigned)0x100 >> p->lc);

  
}


static void LzmaEnc_InitPrices(CLzmaEnc *p)
{
  if (!p->fastMode)
  {
    FillDistancesPrices(p);
    FillAlignPrices(p);
  }

  p->lenEnc.tableSize =
  p->repLenEnc.tableSize =
      p->numFastBytes + 1 - LZMA_MATCH_LEN_MIN;

  p->repLenEncCounter = REP_LEN_COUNT;

  LenPriceEnc_UpdateTables(&p->lenEnc, (unsigned)1 << p->pb, &p->lenProbs, p->ProbPrices);
  LenPriceEnc_UpdateTables(&p->repLenEnc, (unsigned)1 << p->pb, &p->repLenProbs, p->ProbPrices);
}

static int LzmaEnc_AllocAndInit(CLzmaEnc *p, Uint keepWindowSize)
{
  unsigned i;
  for (i = kEndPosModelIndex / 2; i < kDicLogSizeMax; i++)
    if (p->dictSize <= ((Uint)1 << i))
      break;
  p->distTableSize = i * 2;

  p->finished = 0;
  p->result = SZ_OK;
  p->nowPos64 = 0;
  p->needInit = 1;
  RINOK(LzmaEnc_Alloc(p, keepWindowSize))
  LzmaEnc_Init(p);
  LzmaEnc_InitPrices(p);
  return SZ_OK;
}

int LzmaEnc_MemPrepare(CLzmaEnc* p,
    const Byte *src, size_t srcLen,
    Uint keepWindowSize)
{
  
  MatchFinder_SET_DIRECT_INPUT_BUF(&MFB, src, srcLen)
  LzmaEnc_SetDataSize(p, srcLen);
  return LzmaEnc_AllocAndInit(p, keepWindowSize);
}

void LzmaEnc_Finish(CLzmaEnc* p)
{
  UNUSED_VAR(p)
}


typedef struct
{
  ISeqOutStream vt;
  Byte *data;
  size_t rem;
  int overflow;
} CLzmaEnc_SeqOutStreamBuf;

static size_t SeqOutStreamBuf_Write(ISeqOutStream* pp, const void *data, size_t size)
{
  Z7_CONTAINER_FROM_VTBL_TO_DECL_VAR(pp, CLzmaEnc_SeqOutStreamBuf, vt, p)
  if (p->rem < size)
  {
    size = p->rem;
    p->overflow = 1;
  }
  if (size != 0)
  {
    memcpy(p->data, data, size);
    p->rem -= size;
    p->data += size;
  }
  return size;
}

static int LzmaEnc_Encode2(CLzmaEnc *p)
{
  int res = SZ_OK;

  for (;;)
  {
    res = LzmaEnc_CodeOneBlock(p, 0, 0);
    if (res != SZ_OK || p->finished)
      break;
  }
  
  LzmaEnc_Finish((CLzmaEnc*)(void *)p);

  return res;
}

int LzmaEnc_WriteProperties(CLzmaEnc* p, Byte *props, size_t *size)
{
  if (*size < LZMA_PROPS_SIZE)
    return SZ_ERROR_PARAM;
  *size = LZMA_PROPS_SIZE;
  {
    
    const Uint dictSize = p->dictSize;
    Uint v;
    props[0] = (Byte)((p->pb * 5 + p->lp) * 9 + p->lc);
    
    
    if (dictSize >= ((Uint)1 << 21))
    {
      const Uint kDictMask = ((Uint)1 << 20) - 1;
      v = (dictSize + kDictMask) & ~kDictMask;
      if (v < dictSize)
        v = dictSize;
    }
    else
    {
      unsigned i = 11 * 2;
      do
      {
        v = (Uint)(2 + (i & 1)) << (i >> 1);
        i++;
      }
      while (v < dictSize);
    }

    SetUi32(props + 1, v)
    return SZ_OK;
  }
}


int LzmaEnc_MemEncode(CLzmaEnc* p, Byte *dest, size_t *destLen, const Byte *src, size_t srcLen,
    int writeEndMark)
{
  int res;
  

  CLzmaEnc_SeqOutStreamBuf outStream;

  outStream.vt.Write = SeqOutStreamBuf_Write;
  outStream.data = dest;
  outStream.rem = *destLen;
  outStream.overflow = 0;

  p->writeEndMark = writeEndMark;
  p->rc.outStream = &outStream.vt;

  res = LzmaEnc_MemPrepare(p, src, srcLen, 0);
  
  if (res == SZ_OK)
  {
    res = LzmaEnc_Encode2(p);
    if (res == SZ_OK && p->nowPos64 != srcLen)
      res = SZ_ERROR_FAIL;
  }

  *destLen -= (size_t)outStream.rem;
  if (outStream.overflow)
    return 7;
  return res;
}


int LzmaEncode(Byte *dest, size_t *destLen, const Byte *src, size_t srcLen,
    const CLzmaEncProps *props, Byte *propsEncoded, size_t *propsSize, int writeEndMark)
{
  CLzmaEnc* p = LzmaEnc_Create();
  int res;
  if (!p)
    return SZ_ERROR_MEM;

  res = LzmaEnc_SetProps(p, props);
  if (res == SZ_OK)
  {
    res = LzmaEnc_WriteProperties(p, propsEncoded, propsSize);
    if (res == SZ_OK)
      res = LzmaEnc_MemEncode(p, dest, destLen, src, srcLen,
          writeEndMark);
  }

  LzmaEnc_Destroy(p);
  return res;
}

int LzmaCompress(unsigned char *dest, size_t *destLen, const unsigned char *src, size_t srcLen,
  unsigned char *outProps, size_t *outPropsSize,
  int level, 
  unsigned dictSize, 
  int lc, 
  int lp, 
  int pb, 
  int fb,  
  int numThreads 
)
{
  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  props.level = level;
  props.dictSize = dictSize;
  props.lc = lc;
  props.lp = lp;
  props.pb = pb;
  props.fb = fb;
  props.numThreads = numThreads;

  return LzmaEncode(dest, destLen, src, srcLen, &props, outProps, outPropsSize, 0);
}


int LzmaUncompress(unsigned char *dest, size_t *destLen, const unsigned char *src, size_t *srcLen,
  const unsigned char *props, size_t propsSize)
{
  ELzmaStatus status;
  return LzmaDecode(dest, destLen, src, srcLen, props, (unsigned)propsSize, LZMA_FINISH_ANY, &status);
}


namespace sz {
  td::BufferSlice compress(td::Slice data) {
    auto size = data.size();

    size_t dest_len = 1 << 22;
    auto dest = new unsigned char[dest_len];

    unsigned char outProps[LZMA_PROPS_SIZE] = {};
    size_t outPropsSize = LZMA_PROPS_SIZE;

    int level = 9;      
    unsigned dictSize = 1<<20;  
    int lc = 8;        
    int lp = 0;        
    int pb = 0;        
    int fb = 273;        
    int numThreads = 1; 


    int res = LzmaCompress(dest, &dest_len, (unsigned char*)data.data(), size,
          outProps, &outPropsSize, level, dictSize, lc, lp, pb, fb, numThreads);

    CHECK(res == SZ_OK);
    return td::BufferSlice((char*)dest, dest_len);
  }

  td::BufferSlice decompress(td::Slice data) {
    auto size = data.size();
    size_t dest_len = 1 << 22;
    auto dest = new unsigned char[dest_len];

    unsigned char props[LZMA_PROPS_SIZE] = {
      0x8, 0, 0, 0, 0x80
    };
    size_t propsSize = LZMA_PROPS_SIZE;

    int res = LzmaUncompress(dest, &dest_len, (unsigned char*)data.data(), &size, props, propsSize);
    CHECK(res == SZ_OK || res == SZ_ERROR_INPUT_EOF);
    return td::BufferSlice((char*)dest, dest_len);
  }
}
