// zpaq.cpp - Journaling incremental deduplicating archiver

#define NOJIT 1

#define ZPAQ_VERSION "7.15"

#define _FILE_OFFSET_BITS 64  // In Linux make sizeof(off_t) == 8
#ifndef UNICODE
#define UNICODE  // For Windows
#endif
// #include "libzpaq.h"

/* libzpaq.h - LIBZPAQ Version 7.12 header - Apr. 19, 2016.

  This software is provided as-is, with no warranty.
  I, Matt Mahoney, release this software into
  the public domain.   This applies worldwide.
  In some countries this may not be legally possible; if so:
  I grant anyone the right to use this software for any purpose,
  without any conditions, unless such conditions are required by law.

LIBZPAQ is a C++ library providing data compression and decompression
services using the ZPAQ level 2 format as described in
http://mattmahoney.net/zpaq/

An application wishing to use these services should #include "libzpaq.h"
and link to libzpaq.cpp (and advapi32.lib in Windows/VC++).
libzpaq recognizes the following options:

  -DDEBUG   Turn on assertion checks (slower).
  -DNOJIT   Don't assume x86-32 or x86-64 with SSE2 (slower).
  -Dunix    Without -DNOJIT, assume Unix (Linux, Mac) rather than Windows.

The application must provide an error handling function and derived
implementations of two abstract classes, Reader and Writer,
specifying the input and output byte streams. For example, to compress
from stdin to stdout (assuming binary I/O as in Linux):

  #include "libzpaq.h"
  #include <stdio.h>
  #include <stdlib.h>

  void libzpaq::error(const char* msg) {  // print message and exit
    fprintf(stderr, "Oops: %s\n", msg);
    exit(1);
  }

  class In: public libzpaq::Reader {
  public:
    int get() {return getchar();}  // returns byte 0..255 or -1 at EOF
  } in;

  class Out: public libzpaq::Writer {
  public:
    void put(int c) {putchar(c);}  // writes 1 byte 0..255
  } out;

  int main() {
    libzpaq::compress(&in, &out, "1");  // "0".."5" = faster..better
  }

Or to decompress:

    libzpaq::decompress(&in, &out);

The function error() will be called with an English language message
in case of an unrecoverable error such as badly formatted compressed
input data or running out of memory. error() should not return.
In a multi-threaded application where ZPAQ blocks are being decompressed
in separate threads, error() should exit the thread, but other threads
may continue. Blocks are independent and libzpaq is thread safe.

Reader and Writer provide default implementations of read() and write()
for block I/O. You may override these with your own versions, which
might be faster. The default is to call get() or put() the appropriate
number of times. For example:

  // Read n bytes into buf[0..n-1] or to EOF, whichever is first.
  // Return the number of bytes actually read.
  int In::read(char* buf, int n) {return fread(buf, 1, n, stdin);}

  // Write buf[0..n-1]
  void Out::write(char* buf, int n) {fwrite(buf, 1, n, stdout);}

By default, compress() divides the input into blocks with one segment
each. The segment filename field is empty. The comment field of each
block is the uncompressed size as a decimal string. The checksum
is saved. To override:

  compress(&in, &out, "1", "filename", "comment", false);

If the filename is not NULL then it is saved in the first block only.
If the comment is not NULL then a space and the comment are appended
to the decimal size in the first block only. The comment would normally
be the date and attributes like "20141231235959 w32", or "jDC\x01" for
a journaling archive as described in the ZPAQ specification.

The method string has the general form of a concatenation of single
character commands each possibly followed by a list of decimal
numeric arguments separated by commas or periods:

  {012345xciawmst}[N1[{.,}N2]...]...

For example "1" or "14,128,0" or "x6.3ci1m".

Only the first command can be a digit 0..5. If it is, then it selects
a compression level and the other commands are ignored. Otherwise,
if it is "x" then the arguments and remaining commands describe
the compression method. Any other letter as the first command is
interpreted the same as "x". 

Higher compression levels are slower but compress better. "1" is
good for most purposes. "0" does not compress. "2" compresses slower
but decompression is just as fast as 1. "3", "4", and "5" also
decompress slower. The numeric arguments are as follows:

  N1: 0..11 = block size of at most 2^N1 MiB - 4096 bytes (default 4).
  N2: 0..255 = estimated ease of compression (default 128).
  N3: 0..3 = data type. 1 = text, 2 = exe, 3 = both (default 0).

For example, "14" or "54" divide the input in 16 MB blocks which
are compressed independently. N2 and N3 are hints to the compressor
based on analysis of the input data. N2 is 0 if the data is random
or 255 if the data is easily compressed (for example, all zero bytes).
Most compression methods will simply store random data with no
compression. The default is "14,128,0".

If the first command is "x" then the string describes the exact
compression method. The arguments to "x" describe the pre/post
processing (LZ77, BWT, E8E9), and remaining commands describe the
context model, if any, of the transformed data. The arguments to "x" are:

  N1: 0..11 = block size as before.
  N2: 0..7: 0=none, 1=packed LZ77, 2=LZ77, 3=BWT, 4..7 = 0..3 + E8E9.
  N3: 4..63: LZ77 min match.
  N4: LZ77 secondary match to try first or 0 to skip.
  N5: LZ77 log search depth.
  N6: LZ77 log hash table size, or N1+21 to use a suffix array.
  N7: LZ77 lookahead.

N2 selects the basic transform applied before context modeling.
N2 = 0 does not transform the input. N2 = 1 selects LZ77 encoding
of literals strings and matches using bit-packed codes. It is normally
not used with a context model. N2 = 2 selects byte aligned LZ77, which
compresses worse by itself but better than 1 when a context model is
used. It uses single bytes to encode either a literal of length 1..64
or a match of length N3..N3+63 with a 2, 3, or 4 byte offset.

N2 = 3 selects a Burrows-Wheeler transform, in which the input is
sorted by right-context. This does not compress by itself but makes
the data more compressible using a low order, adaptive context model.
BWT requires 4 times the block size in additional memory for both
compression and decompression.

N2 = 4..7 are the same as 0..3 except that a E8E9 transform is first applied
to improve the compression of x86 code usually found .exe and .dll files.
It scans the input block backward for 5 byte strings of the form
{E8|E9 xx xx xx 00|FF} and adds the offset from the start of the
block to the middle 3 bytes interpreted as a little-endian (LSB first)
number (mod 2^24). E8 and E9 are the CALL and JMP instructions, followed
by a 32 bit relative offset.

N3..N7 apply only to LZ77. For either type, it searches for matches
by hashing the next N4 bytes, and then the next N3 bytes, and looking
up each of the hashes at 2^N5 locations in a table with 2^N6 entries.
Of those, it picks the longest match, or closest in case of a tie.
If no match is at least N3, then a literal is encoded instead. If N5
is 0 then only one hash is computed, which is faster but does not
compress as well. Typical good values for fast compression are
"x4.1.5.0.3.22" which means 16 MiB blocks, packed LZ77, mininum match
length 5, no secondary match, search depth 2^3 = 8, and 2^22 = 4M
hash table (using 16 MiB memory).

The hash table requires 4 x 2^N6 bytes of memory. If N6 = N1+21, then
matches are found using a suffix array and inverse suffix array using
2.25 x 2^N6 bytes (4.5 x block size). This finds better matches but
takes longer to compute the suffix array (SA). The matches are found by
searching forward and backward in the SA 2^N5 in each direction up
to the first earlier match, and picking the longer of the two.
Good values are "x4.1.4.0.8.25". The secondary match N4 has no effect.

N7 is the lookahead. It looks for matches of length at least N4+N7
when using a hash table or N3+N7 for a SA, but allows the first N7
bytes not to match and be coded as literals if this results in
a significantly longer match. Values higher than 1 are rarely effective.
The default is 0.

All subsequent commands after "x" describe a context model. A model
consists of a set of components that output a bit prediction, taking
a context and possibly earlier predictions as input. The final prediction
is arithmetic coded. The component types are:

  c = CM or ICM (context model or indirect context model).
  i = ISSE chain (indirect secondary symbol estimator).
  a = MATCH.
  w = word model (ICM-ISSE chain with whole word contexts).
  m = MIX.
  s = SSE (secondary symbol estimator).
  t = MIX2 (2 input MIX).

For example, "x4.3ci1" describes a BWT followed by an order 0 CM
and order 1 ISSE, which is used for level 3 text compression. The
parameters to "c" (default all 0) are as follows:

  N1: 0 = ICM, 1..256 CM with faster..slower adaptation, +1000 halves memory.
  N2: 1..255 = offset mod N2, 1000..1255 = offset to last N2-1000 byte.
  N3: 0..255 = order 0 context mask, 256..511 mixes LZ77 parse state.
  N4...: 0..255 order 1... context masks. 1000... skips N4-1000 bytes.

Most components use no more memory than the block size, depending on
the number of context bits, but it is possible to select less memory
and lose compression.

A CM inputs a context hash and outputs a prediction from a table.
The table entry is then updated by adjusting in the direction of the
actual bit. The adjustment is 1/count, where the maximum count is 4 x N1.
Larger values are best for stationary data. Smaller values adapt faster
to changing data.

If N1 is 0 then c selects an ICM. An ICM maps a context to a bit history
(8 bit state), and then to slow adapting prediction. It is generally
better than a CM on most nonstationary data.

The context for a CM or ICM is a hash of all selected contexts: a
cyclic counter (N2 = 1..255), the distance from the last occurrence
of some byte value (N2 = 1000..1255), and the masked history of the
last 64K bytes ANDED with N3, N4... For example, "c0.0.255.255.255" is
an order 3 ICM. "C0.1010.255" is an order 1 context hashed together
with the column number in a text file (distance to the last linefeed,
ASCII 10). "c256.0.255.1511.255" is a stationary grayscale 512 byte
wide image model using the two previous neighboring pixels as context.
"c0.0.511.255" is an order 1 model for LZ77, which helps compress
literal strings. The LZ77 state context applies only to byte aligned
LZ77 (type 2 or 6).

The parameters to "i" (ISSE chain) are the initial context length and
subsequent increments for a chain connected to an existing earlier component.
For example, "ci1.1.2" specifies an ICM (order 0) followed by a chain
of 3 ISSE with orders 1, 2, and 4. An ISSE maps a context to a bit
history like an ISSE, but uses the history to select a pair of weights
to mix the input prediction with a constant 1, thus performing the
mapping q' := w1 x q + w2 in the logistic domain (q = log p/(1-p)).
The mixer is then updated by adjusting the weights to improve the
prediction. High order ISSE chains (like "x4.0ci1.1.1.1.2") and BWT
followed by a low order chain (like "x4.3ci1") both provide
excellent general purpose compression.

A MATCH ("a") keeps a rotating history buffer and a hash table to look
up the previous occurrence of the current context hash and predicts
whatever bit came next. The parameters are:

  N1 = hash multiplier, default 24.
  N2 = halve buffer size, default 0 = same size as input block.
  N3 = halve hash table size, default 0 = block size / 4.

For example, "x4.0m24.1.1" selects a 16 MiB block size, 8 MiB match
buffer size, and 2M hash table size (using 8 MiB at 4 bytes per entry).
The hash is computed as hash := hash x N1 + next_byte + 1 (mod hash table
size). Thus, N1 = 12 selects a higher order context, and N1 = 48 selects a
lower order.

A word model ('w") is an ICM-ISSE chain of length N1 (orders 0..N1-1)
in which the contexts are whole words. A word is defined as the set
of characters in the range N2..N2+N3-1 after ANDing with N4. The context
is hashed using multiplier N5. Memory is halved by N6. The default is
"w1.65.26.223.20.0" which is a chain of length 1 (ICM only), where words
are in range 65 ('A') to 65+26-1 ('Z') after ANDing with 223 (which
converts to upper case). The hash multiplier is 20, which has the
effect of shifting the high 2 bits out of the hash. The memory usage
of each component is the same as the block size.

A MIX ("m") performs the weighted average of all previous component
predictions. The weights are then adjusted to improve the prediction
by favoring the most accurate components. N1 selects the number of
context bits (not hashed) to select a set of weights. N2 is the
learning rate (around 16..32 works well). The default is "m8.24"
which selects the previously modeled bits of the current byte as
context. When N1 is not a multiple of 8, it selects the most significant
bits of the oldest byte.

A SSE ("s") adjusts the previous prediction like an ISSE, but uses
a direct lookup table of the quantized and interpolated input prediction
and a direct (not hashed) N1-bit context. The adjustment is 1/count where
the count is allowed to range from N2 to 4 x N3. The default
is "s8.32.255".

A MIX2 ("t") is a MIX but mixing only the last 2 components. The
default is "t8.24" where the meaning is the same as "m".

For example, a good model for text is "x6.0ci1.1.1.1.2aw2mm16tst"
which selects 2^6 = 64 MiB blocks, no preprocessing, an order 0 ICM,
an ISSE chain with orders 1, 2, 3, 4, 6, a MATCH, an order 0-1 word
ICM-ISSE chain, two mixers with 0 and 1 byte contexts, whose outputs are
mixed by a MIX2. The MIX2 output is adjusted by a SSE, and finally
the SSE input and outputs are mixed again for the final bit prediction.


COMPRESSBLOCK

CompressBlock() takes the same arguments as compress() except that
the input is a StringBuffer instead of a Reader. The output is always
a single block, regardless of the N1 (block size) argument in the method.

  void compressBlock(StringBuffer* in, Writer* out, const char* method,
                     const char* filename=0, const char* comment=0,
                     bool compute_sha1=false);

A StringBuffer is both a Reader and a Writer, but also allows random
memory access. It provides convenient and efficient storage when the
input size is unknown.

  class StringBuffer: public libzpaq::Reader, public libzpaq::Writer {
  public:
    StringBuffer(size_t n=0);     // initial allocation after first use
    ~StringBuffer();
    int get();                    // read 1 byte or EOF from memory
    int read(char* buf, int n);   // read n bytes
    void put(int c);              // write 1 byte to memory
    void write(const char* buf, int n);  // write n bytes
    const char* c_str() const;    // read-only access to written data
    unsigned char* data();        // read-write access
    size_t size() const;          // number of bytes written
    size_t remaining() const;     // number of bytes to read until EOF
    void setLimit(size_t n);      // set maximum write size
    void reset();                 // discard contents and free memory
    void resize(size_t n);        // truncate to n bytes
    void swap(StringBuffer& s);   // exchange contents efficiently
  };

The constructor sets the inital allocation size after the first
write to n or 128, whichever is larger. Initially, no memory is allocated.
The allocated size is always n x (2^k - 1), for example
128 x (1, 3, 7, 15, 31...).

put() and write() append 1 or n bytes, allocating memory as needed.
buf can be NULL and the StringBuffer will be enlarged by n.
get() and read() read 1 or up to n bytes. get() returns EOF if you
attempt to read past the end of written data. read() returns less
than n if it reaches EOF first, or 0 at EOF.

size() is the number of bytes written, which does not change when
data is read. remaining() is the number of bytes left to read
before EOF.

c_str() provides read-only access to the data. It is not NUL terminated.
data() provides read-write access. Either may return NULL if size()
is 0. write(), put(), reset(), swap(), and the destructor may
invalidate saved pointers.

setLimit() sets a maximum size. It will call error() if you try to
write past it. The default is -1 or no limit.

reset() sets the size to 0 and frees memory. resize() sets the size
to n by moving the write pointer, but does not allocate or free memory.
Moving the pointer forward does not overwrite the previous contents
in between. The write pointer can be moved past the end of allocated
memory, and the next put() or write() will allocate as needed. If the
write pointer is moved back before the read pointer, then remaining()
is set to 0.

swap() swaps 2 StringBuffers efficiently, but does not change their
initial allocations.


DECOMPRESSER

decompress() will decompress any valid ZPAQ stream, which may contain
multiple blocks with multiple segments each. It will ignore filenames,
comments, and checksums. You need the Decompresser class if you want to
do something other than decompress all of the data serially to a single
file. To decompress individual blocks and segments and retrieve the
filenames, comments, data, and hashes of each segment (in exactly this
order):

  libzpaq::Decompresser d;               // to decompress
  libzpaq::SHA1 sha1;                    // to verify output hashes
  double memory;                         // bytes required to decompress
  Out filename, comment;
  char sha1out[21];
  d.setInput(&in);
  while (d.findBlock(&memory)) {         // default is NULL
    while (d.findFilename(&filename)) {  // default is NULL
      d.readComment(&comment);           // default is NULL
      d.setOutput(&out);                 // if omitted or NULL, discard output
      d.setSHA1(&sha1);                  // optional
      while (d.decompress(1000));        // bytes to decode, default is all
      d.readSegmentEnd(sha1out);         // {0} or {1,hash[20]}
      if (sha1out[0]==1 && memcmp(sha1.result(), sha1out+1, 20))
        error("checksum error");
    }
  }

findBlock() scans the input for the next ZPAQ block and returns true
if found. It optionally sets memory to the approximate number of bytes
that it will allocate at the first call to decompress().

findFilename() finds the next segment and returns false if there are
no more in the current block. It optionally writes the saved filename.

readComment() optionally writes the comment. It must be called
after reading the filename and before decompressing.

setSHA1() specifies an SHA1 object for computing a hash of the segment.
It may be omitted if you do not want to compute a hash.

decompress() decodes the requested number of bytes, postprocesses them,
and writes them to out. For the 3 built in compression levels, this
is the same as the number of bytes output, but it may be different if
postprocessing was used. It returns true until there is no more data
to decompress in the current segment. The default (-1) is to decompress the
whole segment.

readSegmentEnd() skips any remaining data not yet decompressed in the
segment and writes 21 bytes, either a 0 if no hash was saved, 
or a 1 followed by the 20 byte saved hash. If any data is skipped,
then all data in the remaining segments in the current block must
also be skipped.


SHA1

The SHA1 object computes SHA-1 cryptographic hashes. It is safe to
assume that two inputs with the same hash are identical. For example:

  libzpaq::SHA1 sha1;
  int ch;
  while ((ch=getchar())!=EOF)
    sha1.put(ch);
  printf("Size is %1.0f or %1.0f bytes\n", sha1.size(), double(sha1.usize()));

size() returns the number of bytes read as a double, and usize() as a
64 bit integer. result() returns a pointer to the 20 byte hash and
resets the size to 0. The hash (not just the pointer) should be copied
before the next call to result() if you want to save it. You can also
call sha1.write(buffer, n) to hash n bytes of char* buffer.


COMPRESSOR

A Compressor object allows greater control over the compressed data.
In particular you can specify the compression algorithm in ZPAQL to
specify methods not possible using compress() or compressBlock(). You
can create blocks with multiple segments specifying different files,
or compress streams of unlimited size to a single block when the
input size is not known.

  libzpaq::Compressor c;
  for (int i=0; i<num_blocks; ++i) {
    c.setOutput(&out);              // if omitted or NULL, discard output
    c.writeTag();                   // optional locator tag
    c.startBlock(2);                // compression level 1, 2, or 3
    for (int j=0; j<num_segments; ++j) {
      c.startSegment("filename", "comment");  // default NULL = empty
      c.setInput(&in);
      while (c.compress(1000));     // bytes to compress, default -1 = all
      c.endSegment(sha1.result());  // default NULL = don't save checksum
    }
    c.endBlock();
  }

Input and output can be set anywhere before the first input and output
operation, respectively. Output may be changed any time.

writeTag() outputs a 13 byte string that allows decompress() to scan
for blocks that don't occur immediately, such as searching from the
start of a self extracting archive.

startBlock() specifies either a compression level (1, 2, 3), or a ZPAQL
program, described below. It does not work with the fast method type
arguments to compress() or compressBlock(). Levels 1, 2, and 3 correspond
approximately to "3", "4", and "5". Any preprocessing must be done
by the application before input to the Compressor.

StartSegment() starts a segment. An empty or NULL filename continues
the previous file. The comment normally contains the uncompressed size
of the segment as a decimal string, but it is allowed to omit it.

compress() will read the requested number of bytes or until in.get()
returns EOF (-1), whichever comes first, and return true if there is
more data to decompress. If the argument is omitted or -1, then it will
read to EOF and return false.

endSegment() writes a provided SHA-1 cryptographic hash checksum of the
input segment before any preprocessing. It may be omitted.


ZPAQL

ZPAQ supports arbitrary compression algorithms in addition to the
built in levels. For example, method "x4.0c0.0.255.255i4" compression could
alternatively be specified using the ZPAQL language description of the
compression algorithm:

  int args[9]={0}
  c.startBlock(
    "(min.cfg - equivalent to level 1) "
    "comp 1 2 0 0 2 (log array sizes hh,hm,ph,pm and number of components n) "
    "  0 icm 16    (order 2 indirect context model using 4 MB memory) "
    "  1 isse 19 0 (order 4 indirect secondary symbol estimator, 32 MB) "
    "hcomp (context computation, input is last modeled byte in A) "
    "  *b=a a=0 (save in rotating buffer M pointed to by B) "
    "  d=0 hash b-- hash *d=a (put order 2 context hash in H[0] pointed by D)"
    "  d++ b-- hash b-- hash *d=a (put order 4 context in H[1]) "
    "  halt "
    "end " (no pre/post processing) ",
    args,     // Arguments $1 through $9 to ZPAQL code (unused, can be NULL)
    &out);    // Writer* to write pcomp command (default is NULL)

The first argument is a description of the compression algorithm in
the ZPAQL language. It is compiled into byte code and saved in the
archive block header so that the decompressor knows how read the data.
A ZPAQL program accepts up to 9 numeric arguments, which should be
passed in array.

A decompression algorithm has two optional parts, a context mixing
model and a postprocessor. The context model is identical for both
the compressor and decompressor, so is used in both instances. The
postprocessor, if present, is generally different, which presents
the possibility that the user supplied code may not restore the
original data exactly. It is assumed that the input has already been
preprocessed by the application but that the hash supplied to endSegment()
is of the original input before preprocessing. The following functions
allow you to test the postprocesser during compression:

  c.setVerify(true); // before c.compress(), may run slower
  c.getSize();       // return 64 bit size of postprocessed output
  c.getChecksum();   // after c.readSegmentEnd(), return hash, reset size to 0

This example has no postprocessor, but if it did, then setVerify(true)
would cause compress() to run the preprocessed input through the
postprocessor and into a SHA1 in parallel with compression. Then,
getChecksum() would return the hash which could be compared with
the hash of the input computed by the application. Also,

  int64_t size;
  c.endSegmentChecksum(&size, true);

instead of c.endSegment() will automatically add the computed checksum
if setVerify is true and return the checksum, whether or not there
is a postprocessor. If &size is not NULL then the segment size is written
to size. If the second argument is false then the computed checksum is
not saved to output. Default is true. If setVerify is false, then no
checksum is saved and the function returns 0 with size not written.

A context model consists of two parts, an array COMP of n components,
and some code HCOMP that computes contexts for the components.
The model compresses one bit at a time (MSB to LSB order) by computing
a probability that the next bit will be a 1, and then arithmetic
coding that bit. Better predictions compress smaller.

If n is 0 then the data is uncompressed. Otherwise, there is an array
of n = 1..255 components each taking a context and possibly the
predictions of previous components as input and outputting a new
prediction. The output of the last prediction is used to encode the
bit. After encoding, the state of each component is updated to
reduce the prediction error when the same context occurs again.
Components are as follows. Most arguments have range 0...255.

  CONST c          predict a 1 (c > 128) or 0 (c < 128).
  CM s t           context model with 2^s contexts, learning rate 1/4t.
  ICM s            indirect context model with 2^(s+6) contexts.
  MATCH s b        match model with 2^s context hashes and 2^b history.
  AVG j k wt       average components j and k with weight wt/256 for j.
  MIX2 s j k r x   average j and k with 2^s contexts, rate r, mask x.
  MIX  s j m r x   average j..j+m-1 with 2^s contexts, rate r, mask x.
  ISSE s j         adjust prediction j using 2^(s+6) indirect contexts.
  SSE s j t1 t2    adjust j using 2^s direct contexts, rate 1/t1..1/4t2.

A CONST predicts a 1 with probability 1/(1+exp((128-c)/16)), i.e
numbers near 0 or 255 are the most confident.
  
A CM maps a context to a prediction and a count. It is updated by
adjusting the prediction to reduce the error by 1/count and incrementing
the count up to 4t.

A ICM maps a s+10 bit context hash to a bit history (8 bit state)
representing a bounded count of zeros and ones previously seen in the
context and which bit was last. The bit history is mapped to a
prediction, which is updated by reducing the error by 1/1024.
The initial prediction is estimated from the counts represented by each
bit history.

A MATCH looks up a context hash and predicts whatever bit came next
following the previous occurrence in the history buffer. The strength
of the prediction depends on the match length.

AVG, MIX2, and MIX perform weighted averaging of predictions in the
logistic domain (log(p/(1-p))). AVG uses a fixed weight. MIX2 and MIX
adjust the weights (selected by context) to reduce prediction error
by a rate that increases with r. The mask is AND-ed with the current
partially coded byte to compute that context. Normally it is 255.
A MIX takes a contiguous range of m components as input.

ISSE adjusts a prediction using a bit history (as with an ICM) to
select a pair of weights for a 2 input MIX. It mixes the input
prediction with a constant 1 in the logistic domain.

SSE adjusts a logistic prediction by quantizing it to 32 levels and
selecting a new prediction from a table indexed by context, interpolating
between the nearest two steps. The nearest prediction error is
reduced by 1/count where count increments from t1 to 4*t2.

Contexts are computed and stored in an array H of 32 bit unsigned
integers by the HCOMP program written in ZPAQL. The program is called
after encoding a whole byte. To form a complete context, these values
are combined with the previous 0 to 7 bits of the current parital byte.
The method depends on the component type as follows:

  CM: H[i]    XOR hmap4(c).
  ICM, ISSE:  hash table lookup of (H[i]*16+c) on nibble boundaries.
  MIX2, MIX:  H[i] + (c AND x).
  SSE:        H[i] + c.

where c is the previous bits with a leading 1 bit (1, 1x, 1xx, ...,
1xxxxxxx where x is a previously coded bit). hmap4(c) maps c
to a 9 bit value to reduce cache misses. The first nibble is
mapped as before and the second nibble with 1xxxx in the high
5 bits. For example, after 6 bits, where c = 1xxxxxx,
hmap4(c) = 1xxxx01xx with the bits in the same order.

There are two ZPAQL virtual machines, HCOMP to compute contexts
and PCOMP to post-process the decoded output. Each has the
following state:

  PC: 16 bit program counter.
  A, B, C, D, R0...R255: 32 bit unsigned registers.
  F: 1 bit condition register.
  H: array of 2^h 32 bit unsigned values (output for HCOMP).
  M: array of 2^m 8 bit unsigned values.

All values are initialized to 0 at the beginning of a block
and retain their values between calls. There are two machines.
HCOMP is called after coding each byte with the value of that
byte in A. PCOMP, if present, is called once for each decoded
byte with that byte in A, and once more at the end of each
segment with 2^32 - 1 in A.

Normally, A is an accumulator. It is the destination of all
binary operations except assignment. The low m bits of B and
C index M. The low h bits of D indexes H. We write *B, *C, *D
to refer to the elements they point to. The instruction set
is as follows, where X is A, B, C, D, *B, *C, *D except as
indicated. X may also be a constant 0...255, written with
a leading space if it appears on the right side of an operator,
e.g. "*B= 255". Instructions taking a numeric argument are 2 bytes,
otherwise 1. Arithmetic is modulo 2^32.

  X<>A    Swap X with A (X cannot be A).
  X++     Add 1.
  X--     Subtract 1.
  X!      Complement bits of X.
  X=0     Clear X (1 byte instruction).
  X=X     Assignment to left hand side.
  A+=X    Add to A
  A-=X    Subtract from A
  A*=X    Multipy
  A/=X    Divide. If X is 0 then A=0.
  A%=X    Mod. If X is 0 then A=0.
  A&=X    Clear bits of A that are 0 in X.
  A&~X    Clear bits of A that are 1 in X.
  A|=X    Set bits of A that are 1 in X.
  A^=X    Complement bits of A that are set in X.
  A<<=X   Shift A left by (X mod 32) bits.
  A>>=X   Shift right (zero fill) A by (X mod 32) bits.
  A==X    Set F=1 if equal else F=0.
  A<X     Set F=1 if less else F=0.
  A>X     Set F=1 if greater else F=0.
  X=R N   Set A,B,C,D to RN (R0...R255).
  R=A N   Set R0...R255 to A.
  JMP N   Jump N=-128...127 bytes from next instruction.
  JT N    Jump N=-128...127 if F is 1.
  JF N    Jump N=-128...127 if F is 0.
  LJ N    Long jump to location 0...65535 (only 3 byte instruction).
  OUT     Output A (PCOMP only).
  HASH    A=(A+*B+512)*773.
  HASHD   *D=(*D+A+512)*773.
  HALT    Return at end of program.
  ERROR   Fail if executed.

Rather than using jump instructions, the following constructs are
allowed and translated appropriately.

  IF ... ENDIF              Execute if F is 1.
  IFNOT ... ENDIF           Execute if F is 0.
  IF ... ELSE ... ENDIF     Execute first part if F is 1 else second part.
  IFNOT ... ELSE ... ENDIF  Execute first part if F is 0 else second part.
  DO ... WHILE              Loop while F is 1.
  DO ... UNTIL              Loop while F is 0.
  DO ... FOREVER            Loop unconditionally.

Forward jumps (IF, IFNOT, ELSE) will not compile if beyond 127
instructions. In that case, use the long form (IFL, IFNOTL, ELSEL).
DO loops automatically use long jumps if needed. IF and DO loops
may intersect. For example, DO ... IF ... FOREVER ENDIF is equivalent
to a while-loop.

A config argument without a postprocessor has the following syntax:

  COMP hh hm ph pm n
    i COMP args...
  HCOMP
    zpaql...
  END (or POST 0 END for backward compatibility)

With a postprocessor:

  COMP hh hm ph pm n
    i COMP args...
  HCOMP
    zpaql...
  PCOMP command args... ;
    zpaql...
  END

In HCOMP, H and M have sizes 2^hh and 2^hm respectively. In PCOMP,
H and M have sizes 2^ph and 2^pm respectively. There are n components,
which must be numbered i = 0 to n-1. If a postprocessor is used, then
"command args..." is written to the Writer* passed as the 4'th argument,
but otherwise ignored. A typical use in a development environment might
be to call an external program that will be passed two additional
arguments on the command line, the input and output file names
respectively.

You can pass up to 9 signed numeric arguments in args[]. In any
place that a number "N" is allowed, you can write "$M" or "$M+N"
(like "$1" or $9+25") and value args[M-1]+N will be substituted.

ZPAQL allows (nested) comments in parenthesis. It is not case sensitive.
If there are input errors, then error() will report the error. If the
string contains newlines, it will report the line number of the error.

ZPAQL is compiled internally into a byte code, and then to native x86
32 or 64 bit code (unless compiled with -DNOJIT, in which case the
byte code is interpreted). You can also specify the algorithm directly
in byte code, although this is less convenient because it requires two
steps:

  c.startBlock(hcomp);      // COMP and HCOMP at start of block
  c.postProcess(pcomp, 0);  // PCOMP right before compress() in first segment

This is necessary because the COMP and HCOMP sections are stored in
the block header, but the PCOMP section is compressed in the first
segment after the filename and comment but before any data.

To retrive compiled byte code in suitable format after startBlock():

  c.hcomp(&out);      // writes COMP and HCOMP sections
  c.pcomp(&out);      // writes PCOMP section if any

Or during decompression:

  d.hcomp(&out);      // valid after findBlock()
  d.pcomp(&out);      // valid after decompress(0) in first segment

Both versions of pcomp() write nothing and return false if there is no
PCOMP section. The output of hcomp() and pcomp() may be passed to the
input of startBlock() and postProcess(). These are strings in which the
first 2 bytes encode the length of the rest of the string, least
significant byte first. Alternatively, postProcess() allows the length to
be omitted and passed separately as the second argument. In the case
of decompression, the HCOMP and PCOMP strings are read from the archive.
The preprocessor command (from "PCOMP cmd ;") is not saved in the compressed
data.


ARRAY

The libzpaq::Array template class is convenient for creating arrays aligned
on 64 byte addresses. It calls error("Out of memory") if needed.
It is used as follows:

  libzpaq::Array<T> a(n);  // array a[0]..a[n-1] of type T, zeroed
  a.resize(n);             // change size and zero contents
  a[i]                     // i'th element
  a(i)                     // a[i%n], valid only if n is a power of 2
  a.size()                 // n (as a size_t)
  a.isize()                // n (as a signed int)

T should be a simple type without constructors or destructors. Arrays
cannot be copied or assigned. You can also specify the size:

  Array<T> a(n, e);  // n << e
  a.resize(n, e);    // n << e

which is equivalent to n << e except that it calls error("Array too big")
rather than overflow if n << e would require more than 32 bits. If
compiled with -DDEBUG, then bounds are checked at run time.


ENCRYPTION

There is a class libzpaq::SHA256 with put(), result(), size(), and usize()
as in SHA1. result() returns a 32 byte SHA-256 hash. It is used by scrypt.

The libzpaq::AES_CTR class allows encryption in CTR mode with 128, 192,
or 256 bit keys. The public members are:

class AES_CTR {
public:
  AES_CTR(const char* key, int keylen, char* iv=0);
  void encrypt(U32 s0, U32 s1, U32 s2, U32 s3, unsigned char* ct);
  void encrypt(char* buf, int n, U64 offset);
};

The constructor initializes with a 16, 24, or 32 byte key. The length
is given by keylen. iv can be an 8 byte string or NULL. If not NULL
then iv0, iv1 are initialized with iv[0..7] in big-endian order, else 0.

encrypt(s0, s1, s2, s3, ct) encrypts a plaintext block divided into
4 32-bit words MSB first. The first byte of plaintext is the high 8
bits of s0. The output is to ct[16].

encrypt(buf, n, offset) encrypts or decrypts an n byte slice of a string
starting at offset. The i'th 16 byte block is encrypted by XOR with
the result (in ct) of encrypt(iv0, iv1, i>>32, i&0xffffffff, ct) starting
with i = 0. For example:

  AES_CTR a("a 128 bit key!!!", 16);
  char buf[500];             // some data 
  a.encrypt(buf, 100, 0);    // encrypt first 100 bytes
  a.encrypt(buf, 400, 100);  // encrypt next 400 bytes
  a.encrypt(buf, 500, 0);    // decrypt in one step

libzpaq::stretchKey(char* out, const char* in, const char* salt);

Generate a 32 byte key out[0..31] from key[0..31] and salt[0..31]
using scrypt(key, salt, N=16384, r=8, p=1). key[0..31] should be
the SHA-256 hash of the password. With these parameters, the function
uses 0.1 to 0.3 seconds and 16 MiB memory.
Scrypt is defined in http://www.tarsnap.com/scrypt/scrypt.pdf

void random(char* buf, int n);

Puts n cryptographic random bytes in buf[0..n-1], where the first
byte is never '7' or 'z' (start of a ZPAQ archive). For a pure
random string, discard the first byte.

Other classes and functions defined here are for internal use.
Use at your own risk.
*/

//////////////////////////////////////////////////////////////

#ifndef LIBZPAQ_H
#define LIBZPAQ_H

#ifndef DEBUG
#define NDEBUG 1
#endif
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace libzpaq {

// 1, 2, 4, 8 byte unsigned integers
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

// Tables for parsing ZPAQL source code
extern const char* compname[256];    // list of ZPAQL component types
extern const int compsize[256];      // number of bytes to encode a component
extern const char* opcodelist[272];  // list of ZPAQL instructions

// Callback for error handling
extern void error(const char* msg);

// Virtual base classes for input and output
// get() and put() must be overridden to read or write 1 byte.
// read() and write() may be overridden to read or write n bytes more
// efficiently than calling get() or put() n times.
class Reader {
public:
  virtual int get() = 0;  // should return 0..255, or -1 at EOF
  virtual int read(char* buf, int n); // read to buf[n], return no. read
  virtual ~Reader() {}
};

class Writer {
public:
  virtual void put(int c) = 0;  // should output low 8 bits of c
  virtual void write(const char* buf, int n);  // write buf[n]
  virtual ~Writer() {}
};

// Read 16 bit little-endian number
int toU16(const char* p);

// An Array of T is cleared and aligned on a 64 byte address
//   with no constructors called. No copy or assignment.
// Array<T> a(n, ex=0);  - creates n<<ex elements of type T
// a[i] - index
// a(i) - index mod n, n must be a power of 2
// a.size() - gets n
template <typename T>
class Array {
  T *data;     // user location of [0] on a 64 byte boundary
  size_t n;    // user size
  int offset;  // distance back in bytes to start of actual allocation
  void operator=(const Array&);  // no assignment
  Array(const Array&);  // no copy
public:
  Array(size_t sz=0, int ex=0): data(0), n(0), offset(0) {
    resize(sz, ex);} // [0..sz-1] = 0
  void resize(size_t sz, int ex=0); // change size, erase content to zeros
  ~Array() {resize(0);}  // free memory
  size_t size() const {return n;}  // get size
  int isize() const {return int(n);}  // get size as an int
  T& operator[](size_t i) {assert(n>0 && i<n); return data[i];}
  T& operator()(size_t i) {assert(n>0 && (n&(n-1))==0); return data[i&(n-1)];}
};

// Change size to sz<<ex elements of 0
template<typename T>
void Array<T>::resize(size_t sz, int ex) {
  assert(size_t(-1)>0);  // unsigned type?
  while (ex>0) {
    if (sz>sz*2) error("Array too big");
    sz*=2, --ex;
  }
  if (n>0) {
    assert(offset>0 && offset<=64);
    assert((char*)data-offset);
    ::free((char*)data-offset);
  }
  n=0;
  offset=0;
  if (sz==0) return;
  n=sz;
  const size_t nb=128+n*sizeof(T);  // test for overflow
  if (nb<=128 || (nb-128)/sizeof(T)!=n) n=0, error("Array too big");
  data=(T*)::calloc(nb, 1);
  if (!data) n=0, error("Out of memory");
  offset=64-(((char*)data-(char*)0)&63);
  assert(offset>0 && offset<=64);
  data=(T*)((char*)data+offset);
}

//////////////////////////// SHA1 ////////////////////////////

// For computing SHA-1 checksums
class SHA1 {
public:
  void put(int c) {  // hash 1 byte
    U32& r=w[U32(len)>>5&15];
    r=(r<<8)|(c&255);
    len+=8;
    if ((U32(len)&511)==0) process();
  }
  void write(const char* buf, int64_t n); // hash buf[0..n-1]
  double size() const {return len/8;}     // size in bytes
  uint64_t usize() const {return len/8;}  // size in bytes
  const char* result();  // get hash and reset
  SHA1() {init();}
private:
  void init();      // reset, but don't clear hbuf
  U64 len;          // length in bits
  U32 h[5];         // hash state
  U32 w[16];        // input buffer
  char hbuf[20];    // result
  void process();   // hash 1 block
};

//////////////////////////// SHA256 //////////////////////////

// For computing SHA-256 checksums
// http://en.wikipedia.org/wiki/SHA-2
class SHA256 {
public:
  void put(int c) {  // hash 1 byte
    unsigned& r=w[len0>>5&15];
    r=(r<<8)|(c&255);
    if (!(len0+=8)) ++len1;
    if ((len0&511)==0) process();
  }
  double size() const {return len0/8+len1*536870912.0;} // size in bytes
  uint64_t usize() const {return len0/8+(U64(len1)<<29);} //size in bytes
  const char* result();  // get hash and reset
  SHA256() {init();}
private:
  void init();           // reset, but don't clear hbuf
  unsigned len0, len1;   // length in bits (low, high)
  unsigned s[8];         // hash state
  unsigned w[16];        // input buffer
  char hbuf[32];         // result
  void process();        // hash 1 block
};

//////////////////////////// AES /////////////////////////////

// For encrypting with AES in CTR mode.
// The i'th 16 byte block is encrypted by XOR with AES(i)
// (i is big endian or MSB first, starting with 0).
class AES_CTR {
  U32 Te0[256], Te1[256], Te2[256], Te3[256], Te4[256]; // encryption tables
  U32 ek[60];  // round key
  int Nr;  // number of rounds (10, 12, 14 for AES 128, 192, 256)
  U32 iv0, iv1;  // first 8 bytes in CTR mode
public:
  AES_CTR(const char* key, int keylen, const char* iv=0);
    // Schedule: keylen is 16, 24, or 32, iv is 8 bytes or NULL
  void encrypt(U32 s0, U32 s1, U32 s2, U32 s3, unsigned char* ct);
  void encrypt(char* buf, int n, U64 offset);  // encrypt n bytes of buf
};

//////////////////////////// stretchKey //////////////////////

// Strengthen password pw[0..pwlen-1] and salt[0..saltlen-1]
// to produce key buf[0..buflen-1]. Uses O(n*r*p) time and 128*r*n bytes
// of memory. n must be a power of 2 and r <= 8.
void scrypt(const char* pw, int pwlen,
            const char* salt, int saltlen,
            int n, int r, int p, char* buf, int buflen);

// Generate a strong key out[0..31] key[0..31] and salt[0..31].
// Calls scrypt(key, 32, salt, 32, 16384, 8, 1, out, 32);
void stretchKey(char* out, const char* key, const char* salt);

//////////////////////////// random //////////////////////////

// Fill buf[0..n-1] with n cryptographic random bytes. The first
// byte is never '7' or 'z'.
void random(char* buf, int n);

//////////////////////////// ZPAQL ///////////////////////////

// Symbolic constants, instruction size, and names
typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE} CompType;
extern const int compsize[256];
class Decoder;  // forward

// A ZPAQL machine COMP+HCOMP or PCOMP.
class ZPAQL {
public:
  ZPAQL();
  ~ZPAQL();
  void clear();           // Free memory, erase program, reset machine state
  void inith();           // Initialize as HCOMP to run
  void initp();           // Initialize as PCOMP to run
  double memory();        // Return memory requirement in bytes
  void run(U32 input);    // Execute with input
  int read(Reader* in2);  // Read header
  bool write(Writer* out2, bool pp); // If pp write PCOMP else HCOMP header
  int step(U32 input, int mode);  // Trace execution (defined externally)

  Writer* output;         // Destination for OUT instruction, or 0 to suppress
  SHA1* sha1;             // Points to checksum computer
  U32 H(int i) {return h(i);}  // get element of h

  void flush();           // write outbuf[0..bufptr-1] to output and sha1
  void outc(int ch) {     // output byte ch (0..255) or -1 at EOS
    if (ch<0 || (outbuf[bufptr]=ch, ++bufptr==outbuf.isize())) flush();
  }

  // ZPAQ1 block header
  Array<U8> header;   // hsize[2] hh hm ph pm n COMP (guard) HCOMP (guard)
  int cend;           // COMP in header[7...cend-1]
  int hbegin, hend;   // HCOMP/PCOMP in header[hbegin...hend-1]

private:
  // Machine state for executing HCOMP
  Array<U8> m;        // memory array M for HCOMP
  Array<U32> h;       // hash array H for HCOMP
  Array<U32> r;       // 256 element register array
  Array<char> outbuf; // output buffer
  int bufptr;         // number of bytes in outbuf
  U32 a, b, c, d;     // machine registers
  int f;              // condition flag
  int pc;             // program counter
  int rcode_size;     // length of rcode
  U8* rcode;          // JIT code for run()

  // Support code
  int assemble();  // put JIT code in rcode
  void init(int hbits, int mbits);  // initialize H and M sizes
  int execute();  // interpret 1 instruction, return 0 after HALT, else 1
  void run0(U32 input);  // default run() if not JIT
  void div(U32 x) {if (x) a/=x; else a=0;}
  void mod(U32 x) {if (x) a%=x; else a=0;}
  void swap(U32& x) {a^=x; x^=a; a^=x;}
  void swap(U8& x)  {a^=x; x^=a; a^=x;}
  void err();  // exit with run time error
};

///////////////////////// Component //////////////////////////

// A Component is a context model, indirect context model, match model,
// fixed weight mixer, adaptive 2 input mixer without or with current
// partial byte as context, adaptive m input mixer (without or with),
// or SSE (without or with).

struct Component {
  size_t limit;   // max count for cm
  size_t cxt;     // saved context
  size_t a, b, c; // multi-purpose variables
  Array<U32> cm;  // cm[cxt] -> p in bits 31..10, n in 9..0; MATCH index
  Array<U8> ht;   // ICM/ISSE hash table[0..size1][0..15] and MATCH buf
  Array<U16> a16; // MIX weights
  void init();    // initialize to all 0
  Component() {init();}
};

////////////////////////// StateTable ////////////////////////

// Next state table
class StateTable {
public:
  U8 ns[1024]; // state*4 -> next state if 0, if 1, n0, n1
  int next(int state, int y) {  // next state for bit y
    assert(state>=0 && state<256);
    assert(y>=0 && y<4);
    return ns[state*4+y];
  }
  int cminit(int state) {  // initial probability of 1 * 2^23
    assert(state>=0 && state<256);
    return ((ns[state*4+3]*2+1)<<22)/(ns[state*4+2]+ns[state*4+3]+1);
  }
  StateTable();
};

///////////////////////// Predictor //////////////////////////

// A predictor guesses the next bit
class Predictor {
public:
  Predictor(ZPAQL&);
  ~Predictor();
  void init();          // build model
  int predict();        // probability that next bit is a 1 (0..4095)
  void update(int y);   // train on bit y (0..1)
  int stat(int);        // Defined externally
  bool isModeled() {    // n>0 components?
    assert(z.header.isize()>6);
    return z.header[6]!=0;
  }
private:

  // Predictor state
  int c8;               // last 0...7 bits.
  int hmap4;            // c8 split into nibbles
  int p[256];           // predictions
  U32 h[256];           // unrolled copy of z.h
  ZPAQL& z;             // VM to compute context hashes, includes H, n
  Component comp[256];  // the model, includes P
  bool initTables;      // are tables initialized?

  // Modeling support functions
  int predict0();       // default
  void update0(int y);  // default
  int dt2k[256];        // division table for match: dt2k[i] = 2^12/i
  int dt[1024];         // division table for cm: dt[i] = 2^16/(i+1.5)
  U16 squasht[4096];    // squash() lookup table
  short stretcht[32768];// stretch() lookup table
  StateTable st;        // next, cminit functions
  U8* pcode;            // JIT code for predict() and update()
  int pcode_size;       // length of pcode

  // reduce prediction error in cr.cm
  void train(Component& cr, int y) {
    assert(y==0 || y==1);
    U32& pn=cr.cm(cr.cxt);
    U32 count=pn&0x3ff;
    int error=y*32767-(cr.cm(cr.cxt)>>17);
    pn+=(error*dt[count]&-1024)+(count<cr.limit);
  }

  // x -> floor(32768/(1+exp(-x/64)))
  int squash(int x) {
    assert(initTables);
    assert(x>=-2048 && x<=2047);
    return squasht[x+2048];
  }

  // x -> round(64*log((x+0.5)/(32767.5-x))), approx inverse of squash
  int stretch(int x) {
    assert(initTables);
    assert(x>=0 && x<=32767);
    return stretcht[x];
  }

  // bound x to a 12 bit signed int
  int clamp2k(int x) {
    if (x<-2048) return -2048;
    else if (x>2047) return 2047;
    else return x;
  }

  // bound x to a 20 bit signed int
  int clamp512k(int x) {
    if (x<-(1<<19)) return -(1<<19);
    else if (x>=(1<<19)) return (1<<19)-1;
    else return x;
  }

  // Get cxt in ht, creating a new row if needed
  size_t find(Array<U8>& ht, int sizebits, U32 cxt);

  // Put JIT code in pcode
  int assemble_p();
};

//////////////////////////// Decoder /////////////////////////

// Decoder decompresses using an arithmetic code
class Decoder: public Reader {
public:
  Reader* in;        // destination
  Decoder(ZPAQL& z);
  int decompress();  // return a byte or EOF
  int skip();        // skip to the end of the segment, return next byte
  void init();       // initialize at start of block
  int stat(int x) {return pr.stat(x);}
  int get() {        // return 1 byte of buffered input or EOF
    if (rpos==wpos) {
      rpos=0;
      wpos=in ? in->read(&buf[0], BUFSIZE) : 0;
      assert(wpos<=BUFSIZE);
    }
    return rpos<wpos ? U8(buf[rpos++]) : -1;
  }
  int buffered() {return wpos-rpos;}  // how far read ahead?
private:
  U32 low, high;     // range
  U32 curr;          // last 4 bytes of archive or remaining bytes in subblock
  U32 rpos, wpos;    // read, write position in buf
  Predictor pr;      // to get p
  enum {BUFSIZE=1<<16};
  Array<char> buf;   // input buffer of size BUFSIZE bytes
  int decode(int p); // return decoded bit (0..1) with prob. p (0..65535)
};

/////////////////////////// PostProcessor ////////////////////

class PostProcessor {
  int state;   // input parse state: 0=INIT, 1=PASS, 2..4=loading, 5=POST
  int hsize;   // header size
  int ph, pm;  // sizes of H and M in z
public:
  ZPAQL z;     // holds PCOMP
  PostProcessor(): state(0), hsize(0), ph(0), pm(0) {}
  void init(int h, int m);  // ph, pm sizes of H and M
  int write(int c);  // Input a byte, return state
  int getState() const {return state;}
  void setOutput(Writer* out) {z.output=out;}
  void setSHA1(SHA1* sha1ptr) {z.sha1=sha1ptr;}
};

//////////////////////// Decompresser ////////////////////////

// For decompression and listing archive contents
class Decompresser {
public:
  Decompresser(): z(), dec(z), pp(), state(BLOCK), decode_state(FIRSTSEG) {}
  void setInput(Reader* in) {dec.in=in;}
  bool findBlock(double* memptr = 0);
  void hcomp(Writer* out2) {z.write(out2, false);}
  bool findFilename(Writer* = 0);
  void readComment(Writer* = 0);
  void setOutput(Writer* out) {pp.setOutput(out);}
  void setSHA1(SHA1* sha1ptr) {pp.setSHA1(sha1ptr);}
  bool decompress(int n = -1);  // n bytes, -1=all, return true until done
  bool pcomp(Writer* out2) {return pp.z.write(out2, true);}
  void readSegmentEnd(char* sha1string = 0);
  int stat(int x) {return dec.stat(x);}
  int buffered() {return dec.buffered();}
private:
  ZPAQL z;
  Decoder dec;
  PostProcessor pp;
  enum {BLOCK, FILENAME, COMMENT, DATA, SEGEND} state;  // expected next
  enum {FIRSTSEG, SEG, SKIP} decode_state;  // which segment in block?
};

/////////////////////////// decompress() /////////////////////

void decompress(Reader* in, Writer* out);

//////////////////////////// Encoder /////////////////////////

// Encoder compresses using an arithmetic code
class Encoder {
public:
  Encoder(ZPAQL& z, int size=0):
    out(0), low(1), high(0xFFFFFFFF), pr(z) {}
  void init();
  void compress(int c);  // c is 0..255 or EOF
  int stat(int x) {return pr.stat(x);}
  Writer* out;  // destination
private:
  U32 low, high; // range
  Predictor pr;  // to get p
  Array<char> buf; // unmodeled input
  void encode(int y, int p); // encode bit y (0..1) with prob. p (0..65535)
};

//////////////////////////// Compiler ////////////////////////

// Input ZPAQL source code with args and store the compiled code
// in hz and pz and write pcomp_cmd to out2.

class Compiler {
public:
  Compiler(const char* in, int* args, ZPAQL& hz, ZPAQL& pz, Writer* out2);
private:
  const char* in;  // ZPAQL source code
  int* args;       // Array of up to 9 args, default NULL = all 0
  ZPAQL& hz;       // Output of COMP and HCOMP sections
  ZPAQL& pz;       // Output of PCOMP section
  Writer* out2;    // Output ... of "PCOMP ... ;"
  int line;        // Input line number for reporting errors
  int state;       // parse state: 0=space -1=word >0 (nest level)

  // Symbolic constants
  typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,
    JT=39,JF=47,JMP=63,LJ=255,
    POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,
    WHILE,UNTIL,FOREVER,IFL,IFNOTL,ELSEL,SEMICOLON} CompType;

  void syntaxError(const char* msg, const char* expected=0); // error()
  void next();                     // advance in to next token
  bool matchToken(const char* tok);// in==token?
  int rtoken(int low, int high);   // return token which must be in range
  int rtoken(const char* list[]);  // return token by position in list
  void rtoken(const char* s);      // return token which must be s
  int compile_comp(ZPAQL& z);      // compile either HCOMP or PCOMP

  // Stack of n elements
  class Stack {
    libzpaq::Array<U16> s;
    size_t top;
  public:
    Stack(int n): s(n), top(0) {}
    void push(const U16& x) {
      if (top>=s.size()) error("IF or DO nested too deep");
      s[top++]=x;
    }
    U16 pop() {
      if (top<=0) error("unmatched IF or DO");
      return s[--top];
    }
  };

  Stack if_stack, do_stack;
};

//////////////////////// Compressor //////////////////////////

class Compressor {
public:
  Compressor(): enc(z), in(0), state(INIT), verify(false) {}
  void setOutput(Writer* out) {enc.out=out;}
  void writeTag();
  void startBlock(int level);  // level=1,2,3
  void startBlock(const char* hcomp);     // ZPAQL byte code
  void startBlock(const char* config,     // ZPAQL source code
                  int* args,              // NULL or int[9] arguments
                  Writer* pcomp_cmd = 0); // retrieve preprocessor command
  void setVerify(bool v) {verify = v;}    // check postprocessing?
  void hcomp(Writer* out2) {z.write(out2, false);}
  bool pcomp(Writer* out2) {return pz.write(out2, true);}
  void startSegment(const char* filename = 0, const char* comment = 0);
  void setInput(Reader* i) {in=i;}
  void postProcess(const char* pcomp = 0, int len = 0);  // byte code
  bool compress(int n = -1);  // n bytes, -1=all, return true until done
  void endSegment(const char* sha1string = 0);
  char* endSegmentChecksum(int64_t* size = 0, bool dosha1=true);
  int64_t getSize() {return sha1.usize();}
  const char* getChecksum() {return sha1.result();}
  void endBlock();
  int stat(int x) {return enc.stat(x);}
private:
  ZPAQL z, pz;  // model and test postprocessor
  Encoder enc;  // arithmetic encoder containing predictor
  Reader* in;   // input source
  SHA1 sha1;    // to test pz output
  char sha1result[20];  // sha1 output
  enum {INIT, BLOCK1, SEG1, BLOCK2, SEG2} state;
  bool verify;  // if true then test by postprocessing
};

/////////////////////////// StringBuffer /////////////////////

// For (de)compressing to/from a string. Writing appends bytes
// which can be later read.
class StringBuffer: public libzpaq::Reader, public libzpaq::Writer {
  unsigned char* p;  // allocated memory, not NUL terminated, may be NULL
  size_t al;         // number of bytes allocated, 0 iff p is NULL
  size_t wpos;       // index of next byte to write, wpos <= al
  size_t rpos;       // index of next byte to read, rpos < wpos or return EOF.
  size_t limit;      // max size, default = -1
  const size_t init; // initial size on first use after reset

  // Increase capacity to a without changing size
  void reserve(size_t a) {
    assert(!al==!p);
    if (a<=al) return;
    unsigned char* q=0;
    if (a>0) q=(unsigned char*)(p ? realloc(p, a) : malloc(a));
    if (a>0 && !q) error("Out of memory");
    p=q;
    al=a;
  }

  // Enlarge al to make room to write at least n bytes.
  void lengthen(size_t n) {
    assert(wpos<=al);
    if (wpos+n>limit || wpos+n<wpos) error("StringBuffer overflow");
    if (wpos+n<=al) return;
    size_t a=al;
    while (wpos+n>=a) a=a*2+init;
    reserve(a);
  }

  // No assignment or copy
  void operator=(const StringBuffer&);
  StringBuffer(const StringBuffer&);

public:

  // Direct access to data
  unsigned char* data() {assert(p || wpos==0); return p;}

  // Allocate no memory initially
  StringBuffer(size_t n=0):
      p(0), al(0), wpos(0), rpos(0), limit(size_t(-1)), init(n>128?n:128) {}

  // Set output limit
  void setLimit(size_t n) {limit=n;}

  // Free memory
  ~StringBuffer() {if (p) free(p);}

  // Return number of bytes written.
  size_t size() const {return wpos;}

  // Return number of bytes left to read
  size_t remaining() const {return wpos-rpos;}

  // Reset size to 0 and free memory.
  void reset() {
    if (p) free(p);
    p=0;
    al=rpos=wpos=0;
  }

  // Write a single byte.
  void put(int c) {  // write 1 byte
    lengthen(1);
    assert(p);
    assert(wpos<al);
    p[wpos++]=c;
    assert(wpos<=al);
  }

  // Write buf[0..n-1]. If buf is NULL then advance write pointer only.
  void write(const char* buf, int n) {
    if (n<1) return;
    lengthen(n);
    assert(p);
    assert(wpos+n<=al);
    if (buf) memcpy(p+wpos, buf, n);
    wpos+=n;
  }

  // Read a single byte. Return EOF (-1) at end.
  int get() {
    assert(rpos<=wpos);
    assert(rpos==wpos || p);
    return rpos<wpos ? p[rpos++] : -1;
  }

  // Read up to n bytes into buf[0..] or fewer if EOF is first.
  // Return the number of bytes actually read.
  // If buf is NULL then advance read pointer without reading.
  int read(char* buf, int n) {
    assert(rpos<=wpos);
    assert(wpos<=al);
    assert(!al==!p);
    if (rpos+n>wpos) n=wpos-rpos;
    if (n>0 && buf) memcpy(buf, p+rpos, n);
    rpos+=n;
    return n;
  }

  // Return the entire string as a read-only array.
  const char* c_str() const {return (const char*)p;}

  // Truncate the string to size i.
  void resize(size_t i) {
    wpos=i;
    if (rpos>wpos) rpos=wpos;
  }

  // Swap efficiently (init is not swapped)
  void swap(StringBuffer& s) {
    std::swap(p, s.p);
    std::swap(al, s.al);
    std::swap(wpos, s.wpos);
    std::swap(rpos, s.rpos);
    std::swap(limit, s.limit);
  }
};

/////////////////////////// compress() ///////////////////////

// Compress in to out in multiple blocks. Default method is "14,128,0"
// Default filename is "". Comment is appended to input size.
// dosha1 means save the SHA-1 checksum.
void compress(Reader* in, Writer* out, const char* method,
     const char* filename=0, const char* comment=0, bool dosha1=true);

// Same as compress() but output is 1 block, ignoring block size parameter.
void compressBlock(StringBuffer* in, Writer* out, const char* method,
     const char* filename=0, const char* comment=0, bool dosha1=true);

}  // namespace libzpaq

#endif  // LIBZPAQ_H

/* libzpaq.cpp - LIBZPAQ Version 7.15 implementation - Aug. 17, 2016.

  libdivsufsort.c for divsufsort 2.00, included within, is
  (C) 2003-2008 Yuta Mori, all rights reserved.
  It is released under the MIT license as described in the comments
  at the beginning of that section.

  Some of the code for AES is from libtomcrypt 1.17 by Tom St. Denis
  and is public domain.

  The Salsa20/8 code for Scrypt is by D. Bernstein and is public domain.

  All of the remaining software is provided as-is, with no warranty.
  I, Matt Mahoney, release this software into
  the public domain. This applies worldwide.
  In some countries this may not be legally possible; if so:
  I grant anyone the right to use this software for any purpose,
  without any conditions, unless such conditions are required by law.

LIBZPAQ is a C++ library for compression and decompression of data
conforming to the ZPAQ level 2 standard. See http://mattmahoney.net/zpaq/
See libzpaq.h for additional documentation.
*/

/* libzpaq.h - LIBZPAQ Version 7.12 header - Apr. 19, 2016.

  This software is provided as-is, with no warranty.
  I, Matt Mahoney, release this software into
  the public domain.   This applies worldwide.
  In some countries this may not be legally possible; if so:
  I grant anyone the right to use this software for any purpose,
  without any conditions, unless such conditions are required by law.

LIBZPAQ is a C++ library providing data compression and decompression
services using the ZPAQ level 2 format as described in
http://mattmahoney.net/zpaq/

An application wishing to use these services should #include "libzpaq.h"
and link to libzpaq.cpp (and advapi32.lib in Windows/VC++).
libzpaq recognizes the following options:

  -DDEBUG   Turn on assertion checks (slower).
  -DNOJIT   Don't assume x86-32 or x86-64 with SSE2 (slower).
  -Dunix    Without -DNOJIT, assume Unix (Linux, Mac) rather than Windows.

The application must provide an error handling function and derived
implementations of two abstract classes, Reader and Writer,
specifying the input and output byte streams. For example, to compress
from stdin to stdout (assuming binary I/O as in Linux):

  #include "libzpaq.h"
  #include <stdio.h>
  #include <stdlib.h>

  void libzpaq::error(const char* msg) {  // print message and exit
    fprintf(stderr, "Oops: %s\n", msg);
    exit(1);
  }

  class In: public libzpaq::Reader {
  public:
    int get() {return getchar();}  // returns byte 0..255 or -1 at EOF
  } in;

  class Out: public libzpaq::Writer {
  public:
    void put(int c) {putchar(c);}  // writes 1 byte 0..255
  } out;

  int main() {
    libzpaq::compress(&in, &out, "1");  // "0".."5" = faster..better
  }

Or to decompress:

    libzpaq::decompress(&in, &out);

The function error() will be called with an English language message
in case of an unrecoverable error such as badly formatted compressed
input data or running out of memory. error() should not return.
In a multi-threaded application where ZPAQ blocks are being decompressed
in separate threads, error() should exit the thread, but other threads
may continue. Blocks are independent and libzpaq is thread safe.

Reader and Writer provide default implementations of read() and write()
for block I/O. You may override these with your own versions, which
might be faster. The default is to call get() or put() the appropriate
number of times. For example:

  // Read n bytes into buf[0..n-1] or to EOF, whichever is first.
  // Return the number of bytes actually read.
  int In::read(char* buf, int n) {return fread(buf, 1, n, stdin);}

  // Write buf[0..n-1]
  void Out::write(char* buf, int n) {fwrite(buf, 1, n, stdout);}

By default, compress() divides the input into blocks with one segment
each. The segment filename field is empty. The comment field of each
block is the uncompressed size as a decimal string. The checksum
is saved. To override:

  compress(&in, &out, "1", "filename", "comment", false);

If the filename is not NULL then it is saved in the first block only.
If the comment is not NULL then a space and the comment are appended
to the decimal size in the first block only. The comment would normally
be the date and attributes like "20141231235959 w32", or "jDC\x01" for
a journaling archive as described in the ZPAQ specification.

The method string has the general form of a concatenation of single
character commands each possibly followed by a list of decimal
numeric arguments separated by commas or periods:

  {012345xciawmst}[N1[{.,}N2]...]...

For example "1" or "14,128,0" or "x6.3ci1m".

Only the first command can be a digit 0..5. If it is, then it selects
a compression level and the other commands are ignored. Otherwise,
if it is "x" then the arguments and remaining commands describe
the compression method. Any other letter as the first command is
interpreted the same as "x". 

Higher compression levels are slower but compress better. "1" is
good for most purposes. "0" does not compress. "2" compresses slower
but decompression is just as fast as 1. "3", "4", and "5" also
decompress slower. The numeric arguments are as follows:

  N1: 0..11 = block size of at most 2^N1 MiB - 4096 bytes (default 4).
  N2: 0..255 = estimated ease of compression (default 128).
  N3: 0..3 = data type. 1 = text, 2 = exe, 3 = both (default 0).

For example, "14" or "54" divide the input in 16 MB blocks which
are compressed independently. N2 and N3 are hints to the compressor
based on analysis of the input data. N2 is 0 if the data is random
or 255 if the data is easily compressed (for example, all zero bytes).
Most compression methods will simply store random data with no
compression. The default is "14,128,0".

If the first command is "x" then the string describes the exact
compression method. The arguments to "x" describe the pre/post
processing (LZ77, BWT, E8E9), and remaining commands describe the
context model, if any, of the transformed data. The arguments to "x" are:

  N1: 0..11 = block size as before.
  N2: 0..7: 0=none, 1=packed LZ77, 2=LZ77, 3=BWT, 4..7 = 0..3 + E8E9.
  N3: 4..63: LZ77 min match.
  N4: LZ77 secondary match to try first or 0 to skip.
  N5: LZ77 log search depth.
  N6: LZ77 log hash table size, or N1+21 to use a suffix array.
  N7: LZ77 lookahead.

N2 selects the basic transform applied before context modeling.
N2 = 0 does not transform the input. N2 = 1 selects LZ77 encoding
of literals strings and matches using bit-packed codes. It is normally
not used with a context model. N2 = 2 selects byte aligned LZ77, which
compresses worse by itself but better than 1 when a context model is
used. It uses single bytes to encode either a literal of length 1..64
or a match of length N3..N3+63 with a 2, 3, or 4 byte offset.

N2 = 3 selects a Burrows-Wheeler transform, in which the input is
sorted by right-context. This does not compress by itself but makes
the data more compressible using a low order, adaptive context model.
BWT requires 4 times the block size in additional memory for both
compression and decompression.

N2 = 4..7 are the same as 0..3 except that a E8E9 transform is first applied
to improve the compression of x86 code usually found .exe and .dll files.
It scans the input block backward for 5 byte strings of the form
{E8|E9 xx xx xx 00|FF} and adds the offset from the start of the
block to the middle 3 bytes interpreted as a little-endian (LSB first)
number (mod 2^24). E8 and E9 are the CALL and JMP instructions, followed
by a 32 bit relative offset.

N3..N7 apply only to LZ77. For either type, it searches for matches
by hashing the next N4 bytes, and then the next N3 bytes, and looking
up each of the hashes at 2^N5 locations in a table with 2^N6 entries.
Of those, it picks the longest match, or closest in case of a tie.
If no match is at least N3, then a literal is encoded instead. If N5
is 0 then only one hash is computed, which is faster but does not
compress as well. Typical good values for fast compression are
"x4.1.5.0.3.22" which means 16 MiB blocks, packed LZ77, mininum match
length 5, no secondary match, search depth 2^3 = 8, and 2^22 = 4M
hash table (using 16 MiB memory).

The hash table requires 4 x 2^N6 bytes of memory. If N6 = N1+21, then
matches are found using a suffix array and inverse suffix array using
2.25 x 2^N6 bytes (4.5 x block size). This finds better matches but
takes longer to compute the suffix array (SA). The matches are found by
searching forward and backward in the SA 2^N5 in each direction up
to the first earlier match, and picking the longer of the two.
Good values are "x4.1.4.0.8.25". The secondary match N4 has no effect.

N7 is the lookahead. It looks for matches of length at least N4+N7
when using a hash table or N3+N7 for a SA, but allows the first N7
bytes not to match and be coded as literals if this results in
a significantly longer match. Values higher than 1 are rarely effective.
The default is 0.

All subsequent commands after "x" describe a context model. A model
consists of a set of components that output a bit prediction, taking
a context and possibly earlier predictions as input. The final prediction
is arithmetic coded. The component types are:

  c = CM or ICM (context model or indirect context model).
  i = ISSE chain (indirect secondary symbol estimator).
  a = MATCH.
  w = word model (ICM-ISSE chain with whole word contexts).
  m = MIX.
  s = SSE (secondary symbol estimator).
  t = MIX2 (2 input MIX).

For example, "x4.3ci1" describes a BWT followed by an order 0 CM
and order 1 ISSE, which is used for level 3 text compression. The
parameters to "c" (default all 0) are as follows:

  N1: 0 = ICM, 1..256 CM with faster..slower adaptation, +1000 halves memory.
  N2: 1..255 = offset mod N2, 1000..1255 = offset to last N2-1000 byte.
  N3: 0..255 = order 0 context mask, 256..511 mixes LZ77 parse state.
  N4...: 0..255 order 1... context masks. 1000... skips N4-1000 bytes.

Most components use no more memory than the block size, depending on
the number of context bits, but it is possible to select less memory
and lose compression.

A CM inputs a context hash and outputs a prediction from a table.
The table entry is then updated by adjusting in the direction of the
actual bit. The adjustment is 1/count, where the maximum count is 4 x N1.
Larger values are best for stationary data. Smaller values adapt faster
to changing data.

If N1 is 0 then c selects an ICM. An ICM maps a context to a bit history
(8 bit state), and then to slow adapting prediction. It is generally
better than a CM on most nonstationary data.

The context for a CM or ICM is a hash of all selected contexts: a
cyclic counter (N2 = 1..255), the distance from the last occurrence
of some byte value (N2 = 1000..1255), and the masked history of the
last 64K bytes ANDED with N3, N4... For example, "c0.0.255.255.255" is
an order 3 ICM. "C0.1010.255" is an order 1 context hashed together
with the column number in a text file (distance to the last linefeed,
ASCII 10). "c256.0.255.1511.255" is a stationary grayscale 512 byte
wide image model using the two previous neighboring pixels as context.
"c0.0.511.255" is an order 1 model for LZ77, which helps compress
literal strings. The LZ77 state context applies only to byte aligned
LZ77 (type 2 or 6).

The parameters to "i" (ISSE chain) are the initial context length and
subsequent increments for a chain connected to an existing earlier component.
For example, "ci1.1.2" specifies an ICM (order 0) followed by a chain
of 3 ISSE with orders 1, 2, and 4. An ISSE maps a context to a bit
history like an ISSE, but uses the history to select a pair of weights
to mix the input prediction with a constant 1, thus performing the
mapping q' := w1 x q + w2 in the logistic domain (q = log p/(1-p)).
The mixer is then updated by adjusting the weights to improve the
prediction. High order ISSE chains (like "x4.0ci1.1.1.1.2") and BWT
followed by a low order chain (like "x4.3ci1") both provide
excellent general purpose compression.

A MATCH ("a") keeps a rotating history buffer and a hash table to look
up the previous occurrence of the current context hash and predicts
whatever bit came next. The parameters are:

  N1 = hash multiplier, default 24.
  N2 = halve buffer size, default 0 = same size as input block.
  N3 = halve hash table size, default 0 = block size / 4.

For example, "x4.0m24.1.1" selects a 16 MiB block size, 8 MiB match
buffer size, and 2M hash table size (using 8 MiB at 4 bytes per entry).
The hash is computed as hash := hash x N1 + next_byte + 1 (mod hash table
size). Thus, N1 = 12 selects a higher order context, and N1 = 48 selects a
lower order.

A word model ('w") is an ICM-ISSE chain of length N1 (orders 0..N1-1)
in which the contexts are whole words. A word is defined as the set
of characters in the range N2..N2+N3-1 after ANDing with N4. The context
is hashed using multiplier N5. Memory is halved by N6. The default is
"w1.65.26.223.20.0" which is a chain of length 1 (ICM only), where words
are in range 65 ('A') to 65+26-1 ('Z') after ANDing with 223 (which
converts to upper case). The hash multiplier is 20, which has the
effect of shifting the high 2 bits out of the hash. The memory usage
of each component is the same as the block size.

A MIX ("m") performs the weighted average of all previous component
predictions. The weights are then adjusted to improve the prediction
by favoring the most accurate components. N1 selects the number of
context bits (not hashed) to select a set of weights. N2 is the
learning rate (around 16..32 works well). The default is "m8.24"
which selects the previously modeled bits of the current byte as
context. When N1 is not a multiple of 8, it selects the most significant
bits of the oldest byte.

A SSE ("s") adjusts the previous prediction like an ISSE, but uses
a direct lookup table of the quantized and interpolated input prediction
and a direct (not hashed) N1-bit context. The adjustment is 1/count where
the count is allowed to range from N2 to 4 x N3. The default
is "s8.32.255".

A MIX2 ("t") is a MIX but mixing only the last 2 components. The
default is "t8.24" where the meaning is the same as "m".

For example, a good model for text is "x6.0ci1.1.1.1.2aw2mm16tst"
which selects 2^6 = 64 MiB blocks, no preprocessing, an order 0 ICM,
an ISSE chain with orders 1, 2, 3, 4, 6, a MATCH, an order 0-1 word
ICM-ISSE chain, two mixers with 0 and 1 byte contexts, whose outputs are
mixed by a MIX2. The MIX2 output is adjusted by a SSE, and finally
the SSE input and outputs are mixed again for the final bit prediction.


COMPRESSBLOCK

CompressBlock() takes the same arguments as compress() except that
the input is a StringBuffer instead of a Reader. The output is always
a single block, regardless of the N1 (block size) argument in the method.

  void compressBlock(StringBuffer* in, Writer* out, const char* method,
                     const char* filename=0, const char* comment=0,
                     bool compute_sha1=false);

A StringBuffer is both a Reader and a Writer, but also allows random
memory access. It provides convenient and efficient storage when the
input size is unknown.

  class StringBuffer: public libzpaq::Reader, public libzpaq::Writer {
  public:
    StringBuffer(size_t n=0);     // initial allocation after first use
    ~StringBuffer();
    int get();                    // read 1 byte or EOF from memory
    int read(char* buf, int n);   // read n bytes
    void put(int c);              // write 1 byte to memory
    void write(const char* buf, int n);  // write n bytes
    const char* c_str() const;    // read-only access to written data
    unsigned char* data();        // read-write access
    size_t size() const;          // number of bytes written
    size_t remaining() const;     // number of bytes to read until EOF
    void setLimit(size_t n);      // set maximum write size
    void reset();                 // discard contents and free memory
    void resize(size_t n);        // truncate to n bytes
    void swap(StringBuffer& s);   // exchange contents efficiently
  };

The constructor sets the inital allocation size after the first
write to n or 128, whichever is larger. Initially, no memory is allocated.
The allocated size is always n x (2^k - 1), for example
128 x (1, 3, 7, 15, 31...).

put() and write() append 1 or n bytes, allocating memory as needed.
buf can be NULL and the StringBuffer will be enlarged by n.
get() and read() read 1 or up to n bytes. get() returns EOF if you
attempt to read past the end of written data. read() returns less
than n if it reaches EOF first, or 0 at EOF.

size() is the number of bytes written, which does not change when
data is read. remaining() is the number of bytes left to read
before EOF.

c_str() provides read-only access to the data. It is not NUL terminated.
data() provides read-write access. Either may return NULL if size()
is 0. write(), put(), reset(), swap(), and the destructor may
invalidate saved pointers.

setLimit() sets a maximum size. It will call error() if you try to
write past it. The default is -1 or no limit.

reset() sets the size to 0 and frees memory. resize() sets the size
to n by moving the write pointer, but does not allocate or free memory.
Moving the pointer forward does not overwrite the previous contents
in between. The write pointer can be moved past the end of allocated
memory, and the next put() or write() will allocate as needed. If the
write pointer is moved back before the read pointer, then remaining()
is set to 0.

swap() swaps 2 StringBuffers efficiently, but does not change their
initial allocations.


DECOMPRESSER

decompress() will decompress any valid ZPAQ stream, which may contain
multiple blocks with multiple segments each. It will ignore filenames,
comments, and checksums. You need the Decompresser class if you want to
do something other than decompress all of the data serially to a single
file. To decompress individual blocks and segments and retrieve the
filenames, comments, data, and hashes of each segment (in exactly this
order):

  libzpaq::Decompresser d;               // to decompress
  libzpaq::SHA1 sha1;                    // to verify output hashes
  double memory;                         // bytes required to decompress
  Out filename, comment;
  char sha1out[21];
  d.setInput(&in);
  while (d.findBlock(&memory)) {         // default is NULL
    while (d.findFilename(&filename)) {  // default is NULL
      d.readComment(&comment);           // default is NULL
      d.setOutput(&out);                 // if omitted or NULL, discard output
      d.setSHA1(&sha1);                  // optional
      while (d.decompress(1000));        // bytes to decode, default is all
      d.readSegmentEnd(sha1out);         // {0} or {1,hash[20]}
      if (sha1out[0]==1 && memcmp(sha1.result(), sha1out+1, 20))
        error("checksum error");
    }
  }

findBlock() scans the input for the next ZPAQ block and returns true
if found. It optionally sets memory to the approximate number of bytes
that it will allocate at the first call to decompress().

findFilename() finds the next segment and returns false if there are
no more in the current block. It optionally writes the saved filename.

readComment() optionally writes the comment. It must be called
after reading the filename and before decompressing.

setSHA1() specifies an SHA1 object for computing a hash of the segment.
It may be omitted if you do not want to compute a hash.

decompress() decodes the requested number of bytes, postprocesses them,
and writes them to out. For the 3 built in compression levels, this
is the same as the number of bytes output, but it may be different if
postprocessing was used. It returns true until there is no more data
to decompress in the current segment. The default (-1) is to decompress the
whole segment.

readSegmentEnd() skips any remaining data not yet decompressed in the
segment and writes 21 bytes, either a 0 if no hash was saved, 
or a 1 followed by the 20 byte saved hash. If any data is skipped,
then all data in the remaining segments in the current block must
also be skipped.


SHA1

The SHA1 object computes SHA-1 cryptographic hashes. It is safe to
assume that two inputs with the same hash are identical. For example:

  libzpaq::SHA1 sha1;
  int ch;
  while ((ch=getchar())!=EOF)
    sha1.put(ch);
  printf("Size is %1.0f or %1.0f bytes\n", sha1.size(), double(sha1.usize()));

size() returns the number of bytes read as a double, and usize() as a
64 bit integer. result() returns a pointer to the 20 byte hash and
resets the size to 0. The hash (not just the pointer) should be copied
before the next call to result() if you want to save it. You can also
call sha1.write(buffer, n) to hash n bytes of char* buffer.


COMPRESSOR

A Compressor object allows greater control over the compressed data.
In particular you can specify the compression algorithm in ZPAQL to
specify methods not possible using compress() or compressBlock(). You
can create blocks with multiple segments specifying different files,
or compress streams of unlimited size to a single block when the
input size is not known.

  libzpaq::Compressor c;
  for (int i=0; i<num_blocks; ++i) {
    c.setOutput(&out);              // if omitted or NULL, discard output
    c.writeTag();                   // optional locator tag
    c.startBlock(2);                // compression level 1, 2, or 3
    for (int j=0; j<num_segments; ++j) {
      c.startSegment("filename", "comment");  // default NULL = empty
      c.setInput(&in);
      while (c.compress(1000));     // bytes to compress, default -1 = all
      c.endSegment(sha1.result());  // default NULL = don't save checksum
    }
    c.endBlock();
  }

Input and output can be set anywhere before the first input and output
operation, respectively. Output may be changed any time.

writeTag() outputs a 13 byte string that allows decompress() to scan
for blocks that don't occur immediately, such as searching from the
start of a self extracting archive.

startBlock() specifies either a compression level (1, 2, 3), or a ZPAQL
program, described below. It does not work with the fast method type
arguments to compress() or compressBlock(). Levels 1, 2, and 3 correspond
approximately to "3", "4", and "5". Any preprocessing must be done
by the application before input to the Compressor.

StartSegment() starts a segment. An empty or NULL filename continues
the previous file. The comment normally contains the uncompressed size
of the segment as a decimal string, but it is allowed to omit it.

compress() will read the requested number of bytes or until in.get()
returns EOF (-1), whichever comes first, and return true if there is
more data to decompress. If the argument is omitted or -1, then it will
read to EOF and return false.

endSegment() writes a provided SHA-1 cryptographic hash checksum of the
input segment before any preprocessing. It may be omitted.


ZPAQL

ZPAQ supports arbitrary compression algorithms in addition to the
built in levels. For example, method "x4.0c0.0.255.255i4" compression could
alternatively be specified using the ZPAQL language description of the
compression algorithm:

  int args[9]={0}
  c.startBlock(
    "(min.cfg - equivalent to level 1) "
    "comp 1 2 0 0 2 (log array sizes hh,hm,ph,pm and number of components n) "
    "  0 icm 16    (order 2 indirect context model using 4 MB memory) "
    "  1 isse 19 0 (order 4 indirect secondary symbol estimator, 32 MB) "
    "hcomp (context computation, input is last modeled byte in A) "
    "  *b=a a=0 (save in rotating buffer M pointed to by B) "
    "  d=0 hash b-- hash *d=a (put order 2 context hash in H[0] pointed by D)"
    "  d++ b-- hash b-- hash *d=a (put order 4 context in H[1]) "
    "  halt "
    "end " (no pre/post processing) ",
    args,     // Arguments $1 through $9 to ZPAQL code (unused, can be NULL)
    &out);    // Writer* to write pcomp command (default is NULL)

The first argument is a description of the compression algorithm in
the ZPAQL language. It is compiled into byte code and saved in the
archive block header so that the decompressor knows how read the data.
A ZPAQL program accepts up to 9 numeric arguments, which should be
passed in array.

A decompression algorithm has two optional parts, a context mixing
model and a postprocessor. The context model is identical for both
the compressor and decompressor, so is used in both instances. The
postprocessor, if present, is generally different, which presents
the possibility that the user supplied code may not restore the
original data exactly. It is assumed that the input has already been
preprocessed by the application but that the hash supplied to endSegment()
is of the original input before preprocessing. The following functions
allow you to test the postprocesser during compression:

  c.setVerify(true); // before c.compress(), may run slower
  c.getSize();       // return 64 bit size of postprocessed output
  c.getChecksum();   // after c.readSegmentEnd(), return hash, reset size to 0

This example has no postprocessor, but if it did, then setVerify(true)
would cause compress() to run the preprocessed input through the
postprocessor and into a SHA1 in parallel with compression. Then,
getChecksum() would return the hash which could be compared with
the hash of the input computed by the application. Also,

  int64_t size;
  c.endSegmentChecksum(&size, true);

instead of c.endSegment() will automatically add the computed checksum
if setVerify is true and return the checksum, whether or not there
is a postprocessor. If &size is not NULL then the segment size is written
to size. If the second argument is false then the computed checksum is
not saved to output. Default is true. If setVerify is false, then no
checksum is saved and the function returns 0 with size not written.

A context model consists of two parts, an array COMP of n components,
and some code HCOMP that computes contexts for the components.
The model compresses one bit at a time (MSB to LSB order) by computing
a probability that the next bit will be a 1, and then arithmetic
coding that bit. Better predictions compress smaller.

If n is 0 then the data is uncompressed. Otherwise, there is an array
of n = 1..255 components each taking a context and possibly the
predictions of previous components as input and outputting a new
prediction. The output of the last prediction is used to encode the
bit. After encoding, the state of each component is updated to
reduce the prediction error when the same context occurs again.
Components are as follows. Most arguments have range 0...255.

  CONST c          predict a 1 (c > 128) or 0 (c < 128).
  CM s t           context model with 2^s contexts, learning rate 1/4t.
  ICM s            indirect context model with 2^(s+6) contexts.
  MATCH s b        match model with 2^s context hashes and 2^b history.
  AVG j k wt       average components j and k with weight wt/256 for j.
  MIX2 s j k r x   average j and k with 2^s contexts, rate r, mask x.
  MIX  s j m r x   average j..j+m-1 with 2^s contexts, rate r, mask x.
  ISSE s j         adjust prediction j using 2^(s+6) indirect contexts.
  SSE s j t1 t2    adjust j using 2^s direct contexts, rate 1/t1..1/4t2.

A CONST predicts a 1 with probability 1/(1+exp((128-c)/16)), i.e
numbers near 0 or 255 are the most confident.
  
A CM maps a context to a prediction and a count. It is updated by
adjusting the prediction to reduce the error by 1/count and incrementing
the count up to 4t.

A ICM maps a s+10 bit context hash to a bit history (8 bit state)
representing a bounded count of zeros and ones previously seen in the
context and which bit was last. The bit history is mapped to a
prediction, which is updated by reducing the error by 1/1024.
The initial prediction is estimated from the counts represented by each
bit history.

A MATCH looks up a context hash and predicts whatever bit came next
following the previous occurrence in the history buffer. The strength
of the prediction depends on the match length.

AVG, MIX2, and MIX perform weighted averaging of predictions in the
logistic domain (log(p/(1-p))). AVG uses a fixed weight. MIX2 and MIX
adjust the weights (selected by context) to reduce prediction error
by a rate that increases with r. The mask is AND-ed with the current
partially coded byte to compute that context. Normally it is 255.
A MIX takes a contiguous range of m components as input.

ISSE adjusts a prediction using a bit history (as with an ICM) to
select a pair of weights for a 2 input MIX. It mixes the input
prediction with a constant 1 in the logistic domain.

SSE adjusts a logistic prediction by quantizing it to 32 levels and
selecting a new prediction from a table indexed by context, interpolating
between the nearest two steps. The nearest prediction error is
reduced by 1/count where count increments from t1 to 4*t2.

Contexts are computed and stored in an array H of 32 bit unsigned
integers by the HCOMP program written in ZPAQL. The program is called
after encoding a whole byte. To form a complete context, these values
are combined with the previous 0 to 7 bits of the current parital byte.
The method depends on the component type as follows:

  CM: H[i]    XOR hmap4(c).
  ICM, ISSE:  hash table lookup of (H[i]*16+c) on nibble boundaries.
  MIX2, MIX:  H[i] + (c AND x).
  SSE:        H[i] + c.

where c is the previous bits with a leading 1 bit (1, 1x, 1xx, ...,
1xxxxxxx where x is a previously coded bit). hmap4(c) maps c
to a 9 bit value to reduce cache misses. The first nibble is
mapped as before and the second nibble with 1xxxx in the high
5 bits. For example, after 6 bits, where c = 1xxxxxx,
hmap4(c) = 1xxxx01xx with the bits in the same order.

There are two ZPAQL virtual machines, HCOMP to compute contexts
and PCOMP to post-process the decoded output. Each has the
following state:

  PC: 16 bit program counter.
  A, B, C, D, R0...R255: 32 bit unsigned registers.
  F: 1 bit condition register.
  H: array of 2^h 32 bit unsigned values (output for HCOMP).
  M: array of 2^m 8 bit unsigned values.

All values are initialized to 0 at the beginning of a block
and retain their values between calls. There are two machines.
HCOMP is called after coding each byte with the value of that
byte in A. PCOMP, if present, is called once for each decoded
byte with that byte in A, and once more at the end of each
segment with 2^32 - 1 in A.

Normally, A is an accumulator. It is the destination of all
binary operations except assignment. The low m bits of B and
C index M. The low h bits of D indexes H. We write *B, *C, *D
to refer to the elements they point to. The instruction set
is as follows, where X is A, B, C, D, *B, *C, *D except as
indicated. X may also be a constant 0...255, written with
a leading space if it appears on the right side of an operator,
e.g. "*B= 255". Instructions taking a numeric argument are 2 bytes,
otherwise 1. Arithmetic is modulo 2^32.

  X<>A    Swap X with A (X cannot be A).
  X++     Add 1.
  X--     Subtract 1.
  X!      Complement bits of X.
  X=0     Clear X (1 byte instruction).
  X=X     Assignment to left hand side.
  A+=X    Add to A
  A-=X    Subtract from A
  A*=X    Multipy
  A/=X    Divide. If X is 0 then A=0.
  A%=X    Mod. If X is 0 then A=0.
  A&=X    Clear bits of A that are 0 in X.
  A&~X    Clear bits of A that are 1 in X.
  A|=X    Set bits of A that are 1 in X.
  A^=X    Complement bits of A that are set in X.
  A<<=X   Shift A left by (X mod 32) bits.
  A>>=X   Shift right (zero fill) A by (X mod 32) bits.
  A==X    Set F=1 if equal else F=0.
  A<X     Set F=1 if less else F=0.
  A>X     Set F=1 if greater else F=0.
  X=R N   Set A,B,C,D to RN (R0...R255).
  R=A N   Set R0...R255 to A.
  JMP N   Jump N=-128...127 bytes from next instruction.
  JT N    Jump N=-128...127 if F is 1.
  JF N    Jump N=-128...127 if F is 0.
  LJ N    Long jump to location 0...65535 (only 3 byte instruction).
  OUT     Output A (PCOMP only).
  HASH    A=(A+*B+512)*773.
  HASHD   *D=(*D+A+512)*773.
  HALT    Return at end of program.
  ERROR   Fail if executed.

Rather than using jump instructions, the following constructs are
allowed and translated appropriately.

  IF ... ENDIF              Execute if F is 1.
  IFNOT ... ENDIF           Execute if F is 0.
  IF ... ELSE ... ENDIF     Execute first part if F is 1 else second part.
  IFNOT ... ELSE ... ENDIF  Execute first part if F is 0 else second part.
  DO ... WHILE              Loop while F is 1.
  DO ... UNTIL              Loop while F is 0.
  DO ... FOREVER            Loop unconditionally.

Forward jumps (IF, IFNOT, ELSE) will not compile if beyond 127
instructions. In that case, use the long form (IFL, IFNOTL, ELSEL).
DO loops automatically use long jumps if needed. IF and DO loops
may intersect. For example, DO ... IF ... FOREVER ENDIF is equivalent
to a while-loop.

A config argument without a postprocessor has the following syntax:

  COMP hh hm ph pm n
    i COMP args...
  HCOMP
    zpaql...
  END (or POST 0 END for backward compatibility)

With a postprocessor:

  COMP hh hm ph pm n
    i COMP args...
  HCOMP
    zpaql...
  PCOMP command args... ;
    zpaql...
  END

In HCOMP, H and M have sizes 2^hh and 2^hm respectively. In PCOMP,
H and M have sizes 2^ph and 2^pm respectively. There are n components,
which must be numbered i = 0 to n-1. If a postprocessor is used, then
"command args..." is written to the Writer* passed as the 4'th argument,
but otherwise ignored. A typical use in a development environment might
be to call an external program that will be passed two additional
arguments on the command line, the input and output file names
respectively.

You can pass up to 9 signed numeric arguments in args[]. In any
place that a number "N" is allowed, you can write "$M" or "$M+N"
(like "$1" or $9+25") and value args[M-1]+N will be substituted.

ZPAQL allows (nested) comments in parenthesis. It is not case sensitive.
If there are input errors, then error() will report the error. If the
string contains newlines, it will report the line number of the error.

ZPAQL is compiled internally into a byte code, and then to native x86
32 or 64 bit code (unless compiled with -DNOJIT, in which case the
byte code is interpreted). You can also specify the algorithm directly
in byte code, although this is less convenient because it requires two
steps:

  c.startBlock(hcomp);      // COMP and HCOMP at start of block
  c.postProcess(pcomp, 0);  // PCOMP right before compress() in first segment

This is necessary because the COMP and HCOMP sections are stored in
the block header, but the PCOMP section is compressed in the first
segment after the filename and comment but before any data.

To retrive compiled byte code in suitable format after startBlock():

  c.hcomp(&out);      // writes COMP and HCOMP sections
  c.pcomp(&out);      // writes PCOMP section if any

Or during decompression:

  d.hcomp(&out);      // valid after findBlock()
  d.pcomp(&out);      // valid after decompress(0) in first segment

Both versions of pcomp() write nothing and return false if there is no
PCOMP section. The output of hcomp() and pcomp() may be passed to the
input of startBlock() and postProcess(). These are strings in which the
first 2 bytes encode the length of the rest of the string, least
significant byte first. Alternatively, postProcess() allows the length to
be omitted and passed separately as the second argument. In the case
of decompression, the HCOMP and PCOMP strings are read from the archive.
The preprocessor command (from "PCOMP cmd ;") is not saved in the compressed
data.


ARRAY

The libzpaq::Array template class is convenient for creating arrays aligned
on 64 byte addresses. It calls error("Out of memory") if needed.
It is used as follows:

  libzpaq::Array<T> a(n);  // array a[0]..a[n-1] of type T, zeroed
  a.resize(n);             // change size and zero contents
  a[i]                     // i'th element
  a(i)                     // a[i%n], valid only if n is a power of 2
  a.size()                 // n (as a size_t)
  a.isize()                // n (as a signed int)

T should be a simple type without constructors or destructors. Arrays
cannot be copied or assigned. You can also specify the size:

  Array<T> a(n, e);  // n << e
  a.resize(n, e);    // n << e

which is equivalent to n << e except that it calls error("Array too big")
rather than overflow if n << e would require more than 32 bits. If
compiled with -DDEBUG, then bounds are checked at run time.


ENCRYPTION

There is a class libzpaq::SHA256 with put(), result(), size(), and usize()
as in SHA1. result() returns a 32 byte SHA-256 hash. It is used by scrypt.

The libzpaq::AES_CTR class allows encryption in CTR mode with 128, 192,
or 256 bit keys. The public members are:

class AES_CTR {
public:
  AES_CTR(const char* key, int keylen, char* iv=0);
  void encrypt(U32 s0, U32 s1, U32 s2, U32 s3, unsigned char* ct);
  void encrypt(char* buf, int n, U64 offset);
};

The constructor initializes with a 16, 24, or 32 byte key. The length
is given by keylen. iv can be an 8 byte string or NULL. If not NULL
then iv0, iv1 are initialized with iv[0..7] in big-endian order, else 0.

encrypt(s0, s1, s2, s3, ct) encrypts a plaintext block divided into
4 32-bit words MSB first. The first byte of plaintext is the high 8
bits of s0. The output is to ct[16].

encrypt(buf, n, offset) encrypts or decrypts an n byte slice of a string
starting at offset. The i'th 16 byte block is encrypted by XOR with
the result (in ct) of encrypt(iv0, iv1, i>>32, i&0xffffffff, ct) starting
with i = 0. For example:

  AES_CTR a("a 128 bit key!!!", 16);
  char buf[500];             // some data 
  a.encrypt(buf, 100, 0);    // encrypt first 100 bytes
  a.encrypt(buf, 400, 100);  // encrypt next 400 bytes
  a.encrypt(buf, 500, 0);    // decrypt in one step

libzpaq::stretchKey(char* out, const char* in, const char* salt);

Generate a 32 byte key out[0..31] from key[0..31] and salt[0..31]
using scrypt(key, salt, N=16384, r=8, p=1). key[0..31] should be
the SHA-256 hash of the password. With these parameters, the function
uses 0.1 to 0.3 seconds and 16 MiB memory.
Scrypt is defined in http://www.tarsnap.com/scrypt/scrypt.pdf

void random(char* buf, int n);

Puts n cryptographic random bytes in buf[0..n-1], where the first
byte is never '7' or 'z' (start of a ZPAQ archive). For a pure
random string, discard the first byte.

Other classes and functions defined here are for internal use.
Use at your own risk.
*/

//////////////////////////////////////////////////////////////

#define unix

#ifndef LIBZPAQ_H
#define LIBZPAQ_H

#ifndef DEBUG
#define NDEBUG 1
#endif
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

namespace libzpaq {

// 1, 2, 4, 8 byte unsigned integers
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

// Tables for parsing ZPAQL source code
extern const char* compname[256];    // list of ZPAQL component types
extern const int compsize[256];      // number of bytes to encode a component
extern const char* opcodelist[272];  // list of ZPAQL instructions

// Callback for error handling
extern void error(const char* msg);

// Virtual base classes for input and output
// get() and put() must be overridden to read or write 1 byte.
// read() and write() may be overridden to read or write n bytes more
// efficiently than calling get() or put() n times.
class Reader {
public:
  virtual int get() = 0;  // should return 0..255, or -1 at EOF
  virtual int read(char* buf, int n); // read to buf[n], return no. read
  virtual ~Reader() {}
};

class Writer {
public:
  virtual void put(int c) = 0;  // should output low 8 bits of c
  virtual void write(const char* buf, int n);  // write buf[n]
  virtual ~Writer() {}
};

// Read 16 bit little-endian number
int toU16(const char* p);

// An Array of T is cleared and aligned on a 64 byte address
//   with no constructors called. No copy or assignment.
// Array<T> a(n, ex=0);  - creates n<<ex elements of type T
// a[i] - index
// a(i) - index mod n, n must be a power of 2
// a.size() - gets n
template <typename T>
class Array {
  T *data;     // user location of [0] on a 64 byte boundary
  size_t n;    // user size
  int offset;  // distance back in bytes to start of actual allocation
  void operator=(const Array&);  // no assignment
  Array(const Array&);  // no copy
public:
  Array(size_t sz=0, int ex=0): data(0), n(0), offset(0) {
    resize(sz, ex);} // [0..sz-1] = 0
  void resize(size_t sz, int ex=0); // change size, erase content to zeros
  ~Array() {resize(0);}  // free memory
  size_t size() const {return n;}  // get size
  int isize() const {return int(n);}  // get size as an int
  T& operator[](size_t i) {assert(n>0 && i<n); return data[i];}
  T& operator()(size_t i) {assert(n>0 && (n&(n-1))==0); return data[i&(n-1)];}
};

// Change size to sz<<ex elements of 0
template<typename T>
void Array<T>::resize(size_t sz, int ex) {
  assert(size_t(-1)>0);  // unsigned type?
  while (ex>0) {
    if (sz>sz*2) error("Array too big");
    sz*=2, --ex;
  }
  if (n>0) {
    assert(offset>0 && offset<=64);
    assert((char*)data-offset);
    ::free((char*)data-offset);
  }
  n=0;
  offset=0;
  if (sz==0) return;
  n=sz;
  const size_t nb=128+n*sizeof(T);  // test for overflow
  if (nb<=128 || (nb-128)/sizeof(T)!=n) n=0, error("Array too big");
  data=(T*)::calloc(nb, 1);
  if (!data) n=0, error("Out of memory");
  offset=64-(((char*)data-(char*)0)&63);
  assert(offset>0 && offset<=64);
  data=(T*)((char*)data+offset);
}

//////////////////////////// SHA1 ////////////////////////////

// For computing SHA-1 checksums
class SHA1 {
public:
  void put(int c) {  // hash 1 byte
    U32& r=w[U32(len)>>5&15];
    r=(r<<8)|(c&255);
    len+=8;
    if ((U32(len)&511)==0) process();
  }
  void write(const char* buf, int64_t n); // hash buf[0..n-1]
  double size() const {return len/8;}     // size in bytes
  uint64_t usize() const {return len/8;}  // size in bytes
  const char* result();  // get hash and reset
  SHA1() {init();}
private:
  void init();      // reset, but don't clear hbuf
  U64 len;          // length in bits
  U32 h[5];         // hash state
  U32 w[16];        // input buffer
  char hbuf[20];    // result
  void process();   // hash 1 block
};

//////////////////////////// SHA256 //////////////////////////

// For computing SHA-256 checksums
// http://en.wikipedia.org/wiki/SHA-2
class SHA256 {
public:
  void put(int c) {  // hash 1 byte
    unsigned& r=w[len0>>5&15];
    r=(r<<8)|(c&255);
    if (!(len0+=8)) ++len1;
    if ((len0&511)==0) process();
  }
  double size() const {return len0/8+len1*536870912.0;} // size in bytes
  uint64_t usize() const {return len0/8+(U64(len1)<<29);} //size in bytes
  const char* result();  // get hash and reset
  SHA256() {init();}
private:
  void init();           // reset, but don't clear hbuf
  unsigned len0, len1;   // length in bits (low, high)
  unsigned s[8];         // hash state
  unsigned w[16];        // input buffer
  char hbuf[32];         // result
  void process();        // hash 1 block
};

//////////////////////////// AES /////////////////////////////

// For encrypting with AES in CTR mode.
// The i'th 16 byte block is encrypted by XOR with AES(i)
// (i is big endian or MSB first, starting with 0).
class AES_CTR {
  U32 Te0[256], Te1[256], Te2[256], Te3[256], Te4[256]; // encryption tables
  U32 ek[60];  // round key
  int Nr;  // number of rounds (10, 12, 14 for AES 128, 192, 256)
  U32 iv0, iv1;  // first 8 bytes in CTR mode
public:
  AES_CTR(const char* key, int keylen, const char* iv=0);
    // Schedule: keylen is 16, 24, or 32, iv is 8 bytes or NULL
  void encrypt(U32 s0, U32 s1, U32 s2, U32 s3, unsigned char* ct);
  void encrypt(char* buf, int n, U64 offset);  // encrypt n bytes of buf
};

//////////////////////////// stretchKey //////////////////////

// Strengthen password pw[0..pwlen-1] and salt[0..saltlen-1]
// to produce key buf[0..buflen-1]. Uses O(n*r*p) time and 128*r*n bytes
// of memory. n must be a power of 2 and r <= 8.
void scrypt(const char* pw, int pwlen,
            const char* salt, int saltlen,
            int n, int r, int p, char* buf, int buflen);

// Generate a strong key out[0..31] key[0..31] and salt[0..31].
// Calls scrypt(key, 32, salt, 32, 16384, 8, 1, out, 32);
void stretchKey(char* out, const char* key, const char* salt);

//////////////////////////// random //////////////////////////

// Fill buf[0..n-1] with n cryptographic random bytes. The first
// byte is never '7' or 'z'.
void random(char* buf, int n);

//////////////////////////// ZPAQL ///////////////////////////

// Symbolic constants, instruction size, and names
typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE} CompType;
extern const int compsize[256];
class Decoder;  // forward

// A ZPAQL machine COMP+HCOMP or PCOMP.
class ZPAQL {
public:
  ZPAQL();
  ~ZPAQL();
  void clear();           // Free memory, erase program, reset machine state
  void inith();           // Initialize as HCOMP to run
  void initp();           // Initialize as PCOMP to run
  double memory();        // Return memory requirement in bytes
  void run(U32 input);    // Execute with input
  int read(Reader* in2);  // Read header
  bool write(Writer* out2, bool pp); // If pp write PCOMP else HCOMP header
  int step(U32 input, int mode);  // Trace execution (defined externally)

  Writer* output;         // Destination for OUT instruction, or 0 to suppress
  SHA1* sha1;             // Points to checksum computer
  U32 H(int i) {return h(i);}  // get element of h

  void flush();           // write outbuf[0..bufptr-1] to output and sha1
  void outc(int ch) {     // output byte ch (0..255) or -1 at EOS
    if (ch<0 || (outbuf[bufptr]=ch, ++bufptr==outbuf.isize())) flush();
  }

  // ZPAQ1 block header
  Array<U8> header;   // hsize[2] hh hm ph pm n COMP (guard) HCOMP (guard)
  int cend;           // COMP in header[7...cend-1]
  int hbegin, hend;   // HCOMP/PCOMP in header[hbegin...hend-1]

private:
  // Machine state for executing HCOMP
  Array<U8> m;        // memory array M for HCOMP
  Array<U32> h;       // hash array H for HCOMP
  Array<U32> r;       // 256 element register array
  Array<char> outbuf; // output buffer
  int bufptr;         // number of bytes in outbuf
  U32 a, b, c, d;     // machine registers
  int f;              // condition flag
  int pc;             // program counter
  int rcode_size;     // length of rcode
  U8* rcode;          // JIT code for run()

  // Support code
  int assemble();  // put JIT code in rcode
  void init(int hbits, int mbits);  // initialize H and M sizes
  int execute();  // interpret 1 instruction, return 0 after HALT, else 1
  void run0(U32 input);  // default run() if not JIT
  void div(U32 x) {if (x) a/=x; else a=0;}
  void mod(U32 x) {if (x) a%=x; else a=0;}
  void swap(U32& x) {a^=x; x^=a; a^=x;}
  void swap(U8& x)  {a^=x; x^=a; a^=x;}
  void err();  // exit with run time error
};

///////////////////////// Component //////////////////////////

// A Component is a context model, indirect context model, match model,
// fixed weight mixer, adaptive 2 input mixer without or with current
// partial byte as context, adaptive m input mixer (without or with),
// or SSE (without or with).

struct Component {
  size_t limit;   // max count for cm
  size_t cxt;     // saved context
  size_t a, b, c; // multi-purpose variables
  Array<U32> cm;  // cm[cxt] -> p in bits 31..10, n in 9..0; MATCH index
  Array<U8> ht;   // ICM/ISSE hash table[0..size1][0..15] and MATCH buf
  Array<U16> a16; // MIX weights
  void init();    // initialize to all 0
  Component() {init();}
};

////////////////////////// StateTable ////////////////////////

// Next state table
class StateTable {
public:
  U8 ns[1024]; // state*4 -> next state if 0, if 1, n0, n1
  int next(int state, int y) {  // next state for bit y
    assert(state>=0 && state<256);
    assert(y>=0 && y<4);
    return ns[state*4+y];
  }
  int cminit(int state) {  // initial probability of 1 * 2^23
    assert(state>=0 && state<256);
    return ((ns[state*4+3]*2+1)<<22)/(ns[state*4+2]+ns[state*4+3]+1);
  }
  StateTable();
};

///////////////////////// Predictor //////////////////////////

// A predictor guesses the next bit
class Predictor {
public:
  Predictor(ZPAQL&);
  ~Predictor();
  void init();          // build model
  int predict();        // probability that next bit is a 1 (0..4095)
  void update(int y);   // train on bit y (0..1)
  int stat(int);        // Defined externally
  bool isModeled() {    // n>0 components?
    assert(z.header.isize()>6);
    return z.header[6]!=0;
  }
private:

  // Predictor state
  int c8;               // last 0...7 bits.
  int hmap4;            // c8 split into nibbles
  int p[256];           // predictions
  U32 h[256];           // unrolled copy of z.h
  ZPAQL& z;             // VM to compute context hashes, includes H, n
  Component comp[256];  // the model, includes P
  bool initTables;      // are tables initialized?

  // Modeling support functions
  int predict0();       // default
  void update0(int y);  // default
  int dt2k[256];        // division table for match: dt2k[i] = 2^12/i
  int dt[1024];         // division table for cm: dt[i] = 2^16/(i+1.5)
  U16 squasht[4096];    // squash() lookup table
  short stretcht[32768];// stretch() lookup table
  StateTable st;        // next, cminit functions
  U8* pcode;            // JIT code for predict() and update()
  int pcode_size;       // length of pcode

  // reduce prediction error in cr.cm
  void train(Component& cr, int y) {
    assert(y==0 || y==1);
    U32& pn=cr.cm(cr.cxt);
    U32 count=pn&0x3ff;
    int error=y*32767-(cr.cm(cr.cxt)>>17);
    pn+=(error*dt[count]&-1024)+(count<cr.limit);
  }

  // x -> floor(32768/(1+exp(-x/64)))
  int squash(int x) {
    assert(initTables);
    assert(x>=-2048 && x<=2047);
    return squasht[x+2048];
  }

  // x -> round(64*log((x+0.5)/(32767.5-x))), approx inverse of squash
  int stretch(int x) {
    assert(initTables);
    assert(x>=0 && x<=32767);
    return stretcht[x];
  }

  // bound x to a 12 bit signed int
  int clamp2k(int x) {
    if (x<-2048) return -2048;
    else if (x>2047) return 2047;
    else return x;
  }

  // bound x to a 20 bit signed int
  int clamp512k(int x) {
    if (x<-(1<<19)) return -(1<<19);
    else if (x>=(1<<19)) return (1<<19)-1;
    else return x;
  }

  // Get cxt in ht, creating a new row if needed
  size_t find(Array<U8>& ht, int sizebits, U32 cxt);

  // Put JIT code in pcode
  int assemble_p();
};

//////////////////////////// Decoder /////////////////////////

// Decoder decompresses using an arithmetic code
class Decoder: public Reader {
public:
  Reader* in;        // destination
  Decoder(ZPAQL& z);
  int decompress();  // return a byte or EOF
  int skip();        // skip to the end of the segment, return next byte
  void init();       // initialize at start of block
  int stat(int x) {return pr.stat(x);}
  int get() {        // return 1 byte of buffered input or EOF
    if (rpos==wpos) {
      rpos=0;
      wpos=in ? in->read(&buf[0], BUFSIZE) : 0;
      assert(wpos<=BUFSIZE);
    }
    return rpos<wpos ? U8(buf[rpos++]) : -1;
  }
  int buffered() {return wpos-rpos;}  // how far read ahead?
private:
  U32 low, high;     // range
  U32 curr;          // last 4 bytes of archive or remaining bytes in subblock
  U32 rpos, wpos;    // read, write position in buf
  Predictor pr;      // to get p
  enum {BUFSIZE=1<<16};
  Array<char> buf;   // input buffer of size BUFSIZE bytes
  int decode(int p); // return decoded bit (0..1) with prob. p (0..65535)
};

/////////////////////////// PostProcessor ////////////////////

class PostProcessor {
  int state;   // input parse state: 0=INIT, 1=PASS, 2..4=loading, 5=POST
  int hsize;   // header size
  int ph, pm;  // sizes of H and M in z
public:
  ZPAQL z;     // holds PCOMP
  PostProcessor(): state(0), hsize(0), ph(0), pm(0) {}
  void init(int h, int m);  // ph, pm sizes of H and M
  int write(int c);  // Input a byte, return state
  int getState() const {return state;}
  void setOutput(Writer* out) {z.output=out;}
  void setSHA1(SHA1* sha1ptr) {z.sha1=sha1ptr;}
};

//////////////////////// Decompresser ////////////////////////

// For decompression and listing archive contents
class Decompresser {
public:
  Decompresser(): z(), dec(z), pp(), state(BLOCK), decode_state(FIRSTSEG) {}
  void setInput(Reader* in) {dec.in=in;}
  bool findBlock(double* memptr = 0);
  void hcomp(Writer* out2) {z.write(out2, false);}
  bool findFilename(Writer* = 0);
  void readComment(Writer* = 0);
  void setOutput(Writer* out) {pp.setOutput(out);}
  void setSHA1(SHA1* sha1ptr) {pp.setSHA1(sha1ptr);}
  bool decompress(int n = -1);  // n bytes, -1=all, return true until done
  bool pcomp(Writer* out2) {return pp.z.write(out2, true);}
  void readSegmentEnd(char* sha1string = 0);
  int stat(int x) {return dec.stat(x);}
  int buffered() {return dec.buffered();}
private:
  ZPAQL z;
  Decoder dec;
  PostProcessor pp;
  enum {BLOCK, FILENAME, COMMENT, DATA, SEGEND} state;  // expected next
  enum {FIRSTSEG, SEG, SKIP} decode_state;  // which segment in block?
};

/////////////////////////// decompress() /////////////////////

void decompress(Reader* in, Writer* out);

//////////////////////////// Encoder /////////////////////////

// Encoder compresses using an arithmetic code
class Encoder {
public:
  Encoder(ZPAQL& z, int size=0):
    out(0), low(1), high(0xFFFFFFFF), pr(z) {}
  void init();
  void compress(int c);  // c is 0..255 or EOF
  int stat(int x) {return pr.stat(x);}
  Writer* out;  // destination
private:
  U32 low, high; // range
  Predictor pr;  // to get p
  Array<char> buf; // unmodeled input
  void encode(int y, int p); // encode bit y (0..1) with prob. p (0..65535)
};

//////////////////////////// Compiler ////////////////////////

// Input ZPAQL source code with args and store the compiled code
// in hz and pz and write pcomp_cmd to out2.

class Compiler {
public:
  Compiler(const char* in, int* args, ZPAQL& hz, ZPAQL& pz, Writer* out2);
private:
  const char* in;  // ZPAQL source code
  int* args;       // Array of up to 9 args, default NULL = all 0
  ZPAQL& hz;       // Output of COMP and HCOMP sections
  ZPAQL& pz;       // Output of PCOMP section
  Writer* out2;    // Output ... of "PCOMP ... ;"
  int line;        // Input line number for reporting errors
  int state;       // parse state: 0=space -1=word >0 (nest level)

  // Symbolic constants
  typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,
    JT=39,JF=47,JMP=63,LJ=255,
    POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,
    WHILE,UNTIL,FOREVER,IFL,IFNOTL,ELSEL,SEMICOLON} CompType;

  void syntaxError(const char* msg, const char* expected=0); // error()
  void next();                     // advance in to next token
  bool matchToken(const char* tok);// in==token?
  int rtoken(int low, int high);   // return token which must be in range
  int rtoken(const char* list[]);  // return token by position in list
  void rtoken(const char* s);      // return token which must be s
  int compile_comp(ZPAQL& z);      // compile either HCOMP or PCOMP

  // Stack of n elements
  class Stack {
    libzpaq::Array<U16> s;
    size_t top;
  public:
    Stack(int n): s(n), top(0) {}
    void push(const U16& x) {
      if (top>=s.size()) error("IF or DO nested too deep");
      s[top++]=x;
    }
    U16 pop() {
      if (top<=0) error("unmatched IF or DO");
      return s[--top];
    }
  };

  Stack if_stack, do_stack;
};

//////////////////////// Compressor //////////////////////////

class Compressor {
public:
  Compressor(): enc(z), in(0), state(INIT), verify(false) {}
  void setOutput(Writer* out) {enc.out=out;}
  void writeTag();
  void startBlock(int level);  // level=1,2,3
  void startBlock(const char* hcomp);     // ZPAQL byte code
  void startBlock(const char* config,     // ZPAQL source code
                  int* args,              // NULL or int[9] arguments
                  Writer* pcomp_cmd = 0); // retrieve preprocessor command
  void setVerify(bool v) {verify = v;}    // check postprocessing?
  void hcomp(Writer* out2) {z.write(out2, false);}
  bool pcomp(Writer* out2) {return pz.write(out2, true);}
  void startSegment(const char* filename = 0, const char* comment = 0);
  void setInput(Reader* i) {in=i;}
  void postProcess(const char* pcomp = 0, int len = 0);  // byte code
  bool compress(int n = -1);  // n bytes, -1=all, return true until done
  void endSegment(const char* sha1string = 0);
  char* endSegmentChecksum(int64_t* size = 0, bool dosha1=true);
  int64_t getSize() {return sha1.usize();}
  const char* getChecksum() {return sha1.result();}
  void endBlock();
  int stat(int x) {return enc.stat(x);}
private:
  ZPAQL z, pz;  // model and test postprocessor
  Encoder enc;  // arithmetic encoder containing predictor
  Reader* in;   // input source
  SHA1 sha1;    // to test pz output
  char sha1result[20];  // sha1 output
  enum {INIT, BLOCK1, SEG1, BLOCK2, SEG2} state;
  bool verify;  // if true then test by postprocessing
};

/////////////////////////// StringBuffer /////////////////////

// For (de)compressing to/from a string. Writing appends bytes
// which can be later read.
class StringBuffer: public libzpaq::Reader, public libzpaq::Writer {
  unsigned char* p;  // allocated memory, not NUL terminated, may be NULL
  size_t al;         // number of bytes allocated, 0 iff p is NULL
  size_t wpos;       // index of next byte to write, wpos <= al
  size_t rpos;       // index of next byte to read, rpos < wpos or return EOF.
  size_t limit;      // max size, default = -1
  const size_t init; // initial size on first use after reset

  // Increase capacity to a without changing size
  void reserve(size_t a) {
    assert(!al==!p);
    if (a<=al) return;
    unsigned char* q=0;
    if (a>0) q=(unsigned char*)(p ? realloc(p, a) : malloc(a));
    if (a>0 && !q) error("Out of memory");
    p=q;
    al=a;
  }

  // Enlarge al to make room to write at least n bytes.
  void lengthen(size_t n) {
    assert(wpos<=al);
    if (wpos+n>limit || wpos+n<wpos) error("StringBuffer overflow");
    if (wpos+n<=al) return;
    size_t a=al;
    while (wpos+n>=a) a=a*2+init;
    reserve(a);
  }

  // No assignment or copy
  void operator=(const StringBuffer&);
  StringBuffer(const StringBuffer&);

public:

  // Direct access to data
  unsigned char* data() {assert(p || wpos==0); return p;}

  // Allocate no memory initially
  StringBuffer(size_t n=0):
      p(0), al(0), wpos(0), rpos(0), limit(size_t(-1)), init(n>128?n:128) {}

  // Set output limit
  void setLimit(size_t n) {limit=n;}

  // Free memory
  ~StringBuffer() {if (p) free(p);}

  // Return number of bytes written.
  size_t size() const {return wpos;}

  // Return number of bytes left to read
  size_t remaining() const {return wpos-rpos;}

  // Reset size to 0 and free memory.
  void reset() {
    if (p) free(p);
    p=0;
    al=rpos=wpos=0;
  }

  // Write a single byte.
  void put(int c) {  // write 1 byte
    lengthen(1);
    assert(p);
    assert(wpos<al);
    p[wpos++]=c;
    assert(wpos<=al);
  }

  // Write buf[0..n-1]. If buf is NULL then advance write pointer only.
  void write(const char* buf, int n) {
    if (n<1) return;
    lengthen(n);
    assert(p);
    assert(wpos+n<=al);
    if (buf) memcpy(p+wpos, buf, n);
    wpos+=n;
  }

  // Read a single byte. Return EOF (-1) at end.
  int get() {
    assert(rpos<=wpos);
    assert(rpos==wpos || p);
    return rpos<wpos ? p[rpos++] : -1;
  }

  // Read up to n bytes into buf[0..] or fewer if EOF is first.
  // Return the number of bytes actually read.
  // If buf is NULL then advance read pointer without reading.
  int read(char* buf, int n) {
    assert(rpos<=wpos);
    assert(wpos<=al);
    assert(!al==!p);
    if (rpos+n>wpos) n=wpos-rpos;
    if (n>0 && buf) memcpy(buf, p+rpos, n);
    rpos+=n;
    return n;
  }

  // Return the entire string as a read-only array.
  const char* c_str() const {return (const char*)p;}

  // Truncate the string to size i.
  void resize(size_t i) {
    wpos=i;
    if (rpos>wpos) rpos=wpos;
  }

  // Swap efficiently (init is not swapped)
  void swap(StringBuffer& s) {
    std::swap(p, s.p);
    std::swap(al, s.al);
    std::swap(wpos, s.wpos);
    std::swap(rpos, s.rpos);
    std::swap(limit, s.limit);
  }
};

/////////////////////////// compress() ///////////////////////

// Compress in to out in multiple blocks. Default method is "14,128,0"
// Default filename is "". Comment is appended to input size.
// dosha1 means save the SHA-1 checksum.
void compress(Reader* in, Writer* out, const char* method,
     const char* filename=0, const char* comment=0, bool dosha1=true);

// Same as compress() but output is 1 block, ignoring block size parameter.
void compressBlock(StringBuffer* in, Writer* out, const char* method,
     const char* filename=0, const char* comment=0, bool dosha1=true);

}  // namespace libzpaq

#endif  // LIBZPAQ_H



#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>

#ifdef unix
#ifndef NOJIT
#include <sys/mman.h>
#endif
#else
#include <windows.h>
#include <wincrypt.h>
#endif

namespace libzpaq {

// Read 16 bit little-endian number
int toU16(const char* p) {
  return (p[0]&255)+256*(p[1]&255);
}

// Default read() and write()
int Reader::read(char* buf, int n) {
  int i=0, c;
  while (i<n && (c=get())>=0)
    buf[i++]=c;
  return i;
}

void Writer::write(const char* buf, int n) {
  for (int i=0; i<n; ++i)
    put(U8(buf[i]));
}

///////////////////////// allocx //////////////////////

// Allocate newsize > 0 bytes of executable memory and update
// p to point to it and newsize = n. Free any previously
// allocated memory first. If newsize is 0 then free only.
// Call error in case of failure. If NOJIT, ignore newsize
// and set p=0, n=0 without allocating memory.
void allocx(U8* &p, int &n, int newsize) {
#ifdef NOJIT
  p=0;
  n=0;
#else
  if (p || n) {
    if (p)
#ifdef unix
      munmap(p, n);
#else // Windows
      VirtualFree(p, 0, MEM_RELEASE);
#endif
    p=0;
    n=0;
  }
  if (newsize>0) {
#ifdef unix
    p=(U8*)mmap(0, newsize, PROT_READ|PROT_WRITE|PROT_EXEC,
                MAP_PRIVATE|MAP_ANON, -1, 0);
    if ((void*)p==MAP_FAILED) p=0;
#else
    p=(U8*)VirtualAlloc(0, newsize, MEM_RESERVE|MEM_COMMIT,
                        PAGE_EXECUTE_READWRITE);
#endif
    if (p)
      n=newsize;
    else {
      n=0;
      error("allocx failed");
    }
  }
#endif
}

//////////////////////////// SHA1 ////////////////////////////

// SHA1 code, see http://en.wikipedia.org/wiki/SHA-1

// Start a new hash
void SHA1::init() {
  len=0;
  h[0]=0x67452301;
  h[1]=0xEFCDAB89;
  h[2]=0x98BADCFE;
  h[3]=0x10325476;
  h[4]=0xC3D2E1F0;
  memset(w, 0, sizeof(w));
}

// Return old result and start a new hash
const char* SHA1::result() {

  // pad and append length
  const U64 s=len;
  put(0x80);
  while ((len&511)!=448)
    put(0);
  put(s>>56);
  put(s>>48);
  put(s>>40);
  put(s>>32);
  put(s>>24);
  put(s>>16);
  put(s>>8);
  put(s);

  // copy h to hbuf
  for (int i=0; i<5; ++i) {
    hbuf[4*i]=h[i]>>24;
    hbuf[4*i+1]=h[i]>>16;
    hbuf[4*i+2]=h[i]>>8;
    hbuf[4*i+3]=h[i];
  }

  // return hash prior to clearing state
  init();
  return hbuf;
}

// Hash buf[0..n-1]
void SHA1::write(const char* buf, int64_t n) {
  const unsigned char* p=(const unsigned char*) buf;
  for (; n>0 && (U32(len)&511)!=0; --n) put(*p++);
  for (; n>=64; n-=64) {
    for (int i=0; i<16; ++i)
      w[i]=p[0]<<24|p[1]<<16|p[2]<<8|p[3], p+=4;
    len+=512;
    process();
  }
  for (; n>0; --n) put(*p++);
}

// Hash 1 block of 64 bytes
void SHA1::process() {
  U32 a=h[0], b=h[1], c=h[2], d=h[3], e=h[4];
  static const U32 k[4]={0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};
  #define f(a,b,c,d,e,i) \
    if (i>=16) \
      w[(i)&15]^=w[(i-3)&15]^w[(i-8)&15]^w[(i-14)&15], \
      w[(i)&15]=w[(i)&15]<<1|w[(i)&15]>>31; \
    e+=(a<<5|a>>27)+k[(i)/20]+w[(i)&15] \
      +((i)%40>=20 ? b^c^d : i>=40 ? (b&c)|(d&(b|c)) : d^(b&(c^d))); \
    b=b<<30|b>>2;
  #define r(i) f(a,b,c,d,e,i) f(e,a,b,c,d,i+1) f(d,e,a,b,c,i+2) \
               f(c,d,e,a,b,i+3) f(b,c,d,e,a,i+4)
  r(0)  r(5)  r(10) r(15) r(20) r(25) r(30) r(35)
  r(40) r(45) r(50) r(55) r(60) r(65) r(70) r(75)
  #undef f
  #undef r
  h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e;
}

//////////////////////////// SHA256 //////////////////////////

void SHA256::init() {
  len0=len1=0;
  s[0]=0x6a09e667;
  s[1]=0xbb67ae85;
  s[2]=0x3c6ef372;
  s[3]=0xa54ff53a;
  s[4]=0x510e527f;
  s[5]=0x9b05688c;
  s[6]=0x1f83d9ab;
  s[7]=0x5be0cd19;
  memset(w, 0, sizeof(w));
}

void SHA256::process() {

  #define ror(a,b) ((a)>>(b)|(a<<(32-(b))))

  #define m(i) \
     w[(i)&15]+=w[(i-7)&15] \
       +(ror(w[(i-15)&15],7)^ror(w[(i-15)&15],18)^(w[(i-15)&15]>>3)) \
       +(ror(w[(i-2)&15],17)^ror(w[(i-2)&15],19)^(w[(i-2)&15]>>10))

  #define r(a,b,c,d,e,f,g,h,i) { \
    unsigned t1=ror(e,14)^e; \
    t1=ror(t1,5)^e; \
    h+=ror(t1,6)+((e&f)^(~e&g))+k[i]+w[(i)&15]; } \
    d+=h; \
    {unsigned t1=ror(a,9)^a; \
    t1=ror(t1,11)^a; \
    h+=ror(t1,2)+((a&b)^(c&(a^b))); }

  #define mr(a,b,c,d,e,f,g,h,i) m(i); r(a,b,c,d,e,f,g,h,i);

  #define r8(i) \
    r(a,b,c,d,e,f,g,h,i);   \
    r(h,a,b,c,d,e,f,g,i+1); \
    r(g,h,a,b,c,d,e,f,i+2); \
    r(f,g,h,a,b,c,d,e,i+3); \
    r(e,f,g,h,a,b,c,d,i+4); \
    r(d,e,f,g,h,a,b,c,i+5); \
    r(c,d,e,f,g,h,a,b,i+6); \
    r(b,c,d,e,f,g,h,a,i+7);

  #define mr8(i) \
    mr(a,b,c,d,e,f,g,h,i);   \
    mr(h,a,b,c,d,e,f,g,i+1); \
    mr(g,h,a,b,c,d,e,f,i+2); \
    mr(f,g,h,a,b,c,d,e,i+3); \
    mr(e,f,g,h,a,b,c,d,i+4); \
    mr(d,e,f,g,h,a,b,c,i+5); \
    mr(c,d,e,f,g,h,a,b,i+6); \
    mr(b,c,d,e,f,g,h,a,i+7);

  static const unsigned k[64]={
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

  unsigned a=s[0];
  unsigned b=s[1];
  unsigned c=s[2];
  unsigned d=s[3];
  unsigned e=s[4];
  unsigned f=s[5];
  unsigned g=s[6];
  unsigned h=s[7];

  r8(0);
  r8(8);
  mr8(16);
  mr8(24);
  mr8(32);
  mr8(40);
  mr8(48);
  mr8(56);

  s[0]+=a;
  s[1]+=b;
  s[2]+=c;
  s[3]+=d;
  s[4]+=e;
  s[5]+=f;
  s[6]+=g;
  s[7]+=h;

  #undef mr8
  #undef r8
  #undef mr
  #undef r
  #undef m
  #undef ror
};

// Return old result and start a new hash
const char* SHA256::result() {

  // pad and append length
  const unsigned s1=len1, s0=len0;
  put(0x80);
  while ((len0&511)!=448) put(0);
  put(s1>>24);
  put(s1>>16);
  put(s1>>8);
  put(s1);
  put(s0>>24);
  put(s0>>16);
  put(s0>>8);
  put(s0);

  // copy s to hbuf
  for (int i=0; i<8; ++i) {
    hbuf[4*i]=s[i]>>24;
    hbuf[4*i+1]=s[i]>>16;
    hbuf[4*i+2]=s[i]>>8;
    hbuf[4*i+3]=s[i];
  }

  // return hash prior to clearing state
  init();
  return hbuf;
}

//////////////////////////// AES /////////////////////////////

// Some AES code is derived from libtomcrypt 1.17 (public domain).

#define Te4_0 0x000000FF & Te4
#define Te4_1 0x0000FF00 & Te4
#define Te4_2 0x00FF0000 & Te4
#define Te4_3 0xFF000000 & Te4

// Extract byte n of x
static inline unsigned byte(unsigned x, unsigned n) {return (x>>(8*n))&255;}

// x = y[0..3] MSB first
static inline void LOAD32H(U32& x, const char* y) {
  const unsigned char* u=(const unsigned char*)y;
  x=u[0]<<24|u[1]<<16|u[2]<<8|u[3];
}

// y[0..3] = x MSB first
static inline void STORE32H(U32& x, unsigned char* y) {
  y[0]=x>>24;
  y[1]=x>>16;
  y[2]=x>>8;
  y[3]=x;
}

#define setup_mix(temp) \
  ((Te4_3[byte(temp, 2)]) ^ (Te4_2[byte(temp, 1)]) ^ \
   (Te4_1[byte(temp, 0)]) ^ (Te4_0[byte(temp, 3)]))

// Initialize encryption tables and round key. keylen is 16, 24, or 32.
AES_CTR::AES_CTR(const char* key, int keylen, const char* iv) {
  assert(key  != NULL);
  assert(keylen==16 || keylen==24 || keylen==32);

  // Initialize IV (default 0)
  iv0=iv1=0;
  if (iv) {
    LOAD32H(iv0, iv);
    LOAD32H(iv1, iv+4);
  }

  // Initialize encryption tables
  for (int i=0; i<256; ++i) {
    unsigned s1=
    "\x63\x7c\x77\x7b\xf2\x6b\x6f\xc5\x30\x01\x67\x2b\xfe\xd7\xab\x76"
    "\xca\x82\xc9\x7d\xfa\x59\x47\xf0\xad\xd4\xa2\xaf\x9c\xa4\x72\xc0"
    "\xb7\xfd\x93\x26\x36\x3f\xf7\xcc\x34\xa5\xe5\xf1\x71\xd8\x31\x15"
    "\x04\xc7\x23\xc3\x18\x96\x05\x9a\x07\x12\x80\xe2\xeb\x27\xb2\x75"
    "\x09\x83\x2c\x1a\x1b\x6e\x5a\xa0\x52\x3b\xd6\xb3\x29\xe3\x2f\x84"
    "\x53\xd1\x00\xed\x20\xfc\xb1\x5b\x6a\xcb\xbe\x39\x4a\x4c\x58\xcf"
    "\xd0\xef\xaa\xfb\x43\x4d\x33\x85\x45\xf9\x02\x7f\x50\x3c\x9f\xa8"
    "\x51\xa3\x40\x8f\x92\x9d\x38\xf5\xbc\xb6\xda\x21\x10\xff\xf3\xd2"
    "\xcd\x0c\x13\xec\x5f\x97\x44\x17\xc4\xa7\x7e\x3d\x64\x5d\x19\x73"
    "\x60\x81\x4f\xdc\x22\x2a\x90\x88\x46\xee\xb8\x14\xde\x5e\x0b\xdb"
    "\xe0\x32\x3a\x0a\x49\x06\x24\x5c\xc2\xd3\xac\x62\x91\x95\xe4\x79"
    "\xe7\xc8\x37\x6d\x8d\xd5\x4e\xa9\x6c\x56\xf4\xea\x65\x7a\xae\x08"
    "\xba\x78\x25\x2e\x1c\xa6\xb4\xc6\xe8\xdd\x74\x1f\x4b\xbd\x8b\x8a"
    "\x70\x3e\xb5\x66\x48\x03\xf6\x0e\x61\x35\x57\xb9\x86\xc1\x1d\x9e"
    "\xe1\xf8\x98\x11\x69\xd9\x8e\x94\x9b\x1e\x87\xe9\xce\x55\x28\xdf"
    "\x8c\xa1\x89\x0d\xbf\xe6\x42\x68\x41\x99\x2d\x0f\xb0\x54\xbb\x16"
    [i]&255;
    unsigned s2=s1<<1;
    if (s2>=0x100) s2^=0x11b;
    unsigned s3=s1^s2;
    Te0[i]=s2<<24|s1<<16|s1<<8|s3;
    Te1[i]=s3<<24|s2<<16|s1<<8|s1;
    Te2[i]=s1<<24|s3<<16|s2<<8|s1;
    Te3[i]=s1<<24|s1<<16|s3<<8|s2;
    Te4[i]=s1<<24|s1<<16|s1<<8|s1;
  }

  // setup the forward key
  Nr = 10 + ((keylen/8)-2)*2;  // 10, 12, or 14 rounds
  int i = 0;
  U32* rk = &ek[0];
  U32 temp;
  static const U32 rcon[10] = {
    0x01000000UL, 0x02000000UL, 0x04000000UL, 0x08000000UL,
    0x10000000UL, 0x20000000UL, 0x40000000UL, 0x80000000UL,
    0x1B000000UL, 0x36000000UL};  // round constants

  LOAD32H(rk[0], key   );
  LOAD32H(rk[1], key +  4);
  LOAD32H(rk[2], key +  8);
  LOAD32H(rk[3], key + 12);
  if (keylen == 16) {
    for (;;) {
      temp  = rk[3];
      rk[4] = rk[0] ^ setup_mix(temp) ^ rcon[i];
      rk[5] = rk[1] ^ rk[4];
      rk[6] = rk[2] ^ rk[5];
      rk[7] = rk[3] ^ rk[6];
      if (++i == 10) {
         break;
      }
      rk += 4;
    }
  }
  else if (keylen == 24) {
    LOAD32H(rk[4], key + 16);
    LOAD32H(rk[5], key + 20);
    for (;;) {
      temp = rk[5];
      rk[ 6] = rk[ 0] ^ setup_mix(temp) ^ rcon[i];
      rk[ 7] = rk[ 1] ^ rk[ 6];
      rk[ 8] = rk[ 2] ^ rk[ 7];
      rk[ 9] = rk[ 3] ^ rk[ 8];
      if (++i == 8) {
        break;
      }
      rk[10] = rk[ 4] ^ rk[ 9];
      rk[11] = rk[ 5] ^ rk[10];
      rk += 6;
    }
  }
  else if (keylen == 32) {
    LOAD32H(rk[4], key + 16);
    LOAD32H(rk[5], key + 20);
    LOAD32H(rk[6], key + 24);
    LOAD32H(rk[7], key + 28);
    for (;;) {
      temp = rk[7];
      rk[ 8] = rk[ 0] ^ setup_mix(temp) ^ rcon[i];
      rk[ 9] = rk[ 1] ^ rk[ 8];
      rk[10] = rk[ 2] ^ rk[ 9];
      rk[11] = rk[ 3] ^ rk[10];
      if (++i == 7) {
        break;
      }
      temp = rk[11];
      rk[12] = rk[ 4] ^ setup_mix(temp<<24|temp>>8);
      rk[13] = rk[ 5] ^ rk[12];
      rk[14] = rk[ 6] ^ rk[13];
      rk[15] = rk[ 7] ^ rk[14];
      rk += 8;
    }
  }
}

// Encrypt to ct[16]
void AES_CTR::encrypt(U32 s0, U32 s1, U32 s2, U32 s3, unsigned char* ct) {
  int r = Nr >> 1;
  U32 *rk = &ek[0];
  U32 t0=0, t1=0, t2=0, t3=0;
  s0 ^= rk[0];
  s1 ^= rk[1];
  s2 ^= rk[2];
  s3 ^= rk[3];
  for (;;) {
    t0 =
      Te0[byte(s0, 3)] ^
      Te1[byte(s1, 2)] ^
      Te2[byte(s2, 1)] ^
      Te3[byte(s3, 0)] ^
      rk[4];
    t1 =
      Te0[byte(s1, 3)] ^
      Te1[byte(s2, 2)] ^
      Te2[byte(s3, 1)] ^
      Te3[byte(s0, 0)] ^
      rk[5];
    t2 =
      Te0[byte(s2, 3)] ^
      Te1[byte(s3, 2)] ^
      Te2[byte(s0, 1)] ^
      Te3[byte(s1, 0)] ^
      rk[6];
    t3 =
      Te0[byte(s3, 3)] ^
      Te1[byte(s0, 2)] ^
      Te2[byte(s1, 1)] ^
      Te3[byte(s2, 0)] ^
      rk[7];

    rk += 8;
    if (--r == 0) {
      break;
    }

    s0 =
      Te0[byte(t0, 3)] ^
      Te1[byte(t1, 2)] ^
      Te2[byte(t2, 1)] ^
      Te3[byte(t3, 0)] ^
      rk[0];
    s1 =
      Te0[byte(t1, 3)] ^
      Te1[byte(t2, 2)] ^
      Te2[byte(t3, 1)] ^
      Te3[byte(t0, 0)] ^
      rk[1];
    s2 =
      Te0[byte(t2, 3)] ^
      Te1[byte(t3, 2)] ^
      Te2[byte(t0, 1)] ^
      Te3[byte(t1, 0)] ^
      rk[2];
    s3 =
      Te0[byte(t3, 3)] ^
      Te1[byte(t0, 2)] ^
      Te2[byte(t1, 1)] ^
      Te3[byte(t2, 0)] ^
      rk[3];
  }

  // apply last round and map cipher state to byte array block:
  s0 =
    (Te4_3[byte(t0, 3)]) ^
    (Te4_2[byte(t1, 2)]) ^
    (Te4_1[byte(t2, 1)]) ^
    (Te4_0[byte(t3, 0)]) ^
    rk[0];
  STORE32H(s0, ct);
  s1 =
    (Te4_3[byte(t1, 3)]) ^
    (Te4_2[byte(t2, 2)]) ^
    (Te4_1[byte(t3, 1)]) ^
    (Te4_0[byte(t0, 0)]) ^
    rk[1];
  STORE32H(s1, ct+4);
  s2 =
    (Te4_3[byte(t2, 3)]) ^
    (Te4_2[byte(t3, 2)]) ^
    (Te4_1[byte(t0, 1)]) ^
    (Te4_0[byte(t1, 0)]) ^
    rk[2];
  STORE32H(s2, ct+8);
  s3 =
    (Te4_3[byte(t3, 3)]) ^
    (Te4_2[byte(t0, 2)]) ^
    (Te4_1[byte(t1, 1)]) ^
    (Te4_0[byte(t2, 0)]) ^ 
    rk[3];
  STORE32H(s3, ct+12);
}

// Encrypt or decrypt slice buf[0..n-1] at offset by XOR with AES(i) where
// i is the 128 bit big-endian distance from the start in 16 byte blocks.
void AES_CTR::encrypt(char* buf, int n, U64 offset) {
  for (U64 i=offset/16; i<=(offset+n)/16; ++i) {
    unsigned char ct[16];
    encrypt(iv0, iv1, i>>32, i, ct);
    for (int j=0; j<16; ++j) {
      const int k=i*16-offset+j;
      if (k>=0 && k<n)
        buf[k]^=ct[j];
    }
  }
}

#undef setup_mix
#undef Te4_3
#undef Te4_2
#undef Te4_1
#undef Te4_0

//////////////////////////// stretchKey //////////////////////

// PBKDF2(pw[0..pwlen], salt[0..saltlen], c) to buf[0..dkLen-1]
// using HMAC-SHA256, for the special case of c = 1 iterations
// output size dkLen a multiple of 32, and pwLen <= 64.
static void pbkdf2(const char* pw, int pwLen, const char* salt, int saltLen,
                   int c, char* buf, int dkLen) {
  assert(c==1);
  assert(dkLen%32==0);
  assert(pwLen<=64);

  libzpaq::SHA256 sha256;
  char b[32];
  for (int i=1; i*32<=dkLen; ++i) {
    for (int j=0; j<pwLen; ++j) sha256.put(pw[j]^0x36);
    for (int j=pwLen; j<64; ++j) sha256.put(0x36);
    for (int j=0; j<saltLen; ++j) sha256.put(salt[j]);
    for (int j=24; j>=0; j-=8) sha256.put(i>>j);
    memcpy(b, sha256.result(), 32);
    for (int j=0; j<pwLen; ++j) sha256.put(pw[j]^0x5c);
    for (int j=pwLen; j<64; ++j) sha256.put(0x5c);
    for (int j=0; j<32; ++j) sha256.put(b[j]);
    memcpy(buf+i*32-32, sha256.result(), 32);
  }
}

// Hash b[0..15] using 8 rounds of salsa20
// Modified from http://cr.yp.to/salsa20.html (public domain) to 8 rounds
static void salsa8(U32* b) {
  unsigned x[16]={0};
  memcpy(x, b, 64);
  for (int i=0; i<4; ++i) {
    #define R(a,b) (((a)<<(b))+((a)>>(32-b)))
    x[ 4] ^= R(x[ 0]+x[12], 7);  x[ 8] ^= R(x[ 4]+x[ 0], 9);
    x[12] ^= R(x[ 8]+x[ 4],13);  x[ 0] ^= R(x[12]+x[ 8],18);
    x[ 9] ^= R(x[ 5]+x[ 1], 7);  x[13] ^= R(x[ 9]+x[ 5], 9);
    x[ 1] ^= R(x[13]+x[ 9],13);  x[ 5] ^= R(x[ 1]+x[13],18);
    x[14] ^= R(x[10]+x[ 6], 7);  x[ 2] ^= R(x[14]+x[10], 9);
    x[ 6] ^= R(x[ 2]+x[14],13);  x[10] ^= R(x[ 6]+x[ 2],18);
    x[ 3] ^= R(x[15]+x[11], 7);  x[ 7] ^= R(x[ 3]+x[15], 9);
    x[11] ^= R(x[ 7]+x[ 3],13);  x[15] ^= R(x[11]+x[ 7],18);
    x[ 1] ^= R(x[ 0]+x[ 3], 7);  x[ 2] ^= R(x[ 1]+x[ 0], 9);
    x[ 3] ^= R(x[ 2]+x[ 1],13);  x[ 0] ^= R(x[ 3]+x[ 2],18);
    x[ 6] ^= R(x[ 5]+x[ 4], 7);  x[ 7] ^= R(x[ 6]+x[ 5], 9);
    x[ 4] ^= R(x[ 7]+x[ 6],13);  x[ 5] ^= R(x[ 4]+x[ 7],18);
    x[11] ^= R(x[10]+x[ 9], 7);  x[ 8] ^= R(x[11]+x[10], 9);
    x[ 9] ^= R(x[ 8]+x[11],13);  x[10] ^= R(x[ 9]+x[ 8],18);
    x[12] ^= R(x[15]+x[14], 7);  x[13] ^= R(x[12]+x[15], 9);
    x[14] ^= R(x[13]+x[12],13);  x[15] ^= R(x[14]+x[13],18);
    #undef R
  }
  for (int i=0; i<16; ++i) b[i]+=x[i];
}

// BlockMix_{Salsa20/8, r} on b[0..128*r-1]
static void blockmix(U32* b, int r) {
  assert(r<=8);
  U32 x[16];
  U32 y[256];
  memcpy(x, b+32*r-16, 64);
  for (int i=0; i<2*r; ++i) {
    for (int j=0; j<16; ++j) x[j]^=b[i*16+j];
    salsa8(x);
    memcpy(&y[i*16], x, 64);
  }
  for (int i=0; i<r; ++i) memcpy(b+i*16, &y[i*32], 64);
  for (int i=0; i<r; ++i) memcpy(b+(i+r)*16, &y[i*32+16], 64);
}

// Mix b[0..128*r-1]. Uses 128*r*n bytes of memory and O(r*n) time
static void smix(char* b, int r, int n) {
  libzpaq::Array<U32> x(32*r), v(32*r*n);
  for (int i=0; i<r*128; ++i) x[i/4]+=(b[i]&255)<<i%4*8;
  for (int i=0; i<n; ++i) {
    memcpy(&v[i*r*32], &x[0], r*128);
    blockmix(&x[0], r);
  }
  for (int i=0; i<n; ++i) {
    U32 j=x[(2*r-1)*16]&(n-1);
    for (int k=0; k<r*32; ++k) x[k]^=v[j*r*32+k];
    blockmix(&x[0], r);
  }
  for (int i=0; i<r*128; ++i) b[i]=x[i/4]>>(i%4*8);
}

// Strengthen password pw[0..pwlen-1] and salt[0..saltlen-1]
// to produce key buf[0..buflen-1]. Uses O(n*r*p) time and 128*r*n bytes
// of memory. n must be a power of 2 and r <= 8.
void scrypt(const char* pw, int pwlen,
            const char* salt, int saltlen,
            int n, int r, int p, char* buf, int buflen) {
  assert(r<=8);
  assert(n>0 && (n&(n-1))==0);  // power of 2?
  libzpaq::Array<char> b(p*r*128);
  pbkdf2(pw, pwlen, salt, saltlen, 1, &b[0], p*r*128);
  for (int i=0; i<p; ++i) smix(&b[i*r*128], r, n);
  pbkdf2(pw, pwlen, &b[0], p*r*128, 1, buf, buflen);
}

// Stretch key in[0..31], assumed to be SHA256(password), with
// NUL terminate salt to produce new key out[0..31]
void stretchKey(char* out, const char* in, const char* salt) {
  scrypt(in, 32, salt, 32, 1<<14, 8, 1, out, 32);
}

//////////////////////////// random //////////////////////////

// Put n cryptographic random bytes in buf[0..n-1].
// The first byte will not be 'z' or '7' (start of a ZPAQ archive).
// For a pure random number, discard the first byte.
// In VC++, must link to advapi32.lib.

void random(char* buf, int n) {
#ifdef unix
  FILE* in=fopen("/dev/urandom", "rb");
  if (in && int(fread(buf, 1, n, in))==n)
    fclose(in);
  else {
    error("key generation failed");
  }
#else
  HCRYPTPROV h;
  if (CryptAcquireContext(&h, NULL, NULL, PROV_RSA_FULL,
      CRYPT_VERIFYCONTEXT) && CryptGenRandom(h, n, (BYTE*)buf))
    CryptReleaseContext(h, 0);
  else {
    fprintf(stderr, "CryptGenRandom: error %d\n", int(GetLastError()));
    error("key generation failed");
  }
#endif
  if (n>=1 && (buf[0]=='z' || buf[0]=='7'))
    buf[0]^=0x80;
}

//////////////////////////// Component ///////////////////////

// A Component is a context model, indirect context model, match model,
// fixed weight mixer, adaptive 2 input mixer without or with current
// partial byte as context, adaptive m input mixer (without or with),
// or SSE (without or with).

const int compsize[256]={0,2,3,2,3,4,6,6,3,5};

void Component::init() {
  limit=cxt=a=b=c=0;
  cm.resize(0);
  ht.resize(0);
  a16.resize(0);
}

////////////////////////// StateTable ////////////////////////

// sns[i*4] -> next state if 0, next state if 1, n0, n1
static const U8 sns[1024]={
     1,     2,     0,     0,     3,     5,     1,     0,
     4,     6,     0,     1,     7,     9,     2,     0,
     8,    11,     1,     1,     8,    11,     1,     1,
    10,    12,     0,     2,    13,    15,     3,     0,
    14,    17,     2,     1,    14,    17,     2,     1,
    16,    19,     1,     2,    16,    19,     1,     2,
    18,    20,     0,     3,    21,    23,     4,     0,
    22,    25,     3,     1,    22,    25,     3,     1,
    24,    27,     2,     2,    24,    27,     2,     2,
    26,    29,     1,     3,    26,    29,     1,     3,
    28,    30,     0,     4,    31,    33,     5,     0,
    32,    35,     4,     1,    32,    35,     4,     1,
    34,    37,     3,     2,    34,    37,     3,     2,
    36,    39,     2,     3,    36,    39,     2,     3,
    38,    41,     1,     4,    38,    41,     1,     4,
    40,    42,     0,     5,    43,    33,     6,     0,
    44,    47,     5,     1,    44,    47,     5,     1,
    46,    49,     4,     2,    46,    49,     4,     2,
    48,    51,     3,     3,    48,    51,     3,     3,
    50,    53,     2,     4,    50,    53,     2,     4,
    52,    55,     1,     5,    52,    55,     1,     5,
    40,    56,     0,     6,    57,    45,     7,     0,
    58,    47,     6,     1,    58,    47,     6,     1,
    60,    63,     5,     2,    60,    63,     5,     2,
    62,    65,     4,     3,    62,    65,     4,     3,
    64,    67,     3,     4,    64,    67,     3,     4,
    66,    69,     2,     5,    66,    69,     2,     5,
    52,    71,     1,     6,    52,    71,     1,     6,
    54,    72,     0,     7,    73,    59,     8,     0,
    74,    61,     7,     1,    74,    61,     7,     1,
    76,    63,     6,     2,    76,    63,     6,     2,
    78,    81,     5,     3,    78,    81,     5,     3,
    80,    83,     4,     4,    80,    83,     4,     4,
    82,    85,     3,     5,    82,    85,     3,     5,
    66,    87,     2,     6,    66,    87,     2,     6,
    68,    89,     1,     7,    68,    89,     1,     7,
    70,    90,     0,     8,    91,    59,     9,     0,
    92,    77,     8,     1,    92,    77,     8,     1,
    94,    79,     7,     2,    94,    79,     7,     2,
    96,    81,     6,     3,    96,    81,     6,     3,
    98,   101,     5,     4,    98,   101,     5,     4,
   100,   103,     4,     5,   100,   103,     4,     5,
    82,   105,     3,     6,    82,   105,     3,     6,
    84,   107,     2,     7,    84,   107,     2,     7,
    86,   109,     1,     8,    86,   109,     1,     8,
    70,   110,     0,     9,   111,    59,    10,     0,
   112,    77,     9,     1,   112,    77,     9,     1,
   114,    97,     8,     2,   114,    97,     8,     2,
   116,    99,     7,     3,   116,    99,     7,     3,
    62,   101,     6,     4,    62,   101,     6,     4,
    80,    83,     5,     5,    80,    83,     5,     5,
   100,    67,     4,     6,   100,    67,     4,     6,
   102,   119,     3,     7,   102,   119,     3,     7,
   104,   121,     2,     8,   104,   121,     2,     8,
    86,   123,     1,     9,    86,   123,     1,     9,
    70,   124,     0,    10,   125,    59,    11,     0,
   126,    77,    10,     1,   126,    77,    10,     1,
   128,    97,     9,     2,   128,    97,     9,     2,
    60,    63,     8,     3,    60,    63,     8,     3,
    66,    69,     3,     8,    66,    69,     3,     8,
   104,   131,     2,     9,   104,   131,     2,     9,
    86,   133,     1,    10,    86,   133,     1,    10,
    70,   134,     0,    11,   135,    59,    12,     0,
   136,    77,    11,     1,   136,    77,    11,     1,
   138,    97,    10,     2,   138,    97,    10,     2,
   104,   141,     2,    10,   104,   141,     2,    10,
    86,   143,     1,    11,    86,   143,     1,    11,
    70,   144,     0,    12,   145,    59,    13,     0,
   146,    77,    12,     1,   146,    77,    12,     1,
   148,    97,    11,     2,   148,    97,    11,     2,
   104,   151,     2,    11,   104,   151,     2,    11,
    86,   153,     1,    12,    86,   153,     1,    12,
    70,   154,     0,    13,   155,    59,    14,     0,
   156,    77,    13,     1,   156,    77,    13,     1,
   158,    97,    12,     2,   158,    97,    12,     2,
   104,   161,     2,    12,   104,   161,     2,    12,
    86,   163,     1,    13,    86,   163,     1,    13,
    70,   164,     0,    14,   165,    59,    15,     0,
   166,    77,    14,     1,   166,    77,    14,     1,
   168,    97,    13,     2,   168,    97,    13,     2,
   104,   171,     2,    13,   104,   171,     2,    13,
    86,   173,     1,    14,    86,   173,     1,    14,
    70,   174,     0,    15,   175,    59,    16,     0,
   176,    77,    15,     1,   176,    77,    15,     1,
   178,    97,    14,     2,   178,    97,    14,     2,
   104,   181,     2,    14,   104,   181,     2,    14,
    86,   183,     1,    15,    86,   183,     1,    15,
    70,   184,     0,    16,   185,    59,    17,     0,
   186,    77,    16,     1,   186,    77,    16,     1,
    74,    97,    15,     2,    74,    97,    15,     2,
   104,    89,     2,    15,   104,    89,     2,    15,
    86,   187,     1,    16,    86,   187,     1,    16,
    70,   188,     0,    17,   189,    59,    18,     0,
   190,    77,    17,     1,    86,   191,     1,    17,
    70,   192,     0,    18,   193,    59,    19,     0,
   194,    77,    18,     1,    86,   195,     1,    18,
    70,   196,     0,    19,   193,    59,    20,     0,
   197,    77,    19,     1,    86,   198,     1,    19,
    70,   196,     0,    20,   199,    77,    20,     1,
    86,   200,     1,    20,   201,    77,    21,     1,
    86,   202,     1,    21,   203,    77,    22,     1,
    86,   204,     1,    22,   205,    77,    23,     1,
    86,   206,     1,    23,   207,    77,    24,     1,
    86,   208,     1,    24,   209,    77,    25,     1,
    86,   210,     1,    25,   211,    77,    26,     1,
    86,   212,     1,    26,   213,    77,    27,     1,
    86,   214,     1,    27,   215,    77,    28,     1,
    86,   216,     1,    28,   217,    77,    29,     1,
    86,   218,     1,    29,   219,    77,    30,     1,
    86,   220,     1,    30,   221,    77,    31,     1,
    86,   222,     1,    31,   223,    77,    32,     1,
    86,   224,     1,    32,   225,    77,    33,     1,
    86,   226,     1,    33,   227,    77,    34,     1,
    86,   228,     1,    34,   229,    77,    35,     1,
    86,   230,     1,    35,   231,    77,    36,     1,
    86,   232,     1,    36,   233,    77,    37,     1,
    86,   234,     1,    37,   235,    77,    38,     1,
    86,   236,     1,    38,   237,    77,    39,     1,
    86,   238,     1,    39,   239,    77,    40,     1,
    86,   240,     1,    40,   241,    77,    41,     1,
    86,   242,     1,    41,   243,    77,    42,     1,
    86,   244,     1,    42,   245,    77,    43,     1,
    86,   246,     1,    43,   247,    77,    44,     1,
    86,   248,     1,    44,   249,    77,    45,     1,
    86,   250,     1,    45,   251,    77,    46,     1,
    86,   252,     1,    46,   253,    77,    47,     1,
    86,   254,     1,    47,   253,    77,    48,     1,
    86,   254,     1,    48,     0,     0,     0,     0
};

// Initialize next state table ns[state*4] -> next if 0, next if 1, n0, n1
StateTable::StateTable() {
  memcpy(ns, sns, sizeof(ns));
}

/////////////////////////// ZPAQL //////////////////////////

// Write header to out2, return true if HCOMP/PCOMP section is present.
// If pp is true, then write only the postprocessor code.
bool ZPAQL::write(Writer* out2, bool pp) {
  if (header.size()<=6) return false;
  assert(header[0]+256*header[1]==cend-2+hend-hbegin);
  assert(cend>=7);
  assert(hbegin>=cend);
  assert(hend>=hbegin);
  assert(out2);
  if (!pp) {  // if not a postprocessor then write COMP
    for (int i=0; i<cend; ++i)
      out2->put(header[i]);
  }
  else {  // write PCOMP size only
    out2->put((hend-hbegin)&255);
    out2->put((hend-hbegin)>>8);
  }
  for (int i=hbegin; i<hend; ++i)
    out2->put(header[i]);
  return true;
}

// Read header from in2
int ZPAQL::read(Reader* in2) {

  // Get header size and allocate
  int hsize=in2->get();
  hsize+=in2->get()*256;
  header.resize(hsize+300);
  cend=hbegin=hend=0;
  header[cend++]=hsize&255;
  header[cend++]=hsize>>8;
  while (cend<7) header[cend++]=in2->get(); // hh hm ph pm n

  // Read COMP
  int n=header[cend-1];
  for (int i=0; i<n; ++i) {
    int type=in2->get();  // component type
    if (type<0 || type>255) error("unexpected end of file");
    header[cend++]=type;  // component type
    int size=compsize[type];
    if (size<1) error("Invalid component type");
    if (cend+size>hsize) error("COMP overflows header");
    for (int j=1; j<size; ++j)
      header[cend++]=in2->get();
  }
  if ((header[cend++]=in2->get())!=0) error("missing COMP END");

  // Insert a guard gap and read HCOMP
  hbegin=hend=cend+128;
  if (hend>hsize+129) error("missing HCOMP");
  while (hend<hsize+129) {
    assert(hend<header.isize()-8);
    int op=in2->get();
    if (op==-1) error("unexpected end of file");
    header[hend++]=op;
  }
  if ((header[hend++]=in2->get())!=0) error("missing HCOMP END");
  assert(cend>=7 && cend<header.isize());
  assert(hbegin==cend+128 && hbegin<header.isize());
  assert(hend>hbegin && hend<header.isize());
  assert(hsize==header[0]+256*header[1]);
  assert(hsize==cend-2+hend-hbegin);
  allocx(rcode, rcode_size, 0);  // clear JIT code
  return cend+hend-hbegin;
}

// Free memory, but preserve output, sha1 pointers
void ZPAQL::clear() {
  cend=hbegin=hend=0;  // COMP and HCOMP locations
  a=b=c=d=f=pc=0;      // machine state
  header.resize(0);
  h.resize(0);
  m.resize(0);
  r.resize(0);
  allocx(rcode, rcode_size, 0);
}

// Constructor
ZPAQL::ZPAQL() {
  output=0;
  sha1=0;
  rcode=0;
  rcode_size=0;
  clear();
  outbuf.resize(1<<14);
  bufptr=0;
}

ZPAQL::~ZPAQL() {
  allocx(rcode, rcode_size, 0);
}

// Initialize machine state as HCOMP
void ZPAQL::inith() {
  assert(header.isize()>6);
  assert(output==0);
  assert(sha1==0);
  init(header[2], header[3]); // hh, hm
}

// Initialize machine state as PCOMP
void ZPAQL::initp() {
  assert(header.isize()>6);
  init(header[4], header[5]); // ph, pm
}

// Flush pending output
void ZPAQL::flush() {
  if (output) output->write(&outbuf[0], bufptr);
  if (sha1) sha1->write(&outbuf[0], bufptr);
  bufptr=0;
}

// pow(2, x)
static double pow2(int x) {
  double r=1;
  for (; x>0; x--) r+=r;
  return r;
}

// Return memory requirement in bytes
double ZPAQL::memory() {
  double mem=pow2(header[2]+2)+pow2(header[3])  // hh hm
            +pow2(header[4]+2)+pow2(header[5])  // ph pm
            +header.size();
  int cp=7;  // start of comp list
  for (int i=0; i<header[6]; ++i) {  // n
    assert(cp<cend);
    double size=pow2(header[cp+1]); // sizebits
    switch(header[cp]) {
      case CM: mem+=4*size; break;
      case ICM: mem+=64*size+1024; break;
      case MATCH: mem+=4*size+pow2(header[cp+2]); break; // bufbits
      case MIX2: mem+=2*size; break;
      case MIX: mem+=4*size*header[cp+3]; break; // m
      case ISSE: mem+=64*size+2048; break;
      case SSE: mem+=128*size; break;
    }
    cp+=compsize[header[cp]];
  }
  return mem;
}

// Initialize machine state to run a program.
void ZPAQL::init(int hbits, int mbits) {
  assert(header.isize()>0);
  assert(cend>=7);
  assert(hbegin>=cend+128);
  assert(hend>=hbegin);
  assert(hend<header.isize()-130);
  assert(header[0]+256*header[1]==cend-2+hend-hbegin);
  assert(bufptr==0);
  assert(outbuf.isize()>0);
  if (hbits>32) error("H too big");
  if (mbits>32) error("M too big");
  h.resize(1, hbits);
  m.resize(1, mbits);
  r.resize(256);
  a=b=c=d=pc=f=0;
}

// Run program on input by interpreting header
void ZPAQL::run0(U32 input) {
  assert(cend>6);
  assert(hbegin>=cend+128);
  assert(hend>=hbegin);
  assert(hend<header.isize()-130);
  assert(m.size()>0);
  assert(h.size()>0);
  assert(header[0]+256*header[1]==cend+hend-hbegin-2);
  pc=hbegin;
  a=input;
  while (execute()) ;
}

// Execute one instruction, return 0 after HALT else 1
int ZPAQL::execute() {
  switch(header[pc++]) {
    case 0: err(); break; // ERROR
    case 1: ++a; break; // A++
    case 2: --a; break; // A--
    case 3: a = ~a; break; // A!
    case 4: a = 0; break; // A=0
    case 7: a = r[header[pc++]]; break; // A=R N
    case 8: swap(b); break; // B<>A
    case 9: ++b; break; // B++
    case 10: --b; break; // B--
    case 11: b = ~b; break; // B!
    case 12: b = 0; break; // B=0
    case 15: b = r[header[pc++]]; break; // B=R N
    case 16: swap(c); break; // C<>A
    case 17: ++c; break; // C++
    case 18: --c; break; // C--
    case 19: c = ~c; break; // C!
    case 20: c = 0; break; // C=0
    case 23: c = r[header[pc++]]; break; // C=R N
    case 24: swap(d); break; // D<>A
    case 25: ++d; break; // D++
    case 26: --d; break; // D--
    case 27: d = ~d; break; // D!
    case 28: d = 0; break; // D=0
    case 31: d = r[header[pc++]]; break; // D=R N
    case 32: swap(m(b)); break; // *B<>A
    case 33: ++m(b); break; // *B++
    case 34: --m(b); break; // *B--
    case 35: m(b) = ~m(b); break; // *B!
    case 36: m(b) = 0; break; // *B=0
    case 39: if (f) pc+=((header[pc]+128)&255)-127; else ++pc; break; // JT N
    case 40: swap(m(c)); break; // *C<>A
    case 41: ++m(c); break; // *C++
    case 42: --m(c); break; // *C--
    case 43: m(c) = ~m(c); break; // *C!
    case 44: m(c) = 0; break; // *C=0
    case 47: if (!f) pc+=((header[pc]+128)&255)-127; else ++pc; break; // JF N
    case 48: swap(h(d)); break; // *D<>A
    case 49: ++h(d); break; // *D++
    case 50: --h(d); break; // *D--
    case 51: h(d) = ~h(d); break; // *D!
    case 52: h(d) = 0; break; // *D=0
    case 55: r[header[pc++]] = a; break; // R=A N
    case 56: return 0  ; // HALT
    case 57: outc(a&255); break; // OUT
    case 59: a = (a+m(b)+512)*773; break; // HASH
    case 60: h(d) = (h(d)+a+512)*773; break; // HASHD
    case 63: pc+=((header[pc]+128)&255)-127; break; // JMP N
    case 64: break; // A=A
    case 65: a = b; break; // A=B
    case 66: a = c; break; // A=C
    case 67: a = d; break; // A=D
    case 68: a = m(b); break; // A=*B
    case 69: a = m(c); break; // A=*C
    case 70: a = h(d); break; // A=*D
    case 71: a = header[pc++]; break; // A= N
    case 72: b = a; break; // B=A
    case 73: break; // B=B
    case 74: b = c; break; // B=C
    case 75: b = d; break; // B=D
    case 76: b = m(b); break; // B=*B
    case 77: b = m(c); break; // B=*C
    case 78: b = h(d); break; // B=*D
    case 79: b = header[pc++]; break; // B= N
    case 80: c = a; break; // C=A
    case 81: c = b; break; // C=B
    case 82: break; // C=C
    case 83: c = d; break; // C=D
    case 84: c = m(b); break; // C=*B
    case 85: c = m(c); break; // C=*C
    case 86: c = h(d); break; // C=*D
    case 87: c = header[pc++]; break; // C= N
    case 88: d = a; break; // D=A
    case 89: d = b; break; // D=B
    case 90: d = c; break; // D=C
    case 91: break; // D=D
    case 92: d = m(b); break; // D=*B
    case 93: d = m(c); break; // D=*C
    case 94: d = h(d); break; // D=*D
    case 95: d = header[pc++]; break; // D= N
    case 96: m(b) = a; break; // *B=A
    case 97: m(b) = b; break; // *B=B
    case 98: m(b) = c; break; // *B=C
    case 99: m(b) = d; break; // *B=D
    case 100: break; // *B=*B
    case 101: m(b) = m(c); break; // *B=*C
    case 102: m(b) = h(d); break; // *B=*D
    case 103: m(b) = header[pc++]; break; // *B= N
    case 104: m(c) = a; break; // *C=A
    case 105: m(c) = b; break; // *C=B
    case 106: m(c) = c; break; // *C=C
    case 107: m(c) = d; break; // *C=D
    case 108: m(c) = m(b); break; // *C=*B
    case 109: break; // *C=*C
    case 110: m(c) = h(d); break; // *C=*D
    case 111: m(c) = header[pc++]; break; // *C= N
    case 112: h(d) = a; break; // *D=A
    case 113: h(d) = b; break; // *D=B
    case 114: h(d) = c; break; // *D=C
    case 115: h(d) = d; break; // *D=D
    case 116: h(d) = m(b); break; // *D=*B
    case 117: h(d) = m(c); break; // *D=*C
    case 118: break; // *D=*D
    case 119: h(d) = header[pc++]; break; // *D= N
    case 128: a += a; break; // A+=A
    case 129: a += b; break; // A+=B
    case 130: a += c; break; // A+=C
    case 131: a += d; break; // A+=D
    case 132: a += m(b); break; // A+=*B
    case 133: a += m(c); break; // A+=*C
    case 134: a += h(d); break; // A+=*D
    case 135: a += header[pc++]; break; // A+= N
    case 136: a -= a; break; // A-=A
    case 137: a -= b; break; // A-=B
    case 138: a -= c; break; // A-=C
    case 139: a -= d; break; // A-=D
    case 140: a -= m(b); break; // A-=*B
    case 141: a -= m(c); break; // A-=*C
    case 142: a -= h(d); break; // A-=*D
    case 143: a -= header[pc++]; break; // A-= N
    case 144: a *= a; break; // A*=A
    case 145: a *= b; break; // A*=B
    case 146: a *= c; break; // A*=C
    case 147: a *= d; break; // A*=D
    case 148: a *= m(b); break; // A*=*B
    case 149: a *= m(c); break; // A*=*C
    case 150: a *= h(d); break; // A*=*D
    case 151: a *= header[pc++]; break; // A*= N
    case 152: div(a); break; // A/=A
    case 153: div(b); break; // A/=B
    case 154: div(c); break; // A/=C
    case 155: div(d); break; // A/=D
    case 156: div(m(b)); break; // A/=*B
    case 157: div(m(c)); break; // A/=*C
    case 158: div(h(d)); break; // A/=*D
    case 159: div(header[pc++]); break; // A/= N
    case 160: mod(a); break; // A%=A
    case 161: mod(b); break; // A%=B
    case 162: mod(c); break; // A%=C
    case 163: mod(d); break; // A%=D
    case 164: mod(m(b)); break; // A%=*B
    case 165: mod(m(c)); break; // A%=*C
    case 166: mod(h(d)); break; // A%=*D
    case 167: mod(header[pc++]); break; // A%= N
    case 168: a &= a; break; // A&=A
    case 169: a &= b; break; // A&=B
    case 170: a &= c; break; // A&=C
    case 171: a &= d; break; // A&=D
    case 172: a &= m(b); break; // A&=*B
    case 173: a &= m(c); break; // A&=*C
    case 174: a &= h(d); break; // A&=*D
    case 175: a &= header[pc++]; break; // A&= N
    case 176: a &= ~ a; break; // A&~A
    case 177: a &= ~ b; break; // A&~B
    case 178: a &= ~ c; break; // A&~C
    case 179: a &= ~ d; break; // A&~D
    case 180: a &= ~ m(b); break; // A&~*B
    case 181: a &= ~ m(c); break; // A&~*C
    case 182: a &= ~ h(d); break; // A&~*D
    case 183: a &= ~ header[pc++]; break; // A&~ N
    case 184: a |= a; break; // A|=A
    case 185: a |= b; break; // A|=B
    case 186: a |= c; break; // A|=C
    case 187: a |= d; break; // A|=D
    case 188: a |= m(b); break; // A|=*B
    case 189: a |= m(c); break; // A|=*C
    case 190: a |= h(d); break; // A|=*D
    case 191: a |= header[pc++]; break; // A|= N
    case 192: a ^= a; break; // A^=A
    case 193: a ^= b; break; // A^=B
    case 194: a ^= c; break; // A^=C
    case 195: a ^= d; break; // A^=D
    case 196: a ^= m(b); break; // A^=*B
    case 197: a ^= m(c); break; // A^=*C
    case 198: a ^= h(d); break; // A^=*D
    case 199: a ^= header[pc++]; break; // A^= N
    case 200: a <<= (a&31); break; // A<<=A
    case 201: a <<= (b&31); break; // A<<=B
    case 202: a <<= (c&31); break; // A<<=C
    case 203: a <<= (d&31); break; // A<<=D
    case 204: a <<= (m(b)&31); break; // A<<=*B
    case 205: a <<= (m(c)&31); break; // A<<=*C
    case 206: a <<= (h(d)&31); break; // A<<=*D
    case 207: a <<= (header[pc++]&31); break; // A<<= N
    case 208: a >>= (a&31); break; // A>>=A
    case 209: a >>= (b&31); break; // A>>=B
    case 210: a >>= (c&31); break; // A>>=C
    case 211: a >>= (d&31); break; // A>>=D
    case 212: a >>= (m(b)&31); break; // A>>=*B
    case 213: a >>= (m(c)&31); break; // A>>=*C
    case 214: a >>= (h(d)&31); break; // A>>=*D
    case 215: a >>= (header[pc++]&31); break; // A>>= N
    case 216: f = 1; break; // A==A
    case 217: f = (a == b); break; // A==B
    case 218: f = (a == c); break; // A==C
    case 219: f = (a == d); break; // A==D
    case 220: f = (a == U32(m(b))); break; // A==*B
    case 221: f = (a == U32(m(c))); break; // A==*C
    case 222: f = (a == h(d)); break; // A==*D
    case 223: f = (a == U32(header[pc++])); break; // A== N
    case 224: f = 0; break; // A<A
    case 225: f = (a < b); break; // A<B
    case 226: f = (a < c); break; // A<C
    case 227: f = (a < d); break; // A<D
    case 228: f = (a < U32(m(b))); break; // A<*B
    case 229: f = (a < U32(m(c))); break; // A<*C
    case 230: f = (a < h(d)); break; // A<*D
    case 231: f = (a < U32(header[pc++])); break; // A< N
    case 232: f = 0; break; // A>A
    case 233: f = (a > b); break; // A>B
    case 234: f = (a > c); break; // A>C
    case 235: f = (a > d); break; // A>D
    case 236: f = (a > U32(m(b))); break; // A>*B
    case 237: f = (a > U32(m(c))); break; // A>*C
    case 238: f = (a > h(d)); break; // A>*D
    case 239: f = (a > U32(header[pc++])); break; // A> N
    case 255: if((pc=hbegin+header[pc]+256*header[pc+1])>=hend)err();break;//LJ
    default: err();
  }
  return 1;
}

// Print illegal instruction error message and exit
void ZPAQL::err() {
  error("ZPAQL execution error");
}

///////////////////////// Predictor /////////////////////////

// sdt2k[i]=2048/i;
static const int sdt2k[256]={
     0,  2048,  1024,   682,   512,   409,   341,   292,
   256,   227,   204,   186,   170,   157,   146,   136,
   128,   120,   113,   107,   102,    97,    93,    89,
    85,    81,    78,    75,    73,    70,    68,    66,
    64,    62,    60,    58,    56,    55,    53,    52,
    51,    49,    48,    47,    46,    45,    44,    43,
    42,    41,    40,    40,    39,    38,    37,    37,
    36,    35,    35,    34,    34,    33,    33,    32,
    32,    31,    31,    30,    30,    29,    29,    28,
    28,    28,    27,    27,    26,    26,    26,    25,
    25,    25,    24,    24,    24,    24,    23,    23,
    23,    23,    22,    22,    22,    22,    21,    21,
    21,    21,    20,    20,    20,    20,    20,    19,
    19,    19,    19,    19,    18,    18,    18,    18,
    18,    18,    17,    17,    17,    17,    17,    17,
    17,    16,    16,    16,    16,    16,    16,    16,
    16,    15,    15,    15,    15,    15,    15,    15,
    15,    14,    14,    14,    14,    14,    14,    14,
    14,    14,    14,    13,    13,    13,    13,    13,
    13,    13,    13,    13,    13,    13,    12,    12,
    12,    12,    12,    12,    12,    12,    12,    12,
    12,    12,    12,    11,    11,    11,    11,    11,
    11,    11,    11,    11,    11,    11,    11,    11,
    11,    11,    11,    10,    10,    10,    10,    10,
    10,    10,    10,    10,    10,    10,    10,    10,
    10,    10,    10,    10,    10,     9,     9,     9,
     9,     9,     9,     9,     9,     9,     9,     9,
     9,     9,     9,     9,     9,     9,     9,     9,
     9,     9,     9,     9,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,     8,     8,     8,     8,     8,     8
};

// sdt[i]=(1<<17)/(i*2+3)*2;
static const int sdt[1024]={
 87380, 52428, 37448, 29126, 23830, 20164, 17476, 15420,
 13796, 12482, 11396, 10484,  9708,  9038,  8456,  7942,
  7488,  7084,  6720,  6392,  6096,  5824,  5576,  5348,
  5140,  4946,  4766,  4598,  4442,  4296,  4160,  4032,
  3912,  3798,  3692,  3590,  3494,  3404,  3318,  3236,
  3158,  3084,  3012,  2944,  2880,  2818,  2758,  2702,
  2646,  2594,  2544,  2496,  2448,  2404,  2360,  2318,
  2278,  2240,  2202,  2166,  2130,  2096,  2064,  2032,
  2000,  1970,  1940,  1912,  1884,  1858,  1832,  1806,
  1782,  1758,  1736,  1712,  1690,  1668,  1648,  1628,
  1608,  1588,  1568,  1550,  1532,  1514,  1496,  1480,
  1464,  1448,  1432,  1416,  1400,  1386,  1372,  1358,
  1344,  1330,  1316,  1304,  1290,  1278,  1266,  1254,
  1242,  1230,  1218,  1208,  1196,  1186,  1174,  1164,
  1154,  1144,  1134,  1124,  1114,  1106,  1096,  1086,
  1078,  1068,  1060,  1052,  1044,  1036,  1028,  1020,
  1012,  1004,   996,   988,   980,   974,   966,   960,
   952,   946,   938,   932,   926,   918,   912,   906,
   900,   894,   888,   882,   876,   870,   864,   858,
   852,   848,   842,   836,   832,   826,   820,   816,
   810,   806,   800,   796,   790,   786,   782,   776,
   772,   768,   764,   758,   754,   750,   746,   742,
   738,   734,   730,   726,   722,   718,   714,   710,
   706,   702,   698,   694,   690,   688,   684,   680,
   676,   672,   670,   666,   662,   660,   656,   652,
   650,   646,   644,   640,   636,   634,   630,   628,
   624,   622,   618,   616,   612,   610,   608,   604,
   602,   598,   596,   594,   590,   588,   586,   582,
   580,   578,   576,   572,   570,   568,   566,   562,
   560,   558,   556,   554,   550,   548,   546,   544,
   542,   540,   538,   536,   532,   530,   528,   526,
   524,   522,   520,   518,   516,   514,   512,   510,
   508,   506,   504,   502,   500,   498,   496,   494,
   492,   490,   488,   488,   486,   484,   482,   480,
   478,   476,   474,   474,   472,   470,   468,   466,
   464,   462,   462,   460,   458,   456,   454,   454,
   452,   450,   448,   448,   446,   444,   442,   442,
   440,   438,   436,   436,   434,   432,   430,   430,
   428,   426,   426,   424,   422,   422,   420,   418,
   418,   416,   414,   414,   412,   410,   410,   408,
   406,   406,   404,   402,   402,   400,   400,   398,
   396,   396,   394,   394,   392,   390,   390,   388,
   388,   386,   386,   384,   382,   382,   380,   380,
   378,   378,   376,   376,   374,   372,   372,   370,
   370,   368,   368,   366,   366,   364,   364,   362,
   362,   360,   360,   358,   358,   356,   356,   354,
   354,   352,   352,   350,   350,   348,   348,   348,
   346,   346,   344,   344,   342,   342,   340,   340,
   340,   338,   338,   336,   336,   334,   334,   332,
   332,   332,   330,   330,   328,   328,   328,   326,
   326,   324,   324,   324,   322,   322,   320,   320,
   320,   318,   318,   316,   316,   316,   314,   314,
   312,   312,   312,   310,   310,   310,   308,   308,
   308,   306,   306,   304,   304,   304,   302,   302,
   302,   300,   300,   300,   298,   298,   298,   296,
   296,   296,   294,   294,   294,   292,   292,   292,
   290,   290,   290,   288,   288,   288,   286,   286,
   286,   284,   284,   284,   284,   282,   282,   282,
   280,   280,   280,   278,   278,   278,   276,   276,
   276,   276,   274,   274,   274,   272,   272,   272,
   272,   270,   270,   270,   268,   268,   268,   268,
   266,   266,   266,   266,   264,   264,   264,   262,
   262,   262,   262,   260,   260,   260,   260,   258,
   258,   258,   258,   256,   256,   256,   256,   254,
   254,   254,   254,   252,   252,   252,   252,   250,
   250,   250,   250,   248,   248,   248,   248,   248,
   246,   246,   246,   246,   244,   244,   244,   244,
   242,   242,   242,   242,   242,   240,   240,   240,
   240,   238,   238,   238,   238,   238,   236,   236,
   236,   236,   234,   234,   234,   234,   234,   232,
   232,   232,   232,   232,   230,   230,   230,   230,
   230,   228,   228,   228,   228,   228,   226,   226,
   226,   226,   226,   224,   224,   224,   224,   224,
   222,   222,   222,   222,   222,   220,   220,   220,
   220,   220,   220,   218,   218,   218,   218,   218,
   216,   216,   216,   216,   216,   216,   214,   214,
   214,   214,   214,   212,   212,   212,   212,   212,
   212,   210,   210,   210,   210,   210,   210,   208,
   208,   208,   208,   208,   208,   206,   206,   206,
   206,   206,   206,   204,   204,   204,   204,   204,
   204,   204,   202,   202,   202,   202,   202,   202,
   200,   200,   200,   200,   200,   200,   198,   198,
   198,   198,   198,   198,   198,   196,   196,   196,
   196,   196,   196,   196,   194,   194,   194,   194,
   194,   194,   194,   192,   192,   192,   192,   192,
   192,   192,   190,   190,   190,   190,   190,   190,
   190,   188,   188,   188,   188,   188,   188,   188,
   186,   186,   186,   186,   186,   186,   186,   186,
   184,   184,   184,   184,   184,   184,   184,   182,
   182,   182,   182,   182,   182,   182,   182,   180,
   180,   180,   180,   180,   180,   180,   180,   178,
   178,   178,   178,   178,   178,   178,   178,   176,
   176,   176,   176,   176,   176,   176,   176,   176,
   174,   174,   174,   174,   174,   174,   174,   174,
   172,   172,   172,   172,   172,   172,   172,   172,
   172,   170,   170,   170,   170,   170,   170,   170,
   170,   170,   168,   168,   168,   168,   168,   168,
   168,   168,   168,   166,   166,   166,   166,   166,
   166,   166,   166,   166,   166,   164,   164,   164,
   164,   164,   164,   164,   164,   164,   162,   162,
   162,   162,   162,   162,   162,   162,   162,   162,
   160,   160,   160,   160,   160,   160,   160,   160,
   160,   160,   158,   158,   158,   158,   158,   158,
   158,   158,   158,   158,   158,   156,   156,   156,
   156,   156,   156,   156,   156,   156,   156,   154,
   154,   154,   154,   154,   154,   154,   154,   154,
   154,   154,   152,   152,   152,   152,   152,   152,
   152,   152,   152,   152,   152,   150,   150,   150,
   150,   150,   150,   150,   150,   150,   150,   150,
   150,   148,   148,   148,   148,   148,   148,   148,
   148,   148,   148,   148,   148,   146,   146,   146,
   146,   146,   146,   146,   146,   146,   146,   146,
   146,   144,   144,   144,   144,   144,   144,   144,
   144,   144,   144,   144,   144,   142,   142,   142,
   142,   142,   142,   142,   142,   142,   142,   142,
   142,   142,   140,   140,   140,   140,   140,   140,
   140,   140,   140,   140,   140,   140,   140,   138,
   138,   138,   138,   138,   138,   138,   138,   138,
   138,   138,   138,   138,   138,   136,   136,   136,
   136,   136,   136,   136,   136,   136,   136,   136,
   136,   136,   136,   134,   134,   134,   134,   134,
   134,   134,   134,   134,   134,   134,   134,   134,
   134,   132,   132,   132,   132,   132,   132,   132,
   132,   132,   132,   132,   132,   132,   132,   132,
   130,   130,   130,   130,   130,   130,   130,   130,
   130,   130,   130,   130,   130,   130,   130,   128,
   128,   128,   128,   128,   128,   128,   128,   128,
   128,   128,   128,   128,   128,   128,   128,   126
};

// ssquasht[i]=int(32768.0/(1+exp((i-2048)*(-1.0/64))));
// Middle 1344 of 4096 entries only.
static const U16 ssquasht[1344]={
     0,     0,     0,     0,     0,     0,     0,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,
     4,     4,     4,     4,     4,     4,     4,     4,
     4,     4,     4,     4,     4,     4,     5,     5,
     5,     5,     5,     5,     5,     5,     5,     5,
     5,     5,     6,     6,     6,     6,     6,     6,
     6,     6,     6,     6,     7,     7,     7,     7,
     7,     7,     7,     7,     8,     8,     8,     8,
     8,     8,     8,     8,     9,     9,     9,     9,
     9,     9,    10,    10,    10,    10,    10,    10,
    10,    11,    11,    11,    11,    11,    12,    12,
    12,    12,    12,    13,    13,    13,    13,    13,
    14,    14,    14,    14,    15,    15,    15,    15,
    15,    16,    16,    16,    17,    17,    17,    17,
    18,    18,    18,    18,    19,    19,    19,    20,
    20,    20,    21,    21,    21,    22,    22,    22,
    23,    23,    23,    24,    24,    25,    25,    25,
    26,    26,    27,    27,    28,    28,    28,    29,
    29,    30,    30,    31,    31,    32,    32,    33,
    33,    34,    34,    35,    36,    36,    37,    37,
    38,    38,    39,    40,    40,    41,    42,    42,
    43,    44,    44,    45,    46,    46,    47,    48,
    49,    49,    50,    51,    52,    53,    54,    54,
    55,    56,    57,    58,    59,    60,    61,    62,
    63,    64,    65,    66,    67,    68,    69,    70,
    71,    72,    73,    74,    76,    77,    78,    79,
    81,    82,    83,    84,    86,    87,    88,    90,
    91,    93,    94,    96,    97,    99,   100,   102,
   103,   105,   107,   108,   110,   112,   114,   115,
   117,   119,   121,   123,   125,   127,   129,   131,
   133,   135,   137,   139,   141,   144,   146,   148,
   151,   153,   155,   158,   160,   163,   165,   168,
   171,   173,   176,   179,   182,   184,   187,   190,
   193,   196,   199,   202,   206,   209,   212,   215,
   219,   222,   226,   229,   233,   237,   240,   244,
   248,   252,   256,   260,   264,   268,   272,   276,
   281,   285,   289,   294,   299,   303,   308,   313,
   318,   323,   328,   333,   338,   343,   349,   354,
   360,   365,   371,   377,   382,   388,   394,   401,
   407,   413,   420,   426,   433,   440,   446,   453,
   460,   467,   475,   482,   490,   497,   505,   513,
   521,   529,   537,   545,   554,   562,   571,   580,
   589,   598,   607,   617,   626,   636,   646,   656,
   666,   676,   686,   697,   708,   719,   730,   741,
   752,   764,   776,   788,   800,   812,   825,   837,
   850,   863,   876,   890,   903,   917,   931,   946,
   960,   975,   990,  1005,  1020,  1036,  1051,  1067,
  1084,  1100,  1117,  1134,  1151,  1169,  1186,  1204,
  1223,  1241,  1260,  1279,  1298,  1318,  1338,  1358,
  1379,  1399,  1421,  1442,  1464,  1486,  1508,  1531,
  1554,  1577,  1600,  1624,  1649,  1673,  1698,  1724,
  1749,  1775,  1802,  1829,  1856,  1883,  1911,  1940,
  1968,  1998,  2027,  2057,  2087,  2118,  2149,  2181,
  2213,  2245,  2278,  2312,  2345,  2380,  2414,  2450,
  2485,  2521,  2558,  2595,  2633,  2671,  2709,  2748,
  2788,  2828,  2869,  2910,  2952,  2994,  3037,  3080,
  3124,  3168,  3213,  3259,  3305,  3352,  3399,  3447,
  3496,  3545,  3594,  3645,  3696,  3747,  3799,  3852,
  3906,  3960,  4014,  4070,  4126,  4182,  4240,  4298,
  4356,  4416,  4476,  4537,  4598,  4660,  4723,  4786,
  4851,  4916,  4981,  5048,  5115,  5183,  5251,  5320,
  5390,  5461,  5533,  5605,  5678,  5752,  5826,  5901,
  5977,  6054,  6131,  6210,  6289,  6369,  6449,  6530,
  6613,  6695,  6779,  6863,  6949,  7035,  7121,  7209,
  7297,  7386,  7476,  7566,  7658,  7750,  7842,  7936,
  8030,  8126,  8221,  8318,  8415,  8513,  8612,  8712,
  8812,  8913,  9015,  9117,  9221,  9324,  9429,  9534,
  9640,  9747,  9854,  9962, 10071, 10180, 10290, 10401,
 10512, 10624, 10737, 10850, 10963, 11078, 11192, 11308,
 11424, 11540, 11658, 11775, 11893, 12012, 12131, 12251,
 12371, 12491, 12612, 12734, 12856, 12978, 13101, 13224,
 13347, 13471, 13595, 13719, 13844, 13969, 14095, 14220,
 14346, 14472, 14599, 14725, 14852, 14979, 15106, 15233,
 15361, 15488, 15616, 15744, 15872, 16000, 16128, 16256,
 16384, 16511, 16639, 16767, 16895, 17023, 17151, 17279,
 17406, 17534, 17661, 17788, 17915, 18042, 18168, 18295,
 18421, 18547, 18672, 18798, 18923, 19048, 19172, 19296,
 19420, 19543, 19666, 19789, 19911, 20033, 20155, 20276,
 20396, 20516, 20636, 20755, 20874, 20992, 21109, 21227,
 21343, 21459, 21575, 21689, 21804, 21917, 22030, 22143,
 22255, 22366, 22477, 22587, 22696, 22805, 22913, 23020,
 23127, 23233, 23338, 23443, 23546, 23650, 23752, 23854,
 23955, 24055, 24155, 24254, 24352, 24449, 24546, 24641,
 24737, 24831, 24925, 25017, 25109, 25201, 25291, 25381,
 25470, 25558, 25646, 25732, 25818, 25904, 25988, 26072,
 26154, 26237, 26318, 26398, 26478, 26557, 26636, 26713,
 26790, 26866, 26941, 27015, 27089, 27162, 27234, 27306,
 27377, 27447, 27516, 27584, 27652, 27719, 27786, 27851,
 27916, 27981, 28044, 28107, 28169, 28230, 28291, 28351,
 28411, 28469, 28527, 28585, 28641, 28697, 28753, 28807,
 28861, 28915, 28968, 29020, 29071, 29122, 29173, 29222,
 29271, 29320, 29368, 29415, 29462, 29508, 29554, 29599,
 29643, 29687, 29730, 29773, 29815, 29857, 29898, 29939,
 29979, 30019, 30058, 30096, 30134, 30172, 30209, 30246,
 30282, 30317, 30353, 30387, 30422, 30455, 30489, 30522,
 30554, 30586, 30618, 30649, 30680, 30710, 30740, 30769,
 30799, 30827, 30856, 30884, 30911, 30938, 30965, 30992,
 31018, 31043, 31069, 31094, 31118, 31143, 31167, 31190,
 31213, 31236, 31259, 31281, 31303, 31325, 31346, 31368,
 31388, 31409, 31429, 31449, 31469, 31488, 31507, 31526,
 31544, 31563, 31581, 31598, 31616, 31633, 31650, 31667,
 31683, 31700, 31716, 31731, 31747, 31762, 31777, 31792,
 31807, 31821, 31836, 31850, 31864, 31877, 31891, 31904,
 31917, 31930, 31942, 31955, 31967, 31979, 31991, 32003,
 32015, 32026, 32037, 32048, 32059, 32070, 32081, 32091,
 32101, 32111, 32121, 32131, 32141, 32150, 32160, 32169,
 32178, 32187, 32196, 32205, 32213, 32222, 32230, 32238,
 32246, 32254, 32262, 32270, 32277, 32285, 32292, 32300,
 32307, 32314, 32321, 32327, 32334, 32341, 32347, 32354,
 32360, 32366, 32373, 32379, 32385, 32390, 32396, 32402,
 32407, 32413, 32418, 32424, 32429, 32434, 32439, 32444,
 32449, 32454, 32459, 32464, 32468, 32473, 32478, 32482,
 32486, 32491, 32495, 32499, 32503, 32507, 32511, 32515,
 32519, 32523, 32527, 32530, 32534, 32538, 32541, 32545,
 32548, 32552, 32555, 32558, 32561, 32565, 32568, 32571,
 32574, 32577, 32580, 32583, 32585, 32588, 32591, 32594,
 32596, 32599, 32602, 32604, 32607, 32609, 32612, 32614,
 32616, 32619, 32621, 32623, 32626, 32628, 32630, 32632,
 32634, 32636, 32638, 32640, 32642, 32644, 32646, 32648,
 32650, 32652, 32653, 32655, 32657, 32659, 32660, 32662,
 32664, 32665, 32667, 32668, 32670, 32671, 32673, 32674,
 32676, 32677, 32679, 32680, 32681, 32683, 32684, 32685,
 32686, 32688, 32689, 32690, 32691, 32693, 32694, 32695,
 32696, 32697, 32698, 32699, 32700, 32701, 32702, 32703,
 32704, 32705, 32706, 32707, 32708, 32709, 32710, 32711,
 32712, 32713, 32713, 32714, 32715, 32716, 32717, 32718,
 32718, 32719, 32720, 32721, 32721, 32722, 32723, 32723,
 32724, 32725, 32725, 32726, 32727, 32727, 32728, 32729,
 32729, 32730, 32730, 32731, 32731, 32732, 32733, 32733,
 32734, 32734, 32735, 32735, 32736, 32736, 32737, 32737,
 32738, 32738, 32739, 32739, 32739, 32740, 32740, 32741,
 32741, 32742, 32742, 32742, 32743, 32743, 32744, 32744,
 32744, 32745, 32745, 32745, 32746, 32746, 32746, 32747,
 32747, 32747, 32748, 32748, 32748, 32749, 32749, 32749,
 32749, 32750, 32750, 32750, 32750, 32751, 32751, 32751,
 32752, 32752, 32752, 32752, 32752, 32753, 32753, 32753,
 32753, 32754, 32754, 32754, 32754, 32754, 32755, 32755,
 32755, 32755, 32755, 32756, 32756, 32756, 32756, 32756,
 32757, 32757, 32757, 32757, 32757, 32757, 32757, 32758,
 32758, 32758, 32758, 32758, 32758, 32759, 32759, 32759,
 32759, 32759, 32759, 32759, 32759, 32760, 32760, 32760,
 32760, 32760, 32760, 32760, 32760, 32761, 32761, 32761,
 32761, 32761, 32761, 32761, 32761, 32761, 32761, 32762,
 32762, 32762, 32762, 32762, 32762, 32762, 32762, 32762,
 32762, 32762, 32762, 32763, 32763, 32763, 32763, 32763,
 32763, 32763, 32763, 32763, 32763, 32763, 32763, 32763,
 32763, 32764, 32764, 32764, 32764, 32764, 32764, 32764,
 32764, 32764, 32764, 32764, 32764, 32764, 32764, 32764,
 32764, 32764, 32764, 32764, 32765, 32765, 32765, 32765,
 32765, 32765, 32765, 32765, 32765, 32765, 32765, 32765,
 32765, 32765, 32765, 32765, 32765, 32765, 32765, 32765,
 32765, 32765, 32765, 32765, 32765, 32765, 32766, 32766,
 32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
 32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
 32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
 32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
 32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
 32766, 32766, 32767, 32767, 32767, 32767, 32767, 32767
};

// stdt[i]=count of -i or i in botton or top of stretcht[]
static const U8 stdt[712]={
    64,   128,   128,   128,   128,   128,   127,   128,
   127,   128,   127,   127,   127,   127,   126,   126,
   126,   126,   126,   125,   125,   124,   125,   124,
   123,   123,   123,   123,   122,   122,   121,   121,
   120,   120,   119,   119,   118,   118,   118,   116,
   117,   115,   116,   114,   114,   113,   113,   112,
   112,   111,   110,   110,   109,   108,   108,   107,
   106,   106,   105,   104,   104,   102,   103,   101,
   101,   100,    99,    98,    98,    97,    96,    96,
    94,    94,    94,    92,    92,    91,    90,    89,
    89,    88,    87,    86,    86,    84,    84,    84,
    82,    82,    81,    80,    79,    79,    78,    77,
    76,    76,    75,    74,    73,    73,    72,    71,
    70,    70,    69,    68,    67,    67,    66,    65,
    65,    64,    63,    62,    62,    61,    61,    59,
    59,    59,    57,    58,    56,    56,    55,    54,
    54,    53,    52,    52,    51,    51,    50,    49,
    49,    48,    48,    47,    47,    45,    46,    44,
    45,    43,    43,    43,    42,    41,    41,    40,
    40,    40,    39,    38,    38,    37,    37,    36,
    36,    36,    35,    34,    34,    34,    33,    32,
    33,    32,    31,    31,    30,    31,    29,    30,
    28,    29,    28,    28,    27,    27,    27,    26,
    26,    25,    26,    24,    25,    24,    24,    23,
    23,    23,    23,    22,    22,    21,    22,    21,
    20,    21,    20,    19,    20,    19,    19,    19,
    18,    18,    18,    18,    17,    17,    17,    17,
    16,    16,    16,    16,    15,    15,    15,    15,
    15,    14,    14,    14,    14,    13,    14,    13,
    13,    13,    12,    13,    12,    12,    12,    11,
    12,    11,    11,    11,    11,    11,    10,    11,
    10,    10,    10,    10,     9,    10,     9,     9,
     9,     9,     9,     8,     9,     8,     9,     8,
     8,     8,     7,     8,     8,     7,     7,     8,
     7,     7,     7,     6,     7,     7,     6,     6,
     7,     6,     6,     6,     6,     6,     6,     5,
     6,     5,     6,     5,     5,     5,     5,     5,
     5,     5,     5,     5,     4,     5,     4,     5,
     4,     4,     5,     4,     4,     4,     4,     4,
     4,     3,     4,     4,     3,     4,     4,     3,
     3,     4,     3,     3,     3,     4,     3,     3,
     3,     3,     3,     3,     2,     3,     3,     3,
     2,     3,     2,     3,     3,     2,     2,     3,
     2,     2,     3,     2,     2,     2,     2,     3,
     2,     2,     2,     2,     2,     2,     1,     2,
     2,     2,     2,     1,     2,     2,     2,     1,
     2,     1,     2,     2,     1,     2,     1,     2,
     1,     1,     2,     1,     1,     2,     1,     1,
     2,     1,     1,     1,     1,     2,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     0,     1,     1,     1,     1,     0,
     1,     1,     1,     0,     1,     1,     1,     0,
     1,     1,     0,     1,     1,     0,     1,     0,
     1,     1,     0,     1,     0,     1,     0,     1,
     0,     1,     0,     1,     0,     1,     0,     1,
     0,     1,     0,     1,     0,     1,     0,     0,
     1,     0,     1,     0,     0,     1,     0,     1,
     0,     0,     1,     0,     0,     1,     0,     0,
     1,     0,     0,     1,     0,     0,     0,     1,
     0,     0,     1,     0,     0,     0,     1,     0,
     0,     0,     1,     0,     0,     0,     1,     0,
     0,     0,     0,     1,     0,     0,     0,     0,
     1,     0,     0,     0,     0,     1,     0,     0,
     0,     0,     0,     1,     0,     0,     0,     0,
     0,     1,     0,     0,     0,     0,     0,     0,
     1,     0,     0,     0,     0,     0,     0,     0,
     1,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     1,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     1,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     1,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     1,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     1,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     1,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     1,     0
};

Predictor::Predictor(ZPAQL& zr):
    c8(1), hmap4(1), z(zr) {
  assert(sizeof(U8)==1);
  assert(sizeof(U16)==2);
  assert(sizeof(U32)==4);
  assert(sizeof(U64)==8);
  assert(sizeof(short)==2);
  assert(sizeof(int)==4);
  pcode=0;
  pcode_size=0;
  initTables=false;
}

Predictor::~Predictor() {
  allocx(pcode, pcode_size, 0);  // free executable memory
}

// Initialize the predictor with a new model in z
void Predictor::init() {

  // Clear old JIT code if any
  allocx(pcode, pcode_size, 0);

  // Initialize context hash function
  z.inith();

  // Initialize model independent tables
  if (!initTables && isModeled()) {
    initTables=true;
    memcpy(dt2k, sdt2k, sizeof(dt2k));
    memcpy(dt, sdt, sizeof(dt));

    // ssquasht[i]=int(32768.0/(1+exp((i-2048)*(-1.0/64))));
    // Copy middle 1344 of 4096 entries.
    memset(squasht, 0, 1376*2);
    memcpy(squasht+1376, ssquasht, 1344*2);
    for (int i=2720; i<4096; ++i) squasht[i]=32767;

    // sstretcht[i]=int(log((i+0.5)/(32767.5-i))*64+0.5+100000)-100000;
    int k=16384;
    for (int i=0; i<712; ++i)
      for (int j=stdt[i]; j>0; --j)
        stretcht[k++]=i;
    assert(k==32768);
    for (int i=0; i<16384; ++i)
      stretcht[i]=-stretcht[32767-i];

#ifndef NDEBUG
    // Verify floating point math for squash() and stretch()
    U32 sqsum=0, stsum=0;
    for (int i=32767; i>=0; --i)
      stsum=stsum*3+stretch(i);
    for (int i=4095; i>=0; --i)
      sqsum=sqsum*3+squash(i-2048);
    assert(stsum==3887533746u);
    assert(sqsum==2278286169u);
#endif
  }

  // Initialize predictions
  for (int i=0; i<256; ++i) h[i]=p[i]=0;

  // Initialize components
  for (int i=0; i<256; ++i)  // clear old model
    comp[i].init();
  int n=z.header[6]; // hsize[0..1] hh hm ph pm n (comp)[n] END 0[128] (hcomp) END
  const U8* cp=&z.header[7];  // start of component list
  for (int i=0; i<n; ++i) {
    assert(cp<&z.header[z.cend]);
    assert(cp>&z.header[0] && cp<&z.header[z.header.isize()-8]);
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  // c
        p[i]=(cp[1]-128)*4;
        break;
      case CM: // sizebits limit
        if (cp[1]>32) error("max size for CM is 32");
        cr.cm.resize(1, cp[1]);  // packed CM (22 bits) + CMCOUNT (10 bits)
        cr.limit=cp[2]*4;
        for (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=0x80000000;
        break;
      case ICM: // sizebits
        if (cp[1]>26) error("max size for ICM is 26");
        cr.limit=1023;
        cr.cm.resize(256);
        cr.ht.resize(64, cp[1]);
        for (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=st.cminit(j);
        break;
      case MATCH:  // sizebits
        if (cp[1]>32 || cp[2]>32) error("max size for MATCH is 32 32");
        cr.cm.resize(1, cp[1]);  // index
        cr.ht.resize(1, cp[2]);  // buf
        cr.ht(0)=1;
        break;
      case AVG: // j k wt
        if (cp[1]>=i) error("AVG j >= i");
        if (cp[2]>=i) error("AVG k >= i");
        break;
      case MIX2:  // sizebits j k rate mask
        if (cp[1]>32) error("max size for MIX2 is 32");
        if (cp[3]>=i) error("MIX2 k >= i");
        if (cp[2]>=i) error("MIX2 j >= i");
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.a16.resize(1, cp[1]);  // wt[size][m]
        for (size_t j=0; j<cr.a16.size(); ++j)
          cr.a16[j]=32768;
        break;
      case MIX: {  // sizebits j m rate mask
        if (cp[1]>32) error("max size for MIX is 32");
        if (cp[2]>=i) error("MIX j >= i");
        if (cp[3]<1 || cp[3]>i-cp[2]) error("MIX m not in 1..i-j");
        int m=cp[3];  // number of inputs
        assert(m>=1);
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.cm.resize(m, cp[1]);  // wt[size][m]
        for (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=65536/m;
        break;
      }
      case ISSE:  // sizebits j
        if (cp[1]>32) error("max size for ISSE is 32");
        if (cp[2]>=i) error("ISSE j >= i");
        cr.ht.resize(64, cp[1]);
        cr.cm.resize(512);
        for (int j=0; j<256; ++j) {
          cr.cm[j*2]=1<<15;
          cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);
        }
        break;
      case SSE: // sizebits j start limit
        if (cp[1]>32) error("max size for SSE is 32");
        if (cp[2]>=i) error("SSE j >= i");
        if (cp[3]>cp[4]*4) error("SSE start > limit*4");
        cr.cm.resize(32, cp[1]);
        cr.limit=cp[4]*4;
        for (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];
        break;
      default: error("unknown component type");
    }
    assert(compsize[*cp]>0);
    cp+=compsize[*cp];
    assert(cp>=&z.header[7] && cp<&z.header[z.cend]);
  }
}

// Return next bit prediction using interpreted COMP code
int Predictor::predict0() {
  assert(initTables);
  assert(c8>=1 && c8<=255);

  // Predict next bit
  int n=z.header[6];
  assert(n>0 && n<=255);
  const U8* cp=&z.header[7];
  assert(cp[-1]==n);
  for (int i=0; i<n; ++i) {
    assert(cp>&z.header[0] && cp<&z.header[z.header.isize()-8]);
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  // c
        break;
      case CM:  // sizebits limit
        cr.cxt=h[i]^hmap4;
        p[i]=stretch(cr.cm(cr.cxt)>>17);
        break;
      case ICM: // sizebits
        assert((hmap4&15)>0);
        if (c8==1 || (c8&0xf0)==16) cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];
        p[i]=stretch(cr.cm(cr.cxt)>>8);
        break;
      case MATCH: // sizebits bufbits: a=len, b=offset, c=bit, cxt=bitpos,
                  //                   ht=buf, limit=pos
        assert(cr.cm.size()==(size_t(1)<<cp[1]));
        assert(cr.ht.size()==(size_t(1)<<cp[2]));
        assert(cr.a<=255);
        assert(cr.c==0 || cr.c==1);
        assert(cr.cxt<8);
        assert(cr.limit<cr.ht.size());
        if (cr.a==0) p[i]=0;
        else {
          cr.c=(cr.ht(cr.limit-cr.b)>>(7-cr.cxt))&1; // predicted bit
          p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        }
        break;
      case AVG: // j k wt
        p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;
        break;
      case MIX2: { // sizebits j k rate mask
                   // c=size cm=wt[size] cxt=input
        cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));
        assert(cr.cxt<cr.a16.size());
        int w=cr.a16[cr.cxt];
        assert(w>=0 && w<65536);
        p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;
        assert(p[i]>=-2048 && p[i]<2048);
      }
        break;
      case MIX: {  // sizebits j m rate mask
                   // c=size cm=wt[size][m] cxt=index of wt in cm
        int m=cp[3];
        assert(m>=1 && m<=i);
        cr.cxt=h[i]+(c8&cp[5]);
        cr.cxt=(cr.cxt&(cr.c-1))*m; // pointer to row of weights
        assert(cr.cxt<=cr.cm.size()-m);
        int* wt=(int*)&cr.cm[cr.cxt];
        p[i]=0;
        for (int j=0; j<m; ++j)
          p[i]+=(wt[j]>>8)*p[cp[2]+j];
        p[i]=clamp2k(p[i]>>8);
      }
        break;
      case ISSE: { // sizebits j -- c=hi, cxt=bh
        assert((hmap4&15)>0);
        if (c8==1 || (c8&0xf0)==16)
          cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];  // bit history
        int *wt=(int*)&cr.cm[cr.cxt*2];
        p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);
      }
        break;
      case SSE: { // sizebits j start limit
        cr.cxt=(h[i]+c8)*32;
        int pq=p[cp[2]]+992;
        if (pq<0) pq=0;
        if (pq>1983) pq=1983;
        int wt=pq&63;
        pq>>=6;
        assert(pq>=0 && pq<=30);
        cr.cxt+=pq;
        p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)+(cr.cm(cr.cxt+1)>>10)*wt)>>13);
        cr.cxt+=wt>>5;
      }
        break;
      default:
        error("component predict not implemented");
    }
    cp+=compsize[cp[0]];
    assert(cp<&z.header[z.cend]);
    assert(p[i]>=-2048 && p[i]<2048);
  }
  assert(cp[0]==NONE);
  return squash(p[n-1]);
}

// Update model with decoded bit y (0...1)
void Predictor::update0(int y) {
  assert(initTables);
  assert(y==0 || y==1);
  assert(c8>=1 && c8<=255);
  assert(hmap4>=1 && hmap4<=511);

  // Update components
  const U8* cp=&z.header[7];
  int n=z.header[6];
  assert(n>=1 && n<=255);
  assert(cp[-1]==n);
  for (int i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  // c
        break;
      case CM:  // sizebits limit
        train(cr, y);
        break;
      case ICM: { // sizebits: cxt=ht[b]=bh, ht[c][0..15]=bh row, cxt=bh
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.ht[cr.c+(hmap4&15)], y);
        U32& pn=cr.cm(cr.cxt);
        pn+=int(y*32767-(pn>>8))>>2;
      }
        break;
      case MATCH: // sizebits bufbits:
                  //   a=len, b=offset, c=bit, cm=index, cxt=bitpos
                  //   ht=buf, limit=pos
      {
        assert(cr.a<=255);
        assert(cr.c==0 || cr.c==1);
        assert(cr.cxt<8);
        assert(cr.cm.size()==(size_t(1)<<cp[1]));
        assert(cr.ht.size()==(size_t(1)<<cp[2]));
        assert(cr.limit<cr.ht.size());
        if (int(cr.c)!=y) cr.a=0;  // mismatch?
        cr.ht(cr.limit)+=cr.ht(cr.limit)+y;
        if (++cr.cxt==8) {
          cr.cxt=0;
          ++cr.limit;
          cr.limit&=(1<<cp[2])-1;
          if (cr.a==0) {  // look for a match
            cr.b=cr.limit-cr.cm(h[i]);
            if (cr.b&(cr.ht.size()-1))
              while (cr.a<255
                     && cr.ht(cr.limit-cr.a-1)==cr.ht(cr.limit-cr.a-cr.b-1))
                ++cr.a;
          }
          else cr.a+=cr.a<255;
          cr.cm(h[i])=cr.limit;
        }
      }
        break;
      case AVG:  // j k wt
        break;
      case MIX2: { // sizebits j k rate mask
                   // cm=wt[size], cxt=input
        assert(cr.a16.size()==cr.c);
        assert(cr.cxt<cr.a16.size());
        int err=(y*32767-squash(p[i]))*cp[4]>>5;
        int w=cr.a16[cr.cxt];
        w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;
        if (w<0) w=0;
        if (w>65535) w=65535;
        cr.a16[cr.cxt]=w;
      }
        break;
      case MIX: {   // sizebits j m rate mask
                    // cm=wt[size][m], cxt=input
        int m=cp[3];
        assert(m>0 && m<=i);
        assert(cr.cm.size()==m*cr.c);
        assert(cr.cxt+m<=cr.cm.size());
        int err=(y*32767-squash(p[i]))*cp[4]>>4;
        int* wt=(int*)&cr.cm[cr.cxt];
        for (int j=0; j<m; ++j)
          wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));
      }
        break;
      case ISSE: { // sizebits j  -- c=hi, cxt=bh
        assert(cr.cxt==cr.ht[cr.c+(hmap4&15)]);
        int err=y*32767-squash(p[i]);
        int *wt=(int*)&cr.cm[cr.cxt*2];
        wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));
        wt[1]=clamp512k(wt[1]+((err+16)>>5));
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.cxt, y);
      }
        break;
      case SSE:  // sizebits j start limit
        train(cr, y);
        break;
      default:
        assert(0);
    }
    cp+=compsize[cp[0]];
    assert(cp>=&z.header[7] && cp<&z.header[z.cend] 
           && cp<&z.header[z.header.isize()-8]);
  }
  assert(cp[0]==NONE);

  // Save bit y in c8, hmap4
  c8+=c8+y;
  if (c8>=256) {
    z.run(c8-256);
    hmap4=1;
    c8=1;
    for (int i=0; i<n; ++i) h[i]=z.H(i);
  }
  else if (c8>=16 && c8<32)
    hmap4=(hmap4&0xf)<<5|y<<4|1;
  else
    hmap4=(hmap4&0x1f0)|(((hmap4&0xf)*2+y)&0xf);
}

// Find cxt row in hash table ht. ht has rows of 16 indexed by the
// low sizebits of cxt with element 0 having the next higher 8 bits for
// collision detection. If not found after 3 adjacent tries, replace the
// row with lowest element 1 as priority. Return index of row.
size_t Predictor::find(Array<U8>& ht, int sizebits, U32 cxt) {
  assert(initTables);
  assert(ht.size()==size_t(16)<<sizebits);
  int chk=cxt>>sizebits&255;
  size_t h0=(cxt*16)&(ht.size()-16);
  if (ht[h0]==chk) return h0;
  size_t h1=h0^16;
  if (ht[h1]==chk) return h1;
  size_t h2=h0^32;
  if (ht[h2]==chk) return h2;
  if (ht[h0+1]<=ht[h1+1] && ht[h0+1]<=ht[h2+1])
    return memset(&ht[h0], 0, 16), ht[h0]=chk, h0;
  else if (ht[h1+1]<ht[h2+1])
    return memset(&ht[h1], 0, 16), ht[h1]=chk, h1;
  else
    return memset(&ht[h2], 0, 16), ht[h2]=chk, h2;
}

/////////////////////// Decoder ///////////////////////

Decoder::Decoder(ZPAQL& z):
    in(0), low(1), high(0xFFFFFFFF), curr(0), rpos(0), wpos(0),
    pr(z), buf(BUFSIZE) {
}

void Decoder::init() {
  pr.init();
  if (pr.isModeled()) low=1, high=0xFFFFFFFF, curr=0;
  else low=high=curr=0;
}

// Return next bit of decoded input, which has 16 bit probability p of being 1
int Decoder::decode(int p) {
  assert(pr.isModeled());
  assert(p>=0 && p<65536);
  assert(high>low && low>0);
  if (curr<low || curr>high) error("archive corrupted");
  assert(curr>=low && curr<=high);
  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  // split range
  assert(high>mid && mid>=low);
  int y;
  if (curr<=mid) y=1, high=mid;  // pick half
  else y=0, low=mid+1;
  while ((high^low)<0x1000000) { // shift out identical leading bytes
    high=high<<8|255;
    low=low<<8;
    low+=(low==0);
    int c=get();
    if (c<0) error("unexpected end of file");
    curr=curr<<8|c;
  }
  return y;
}

// Decompress 1 byte or -1 at end of input
int Decoder::decompress() {
  if (pr.isModeled()) {  // n>0 components?
    if (curr==0) {  // segment initialization
      for (int i=0; i<4; ++i)
        curr=curr<<8|get();
    }
    if (decode(0)) {
      if (curr!=0) error("decoding end of stream");
      return -1;
    }
    else {
      int c=1;
      while (c<256) {  // get 8 bits
        int p=pr.predict()*2+1;
        c+=c+decode(p);
        pr.update(c&1);
      }
      return c-256;
    }
  }
  else {
    if (curr==0) {
      for (int i=0; i<4; ++i) curr=curr<<8|get();
      if (curr==0) return -1;
    }
    --curr;
    return get();
  }
}

// Find end of compressed data and return next byte
int Decoder::skip() {
  int c=-1;
  if (pr.isModeled()) {
    while (curr==0)  // at start?
      curr=get();
    while (curr && (c=get())>=0)  // find 4 zeros
      curr=curr<<8|c;
    while ((c=get())==0) ;  // might be more than 4
    return c;
  }
  else {
    if (curr==0)  // at start?
      for (int i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
    while (curr>0) {
      while (curr>0) {
        --curr;
        if (get()<0) return error("skipped to EOF"), -1;
      }
      for (int i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
    }
    if (c>=0) c=get();
    return c;
  }
}

////////////////////// PostProcessor //////////////////////

// Copy ph, pm from block header
void PostProcessor::init(int h, int m) {
  state=hsize=0;
  ph=h;
  pm=m;
  z.clear();
}

// (PASS=0 | PROG=1 psize[0..1] pcomp[0..psize-1]) data... EOB=-1
// Return state: 1=PASS, 2..4=loading PROG, 5=PROG loaded
int PostProcessor::write(int c) {
  assert(c>=-1 && c<=255);
  switch (state) {
    case 0:  // initial state
      if (c<0) error("Unexpected EOS");
      state=c+1;  // 1=PASS, 2=PROG
      if (state>2) error("unknown post processing type");
      if (state==1) z.clear();
      break;
    case 1:  // PASS
      z.outc(c);
      break;
    case 2: // PROG
      if (c<0) error("Unexpected EOS");
      hsize=c;  // low byte of size
      state=3;
      break;
    case 3:  // PROG psize[0]
      if (c<0) error("Unexpected EOS");
      hsize+=c*256;  // high byte of psize
      if (hsize<1) error("Empty PCOMP");
      z.header.resize(hsize+300);
      z.cend=8;
      z.hbegin=z.hend=z.cend+128;
      z.header[4]=ph;
      z.header[5]=pm;
      state=4;
      break;
    case 4:  // PROG psize[0..1] pcomp[0...]
      if (c<0) error("Unexpected EOS");
      assert(z.hend<z.header.isize());
      z.header[z.hend++]=c;  // one byte of pcomp
      if (z.hend-z.hbegin==hsize) {  // last byte of pcomp?
        hsize=z.cend-2+z.hend-z.hbegin;
        z.header[0]=hsize&255;  // header size with empty COMP
        z.header[1]=hsize>>8;
        z.initp();
        state=5;
      }
      break;
    case 5:  // PROG ... data
      z.run(c);
      if (c<0) z.flush();
      break;
  }
  return state;
}

/////////////////////// Decompresser /////////////////////

// Find the start of a block and return true if found. Set memptr
// to memory used.
bool Decompresser::findBlock(double* memptr) {
  assert(state==BLOCK);

  // Find start of block
  U32 h1=0x3D49B113, h2=0x29EB7F93, h3=0x2614BE13, h4=0x3828EB13;
  // Rolling hashes initialized to hash of first 13 bytes
  int c;
  while ((c=dec.get())!=-1) {
    h1=h1*12+c;
    h2=h2*20+c;
    h3=h3*28+c;
    h4=h4*44+c;
    if (h1==0xB16B88F1 && h2==0xFF5376F1 && h3==0x72AC5BF1 && h4==0x2F909AF1)
      break;  // hash of 16 byte string
  }
  if (c==-1) return false;

  // Read header
  if ((c=dec.get())!=1 && c!=2) error("unsupported ZPAQ level");
  if (dec.get()!=1) error("unsupported ZPAQL type");
  z.read(&dec);
  if (c==1 && z.header.isize()>6 && z.header[6]==0)
    error("ZPAQ level 1 requires at least 1 component");
  if (memptr) *memptr=z.memory();
  state=FILENAME;
  decode_state=FIRSTSEG;
  return true;
}

// Read the start of a segment (1) or end of block code (255).
// If a segment is found, write the filename and return true, else false.
bool Decompresser::findFilename(Writer* filename) {
  assert(state==FILENAME);
  int c=dec.get();
  if (c==1) {  // segment found
    while (true) {
      c=dec.get();
      if (c==-1) error("unexpected EOF");
      if (c==0) {
        state=COMMENT;
        return true;
      }
      if (filename) filename->put(c);
    }
  }
  else if (c==255) {  // end of block found
    state=BLOCK;
    return false;
  }
  else
    error("missing segment or end of block");
  return false;
}

// Read the comment from the segment header
void Decompresser::readComment(Writer* comment) {
  assert(state==COMMENT);
  state=DATA;
  while (true) {
    int c=dec.get();
    if (c==-1) error("unexpected EOF");
    if (c==0) break;
    if (comment) comment->put(c);
  }
  if (dec.get()!=0) error("missing reserved byte");
}

// Decompress n bytes, or all if n < 0. Return false if done
bool Decompresser::decompress(int n) {
  assert(state==DATA);
  if (decode_state==SKIP) error("decompression after skipped segment");
  assert(decode_state!=SKIP);

  // Initialize models to start decompressing block
  if (decode_state==FIRSTSEG) {
    dec.init();
    assert(z.header.size()>5);
    pp.init(z.header[4], z.header[5]);
    decode_state=SEG;
  }

  // Decompress and load PCOMP into postprocessor
  while ((pp.getState()&3)!=1)
    pp.write(dec.decompress());

  // Decompress n bytes, or all if n < 0
  while (n) {
    int c=dec.decompress();
    pp.write(c);
    if (c==-1) {
      state=SEGEND;
      return false;
    }
    if (n>0) --n;
  }
  return true;
}

// Read end of block. If a SHA1 checksum is present, write 1 and the
// 20 byte checksum into sha1string, else write 0 in first byte.
// If sha1string is 0 then discard it.
void Decompresser::readSegmentEnd(char* sha1string) {
  assert(state==DATA || state==SEGEND);

  // Skip remaining data if any and get next byte
  int c=0;
  if (state==DATA) {
    c=dec.skip();
    decode_state=SKIP;
  }
  else if (state==SEGEND)
    c=dec.get();
  state=FILENAME;

  // Read checksum
  if (c==254) {
    if (sha1string) sha1string[0]=0;  // no checksum
  }
  else if (c==253) {
    if (sha1string) sha1string[0]=1;
    for (int i=1; i<=20; ++i) {
      c=dec.get();
      if (sha1string) sha1string[i]=c;
    }
  }
  else
    error("missing end of segment marker");
}

/////////////////////////// decompress() //////////////////////

void decompress(Reader* in, Writer* out) {
  Decompresser d;
  d.setInput(in);
  d.setOutput(out);
  while (d.findBlock()) {       // don't calculate memory
    while (d.findFilename()) {  // discard filename
      d.readComment();          // discard comment
      d.decompress();           // to end of segment
      d.readSegmentEnd();       // discard sha1string
    }
  }
}

/////////////////////////// Encoder ///////////////////////////

// Initialize for start of block
void Encoder::init() {
  low=1;
  high=0xFFFFFFFF;
  pr.init();
  if (!pr.isModeled()) low=0, buf.resize(1<<16);
}

// compress bit y having probability p/64K
void Encoder::encode(int y, int p) {
  assert(out);
  assert(p>=0 && p<65536);
  assert(y==0 || y==1);
  assert(high>low && low>0);
  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  // split range
  assert(high>mid && mid>=low);
  if (y) high=mid; else low=mid+1; // pick half
  while ((high^low)<0x1000000) { // write identical leading bytes
    out->put(high>>24);  // same as low>>24
    high=high<<8|255;
    low=low<<8;
    low+=(low==0); // so we don't code 4 0 bytes in a row
  }
}

// compress byte c (0..255 or -1=EOS)
void Encoder::compress(int c) {
  assert(out);
  if (pr.isModeled()) {
    if (c==-1)
      encode(1, 0);
    else {
      assert(c>=0 && c<=255);
      encode(0, 0);
      for (int i=7; i>=0; --i) {
        int p=pr.predict()*2+1;
        assert(p>0 && p<65536);
        int y=c>>i&1;
        encode(y, p);
        pr.update(y);
      }
    }
  }
  else {
    if (low && (c<0 || low==buf.size())) {
      out->put((low>>24)&255);
      out->put((low>>16)&255);
      out->put((low>>8)&255);
      out->put(low&255);
      out->write(&buf[0], low);
      low=0;
    }
    if (c>=0) buf[low++]=c;
  }
}

//////////////////////////// Compiler /////////////////////////

// Component names
const char* compname[256]=
  {"","const","cm","icm","match","avg","mix2","mix","isse","sse",0};

// Opcodes
const char* opcodelist[272]={
"error","a++",  "a--",  "a!",   "a=0",  "",     "",     "a=r",
"b<>a", "b++",  "b--",  "b!",   "b=0",  "",     "",     "b=r",
"c<>a", "c++",  "c--",  "c!",   "c=0",  "",     "",     "c=r",
"d<>a", "d++",  "d--",  "d!",   "d=0",  "",     "",     "d=r",
"*b<>a","*b++", "*b--", "*b!",  "*b=0", "",     "",     "jt",
"*c<>a","*c++", "*c--", "*c!",  "*c=0", "",     "",     "jf",
"*d<>a","*d++", "*d--", "*d!",  "*d=0", "",     "",     "r=a",
"halt", "out",  "",     "hash", "hashd","",     "",     "jmp",
"a=a",  "a=b",  "a=c",  "a=d",  "a=*b", "a=*c", "a=*d", "a=",
"b=a",  "b=b",  "b=c",  "b=d",  "b=*b", "b=*c", "b=*d", "b=",
"c=a",  "c=b",  "c=c",  "c=d",  "c=*b", "c=*c", "c=*d", "c=",
"d=a",  "d=b",  "d=c",  "d=d",  "d=*b", "d=*c", "d=*d", "d=",
"*b=a", "*b=b", "*b=c", "*b=d", "*b=*b","*b=*c","*b=*d","*b=",
"*c=a", "*c=b", "*c=c", "*c=d", "*c=*b","*c=*c","*c=*d","*c=",
"*d=a", "*d=b", "*d=c", "*d=d", "*d=*b","*d=*c","*d=*d","*d=",
"",     "",     "",     "",     "",     "",     "",     "",
"a+=a", "a+=b", "a+=c", "a+=d", "a+=*b","a+=*c","a+=*d","a+=",
"a-=a", "a-=b", "a-=c", "a-=d", "a-=*b","a-=*c","a-=*d","a-=",
"a*=a", "a*=b", "a*=c", "a*=d", "a*=*b","a*=*c","a*=*d","a*=",
"a/=a", "a/=b", "a/=c", "a/=d", "a/=*b","a/=*c","a/=*d","a/=",
"a%=a", "a%=b", "a%=c", "a%=d", "a%=*b","a%=*c","a%=*d","a%=",
"a&=a", "a&=b", "a&=c", "a&=d", "a&=*b","a&=*c","a&=*d","a&=",
"a&~a", "a&~b", "a&~c", "a&~d", "a&~*b","a&~*c","a&~*d","a&~",
"a|=a", "a|=b", "a|=c", "a|=d", "a|=*b","a|=*c","a|=*d","a|=",
"a^=a", "a^=b", "a^=c", "a^=d", "a^=*b","a^=*c","a^=*d","a^=",
"a<<=a","a<<=b","a<<=c","a<<=d","a<<=*b","a<<=*c","a<<=*d","a<<=",
"a>>=a","a>>=b","a>>=c","a>>=d","a>>=*b","a>>=*c","a>>=*d","a>>=",
"a==a", "a==b", "a==c", "a==d", "a==*b","a==*c","a==*d","a==",
"a<a",  "a<b",  "a<c",  "a<d",  "a<*b", "a<*c", "a<*d", "a<",
"a>a",  "a>b",  "a>c",  "a>d",  "a>*b", "a>*c", "a>*d", "a>",
"",     "",     "",     "",     "",     "",     "",     "",
"",     "",     "",     "",     "",     "",     "",     "lj",
"post", "pcomp","end",  "if",   "ifnot","else", "endif","do",
"while","until","forever","ifl","ifnotl","elsel",";",    0};

// Advance in to start of next token. Tokens are delimited by white
// space. Comments inclosed in ((nested) parenthsis) are skipped.
void Compiler::next() {
  assert(in);
  for (; *in; ++in) {
    if (*in=='\n') ++line;
    if (*in=='(') state+=1+(state<0);
    else if (state>0 && *in==')') --state;
    else if (state<0 && *in<=' ') state=0;
    else if (state==0 && *in>' ') {state=-1; break;}
  }
  if (!*in) error("unexpected end of config");
}

// convert to lower case
int tolower(int c) {return (c>='A' && c<='Z') ? c+'a'-'A' : c;}

// return true if in==word up to white space or '(', case insensitive
bool Compiler::matchToken(const char* word) {
  const char* a=in;
  for (; (*a>' ' && *a!='(' && *word); ++a, ++word)
    if (tolower(*a)!=tolower(*word)) return false;
  return !*word && (*a<=' ' || *a=='(');
}

// Print error message and exit
void Compiler::syntaxError(const char* msg, const char* expected) {
  Array<char> sbuf(128);  // error message to report
  char* s=&sbuf[0];
  strcat(s, "Config line ");
  for (int i=strlen(s), r=1000000; r; r/=10)  // append line number
    if (line/r) s[i++]='0'+line/r%10;
  strcat(s, " at ");
  for (int i=strlen(s); i<40 && *in>' '; ++i)  // append token found
    s[i]=*in++;
  strcat(s, ": ");
  strncat(s, msg, 40);  // append message
  if (expected) {
    strcat(s, ", expected: ");
    strncat(s, expected, 20);  // append expected token if any
  }
  error(s);
}

// Read a token, which must be in the NULL terminated list or else
// exit with an error. If found, return its index.
int Compiler::rtoken(const char* list[]) {
  assert(in);
  assert(list);
  next();
  for (int i=0; list[i]; ++i)
    if (matchToken(list[i]))
      return i;
  syntaxError("unexpected");
  assert(0);
  return -1; // not reached
}

// Read a token which must be the specified value s
void Compiler::rtoken(const char* s) {
  assert(s);
  next();
  if (!matchToken(s)) syntaxError("expected", s);
}

// Read a number in (low...high) or exit with an error
// For numbers like $N+M, return arg[N-1]+M
int Compiler::rtoken(int low, int high) {
  next();
  int r=0;
  if (in[0]=='$' && in[1]>='1' && in[1]<='9') {
    if (in[2]=='+') r=atoi(in+3);
    if (args) r+=args[in[1]-'1'];
  }
  else if (in[0]=='-' || (in[0]>='0' && in[0]<='9')) r=atoi(in);
  else syntaxError("expected a number");
  if (r<low) syntaxError("number too low");
  if (r>high) syntaxError("number too high");
  return r;
}

// Compile HCOMP or PCOMP code. Exit on error. Return
// code for end token (POST, PCOMP, END)
int Compiler::compile_comp(ZPAQL& z) {
  int op=0;
  const int comp_begin=z.hend;
  while (true) {
    op=rtoken(opcodelist);
    if (op==POST || op==PCOMP || op==END) break;
    int operand=-1; // 0...255 if 2 bytes
    int operand2=-1;  // 0...255 if 3 bytes
    if (op==IF) {
      op=JF;
      operand=0; // set later
      if_stack.push(z.hend+1); // save jump target location
    }
    else if (op==IFNOT) {
      op=JT;
      operand=0;
      if_stack.push(z.hend+1); // save jump target location
    }
    else if (op==IFL || op==IFNOTL) {  // long if
      if (op==IFL) z.header[z.hend++]=(JT);
      if (op==IFNOTL) z.header[z.hend++]=(JF);
      z.header[z.hend++]=(3);
      op=LJ;
      operand=operand2=0;
      if_stack.push(z.hend+1);
    }
    else if (op==ELSE || op==ELSEL) {
      if (op==ELSE) op=JMP, operand=0;
      if (op==ELSEL) op=LJ, operand=operand2=0;
      int a=if_stack.pop();  // conditional jump target location
      assert(a>comp_begin && a<int(z.hend));
      if (z.header[a-1]!=LJ) {  // IF, IFNOT
        assert(z.header[a-1]==JT || z.header[a-1]==JF || z.header[a-1]==JMP);
        int j=z.hend-a+1+(op==LJ); // offset at IF
        assert(j>=0);
        if (j>127) syntaxError("IF too big, try IFL, IFNOTL");
        z.header[a]=j;
      }
      else {  // IFL, IFNOTL
        int j=z.hend-comp_begin+2+(op==LJ);
        assert(j>=0);
        z.header[a]=j&255;
        z.header[a+1]=(j>>8)&255;
      }
      if_stack.push(z.hend+1);  // save JMP target location
    }
    else if (op==ENDIF) {
      int a=if_stack.pop();  // jump target address
      assert(a>comp_begin && a<int(z.hend));
      int j=z.hend-a-1;  // jump offset
      assert(j>=0);
      if (z.header[a-1]!=LJ) {
        assert(z.header[a-1]==JT || z.header[a-1]==JF || z.header[a-1]==JMP);
        if (j>127) syntaxError("IF too big, try IFL, IFNOTL, ELSEL\n");
        z.header[a]=j;
      }
      else {
        assert(a+1<int(z.hend));
        j=z.hend-comp_begin;
        z.header[a]=j&255;
        z.header[a+1]=(j>>8)&255;
      }
    }
    else if (op==DO) {
      do_stack.push(z.hend);
    }
    else if (op==WHILE || op==UNTIL || op==FOREVER) {
      int a=do_stack.pop();
      assert(a>=comp_begin && a<int(z.hend));
      int j=a-z.hend-2;
      assert(j<=-2);
      if (j>=-127) {  // backward short jump
        if (op==WHILE) op=JT;
        if (op==UNTIL) op=JF;
        if (op==FOREVER) op=JMP;
        operand=j&255;
      }
      else {  // backward long jump
        j=a-comp_begin;
        assert(j>=0 && j<int(z.hend)-comp_begin);
        if (op==WHILE) {
          z.header[z.hend++]=(JF);
          z.header[z.hend++]=(3);
        }
        if (op==UNTIL) {
          z.header[z.hend++]=(JT);
          z.header[z.hend++]=(3);
        }
        op=LJ;
        operand=j&255;
        operand2=j>>8;
      }
    }
    else if ((op&7)==7) { // 2 byte operand, read N
      if (op==LJ) {
        operand=rtoken(0, 65535);
        operand2=operand>>8;
        operand&=255;
      }
      else if (op==JT || op==JF || op==JMP) {
        operand=rtoken(-128, 127);
        operand&=255;
      }
      else
        operand=rtoken(0, 255);
    }
    if (op>=0 && op<=255)
      z.header[z.hend++]=(op);
    if (operand>=0)
      z.header[z.hend++]=(operand);
    if (operand2>=0)
      z.header[z.hend++]=(operand2);
    if (z.hend>=z.header.isize()-130 || z.hend-z.hbegin+z.cend-2>65535)
      syntaxError("program too big");
  }
  z.header[z.hend++]=(0); // END
  return op;
}

// Compile a configuration file. Store COMP/HCOMP section in hcomp.
// If there is a PCOMP section, store it in pcomp and store the PCOMP
// command in pcomp_cmd. Replace "$1..$9+n" with args[0..8]+n

Compiler::Compiler(const char* in_, int* args_, ZPAQL& hz_, ZPAQL& pz_,
                   Writer* out2_): in(in_), args(args_), hz(hz_), pz(pz_),
                   out2(out2_), if_stack(1000), do_stack(1000) {
  line=1;
  state=0;
  hz.clear();
  pz.clear();
  hz.header.resize(68000); 

  // Compile the COMP section of header
  rtoken("comp");
  hz.header[2]=rtoken(0, 255);  // hh
  hz.header[3]=rtoken(0, 255);  // hm
  hz.header[4]=rtoken(0, 255);  // ph
  hz.header[5]=rtoken(0, 255);  // pm
  const int n=hz.header[6]=rtoken(0, 255);  // n
  hz.cend=7;
  for (int i=0; i<n; ++i) {
    rtoken(i, i);
    CompType type=CompType(rtoken(compname));
    hz.header[hz.cend++]=type;
    int clen=libzpaq::compsize[type&255];
    if (clen<1 || clen>10) syntaxError("invalid component");
    for (int j=1; j<clen; ++j)
      hz.header[hz.cend++]=rtoken(0, 255);  // component arguments
  }
  hz.header[hz.cend++];  // end
  hz.hbegin=hz.hend=hz.cend+128;

  // Compile HCOMP
  rtoken("hcomp");
  int op=compile_comp(hz);

  // Compute header size
  int hsize=hz.cend-2+hz.hend-hz.hbegin;
  hz.header[0]=hsize&255;
  hz.header[1]=hsize>>8;

  // Compile POST 0 END
  if (op==POST) {
    rtoken(0, 0);
    rtoken("end");
  }

  // Compile PCOMP pcomp_cmd ; program... END
  else if (op==PCOMP) {
    pz.header.resize(68000);
    pz.header[4]=hz.header[4];  // ph
    pz.header[5]=hz.header[5];  // pm
    pz.cend=8;
    pz.hbegin=pz.hend=pz.cend+128;

    // get pcomp_cmd ending with ";" (case sensitive)
    next();
    while (*in && *in!=';') {
      if (out2)
        out2->put(*in);
      ++in;
    }
    if (*in) ++in;

    // Compile PCOMP
    op=compile_comp(pz);
    int len=pz.cend-2+pz.hend-pz.hbegin;  // insert header size
    assert(len>=0);
    pz.header[0]=len&255;
    pz.header[1]=len>>8;
    if (op!=END)
      syntaxError("expected END");
  }
  else if (op!=END)
    syntaxError("expected END or POST 0 END or PCOMP cmd ; ... END");
}

///////////////////// Compressor //////////////////////

// Write 13 byte start tag
// "\x37\x6B\x53\x74\xA0\x31\x83\xD3\x8C\xB2\x28\xB0\xD3"
void Compressor::writeTag() {
  assert(state==INIT);
  enc.out->put(0x37);
  enc.out->put(0x6b);
  enc.out->put(0x53);
  enc.out->put(0x74);
  enc.out->put(0xa0);
  enc.out->put(0x31);
  enc.out->put(0x83);
  enc.out->put(0xd3);
  enc.out->put(0x8c);
  enc.out->put(0xb2);
  enc.out->put(0x28);
  enc.out->put(0xb0);
  enc.out->put(0xd3);
}

void Compressor::startBlock(int level) {

  // Model 1 - min.cfg
  static const char models[]={
  26,0,1,2,0,0,2,3,16,8,19,0,0,96,4,28,
  59,10,59,112,25,10,59,10,59,112,56,0,

  // Model 2 - mid.cfg
  69,0,3,3,0,0,8,3,5,8,13,0,8,17,1,8,
  18,2,8,18,3,8,19,4,4,22,24,7,16,0,7,24,
  (char)-1,0,17,104,74,4,95,1,59,112,10,25,59,112,10,25,
  59,112,10,25,59,112,10,25,59,112,10,25,59,10,59,112,
  25,69,(char)-49,8,112,56,0,

  // Model 3 - max.cfg
  (char)-60,0,5,9,0,0,22,1,(char)-96,3,5,8,13,1,8,16,
  2,8,18,3,8,19,4,8,19,5,8,20,6,4,22,24,
  3,17,8,19,9,3,13,3,13,3,13,3,14,7,16,0,
  15,24,(char)-1,7,8,0,16,10,(char)-1,6,0,15,16,24,0,9,
  8,17,32,(char)-1,6,8,17,18,16,(char)-1,9,16,19,32,(char)-1,6,
  0,19,20,16,0,0,17,104,74,4,95,2,59,112,10,25,
  59,112,10,25,59,112,10,25,59,112,10,25,59,112,10,25,
  59,10,59,112,10,25,59,112,10,25,69,(char)-73,32,(char)-17,64,47,
  14,(char)-25,91,47,10,25,60,26,48,(char)-122,(char)-105,20,112,63,9,70,
  (char)-33,0,39,3,25,112,26,52,25,25,74,10,4,59,112,25,
  10,4,59,112,25,10,4,59,112,25,65,(char)-113,(char)-44,72,4,59,
  112,8,(char)-113,(char)-40,8,68,(char)-81,60,60,25,69,(char)-49,9,112,25,25,
  25,25,25,112,56,0,

  0,0}; // 0,0 = end of list

  if (level<1) error("compression level must be at least 1");
  const char* p=models;
  int i;
  for (i=1; i<level && toU16(p); ++i)
    p+=toU16(p)+2;
  if (toU16(p)<1) error("compression level too high");
  startBlock(p);
}

// Memory reader
class MemoryReader: public Reader {
  const char* p;
public:
  MemoryReader(const char* p_): p(p_) {}
  int get() {return *p++&255;}
};

void Compressor::startBlock(const char* hcomp) {
  assert(state==INIT);
  MemoryReader m(hcomp);
  z.read(&m);
  pz.sha1=&sha1;
  assert(z.header.isize()>6);
  enc.out->put('z');
  enc.out->put('P');
  enc.out->put('Q');
  enc.out->put(1+(z.header[6]==0));  // level 1 or 2
  enc.out->put(1);
  z.write(enc.out, false);
  state=BLOCK1;
}

void Compressor::startBlock(const char* config, int* args, Writer* pcomp_cmd) {
  assert(state==INIT);
  Compiler(config, args, z, pz, pcomp_cmd);
  pz.sha1=&sha1;
  assert(z.header.isize()>6);
  enc.out->put('z');
  enc.out->put('P');
  enc.out->put('Q');
  enc.out->put(1+(z.header[6]==0));  // level 1 or 2
  enc.out->put(1);
  z.write(enc.out, false);
  state=BLOCK1;
}

// Write a segment header
void Compressor::startSegment(const char* filename, const char* comment) {
  assert(state==BLOCK1 || state==BLOCK2);
  enc.out->put(1);
  while (filename && *filename)
    enc.out->put(*filename++);
  enc.out->put(0);
  while (comment && *comment)
    enc.out->put(*comment++);
  enc.out->put(0);
  enc.out->put(0);
  if (state==BLOCK1) state=SEG1;
  if (state==BLOCK2) state=SEG2;
}

// Initialize encoding and write pcomp to first segment
// If len is 0 then length is encoded in pcomp[0..1]
// if pcomp is 0 then get pcomp from pz.header
void Compressor::postProcess(const char* pcomp, int len) {
  if (state==SEG2) return;
  assert(state==SEG1);
  enc.init();
  if (!pcomp) {
    len=pz.hend-pz.hbegin;
    if (len>0) {
      assert(pz.header.isize()>pz.hend);
      assert(pz.hbegin>=0);
      pcomp=(const char*)&pz.header[pz.hbegin];
    }
    assert(len>=0);
  }
  else if (len==0) {
    len=toU16(pcomp);
    pcomp+=2;
  }
  if (len>0) {
    enc.compress(1);
    enc.compress(len&255);
    enc.compress((len>>8)&255);
    for (int i=0; i<len; ++i)
      enc.compress(pcomp[i]&255);
    if (verify)
      pz.initp();
  }
  else
    enc.compress(0);
  state=SEG2;
}

// Compress n bytes, or to EOF if n < 0
bool Compressor::compress(int n) {
  if (state==SEG1)
    postProcess();
  assert(state==SEG2);

  const int BUFSIZE=1<<14;
  char buf[BUFSIZE];  // input buffer
  while (n) {
    int nbuf=BUFSIZE;  // bytes read into buf
    if (n>=0 && n<nbuf) nbuf=n;
    int nr=in->read(buf, nbuf);
    if (nr<0 || nr>BUFSIZE || nr>nbuf) error("invalid read size");
    if (nr<=0) return false;
    if (n>=0) n-=nr;
    for (int i=0; i<nr; ++i) {
      int ch=U8(buf[i]);
      enc.compress(ch);
      if (verify) {
        if (pz.hend) pz.run(ch);
        else sha1.put(ch);
      }
    }
  }
  return true;
}

// End segment, write sha1string if present
void Compressor::endSegment(const char* sha1string) {
  if (state==SEG1)
    postProcess();
  assert(state==SEG2);
  enc.compress(-1);
  if (verify && pz.hend) {
    pz.run(-1);
    pz.flush();
  }
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  if (sha1string) {
    enc.out->put(253);
    for (int i=0; i<20; ++i)
      enc.out->put(sha1string[i]);
  }
  else
    enc.out->put(254);
  state=BLOCK2;
}

// End segment, write checksum and size is verify is true
char* Compressor::endSegmentChecksum(int64_t* size, bool dosha1) {
  if (state==SEG1)
    postProcess();
  assert(state==SEG2);
  enc.compress(-1);
  if (verify && pz.hend) {
    pz.run(-1);
    pz.flush();
  }
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  if (verify) {
    if (size) *size=sha1.usize();
    memcpy(sha1result, sha1.result(), 20);
  }
  if (verify && dosha1) {
    enc.out->put(253);
    for (int i=0; i<20; ++i)
      enc.out->put(sha1result[i]);
  }
  else
    enc.out->put(254);
  state=BLOCK2;
  return verify ? sha1result : 0;
}

// End block
void Compressor::endBlock() {
  assert(state==BLOCK2);
  enc.out->put(255);
  state=INIT;
}

/////////////////////////// compress() ///////////////////////

void compress(Reader* in, Writer* out, const char* method,
              const char* filename, const char* comment, bool dosha1) {

  // Get block size
  int bs=4;
  if (method && method[0] && method[1]>='0' && method[1]<='9') {
    bs=method[1]-'0';
    if (method[2]>='0' && method[2]<='9') bs=bs*10+method[2]-'0';
    if (bs>11) bs=11;
  }
  bs=(0x100000<<bs)-4096;

  // Compress in blocks
  StringBuffer sb(bs);
  sb.write(0, bs);
  int n=0;
  while (in && (n=in->read((char*)sb.data(), bs))>0) {
    sb.resize(n);
    compressBlock(&sb, out, method, filename, comment, dosha1);
    filename=0;
    comment=0;
    sb.resize(0);
  }
}

//////////////////////// ZPAQL::assemble() ////////////////////

#ifndef NOJIT
/*
assemble();

Assembles the ZPAQL code in hcomp[0..hlen-1] and stores x86-32 or x86-64
code in rcode[0..rcode_size-1]. Execution begins at rcode[0]. It will not
write beyond the end of rcode, but in any case it returns the number of
bytes that would have been written. It returns 0 in case of error.

The assembled code implements int run() and returns 0 if successful,
1 if the ZPAQL code executes an invalid instruction or jumps out of
bounds, or 2 if OUT throws bad_alloc, or 3 for other OUT exceptions.

A ZPAQL virtual machine has the following state. All values are
unsigned and initially 0:

  a, b, c, d: 32 bit registers (pointed to by their respective parameters)
  f: 1 bit flag register (pointed to)
  r[0..255]: 32 bit registers
  m[0..msize-1]: 8 bit registers, where msize is a power of 2
  h[0..hsize-1]: 32 bit registers, where hsize is a power of 2
  out: pointer to a Writer
  sha1: pointer to a SHA1

Generally a ZPAQL machine is used to compute contexts which are
placed in h. A second machine might post-process, and write its
output to out and sha1. In either case, a machine is called with
its input in a, representing a single byte (0..255) or
(for a postprocessor) EOF (0xffffffff). Execution returs after a
ZPAQL halt instruction.

ZPAQL instructions are 1 byte unless the last 3 bits are 1.
In this case, a second operand byte follows. Opcode 255 is
the only 3 byte instruction. They are organized:

  00dddxxx = unary opcode xxx on destination ddd (ddd < 111)
  00111xxx = special instruction xxx
  01dddsss = assignment: ddd = sss (ddd < 111)
  1xxxxsss = operation xxxx from sss to a

The meaning of sss and ddd are as follows:

  000 = a   (accumulator)
  001 = b
  010 = c
  011 = d
  100 = *b  (means m[b mod msize])
  101 = *c  (means m[c mod msize])
  110 = *d  (means h[d mod hsize])
  111 = n   (constant 0..255 in second byte of instruction)

For example, 01001110 assigns *d to b. The other instructions xxx
are as follows:

Group 00dddxxx where ddd < 111 and xxx is:
  000 = ddd<>a, swap with a (except 00000000 is an error, and swap
        with *b or *c leaves the high bits of a unchanged)
  001 = ddd++, increment
  010 = ddd--, decrement
  011 = ddd!, not (invert all bits)
  100 = ddd=0, clear (set all bits of ddd to 0)
  101 = not used (error)
  110 = not used
  111 = ddd=r n, assign from r[n] to ddd, n=0..255 in next opcode byte
Except:
  00100111 = jt n, jump if f is true (n = -128..127, relative to next opcode)
  00101111 = jf n, jump if f is false (n = -128..127)
  00110111 = r=a n, assign r[n] = a (n = 0..255)

Group 00111xxx where xxx is:
  000 = halt (return)
  001 = output a
  010 = not used
  011 = hash: a = (a + *b + 512) * 773
  100 = hashd: *d = (*d + a + 512) * 773
  101 = not used
  110 = not used
  111 = unconditional jump (n = -128 to 127, relative to next opcode)
  
Group 1xxxxsss where xxxx is:
  0000 = a += sss (add, subtract, multiply, divide sss to a)
  0001 = a -= sss
  0010 = a *= sss
  0011 = a /= sss (unsigned, except set a = 0 if sss is 0)
  0100 = a %= sss (remainder, except set a = 0 if sss is 0)
  0101 = a &= sss (bitwise AND)
  0110 = a &= ~sss (bitwise AND with complement of sss)
  0111 = a |= sss (bitwise OR)
  1000 = a ^= sss (bitwise XOR)
  1001 = a <<= (sss % 32) (left shift by low 5 bits of sss)
  1010 = a >>= (sss % 32) (unsigned, zero bits shifted in)
  1011 = a == sss (compare, set f = true if equal or false otherwise)
  1100 = a < sss (unsigned compare, result in f)
  1101 = a > sss (unsigned compare)
  1110 = not used
  1111 = not used except 11111111 is a 3 byte jump to the absolute address
         in the next 2 bytes in little-endian (LSB first) order.

assemble() translates ZPAQL to 32 bit x86 code to be executed by run().
Registers are mapped as follows:

  eax = source sss from *b, *c, *d or sometimes n
  ecx = pointer to destination *b, *c, *d, or spare
  edx = a
  ebx = f (1 for true, 0 for false)
  esp = stack pointer
  ebp = d
  esi = b
  edi = c

run() saves non-volatile registers (ebp, esi, edi, ebx) on the stack,
loads a, b, c, d, f, and executes the translated instructions.
A halt instruction saves a, b, c, d, f, pops the saved registers
and returns. Invalid instructions or jumps outside of the range
of the ZPAQL code call libzpaq::error().

In 64 bit mode, the following additional registers are used:

  r12 = h
  r14 = r
  r15 = m

*/

// Called by out
static int flush1(ZPAQL* z) {
  try {
    z->flush();
    return 0;
  }
  catch(std::bad_alloc& x) {
    return 2;
  }
  catch(...) {
    return 3;
  }
}

// return true if op is an undefined ZPAQL instruction
static bool iserr(int op) {
  return op==0 || (op>=120 && op<=127) || (op>=240 && op<=254)
    || op==58 || (op<64 && (op%8==5 || op%8==6));
}

// Return length of ZPAQL instruction at hcomp[0]. Assume 0 padding at end.
// A run of identical ++ or -- is counted as 1 instruction.
static int oplen(const U8* hcomp) {
  if (*hcomp==255) return 3;
  if (*hcomp%8==7) return 2;
  if (*hcomp<51 && (*hcomp%8-1)/2==0) {  // ++ or -- opcode
    int i;
    for (i=1; i<127 && hcomp[i]==hcomp[0]; ++i);
    return i;
  }
  return 1;
}

// Write k bytes of x to rcode[o++] MSB first
static void put(U8* rcode, int n, int& o, U32 x, int k) {
  while (k-->0) {
    if (o<n) rcode[o]=(x>>(k*8))&255;
    ++o;
  }
}

// Write 4 bytes of x to rcode[o++] LSB first
static void put4lsb(U8* rcode, int n, int& o, U32 x) {
  for (int k=0; k<4; ++k) {
    if (o<n) rcode[o]=(x>>(k*8))&255;
    ++o;
  }
}

// Write a 1-4 byte x86 opcode without or with an 4 byte operand
// to rcode[o...]
#define put1(x) put(rcode, rcode_size, o, (x), 1)
#define put2(x) put(rcode, rcode_size, o, (x), 2)
#define put3(x) put(rcode, rcode_size, o, (x), 3)
#define put4(x) put(rcode, rcode_size, o, (x), 4)
#define put5(x,y) put4(x), put1(y)
#define put6(x,y) put4(x), put2(y)
#define put4r(x) put4lsb(rcode, rcode_size, o, x)
#define puta(x) t=U32(size_t(x)), put4r(t)
#define put1a(x,y) put1(x), puta(y)
#define put2a(x,y) put2(x), puta(y)
#define put3a(x,y) put3(x), puta(y)
#define put4a(x,y) put4(x), puta(y)
#define put5a(x,y,z) put4(x), put1(y), puta(z)
#define put2l(x,y) put2(x), t=U32(size_t(y)), put4r(t), \
  t=U32(size_t(y)>>(S*4)), put4r(t)

// Assemble ZPAQL in in the HCOMP section of header to rcode,
// but do not write beyond rcode_size. Return the number of
// bytes output or that would have been output.
// Execution starts at rcode[0] and returns 1 if successful or 0
// in case of a ZPAQL execution error.
int ZPAQL::assemble() {

  // x86? (not foolproof)
  const int S=sizeof(char*);      // 4 = x86, 8 = x86-64
  U32 t=0x12345678;
  if (*(char*)&t!=0x78 || (S!=4 && S!=8))
    error("JIT supported only for x86-32 and x86-64");

  const U8* hcomp=&header[hbegin];
  const int hlen=hend-hbegin+2;
  const int msize=m.size();
  const int hsize=h.size();
  static const int regcode[8]={2,6,7,5}; // a,b,c,d.. -> edx,esi,edi,ebp,eax..
  Array<int> it(hlen);            // hcomp -> rcode locations
  int done=0;  // number of instructions assembled (0..hlen)
  int o=5;  // rcode output index, reserve space for jmp

  // Code for the halt instruction (restore registers and return)
  const int halt=o;
  if (S==8) {
    put2l(0x48b9, &a);        // mov rcx, a
    put2(0x8911);             // mov [rcx], edx
    put2l(0x48b9, &b);        // mov rcx, b
    put2(0x8931);             // mov [rcx], esi
    put2l(0x48b9, &c);        // mov rcx, c
    put2(0x8939);             // mov [rcx], edi
    put2l(0x48b9, &d);        // mov rcx, d
    put2(0x8929);             // mov [rcx], ebp
    put2l(0x48b9, &f);        // mov rcx, f
    put2(0x8919);             // mov [rcx], ebx
    put4(0x4883c408);         // add rsp, 8
    put2(0x415f);             // pop r15
    put2(0x415e);             // pop r14
    put2(0x415d);             // pop r13
    put2(0x415c);             // pop r12
  }
  else {
    put2a(0x8915, &a);        // mov [a], edx
    put2a(0x8935, &b);        // mov [b], esi
    put2a(0x893d, &c);        // mov [c], edi
    put2a(0x892d, &d);        // mov [d], ebp
    put2a(0x891d, &f);        // mov [f], ebx
    put3(0x83c40c);           // add esp, 12
  }
  put1(0x5b);                 // pop ebx
  put1(0x5f);                 // pop edi
  put1(0x5e);                 // pop esi
  put1(0x5d);                 // pop ebp
  put1(0xc3);                 // ret

  // Code for the out instruction.
  // Store a=edx at outbuf[bufptr++]. If full, call flush1().
  const int outlabel=o;
  if (S==8) {
    put2l(0x48b8, &outbuf[0]);// mov rax, outbuf.p
    put2l(0x49ba, &bufptr);   // mov r10, &bufptr
    put3(0x418b0a);           // mov rcx, [r10]
    put3(0x881408);           // mov [rax+rcx], dl
    put2(0xffc1);             // inc rcx
    put3(0x41890a);           // mov [r10], ecx
    put2a(0x81f9, outbuf.size());  // cmp rcx, outbuf.size()
    put2(0x7403);             // jz L1
    put2(0x31c0);             // xor eax, eax
    put1(0xc3);               // ret

    put1(0x55);               // L1: push rbp ; call flush1(this)
    put1(0x57);               // push rdi
    put1(0x56);               // push rsi
    put1(0x52);               // push rdx
    put1(0x51);               // push rcx
    put3(0x4889e5);           // mov rbp, rsp
    put4(0x4883c570);         // add rbp, 112
#if defined(unix) && !defined(__CYGWIN__)
    put2l(0x48bf, this);      // mov rdi, this
#else  // Windows
    put2l(0x48b9, this);      // mov rcx, this
#endif
    put2l(0x49bb, &flush1);   // mov r11, &flush1
    put3(0x41ffd3);           // call r11
    put1(0x59);               // pop rcx
    put1(0x5a);               // pop rdx
    put1(0x5e);               // pop rsi
    put1(0x5f);               // pop rdi
    put1(0x5d);               // pop rbp
  }
  else {
    put1a(0xb8, &outbuf[0]);  // mov eax, outbuf.p
    put2a(0x8b0d, &bufptr);   // mov ecx, [bufptr]
    put3(0x881408);           // mov [eax+ecx], dl
    put2(0xffc1);             // inc ecx
    put2a(0x890d, &bufptr);   // mov [bufptr], ecx
    put2a(0x81f9, outbuf.size());  // cmp ecx, outbuf.size()
    put2(0x7403);             // jz L1
    put2(0x31c0);             // xor eax, eax
    put1(0xc3);               // ret
    put3(0x83ec0c);           // L1: sub esp, 12
    put4(0x89542404);         // mov [esp+4], edx
    put3a(0xc70424, this);    // mov [esp], this
    put1a(0xb8, &flush1);     // mov eax, &flush1
    put2(0xffd0);             // call eax
    put4(0x8b542404);         // mov edx, [esp+4]
    put3(0x83c40c);           // add esp, 12
  }
  put1(0xc3);               // ret

  // Set it[i]=1 for each ZPAQL instruction reachable from the previous
  // instruction + 2 if reachable by a jump (or 3 if both).
  it[0]=2;
  assert(hlen>0 && hcomp[hlen-1]==0);  // ends with error
  do {
    done=0;
    const int NONE=0x80000000;
    for (int i=0; i<hlen; ++i) {
      int op=hcomp[i];
      if (it[i]) {
        int next1=i+oplen(hcomp+i), next2=NONE; // next and jump targets
        if (iserr(op)) next1=NONE;  // error
        if (op==56) next1=NONE, next2=0;  // halt
        if (op==255) next1=NONE, next2=hcomp[i+1]+256*hcomp[i+2]; // lj
        if (op==39||op==47||op==63)next2=i+2+(hcomp[i+1]<<24>>24);// jt,jf,jmp
        if (op==63) next1=NONE;  // jmp
        if ((next2<0 || next2>=hlen) && next2!=NONE) next2=hlen-1; // error
        if (next1>=0 && next1<hlen && !(it[next1]&1)) it[next1]|=1, ++done;
        if (next2>=0 && next2<hlen && !(it[next2]&2)) it[next2]|=2, ++done;
      }
    }
  } while (done>0);

  // Set it[i] bits 2-3 to 4, 8, or 12 if a comparison
  //  (==, <, > respectively) does not need to save the result in f,
  // or if a conditional jump (jt, jf) does not need to read f.
  // This is true if a comparison is followed directly by a jt/jf,
  // the jt/jf is not a jump target, the byte before is not a jump
  // target (for a 2 byte comparison), and for the comparison instruction
  // if both paths after the jt/jf lead to another comparison or error
  // before another jt/jf. At most hlen steps are traced because after
  // that it must be an infinite loop.
  for (int i=0; i<hlen; ++i) {
    const int op1=hcomp[i]; // 216..239 = comparison
    const int i2=i+1+(op1%8==7);  // address of next instruction
    const int op2=hcomp[i2];  // 39,47 = jt,jf
    if (it[i] && op1>=216 && op1<240 && (op2==39 || op2==47)
        && it[i2]==1 && (i2==i+1 || it[i+1]==0)) {
      int code=(op1-208)/8*4; // 4,8,12 is ==,<,>
      it[i2]+=code;  // OK to test CF, ZF instead of f
      for (int j=0; j<2 && code; ++j) {  // trace each path from i2
        int k=i2+2; // branch not taken
        if (j==1) k=i2+2+(hcomp[i2+1]<<24>>24);  // branch taken
        for (int l=0; l<hlen && code; ++l) {  // trace at most hlen steps
          if (k<0 || k>=hlen) break;  // out of bounds, pass
          const int op=hcomp[k];
          if (op==39 || op==47) code=0;  // jt,jf, fail
          else if (op>=216 && op<240) break;  // ==,<,>, pass
          else if (iserr(op)) break;  // error, pass
          else if (op==255) k=hcomp[k+1]+256*hcomp[k+2]; // lj
          else if (op==63) k=k+2+(hcomp[k+1]<<24>>24);  // jmp
          else if (op==56) k=0;  // halt
          else k=k+1+(op%8==7);  // ordinary instruction
        }
      }
      it[i]+=code;  // if > 0 then OK to not save flags in f (bl)
    }
  }

  // Start of run(): Save x86 and load ZPAQL registers
  const int start=o;
  assert(start>=16);
  put1(0x55);          // push ebp/rbp
  put1(0x56);          // push esi/rsi
  put1(0x57);          // push edi/rdi
  put1(0x53);          // push ebx/rbx
  if (S==8) {
    put2(0x4154);      // push r12
    put2(0x4155);      // push r13
    put2(0x4156);      // push r14
    put2(0x4157);      // push r15
    put4(0x4883ec08);  // sub rsp, 8
    put2l(0x48b8, &a); // mov rax, a
    put2(0x8b10);      // mov edx, [rax]
    put2l(0x48b8, &b); // mov rax, b
    put2(0x8b30);      // mov esi, [rax]
    put2l(0x48b8, &c); // mov rax, c
    put2(0x8b38);      // mov edi, [rax]
    put2l(0x48b8, &d); // mov rax, d
    put2(0x8b28);      // mov ebp, [rax]
    put2l(0x48b8, &f); // mov rax, f
    put2(0x8b18);      // mov ebx, [rax]
    put2l(0x49bc, &h[0]);   // mov r12, h
    put2l(0x49bd, &outbuf[0]); // mov r13, outbuf.p
    put2l(0x49be, &r[0]);   // mov r14, r
    put2l(0x49bf, &m[0]);   // mov r15, m
  }
  else {
    put3(0x83ec0c);    // sub esp, 12
    put2a(0x8b15, &a); // mov edx, [a]
    put2a(0x8b35, &b); // mov esi, [b]
    put2a(0x8b3d, &c); // mov edi, [c]
    put2a(0x8b2d, &d); // mov ebp, [d]
    put2a(0x8b1d, &f); // mov ebx, [f]
  }

  // Assemble in multiple passes until every byte of hcomp has a translation
  for (int istart=0; istart<hlen; ++istart) {
    int inc=0;
    for (int i=istart; i<hlen && it[i]; i+=inc) {
      const int code=it[i];
      inc=oplen(hcomp+i);

      // If already assembled, then assemble a jump to it
      U32 t;
      assert(it.isize()>i);
      assert(i>=0 && i<hlen);
      if (code>=16) {
        if (i>istart) {
          int a=code-o;
          if (a>-120 && a<120)
            put2(0xeb00+((a-2)&255)); // jmp short o
          else
            put1a(0xe9, a-5);  // jmp near o
        }
        break;
      }

      // Else assemble the instruction at hcomp[i] to rcode[o]
      else {
        assert(i>=0 && i<it.isize());
        assert(it[i]>0 && it[i]<16);
        assert(o>=16);
        it[i]=o;
        ++done;
        const int op=hcomp[i];
        const int arg=hcomp[i+1]+((op==255)?256*hcomp[i+2]:0);
        const int ddd=op/8%8;
        const int sss=op%8;

        // error instruction: return 1
        if (iserr(op)) {
          put1a(0xb8, 1);         // mov eax, 1
          put1a(0xe9, halt-o-4);  // jmp near halt
          continue;
        }

        // Load source *b, *c, *d, or hash (*b) into eax except:
        // {a,b,c,d}=*d, a{+,-,*,&,|,^,=,==,>,>}=*d: load address to eax
        // {a,b,c,d}={*b,*c}: load source into ddd
        if (op==59 || (op>=64 && op<240 && op%8>=4 && op%8<7)) {
          put2(0x89c0+8*regcode[sss-3+(op==59)]);  // mov eax, {esi,edi,ebp}
          const int sz=(sss==6?hsize:msize)-1;
          if (sz>=128) put1a(0x25, sz);            // and eax, dword msize-1
          else put3(0x83e000+sz);                  // and eax, byte msize-1
          const int move=(op>=64 && op<112); // = or else ddd is eax
          if (sss<6) { // ddd={a,b,c,d,*b,*c}
            if (S==8) put5(0x410fb604+8*move*regcode[ddd],0x07);
                                                   // movzx ddd, byte [r15+rax]
            else put3a(0x0fb680+8*move*regcode[ddd], &m[0]);
                                                   // movzx ddd, byte [m+eax]
          }
          else if ((0x06587000>>(op/8))&1) {// {*b,*c,*d,a/,a%,a&~,a<<,a>>}=*d
            if (S==8) put4(0x418b0484);            // mov eax, [r12+rax*4]
            else put3a(0x8b0485, &h[0]);           // mov eax, [h+eax*4]
          }
        }

        // Load destination address *b, *c, *d or hashd (*d) into ecx
        if ((op>=32 && op<56 && op%8<5) || (op>=96 && op<120) || op==60) {
          put2(0x89c1+8*regcode[op/8%8-3-(op==60)]);// mov ecx,{esi,edi,ebp}
          const int sz=(ddd==6||op==60?hsize:msize)-1;
          if (sz>=128) put2a(0x81e1, sz);   // and ecx, dword sz
          else put3(0x83e100+sz);           // and ecx, byte sz
          if (op/8%8==6 || op==60) { // *d
            if (S==8) put4(0x498d0c8c);     // lea rcx, [r12+rcx*4]
            else put3a(0x8d0c8d, &h[0]);    // lea ecx, [ecx*4+h]
          }
          else { // *b, *c
            if (S==8) put4(0x498d0c0f);     // lea rcx, [r15+rcx]
            else put2a(0x8d89, &m[0]);      // lea ecx, [ecx+h]
          }
        }

        // Translate by opcode
        switch((op/8)&31) {
          case 0:  // ddd = a
          case 1:  // ddd = b
          case 2:  // ddd = c
          case 3:  // ddd = d
            switch(sss) {
              case 0:  // ddd<>a (swap)
                put2(0x87d0+regcode[ddd]);   // xchg edx, ddd
                break;
              case 1:  // ddd++
                put3(0x83c000+256*regcode[ddd]+inc); // add ddd, inc
                break;
              case 2:  // ddd--
                put3(0x83e800+256*regcode[ddd]+inc); // sub ddd, inc
                break;
              case 3:  // ddd!
                put2(0xf7d0+regcode[ddd]);   // not ddd
                break;
              case 4:  // ddd=0
                put2(0x31c0+9*regcode[ddd]); // xor ddd,ddd
                break;
              case 7:  // ddd=r n
                if (S==8)
                  put3a(0x418b86+8*regcode[ddd], arg*4); // mov ddd, [r14+n*4]
                else
                  put2a(0x8b05+8*regcode[ddd], (&r[arg]));//mov ddd, [r+n]
                break;
            }
            break;
          case 4:  // ddd = *b
          case 5:  // ddd = *c
            switch(sss) {
              case 0:  // ddd<>a (swap)
                put2(0x8611);                // xchg dl, [ecx]
                break;
              case 1:  // ddd++
                put3(0x800100+inc);          // add byte [ecx], inc
                break;
              case 2:  // ddd--
                put3(0x802900+inc);          // sub byte [ecx], inc
                break;
              case 3:  // ddd!
                put2(0xf611);                // not byte [ecx]
                break;
              case 4:  // ddd=0
                put2(0x31c0);                // xor eax, eax
                put2(0x8801);                // mov [ecx], al
                break;
              case 7:  // jt, jf
              {
                assert(code>=0 && code<16);
                static const unsigned char jtab[2][4]={{5,4,2,7},{4,5,3,6}};
                               // jnz,je,jb,ja, jz,jne,jae,jbe
                if (code<4) put2(0x84db);    // test bl, bl
                if (arg>=128 && arg-257-i>=0 && o-it[arg-257-i]<120)
                  put2(0x7000+256*jtab[op==47][code/4]); // jx short 0
                else
                  put2a(0x0f80+jtab[op==47][code/4], 0); // jx near 0
                break;
              }
            }
            break;
          case 6:  // ddd = *d
            switch(sss) {
              case 0:  // ddd<>a (swap)
                put2(0x8711);             // xchg edx, [ecx]
                break;
              case 1:  // ddd++
                put3(0x830100+inc);       // add dword [ecx], inc
                break;
              case 2:  // ddd--
                put3(0x832900+inc);       // sub dword [ecx], inc
                break;
              case 3:  // ddd!
                put2(0xf711);             // not dword [ecx]
                break;
              case 4:  // ddd=0
                put2(0x31c0);             // xor eax, eax
                put2(0x8901);             // mov [ecx], eax
                break;
              case 7:  // ddd=r n
                if (S==8)
                  put3a(0x418996, arg*4); // mov [r14+n*4], edx
                else
                  put2a(0x8915, &r[arg]); // mov [r+n], edx
                break;
            }
            break;
          case 7:  // special
            switch(op) {
              case 56: // halt
                put2(0x31c0);             // xor eax, eax  ; return 0
                put1a(0xe9, halt-o-4);    // jmp near halt
                break;
              case 57:  // out
                put1a(0xe8, outlabel-o-4);// call outlabel
                put3(0x83f800);           // cmp eax, 0  ; returned error code
                put2(0x7405);             // je L1:
                put1a(0xe9, halt-o-4);    // jmp near halt ; L1:
                break;
              case 59:  // hash: a = (a + *b + 512) * 773
                put3a(0x8d8410, 512);     // lea edx, [eax+edx+512]
                put2a(0x69d0, 773);       // imul edx, eax, 773
                break;
              case 60:  // hashd: *d = (*d + a + 512) * 773
                put2(0x8b01);             // mov eax, [ecx]
                put3a(0x8d8410, 512);     // lea eax, [eax+edx+512]
                put2a(0x69c0, 773);       // imul eax, eax, 773
                put2(0x8901);             // mov [ecx], eax
                break;
              case 63:  // jmp
                put1a(0xe9, 0);           // jmp near 0 (fill in target later)
                break;
            }
            break;
          case 8:   // a=
          case 9:   // b=
          case 10:  // c=
          case 11:  // d=
            if (sss==7)  // n
              put1a(0xb8+regcode[ddd], arg);         // mov ddd, n
            else if (sss==6) { // *d
              if (S==8)
                put4(0x418b0484+(regcode[ddd]<<11)); // mov ddd, [r12+rax*4]
              else
                put3a(0x8b0485+(regcode[ddd]<<11),&h[0]);// mov ddd, [h+eax*4]
            }
            else if (sss<4) // a, b, c, d
              put2(0x89c0+regcode[ddd]+8*regcode[sss]);// mov ddd,sss
            break;
          case 12:  // *b=
          case 13:  // *c=
            if (sss==7) put3(0xc60100+arg);          // mov byte [ecx], n
            else if (sss==0) put2(0x8811);           // mov byte [ecx], dl
            else {
              if (sss<4) put2(0x89c0+8*regcode[sss]);// mov eax, sss
              put2(0x8801);                          // mov byte [ecx], al
            }
            break;
          case 14:  // *d=
            if (sss<7) put2(0x8901+8*regcode[sss]);  // mov [ecx], sss
            else put2a(0xc701, arg);                 // mov dword [ecx], n
            break;
          case 15: break; // not used
          case 16:  // a+=
            if (sss==6) {
              if (S==8) put4(0x41031484);            // add edx, [r12+rax*4]
              else put3a(0x031485, &h[0]);           // add edx, [h+eax*4]
            }
            else if (sss<7) put2(0x01c2+8*regcode[sss]);// add edx, sss
            else if (arg>=128) put2a(0x81c2, arg);   // add edx, n
            else put3(0x83c200+arg);                 // add edx, byte n
            break;
          case 17:  // a-=
            if (sss==6) {
              if (S==8) put4(0x412b1484);            // sub edx, [r12+rax*4]
              else put3a(0x2b1485, &h[0]);           // sub edx, [h+eax*4]
            }
            else if (sss<7) put2(0x29c2+8*regcode[sss]);// sub edx, sss
            else if (arg>=128) put2a(0x81ea, arg);   // sub edx, n
            else put3(0x83ea00+arg);                 // sub edx, byte n
            break;
          case 18:  // a*=
            if (sss==6) {
              if (S==8) put5(0x410faf14,0x84);       // imul edx, [r12+rax*4]
              else put4a(0x0faf1485, &h[0]);         // imul edx, [h+eax*4]
            }
            else if (sss<7) put3(0x0fafd0+regcode[sss]);// imul edx, sss
            else if (arg>=128) put2a(0x69d2, arg);   // imul edx, n
            else put3(0x6bd200+arg);                 // imul edx, byte n
            break;
          case 19:  // a/=
          case 20:  // a%=
            if (sss<7) put2(0x89c1+8*regcode[sss]);  // mov ecx, sss
            else put1a(0xb9, arg);                   // mov ecx, n
            put2(0x85c9);                            // test ecx, ecx
            put3(0x0f44d1);                          // cmovz edx, ecx
            put2(0x7408-2*(op/8==20));               // jz (over rest)
            put2(0x89d0);                            // mov eax, edx
            put2(0x31d2);                            // xor edx, edx
            put2(0xf7f1);                            // div ecx
            if (op/8==19) put2(0x89c2);              // mov edx, eax
            break;
          case 21:  // a&=
            if (sss==6) {
              if (S==8) put4(0x41231484);            // and edx, [r12+rax*4]
              else put3a(0x231485, &h[0]);           // and edx, [h+eax*4]
            }
            else if (sss<7) put2(0x21c2+8*regcode[sss]);// and edx, sss
            else if (arg>=128) put2a(0x81e2, arg);   // and edx, n
            else put3(0x83e200+arg);                 // and edx, byte n
            break;
          case 22:  // a&~
            if (sss==7) {
              if (arg<128) put3(0x83e200+(~arg&255));// and edx, byte ~n
              else put2a(0x81e2, ~arg);              // and edx, ~n
            }
            else {
              if (sss<4) put2(0x89c0+8*regcode[sss]);// mov eax, sss
              put2(0xf7d0);                          // not eax
              put2(0x21c2);                          // and edx, eax
            }
            break;
          case 23:  // a|=
            if (sss==6) {
              if (S==8) put4(0x410b1484);            // or edx, [r12+rax*4]
              else put3a(0x0b1485, &h[0]);           // or edx, [h+eax*4]
            }
            else if (sss<7) put2(0x09c2+8*regcode[sss]);// or edx, sss
            else if (arg>=128) put2a(0x81ca, arg);   // or edx, n
            else put3(0x83ca00+arg);                 // or edx, byte n
            break;
          case 24:  // a^=
            if (sss==6) {
              if (S==8) put4(0x41331484);            // xor edx, [r12+rax*4]
              else put3a(0x331485, &h[0]);           // xor edx, [h+eax*4]
            }
            else if (sss<7) put2(0x31c2+8*regcode[sss]);// xor edx, sss
            else if (arg>=128) put2a(0x81f2, arg);   // xor edx, byte n
            else put3(0x83f200+arg);                 // xor edx, n
            break;
          case 25:  // a<<=
          case 26:  // a>>=
            if (sss==7)  // sss = n
              put3(0xc1e200+8*256*(op/8==26)+arg);   // shl/shr n
            else {
              put2(0x89c1+8*regcode[sss]);           // mov ecx, sss
              put2(0xd3e2+8*(op/8==26));             // shl/shr edx, cl
            }
            break;
          case 27:  // a==
          case 28:  // a<
          case 29:  // a>
            if (sss==6) {
              if (S==8) put4(0x413b1484);            // cmp edx, [r12+rax*4]
              else put3a(0x3b1485, &h[0]);           // cmp edx, [h+eax*4]
            }
            else if (sss==7)  // sss = n
              put2a(0x81fa, arg);                    // cmp edx, dword n
            else
              put2(0x39c2+8*regcode[sss]);           // cmp edx, sss
            if (code<4) {
              if (op/8==27) put3(0x0f94c3);          // setz bl
              if (op/8==28) put3(0x0f92c3);          // setc bl
              if (op/8==29) put3(0x0f97c3);          // seta bl
            }
            break;
          case 30:  // not used
          case 31:  // 255 = lj
            if (op==255) put1a(0xe9, 0);             // jmp near
            break;
        }
      }
    }
  }

  // Finish first pass
  const int rsize=o;
  if (o>rcode_size) return rsize;

  // Fill in jump addresses (second pass)
  for (int i=0; i<hlen; ++i) {
    if (it[i]<16) continue;
    int op=hcomp[i];
    if (op==39 || op==47 || op==63 || op==255) {  // jt, jf, jmp, lj
      int target=hcomp[i+1];
      if (op==255) target+=hcomp[i+2]*256;  // lj
      else {
        if (target>=128) target-=256;
        target+=i+2;
      }
      if (target<0 || target>=hlen) target=hlen-1;  // runtime ZPAQL error
      o=it[i];
      assert(o>=16 && o<rcode_size);
      if ((op==39 || op==47) && rcode[o]==0x84) o+=2;  // jt, jf -> skip test
      assert(o>=16 && o<rcode_size);
      if (rcode[o]==0x0f) ++o;  // first byte of jz near, jnz near
      assert(o<rcode_size);
      op=rcode[o++];  // x86 opcode
      target=it[target]-o;
      if ((op>=0x72 && op<0x78) || op==0xeb) {  // jx, jmp short
        --target;
        if (target<-128 || target>127)
          error("Cannot code x86 short jump");
        assert(o<rcode_size);
        rcode[o]=target&255;
      }
      else if ((op>=0x82 && op<0x88) || op==0xe9) // jx, jmp near
      {
        target-=4;
        puta(target);
      }
      else assert(false);  // not a x86 jump
    }
  }

  // Jump to start
  o=0;
  put1a(0xe9, start-5);  // jmp near start
  return rsize;
}

//////////////////////// Predictor::assemble_p() /////////////////////

// Assemble the ZPAQL code in the HCOMP section of z.header to pcomp and
// return the number of bytes of x86 or x86-64 code written, or that would
// be written if pcomp were large enough. The code for predict() begins
// at pr.pcomp[0] and update() at pr.pcomp[5], both as jmp instructions.

// The assembled code is equivalent to int predict(Predictor*)
// and void update(Predictor*, int y); The Preditor address is placed in
// edi/rdi. The update bit y is placed in ebp/rbp.

int Predictor::assemble_p() {
  Predictor& pr=*this;
  U8* rcode=pr.pcode;         // x86 output array
  int rcode_size=pcode_size;  // output size
  int o=0;                    // output index in pcode
  const int S=sizeof(char*);  // 4 or 8
  U8* hcomp=&pr.z.header[0];  // The code to translate
#define off(x)  ((char*)&(pr.x)-(char*)&pr)
#define offc(x) ((char*)&(pr.comp[i].x)-(char*)&pr)

  // test for little-endian (probably x86)
  U32 t=0x12345678;
  if (*(char*)&t!=0x78 || (S!=4 && S!=8))
    error("JIT supported only for x86-32 and x86-64");

  // Initialize for predict(). Put predictor address in edi/rdi
  put1a(0xe9, 5);             // jmp predict
  put1a(0, 0x90909000);       // reserve space for jmp update
  put1(0x53);                 // push ebx/rbx
  put1(0x55);                 // push ebp/rbp
  put1(0x56);                 // push esi/rsi
  put1(0x57);                 // push edi/rdi
  if (S==4)
    put4(0x8b7c2414);         // mov edi,[esp+0x14] ; pr
  else {
#if !defined(unix) || defined(__CYGWIN__)
    put3(0x4889cf);           // mov rdi, rcx (1st arg in Win64)
#endif
  }

  // Code predict() for each component
  const int n=hcomp[6];  // number of components
  U8* cp=hcomp+7;
  for (int i=0; i<n; ++i, cp+=compsize[cp[0]]) {
    if (cp-hcomp>=pr.z.cend) error("comp too big");
    if (cp[0]<1 || cp[0]>9) error("invalid component");
    assert(compsize[cp[0]]>0 && compsize[cp[0]]<8);
    switch (cp[0]) {

      case CONS:  // c
        break;

      case CM:  // sizebits limit
        // Component& cr=comp[i];
        // cr.cxt=h[i]^hmap4;
        // p[i]=stretch(cr.cm(cr.cxt)>>17);

        put2a(0x8b87, off(h[i]));              // mov eax, [edi+&h[i]]
        put2a(0x3387, off(hmap4));             // xor eax, [edi+&hmap4]
        put1a(0x25, (1<<cp[1])-1);             // and eax, size-1
        put2a(0x8987, offc(cxt));              // mov [edi+cxt], eax
        if (S==8) put1(0x48);                  // rex.w (esi->rsi)
        put2a(0x8bb7, offc(cm));               // mov esi, [edi+&cm]
        put3(0x8b0486);                        // mov eax, [esi+eax*4]
        put3(0xc1e811);                        // shr eax, 17
        put4a(0x0fbf8447, off(stretcht));      // movsx eax,word[edi+eax*2+..]
        put2a(0x8987, off(p[i]));              // mov [edi+&p[i]], eax
        break;

      case ISSE:  // sizebits j -- c=hi, cxt=bh
        // assert((hmap4&15)>0);
        // if (c8==1 || (c8&0xf0)==16)
        //   cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        // cr.cxt=cr.ht[cr.c+(hmap4&15)];  // bit history
        // int *wt=(int*)&cr.cm[cr.cxt*2];
        // p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);

      case ICM: // sizebits
        // assert((hmap4&15)>0);
        // if (c8==1 || (c8&0xf0)==16) cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        // cr.cxt=cr.ht[cr.c+(hmap4&15)];
        // p[i]=stretch(cr.cm(cr.cxt)>>8);
        //
        // Find cxt row in hash table ht. ht has rows of 16 indexed by the low
        // sizebits of cxt with element 0 having the next higher 8 bits for
        // collision detection. If not found after 3 adjacent tries, replace
        // row with lowest element 1 as priority. Return index of row.
        //
        // size_t Predictor::find(Array<U8>& ht, int sizebits, U32 cxt) {
        //  assert(ht.size()==size_t(16)<<sizebits);
        //  int chk=cxt>>sizebits&255;
        //  size_t h0=(cxt*16)&(ht.size()-16);
        //  if (ht[h0]==chk) return h0;
        //  size_t h1=h0^16;
        //  if (ht[h1]==chk) return h1;
        //  size_t h2=h0^32;
        //  if (ht[h2]==chk) return h2;
        //  if (ht[h0+1]<=ht[h1+1] && ht[h0+1]<=ht[h2+1])
        //    return memset(&ht[h0], 0, 16), ht[h0]=chk, h0;
        //  else if (ht[h1+1]<ht[h2+1])
        //    return memset(&ht[h1], 0, 16), ht[h1]=chk, h1;
        //  else
        //    return memset(&ht[h2], 0, 16), ht[h2]=chk, h2;
        // }

        if (S==8) put1(0x48);                  // rex.w
        put2a(0x8bb7, offc(ht));               // mov esi, [edi+&ht]
        put2(0x8b07);                          // mov eax, edi ; c8
        put2(0x89c1);                          // mov ecx, eax ; c8
        put3(0x83f801);                        // cmp eax, 1
        put2(0x740a);                          // je L1
        put1a(0x25, 240);                      // and eax, 0xf0
        put3(0x83f810);                        // cmp eax, 16
        put2(0x7576);                          // jne L2 ; skip find()
           // L1: ; find cxt in ht, return index in eax
        put3(0xc1e104);                        // shl ecx, 4
        put2a(0x038f, off(h[i]));              // add [edi+&h[i]]
        put2(0x89c8);                          // mov eax, ecx ; cxt
        put3(0xc1e902+cp[1]);                  // shr ecx, sizebits+2
        put2a(0x81e1, 255);                    // and eax, 255 ; chk
        put3(0xc1e004);                        // shl eax, 4
        put1a(0x25, (64<<cp[1])-16);           // and eax, ht.size()-16 = h0
        put3(0x3a0c06);                        // cmp cl, [esi+eax] ; ht[h0]
        put2(0x744d);                          // je L3 ; match h0
        put3(0x83f010);                        // xor eax, 16 ; h1
        put3(0x3a0c06);                        // cmp cl, [esi+eax]
        put2(0x7445);                          // je L3 ; match h1
        put3(0x83f030);                        // xor eax, 48 ; h2
        put3(0x3a0c06);                        // cmp cl, [esi+eax]
        put2(0x743d);                          // je L3 ; match h2
          // No checksum match, so replace the lowest priority among h0,h1,h2
        put3(0x83f021);                        // xor eax, 33 ; h0+1
        put3(0x8a1c06);                        // mov bl, [esi+eax] ; ht[h0+1]
        put2(0x89c2);                          // mov edx, eax ; h0+1
        put3(0x83f220);                        // xor edx, 32  ; h2+1
        put3(0x3a1c16);                        // cmp bl, [esi+edx]
        put2(0x7708);                          // ja L4 ; test h1 vs h2
        put3(0x83f230);                        // xor edx, 48  ; h1+1
        put3(0x3a1c16);                        // cmp bl, [esi+edx]
        put2(0x7611);                          // jbe L7 ; replace h0
          // L4: ; h0 is not lowest, so replace h1 or h2
        put3(0x83f010);                        // xor eax, 16 ; h1+1
        put3(0x8a1c06);                        // mov bl, [esi+eax]
        put3(0x83f030);                        // xor eax, 48 ; h2+1
        put3(0x3a1c06);                        // cmp bl, [esi+eax]
        put2(0x7303);                          // jae L7
        put3(0x83f030);                        // xor eax, 48 ; h1+1
          // L7: ; replace row pointed to by eax = h0,h1,h2
        put3(0x83f001);                        // xor eax, 1
        put3(0x890c06);                        // mov [esi+eax], ecx ; chk
        put2(0x31c9);                          // xor ecx, ecx
        put4(0x894c0604);                      // mov [esi+eax+4], ecx
        put4(0x894c0608);                      // mov [esi+eax+8], ecx
        put4(0x894c060c);                      // mov [esi+eax+12], ecx
          // L3: ; save nibble context (in eax) in c
        put2a(0x8987, offc(c));                // mov [edi+c], eax
        put2(0xeb06);                          // jmp L8
          // L2: ; get nibble context
        put2a(0x8b87, offc(c));                // mov eax, [edi+c]
          // L8: ; nibble context is in eax
        put2a(0x8b97, off(hmap4));             // mov edx, [edi+&hmap4]
        put3(0x83e20f);                        // and edx, 15  ; hmap4
        put2(0x01d0);                          // add eax, edx ; c+(hmap4&15)
        put4(0x0fb61406);                      // movzx edx, byte [esi+eax]
        put2a(0x8997, offc(cxt));              // mov [edi+&cxt], edx ; cxt=bh
        if (S==8) put1(0x48);                  // rex.w
        put2a(0x8bb7, offc(cm));               // mov esi, [edi+&cm] ; cm

        // esi points to cm[256] (ICM) or cm[512] (ISSE) with 23 bit
        // prediction (ICM) or a pair of 20 bit signed weights (ISSE).
        // cxt = bit history bh (0..255) is in edx.
        if (cp[0]==ICM) {
          put3(0x8b0496);                      // mov eax, [esi+edx*4];cm[bh]
          put3(0xc1e808);                      // shr eax, 8
          put4a(0x0fbf8447, off(stretcht));    // movsx eax,word[edi+eax*2+..]
        }
        else {  // ISSE
          put2a(0x8b87, off(p[cp[2]]));        // mov eax, [edi+&p[j]]
          put4(0x0faf04d6);                    // imul eax, [esi+edx*8] ;wt[0]
          put4(0x8b4cd604);                    // mov ecx, [esi+edx*8+4];wt[1]
          put3(0xc1e106);                      // shl ecx, 6
          put2(0x01c8);                        // add eax, ecx
          put3(0xc1f810);                      // sar eax, 16
          put1a(0xb9, 2047);                   // mov ecx, 2047
          put2(0x39c8);                        // cmp eax, ecx
          put3(0x0f4fc1);                      // cmovg eax, ecx
          put1a(0xb9, -2048);                  // mov ecx, -2048
          put2(0x39c8);                        // cmp eax, ecx
          put3(0x0f4cc1);                      // cmovl eax, ecx

        }
        put2a(0x8987, off(p[i]));              // mov [edi+&p[i]], eax
        break;

      case MATCH: // sizebits bufbits: a=len, b=offset, c=bit, cxt=bitpos,
                  //                   ht=buf, limit=pos
        // assert(cr.cm.size()==(size_t(1)<<cp[1]));
        // assert(cr.ht.size()==(size_t(1)<<cp[2]));
        // assert(cr.a<=255);
        // assert(cr.c==0 || cr.c==1);
        // assert(cr.cxt<8);
        // assert(cr.limit<cr.ht.size());
        // if (cr.a==0) p[i]=0;
        // else {
        //   cr.c=(cr.ht(cr.limit-cr.b)>>(7-cr.cxt))&1; // predicted bit
        //   p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        // }

        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(ht));       // mov esi, [edi+&ht]

        // If match length (a) is 0 then p[i]=0
        put2a(0x8b87, offc(a));        // mov eax, [edi+&a]
        put2(0x85c0);                  // test eax, eax
        put2(0x7449);                  // jz L2 ; p[i]=0

        // Else put predicted bit in c
        put1a(0xb9, 7);                // mov ecx, 7
        put2a(0x2b8f, offc(cxt));      // sub ecx, [edi+&cxt]
        put2a(0x8b87, offc(limit));    // mov eax, [edi+&limit]
        put2a(0x2b87, offc(b));        // sub eax, [edi+&b]
        put1a(0x25, (1<<cp[2])-1);     // and eax, ht.size()-1
        put4(0x0fb60406);              // movzx eax, byte [esi+eax]
        put2(0xd3e8);                  // shr eax, cl
        put3(0x83e001);                // and eax, 1  ; predicted bit
        put2a(0x8987, offc(c));        // mov [edi+&c], eax ; c

        // p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        put2a(0x8b87, offc(a));        // mov eax, [edi+&a]
        put3a(0x8b8487, off(dt2k));    // mov eax, [edi+eax*4+&dt2k] ; weight
        put2(0x7402);                  // jz L1 ; z if c==0
        put2(0xf7d8);                  // neg eax
        put1a(0x25, 0x7fff);           // L1: and eax, 32767
        put4a(0x0fbf8447, off(stretcht)); //movsx eax, word [edi+eax*2+...]
        put2a(0x8987, off(p[i]));      // L2: mov [edi+&p[i]], eax
        break;

      case AVG: // j k wt
        // p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;

        put2a(0x8b87, off(p[cp[1]]));  // mov eax, [edi+&p[j]]
        put2a(0x2b87, off(p[cp[2]]));  // sub eax, [edi+&p[k]]
        put2a(0x69c0, cp[3]);          // imul eax, wt
        put3(0xc1f808);                // sar eax, 8
        put2a(0x0387, off(p[cp[2]]));  // add eax, [edi+&p[k]]
        put2a(0x8987, off(p[i]));      // mov [edi+&p[i]], eax
        break;

      case MIX2:   // sizebits j k rate mask
                   // c=size cm=wt[size] cxt=input
        // cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));
        // assert(cr.cxt<cr.a16.size());
        // int w=cr.a16[cr.cxt];
        // assert(w>=0 && w<65536);
        // p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;
        // assert(p[i]>=-2048 && p[i]<2048);

        put2(0x8b07);                  // mov eax, [edi] ; c8
        put1a(0x25, cp[5]);            // and eax, mask
        put2a(0x0387, off(h[i]));      // add eax, [edi+&h[i]]
        put1a(0x25, (1<<cp[1])-1);     // and eax, size-1
        put2a(0x8987, offc(cxt));      // mov [edi+&cxt], eax ; cxt
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(a16));      // mov esi, [edi+&a16]
        put4(0x0fb70446);              // movzx eax, word [edi+eax*2] ; w
        put2a(0x8b8f, off(p[cp[2]]));  // mov ecx, [edi+&p[j]]
        put2a(0x8b97, off(p[cp[3]]));  // mov edx, [edi+&p[k]]
        put2(0x29d1);                  // sub ecx, edx
        put3(0x0fafc8);                // imul ecx, eax
        put3(0xc1e210);                // shl edx, 16
        put2(0x01d1);                  // add ecx, edx
        put3(0xc1f910);                // sar ecx, 16
        put2a(0x898f, off(p[i]));      // mov [edi+&p[i]]
        break;

      case MIX:    // sizebits j m rate mask
                   // c=size cm=wt[size][m] cxt=index of wt in cm
        // int m=cp[3];
        // assert(m>=1 && m<=i);
        // cr.cxt=h[i]+(c8&cp[5]);
        // cr.cxt=(cr.cxt&(cr.c-1))*m; // pointer to row of weights
        // assert(cr.cxt<=cr.cm.size()-m);
        // int* wt=(int*)&cr.cm[cr.cxt];
        // p[i]=0;
        // for (int j=0; j<m; ++j)
        //   p[i]+=(wt[j]>>8)*p[cp[2]+j];
        // p[i]=clamp2k(p[i]>>8);

        put2(0x8b07);                          // mov eax, [edi] ; c8
        put1a(0x25, cp[5]);                    // and eax, mask
        put2a(0x0387, off(h[i]));              // add eax, [edi+&h[i]]
        put1a(0x25, (1<<cp[1])-1);             // and eax, size-1
        put2a(0x69c0, cp[3]);                  // imul eax, m
        put2a(0x8987, offc(cxt));              // mov [edi+&cxt], eax ; cxt
        if (S==8) put1(0x48);                  // rex.w
        put2a(0x8bb7, offc(cm));               // mov esi, [edi+&cm]
        if (S==8) put1(0x48);                  // rex.w
        put3(0x8d3486);                        // lea esi, [esi+eax*4] ; wt

        // Unroll summation loop: esi=wt[0..m-1]
        for (int k=0; k<cp[3]; k+=8) {
          const int tail=cp[3]-k;  // number of elements remaining

          // pack 8 elements of wt in xmm1, 8 elements of p in xmm3
          put4a(0xf30f6f8e, k*4);              // movdqu xmm1, [esi+k*4]
          if (tail>3) put4a(0xf30f6f96, k*4+16);//movdqu xmm2, [esi+k*4+16]
          put5(0x660f72e1,0x08);               // psrad xmm1, 8
          if (tail>3) put5(0x660f72e2,0x08);   // psrad xmm2, 8
          put4(0x660f6bca);                    // packssdw xmm1, xmm2
          put4a(0xf30f6f9f, off(p[cp[2]+k]));  // movdqu xmm3, [edi+&p[j+k]]
          if (tail>3)
            put4a(0xf30f6fa7,off(p[cp[2]+k+4]));//movdqu xmm4, [edi+&p[j+k+4]]
          put4(0x660f6bdc);                    // packssdw, xmm3, xmm4
          if (tail>0 && tail<8) {  // last loop, mask extra weights
            put4(0x660f76ed);                  // pcmpeqd xmm5, xmm5 ; -1
            put5(0x660f73dd, 16-tail*2);       // psrldq xmm5, 16-tail*2
            put4(0x660fdbcd);                  // pand xmm1, xmm5
          }
          if (k==0) {  // first loop, initialize sum in xmm0
            put4(0xf30f6fc1);                  // movdqu xmm0, xmm1
            put4(0x660ff5c3);                  // pmaddwd xmm0, xmm3
          }
          else {  // accumulate sum in xmm0
            put4(0x660ff5cb);                  // pmaddwd xmm1, xmm3
            put4(0x660ffec1);                  // paddd xmm0, xmm1
          }
        }

        // Add up the 4 elements of xmm0 = p[i] in the first element
        put4(0xf30f6fc8);                      // movdqu xmm1, xmm0
        put5(0x660f73d9,0x08);                 // psrldq xmm1, 8
        put4(0x660ffec1);                      // paddd xmm0, xmm1
        put4(0xf30f6fc8);                      // movdqu xmm1, xmm0
        put5(0x660f73d9,0x04);                 // psrldq xmm1, 4
        put4(0x660ffec1);                      // paddd xmm0, xmm1
        put4(0x660f7ec0);                      // movd eax, xmm0 ; p[i]
        put3(0xc1f808);                        // sar eax, 8
        put1a(0x3d, 2047);                     // cmp eax, 2047
        put2(0x7e05);                          // jle L1
        put1a(0xb8, 2047);                     // mov eax, 2047
        put1a(0x3d, -2048);                    // L1: cmp eax, -2048
        put2(0x7d05);                          // jge, L2
        put1a(0xb8, -2048);                    // mov eax, -2048
        put2a(0x8987, off(p[i]));              // L2: mov [edi+&p[i]], eax
        break;

      case SSE:  // sizebits j start limit
        // cr.cxt=(h[i]+c8)*32;
        // int pq=p[cp[2]]+992;
        // if (pq<0) pq=0;
        // if (pq>1983) pq=1983;
        // int wt=pq&63;
        // pq>>=6;
        // assert(pq>=0 && pq<=30);
        // cr.cxt+=pq;
        // p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)       // p0
        //               +(cr.cm(cr.cxt+1)>>10)*wt)>>13);  // p1
        // // p = p0*(64-wt)+p1*wt = (p1-p0)*wt + p0*64
        // cr.cxt+=wt>>5;

        put2a(0x8b8f, off(h[i]));      // mov ecx, [edi+&h[i]]
        put2(0x030f);                  // add ecx, [edi]  ; c0
        put2a(0x81e1, (1<<cp[1])-1);   // and ecx, size-1
        put3(0xc1e105);                // shl ecx, 5  ; cxt in 0..size*32-32
        put2a(0x8b87, off(p[cp[2]]));  // mov eax, [edi+&p[j]] ; pq
        put1a(0x05, 992);              // add eax, 992
        put2(0x31d2);                  // xor edx, edx ; 0
        put2(0x39d0);                  // cmp eax, edx
        put3(0x0f4cc2);                // cmovl eax, edx
        put1a(0xba, 1983);             // mov edx, 1983
        put2(0x39d0);                  // cmp eax, edx
        put3(0x0f4fc2);                // cmovg eax, edx ; pq in 0..1983
        put2(0x89c2);                  // mov edx, eax
        put3(0x83e23f);                // and edx, 63  ; wt in 0..63
        put3(0xc1e806);                // shr eax, 6   ; pq in 0..30
        put2(0x01c1);                  // add ecx, eax ; cxt in 0..size*32-2
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(cm));       // mov esi, [edi+cm]
        put3(0x8b048e);                // mov eax, [esi+ecx*4] ; cm[cxt]
        put4(0x8b5c8e04);              // mov ebx, [esi+ecx*4+4] ; cm[cxt+1]
        put3(0x83fa20);                // cmp edx, 32  ; wt
        put3(0x83d9ff);                // sbb ecx, -1  ; cxt+=wt>>5
        put2a(0x898f, offc(cxt));      // mov [edi+cxt], ecx  ; cxt saved
        put3(0xc1e80a);                // shr eax, 10 ; p0 = cm[cxt]>>10
        put3(0xc1eb0a);                // shr ebx, 10 ; p1 = cm[cxt+1]>>10
        put2(0x29c3);                  // sub ebx, eax, ; p1-p0
        put3(0x0fafda);                // imul ebx, edx ; (p1-p0)*wt
        put3(0xc1e006);                // shr eax, 6
        put2(0x01d8);                  // add eax, ebx ; p in 0..2^28-1
        put3(0xc1e80d);                // shr eax, 13  ; p in 0..32767
        put4a(0x0fbf8447, off(stretcht));  // movsx eax, word [edi+eax*2+...]
        put2a(0x8987, off(p[i]));      // mov [edi+&p[i]], eax
        break;

      default:
        error("invalid ZPAQ component");
    }
  }

  // return squash(p[n-1])
  put2a(0x8b87, off(p[n-1]));          // mov eax, [edi+...]
  put1a(0x05, 0x800);                  // add eax, 2048
  put4a(0x0fbf8447, off(squasht[0]));  // movsx eax, word [edi+eax*2+...]
  put1(0x5f);                          // pop edi
  put1(0x5e);                          // pop esi
  put1(0x5d);                          // pop ebp
  put1(0x5b);                          // pop ebx
  put1(0xc3);                          // ret

  // Initialize for update() Put predictor address in edi/rdi
  // and bit y=0..1 in ebp
  int save_o=o;
  o=5;
  put1a(0xe9, save_o-10);      // jmp update
  o=save_o;
  put1(0x53);                  // push ebx/rbx
  put1(0x55);                  // push ebp/rbp
  put1(0x56);                  // push esi/rsi
  put1(0x57);                  // push edi/rdi
  if (S==4) {
    put4(0x8b7c2414);          // mov edi,[esp+0x14] ; (1st arg = pr)
    put4(0x8b6c2418);          // mov ebp,[esp+0x18] ; (2nd arg = y)
  }
  else {
#if defined(unix) && !defined(__CYGWIN__)  // (1st arg already in rdi)
    put3(0x4889f5);            // mov rbp, rsi (2nd arg in Linux-64)
#else
    put3(0x4889cf);            // mov rdi, rcx (1st arg in Win64)
    put3(0x4889d5);            // mov rbp, rdx (2nd arg)
#endif
  }

  // Code update() for each component
  cp=hcomp+7;
  for (int i=0; i<n; ++i, cp+=compsize[cp[0]]) {
    assert(cp-hcomp<pr.z.cend);
    assert (cp[0]>=1 && cp[0]<=9);
    assert(compsize[cp[0]]>0 && compsize[cp[0]]<8);
    switch (cp[0]) {

      case CONS:  // c
        break;

      case SSE:  // sizebits j start limit
      case CM:   // sizebits limit
        // train(cr, y);
        //
        // reduce prediction error in cr.cm
        // void train(Component& cr, int y) {
        //   assert(y==0 || y==1);
        //   U32& pn=cr.cm(cr.cxt);
        //   U32 count=pn&0x3ff;
        //   int error=y*32767-(cr.cm(cr.cxt)>>17);
        //   pn+=(error*dt[count]&-1024)+(count<cr.limit);

        if (S==8) put1(0x48);          // rex.w (esi->rsi)
        put2a(0x8bb7, offc(cm));       // mov esi,[edi+cm]  ; cm
        put2a(0x8b87, offc(cxt));      // mov eax,[edi+cxt] ; cxt
        put1a(0x25, pr.comp[i].cm.size()-1);  // and eax, size-1
        if (S==8) put1(0x48);          // rex.w
        put3(0x8d3486);                // lea esi,[esi+eax*4] ; &cm[cxt]
        put2(0x8b06);                  // mov eax,[esi] ; cm[cxt]
        put2(0x89c2);                  // mov edx, eax  ; cm[cxt]
        put3(0xc1e811);                // shr eax, 17   ; cm[cxt]>>17
        put2(0x89e9);                  // mov ecx, ebp  ; y
        put3(0xc1e10f);                // shl ecx, 15   ; y*32768
        put2(0x29e9);                  // sub ecx, ebp  ; y*32767
        put2(0x29c1);                  // sub ecx, eax  ; error
        put2a(0x81e2, 0x3ff);          // and edx, 1023 ; count
        put3a(0x8b8497, off(dt));      // mov eax,[edi+edx*4+dt] ; dt[count]
        put3(0x0fafc8);                // imul ecx, eax ; error*dt[count]
        put2a(0x81e1, 0xfffffc00);     // and ecx, -1024
        put2a(0x81fa, cp[2+2*(cp[0]==SSE)]*4); // cmp edx, limit*4
        put2(0x110e);                  // adc [esi], ecx ; pn+=...
        break;

      case ICM:   // sizebits: cxt=bh, ht[c][0..15]=bh row
        // cr.ht[cr.c+(hmap4&15)]=st.next(cr.ht[cr.c+(hmap4&15)], y);
        // U32& pn=cr.cm(cr.cxt);
        // pn+=int(y*32767-(pn>>8))>>2;

      case ISSE:  // sizebits j  -- c=hi, cxt=bh
        // assert(cr.cxt==cr.ht[cr.c+(hmap4&15)]);
        // int err=y*32767-squash(p[i]);
        // int *wt=(int*)&cr.cm[cr.cxt*2];
        // wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));
        // wt[1]=clamp512k(wt[1]+((err+16)>>5));
        // cr.ht[cr.c+(hmap4&15)]=st.next(cr.cxt, y);

        // update bit history bh to next(bh,y=ebp) in ht[c+(hmap4&15)]
        put3(0x8b4700+off(hmap4));     // mov eax, [edi+&hmap4]
        put3(0x83e00f);                // and eax, 15
        put2a(0x0387, offc(c));        // add eax [edi+&c] ; cxt
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(ht));       // mov esi, [edi+&ht]
        put4(0x0fb61406);              // movzx edx, byte [esi+eax] ; bh
        put4(0x8d5c9500);              // lea ebx, [ebp+edx*4] ; index to st
        put4a(0x0fb69c1f, off(st));    // movzx ebx,byte[edi+ebx+st]; next bh
        put3(0x881c06);                // mov [esi+eax], bl ; save next bh
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(cm));       // mov esi, [edi+&cm]

        // ICM: update cm[cxt=edx=bit history] to reduce prediction error
        // esi = &cm
        if (cp[0]==ICM) {
          if (S==8) put1(0x48);        // rex.w
          put3(0x8d3496);              // lea esi, [esi+edx*4] ; &cm[bh]
          put2(0x8b06);                // mov eax, [esi] ; pn
          put3(0xc1e808);              // shr eax, 8 ; pn>>8
          put2(0x89e9);                // mov ecx, ebp ; y
          put3(0xc1e10f);              // shl ecx, 15
          put2(0x29e9);                // sub ecx, ebp ; y*32767
          put2(0x29c1);                // sub ecx, eax
          put3(0xc1f902);              // sar ecx, 2
          put2(0x010e);                // add [esi], ecx
        }

        // ISSE: update weights. edx=cxt=bit history (0..255), esi=cm[512]
        else {
          put2a(0x8b87, off(p[i]));    // mov eax, [edi+&p[i]]
          put1a(0x05, 2048);           // add eax, 2048
          put4a(0x0fb78447, off(squasht)); // movzx eax, word [edi+eax*2+..]
          put2(0x89e9);                // mov ecx, ebp ; y
          put3(0xc1e10f);              // shl ecx, 15
          put2(0x29e9);                // sub ecx, ebp ; y*32767
          put2(0x29c1);                // sub ecx, eax ; err
          put2a(0x8b87, off(p[cp[2]]));// mov eax, [edi+&p[j]]
          put3(0x0fafc1);              // imul eax, ecx
          put1a(0x05, (1<<12));        // add eax, 4096
          put3(0xc1f80d);              // sar eax, 13
          put3(0x0304d6);              // add eax, [esi+edx*8] ; wt[0]
          put1a(0x3d, (1<<19)-1);      // cmp eax, (1<<19)-1
          put2(0x7e05);                // jle L1
          put1a(0xb8, (1<<19)-1);      // mov eax, (1<<19)-1
          put1a(0x3d, 0xfff80000);     // cmp eax, -1<<19
          put2(0x7d05);                // jge L2
          put1a(0xb8, 0xfff80000);     // mov eax, -1<<19
          put3(0x8904d6);              // L2: mov [esi+edx*8], eax
          put3(0x83c110);              // add ecx, 16 ; err
          put3(0xc1f905);              // sar ecx, 5
          put4(0x034cd604);            // add ecx, [esi+edx*8+4] ; wt[1]
          put2a(0x81f9, (1<<19)-1);    // cmp ecx, (1<<19)-1
          put2(0x7e05);                // jle L3
          put1a(0xb9, (1<<19)-1);      // mov ecx, (1<<19)-1
          put2a(0x81f9, 0xfff80000);   // cmp ecx, -1<<19
          put2(0x7d05);                // jge L4
          put1a(0xb9, 0xfff80000);     // mov ecx, -1<<19
          put4(0x894cd604);            // L4: mov [esi+edx*8+4], ecx
        }
        break;

      case MATCH: // sizebits bufbits:
                  //   a=len, b=offset, c=bit, cm=index, cxt=bitpos
                  //   ht=buf, limit=pos
        // assert(cr.a<=255);
        // assert(cr.c==0 || cr.c==1);
        // assert(cr.cxt<8);
        // assert(cr.cm.size()==(size_t(1)<<cp[1]));
        // assert(cr.ht.size()==(size_t(1)<<cp[2]));
        // if (int(cr.c)!=y) cr.a=0;  // mismatch?
        // cr.ht(cr.limit)+=cr.ht(cr.limit)+y;
        // if (++cr.cxt==8) {
        //   cr.cxt=0;
        //   ++cr.limit;
        //   cr.limit&=(1<<cp[2])-1;
        //   if (cr.a==0) {  // look for a match
        //     cr.b=cr.limit-cr.cm(h[i]);
        //     if (cr.b&(cr.ht.size()-1))
        //       while (cr.a<255
        //              && cr.ht(cr.limit-cr.a-1)==cr.ht(cr.limit-cr.a-cr.b-1))
        //         ++cr.a;
        //   }
        //   else cr.a+=cr.a<255;
        //   cr.cm(h[i])=cr.limit;
        // }

        // Set pointers ebx=&cm, esi=&ht
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(ht));       // mov esi, [edi+&ht]
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8b9f, offc(cm));       // mov ebx, [edi+&cm]

        // if (c!=y) a=0;
        put2a(0x8b87, offc(c));        // mov eax, [edi+&c]
        put2(0x39e8);                  // cmp eax, ebp ; y
        put2(0x7408);                  // jz L1
        put2(0x31c0);                  // xor eax, eax
        put2a(0x8987, offc(a));        // mov [edi+&a], eax

        // ht(limit)+=ht(limit)+y  (1E)
        put2a(0x8b87, offc(limit));    // mov eax, [edi+&limit]
        put4(0x0fb60c06);              // movzx, ecx, byte [esi+eax]
        put2(0x01c9);                  // add ecx, ecx
        put2(0x01e9);                  // add ecx, ebp
        put3(0x880c06);                // mov [esi+eax], cl

        // if (++cxt==8)
        put2a(0x8b87, offc(cxt));      // mov eax, [edi+&cxt]
        put2(0xffc0);                  // inc eax
        put3(0x83e007);                // and eax,byte +0x7
        put2a(0x8987, offc(cxt));      // mov [edi+&cxt],eax
        put2a(0x0f85, 0x9b);           // jnz L8

        // ++limit;
        // limit&=bufsize-1;
        put2a(0x8b87, offc(limit));    // mov eax,[edi+&limit]
        put2(0xffc0);                  // inc eax
        put1a(0x25, (1<<cp[2])-1);     // and eax, bufsize-1
        put2a(0x8987, offc(limit));    // mov [edi+&limit],eax

        // if (a==0)
        put2a(0x8b87, offc(a));        // mov eax, [edi+&a]
        put2(0x85c0);                  // test eax,eax
        put2(0x755c);                  // jnz L6

        //   b=limit-cm(h[i])
        put2a(0x8b8f, off(h[i]));      // mov ecx,[edi+h[i]]
        put2a(0x81e1, (1<<cp[1])-1);   // and ecx, size-1
        put2a(0x8b87, offc(limit));    // mov eax,[edi-&limit]
        put3(0x2b048b);                // sub eax,[ebx+ecx*4]
        put2a(0x8987, offc(b));        // mov [edi+&b],eax

        //   if (b&(bufsize-1))
        put1a(0xa9, (1<<cp[2])-1);     // test eax, bufsize-1
        put2(0x7448);                  // jz L7

        //      while (a<255 && ht(limit-a-1)==ht(limit-a-b-1)) ++a;
        put1(0x53);                    // push ebx
        put2a(0x8b9f, offc(limit));    // mov ebx,[edi+&limit]
        put2(0x89da);                  // mov edx,ebx
        put2(0x29c3);                  // sub ebx,eax  ; limit-b
        put2(0x31c9);                  // xor ecx,ecx  ; a=0
        put2a(0x81f9, 0xff);           // L2: cmp ecx,0xff ; while
        put2(0x741c);                  // jz L3 ; break
        put2(0xffca);                  // dec edx
        put2(0xffcb);                  // dec ebx
        put2a(0x81e2, (1<<cp[2])-1);   // and edx, bufsize-1
        put2a(0x81e3, (1<<cp[2])-1);   // and ebx, bufsize-1
        put3(0x8a0416);                // mov al,[esi+edx]
        put3(0x3a041e);                // cmp al,[esi+ebx]
        put2(0x7504);                  // jnz L3 ; break
        put2(0xffc1);                  // inc ecx
        put2(0xebdc);                  // jmp short L2 ; end while
        put1(0x5b);                    // L3: pop ebx
        put2a(0x898f, offc(a));        // mov [edi+&a],ecx
        put2(0xeb0e);                  // jmp short L7

        // a+=(a<255)
        put1a(0x3d, 0xff);             // L6: cmp eax, 0xff ; a
        put3(0x83d000);                // adc eax, 0
        put2a(0x8987, offc(a));        // mov [edi+&a],eax

        // cm(h[i])=limit
        put2a(0x8b87, off(h[i]));      // L7: mov eax,[edi+&h[i]]
        put1a(0x25, (1<<cp[1])-1);     // and eax, size-1
        put2a(0x8b8f, offc(limit));    // mov ecx,[edi+&limit]
        put3(0x890c83);                // mov [ebx+eax*4],ecx
                                       // L8:
        break;

      case AVG:  // j k wt
        break;

      case MIX2: // sizebits j k rate mask
                 // cm=wt[size], cxt=input
        // assert(cr.a16.size()==cr.c);
        // assert(cr.cxt<cr.a16.size());
        // int err=(y*32767-squash(p[i]))*cp[4]>>5;
        // int w=cr.a16[cr.cxt];
        // w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;
        // if (w<0) w=0;
        // if (w>65535) w=65535;
        // cr.a16[cr.cxt]=w;

        // set ecx=err
        put2a(0x8b87, off(p[i]));      // mov eax, [edi+&p[i]]
        put1a(0x05, 2048);             // add eax, 2048
        put4a(0x0fb78447, off(squasht));//movzx eax, word [edi+eax*2+&squasht]
        put2(0x89e9);                  // mov ecx, ebp ; y
        put3(0xc1e10f);                // shl ecx, 15
        put2(0x29e9);                  // sub ecx, ebp ; y*32767
        put2(0x29c1);                  // sub ecx, eax
        put2a(0x69c9, cp[4]);          // imul ecx, rate
        put3(0xc1f905);                // sar ecx, 5  ; err

        // Update w
        put2a(0x8b87, offc(cxt));      // mov eax, [edi+&cxt]
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(a16));      // mov esi, [edi+&a16]
        if (S==8) put1(0x48);          // rex.w
        put3(0x8d3446);                // lea esi, [esi+eax*2] ; &w
        put2a(0x8b87, off(p[cp[2]]));  // mov eax, [edi+&p[j]]
        put2a(0x2b87, off(p[cp[3]]));  // sub eax, [edi+&p[k]] ; p[j]-p[k]
        put3(0x0fafc1);                // imul eax, ecx  ; * err
        put1a(0x05, 1<<12);            // add eax, 4096
        put3(0xc1f80d);                // sar eax, 13
        put3(0x0fb716);                // movzx edx, word [esi] ; w
        put2(0x01d0);                  // add eax, edx
        put1a(0xba, 0xffff);           // mov edx, 65535
        put2(0x39d0);                  // cmp eax, edx
        put3(0x0f4fc2);                // cmovg eax, edx
        put2(0x31d2);                  // xor edx, edx
        put2(0x39d0);                  // cmp eax, edx
        put3(0x0f4cc2);                // cmovl eax, edx
        put3(0x668906);                // mov word [esi], ax
        break;

      case MIX: // sizebits j m rate mask
                // cm=wt[size][m], cxt=input
        // int m=cp[3];
        // assert(m>0 && m<=i);
        // assert(cr.cm.size()==m*cr.c);
        // assert(cr.cxt+m<=cr.cm.size());
        // int err=(y*32767-squash(p[i]))*cp[4]>>4;
        // int* wt=(int*)&cr.cm[cr.cxt];
        // for (int j=0; j<m; ++j)
        //   wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));

        // set ecx=err
        put2a(0x8b87, off(p[i]));      // mov eax, [edi+&p[i]]
        put1a(0x05, 2048);             // add eax, 2048
        put4a(0x0fb78447, off(squasht));//movzx eax, word [edi+eax*2+&squasht]
        put2(0x89e9);                  // mov ecx, ebp ; y
        put3(0xc1e10f);                // shl ecx, 15
        put2(0x29e9);                  // sub ecx, ebp ; y*32767
        put2(0x29c1);                  // sub ecx, eax
        put2a(0x69c9, cp[4]);          // imul ecx, rate
        put3(0xc1f904);                // sar ecx, 4  ; err

        // set esi=wt
        put2a(0x8b87, offc(cxt));      // mov eax, [edi+&cxt] ; cxt
        if (S==8) put1(0x48);          // rex.w
        put2a(0x8bb7, offc(cm));       // mov esi, [edi+&cm]
        if (S==8) put1(0x48);          // rex.w
        put3(0x8d3486);                // lea esi, [esi+eax*4] ; wt

        for (int k=0; k<cp[3]; ++k) {
          put2a(0x8b87,off(p[cp[2]+k]));//mov eax, [edi+&p[cp[2]+k]
          put3(0x0fafc1);              // imul eax, ecx
          put1a(0x05, 1<<12);          // add eax, 1<<12
          put3(0xc1f80d);              // sar eax, 13
          put2(0x0306);                // add eax, [esi]
          put1a(0x3d, (1<<19)-1);      // cmp eax, (1<<19)-1
          put2(0x7e05);                // jge L1
          put1a(0xb8, (1<<19)-1);      // mov eax, (1<<19)-1
          put1a(0x3d, 0xfff80000);     // cmp eax, -1<<19
          put2(0x7d05);                // jle L2
          put1a(0xb8, 0xfff80000);     // mov eax, -1<<19
          put2(0x8906);                // L2: mov [esi], eax
          if (k<cp[3]-1) {
            if (S==8) put1(0x48);      // rex.w
            put3(0x83c604);            // add esi, 4
          }
        }
        break;

      default:
        error("invalid ZPAQ component");
    }
  }

  // return from update()
  put1(0x5f);                 // pop edi
  put1(0x5e);                 // pop esi
  put1(0x5d);                 // pop ebp
  put1(0x5b);                 // pop ebx
  put1(0xc3);                 // ret

  return o;
}

#endif // ifndef NOJIT

// Return a prediction of the next bit in range 0..32767
// Use JIT code starting at pcode[0] if available, or else create it.
int Predictor::predict() {
#ifdef NOJIT
  return predict0();
#else
  if (!pcode) {
    allocx(pcode, pcode_size, (z.cend*100+4096)&-4096);
    int n=assemble_p();
    if (n>pcode_size) {
      allocx(pcode, pcode_size, n);
      n=assemble_p();
    }
    if (!pcode || n<15 || pcode_size<15)
      error("run JIT failed");
  }
  assert(pcode && pcode[0]);
  return ((int(*)(Predictor*))&pcode[10])(this);
#endif
}

// Update the model with bit y = 0..1
// Use the JIT code starting at pcode[5].
void Predictor::update(int y) {
#ifdef NOJIT
  update0(y);
#else
  assert(pcode && pcode[5]);
  ((void(*)(Predictor*, int))&pcode[5])(this, y);

  // Save bit y in c8, hmap4 (not implemented in JIT)
  c8+=c8+y;
  if (c8>=256) {
    z.run(c8-256);
    hmap4=1;
    c8=1;
    for (int i=0; i<z.header[6]; ++i) h[i]=z.H(i);
  }
  else if (c8>=16 && c8<32)
    hmap4=(hmap4&0xf)<<5|y<<4|1;
  else
    hmap4=(hmap4&0x1f0)|(((hmap4&0xf)*2+y)&0xf);
#endif
}

// Execute the ZPAQL code with input byte or -1 for EOF.
// Use JIT code at rcode if available, or else create it.
void ZPAQL::run(U32 input) {
#ifdef NOJIT
  run0(input);
#else
  if (!rcode) {
    allocx(rcode, rcode_size, (hend*10+4096)&-4096);
    int n=assemble();
    if (n>rcode_size) {
      allocx(rcode, rcode_size, n);
      n=assemble();
    }
    if (!rcode || n<10 || rcode_size<10)
      error("run JIT failed");
  }
  a=input;
  const U32 rc=((int(*)())(&rcode[0]))();
  if (rc==0) return;
  else if (rc==1) libzpaq::error("Bad ZPAQL opcode");
  else if (rc==2) libzpaq::error("Out of memory");
  else if (rc==3) libzpaq::error("Write error");
  else libzpaq::error("ZPAQL execution error");
#endif
}

////////////////////////// divsufsort ///////////////////////////////

/*
 * divsufsort.c for libdivsufsort-lite
 * Copyright (c) 2003-2008 Yuta Mori All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*- Constants -*/
#define INLINE __inline
#if defined(ALPHABET_SIZE) && (ALPHABET_SIZE < 1)
# undef ALPHABET_SIZE
#endif
#if !defined(ALPHABET_SIZE)
# define ALPHABET_SIZE (256)
#endif
#define BUCKET_A_SIZE (ALPHABET_SIZE)
#define BUCKET_B_SIZE (ALPHABET_SIZE * ALPHABET_SIZE)
#if defined(SS_INSERTIONSORT_THRESHOLD)
# if SS_INSERTIONSORT_THRESHOLD < 1
#  undef SS_INSERTIONSORT_THRESHOLD
#  define SS_INSERTIONSORT_THRESHOLD (1)
# endif
#else
# define SS_INSERTIONSORT_THRESHOLD (8)
#endif
#if defined(SS_BLOCKSIZE)
# if SS_BLOCKSIZE < 0
#  undef SS_BLOCKSIZE
#  define SS_BLOCKSIZE (0)
# elif 32768 <= SS_BLOCKSIZE
#  undef SS_BLOCKSIZE
#  define SS_BLOCKSIZE (32767)
# endif
#else
# define SS_BLOCKSIZE (1024)
#endif
/* minstacksize = log(SS_BLOCKSIZE) / log(3) * 2 */
#if SS_BLOCKSIZE == 0
# define SS_MISORT_STACKSIZE (96)
#elif SS_BLOCKSIZE <= 4096
# define SS_MISORT_STACKSIZE (16)
#else
# define SS_MISORT_STACKSIZE (24)
#endif
#define SS_SMERGE_STACKSIZE (32)
#define TR_INSERTIONSORT_THRESHOLD (8)
#define TR_STACKSIZE (64)


/*- Macros -*/
#ifndef SWAP
# define SWAP(_a, _b) do { t = (_a); (_a) = (_b); (_b) = t; } while(0)
#endif /* SWAP */
#ifndef MIN
# define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
#endif /* MIN */
#ifndef MAX
# define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
#endif /* MAX */
#define STACK_PUSH(_a, _b, _c, _d)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize++].d = (_d);\
  } while(0)
#define STACK_PUSH5(_a, _b, _c, _d, _e)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize].d = (_d), stack[ssize++].e = (_e);\
  } while(0)
#define STACK_POP(_a, _b, _c, _d)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d;\
  } while(0)
#define STACK_POP5(_a, _b, _c, _d, _e)\
  do {\
    assert(0 <= ssize);\
    if(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d, (_e) = stack[ssize].e;\
  } while(0)
#define BUCKET_A(_c0) bucket_A[(_c0)]
#if ALPHABET_SIZE == 256
#define BUCKET_B(_c0, _c1) (bucket_B[((_c1) << 8) | (_c0)])
#define BUCKET_BSTAR(_c0, _c1) (bucket_B[((_c0) << 8) | (_c1)])
#else
#define BUCKET_B(_c0, _c1) (bucket_B[(_c1) * ALPHABET_SIZE + (_c0)])
#define BUCKET_BSTAR(_c0, _c1) (bucket_B[(_c0) * ALPHABET_SIZE + (_c1)])
#endif


/*- Private Functions -*/

static const int lg_table[256]= {
 -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

#if (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE)

static INLINE
int
ss_ilg(int n) {
#if SS_BLOCKSIZE == 0
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
#elif SS_BLOCKSIZE < 256
  return lg_table[n];
#else
  return (n & 0xff00) ?
          8 + lg_table[(n >> 8) & 0xff] :
          0 + lg_table[(n >> 0) & 0xff];
#endif
}

#endif /* (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE) */

#if SS_BLOCKSIZE != 0

static const int sqq_table[256] = {
  0,  16,  22,  27,  32,  35,  39,  42,  45,  48,  50,  53,  55,  57,  59,  61,
 64,  65,  67,  69,  71,  73,  75,  76,  78,  80,  81,  83,  84,  86,  87,  89,
 90,  91,  93,  94,  96,  97,  98,  99, 101, 102, 103, 104, 106, 107, 108, 109,
110, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
128, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
143, 144, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155,
156, 157, 158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179, 180,
181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190, 191,
192, 192, 193, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 201,
202, 203, 203, 204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211,
212, 212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 221,
221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230,
230, 231, 231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247,
247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255
};

static INLINE
int
ss_isqrt(int x) {
  int y, e;

  if(x >= (SS_BLOCKSIZE * SS_BLOCKSIZE)) { return SS_BLOCKSIZE; }
  e = (x & 0xffff0000) ?
        ((x & 0xff000000) ?
          24 + lg_table[(x >> 24) & 0xff] :
          16 + lg_table[(x >> 16) & 0xff]) :
        ((x & 0x0000ff00) ?
           8 + lg_table[(x >>  8) & 0xff] :
           0 + lg_table[(x >>  0) & 0xff]);

  if(e >= 16) {
    y = sqq_table[x >> ((e - 6) - (e & 1))] << ((e >> 1) - 7);
    if(e >= 24) { y = (y + 1 + x / y) >> 1; }
    y = (y + 1 + x / y) >> 1;
  } else if(e >= 8) {
    y = (sqq_table[x >> ((e - 6) - (e & 1))] >> (7 - (e >> 1))) + 1;
  } else {
    return sqq_table[x] >> 4;
  }

  return (x < (y * y)) ? y - 1 : y;
}

#endif /* SS_BLOCKSIZE != 0 */


/*---------------------------------------------------------------------------*/

/* Compares two suffixes. */
static INLINE
int
ss_compare(const unsigned char *T,
           const int *p1, const int *p2,
           int depth) {
  const unsigned char *U1, *U2, *U1n, *U2n;

  for(U1 = T + depth + *p1,
      U2 = T + depth + *p2,
      U1n = T + *(p1 + 1) + 2,
      U2n = T + *(p2 + 1) + 2;
      (U1 < U1n) && (U2 < U2n) && (*U1 == *U2);
      ++U1, ++U2) {
  }

  return U1 < U1n ?
        (U2 < U2n ? *U1 - *U2 : 1) :
        (U2 < U2n ? -1 : 0);
}


/*---------------------------------------------------------------------------*/

#if (SS_BLOCKSIZE != 1) && (SS_INSERTIONSORT_THRESHOLD != 1)

/* Insertionsort for small size groups */
static
void
ss_insertionsort(const unsigned char *T, const int *PA,
                 int *first, int *last, int depth) {
  int *i, *j;
  int t;
  int r;

  for(i = last - 2; first <= i; --i) {
    for(t = *i, j = i + 1; 0 < (r = ss_compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < last) && (*j < 0));
      if(last <= j) { break; }
    }
    if(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}

#endif /* (SS_BLOCKSIZE != 1) && (SS_INSERTIONSORT_THRESHOLD != 1) */


/*---------------------------------------------------------------------------*/

#if (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE)

static INLINE
void
ss_fixdown(const unsigned char *Td, const int *PA,
           int *SA, int i, int size) {
  int j, k;
  int v;
  int c, d, e;

  for(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    if(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
ss_heapsort(const unsigned char *Td, const int *PA, int *SA, int size) {
  int i, m;
  int t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(Td[PA[SA[m / 2]]] < Td[PA[SA[m]]]) { SWAP(SA[m], SA[m / 2]); }
  }

  for(i = m / 2 - 1; 0 <= i; --i) { ss_fixdown(Td, PA, SA, i, m); }
  if((size % 2) == 0) { SWAP(SA[0], SA[m]); ss_fixdown(Td, PA, SA, 0, m); }
  for(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    ss_fixdown(Td, PA, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static INLINE
int *
ss_median3(const unsigned char *Td, const int *PA,
           int *v1, int *v2, int *v3) {
  int *t;
  if(Td[PA[*v1]] > Td[PA[*v2]]) { SWAP(v1, v2); }
  if(Td[PA[*v2]] > Td[PA[*v3]]) {
    if(Td[PA[*v1]] > Td[PA[*v3]]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static INLINE
int *
ss_median5(const unsigned char *Td, const int *PA,
           int *v1, int *v2, int *v3, int *v4, int *v5) {
  int *t;
  if(Td[PA[*v2]] > Td[PA[*v3]]) { SWAP(v2, v3); }
  if(Td[PA[*v4]] > Td[PA[*v5]]) { SWAP(v4, v5); }
  if(Td[PA[*v2]] > Td[PA[*v4]]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(Td[PA[*v1]] > Td[PA[*v3]]) { SWAP(v1, v3); }
  if(Td[PA[*v1]] > Td[PA[*v4]]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(Td[PA[*v3]] > Td[PA[*v4]]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static INLINE
int *
ss_pivot(const unsigned char *Td, const int *PA, int *first, int *last) {
  int *middle;
  int t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return ss_median3(Td, PA, first, middle, last - 1);
    } else {
      t >>= 2;
      return ss_median5(Td, PA, first, first + t, middle, last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  first  = ss_median3(Td, PA, first, first + t, first + (t << 1));
  middle = ss_median3(Td, PA, middle - t, middle, middle + t);
  last   = ss_median3(Td, PA, last - 1 - (t << 1), last - 1 - t, last - 1);
  return ss_median3(Td, PA, first, middle, last);
}


/*---------------------------------------------------------------------------*/

/* Binary partition for substrings. */
static INLINE
int *
ss_partition(const int *PA,
                    int *first, int *last, int depth) {
  int *a, *b;
  int t;
  for(a = first - 1, b = last;;) {
    for(; (++a < b) && ((PA[*a] + depth) >= (PA[*a + 1] + 1));) { *a = ~*a; }
    for(; (a < --b) && ((PA[*b] + depth) <  (PA[*b + 1] + 1));) { }
    if(b <= a) { break; }
    t = ~*b;
    *b = *a;
    *a = t;
  }
  if(first < a) { *first = ~*first; }
  return a;
}

/* Multikey introsort for medium size groups. */
static
void
ss_mintrosort(const unsigned char *T, const int *PA,
              int *first, int *last,
              int depth) {
#define STACK_SIZE SS_MISORT_STACKSIZE
  struct { int *a, *b, c; int d; } stack[STACK_SIZE];
  const unsigned char *Td;
  int *a, *b, *c, *d, *e, *f;
  int s, t;
  int ssize;
  int limit;
  int v, x = 0;

  for(ssize = 0, limit = ss_ilg(last - first);;) {

    if((last - first) <= SS_INSERTIONSORT_THRESHOLD) {
#if 1 < SS_INSERTIONSORT_THRESHOLD
      if(1 < (last - first)) { ss_insertionsort(T, PA, first, last, depth); }
#endif
      STACK_POP(first, last, depth, limit);
      continue;
    }

    Td = T + depth;
    if(limit-- == 0) { ss_heapsort(Td, PA, first, last - first); }
    if(limit < 0) {
      for(a = first + 1, v = Td[PA[*first]]; a < last; ++a) {
        if((x = Td[PA[*a]]) != v) {
          if(1 < (a - first)) { break; }
          v = x;
          first = a;
        }
      }
      if(Td[PA[*first] - 1] < v) {
        first = ss_partition(PA, first, a, depth);
      }
      if((a - first) <= (last - a)) {
        if(1 < (a - first)) {
          STACK_PUSH(a, last, depth, -1);
          last = a, depth += 1, limit = ss_ilg(a - first);
        } else {
          first = a, limit = -1;
        }
      } else {
        if(1 < (last - a)) {
          STACK_PUSH(first, a, depth + 1, ss_ilg(a - first));
          first = a, limit = -1;
        } else {
          last = a, depth += 1, limit = ss_ilg(a - first);
        }
      }
      continue;
    }

    /* choose pivot */
    a = ss_pivot(Td, PA, first, last);
    v = Td[PA[*a]];
    SWAP(*first, *a);

    /* partition */
    for(b = first; (++b < last) && ((x = Td[PA[*b]]) == v);) { }
    if(((a = b) < last) && (x < v)) {
      for(; (++b < last) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    for(c = last; (b < --c) && ((x = Td[PA[*c]]) == v);) { }
    if((b < (d = c)) && (x > v)) {
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }
    for(; b < c;) {
      SWAP(*b, *c);
      for(; (++b < c) && ((x = Td[PA[*b]]) <= v);) {
        if(x == v) { SWAP(*b, *a); ++a; }
      }
      for(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        if(x == v) { SWAP(*c, *d); --d; }
      }
    }

    if(a <= d) {
      c = b - 1;

      if((s = a - first) > (t = b - a)) { s = t; }
      for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      if((s = d - c) > (t = last - d - 1)) { s = t; }
      for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = first + (b - a), c = last - (d - c);
      b = (v <= Td[PA[*a] - 1]) ? a : ss_partition(PA, a, c, depth);

      if((a - first) <= (last - c)) {
        if((last - c) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          STACK_PUSH(c, last, depth, limit);
          last = a;
        } else if((a - first) <= (c - b)) {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          last = a;
        } else {
          STACK_PUSH(c, last, depth, limit);
          STACK_PUSH(first, a, depth, limit);
          first = b, last = c, depth += 1, limit = ss_ilg(c - b);
        }
      } else {
        if((a - first) <= (c - b)) {
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          STACK_PUSH(first, a, depth, limit);
          first = c;
        } else if((last - c) <= (c - b)) {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(b, c, depth + 1, ss_ilg(c - b));
          first = c;
        } else {
          STACK_PUSH(first, a, depth, limit);
          STACK_PUSH(c, last, depth, limit);
          first = b, last = c, depth += 1, limit = ss_ilg(c - b);
        }
      }
    } else {
      limit += 1;
      if(Td[PA[*first] - 1] < v) {
        first = ss_partition(PA, first, last, depth);
        limit = ss_ilg(last - first);
      }
      depth += 1;
    }
  }
#undef STACK_SIZE
}

#endif /* (SS_BLOCKSIZE == 0) || (SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE) */


/*---------------------------------------------------------------------------*/

#if SS_BLOCKSIZE != 0

static INLINE
void
ss_blockswap(int *a, int *b, int n) {
  int t;
  for(; 0 < n; --n, ++a, ++b) {
    t = *a, *a = *b, *b = t;
  }
}

static INLINE
void
ss_rotate(int *first, int *middle, int *last) {
  int *a, *b, t;
  int l, r;
  l = middle - first, r = last - middle;
  for(; (0 < l) && (0 < r);) {
    if(l == r) { ss_blockswap(first, middle, l); break; }
    if(l < r) {
      a = last - 1, b = middle - 1;
      t = *a;
      do {
        *a-- = *b, *b-- = *a;
        if(b < first) {
          *a = t;
          last = a;
          if((r -= l + 1) <= l) { break; }
          a -= 1, b = middle - 1;
          t = *a;
        }
      } while(1);
    } else {
      a = first, b = middle;
      t = *a;
      do {
        *a++ = *b, *b++ = *a;
        if(last <= b) {
          *a = t;
          first = a + 1;
          if((l -= r + 1) <= r) { break; }
          a += 1, b = middle;
          t = *a;
        }
      } while(1);
    }
  }
}


/*---------------------------------------------------------------------------*/

static
void
ss_inplacemerge(const unsigned char *T, const int *PA,
                int *first, int *middle, int *last,
                int depth) {
  const int *p;
  int *a, *b;
  int len, half;
  int q, r;
  int x;

  for(;;) {
    if(*(last - 1) < 0) { x = 1; p = PA + ~*(last - 1); }
    else                { x = 0; p = PA +  *(last - 1); }
    for(a = first, len = middle - first, half = len >> 1, r = -1;
        0 < len;
        len = half, half >>= 1) {
      b = a + half;
      q = ss_compare(T, PA + ((0 <= *b) ? *b : ~*b), p, depth);
      if(q < 0) {
        a = b + 1;
        half -= (len & 1) ^ 1;
      } else {
        r = q;
      }
    }
    if(a < middle) {
      if(r == 0) { *a = ~*a; }
      ss_rotate(a, middle, last);
      last -= middle - a;
      middle = a;
      if(first == middle) { break; }
    }
    --last;
    if(x != 0) { while(*--last < 0) { } }
    if(middle == last) { break; }
  }
}


/*---------------------------------------------------------------------------*/

/* Merge-forward with internal buffer. */
static
void
ss_mergeforward(const unsigned char *T, const int *PA,
                int *first, int *middle, int *last,
                int *buf, int depth) {
  int *a, *b, *c, *bufend;
  int t;
  int r;

  bufend = buf + (middle - first) - 1;
  ss_blockswap(buf, first, middle - first);

  for(t = *(a = first), b = buf, c = middle;;) {
    r = ss_compare(T, PA + *b, PA + *c, depth);
    if(r < 0) {
      do {
        *a++ = *b;
        if(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);
    } else if(r > 0) {
      do {
        *a++ = *c, *c++ = *a;
        if(last <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    } else {
      *c = ~*c;
      do {
        *a++ = *b;
        if(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);

      do {
        *a++ = *c, *c++ = *a;
        if(last <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    }
  }
}

/* Merge-backward with internal buffer. */
static
void
ss_mergebackward(const unsigned char *T, const int *PA,
                 int *first, int *middle, int *last,
                 int *buf, int depth) {
  const int *p1, *p2;
  int *a, *b, *c, *bufend;
  int t;
  int r;
  int x;

  bufend = buf + (last - middle) - 1;
  ss_blockswap(buf, middle, last - middle);

  x = 0;
  if(*bufend < 0)       { p1 = PA + ~*bufend; x |= 1; }
  else                  { p1 = PA +  *bufend; }
  if(*(middle - 1) < 0) { p2 = PA + ~*(middle - 1); x |= 2; }
  else                  { p2 = PA +  *(middle - 1); }
  for(t = *(a = last - 1), b = bufend, c = middle - 1;;) {
    r = ss_compare(T, p1, p2, depth);
    if(0 < r) {
      if(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = *b;
      if(b <= buf) { *buf = t; break; }
      *b-- = *a;
      if(*b < 0) { p1 = PA + ~*b; x |= 1; }
      else       { p1 = PA +  *b; }
    } else if(r < 0) {
      if(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      if(c < first) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      if(*c < 0) { p2 = PA + ~*c; x |= 2; }
      else       { p2 = PA +  *c; }
    } else {
      if(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = ~*b;
      if(b <= buf) { *buf = t; break; }
      *b-- = *a;
      if(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      if(c < first) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      if(*b < 0) { p1 = PA + ~*b; x |= 1; }
      else       { p1 = PA +  *b; }
      if(*c < 0) { p2 = PA + ~*c; x |= 2; }
      else       { p2 = PA +  *c; }
    }
  }
}

/* D&C based merge. */
static
void
ss_swapmerge(const unsigned char *T, const int *PA,
             int *first, int *middle, int *last,
             int *buf, int bufsize, int depth) {
#define STACK_SIZE SS_SMERGE_STACKSIZE
#define GETIDX(a) ((0 <= (a)) ? (a) : (~(a)))
#define MERGE_CHECK(a, b, c)\
  do {\
    if(((c) & 1) ||\
       (((c) & 2) && (ss_compare(T, PA + GETIDX(*((a) - 1)), PA + *(a), depth) == 0))) {\
      *(a) = ~*(a);\
    }\
    if(((c) & 4) && ((ss_compare(T, PA + GETIDX(*((b) - 1)), PA + *(b), depth) == 0))) {\
      *(b) = ~*(b);\
    }\
  } while(0)
  struct { int *a, *b, *c; int d; } stack[STACK_SIZE];
  int *l, *r, *lm, *rm;
  int m, len, half;
  int ssize;
  int check, next;

  for(check = 0, ssize = 0;;) {
    if((last - middle) <= bufsize) {
      if((first < middle) && (middle < last)) {
        ss_mergebackward(T, PA, first, middle, last, buf, depth);
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
      continue;
    }

    if((middle - first) <= bufsize) {
      if(first < middle) {
        ss_mergeforward(T, PA, first, middle, last, buf, depth);
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
      continue;
    }

    for(m = 0, len = MIN(middle - first, last - middle), half = len >> 1;
        0 < len;
        len = half, half >>= 1) {
      if(ss_compare(T, PA + GETIDX(*(middle + m + half)),
                       PA + GETIDX(*(middle - m - half - 1)), depth) < 0) {
        m += half + 1;
        half -= (len & 1) ^ 1;
      }
    }

    if(0 < m) {
      lm = middle - m, rm = middle + m;
      ss_blockswap(lm, middle, m);
      l = r = middle, next = 0;
      if(rm < last) {
        if(*rm < 0) {
          *rm = ~*rm;
          if(first < lm) { for(; *--l < 0;) { } next |= 4; }
          next |= 1;
        } else if(first < lm) {
          for(; *r < 0; ++r) { }
          next |= 2;
        }
      }

      if((l - first) <= (last - r)) {
        STACK_PUSH(r, rm, last, (next & 3) | (check & 4));
        middle = lm, last = l, check = (check & 3) | (next & 4);
      } else {
        if((next & 2) && (r == middle)) { next ^= 6; }
        STACK_PUSH(first, lm, l, (check & 3) | (next & 4));
        first = r, middle = rm, check = (next & 3) | (check & 4);
      }
    } else {
      if(ss_compare(T, PA + GETIDX(*(middle - 1)), PA + *middle, depth) == 0) {
        *middle = ~*middle;
      }
      MERGE_CHECK(first, last, check);
      STACK_POP(first, middle, last, check);
    }
  }
#undef STACK_SIZE
}

#endif /* SS_BLOCKSIZE != 0 */


/*---------------------------------------------------------------------------*/

/* Substring sort */
static
void
sssort(const unsigned char *T, const int *PA,
       int *first, int *last,
       int *buf, int bufsize,
       int depth, int n, int lastsuffix) {
  int *a;
#if SS_BLOCKSIZE != 0
  int *b, *middle, *curbuf;
  int j, k, curbufsize, limit;
#endif
  int i;

  if(lastsuffix != 0) { ++first; }

#if SS_BLOCKSIZE == 0
  ss_mintrosort(T, PA, first, last, depth);
#else
  if((bufsize < SS_BLOCKSIZE) &&
      (bufsize < (last - first)) &&
      (bufsize < (limit = ss_isqrt(last - first)))) {
    if(SS_BLOCKSIZE < limit) { limit = SS_BLOCKSIZE; }
    buf = middle = last - limit, bufsize = limit;
  } else {
    middle = last, limit = 0;
  }
  for(a = first, i = 0; SS_BLOCKSIZE < (middle - a); a += SS_BLOCKSIZE, ++i) {
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
    ss_mintrosort(T, PA, a, a + SS_BLOCKSIZE, depth);
#elif 1 < SS_BLOCKSIZE
    ss_insertionsort(T, PA, a, a + SS_BLOCKSIZE, depth);
#endif
    curbufsize = last - (a + SS_BLOCKSIZE);
    curbuf = a + SS_BLOCKSIZE;
    if(curbufsize <= bufsize) { curbufsize = bufsize, curbuf = buf; }
    for(b = a, k = SS_BLOCKSIZE, j = i; j & 1; b -= k, k <<= 1, j >>= 1) {
      ss_swapmerge(T, PA, b - k, b, b + k, curbuf, curbufsize, depth);
    }
  }
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
  ss_mintrosort(T, PA, a, middle, depth);
#elif 1 < SS_BLOCKSIZE
  ss_insertionsort(T, PA, a, middle, depth);
#endif
  for(k = SS_BLOCKSIZE; i != 0; k <<= 1, i >>= 1) {
    if(i & 1) {
      ss_swapmerge(T, PA, a - k, a, middle, buf, bufsize, depth);
      a -= k;
    }
  }
  if(limit != 0) {
#if SS_INSERTIONSORT_THRESHOLD < SS_BLOCKSIZE
    ss_mintrosort(T, PA, middle, last, depth);
#elif 1 < SS_BLOCKSIZE
    ss_insertionsort(T, PA, middle, last, depth);
#endif
    ss_inplacemerge(T, PA, first, middle, last, depth);
  }
#endif

  if(lastsuffix != 0) {
    /* Insert last type B* suffix. */
    int PAi[2]; PAi[0] = PA[*(first - 1)], PAi[1] = n - 2;
    for(a = first, i = *(first - 1);
        (a < last) && ((*a < 0) || (0 < ss_compare(T, &(PAi[0]), PA + *a, depth)));
        ++a) {
      *(a - 1) = *a;
    }
    *(a - 1) = i;
  }
}


/*---------------------------------------------------------------------------*/

static INLINE
int
tr_ilg(int n) {
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
}


/*---------------------------------------------------------------------------*/

/* Simple insertionsort for small size groups. */
static
void
tr_insertionsort(const int *ISAd, int *first, int *last) {
  int *a, *b;
  int t, r;

  for(a = first + 1; a < last; ++a) {
    for(t = *a, b = a - 1; 0 > (r = ISAd[t] - ISAd[*b]);) {
      do { *(b + 1) = *b; } while((first <= --b) && (*b < 0));
      if(b < first) { break; }
    }
    if(r == 0) { *b = ~*b; }
    *(b + 1) = t;
  }
}


/*---------------------------------------------------------------------------*/

static INLINE
void
tr_fixdown(const int *ISAd, int *SA, int i, int size) {
  int j, k;
  int v;
  int c, d, e;

  for(v = SA[i], c = ISAd[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = ISAd[SA[k = j++]];
    if(d < (e = ISAd[SA[j]])) { k = j; d = e; }
    if(d <= c) { break; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
void
tr_heapsort(const int *ISAd, int *SA, int size) {
  int i, m;
  int t;

  m = size;
  if((size % 2) == 0) {
    m--;
    if(ISAd[SA[m / 2]] < ISAd[SA[m]]) { SWAP(SA[m], SA[m / 2]); }
  }

  for(i = m / 2 - 1; 0 <= i; --i) { tr_fixdown(ISAd, SA, i, m); }
  if((size % 2) == 0) { SWAP(SA[0], SA[m]); tr_fixdown(ISAd, SA, 0, m); }
  for(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    tr_fixdown(ISAd, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
static INLINE
int *
tr_median3(const int *ISAd, int *v1, int *v2, int *v3) {
  int *t;
  if(ISAd[*v1] > ISAd[*v2]) { SWAP(v1, v2); }
  if(ISAd[*v2] > ISAd[*v3]) {
    if(ISAd[*v1] > ISAd[*v3]) { return v1; }
    else { return v3; }
  }
  return v2;
}

/* Returns the median of five elements. */
static INLINE
int *
tr_median5(const int *ISAd,
           int *v1, int *v2, int *v3, int *v4, int *v5) {
  int *t;
  if(ISAd[*v2] > ISAd[*v3]) { SWAP(v2, v3); }
  if(ISAd[*v4] > ISAd[*v5]) { SWAP(v4, v5); }
  if(ISAd[*v2] > ISAd[*v4]) { SWAP(v2, v4); SWAP(v3, v5); }
  if(ISAd[*v1] > ISAd[*v3]) { SWAP(v1, v3); }
  if(ISAd[*v1] > ISAd[*v4]) { SWAP(v1, v4); SWAP(v3, v5); }
  if(ISAd[*v3] > ISAd[*v4]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
static INLINE
int *
tr_pivot(const int *ISAd, int *first, int *last) {
  int *middle;
  int t;

  t = last - first;
  middle = first + t / 2;

  if(t <= 512) {
    if(t <= 32) {
      return tr_median3(ISAd, first, middle, last - 1);
    } else {
      t >>= 2;
      return tr_median5(ISAd, first, first + t, middle, last - 1 - t, last - 1);
    }
  }
  t >>= 3;
  first  = tr_median3(ISAd, first, first + t, first + (t << 1));
  middle = tr_median3(ISAd, middle - t, middle, middle + t);
  last   = tr_median3(ISAd, last - 1 - (t << 1), last - 1 - t, last - 1);
  return tr_median3(ISAd, first, middle, last);
}


/*---------------------------------------------------------------------------*/

typedef struct _trbudget_t trbudget_t;
struct _trbudget_t {
  int chance;
  int remain;
  int incval;
  int count;
};

static INLINE
void
trbudget_init(trbudget_t *budget, int chance, int incval) {
  budget->chance = chance;
  budget->remain = budget->incval = incval;
}

static INLINE
int
trbudget_check(trbudget_t *budget, int size) {
  if(size <= budget->remain) { budget->remain -= size; return 1; }
  if(budget->chance == 0) { budget->count += size; return 0; }
  budget->remain += budget->incval - size;
  budget->chance -= 1;
  return 1;
}


/*---------------------------------------------------------------------------*/

static INLINE
void
tr_partition(const int *ISAd,
             int *first, int *middle, int *last,
             int **pa, int **pb, int v) {
  int *a, *b, *c, *d, *e, *f;
  int t, s;
  int x = 0;

  for(b = middle - 1; (++b < last) && ((x = ISAd[*b]) == v);) { }
  if(((a = b) < last) && (x < v)) {
    for(; (++b < last) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
  }
  for(c = last; (b < --c) && ((x = ISAd[*c]) == v);) { }
  if((b < (d = c)) && (x > v)) {
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }
  for(; b < c;) {
    SWAP(*b, *c);
    for(; (++b < c) && ((x = ISAd[*b]) <= v);) {
      if(x == v) { SWAP(*b, *a); ++a; }
    }
    for(; (b < --c) && ((x = ISAd[*c]) >= v);) {
      if(x == v) { SWAP(*c, *d); --d; }
    }
  }

  if(a <= d) {
    c = b - 1;
    if((s = a - first) > (t = b - a)) { s = t; }
    for(e = first, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    if((s = d - c) > (t = last - d - 1)) { s = t; }
    for(e = b, f = last - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    first += (b - a), last -= (d - c);
  }
  *pa = first, *pb = last;
}

static
void
tr_copy(int *ISA, const int *SA,
        int *first, int *a, int *b, int *last,
        int depth) {
  /* sort suffixes of middle partition
     by using sorted order of suffixes of left and right partition. */
  int *c, *d, *e;
  int s, v;

  v = b - SA - 1;
  for(c = first, d = a - 1; c <= d; ++c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      ISA[s] = d - SA;
    }
  }
  for(c = last - 1, e = d + 1, d = b; e < d; --c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      ISA[s] = d - SA;
    }
  }
}

static
void
tr_partialcopy(int *ISA, const int *SA,
               int *first, int *a, int *b, int *last,
               int depth) {
  int *c, *d, *e;
  int s, v;
  int rank, lastrank, newrank = -1;

  v = b - SA - 1;
  lastrank = -1;
  for(c = first, d = a - 1; c <= d; ++c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      rank = ISA[s + depth];
      if(lastrank != rank) { lastrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }

  lastrank = -1;
  for(e = d; first <= e; --e) {
    rank = ISA[*e];
    if(lastrank != rank) { lastrank = rank; newrank = e - SA; }
    if(newrank != rank) { ISA[*e] = newrank; }
  }

  lastrank = -1;
  for(c = last - 1, e = d + 1, d = b; e < d; --c) {
    if((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      rank = ISA[s + depth];
      if(lastrank != rank) { lastrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }
}

static
void
tr_introsort(int *ISA, const int *ISAd,
             int *SA, int *first, int *last,
             trbudget_t *budget) {
#define STACK_SIZE TR_STACKSIZE
  struct { const int *a; int *b, *c; int d, e; }stack[STACK_SIZE];
  int *a, *b, *c;
  int t;
  int v, x = 0;
  int incr = ISAd - ISA;
  int limit, next;
  int ssize, trlink = -1;

  for(ssize = 0, limit = tr_ilg(last - first);;) {

    if(limit < 0) {
      if(limit == -1) {
        /* tandem repeat partition */
        tr_partition(ISAd - incr, first, first, last, &a, &b, last - SA - 1);

        /* update ranks */
        if(a < last) {
          for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
        }
        if(b < last) {
          for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; }
        }

        /* push */
        if(1 < (b - a)) {
          STACK_PUSH5(NULL, a, b, 0, 0);
          STACK_PUSH5(ISAd - incr, first, last, -2, trlink);
          trlink = ssize - 2;
        }
        if((a - first) <= (last - b)) {
          if(1 < (a - first)) {
            STACK_PUSH5(ISAd, b, last, tr_ilg(last - b), trlink);
            last = a, limit = tr_ilg(a - first);
          } else if(1 < (last - b)) {
            first = b, limit = tr_ilg(last - b);
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        } else {
          if(1 < (last - b)) {
            STACK_PUSH5(ISAd, first, a, tr_ilg(a - first), trlink);
            first = b, limit = tr_ilg(last - b);
          } else if(1 < (a - first)) {
            last = a, limit = tr_ilg(a - first);
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        }
      } else if(limit == -2) {
        /* tandem repeat copy */
        a = stack[--ssize].b, b = stack[ssize].c;
        if(stack[ssize].d == 0) {
          tr_copy(ISA, SA, first, a, b, last, ISAd - ISA);
        } else {
          if(0 <= trlink) { stack[trlink].d = -1; }
          tr_partialcopy(ISA, SA, first, a, b, last, ISAd - ISA);
        }
        STACK_POP5(ISAd, first, last, limit, trlink);
      } else {
        /* sorted partition */
        if(0 <= *first) {
          a = first;
          do { ISA[*a] = a - SA; } while((++a < last) && (0 <= *a));
          first = a;
        }
        if(first < last) {
          a = first; do { *a = ~*a; } while(*++a < 0);
          next = (ISA[*a] != ISAd[*a]) ? tr_ilg(a - first + 1) : -1;
          if(++a < last) { for(b = first, v = a - SA - 1; b < a; ++b) { ISA[*b] = v; } }

          /* push */
          if(trbudget_check(budget, a - first)) {
            if((a - first) <= (last - a)) {
              STACK_PUSH5(ISAd, a, last, -3, trlink);
              ISAd += incr, last = a, limit = next;
            } else {
              if(1 < (last - a)) {
                STACK_PUSH5(ISAd + incr, first, a, next, trlink);
                first = a, limit = -3;
              } else {
                ISAd += incr, last = a, limit = next;
              }
            }
          } else {
            if(0 <= trlink) { stack[trlink].d = -1; }
            if(1 < (last - a)) {
              first = a, limit = -3;
            } else {
              STACK_POP5(ISAd, first, last, limit, trlink);
            }
          }
        } else {
          STACK_POP5(ISAd, first, last, limit, trlink);
        }
      }
      continue;
    }

    if((last - first) <= TR_INSERTIONSORT_THRESHOLD) {
      tr_insertionsort(ISAd, first, last);
      limit = -3;
      continue;
    }

    if(limit-- == 0) {
      tr_heapsort(ISAd, first, last - first);
      for(a = last - 1; first < a; a = b) {
        for(x = ISAd[*a], b = a - 1; (first <= b) && (ISAd[*b] == x); --b) { *b = ~*b; }
      }
      limit = -3;
      continue;
    }

    /* choose pivot */
    a = tr_pivot(ISAd, first, last);
    SWAP(*first, *a);
    v = ISAd[*first];

    /* partition */
    tr_partition(ISAd, first, first + 1, last, &a, &b, v);
    if((last - first) != (b - a)) {
      next = (ISA[*a] != v) ? tr_ilg(b - a) : -1;

      /* update ranks */
      for(c = first, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      if(b < last) { for(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }

      /* push */
      if((1 < (b - a)) && (trbudget_check(budget, b - a))) {
        if((a - first) <= (last - b)) {
          if((last - b) <= (b - a)) {
            if(1 < (a - first)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              last = a;
            } else if(1 < (last - b)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              first = b;
            } else {
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else if((a - first) <= (b - a)) {
            if(1 < (a - first)) {
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              last = a;
            } else {
              STACK_PUSH5(ISAd, b, last, limit, trlink);
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else {
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            ISAd += incr, first = a, last = b, limit = next;
          }
        } else {
          if((a - first) <= (b - a)) {
            if(1 < (last - b)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              first = b;
            } else if(1 < (a - first)) {
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              last = a;
            } else {
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else if((last - b) <= (b - a)) {
            if(1 < (last - b)) {
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              STACK_PUSH5(ISAd + incr, a, b, next, trlink);
              first = b;
            } else {
              STACK_PUSH5(ISAd, first, a, limit, trlink);
              ISAd += incr, first = a, last = b, limit = next;
            }
          } else {
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            ISAd += incr, first = a, last = b, limit = next;
          }
        }
      } else {
        if((1 < (b - a)) && (0 <= trlink)) { stack[trlink].d = -1; }
        if((a - first) <= (last - b)) {
          if(1 < (a - first)) {
            STACK_PUSH5(ISAd, b, last, limit, trlink);
            last = a;
          } else if(1 < (last - b)) {
            first = b;
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        } else {
          if(1 < (last - b)) {
            STACK_PUSH5(ISAd, first, a, limit, trlink);
            first = b;
          } else if(1 < (a - first)) {
            last = a;
          } else {
            STACK_POP5(ISAd, first, last, limit, trlink);
          }
        }
      }
    } else {
      if(trbudget_check(budget, last - first)) {
        limit = tr_ilg(last - first), ISAd += incr;
      } else {
        if(0 <= trlink) { stack[trlink].d = -1; }
        STACK_POP5(ISAd, first, last, limit, trlink);
      }
    }
  }
#undef STACK_SIZE
}



/*---------------------------------------------------------------------------*/

/* Tandem repeat sort */
static
void
trsort(int *ISA, int *SA, int n, int depth) {
  int *ISAd;
  int *first, *last;
  trbudget_t budget;
  int t, skip, unsorted;

  trbudget_init(&budget, tr_ilg(n) * 2 / 3, n);
/*  trbudget_init(&budget, tr_ilg(n) * 3 / 4, n); */
  for(ISAd = ISA + depth; -n < *SA; ISAd += ISAd - ISA) {
    first = SA;
    skip = 0;
    unsorted = 0;
    do {
      if((t = *first) < 0) { first -= t; skip += t; }
      else {
        if(skip != 0) { *(first + skip) = skip; skip = 0; }
        last = SA + ISA[t] + 1;
        if(1 < (last - first)) {
          budget.count = 0;
          tr_introsort(ISA, ISAd, SA, first, last, &budget);
          if(budget.count != 0) { unsorted += budget.count; }
          else { skip = first - last; }
        } else if((last - first) == 1) {
          skip = -1;
        }
        first = last;
      }
    } while(first < (SA + n));
    if(skip != 0) { *(first + skip) = skip; }
    if(unsorted == 0) { break; }
  }
}


/*---------------------------------------------------------------------------*/

/* Sorts suffixes of type B*. */
static
int
sort_typeBstar(const unsigned char *T, int *SA,
               int *bucket_A, int *bucket_B,
               int n) {
  int *PAb, *ISAb, *buf;
#ifdef _OPENMP
  int *curbuf;
  int l;
#endif
  int i, j, k, t, m, bufsize;
  int c0, c1;
#ifdef _OPENMP
  int d0, d1;
  int tmp;
#endif

  /* Initialize bucket arrays. */
  for(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  for(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  /* Count the number of occurrences of the first one or two characters of each
     type A, B and B* suffix. Moreover, store the beginning position of all
     type B* suffixes into the array SA. */
  for(i = n - 1, m = n, c0 = T[n - 1]; 0 <= i;) {
    /* type A suffix. */
    do { ++BUCKET_A(c1 = c0); } while((0 <= --i) && ((c0 = T[i]) >= c1));
    if(0 <= i) {
      /* type B* suffix. */
      ++BUCKET_BSTAR(c0, c1);
      SA[--m] = i;
      /* type B suffix. */
      for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) {
        ++BUCKET_B(c0, c1);
      }
    }
  }
  m = n - m;
/*
note:
  A type B* suffix is lexicographically smaller than a type B suffix that
  begins with the same first two characters.
*/

  /* Calculate the index of start/end point of each bucket. */
  for(c0 = 0, i = 0, j = 0; c0 < ALPHABET_SIZE; ++c0) {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; /* start point */
    i = t + BUCKET_B(c0, c0);
    for(c1 = c0 + 1; c1 < ALPHABET_SIZE; ++c1) {
      j += BUCKET_BSTAR(c0, c1);
      BUCKET_BSTAR(c0, c1) = j; /* end point */
      i += BUCKET_B(c0, c1);
    }
  }

  if(0 < m) {
    /* Sort the type B* suffixes by their first two characters. */
    PAb = SA + n - m; ISAb = SA + m;
    for(i = m - 2; 0 <= i; --i) {
      t = PAb[i], c0 = T[t], c1 = T[t + 1];
      SA[--BUCKET_BSTAR(c0, c1)] = i;
    }
    t = PAb[m - 1], c0 = T[t], c1 = T[t + 1];
    SA[--BUCKET_BSTAR(c0, c1)] = m - 1;

    /* Sort the type B* substrings using sssort. */
#ifdef _OPENMP
    tmp = omp_get_max_threads();
    buf = SA + m, bufsize = (n - (2 * m)) / tmp;
    c0 = ALPHABET_SIZE - 2, c1 = ALPHABET_SIZE - 1, j = m;
#pragma omp parallel default(shared) private(curbuf, k, l, d0, d1, tmp)
    {
      tmp = omp_get_thread_num();
      curbuf = buf + tmp * bufsize;
      k = 0;
      for(;;) {
        #pragma omp critical(sssort_lock)
        {
          if(0 < (l = j)) {
            d0 = c0, d1 = c1;
            do {
              k = BUCKET_BSTAR(d0, d1);
              if(--d1 <= d0) {
                d1 = ALPHABET_SIZE - 1;
                if(--d0 < 0) { break; }
              }
            } while(((l - k) <= 1) && (0 < (l = k)));
            c0 = d0, c1 = d1, j = k;
          }
        }
        if(l == 0) { break; }
        sssort(T, PAb, SA + k, SA + l,
               curbuf, bufsize, 2, n, *(SA + k) == (m - 1));
      }
    }
#else
    buf = SA + m, bufsize = n - (2 * m);
    for(c0 = ALPHABET_SIZE - 2, j = m; 0 < j; --c0) {
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; j = i, --c1) {
        i = BUCKET_BSTAR(c0, c1);
        if(1 < (j - i)) {
          sssort(T, PAb, SA + i, SA + j,
                 buf, bufsize, 2, n, *(SA + i) == (m - 1));
        }
      }
    }
#endif

    /* Compute ranks of type B* substrings. */
    for(i = m - 1; 0 <= i; --i) {
      if(0 <= SA[i]) {
        j = i;
        do { ISAb[SA[i]] = i; } while((0 <= --i) && (0 <= SA[i]));
        SA[i + 1] = i - j;
        if(i <= 0) { break; }
      }
      j = i;
      do { ISAb[SA[i] = ~SA[i]] = j; } while(SA[--i] < 0);
      ISAb[SA[i]] = j;
    }

    /* Construct the inverse suffix array of type B* suffixes using trsort. */
    trsort(ISAb, SA, m, 1);

    /* Set the sorted order of tyoe B* suffixes. */
    for(i = n - 1, j = m, c0 = T[n - 1]; 0 <= i;) {
      for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) >= c1); --i, c1 = c0) { }
      if(0 <= i) {
        t = i;
        for(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) { }
        SA[ISAb[--j]] = ((t == 0) || (1 < (t - i))) ? t : ~t;
      }
    }

    /* Calculate the index of start/end point of each bucket. */
    BUCKET_B(ALPHABET_SIZE - 1, ALPHABET_SIZE - 1) = n; /* end point */
    for(c0 = ALPHABET_SIZE - 2, k = m - 1; 0 <= c0; --c0) {
      i = BUCKET_A(c0 + 1) - 1;
      for(c1 = ALPHABET_SIZE - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i; /* end point */

        /* Move all type B* suffixes to the correct position. */
        for(i = t, j = BUCKET_BSTAR(c0, c1);
            j <= k;
            --i, --k) { SA[i] = SA[k]; }
      }
      BUCKET_BSTAR(c0, c0 + 1) = i - BUCKET_B(c0, c0) + 1; /* start point */
      BUCKET_B(c0, c0) = i; /* end point */
    }
  }

  return m;
}

/* Constructs the suffix array by using the sorted order of type B* suffixes. */
static
void
construct_SA(const unsigned char *T, int *SA,
             int *bucket_A, int *bucket_B,
             int n, int m) {
  int *i, *j, *k;
  int s;
  int c0, c1, c2;

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 < (s = *j)) {
          assert(T[s] == c1);
          assert(((s + 1) < n) && (T[s] <= T[s + 1]));
          assert(T[s - 1] <= T[s]);
          *j = ~s;
          c0 = T[--s];
          if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          if(c0 != c2) {
            if(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
          assert(k < j);
          *k-- = s;
        } else {
          assert(((s == 0) && (T[s] == c1)) || (s < 0));
          *j = ~s;
        }
      }
    }
  }

  /* Construct the suffix array by using
     the sorted order of type B suffixes. */
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~(n - 1) : (n - 1);
  /* Scan the suffix array from left to right. */
  for(i = SA, j = SA + n; i < j; ++i) {
    if(0 < (s = *i)) {
      assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      if((s == 0) || (T[s - 1] < c0)) { s = ~s; }
      if(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
      assert(i < k);
      *k++ = s;
    } else {
      assert(s < 0);
      *i = ~s;
    }
  }
}

/* Constructs the burrows-wheeler transformed string directly
   by using the sorted order of type B* suffixes. */
static
int
construct_BWT(const unsigned char *T, int *SA,
              int *bucket_A, int *bucket_B,
              int n, int m) {
  int *i, *j, *k, *orig;
  int s;
  int c0, c1, c2;

  if(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    for(c1 = ALPHABET_SIZE - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      for(i = SA + BUCKET_BSTAR(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        if(0 < (s = *j)) {
          assert(T[s] == c1);
          assert(((s + 1) < n) && (T[s] <= T[s + 1]));
          assert(T[s - 1] <= T[s]);
          c0 = T[--s];
          *j = ~((int)c0);
          if((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          if(c0 != c2) {
            if(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
          assert(k < j);
          *k-- = s;
        } else if(s != 0) {
          *j = ~s;
#ifndef NDEBUG
        } else {
          assert(T[s] == c1);
#endif
        }
      }
    }
  }

  /* Construct the BWTed string by using
     the sorted order of type B suffixes. */
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~((int)T[n - 2]) : (n - 1);
  /* Scan the suffix array from left to right. */
  for(i = SA, j = SA + n, orig = SA; i < j; ++i) {
    if(0 < (s = *i)) {
      assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      *i = c0;
      if((0 < s) && (T[s - 1] < c0)) { s = ~((int)T[s - 1]); }
      if(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
      assert(i < k);
      *k++ = s;
    } else if(s != 0) {
      *i = ~s;
    } else {
      orig = i;
    }
  }

  return orig - SA;
}


/*---------------------------------------------------------------------------*/

/*- Function -*/

int
divsufsort(const unsigned char *T, int *SA, int n) {
  int *bucket_A, *bucket_B;
  int m;
  int err = 0;

  /* Check arguments. */
  if((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  else if(n == 0) { return 0; }
  else if(n == 1) { SA[0] = 0; return 0; }
  else if(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; return 0; }

  bucket_A = (int *)malloc(BUCKET_A_SIZE * sizeof(int));
  bucket_B = (int *)malloc(BUCKET_B_SIZE * sizeof(int));

  /* Suffixsort. */
  if((bucket_A != NULL) && (bucket_B != NULL)) {
    m = sort_typeBstar(T, SA, bucket_A, bucket_B, n);
    construct_SA(T, SA, bucket_A, bucket_B, n, m);
  } else {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);

  return err;
}

int
divbwt(const unsigned char *T, unsigned char *U, int *A, int n) {
  int *B;
  int *bucket_A, *bucket_B;
  int m, pidx, i;

  /* Check arguments. */
  if((T == NULL) || (U == NULL) || (n < 0)) { return -1; }
  else if(n <= 1) { if(n == 1) { U[0] = T[0]; } return n; }

  if((B = A) == NULL) { B = (int *)malloc((size_t)(n + 1) * sizeof(int)); }
  bucket_A = (int *)malloc(BUCKET_A_SIZE * sizeof(int));
  bucket_B = (int *)malloc(BUCKET_B_SIZE * sizeof(int));

  /* Burrows-Wheeler Transform. */
  if((B != NULL) && (bucket_A != NULL) && (bucket_B != NULL)) {
    m = sort_typeBstar(T, B, bucket_A, bucket_B, n);
    pidx = construct_BWT(T, B, bucket_A, bucket_B, n, m);

    /* Copy to output string. */
    U[0] = T[n - 1];
    for(i = 0; i < pidx; ++i) { U[i + 1] = (unsigned char)B[i]; }
    for(i += 1; i < n; ++i) { U[i] = (unsigned char)B[i]; }
    pidx += 1;
  } else {
    pidx = -2;
  }

  free(bucket_B);
  free(bucket_A);
  if(A == NULL) { free(B); }

  return pidx;
}

// End divsufsort.c

/////////////////////////////// add ///////////////////////////////////

// Convert non-negative decimal number x to string of at least n digits
std::string itos(int64_t x, int n=1) {
  assert(x>=0);
  assert(n>=0);
  std::string r;
  for (; x || n>0; x/=10, --n) r=std::string(1, '0'+x%10)+r;
  return r;
}

// E8E9 transform of buf[0..n-1] to improve compression of .exe and .dll.
// Patterns (E8|E9 xx xx xx 00|FF) at offset i replace the 3 middle
// bytes with x+i mod 2^24, LSB first, reading backward.
void e8e9(unsigned char* buf, int n) {
  for (int i=n-5; i>=0; --i) {
    if (((buf[i]&254)==0xe8) && ((buf[i+4]+1)&254)==0) {
      unsigned a=(buf[i+1]|buf[i+2]<<8|buf[i+3]<<16)+i;
      buf[i+1]=a;
      buf[i+2]=a>>8;
      buf[i+3]=a>>16;
    }
  }
}

// Encode inbuf to buf using LZ77. args are as follows:
// args[0] is log2 buffer size in MB.
// args[1] is level (1=var. length, 2=byte aligned lz77, 3=bwt) + 4 if E8E9.
// args[2] is the lz77 minimum match length and context order.
// args[3] is the lz77 higher context order to search first, or else 0.
// args[4] is the log2 hash bucket size (number of searches).
// args[5] is the log2 hash table size. If 21+args[0] then use a suffix array.
// args[6] is the secondary context look ahead
// sap is pointer to external suffix array of inbuf or 0. If supplied and
//   args[0]=5..7 then it is assumed that E8E9 was already applied to
//   both the input and sap and the input buffer is not modified.

class LZBuffer: public libzpaq::Reader {
  libzpaq::Array<unsigned> ht;// hash table, confirm in low bits, or SA+ISA
  const unsigned char* in;    // input pointer
  const int checkbits;        // hash confirmation size or lg(ISA size)
  const int level;            // 1=var length LZ77, 2=byte aligned LZ77, 3=BWT
  const unsigned htsize;      // size of hash table
  const unsigned n;           // input length
  unsigned i;                 // current location in in (0 <= i < n)
  const unsigned minMatch;    // minimum match length
  const unsigned minMatch2;   // second context order or 0 if not used
  const unsigned maxMatch;    // longest match length allowed
  const unsigned maxLiteral;  // longest literal length allowed
  const unsigned lookahead;   // second context look ahead
  unsigned h1, h2;            // low, high order context hashes of in[i..]
  const unsigned bucket;      // number of matches to search per hash - 1
  const unsigned shift1, shift2;  // how far to shift h1, h2 per hash
  const int minMatchBoth;     // max(minMatch, minMatch2)
  const unsigned rb;          // number of level 1 r bits in match code
  unsigned bits;              // pending output bits (level 1)
  unsigned nbits;             // number of bits in bits
  unsigned rpos, wpos;        // read, write pointers
  unsigned idx;               // BWT index
  const unsigned* sa;         // suffix array for BWT or LZ77-SA
  unsigned* isa;              // inverse suffix array for LZ77-SA
  enum {BUFSIZE=1<<14};       // output buffer size
  unsigned char buf[BUFSIZE]; // output buffer

  void write_literal(unsigned i, unsigned& lit);
  void write_match(unsigned len, unsigned off);
  void fill();  // encode to buf

  // write k bits of x
  void putb(unsigned x, int k) {
    x&=(1<<k)-1;
    bits|=x<<nbits;
    nbits+=k;
    while (nbits>7) {
      assert(wpos<BUFSIZE);
      buf[wpos++]=bits, bits>>=8, nbits-=8;
    }
  }

  // write last byte
  void flush() {
    assert(wpos<BUFSIZE);
    if (nbits>0) buf[wpos++]=bits;
    bits=nbits=0;
  }

  // write 1 byte
  void put(int c) {
    assert(wpos<BUFSIZE);
    buf[wpos++]=c;
  }

public:
  LZBuffer(StringBuffer& inbuf, int args[], const unsigned* sap=0);

  // return 1 byte of compressed output (overrides Reader)
  int get() {
    int c=-1;
    if (rpos==wpos) fill();
    if (rpos<wpos) c=buf[rpos++];
    if (rpos==wpos) rpos=wpos=0;
    return c;
  }

  // Read up to p[0..n-1] and return bytes read.
  int read(char* p, int n);
};

// LZ/BWT preprocessor for levels 1..3 compression and e8e9 filter.
// Level 1 uses variable length LZ77 codes like in the lazy compressor:
//
//   00,n,L[n] = n literal bytes
//   mm,mmm,n,ll,r,q (mm > 00) = match 4*n+ll at offset (q<<rb)+r-1
//
// where q is written in 8mm+mmm-8 (0..23) bits with an implied leading 1 bit
// and n is written using interleaved Elias Gamma coding, i.e. the leading
// 1 bit is implied, remaining bits are preceded by a 1 and terminated by
// a 0. e.g. abc is written 1,b,1,c,0. Codes are packed LSB first and
// padded with leading 0 bits in the last byte. r is a number with rb bits,
// where rb = log2(blocksize) - 24.
//
// Level 2 is byte oriented LZ77 with minimum match length m = $4 = args[3]
// with m in 1..64. Lengths and offsets are MSB first:
// 00xxxxxx   x+1 (1..64) literals follow
// yyxxxxxx   y+1 (2..4) offset bytes follow, match length x+m (m..m+63)
//
// Level 3 is BWT with the end of string byte coded as 255 and the
// last 4 bytes giving its position LSB first.

// floor(log2(x)) + 1 = number of bits excluding leading zeros (0..32)
int lg(unsigned x) {
  unsigned r=0;
  if (x>=65536) r=16, x>>=16;
  if (x>=256) r+=8, x>>=8;
  if (x>=16) r+=4, x>>=4;
  assert(x>=0 && x<16);
  return
    "\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;
}

// return number of 1 bits in x
int nbits(unsigned x) {
  int r;
  for (r=0; x; x>>=1) r+=x&1;
  return r;
}

// Read n bytes of compressed output into p and return number of
// bytes read in 0..n. 0 signals EOF (overrides Reader).
int LZBuffer::read(char* p, int n) {
  if (rpos==wpos) fill();
  int nr=n;
  if (nr>int(wpos-rpos)) nr=wpos-rpos;
  if (nr) memcpy(p, buf+rpos, nr);
  rpos+=nr;
  assert(rpos<=wpos);
  if (rpos==wpos) rpos=wpos=0;
  return nr;
}

LZBuffer::LZBuffer(StringBuffer& inbuf, int args[], const unsigned* sap):
    ht((args[1]&3)==3 ? (inbuf.size()+1)*!sap      // for BWT suffix array
        : args[5]-args[0]<21 ? 1u<<args[5]         // for LZ77 hash table
        : (inbuf.size()*!sap)+(1u<<17<<args[0])),  // for LZ77 SA and ISA
    in(inbuf.data()),
    checkbits(args[5]-args[0]<21 ? 12-args[0] : 17+args[0]),
    level(args[1]&3),
    htsize(ht.size()),
    n(inbuf.size()),
    i(0),
    minMatch(args[2]),
    minMatch2(args[3]),
    maxMatch(BUFSIZE*3),
    maxLiteral(BUFSIZE/4),
    lookahead(args[6]),
    h1(0), h2(0),
    bucket((1<<args[4])-1), 
    shift1(minMatch>0 ? (args[5]-1)/minMatch+1 : 1),
    shift2(minMatch2>0 ? (args[5]-1)/minMatch2+1 : 0),
    minMatchBoth(MAX(minMatch, minMatch2+lookahead)+4),
    rb(args[0]>4 ? args[0]-4 : 0),
    bits(0), nbits(0), rpos(0), wpos(0),
    idx(0), sa(0), isa(0) {
  assert(args[0]>=0);
  assert(n<=(1u<<20<<args[0]));
  assert(args[1]>=1 && args[1]<=7 && args[1]!=4);
  assert(level>=1 && level<=3);
  if ((minMatch<4 && level==1) || (minMatch<1 && level==2))
    error("match length $3 too small");

  // e8e9 transform
  if (args[1]>4 && !sap) e8e9(inbuf.data(), n);

  // build suffix array if not supplied
  if (args[5]-args[0]>=21 || level==3) {  // LZ77-SA or BWT
    if (sap)
      sa=sap;
    else {
      assert(ht.size()>=n);
      assert(ht.size()>0);
      sa=&ht[0];
      if (n>0) divsufsort((const unsigned char*)in, (int*)sa, n);
    }
    if (level<3) {
      assert(ht.size()>=(n*(sap==0))+(1u<<17<<args[0]));
      isa=&ht[n*(sap==0)];
    }
  }
}

// Encode from in to buf until end of input or buf is not empty
void LZBuffer::fill() {

  // BWT
  if (level==3) {
    assert(in || n==0);
    assert(sa);
    for (; wpos<BUFSIZE && i<n+5; ++i) {
      if (i==0) put(n>0 ? in[n-1] : 255);
      else if (i>n) put(idx&255), idx>>=8;
      else if (sa[i-1]==0) idx=i, put(255);
      else put(in[sa[i-1]-1]);
    }
    return;
  }

  // LZ77: scan the input
  unsigned lit=0;  // number of output literals pending
  const unsigned mask=(1<<checkbits)-1;
  while (i<n && wpos*2<BUFSIZE) {

    // Search for longest match, or pick closest in case of tie
    unsigned blen=minMatch-1;  // best match length
    unsigned bp=0;  // pointer to best match
    unsigned blit=0;  // literals before best match
    int bscore=0;  // best cost

    // Look up contexts in suffix array
    if (isa) {
      if (sa[isa[i&mask]]!=i) // rebuild ISA
        for (unsigned j=0; j<n; ++j)
          if ((sa[j]&~mask)==(i&~mask))
            isa[sa[j]&mask]=j;
      for (unsigned h=0; h<=lookahead; ++h) {
        unsigned q=isa[(h+i)&mask];  // location of h+i in SA
        assert(q<n);
        if (sa[q]!=h+i) continue;
        for (int j=-1; j<=1; j+=2) {  // search backward and forward
          for (unsigned k=1; k<=bucket; ++k) {
            unsigned p;  // match to be tested
            if (q+j*k<n && (p=sa[q+j*k]-h)<i) {
              assert(p<n);
              unsigned l, l1;  // length of match, leading literals
              for (l=h; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              for (l1=h; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
              int score=int(l-l1)*8-lg(i-p)-4*(lit==0 && l1>0)-11;
              for (unsigned a=0; a<h; ++a) score=score*5/8;
              if (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              if (l<blen || l<minMatch || l>255) break;
            }
          }
        }
        if (bscore<=0 || blen<minMatch) break;
      }
    }

    // Look up contexts in a hash table.
    // Try the longest context orders first. If a match is found, then
    // skip the lower order as a speed optimization.
    else if (level==1 || minMatch<=64) {
      if (minMatch2>0) {
        for (unsigned k=0; k<=bucket; ++k) {
          unsigned p=ht[h2^k];
          if (p && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            if (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              unsigned l;  // match length from lookahead
              for (l=lookahead; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              if (l>=minMatch2+lookahead) {
                int l1;  // length back from lookahead
                for (l1=lookahead; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
                assert(l1>=0 && l1<=int(lookahead));
                int score=int(l-l1)*8-lg(i-p)-8*(lit==0 && l1>0)-11;
                if (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              }
            }
          }
          if (blen>=128) break;
        }
      }

      // Search the lower order context
      if (!minMatch2 || blen<minMatch2) {
        for (unsigned k=0; k<=bucket; ++k) {
          unsigned p=ht[h1^k];
          if (p && i+3<n && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            if (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              unsigned l;
              for (l=0; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              int score=l*8-lg(i-p)-2*(lit>0)-11;
              if (score>bscore) blen=l, bp=p, blit=0, bscore=score;
            }
          }
          if (blen>=128) break;
        }
      }
    }

    // If match is long enough, then output any pending literals first,
    // and then the match. blen is the length of the match.
    assert(i>=bp);
    const unsigned off=i-bp;  // offset
    if (off>0 && bscore>0
        && blen-blit>=minMatch+(level==2)*((off>=(1<<16))+(off>=(1<<24)))) {
      lit+=blit;
      write_literal(i+blit, lit);
      write_match(blen-blit, off);
    }

    // Otherwise add to literal length
    else {
      blen=1;
      ++lit;
    }

    // Update index, advance blen bytes
    if (isa)
      i+=blen;
    else {
      while (blen--) {
        if (i+minMatchBoth<n) {
          unsigned ih=((i*1234547)>>19)&bucket;
          const unsigned p=(i<<checkbits)|(in[i+3]&mask);
          assert(ih<=bucket);
          if (minMatch2) {
            ht[h2^ih]=p;
            h2=(((h2*9)<<shift2)
                +(in[i+minMatch2+lookahead]+1)*23456789u)&(htsize-1);
          }
          ht[h1^ih]=p;
          h1=(((h1*5)<<shift1)+(in[i+minMatch]+1)*123456791u)&(htsize-1);
        }
        ++i;
      }
    }

    // Write long literals to keep buf from filling up
    if (lit>=maxLiteral)
      write_literal(i, lit);
  }

  // Write pending literals at end of input
  assert(i<=n);
  if (i==n) {
    write_literal(n, lit);
    flush();
  }
}

// Write literal sequence in[i-lit..i-1], set lit=0
void LZBuffer::write_literal(unsigned i, unsigned& lit) {
  assert(lit>=0);
  assert(i>=0 && i<=n);
  assert(i>=lit);
  if (level==1) {
    if (lit<1) return;
    int ll=lg(lit);
    assert(ll>=1 && ll<=24);
    putb(0, 2);
    --ll;
    while (--ll>=0) {
      putb(1, 1);
      putb((lit>>ll)&1, 1);
    }
    putb(0, 1);
    while (lit) putb(in[i-lit--], 8);
  }
  else {
    assert(level==2);
    while (lit>0) {
      unsigned lit1=lit;
      if (lit1>64) lit1=64;
      put(lit1-1);
      for (unsigned j=i-lit; j<i-lit+lit1; ++j) put(in[j]);
      lit-=lit1;
    }
  }
}

// Write match sequence of given length and offset
void LZBuffer::write_match(unsigned len, unsigned off) {

  // mm,mmm,n,ll,r,q[mmmmm-8] = match n*4+ll, offset ((q-1)<<rb)+r+1
  if (level==1) {
    assert(len>=minMatch && len<=maxMatch);
    assert(off>0);
    assert(len>=4);
    assert(rb>=0 && rb<=8);
    int ll=lg(len)-1;
    assert(ll>=2);
    off+=(1<<rb)-1;
    int lo=lg(off)-1-rb;
    assert(lo>=0 && lo<=23);
    putb((lo+8)>>3, 2);// mm
    putb(lo&7, 3);     // mmm
    while (--ll>=2) {  // n
      putb(1, 1);
      putb((len>>ll)&1, 1);
    }
    putb(0, 1);
    putb(len&3, 2);    // ll
    putb(off, rb);     // r
    putb(off>>rb, lo); // q
  }

  // x[2]:len[6] off[x-1] 
  else {
    assert(level==2);
    assert(minMatch>=1 && minMatch<=64);
    --off;
    while (len>0) {  // Split long matches to len1=minMatch..minMatch+63
      const unsigned len1=len>minMatch*2+63 ? minMatch+63 :
          len>minMatch+63 ? len-minMatch : len;
      assert(wpos<BUFSIZE-5);
      assert(len1>=minMatch && len1<minMatch+64);
      if (off<(1<<16)) {
        put(64+len1-minMatch);
        put(off>>8);
        put(off);
      }
      else if (off<(1<<24)) {
        put(128+len1-minMatch);
        put(off>>16);
        put(off>>8);
        put(off);
      }
      else {
        put(192+len1-minMatch);
        put(off>>24);
        put(off>>16);
        put(off>>8);
        put(off);
      }
      len-=len1;
    }
  }
}

// Generate a config file from the method argument with syntax:
// {0|x|s|i}[N1[,N2]...][{ciamtswf<cfg>}[N1[,N2]]...]...
std::string makeConfig(const char* method, int args[]) {
  assert(method);
  const char type=method[0];
  assert(type=='x' || type=='s' || type=='0' || type=='i');

  // Read "{x|s|i|0}N1,N2...N9" into args[0..8] ($1..$9)
  args[0]=0;  // log block size in MiB
  args[1]=0;  // 0=none, 1=var-LZ77, 2=byte-LZ77, 3=BWT, 4..7 adds E8E9
  args[2]=0;  // lz77 minimum match length
  args[3]=0;  // secondary context length
  args[4]=0;  // log searches
  args[5]=0;  // lz77 hash table size or SA if args[0]+21
  args[6]=0;  // secondary context look ahead
  args[7]=0;  // not used
  args[8]=0;  // not used
  if (isdigit(*++method)) args[0]=0;
  for (int i=0; i<9 && (isdigit(*method) || *method==',' || *method=='.');) {
    if (isdigit(*method))
      args[i]=args[i]*10+*method-'0';
    else if (++i<9)
      args[i]=0;
    ++method;
  }

  // "0..." = No compression
  if (type=='0')
    return "comp 0 0 0 0 0 hcomp end\n";

  // Generate the postprocessor
  std::string hdr, pcomp;
  const int level=args[1]&3;
  const bool doe8=args[1]>=4 && args[1]<=7;

  // LZ77+Huffman, with or without E8E9
  if (level==1) {
    const int rb=args[0]>4 ? args[0]-4 : 0;
    hdr="comp 9 16 0 $1+20 ";
    pcomp=
    "pcomp lazy2 3 ;\n"
    " (r1 = state\n"
    "  r2 = len - match or literal length\n"
    "  r3 = m - number of offset bits expected\n"
    "  r4 = ptr to buf\n"
    "  r5 = r - low bits of offset\n"
    "  c = bits - input buffer\n"
    "  d = n - number of bits in c)\n"
    "\n"
    "  a> 255 if\n";
    if (doe8)
      pcomp+=
      "    b=0 d=r 4 do (for b=0..d-1, d = end of buf)\n"
      "      a=b a==d ifnot\n"
      "        a+= 4 a<d if\n"
      "          a=*b a&= 254 a== 232 if (e8 or e9?)\n"
      "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 if (00 or ff)\n"
      "              b-- a=*b\n"
      "              b-- a<<= 8 a+=*b\n"
      "              b-- a<<= 8 a+=*b\n"
      "              a-=b a++\n"
      "              *b=a a>>= 8 b++\n"
      "              *b=a a>>= 8 b++\n"
      "              *b=a b++\n"
      "            endif\n"
      "            b=c\n"
      "          endif\n"
      "        endif\n"
      "        a=*b out b++\n"
      "      forever\n"
      "    endif\n"
      "\n";
    pcomp+=
    "    (reset state)\n"
    "    a=0 b=0 c=0 d=0 r=a 1 r=a 2 r=a 3 r=a 4\n"
    "    halt\n"
    "  endif\n"
    "\n"
    "  a<<=d a+=c c=a               (bits+=a<<n)\n"
    "  a= 8 a+=d d=a                (n+=8)\n"
    "\n"
    "  (if state==0 (expect new code))\n"
    "  a=r 1 a== 0 if (match code mm,mmm)\n"
    "    a= 1 r=a 2                 (len=1)\n"
    "    a=c a&= 3 a> 0 if          (if (bits&3))\n"
    "      a-- a<<= 3 r=a 3           (m=((bits&3)-1)*8)\n"
    "      a=c a>>= 2 c=a             (bits>>=2)\n"
    "      b=r 3 a&= 7 a+=b r=a 3     (m+=bits&7)\n"
    "      a=c a>>= 3 c=a             (bits>>=3)\n"
    "      a=d a-= 5 d=a              (n-=5)\n"
    "      a= 1 r=a 1                 (state=1)\n"
    "    else (literal, discard 00)\n"
    "      a=c a>>= 2 c=a             (bits>>=2)\n"
    "      d-- d--                    (n-=2)\n"
    "      a= 3 r=a 1                 (state=3)\n"
    "    endif\n"
    "  endif\n"
    "\n"
    "  (while state==1 && n>=3 (expect match length n*4+ll -> r2))\n"
    "  do a=r 1 a== 1 if a=d a> 2 if\n"
    "    a=c a&= 1 a== 1 if         (if bits&1)\n"
    "      a=c a>>= 1 c=a             (bits>>=1)\n"
    "      b=r 2 a=c a&= 1 a+=b a+=b r=a 2 (len+=len+(bits&1))\n"
    "      a=c a>>= 1 c=a             (bits>>=1)\n"
    "      d-- d--                    (n-=2)\n"
    "    else\n"
    "      a=c a>>= 1 c=a             (bits>>=1)\n"
    "      a=r 2 a<<= 2 b=a           (len<<=2)\n"
    "      a=c a&= 3 a+=b r=a 2       (len+=bits&3)\n"
    "      a=c a>>= 2 c=a             (bits>>=2)\n"
    "      d-- d-- d--                (n-=3)\n";
    if (rb)
      pcomp+="      a= 5 r=a 1                 (state=5)\n";
    else
      pcomp+="      a= 2 r=a 1                 (state=2)\n";
    pcomp+=
    "    endif\n"
    "  forever endif endif\n"
    "\n";
    if (rb) pcomp+=  // save r in r5
      "  (if state==5 && n>=8) (expect low bits of offset to put in r5)\n"
      "  a=r 1 a== 5 if a=d a> "+itos(rb-1)+" if\n"
      "    a=c a&= "+itos((1<<rb)-1)+" r=a 5            (save r in r5)\n"
      "    a=c a>>= "+itos(rb)+" c=a\n"
      "    a=d a-= "+itos(rb)+ " d=a\n"
      "    a= 2 r=a 1                   (go to state 2)\n"
      "  endif endif\n"
      "\n";
    pcomp+=
    "  (if state==2 && n>=m) (expect m offset bits)\n"
    "  a=r 1 a== 2 if a=r 3 a>d ifnot\n"
    "    a=c r=a 6 a=d r=a 7          (save c=bits, d=n in r6,r7)\n"
    "    b=r 3 a= 1 a<<=b d=a         (d=1<<m)\n"
    "    a-- a&=c a+=d                (d=offset=bits&((1<<m)-1)|(1<<m))\n";
    if (rb)
      pcomp+=  // insert r into low bits of d
      "    a<<= "+itos(rb)+" d=r 5 a+=d a-= "+itos((1<<rb)-1)+"\n";
    pcomp+=
    "    d=a b=r 4 a=b a-=d c=a       (c=p=(b=ptr)-offset)\n"
    "\n"
    "    (while len-- (copy and output match d bytes from *c to *b))\n"
    "    d=r 2 do a=d a> 0 if d--\n"
    "      a=*c *b=a c++ b++          (buf[ptr++]-buf[p++])\n";
    if (!doe8) pcomp+=" out\n";
    pcomp+=
    "    forever endif\n"
    "    a=b r=a 4\n"
    "\n"
    "    a=r 6 b=r 3 a>>=b c=a        (bits>>=m)\n"
    "    a=r 7 a-=b d=a               (n-=m)\n"
    "    a=0 r=a 1                    (state=0)\n"
    "  endif endif\n"
    "\n"
    "  (while state==3 && n>=2 (expect literal length))\n"
    "  do a=r 1 a== 3 if a=d a> 1 if\n"
    "    a=c a&= 1 a== 1 if         (if bits&1)\n"
    "      a=c a>>= 1 c=a              (bits>>=1)\n"
    "      b=r 2 a&= 1 a+=b a+=b r=a 2 (len+=len+(bits&1))\n"
    "      a=c a>>= 1 c=a              (bits>>=1)\n"
    "      d-- d--                     (n-=2)\n"
    "    else\n"
    "      a=c a>>= 1 c=a              (bits>>=1)\n"
    "      d--                         (--n)\n"
    "      a= 4 r=a 1                  (state=4)\n"
    "    endif\n"
    "  forever endif endif\n"
    "\n"
    "  (if state==4 && n>=8 (expect len literals))\n"
    "  a=r 1 a== 4 if a=d a> 7 if\n"
    "    b=r 4 a=c *b=a\n";
    if (!doe8) pcomp+=" out\n";
    pcomp+=
    "    b++ a=b r=a 4                 (buf[ptr++]=bits)\n"
    "    a=c a>>= 8 c=a                (bits>>=8)\n"
    "    a=d a-= 8 d=a                 (n-=8)\n"
    "    a=r 2 a-- r=a 2 a== 0 if      (if --len<1)\n"
    "      a=0 r=a 1                     (state=0)\n"
    "    endif\n"
    "  endif endif\n"
    "  halt\n"
    "end\n";
  }

  // Byte aligned LZ77, with or without E8E9
  else if (level==2) {
    hdr="comp 9 16 0 $1+20 ";
    pcomp=
    "pcomp lzpre c ;\n"
    "  (Decode LZ77: d=state, M=output buffer, b=size)\n"
    "  a> 255 if (at EOF decode e8e9 and output)\n";
    if (doe8)
      pcomp+=
      "    d=b b=0 do (for b=0..d-1, d = end of buf)\n"
      "      a=b a==d ifnot\n"
      "        a+= 4 a<d if\n"
      "          a=*b a&= 254 a== 232 if (e8 or e9?)\n"
      "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 if (00 or ff)\n"
      "              b-- a=*b\n"
      "              b-- a<<= 8 a+=*b\n"
      "              b-- a<<= 8 a+=*b\n"
      "              a-=b a++\n"
      "              *b=a a>>= 8 b++\n"
      "              *b=a a>>= 8 b++\n"
      "              *b=a b++\n"
      "            endif\n"
      "            b=c\n"
      "          endif\n"
      "        endif\n"
      "        a=*b out b++\n"
      "      forever\n"
      "    endif\n";
    pcomp+=
    "    b=0 c=0 d=0 a=0 r=a 1 r=a 2 (reset state)\n"
    "  halt\n"
    "  endif\n"
    "\n"
    "  (in state d==0, expect a new code)\n"
    "  (put length in r1 and inital part of offset in r2)\n"
    "  c=a a=d a== 0 if\n"
    "    a=c a>>= 6 a++ d=a\n"
    "    a== 1 if (literal?)\n"
    "      a+=c r=a 1 a=0 r=a 2\n"
    "    else (3 to 5 byte match)\n"
    "      d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2\n"
    "    endif\n"
    "  else\n"
    "    a== 1 if (writing literal)\n"
    "      a=c *b=a b++\n";
    if (!doe8) pcomp+=" out\n";
    pcomp+=
    "      a=r 1 a-- a== 0 if d=0 endif r=a 1 (if (--len==0) state=0)\n"
    "    else\n"
    "      a> 2 if (reading offset)\n"
    "        a=r 2 a<<= 8 a|=c r=a 2 d-- (off=off<<8|c, --state)\n"
    "      else (state==2, write match)\n"
    "        a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a (c=i-off-1)\n"
    "        d=r 1 (d=len)\n"
    "        do (copy and output d=len bytes)\n"
    "          a=*c *b=a c++ b++\n";
    if (!doe8) pcomp+=" out\n";
    pcomp+=
    "        d-- a=d a> 0 while\n"
    "        (d=state=0. off, len don\'t matter)\n"
    "      endif\n"
    "    endif\n"
    "  endif\n"
    "  halt\n"
    "end\n";
  }

  // BWT with or without E8E9
  else if (level==3) {  // IBWT
    hdr="comp 9 16 $1+20 $1+20 ";  // 2^$1 = block size in MB
    pcomp=
    "pcomp bwtrle c ;\n"
    "\n"
    "  (read BWT, index into M, size in b)\n"
    "  a> 255 ifnot\n"
    "    *b=a b++\n"
    "\n"
    "  (inverse BWT)\n"
    "  elsel\n"
    "\n"
    "    (index in last 4 bytes, put in c and R1)\n"
    "    b-- a=*b\n"
    "    b-- a<<= 8 a+=*b\n"
    "    b-- a<<= 8 a+=*b\n"
    "    b-- a<<= 8 a+=*b c=a r=a 1\n"
    "\n"
    "    (save size in R2)\n"
    "    a=b r=a 2\n"
    "\n"
    "    (count bytes in H[~1..~255, ~0])\n"
    "    do\n"
    "      a=b a> 0 if\n"
    "        b-- a=*b a++ a&= 255 d=a d! *d++\n"
    "      forever\n"
    "    endif\n"
    "\n"
    "    (cumulative counts: H[~i=0..255] = count of bytes before i)\n"
    "    d=0 d! *d= 1 a=0\n"
    "    do\n"
    "      a+=*d *d=a d--\n"
    "    d<>a a! a> 255 a! d<>a until\n"
    "\n"
    "    (build first part of linked list in H[0..idx-1])\n"
    "    b=0 do\n"
    "      a=c a>b if\n"
    "        d=*b d! *d++ d=*d d-- *d=b\n"
    "      b++ forever\n"
    "    endif\n"
    "\n"
    "    (rest of list in H[idx+1..n-1])\n"
    "    b=c b++ c=r 2 do\n"
    "      a=c a>b if\n"
    "        d=*b d! *d++ d=*d d-- *d=b\n"
    "      b++ forever\n"
    "    endif\n"
    "\n";
    if (args[0]<=4) {  // faster IBWT list traversal limited to 16 MB blocks
      pcomp+=
      "    (copy M to low 8 bits of H to reduce cache misses in next loop)\n"
      "    b=0 do\n"
      "      a=c a>b if\n"
      "        d=b a=*d a<<= 8 a+=*b *d=a\n"
      "      b++ forever\n"
      "    endif\n"
      "\n"
      "    (traverse list and output or copy to M)\n"
      "    d=r 1 b=0 do\n"
      "      a=d a== 0 ifnot\n"
      "        a=*d a>>= 8 d=a\n";
      if (doe8) pcomp+=" *b=*d b++\n";
      else      pcomp+=" a=*d out\n";
      pcomp+=
      "      forever\n"
      "    endif\n"
      "\n";
      if (doe8)  // IBWT+E8E9
        pcomp+=
        "    (e8e9 transform to out)\n"
        "    d=b b=0 do (for b=0..d-1, d = end of buf)\n"
        "      a=b a==d ifnot\n"
        "        a+= 4 a<d if\n"
        "          a=*b a&= 254 a== 232 if\n"
        "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 if\n"
        "              b-- a=*b\n"
        "              b-- a<<= 8 a+=*b\n"
        "              b-- a<<= 8 a+=*b\n"
        "              a-=b a++\n"
        "              *b=a a>>= 8 b++\n"
        "              *b=a a>>= 8 b++\n"
        "              *b=a b++\n"
        "            endif\n"
        "            b=c\n"
        "          endif\n"
        "        endif\n"
        "        a=*b out b++\n"
        "      forever\n"
        "    endif\n";
      pcomp+=
      "  endif\n"
      "  halt\n"
      "end\n";
    }
    else {  // slower IBWT list traversal for all sized blocks
      if (doe8) {  // E8E9 after IBWT
        pcomp+=
        "    (R2 = output size without EOS)\n"
        "    a=r 2 a-- r=a 2\n"
        "\n"
        "    (traverse list (d = IBWT pointer) and output inverse e8e9)\n"
        "    (C = offset = 0..R2-1)\n"
        "    (R4 = last 4 bytes shifted in from MSB end)\n"
        "    (R5 = temp pending output byte)\n"
        "    c=0 d=r 1 do\n"
        "      a=d a== 0 ifnot\n"
        "        d=*d\n"
        "\n"
        "        (store byte in R4 and shift out to R5)\n"
        "        b=d a=*b a<<= 24 b=a\n"
        "        a=r 4 r=a 5 a>>= 8 a|=b r=a 4\n"
        "\n"
        "        (if E8|E9 xx xx xx 00|FF in R4:R5 then subtract c from x)\n"
        "        a=c a> 3 if\n"
        "          a=r 5 a&= 254 a== 232 if\n"
        "            a=r 4 a>>= 24 b=a a++ a&= 254 a< 2 if\n"
        "              a=r 4 a-=c a+= 4 a<<= 8 a>>= 8 \n"
        "              b<>a a<<= 24 a+=b r=a 4\n"
        "            endif\n"
        "          endif\n"
        "        endif\n"
        "\n"
        "        (output buffered byte)\n"
        "        a=c a> 3 if a=r 5 out endif c++\n"
        "\n"
        "      forever\n"
        "    endif\n"
        "\n"
        "    (output up to 4 pending bytes in R4)\n"
        "    b=r 4\n"
        "    a=c a> 3 a=b if out endif a>>= 8 b=a\n"
        "    a=c a> 2 a=b if out endif a>>= 8 b=a\n"
        "    a=c a> 1 a=b if out endif a>>= 8 b=a\n"
        "    a=c a> 0 a=b if out endif\n"
        "\n"
        "  endif\n"
        "  halt\n"
        "end\n";
      }
      else {
        pcomp+=
        "    (traverse list and output)\n"
        "    d=r 1 do\n"
        "      a=d a== 0 ifnot\n"
        "        d=*d\n"
        "        b=d a=*b out\n"
        "      forever\n"
        "    endif\n"
        "  endif\n"
        "  halt\n"
        "end\n";
      }
    }
  }

  // E8E9 or no preprocessing
  else if (level==0) {
    hdr="comp 9 16 0 0 ";
    if (doe8) { // E8E9?
      pcomp=
      "pcomp e8e9 d ;\n"
      "  a> 255 if\n"
      "    a=c a> 4 if\n"
      "      c= 4\n"
      "    else\n"
      "      a! a+= 5 a<<= 3 d=a a=b a>>=d b=a\n"
      "    endif\n"
      "    do a=c a> 0 if\n"
      "      a=b out a>>= 8 b=a c--\n"
      "    forever endif\n"
      "  else\n"
      "    *b=b a<<= 24 d=a a=b a>>= 8 a+=d b=a c++\n"
      "    a=c a> 4 if\n"
      "      a=*b out\n"
      "      a&= 254 a== 232 if\n"
      "        a=b a>>= 24 a++ a&= 254 a== 0 if\n"
      "          a=b a>>= 24 a<<= 24 d=a\n"
      "          a=b a-=c a+= 5\n"
      "          a<<= 8 a>>= 8 a|=d b=a\n"
      "        endif\n"
      "      endif\n"
      "    endif\n"
      "  endif\n"
      "  halt\n"
      "end\n";
    }
    else
      pcomp="end\n";
  }
  else
    error("Unsupported method");
  
  // Build context model (comp, hcomp) assuming:
  // H[0..254] = contexts
  // H[255..511] = location of last byte i-255
  // M = last 64K bytes, filling backward
  // C = pointer to most recent byte
  // R1 = level 2 lz77 1+bytes expected until next code, 0=init
  // R2 = level 2 lz77 first byte of code
  int ncomp=0;  // number of components
  const int membits=args[0]+20;
  int sb=5;  // bits in last context
  std::string comp;
  std::string hcomp="hcomp\n"
    "c-- *c=a a+= 255 d=a *d=c\n";
  if (level==2) {  // put level 2 lz77 parse state in R1, R2
    hcomp+=
    "  (decode lz77 into M. Codes:\n"
    "  00xxxxxx = literal length xxxxxx+1\n"
    "  xx......, xx > 0 = match with xx offset bytes to follow)\n"
    "\n"
    "  a=r 1 a== 0 if (init)\n"
    "    a= "+itos(111+57*doe8)+" (skip post code)\n"
    "  else a== 1 if  (new code?)\n"
    "    a=*c r=a 2  (save code in R2)\n"
    "    a> 63 if a>>= 6 a++ a++  (match)\n"
    "    else a++ a++ endif  (literal)\n"
    "  else (read rest of code)\n"
    "    a--\n"
    "  endif endif\n"
    "  r=a 1  (R1 = 1+expected bytes to next code)\n";
  }

  // Generate the context model
  while (*method && ncomp<254) {

    // parse command C[N1[,N2]...] into v = {C, N1, N2...}
    std::vector<int> v;
    v.push_back(*method++);
    if (isdigit(*method)) {
      v.push_back(*method++-'0');
      while (isdigit(*method) || *method==',' || *method=='.') {
        if (isdigit(*method))
          v.back()=v.back()*10+*method++-'0';
        else {
          v.push_back(0);
          ++method;
        }
      }
    }

    // c: context model
    // N1%1000: 0=ICM 1..256=CM limit N1-1
    // N1/1000: number of times to halve memory
    // N2: 1..255=offset mod N2. 1000..1255=distance to N2-1000
    // N3...: 0..255=byte mask + 256=lz77 state. 1000+=run of N3-1000 zeros.
    if (v[0]=='c') {
      while (v.size()<3) v.push_back(0);
      comp+=itos(ncomp)+" ";
      sb=11;  // count context bits
      if (v[2]<256) sb+=lg(v[2]);
      else sb+=6;
      for (unsigned i=3; i<v.size(); ++i)
        if (v[i]<512) sb+=nbits(v[i])*3/4;
      if (sb>membits) sb=membits;
      if (v[1]%1000==0) comp+="icm "+itos(sb-6-v[1]/1000)+"\n";
      else comp+="cm "+itos(sb-2-v[1]/1000)+" "+itos(v[1]%1000-1)+"\n";

      // special contexts
      hcomp+="d= "+itos(ncomp)+" *d=0\n";
      if (v[2]>1 && v[2]<=255) {  // periodic context
        if (lg(v[2])!=lg(v[2]-1))
          hcomp+="a=c a&= "+itos(v[2]-1)+" hashd\n";
        else
          hcomp+="a=c a%= "+itos(v[2])+" hashd\n";
      }
      else if (v[2]>=1000 && v[2]<=1255)  // distance context
        hcomp+="a= 255 a+= "+itos(v[2]-1000)+
               " d=a a=*d a-=c a> 255 if a= 255 endif d= "+
               itos(ncomp)+" hashd\n";

      // Masked context
      for (unsigned i=3; i<v.size(); ++i) {
        if (i==3) hcomp+="b=c ";
        if (v[i]==255)
          hcomp+="a=*b hashd\n";  // ordinary byte
        else if (v[i]>0 && v[i]<255)
          hcomp+="a=*b a&= "+itos(v[i])+" hashd\n";  // masked byte
        else if (v[i]>=256 && v[i]<512) { // lz77 state or masked literal byte
          hcomp+=
          "a=r 1 a> 1 if\n"  // expect literal or offset
          "  a=r 2 a< 64 if\n"  // expect literal
          "    a=*b ";
          if (v[i]<511) hcomp+="a&= "+itos(v[i]-256);
          hcomp+=" hashd\n"
          "  else\n"  // expect match offset byte
          "    a>>= 6 hashd a=r 1 hashd\n"
          "  endif\n"
          "else\n"  // expect new code
          "  a= 255 hashd a=r 2 hashd\n"
          "endif\n";
        }
        else if (v[i]>=1256)  // skip v[i]-1000 bytes
          hcomp+="a= "+itos(((v[i]-1000)>>8)&255)+" a<<= 8 a+= "
               +itos((v[i]-1000)&255)+
          " a+=b b=a\n";
        else if (v[i]>1000)
          hcomp+="a= "+itos(v[i]-1000)+" a+=b b=a\n";
        if (v[i]<512 && i<v.size()-1)
          hcomp+="b++ ";
      }
      ++ncomp;
    }

    // m,8,24: MIX, size, rate
    // t,8,24: MIX2, size, rate
    // s,8,32,255: SSE, size, start, limit
    if (strchr("mts", v[0]) && ncomp>int(v[0]=='t')) {
      if (v.size()<=1) v.push_back(8);
      if (v.size()<=2) v.push_back(24+8*(v[0]=='s'));
      if (v[0]=='s' && v.size()<=3) v.push_back(255);
      comp+=itos(ncomp);
      sb=5+v[1]*3/4;
      if (v[0]=='m')
        comp+=" mix "+itos(v[1])+" 0 "+itos(ncomp)+" "+itos(v[2])+" 255\n";
      else if (v[0]=='t')
        comp+=" mix2 "+itos(v[1])+" "+itos(ncomp-1)+" "+itos(ncomp-2)
            +" "+itos(v[2])+" 255\n";
      else // s
        comp+=" sse "+itos(v[1])+" "+itos(ncomp-1)+" "+itos(v[2])+" "
            +itos(v[3])+"\n";
      if (v[1]>8) {
        hcomp+="d= "+itos(ncomp)+" *d=0 b=c a=0\n";
        for (; v[1]>=16; v[1]-=8) {
          hcomp+="a<<= 8 a+=*b";
          if (v[1]>16) hcomp+=" b++";
          hcomp+="\n";
        }
        if (v[1]>8)
          hcomp+="a<<= 8 a+=*b a>>= "+itos(16-v[1])+"\n";
        hcomp+="a<<= 8 *d=a\n";
      }
      ++ncomp;
    }

    // i: ISSE chain with order increasing by N1,N2...
    if (v[0]=='i' && ncomp>0) {
      assert(sb>=5);
      hcomp+="d= "+itos(ncomp-1)+" b=c a=*d d++\n";
      for (unsigned i=1; i<v.size() && ncomp<254; ++i) {
        for (int j=0; j<v[i]%10; ++j) {
          hcomp+="hash ";
          if (i<v.size()-1 || j<v[i]%10-1) hcomp+="b++ ";
          sb+=6;
        }
        hcomp+="*d=a";
        if (i<v.size()-1) hcomp+=" d++";
        hcomp+="\n";
        if (sb>membits) sb=membits;
        comp+=itos(ncomp)+" isse "+itos(sb-6-v[i]/10)+" "+itos(ncomp-1)+"\n";
        ++ncomp;
      }
    }

    // a24,0,0: MATCH. N1=hash multiplier. N2,N3=halve buf, table.
    if (v[0]=='a') {
      if (v.size()<=1) v.push_back(24);
      while (v.size()<4) v.push_back(0);
      comp+=itos(ncomp)+" match "+itos(membits-v[3]-2)+" "
          +itos(membits-v[2])+"\n";
      hcomp+="d= "+itos(ncomp)+" a=*d a*= "+itos(v[1])
           +" a+=*c a++ *d=a\n";
      sb=5+(membits-v[2])*3/4;
      ++ncomp;
    }

    // w1,65,26,223,20,0: ICM-ISSE chain of length N1 with word contexts,
    // where a word is a sequence of c such that c&N4 is in N2..N2+N3-1.
    // Word is hashed by: hash := hash*N5+c+1
    // Decrease memory by 2^-N6.
    if (v[0]=='w') {
      if (v.size()<=1) v.push_back(1);
      if (v.size()<=2) v.push_back(65);
      if (v.size()<=3) v.push_back(26);
      if (v.size()<=4) v.push_back(223);
      if (v.size()<=5) v.push_back(20);
      if (v.size()<=6) v.push_back(0);
      comp+=itos(ncomp)+" icm "+itos(membits-6-v[6])+"\n";
      for (int i=1; i<v[1]; ++i)
        comp+=itos(ncomp+i)+" isse "+itos(membits-6-v[6])+" "
            +itos(ncomp+i-1)+"\n";
      hcomp+="a=*c a&= "+itos(v[4])+" a-= "+itos(v[2])+" a&= 255 a< "
           +itos(v[3])+" if\n";
      for (int i=0; i<v[1]; ++i) {
        if (i==0) hcomp+="  d= "+itos(ncomp);
        else hcomp+="  d++";
        hcomp+=" a=*d a*= "+itos(v[5])+" a+=*c a++ *d=a\n";
      }
      hcomp+="else\n";
      for (int i=v[1]-1; i>0; --i)
        hcomp+="  d= "+itos(ncomp+i-1)+" a=*d d++ *d=a\n";
      hcomp+="  d= "+itos(ncomp)+" *d=0\n"
           "endif\n";
      ncomp+=v[1]-1;
      sb=membits-v[6];
      ++ncomp;
    }
  }
  return hdr+itos(ncomp)+"\n"+comp+hcomp+"halt\n"+pcomp;
}

// Compress from in to out in 1 segment in 1 block using the algorithm
// descried in method. If method begins with a digit then choose
// a method depending on type. Save filename and comment
// in the segment header. If comment is 0 then the default is the input size
// as a decimal string, plus " jDC\x01" for a journaling method (method[0]
// is not 's'). Write the generated method to methodOut if not 0.
void compressBlock(StringBuffer* in, Writer* out, const char* method_,
                   const char* filename, const char* comment, bool dosha1) {
  assert(in);
  assert(out);
  assert(method_);
  assert(method_[0]);
  std::string method=method_;
  const unsigned n=in->size();  // input size
  const int arg0=MAX(lg(n+4095)-20, 0);  // block size
  assert((1u<<(arg0+20))>=n+4096);

  // Get type from method "LB,R,t" where L is level 0..5, B is block
  // size 0..11, R is redundancy 0..255, t = 0..3 = binary, text, exe, both.
  unsigned type=0;
  if (isdigit(method[0])) {
    int commas=0, arg[4]={0};
    for (int i=1; i<int(method.size()) && commas<4; ++i) {
      if (method[i]==',' || method[i]=='.') ++commas;
      else if (isdigit(method[i])) arg[commas]=arg[commas]*10+method[i]-'0';
    }
    if (commas==0) type=512;
    else type=arg[1]*4+arg[2];
  }

  // Get hash of input
  libzpaq::SHA1 sha1;
  const char* sha1ptr=0;
#ifdef DEBUG
  if (true) {
#else
  if (dosha1) {
#endif
    sha1.write(in->c_str(), n);
    sha1ptr=sha1.result();
  }

  // Expand default methods
  if (isdigit(method[0])) {
    const int level=method[0]-'0';
    assert(level>=0 && level<=9);

    // build models
    const int doe8=(type&2)*2;
    method="x"+itos(arg0);
    std::string htsz=","+itos(19+arg0+(arg0<=6));  // lz77 hash table size
    std::string sasz=","+itos(21+arg0);            // lz77 suffix array size

    // store uncompressed
    if (level==0)
      method="0"+itos(arg0)+",0";

    // LZ77, no model. Store if hard to compress
    else if (level==1) {
      if (type<40) method+=",0";
      else {
        method+=","+itos(1+doe8)+",";
        if      (type<80)  method+="4,0,1,15";
        else if (type<128) method+="4,0,2,16";
        else if (type<256) method+="4,0,2"+htsz;
        else if (type<960) method+="5,0,3"+htsz;
        else               method+="6,0,3"+htsz;
      }
    }

    // LZ77 with longer search
    else if (level==2) {
      if (type<32) method+=",0";
      else {
        method+=","+itos(1+doe8)+",";
        if (type<64) method+="4,0,3"+htsz;
        else method+="4,0,7"+sasz+",1";
      }
    }

    // LZ77 with CM depending on redundancy
    else if (level==3) {
      if (type<20)  // store if not compressible
        method+=",0";
      else if (type<48)  // fast LZ77 if barely compressible
        method+=","+itos(1+doe8)+",4,0,3"+htsz;
      else if (type>=640 || (type&1))  // BWT if text or highly compressible
        method+=","+itos(3+doe8)+"ci1";
      else  // LZ77 with O0-1 compression of up to 12 literals
        method+=","+itos(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";
    }

    // LZ77+CM, fast CM, or BWT depending on type
    else if (level==4) {
      if (type<12)
        method+=",0";
      else if (type<24)
        method+=","+itos(1+doe8)+",4,0,3"+htsz;
      else if (type<48)
        method+=","+itos(2+doe8)+",5,0,7"+sasz+"1c0,0,511";
      else if (type<900) {
        method+=","+itos(doe8)+"ci1,1,1,1,2a";
        if (type&1) method+="w";
        method+="m";
      }
      else
        method+=","+itos(3+doe8)+"ci1";
    }

    // Slow CM with lots of models
    else {  // 5..9

      // Model text files
      method+=","+itos(doe8);
      if (type&1) method+="w2c0,1010,255i1";
      else method+="w1i1";
      method+="c256ci1,1,1,1,1,1,2a";

      // Analyze the data
      const int NR=1<<12;
      int pt[256]={0};  // position of last occurrence
      int r[NR]={0};    // count repetition gaps of length r
      const unsigned char* p=in->data();
      if (level>0) {
        for (unsigned i=0; i<n; ++i) {
          const int k=i-pt[p[i]];
          if (k>0 && k<NR) ++r[k];
          pt[p[i]]=i;
        }
      }

      // Add periodic models
      int n1=n-r[1]-r[2]-r[3];
      for (int i=0; i<2; ++i) {
        int period=0;
        double score=0;
        int t=0;
        for (int j=5; j<NR && t<n1; ++j) {
          const double s=r[j]/(256.0+n1-t);
          if (s>score) score=s, period=j;
          t+=r[j];
        }
        if (period>4 && score>0.1) {
          method+="c0,0,"+itos(999+period)+",255i1";
          if (period<=255)
            method+="c0,"+itos(period)+"i1";
          n1-=r[period];
          r[period]=0;
        }
        else
          break;
      }
      method+="c0,2,0,255i1c0,3,0,0,255i1c0,4,0,0,0,255i1mm16ts19t0";
    }
  }

  // Compress
  std::string config;
  int args[9]={0};
  config=makeConfig(method.c_str(), args);
  assert(n<=(0x100000u<<args[0])-4096);
  libzpaq::Compressor co;
  co.setOutput(out);
#ifdef DEBUG
  co.setVerify(true);
#endif
  StringBuffer pcomp_cmd;
  co.writeTag();
  co.startBlock(config.c_str(), args, &pcomp_cmd);
  std::string cs=itos(n);
  if (comment) cs=cs+" "+comment;
  co.startSegment(filename, cs.c_str());
  if (args[1]>=1 && args[1]<=7 && args[1]!=4) {  // LZ77 or BWT
    LZBuffer lz(*in, args);
    co.setInput(&lz);
    co.compress();
  }
  else {  // compress with e8e9 or no preprocessing
    if (args[1]>=4 && args[1]<=7)
      e8e9(in->data(), in->size());
    co.setInput(in);
    co.compress();
  }
#ifdef DEBUG  // verify pre-post processing are inverses
  int64_t outsize;
  const char* sha1result=co.endSegmentChecksum(&outsize, dosha1);
  assert(sha1result);
  assert(sha1ptr);
  if (memcmp(sha1result, sha1ptr, 20)!=0)
    error("Pre/post-processor test failed");
#else
  co.endSegment(sha1ptr);
#endif
  co.endBlock();
}

}  // end namespace libzpaq


#undef off
#undef offc

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <fcntl.h>

#ifndef DEBUG
#define NDEBUG 1
#endif
#include <assert.h>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#ifndef unix
#define unix 1
#endif
#endif
#ifdef unix
#define PTHREAD 1
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>
#include <utime.h>
#include <errno.h>
#ifdef BSD
#include <sys/sysctl.h>
#endif

#else  // Assume Windows
#include <windows.h>
#include <io.h>
#endif

// For testing -Dunix in Windows
#ifdef unixtest
#define lstat(a,b) stat(a,b)
#define mkdir(a,b) mkdir(a)
#ifndef fseeko
#define fseeko(a,b,c) fseeko64(a,b,c)
#endif
#ifndef ftello
#define ftello(a) ftello64(a)
#endif
#endif

using std::string;
using std::vector;
using std::map;
using std::min;
using std::max;
using libzpaq::StringBuffer;

// Handle errors in libzpaq and elsewhere
void libzpaq::error(const char* msg) {
  if (strstr(msg, "ut of memory")) throw std::bad_alloc();
  throw std::runtime_error(msg);
}
using libzpaq::error;

// Portable thread types and functions for Windows and Linux. Use like this:
//
// // Create mutex for locking thread-unsafe code
// Mutex mutex;            // shared by all threads
// init_mutex(mutex);      // initialize in unlocked state
// Semaphore sem(n);       // n >= 0 is initial state
//
// // Declare a thread function
// ThreadReturn thread(void *arg) {  // arg points to in/out parameters
//   lock(mutex);          // wait if another thread has it first
//   release(mutex);       // allow another waiting thread to continue
//   sem.wait();           // wait until n>0, then --n
//   sem.signal();         // ++n to allow waiting threads to continue
//   return 0;             // must return 0 to exit thread
// }
//
// // Start a thread
// ThreadID tid;
// run(tid, thread, &arg); // runs in parallel
// join(tid);              // wait for thread to return
// destroy_mutex(mutex);   // deallocate resources used by mutex
// sem.destroy();          // deallocate resources used by semaphore

#ifdef PTHREAD
#include <pthread.h>
typedef void* ThreadReturn;                                // job return type
typedef pthread_t ThreadID;                                // job ID type
void run(ThreadID& tid, ThreadReturn(*f)(void*), void* arg)// start job
  {pthread_create(&tid, NULL, f, arg);}
void join(ThreadID tid) {pthread_join(tid, NULL);}         // wait for job
typedef pthread_mutex_t Mutex;                             // mutex type
void init_mutex(Mutex& m) {pthread_mutex_init(&m, 0);}     // init mutex
void lock(Mutex& m) {pthread_mutex_lock(&m);}              // wait for mutex
void release(Mutex& m) {pthread_mutex_unlock(&m);}         // release mutex
void destroy_mutex(Mutex& m) {pthread_mutex_destroy(&m);}  // destroy mutex

class Semaphore {
public:
  Semaphore() {sem=-1;}
  void init(int n) {
    assert(n>=0);
    assert(sem==-1);
    pthread_cond_init(&cv, 0);
    pthread_mutex_init(&mutex, 0);
    sem=n;
  }
  void destroy() {
    assert(sem>=0);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cv);
  }
  int wait() {
    assert(sem>=0);
    pthread_mutex_lock(&mutex);
    int r=0;
    if (sem==0) r=pthread_cond_wait(&cv, &mutex);
    assert(sem>0);
    --sem;
    pthread_mutex_unlock(&mutex);
    return r;
  }
  void signal() {
    assert(sem>=0);
    pthread_mutex_lock(&mutex);
    ++sem;
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mutex);
  }
private:
  pthread_cond_t cv;  // to signal FINISHED
  pthread_mutex_t mutex; // protects cv
  int sem;  // semaphore count
};

#else  // Windows
typedef DWORD ThreadReturn;
typedef HANDLE ThreadID;
void run(ThreadID& tid, ThreadReturn(*f)(void*), void* arg) {
  tid=CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)f, arg, 0, NULL);
  if (tid==NULL) error("CreateThread failed");
}
void join(ThreadID& tid) {WaitForSingleObject(tid, INFINITE);}
typedef HANDLE Mutex;
void init_mutex(Mutex& m) {m=CreateMutex(NULL, FALSE, NULL);}
void lock(Mutex& m) {WaitForSingleObject(m, INFINITE);}
void release(Mutex& m) {ReleaseMutex(m);}
void destroy_mutex(Mutex& m) {CloseHandle(m);}

class Semaphore {
public:
  enum {MAXCOUNT=2000000000};
  Semaphore(): h(NULL) {}
  void init(int n) {assert(!h); h=CreateSemaphore(NULL, n, MAXCOUNT, NULL);}
  void destroy() {assert(h); CloseHandle(h);}
  int wait() {assert(h); return WaitForSingleObject(h, INFINITE);}
  void signal() {assert(h); ReleaseSemaphore(h, 1, NULL);}
private:
  HANDLE h;  // Windows semaphore
};

#endif

// Global variables
int64_t global_start=0;  // set to mtime() at start of main()

// In Windows, convert 16-bit wide string to UTF-8 and \ to /
#ifndef unix
string wtou(const wchar_t* s) {
  assert(sizeof(wchar_t)==2);  // Not true in Linux
  assert((wchar_t)(-1)==65535);
  string r;
  if (!s) return r;
  for (; *s; ++s) {
    if (*s=='\\') r+='/';
    else if (*s<128) r+=*s;
    else if (*s<2048) r+=192+*s/64, r+=128+*s%64;
    else r+=224+*s/4096, r+=128+*s/64%64, r+=128+*s%64;
  }
  return r;
}

// In Windows, convert UTF-8 string to wide string ignoring
// invalid UTF-8 or >64K. Convert "/" to slash (default "\").
std::wstring utow(const char* ss, char slash='\\') {
  assert(sizeof(wchar_t)==2);
  assert((wchar_t)(-1)==65535);
  std::wstring r;
  if (!ss) return r;
  const unsigned char* s=(const unsigned char*)ss;
  for (; s && *s; ++s) {
    if (s[0]=='/') r+=slash;
    else if (s[0]<128) r+=s[0];
    else if (s[0]>=192 && s[0]<224 && s[1]>=128 && s[1]<192)
      r+=(s[0]-192)*64+s[1]-128, ++s;
    else if (s[0]>=224 && s[0]<240 && s[1]>=128 && s[1]<192
             && s[2]>=128 && s[2]<192)
      r+=(s[0]-224)*4096+(s[1]-128)*64+s[2]-128, s+=2;
  }
  return r;
}
#endif

// Print a UTF-8 string to f (stdout, stderr) so it displays properly
void printUTF8(const char* s, FILE* f=stdout) {
  assert(f);
  assert(s);
#ifdef unix
  fprintf(f, "%s", s);
#else
  const HANDLE h=(HANDLE)_get_osfhandle(_fileno(f));
  DWORD ft=GetFileType(h);
  if (ft==FILE_TYPE_CHAR) {
    fflush(f);
    std::wstring w=utow(s, '/');  // Windows console: convert to UTF-16
    DWORD n=0;
    WriteConsole(h, w.c_str(), w.size(), &n, 0);
  }
  else  // stdout redirected to file
    fprintf(f, "%s", s);
#endif
}

// Return relative time in milliseconds
int64_t mtime() {
#ifdef unix
  timeval tv;
  gettimeofday(&tv, 0);
  return tv.tv_sec*1000LL+tv.tv_usec/1000;
#else
  int64_t t=GetTickCount();
  if (t<global_start) t+=0x100000000LL;
  return t;
#endif
}

// Convert 64 bit decimal YYYYMMDDHHMMSS to "YYYY-MM-DD HH:MM:SS"
// where -1 = unknown date, 0 = deleted.
string dateToString(int64_t date) {
  if (date<=0) return "                   ";
  string s="0000-00-00 00:00:00";
  static const int t[]={18,17,15,14,12,11,9,8,6,5,3,2,1,0};
  for (int i=0; i<14; ++i) s[t[i]]+=int(date%10), date/=10;
  return s;
}

// Convert attributes to a readable format
string attrToString(int64_t attrib) {
  string r="     ";
  if ((attrib&255)=='u') {
    r[0]="0pc3d5b7 9lBsDEF"[(attrib>>20)&15];
    for (int i=0; i<4; ++i)
      r[4-i]=(attrib>>(8+3*i))%8+'0';
  }
  else if ((attrib&255)=='w') {
    for (int i=0, j=0; i<32; ++i) {
      if ((attrib>>(i+8))&1) {
        char c="RHS DAdFTprCoIEivs89012345678901"[i];
        if (j<5) r[j]=c;
        else r+=c;
        ++j;
      }
    }
  }
  return r;
}

// Convert seconds since 0000 1/1/1970 to 64 bit decimal YYYYMMDDHHMMSS
// Valid from 1970 to 2099.
int64_t decimal_time(time_t tt) {
  if (tt==-1) tt=0;
  int64_t t=(sizeof(tt)==4) ? unsigned(tt) : tt;
  const int second=t%60;
  const int minute=t/60%60;
  const int hour=t/3600%24;
  t/=86400;  // days since Jan 1 1970
  const int term=t/1461;  // 4 year terms since 1970
  t%=1461;
  t+=(t>=59);  // insert Feb 29 on non leap years
  t+=(t>=425);
  t+=(t>=1157);
  const int year=term*4+t/366+1970;  // actual year
  t%=366;
  t+=(t>=60)*2;  // make Feb. 31 days
  t+=(t>=123);   // insert Apr 31
  t+=(t>=185);   // insert June 31
  t+=(t>=278);   // insert Sept 31
  t+=(t>=340);   // insert Nov 31
  const int month=t/31+1;
  const int day=t%31+1;
  return year*10000000000LL+month*100000000+day*1000000
         +hour*10000+minute*100+second;
}

// Convert decimal date to time_t - inverse of decimal_time()
time_t unix_time(int64_t date) {
  if (date<=0) return -1;
  static const int days[12]={0,31,59,90,120,151,181,212,243,273,304,334};
  const int year=date/10000000000LL%10000;
  const int month=(date/100000000%100-1)%12;
  const int day=date/1000000%100;
  const int hour=date/10000%100;
  const int min=date/100%100;
  const int sec=date%100;
  return (day-1+days[month]+(year%4==0 && month>1)+((year-1970)*1461+1)/4)
    *86400+hour*3600+min*60+sec;
}

/////////////////////////////// File //////////////////////////////////

// Windows/Linux compatible file type
#ifdef unix
typedef FILE* FP;
const FP FPNULL=NULL;
const char* const RB="rb";
const char* const WB="wb";
const char* const RBPLUS="rb+";
const char* const WBPLUS="wb+";

#else // Windows
typedef HANDLE FP;
const FP FPNULL=INVALID_HANDLE_VALUE;
typedef enum {RB, WB, RBPLUS, WBPLUS} MODE;  // fopen modes

// Open file. Only modes "rb", "wb", "rb+" and "wb+" are supported.
FP fopen(const char* filename, MODE mode) {
  assert(filename);
  DWORD access=0;
  if (mode!=WB) access=GENERIC_READ;
  if (mode!=RB) access|=GENERIC_WRITE;
  DWORD disp=OPEN_ALWAYS;  // wb or wb+
  if (mode==RB || mode==RBPLUS) disp=OPEN_EXISTING;
  DWORD share=FILE_SHARE_READ;
  if (mode==RB) share|=FILE_SHARE_WRITE|FILE_SHARE_DELETE;
  return CreateFile(utow(filename).c_str(), access, share,
                    NULL, disp, FILE_ATTRIBUTE_NORMAL, NULL);
}

// Close file
int fclose(FP fp) {
  return CloseHandle(fp) ? 0 : EOF;
}

// Read nobj objects of size size into ptr. Return number of objects read.
size_t fread(void* ptr, size_t size, size_t nobj, FP fp) {
  DWORD r=0;
  ReadFile(fp, ptr, size*nobj, &r, NULL);
  if (size>1) r/=size;
  return r;
}

// Write nobj objects of size size from ptr to fp. Return number written.
size_t fwrite(const void* ptr, size_t size, size_t nobj, FP fp) {
  DWORD r=0;
  WriteFile(fp, ptr, size*nobj, &r, NULL);
  if (size>1) r/=size;
  return r;
}

// Move file pointer by offset. origin is SEEK_SET (from start), SEEK_CUR,
// (from current position), or SEEK_END (from end).
int fseeko(FP fp, int64_t offset, int origin) {
  if (origin==SEEK_SET) origin=FILE_BEGIN;
  else if (origin==SEEK_CUR) origin=FILE_CURRENT;
  else if (origin==SEEK_END) origin=FILE_END;
  LONG h=uint64_t(offset)>>32;
  SetFilePointer(fp, offset&0xffffffffull, &h, origin);
  return GetLastError()!=NO_ERROR;
}

// Get file position
int64_t ftello(FP fp) {
  LONG h=0;
  DWORD r=SetFilePointer(fp, 0, &h, FILE_CURRENT);
  return r+(uint64_t(h)<<32);
}

#endif

// Return true if a file or directory (UTF-8 without trailing /) exists.
bool exists(string filename) {
  int len=filename.size();
  if (len<1) return false;
  if (filename[len-1]=='/') filename=filename.substr(0, len-1);
#ifdef unix
  struct stat sb;
  return !lstat(filename.c_str(), &sb);
#else
  return GetFileAttributes(utow(filename.c_str()).c_str())
         !=INVALID_FILE_ATTRIBUTES;
#endif
}

// Delete a file, return true if successful
bool delete_file(const char* filename) {
#ifdef unix
  return remove(filename)==0;
#else
  return DeleteFile(utow(filename).c_str());
#endif
}

#ifdef unix

// Print last error message
void printerr(const char* filename) {
  perror(filename);
}

#else

// Print last error message
void printerr(const char* filename) {
  fflush(stdout);
  int err=GetLastError();
  printUTF8(filename, stderr);
  if (err==ERROR_FILE_NOT_FOUND)
    fprintf(stderr, ": file not found\n");
  else if (err==ERROR_PATH_NOT_FOUND)
    fprintf(stderr, ": path not found\n");
  else if (err==ERROR_ACCESS_DENIED)
    fprintf(stderr, ": access denied\n");
  else if (err==ERROR_SHARING_VIOLATION)
    fprintf(stderr, ": sharing violation\n");
  else if (err==ERROR_BAD_PATHNAME)
    fprintf(stderr, ": bad pathname\n");
  else if (err==ERROR_INVALID_NAME)
    fprintf(stderr, ": invalid name\n");
  else if (err==ERROR_NETNAME_DELETED)
    fprintf(stderr, ": network name no longer available\n");
  else
    fprintf(stderr, ": Windows error %d\n", err);
}

#endif

// Close fp if open. Set date and attributes unless 0
void close(const char* filename, int64_t date, int64_t attr, FP fp=FPNULL) {
  assert(filename);
#ifdef unix
  if (fp!=FPNULL) fclose(fp);
  if (date>0) {
    struct utimbuf ub;
    ub.actime=time(NULL);
    ub.modtime=unix_time(date);
    utime(filename, &ub);
  }
  if ((attr&255)=='u')
    chmod(filename, attr>>8);
#else
  const bool ads=strstr(filename, ":$DATA")!=0;  // alternate data stream?
  if (date>0 && !ads) {
    if (fp==FPNULL)
      fp=CreateFile(utow(filename).c_str(),
                    FILE_WRITE_ATTRIBUTES,
                    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (fp!=FPNULL) {
      SYSTEMTIME st;
      st.wYear=date/10000000000LL%10000;
      st.wMonth=date/100000000%100;
      st.wDayOfWeek=0;  // ignored
      st.wDay=date/1000000%100;
      st.wHour=date/10000%100;
      st.wMinute=date/100%100;
      st.wSecond=date%100;
      st.wMilliseconds=0;
      FILETIME ft;
      SystemTimeToFileTime(&st, &ft);
      SetFileTime(fp, NULL, NULL, &ft);
    }
  }
  if (fp!=FPNULL) CloseHandle(fp);
  if ((attr&255)=='w' && !ads)
    SetFileAttributes(utow(filename).c_str(), attr>>8);
#endif
}

// Print file open error and throw exception
void ioerr(const char* msg) {
  printerr(msg);
  throw std::runtime_error(msg);
}

// Create directories as needed. For example if path="/tmp/foo/bar"
// then create directories /, /tmp, and /tmp/foo unless they exist.
// Set date and attributes if not 0.
void makepath(string path, int64_t date=0, int64_t attr=0) {
  for (unsigned i=0; i<path.size(); ++i) {
    if (path[i]=='\\' || path[i]=='/') {
      path[i]=0;
#ifdef unix
      mkdir(path.c_str(), 0777);
#else
      CreateDirectory(utow(path.c_str()).c_str(), 0);
#endif
      path[i]='/';
    }
  }

  // Set date and attributes
  string filename=path;
  if (filename!="" && filename[filename.size()-1]=='/')
    filename=filename.substr(0, filename.size()-1);  // remove trailing slash
  close(filename.c_str(), date, attr);
}

#ifndef unix

// Truncate filename to length. Return -1 if error, else 0.
int truncate(const char* filename, int64_t length) {
  std::wstring w=utow(filename);
  HANDLE out=CreateFile(w.c_str(), GENERIC_READ | GENERIC_WRITE,
                        0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  if (out!=INVALID_HANDLE_VALUE) {
    LONG hi=length>>32;
    if (SetFilePointer(out, length, &hi, FILE_BEGIN)
             !=INVALID_SET_FILE_POINTER
        && SetEndOfFile(out)
        && CloseHandle(out))
      return 0;
  }
  return -1;
}
#endif

/////////////////////////////// Archive ///////////////////////////////

// Convert non-negative decimal number x to string of at least n digits
string itos(int64_t x, int n=1) {
  assert(x>=0);
  assert(n>=0);
  string r;
  for (; x || n>0; x/=10, --n) r=string(1, '0'+x%10)+r;
  return r;
}

// Replace * and ? in fn with part or digits of part
string subpart(string fn, int part) {
  for (int j=fn.size()-1; j>=0; --j) {
    if (fn[j]=='?')
      fn[j]='0'+part%10, part/=10;
    else if (fn[j]=='*')
      fn=fn.substr(0, j)+itos(part)+fn.substr(j+1), part=0;
  }
  return fn;
}

// Base of InputArchive and OutputArchive
class ArchiveBase {
protected:
  libzpaq::AES_CTR* aes;  // NULL if not encrypted
  FP fp;          // currently open file or FPNULL
public:
  ArchiveBase(): aes(0), fp(FPNULL) {}
  ~ArchiveBase() {
    if (aes) delete aes;
    if (fp!=FPNULL) fclose(fp);
  }  
  bool isopen() {return fp!=FPNULL;}
};

// An InputArchive supports encrypted reading
class InputArchive: public ArchiveBase, public libzpaq::Reader {
  vector<int64_t> sz;  // part sizes
  int64_t off;  // current offset
  string fn;  // filename, possibly multi-part with wildcards
public:

  // Open filename. If password then decrypt input.
  InputArchive(const char* filename, const char* password=0);

  // Read and return 1 byte or -1 (EOF)
  int get() {
    error("get() not implemented");
    return -1;
  }

  // Read up to len bytes into obuf at current offset. Return 0..len bytes
  // actually read. 0 indicates EOF.
  int read(char* obuf, int len) {
    int nr=fread(obuf, 1, len, fp);
    if (nr==0) {
      seek(0, SEEK_CUR);
      nr=fread(obuf, 1, len, fp);
    }
    if (nr==0) return 0;
    if (aes) aes->encrypt(obuf, nr, off);
    off+=nr;
    return nr;
  }

  // Like fseeko()
  void seek(int64_t p, int whence);

  // Like ftello()
  int64_t tell() {
    return off;
  }
};

// Like fseeko. If p is out of range then close file.
void InputArchive::seek(int64_t p, int whence) {
  if (!isopen()) return;

  // Compute new offset
  if (whence==SEEK_SET) off=p;
  else if (whence==SEEK_CUR) off+=p;
  else if (whence==SEEK_END) {
    off=p;
    for (unsigned i=0; i<sz.size(); ++i) off+=sz[i];
  }

  // Optimization for single file to avoid close and reopen
  if (sz.size()==1) {
    fseeko(fp, off, SEEK_SET);
    return;
  }

  // Seek across multiple files
  assert(sz.size()>1);
  int64_t sum=0;
  unsigned i;
  for (i=0;; ++i) {
    sum+=sz[i];
    if (sum>off || i+1>=sz.size()) break;
  }
  const string next=subpart(fn, i+1);
  fclose(fp);
  fp=fopen(next.c_str(), RB);
  if (fp==FPNULL) ioerr(next.c_str());
  fseeko(fp, off-sum, SEEK_END);
}

// Open for input. Decrypt with password and using the salt in the
// first 32 bytes. If filename has wildcards then assume multi-part
// and read their concatenation.

InputArchive::InputArchive(const char* filename, const char* password):
    off(0), fn(filename) {
  assert(filename);

  // Get file sizes
  const string part0=subpart(filename, 0);
  for (unsigned i=1; ; ++i) {
    const string parti=subpart(filename, i);
    if (i>1 && parti==part0) break;
    fp=fopen(parti.c_str(), RB);
    if (fp==FPNULL) break;
    fseeko(fp, 0, SEEK_END);
    sz.push_back(ftello(fp));
    fclose(fp);
  }

  // Open first part
  const string part1=subpart(filename, 1);
  fp=fopen(part1.c_str(), RB);
  if (!isopen()) ioerr(part1.c_str());
  assert(fp!=FPNULL);

  // Get encryption salt
  if (password) {
    char salt[32], key[32];
    if (fread(salt, 1, 32, fp)!=32) error("cannot read salt");
    libzpaq::stretchKey(key, password, salt);
    aes=new libzpaq::AES_CTR(key, 32, salt);
    off=32;
  }
}

// An Archive is a file supporting encryption
class OutputArchive: public ArchiveBase, public libzpaq::Writer {
  int64_t off;    // preceding multi-part bytes
  unsigned ptr;   // write pointer in buf: 0 <= ptr <= BUFSIZE
  enum {BUFSIZE=1<<16};
  char buf[BUFSIZE];  // I/O buffer
public:

  // Open. If password then encrypt output.
  OutputArchive(const char* filename, const char* password=0,
                const char* salt_=0, int64_t off_=0);

  // Write pending output
  void flush() {
    assert(fp!=FPNULL);
    if (aes) aes->encrypt(buf, ptr, ftello(fp)+off);
    fwrite(buf, 1, ptr, fp);
    ptr=0;
  }

  // Position the next read or write offset to p.
  void seek(int64_t p, int whence) {
    if (fp!=FPNULL) {
      flush();
      fseeko(fp, p, whence);
    }
    else if (whence==SEEK_SET) off=p;
    else off+=p;  // assume at end
  }

  // Return current file offset.
  int64_t tell() const {
    if (fp!=FPNULL) return ftello(fp)+ptr;
    else return off;
  }

  // Write one byte
  void put(int c) {
    if (fp==FPNULL) ++off;
    else {
      if (ptr>=BUFSIZE) flush();
      buf[ptr++]=c;
    }
  }

  // Write buf[0..n-1]
  void write(const char* ibuf, int len) {
    if (fp==FPNULL) off+=len;
    else while (len-->0) put(*ibuf++);
  }

  // Flush output and close
  void close() {
    if (fp!=FPNULL) {
      flush();
      fclose(fp);
    }
    fp=FPNULL;
  }
};

// Create or update an existing archive or part. If filename is ""
// then keep track of position in off but do not write to disk. Otherwise
// open and encrypt with password if not 0. If the file exists then
// read the salt from the first 32 bytes and off_ must be 0. Otherwise
// encrypt assuming off_ previous bytes, of which the first 32 are salt_.
// If off_ is 0 then write salt_ to the first 32 bytes.

OutputArchive::OutputArchive(const char* filename, const char* password,
    const char* salt_, int64_t off_): off(off_), ptr(0) {
  assert(filename);
  if (!*filename) return;

  // Open existing file
  char salt[32]={0};
  fp=fopen(filename, RBPLUS);
  if (isopen()) {
    if (off!=0) error("file exists and off > 0");
    if (password) {
      if (fread(salt, 1, 32, fp)!=32) error("cannot read salt");
      if (salt_ && memcmp(salt, salt_, 32)) error("salt mismatch");
    }
    seek(0, SEEK_END);
  }

  // Create new file
  else {
    fp=fopen(filename, WB);
    if (!isopen()) ioerr(filename);
    if (password) {
      if (!salt_) error("salt not specified");
      memcpy(salt, salt_, 32);
      if (off==0 && fwrite(salt, 1, 32, fp)!=32) ioerr(filename);
    }
  }

  // Set up encryption
  if (password) {
    char key[32];
    libzpaq::stretchKey(key, password, salt);
    aes=new libzpaq::AES_CTR(key, 32, salt);
  }
}

///////////////////////// System info /////////////////////////////////

// Guess number of cores. In 32 bit mode, max is 2.
int numberOfProcessors() {
  int rc=0;  // result
#ifdef unix
#ifdef BSD  // BSD or Mac OS/X
  size_t rclen=sizeof(rc);
  int mib[2]={CTL_HW, HW_NCPU};
  if (sysctl(mib, 2, &rc, &rclen, 0, 0)!=0)
    perror("sysctl");

#else  // Linux
  // Count lines of the form "processor\t: %d\n" in /proc/cpuinfo
  // where %d is 0, 1, 2,..., rc-1
  FILE *in=fopen("/proc/cpuinfo", "r");
  if (!in) return 1;
  std::string s;
  int c;
  while ((c=getc(in))!=EOF) {
    if (c>='A' && c<='Z') c+='a'-'A';  // convert to lowercase
    if (c>' ') s+=c;  // remove white space
    if (c=='\n') {  // end of line?
      if (s.size()>10 && s.substr(0, 10)=="processor:") {
        c=atoi(s.c_str()+10);
        if (c==rc) ++rc;
      }
      s="";
    }
  }
  fclose(in);
#endif
#else

  // In Windows return %NUMBER_OF_PROCESSORS%
  const char* p=getenv("NUMBER_OF_PROCESSORS");
  if (p) rc=atoi(p);
#endif
  if (rc<1) rc=1;
  if (sizeof(char*)==4 && rc>2) rc=2;
  return rc;
}

////////////////////////////// misc ///////////////////////////////////

// For libzpaq output to a string less than 64K chars
struct StringWriter: public libzpaq::Writer {
  string s;
  void put(int c) {
    if (s.size()>=65535) error("string too long");
    s+=char(c);
  }
};

// In Windows convert upper case to lower case.
inline int tolowerW(int c) {
#ifndef unix
  if (c>='A' && c<='Z') return c-'A'+'a';
#endif
  return c;
}

// Return true if strings a == b or a+"/" is a prefix of b
// or a ends in "/" and is a prefix of b.
// Match ? in a to any char in b.
// Match * in a to any string in b.
// In Windows, not case sensitive.
bool ispath(const char* a, const char* b) {
  for (; *a; ++a, ++b) {
    const int ca=tolowerW(*a);
    const int cb=tolowerW(*b);
    if (ca=='*') {
      while (true) {
        if (ispath(a+1, b)) return true;
        if (!*b) return false;
        ++b;
      }
    }
    else if (ca=='?') {
      if (*b==0) return false;
    }
    else if (ca==cb && ca=='/' && a[1]==0)
      return true;
    else if (ca!=cb)
      return false;
  }
  return *b==0 || *b=='/';
}

// Read 4 byte little-endian int and advance s
unsigned btoi(const char* &s) {
  s+=4;
  return (s[-4]&255)|((s[-3]&255)<<8)|((s[-2]&255)<<16)|((s[-1]&255)<<24);
}

// Read 8 byte little-endian int and advance s
int64_t btol(const char* &s) {
  uint64_t r=btoi(s);
  return r+(uint64_t(btoi(s))<<32);
}

/////////////////////////////// Jidac /////////////////////////////////

// A Jidac object represents an archive contents: a list of file
// fragments with hash, size, and archive offset, and a list of
// files with date, attributes, and list of fragment pointers.
// Methods add to, extract from, compare, and list the archive.

// enum for version
static const int64_t DEFAULT_VERSION=99999999999999LL; // unless -until

// fragment hash table entry
struct HT {
  unsigned char sha1[20];  // fragment hash
  int usize;      // uncompressed size, -1 if unknown, -2 if not init
  HT(const char* s=0, int u=-2) {
    if (s) memcpy(sha1, s, 20);
    else memset(sha1, 0, 20);
    usize=u;
  }
};

// filename entry
struct DT {
  int64_t date;          // decimal YYYYMMDDHHMMSS (UT) or 0 if deleted
  int64_t size;          // size or -1 if unknown
  int64_t attr;          // first 8 attribute bytes
  int64_t data;          // sort key or frags written. -1 = do not write
  vector<unsigned> ptr;  // fragment list
  DT(): date(0), size(0), attr(0), data(0) {}
};
typedef map<string, DT> DTMap;

// list of blocks to extract
struct Block {
  int64_t offset;       // location in archive
  int64_t usize;        // uncompressed size, -1 if unknown (streaming)
  int64_t bsize;        // compressed size
  vector<DTMap::iterator> files;  // list of files pointing here
  unsigned start;       // index in ht of first fragment
  unsigned size;        // number of fragments to decompress
  unsigned frags;       // number of fragments in block
  unsigned extracted;   // number of fragments decompressed OK
  enum {READY, WORKING, GOOD, BAD} state;
  Block(unsigned s, int64_t o): offset(o), usize(-1), bsize(0), start(s),
      size(0), frags(0), extracted(0), state(READY) {}
};

// Version info
struct VER {
  int64_t date;          // Date of C block, 0 if streaming
  int64_t lastdate;      // Latest date of any block
  int64_t offset;        // start of transaction C block
  int64_t data_offset;   // start of first D block
  int64_t csize;         // size of compressed data, -1 = no index
  int updates;           // file updates
  int deletes;           // file deletions
  unsigned firstFragment;// first fragment ID
  VER() {memset(this, 0, sizeof(*this));}
};

// Windows API functions not in Windows XP to be dynamically loaded
#ifndef unix
typedef HANDLE (WINAPI* FindFirstStreamW_t)
                   (LPCWSTR, STREAM_INFO_LEVELS, LPVOID, DWORD);
FindFirstStreamW_t findFirstStreamW=0;
typedef BOOL (WINAPI* FindNextStreamW_t)(HANDLE, LPVOID);
FindNextStreamW_t findNextStreamW=0;
#endif

class CompressJob;

// Do everything
class Jidac {
public:
  int doCommand(int argc, const char** argv);
  friend ThreadReturn decompressThread(void* arg);
  friend ThreadReturn testThread(void* arg);
  friend struct ExtractJob;
private:

  // Command line arguments
  char command;             // command 'a', 'x', or 'l'
  string archive;           // archive name
  vector<string> files;     // filename args
  int all;                  // -all option
  bool force;               // -force option
  int fragment;             // -fragment option
  const char* index;        // index option
  char password_string[32]; // hash of -key argument
  const char* password;     // points to password_string or NULL
  string method;            // default "1"
  bool noattributes;        // -noattributes option
  vector<string> notfiles;  // list of prefixes to exclude
  string nottype;           // -not =...
  vector<string> onlyfiles; // list of prefixes to include
  const char* repack;       // -repack output file
  char new_password_string[32]; // -repack hashed password
  const char* new_password; // points to new_password_string or NULL
  int summary;              // summary option if > 0, detailed if -1
  bool dotest;              // -test option
  int threads;              // default is number of cores
  vector<string> tofiles;   // -to option
  int64_t date;             // now as decimal YYYYMMDDHHMMSS (UT)
  int64_t version;          // version number or 14 digit date

  // Archive state
  int64_t dhsize;           // total size of D blocks according to H blocks
  int64_t dcsize;           // total size of D blocks according to C blocks
  vector<HT> ht;            // list of fragments
  DTMap dt;                 // set of files in archive
  DTMap edt;                // set of external files to add or compare
  vector<Block> block;      // list of data blocks to extract
  vector<VER> ver;          // version info

  // Commands
  int add();                // add, return 1 if error else 0
  int extract();            // extract, return 1 if error else 0
  int list();               // list, return 0
  void usage();             // help

  // Support functions
  string rename(string name);           // rename from -to
  int64_t read_archive(const char* arc, int *errors=0);  // read arc
  bool isselected(const char* filename, bool rn=false);// files, -only, -not
  void scandir(string filename);        // scan dirs to dt
  void addfile(string filename, int64_t edate, int64_t esize,
               int64_t eattr);          // add external file to dt
  void list_versions(int64_t csize);    // print ver. csize=archive size
  bool equal(DTMap::const_iterator p, const char* filename);
             // compare file contents with p
};

// Print help message
void Jidac::usage() {
  printf(
"Usage: zpaq command archive[.zpaq] files... -options...\n"
"Files... may be directory trees. Default is the whole archive.\n"
"Use * or \?\?\?\? in archive name for multi-part or \"\" for empty.\n"
"Commands:\n"
"   a  add         Append files to archive if dates have changed.\n"
"   x  extract     Extract most recent versions of files.\n"
"   l  list        List or compare external files to archive by dates.\n"
"Options:\n"
"  -all [N]        Extract/list versions in N [4] digit directories.\n"
"  -f -force       Add: append files if contents have changed.\n"
"                  Extract: overwrite existing output files.\n"
"                  List: compare file contents instead of dates.\n"
"  -index F        Extract: create index F for archive.\n"
"                  Add: create suffix for archive indexed by F, update F.\n"
"  -key X          Create or access encrypted archive with password X.\n"
"  -mN  -method N  Compress level N (0..5 = faster..better, default 1).\n"
"  -noattributes   Ignore/don't save file attributes or permissions.\n"
"  -not files...   Exclude. * and ? match any string or char.\n"
"       =[+-#^?]   List: exclude by comparison result.\n"
"  -only files...  Include only matches (default: *).\n"
"  -repack F [X]   Extract to new archive F with key X (default: none).\n"
"  -sN -summary N  List: show top N sorted by size. -1: show frag IDs.\n"
"                  Add/Extract: if N > 0 show brief progress.\n"
"  -test           Extract: verify but do not write files.\n"
"  -tN -threads N  Use N threads (default: 0 = %d cores).\n"
"  -to out...      Rename files... to out... or all to out/all.\n"
"  -until N        Roll back archive to N'th update or -N from end.\n"
"  -until %s  Set date, roll back (UT, default time: 235959).\n"
#ifndef NDEBUG
"Advanced options:\n"
"  -fragment N     Use 2^N KiB average fragment size (default: 6).\n"
"  -mNB -method NB Use 2^B MiB blocks (0..11, default: 04, 14, 26..56).\n"
"  -method {xs}B[,N2]...[{ciawmst}[N1[,N2]...]]...  Advanced:\n"
"  x=journaling (default). s=streaming (no dedupe).\n"
"    N2: 0=no pre/post. 1,2=packed,byte LZ77. 3=BWT. 4..7=0..3 with E8E9.\n"
"    N3=LZ77 min match. N4=longer match to try first (0=none). 2^N5=search\n"
"    depth. 2^N6=hash table size (N6=B+21: suffix array). N7=lookahead.\n"
"    Context modeling defaults shown below:\n"
"  c0,0,0: context model. N1: 0=ICM, 1..256=CM max count. 1000..1256 halves\n"
"    memory. N2: 1..255=offset mod N2, 1000..1255=offset from N2-1000 byte.\n"
"    N3...: order 0... context masks (0..255). 256..511=mask+byte LZ77\n"
"    parse state, >1000: gap of N3-1000 zeros.\n"
"  i: ISSE chain. N1=context order. N2...=order increment.\n"
"  a24,0,0: MATCH: N1=hash multiplier. N2=halve buffer. N3=halve hash tab.\n"
"  w1,65,26,223,20,0: Order 0..N1-1 word ISSE chain. A word is bytes\n"
"    N2..N2+N3-1 ANDed with N4, hash mulitpiler N5, memory halved by N6.\n"
"  m8,24: MIX all previous models, N1 context bits, learning rate N2.\n"
"  s8,32,255: SSE last model. N1 context bits, count range N2..N3.\n"
"  t8,24: MIX2 last 2 models, N1 context bits, learning rate N2.\n"
#endif
  , threads, dateToString(date).c_str());
  exit(1);
}

// return a/b such that there is exactly one "/" in between, and
// in Windows, any drive letter in b the : is removed and there
// is a "/" after.
string append_path(string a, string b) {
  int na=a.size();
  int nb=b.size();
#ifndef unix
  if (nb>1 && b[1]==':') {  // remove : from drive letter
    if (nb>2 && b[2]!='/') b[1]='/';
    else b=b[0]+b.substr(2), --nb;
  }
#endif
  if (nb>0 && b[0]=='/') b=b.substr(1);
  if (na>0 && a[na-1]=='/') a=a.substr(0, na-1);
  return a+"/"+b;
}

// Rename name using tofiles[]
string Jidac::rename(string name) {
  if (files.size()==0 && tofiles.size()>0)  // append prefix tofiles[0]
    name=append_path(tofiles[0], name);
  else {  // replace prefix files[i] with tofiles[i]
    const int n=name.size();
    for (unsigned i=0; i<files.size() && i<tofiles.size(); ++i) {
      const int fn=files[i].size();
      if (fn<=n && files[i]==name.substr(0, fn))
        return tofiles[i]+name.substr(fn);
    }
  }
  return name;
}

// Parse the command line. Return 1 if error else 0.
int Jidac::doCommand(int argc, const char** argv) {

  // Initialize options to default values
  command=0;
  force=false;
  fragment=6;
  all=0;
  password=0;  // no password
  index=0;
  method="";  // 0..5
  noattributes=false;
  repack=0;
  new_password=0;
  summary=0; // detailed: -1
  dotest=false;  // -test
  threads=0; // 0 = auto-detect
  version=DEFAULT_VERSION;
  date=0;

  printf("zpaq v" ZPAQ_VERSION " journaling archiver, compiled "
         __DATE__ "\n");

  // Init archive state
  ht.resize(1);  // element 0 not used
  ver.resize(1); // version 0
  dhsize=dcsize=0;

  // Get date
  time_t now=time(NULL);
  tm* t=gmtime(&now);
  date=(t->tm_year+1900)*10000000000LL+(t->tm_mon+1)*100000000LL
      +t->tm_mday*1000000+t->tm_hour*10000+t->tm_min*100+t->tm_sec;

  // Get optional options
  for (int i=1; i<argc; ++i) {
    const string opt=argv[i];  // read command
    if ((opt=="add" || opt=="extract" || opt=="list" || opt=="convert"
         || opt=="a" || opt=="x" || opt=="l" || opt=="c")
        && i<argc-1 && argv[i+1][0]!='-' && command==0) {
      command=opt[0];
      if (opt=="extract") command='x';
      archive=argv[++i];  // append ".zpaq" to archive if no extension
      const char* slash=strrchr(argv[i], '/');
      const char* dot=strrchr(slash ? slash : argv[i], '.');
      if (!dot && archive!="") archive+=".zpaq";
      while (++i<argc && argv[i][0]!='-')  // read filename args
        files.push_back(argv[i]);
      --i;
    }
    else if (opt.size()<2 || opt[0]!='-') usage();
    else if (opt=="-all") {
      all=4;
      if (i<argc-1 && isdigit(argv[i+1][0])) all=atoi(argv[++i]);
    }
    else if (opt=="-force" || opt=="-f") force=true;
    else if (opt=="-fragment" && i<argc-1) fragment=atoi(argv[++i]);
    else if (opt=="-index" && i<argc-1) index=argv[++i];
    else if (opt=="-key" && i<argc-1) {
      libzpaq::SHA256 sha256;
      for (const char* p=argv[++i]; *p; ++p) sha256.put(*p);
      memcpy(password_string, sha256.result(), 32);
      password=password_string;
    }
    else if (opt=="-method" && i<argc-1) method=argv[++i];
    else if (opt[1]=='m') method=argv[i]+2;
    else if (opt=="-noattributes") noattributes=true;
    else if (opt=="-not") {  // read notfiles
      while (++i<argc && argv[i][0]!='-') {
        if (argv[i][0]=='=') nottype=argv[i];
        else notfiles.push_back(argv[i]);
      }
      --i;
    }
    else if (opt=="-only") {  // read onlyfiles
      while (++i<argc && argv[i][0]!='-')
        onlyfiles.push_back(argv[i]);
      --i;
    }
    else if (opt=="-repack" && i<argc-1) {
      repack=argv[++i];
      if (i<argc-1 && argv[i+1][0]!='-') {
        libzpaq::SHA256 sha256;
        for (const char* p=argv[++i]; *p; ++p) sha256.put(*p);
        memcpy(new_password_string, sha256.result(), 32);
        new_password=new_password_string;
      }
    }
    else if (opt=="-summary" && i<argc-1) summary=atoi(argv[++i]);
    else if (opt[1]=='s') summary=atoi(argv[i]+2);
    else if (opt=="-test") dotest=true;
    else if (opt=="-to") {  // read tofiles
      while (++i<argc && argv[i][0]!='-')
        tofiles.push_back(argv[i]);
      if (tofiles.size()==0) tofiles.push_back("");
      --i;
    }
    else if (opt=="-threads" && i<argc-1) threads=atoi(argv[++i]);
    else if (opt[1]=='t') threads=atoi(argv[i]+2);
    else if (opt=="-until" && i+1<argc) {  // read date

      // Read digits from multiple args and fill in leading zeros
      version=0;
      int digits=0;
      if (argv[i+1][0]=='-') {  // negative version
        version=atol(argv[i+1]);
        if (version>-1) usage();
        ++i;
      }
      else {  // positive version or date
        while (++i<argc && argv[i][0]!='-') {
          for (int j=0; ; ++j) {
            if (isdigit(argv[i][j])) {
              version=version*10+argv[i][j]-'0';
              ++digits;
            }
            else {
              if (digits==1) version=version/10*100+version%10;
              digits=0;
              if (argv[i][j]==0) break;
            }
          }
        }
        --i;
      }

      // Append default time
      if (version>=19000000LL     && version<=29991231LL)
        version=version*100+23;
      if (version>=1900000000LL   && version<=2999123123LL)
        version=version*100+59;
      if (version>=190000000000LL && version<=299912312359LL)
        version=version*100+59;
      if (version>9999999) {
        if (version<19000101000000LL || version>29991231235959LL) {
          fflush(stdout);
          fprintf(stderr,
            "Version date %1.0f must be 19000101000000 to 29991231235959\n",
             double(version));
          exit(1);
        }
        date=version;
      }
    }
    else {
      printf("Unknown option ignored: %s\n", argv[i]);
      usage();
    }
  }

  // Set threads
  if (threads<1) threads=numberOfProcessors();

  // Test date
  if (now==-1 || date<19000000000000LL || date>30000000000000LL)
    error("date is incorrect, use -until YYYY-MM-DD HH:MM:SS to set");

  // Adjust negative version
  if (version<0) {
    Jidac jidac(*this);
    jidac.version=DEFAULT_VERSION;
    jidac.read_archive(archive.c_str());
    version+=jidac.ver.size()-1;
    printf("Version %1.0f\n", version+.0);
  }

  // Load dynamic functions in Windows Vista and later
#ifndef unix
  HMODULE h=GetModuleHandle(TEXT("kernel32.dll"));
  if (h==NULL) printerr("GetModuleHandle");
  else {
    findFirstStreamW=
      (FindFirstStreamW_t)GetProcAddress(h, "FindFirstStreamW");
    findNextStreamW=
      (FindNextStreamW_t)GetProcAddress(h, "FindNextStreamW");
  }
  if (!findFirstStreamW || !findNextStreamW)
    printf("Alternate streams not supported in Windows XP.\n");
#endif

  // Execute command
  if (command=='a' && files.size()>0) return add();
  else if (command=='x') return extract();
  else if (command=='l') list();
  else usage();
  return 0;
}

/////////////////////////// read_archive //////////////////////////////

// Read arc up to -date into ht, dt, ver. Return place to
// append. If errors is not NULL then set it to number of errors found.
int64_t Jidac::read_archive(const char* arc, int *errors) {
  if (errors) *errors=0;
  dcsize=dhsize=0;
  assert(ver.size()==1);
  unsigned files=0;  // count

  // Open archive
  InputArchive in(arc, password);
  if (!in.isopen()) {
    if (command!='a') {
      fflush(stdout);
      printUTF8(arc, stderr);
      fprintf(stderr, " not found.\n");
      if (errors) ++*errors;
    }
    return 0;
  }
  printUTF8(arc);
  if (version==DEFAULT_VERSION) printf(": ");
  else printf(" -until %1.0f: ", version+0.0);
  fflush(stdout);

  // Test password
  {
    char s[4]={0};
    const int nr=in.read(s, 4);
    if (nr>0 && memcmp(s, "7kSt", 4) && (memcmp(s, "zPQ", 3) || s[3]<1))
      error("password incorrect");
    in.seek(-nr, SEEK_CUR);
  }

  // Scan archive contents
  string lastfile=archive; // last named file in streaming format
  if (lastfile.size()>5 && lastfile.substr(lastfile.size()-5)==".zpaq")
    lastfile=lastfile.substr(0, lastfile.size()-5); // drop .zpaq
  int64_t block_offset=32*(password!=0);  // start of last block of any type
  int64_t data_offset=block_offset;    // start of last block of d fragments
  bool found_data=false;   // exit if nothing found
  bool first=true;         // first segment in archive?
  StringBuffer os(32832);  // decompressed block
  const bool renamed=command=='l' || command=='a';

  // Detect archive format and read the filenames, fragment sizes,
  // and hashes. In JIDAC format, these are in the index blocks, allowing
  // data to be skipped. Otherwise the whole archive is scanned to get
  // this information from the segment headers and trailers.
  bool done=false;
  while (!done) {
    libzpaq::Decompresser d;
    try {
      d.setInput(&in);
      double mem=0;
      while (d.findBlock(&mem)) {
        found_data=true;

        // Read the segments in the current block
        StringWriter filename, comment;
        int segs=0;  // segments in block
        bool skip=false;  // skip decompression?
        while (d.findFilename(&filename)) {
          if (filename.s.size()) {
            for (unsigned i=0; i<filename.s.size(); ++i)
              if (filename.s[i]=='\\') filename.s[i]='/';
            lastfile=filename.s.c_str();
          }
          comment.s="";
          d.readComment(&comment);

          // Test for JIDAC format. Filename is jDC<fdate>[cdhi]<num>
          // and comment ends with " jDC\x01". Skip d (data) blocks.
          if (comment.s.size()>=4
              && comment.s.substr(comment.s.size()-4)=="jDC\x01") {
            if (filename.s.size()!=28 || filename.s.substr(0, 3)!="jDC")
              error("bad journaling block name");
            if (skip) error("mixed journaling and streaming block");

            // Read uncompressed size from comment
            int64_t usize=0;
            unsigned i;
            for (i=0; i<comment.s.size() && isdigit(comment.s[i]); ++i) {
              usize=usize*10+comment.s[i]-'0';
              if (usize>0xffffffff) error("journaling block too big");
            }

            // Read the date and number in the filename
            int64_t fdate=0, num=0;
            for (i=3; i<17 && isdigit(filename.s[i]); ++i)
              fdate=fdate*10+filename.s[i]-'0';
            if (i!=17 || fdate<19000000000000LL || fdate>=30000000000000LL)
              error("bad date");
            for (i=18; i<28 && isdigit(filename.s[i]); ++i)
              num=num*10+filename.s[i]-'0';
            if (i!=28 || num>0xffffffff) error("bad fragment");

            // Decompress the block.
            os.resize(0);
            os.setLimit(usize);
            d.setOutput(&os);
            libzpaq::SHA1 sha1;
            d.setSHA1(&sha1);
            if (strchr("chi", filename.s[17])) {
              if (mem>1.5e9) error("index block requires too much memory");
              d.decompress();
              char sha1result[21]={0};
              d.readSegmentEnd(sha1result);
              if ((int64_t)os.size()!=usize) error("bad block size");
              if (usize!=int64_t(sha1.usize())) error("bad checksum size");
              if (sha1result[0] && memcmp(sha1result+1, sha1.result(), 20))
                error("bad checksum");
            }
            else
              d.readSegmentEnd();

            // Transaction header (type c).
            // If in the future then stop here, else read 8 byte data size
            // from input and jump over it.
            if (filename.s[17]=='c') {
              if (os.size()<8) error("c block too small");
              data_offset=in.tell()+1-d.buffered();
              const char* s=os.c_str();
              int64_t jmp=btol(s);
              if (jmp<0) printf("Incomplete transaction ignored\n");
              if (jmp<0
                  || (version<19000000000000LL && int64_t(ver.size())>version)
                  || (version>=19000000000000LL && version<fdate)) {
                done=true;  // roll back to here
                goto endblock;
              }
              else {
                dcsize+=jmp;
                if (jmp) in.seek(data_offset+jmp, SEEK_SET);
                ver.push_back(VER());
                ver.back().firstFragment=ht.size();
                ver.back().offset=block_offset;
                ver.back().data_offset=data_offset;
                ver.back().date=ver.back().lastdate=fdate;
                ver.back().csize=jmp;
                if (all) {
                  string fn=itos(ver.size()-1, all)+"/";
                  if (renamed) fn=rename(fn);
                  if (isselected(fn.c_str(), false))
                    dt[fn].date=fdate;
                }
                if (jmp) goto endblock;
              }
            }

            // Fragment table (type h).
            // Contents is bsize[4] (sha1[20] usize[4])... for fragment N...
            // where bsize is the compressed block size.
            // Store in ht[].{sha1,usize}. Set ht[].csize to block offset
            // assuming N in ascending order.
            else if (filename.s[17]=='h') {
              assert(ver.size()>0);
              if (fdate>ver.back().lastdate) ver.back().lastdate=fdate;
              if (os.size()%24!=4) error("bad h block size");
              const unsigned n=(os.size()-4)/24;
              if (num<1 || num+n>0xffffffff) error("bad h fragment");
              const char* s=os.c_str();
              const unsigned bsize=btoi(s);
              dhsize+=bsize;
              assert(ver.size()>0);
              if (int64_t(ht.size())>num) {
                fflush(stdout);
                fprintf(stderr,
                  "Unordered fragment tables: expected >= %d found %1.0f\n",
                  int(ht.size()), double(num));
              }
              for (unsigned i=0; i<n; ++i) {
                if (i==0) {
                  block.push_back(Block(num, data_offset));
                  block.back().usize=8;
                  block.back().bsize=bsize;
                  block.back().frags=os.size()/24;
                }
                while (int64_t(ht.size())<=num+i) ht.push_back(HT());
                memcpy(ht[num+i].sha1, s, 20);
                s+=20;
                assert(block.size()>0);
                unsigned f=btoi(s);
                if (f>0x7fffffff) error("fragment too big");
                block.back().usize+=(ht[num+i].usize=f)+4u;
              }
              data_offset+=bsize;
            }

            // Index (type i)
            // Contents is: 0[8] filename 0 (deletion)
            // or:       date[8] filename 0 na[4] attr[na] ni[4] ptr[ni][4]
            // Read into DT
            else if (filename.s[17]=='i') {
              assert(ver.size()>0);
              if (fdate>ver.back().lastdate) ver.back().lastdate=fdate;
              const char* s=os.c_str();
              const char* const end=s+os.size();
              while (s+9<=end) {
                DT dtr;
                dtr.date=btol(s);  // date
                if (dtr.date) ++ver.back().updates;
                else ++ver.back().deletes;
                const int64_t len=strlen(s);
                if (len>65535) error("filename too long");
                string fn=s;  // filename renamed
                if (all) fn=append_path(itos(ver.size()-1, all), fn);
                const bool issel=isselected(fn.c_str(), renamed);
                s+=len+1;  // skip filename
                if (s>end) error("filename too long");
                if (dtr.date) {
                  ++files;
                  if (s+4>end) error("missing attr");
                  unsigned na=btoi(s);  // attr bytes
                  if (s+na>end || na>65535) error("attr too long");
                  for (unsigned i=0; i<na; ++i, ++s)  // read attr
                    if (i<8) dtr.attr+=int64_t(*s&255)<<(i*8);
                  if (noattributes) dtr.attr=0;
                  if (s+4>end) error("missing ptr");
                  unsigned ni=btoi(s);  // ptr list size
                  if (ni>(end-s)/4u) error("ptr list too long");
                  if (issel) dtr.ptr.resize(ni);
                  for (unsigned i=0; i<ni; ++i) {  // read ptr
                    const unsigned j=btoi(s);
                    if (issel) dtr.ptr[i]=j;
                  }
                }
                if (issel) dt[fn]=dtr;
              }  // end while more files
            }  // end if 'i'
            else {
              printf("Skipping %s %s\n",
                  filename.s.c_str(), comment.s.c_str());
              error("Unexpected journaling block");
            }
          }  // end if journaling

          // Streaming format
          else {

            // If previous version does not exist, start a new one
            if (ver.size()==1) {
              if (version<1) {
                done=true;
                goto endblock;
              }
              ver.push_back(VER());
              ver.back().firstFragment=ht.size();
              ver.back().offset=block_offset;
              ver.back().csize=-1;
            }

            char sha1result[21]={0};
            d.readSegmentEnd(sha1result);
            skip=true;
            string fn=lastfile;
            if (all) fn=append_path(itos(ver.size()-1, all), fn);
            if (isselected(fn.c_str(), renamed)) {
              DT& dtr=dt[fn];
              if (filename.s.size()>0 || first) {
                ++files;
                dtr.date=date;
                dtr.attr=0;
                dtr.ptr.resize(0);
                ++ver.back().updates;
              }
              dtr.ptr.push_back(ht.size());
            }
            assert(ver.size()>0);
            if (segs==0 || block.size()==0)
              block.push_back(Block(ht.size(), block_offset));
            assert(block.size()>0);
            ht.push_back(HT(sha1result+1, -1));
          }  // end else streaming
          ++segs;
          filename.s="";
          first=false;
        }  // end while findFilename
        if (!done) block_offset=in.tell()-d.buffered();
      }  // end while findBlock
      done=true;
    }  // end try
    catch (std::exception& e) {
      in.seek(-d.buffered(), SEEK_CUR);
      fflush(stdout);
      fprintf(stderr, "Skipping block at %1.0f: %s\n", double(block_offset),
              e.what());
      if (errors) ++*errors;
    }
endblock:;
  }  // end while !done
  if (in.tell()>32*(password!=0) && !found_data)
    error("archive contains no data");
  printf("%d versions, %u files, %u fragments, %1.6f MB\n", 
      int(ver.size()-1), files, unsigned(ht.size())-1,
      block_offset/1000000.0);

  // Calculate file sizes
  for (DTMap::iterator p=dt.begin(); p!=dt.end(); ++p) {
    for (unsigned i=0; i<p->second.ptr.size(); ++i) {
      unsigned j=p->second.ptr[i];
      if (j>0 && j<ht.size() && p->second.size>=0) {
        if (ht[j].usize>=0) p->second.size+=ht[j].usize;
        else p->second.size=-1;  // unknown size
      }
    }
  }
  return block_offset;
}

// Test whether filename and attributes are selected by files, -only, and -not
// If rn then test renamed filename.
bool Jidac::isselected(const char* filename, bool rn) {
  bool matched=true;
  if (files.size()>0) {
    matched=false;
    for (unsigned i=0; i<files.size() && !matched; ++i) {
      if (rn && i<tofiles.size()) {
        if (ispath(tofiles[i].c_str(), filename)) matched=true;
      }
      else if (ispath(files[i].c_str(), filename)) matched=true;
    }
  }
  if (!matched) return false;
  if (onlyfiles.size()>0) {
    matched=false;
    for (unsigned i=0; i<onlyfiles.size() && !matched; ++i)
      if (ispath(onlyfiles[i].c_str(), filename))
        matched=true;
  }
  if (!matched) return false;
  for (unsigned i=0; i<notfiles.size(); ++i) {
    if (ispath(notfiles[i].c_str(), filename))
      return false;
  }
  return true;
}

// Return the part of fn up to the last slash
string path(const string& fn) {
  int n=0;
  for (int i=0; fn[i]; ++i)
    if (fn[i]=='/' || fn[i]=='\\') n=i+1;
  return fn.substr(0, n);
}

// Insert external filename (UTF-8 with "/") into dt if selected
// by files, onlyfiles, and notfiles. If filename
// is a directory then also insert its contents.
// In Windows, filename might have wildcards like "file.*" or "dir/*"
void Jidac::scandir(string filename) {

  // Don't scan diretories excluded by -not
  for (unsigned i=0; i<notfiles.size(); ++i)
    if (ispath(notfiles[i].c_str(), filename.c_str()))
      return;

#ifdef unix

  // Add regular files and directories
  while (filename.size()>1 && filename[filename.size()-1]=='/')
    filename=filename.substr(0, filename.size()-1);  // remove trailing /
  struct stat sb;
  if (!lstat(filename.c_str(), &sb)) {
    if (S_ISREG(sb.st_mode))
      addfile(filename, decimal_time(sb.st_mtime), sb.st_size,
              'u'+(sb.st_mode<<8));

    // Traverse directory
    if (S_ISDIR(sb.st_mode)) {
      addfile(filename=="/" ? "/" : filename+"/", decimal_time(sb.st_mtime),
              0, 'u'+(int64_t(sb.st_mode)<<8));
      DIR* dirp=opendir(filename.c_str());
      if (dirp) {
        for (dirent* dp=readdir(dirp); dp; dp=readdir(dirp)) {
          if (strcmp(".", dp->d_name) && strcmp("..", dp->d_name)) {
            string s=filename;
            if (s!="/") s+="/";
            s+=dp->d_name;
            scandir(s);
          }
        }
        closedir(dirp);
      }
      else
        perror(filename.c_str());
    }
  }
  else
    perror(filename.c_str());

#else  // Windows: expand wildcards in filename

  // Expand wildcards
  WIN32_FIND_DATA ffd;
  string t=filename;
  if (t.size()>0 && t[t.size()-1]=='/') t+="*";
  HANDLE h=FindFirstFile(utow(t.c_str()).c_str(), &ffd);
  if (h==INVALID_HANDLE_VALUE
      && GetLastError()!=ERROR_FILE_NOT_FOUND
      && GetLastError()!=ERROR_PATH_NOT_FOUND)
    printerr(t.c_str());
  while (h!=INVALID_HANDLE_VALUE) {

    // For each file, get name, date, size, attributes
    SYSTEMTIME st;
    int64_t edate=0;
    if (FileTimeToSystemTime(&ffd.ftLastWriteTime, &st))
      edate=st.wYear*10000000000LL+st.wMonth*100000000LL+st.wDay*1000000
            +st.wHour*10000+st.wMinute*100+st.wSecond;
    const int64_t esize=ffd.nFileSizeLow+(int64_t(ffd.nFileSizeHigh)<<32);
    const int64_t eattr='w'+(int64_t(ffd.dwFileAttributes)<<8);

    // Ignore links, the names "." and ".." or any unselected file
    t=wtou(ffd.cFileName);
    if (ffd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT
        || t=="." || t=="..") edate=0;  // don't add
    string fn=path(filename)+t;

    // Save directory names with a trailing / and scan their contents
    // Otherwise, save plain files
    if (edate) {
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) fn+="/";
      addfile(fn, edate, esize, eattr);
      if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        fn+="*";
        scandir(fn);
      }

      // enumerate alternate streams (Win2003/Vista or later)
      else if (findFirstStreamW && findNextStreamW) {
        WIN32_FIND_STREAM_DATA fsd;
        HANDLE ah=findFirstStreamW(utow(fn.c_str()).c_str(),
            FindStreamInfoStandard, &fsd, 0);
        while (ah!=INVALID_HANDLE_VALUE && findNextStreamW(ah, &fsd))
          addfile(fn+wtou(fsd.cStreamName), edate,
              fsd.StreamSize.QuadPart, eattr);
        if (ah!=INVALID_HANDLE_VALUE) FindClose(ah);
      }
    }
    if (!FindNextFile(h, &ffd)) {
      if (GetLastError()!=ERROR_NO_MORE_FILES) printerr(fn.c_str());
      break;
    }
  }
  FindClose(h);
#endif
}

// Add external file and its date, size, and attributes to dt
void Jidac::addfile(string filename, int64_t edate,
                    int64_t esize, int64_t eattr) {
  if (!isselected(filename.c_str(), false)) return;
  DT& d=edt[filename];
  d.date=edate;
  d.size=esize;
  d.attr=noattributes?0:eattr;
  d.data=0;
}

//////////////////////////////// add //////////////////////////////////

// Append n bytes of x to sb in LSB order
inline void puti(libzpaq::StringBuffer& sb, uint64_t x, int n) {
  for (; n>0; --n) sb.put(x&255), x>>=8;
}

// Print percent done (td/ts) and estimated time remaining
void print_progress(int64_t ts, int64_t td, int sum) {
  if (td>ts) td=ts;
  if (td>=1000000) {
    double eta=0.001*(mtime()-global_start)*(ts-td)/(td+1.0);
    printf("%5.2f%% %d:%02d:%02d ", td*100.0/(ts+0.5),
       int(eta/3600), int(eta/60)%60, int(eta)%60);
    if (sum>0) printf("\r"), fflush(stdout);
  }
}

// A CompressJob is a queue of blocks to compress and write to the archive.
// Each block cycles through states EMPTY, FILLING, FULL, COMPRESSING,
// COMPRESSED, WRITING. The main thread waits for EMPTY buffers and
// fills them. A set of compressThreads waits for FULL threads and compresses
// them. A writeThread waits for COMPRESSED buffers at the front
// of the queue and writes and removes them.

// Buffer queue element
struct CJ {
  enum {EMPTY, FULL, COMPRESSING, COMPRESSED, WRITING} state;
  StringBuffer in;       // uncompressed input
  StringBuffer out;      // compressed output
  string filename;       // to write in filename field
  string comment;        // if "" use default
  string method;         // compression level or "" to mark end of data
  Semaphore full;        // 1 if in is FULL of data ready to compress
  Semaphore compressed;  // 1 if out contains COMPRESSED data
  CJ(): state(EMPTY) {}
};

// Instructions to a compression job
class CompressJob {
public:
  Mutex mutex;           // protects state changes
private:
  int job;               // number of jobs
  CJ* q;                 // buffer queue
  unsigned qsize;        // number of elements in q
  int front;             // next to remove from queue
  libzpaq::Writer* out;  // archive
  Semaphore empty;       // number of empty buffers ready to fill
  Semaphore compressors; // number of compressors available to run
public:
  friend ThreadReturn compressThread(void* arg);
  friend ThreadReturn writeThread(void* arg);
  CompressJob(int threads, int buffers, libzpaq::Writer* f):
      job(0), q(0), qsize(buffers), front(0), out(f) {
    q=new CJ[buffers];
    if (!q) throw std::bad_alloc();
    init_mutex(mutex);
    empty.init(buffers);
    compressors.init(threads);
    for (int i=0; i<buffers; ++i) {
      q[i].full.init(0);
      q[i].compressed.init(0);
    }
  }
  ~CompressJob() {
    for (int i=qsize-1; i>=0; --i) {
      q[i].compressed.destroy();
      q[i].full.destroy();
    }
    compressors.destroy();
    empty.destroy();
    destroy_mutex(mutex);
    delete[] q;
  }      
  void write(StringBuffer& s, const char* filename, string method,
             const char* comment=0);
  vector<int> csize;  // compressed block sizes
};

// Write s at the back of the queue. Signal end of input with method=""
void CompressJob::write(StringBuffer& s, const char* fn, string method,
                        const char* comment) {
  for (unsigned k=(method=="")?qsize:1; k>0; --k) {
    empty.wait();
    lock(mutex);
    unsigned i, j;
    for (i=0; i<qsize; ++i) {
      if (q[j=(i+front)%qsize].state==CJ::EMPTY) {
        q[j].filename=fn?fn:"";
        q[j].comment=comment?comment:"jDC\x01";
        q[j].method=method;
        q[j].in.resize(0);
        q[j].in.swap(s);
        q[j].state=CJ::FULL;
        q[j].full.signal();
        break;
      }
    }
    release(mutex);
    assert(i<qsize);  // queue should not be full
  }
}

// Compress data in the background, one per buffer
ThreadReturn compressThread(void* arg) {
  CompressJob& job=*(CompressJob*)arg;
  int jobNumber=0;
  try {

    // Get job number = assigned position in queue
    lock(job.mutex);
    jobNumber=job.job++;
    assert(jobNumber>=0 && jobNumber<int(job.qsize));
    CJ& cj=job.q[jobNumber];
    release(job.mutex);

    // Work until done
    while (true) {
      cj.full.wait();
      lock(job.mutex);

      // Check for end of input
      if (cj.method=="") {
        cj.compressed.signal();
        release(job.mutex);
        return 0;
      }

      // Compress
      assert(cj.state==CJ::FULL);
      cj.state=CJ::COMPRESSING;
      release(job.mutex);
      job.compressors.wait();
      libzpaq::compressBlock(&cj.in, &cj.out, cj.method.c_str(),
          cj.filename.c_str(), cj.comment=="" ? 0 : cj.comment.c_str());
      cj.in.resize(0);
      lock(job.mutex);
      cj.state=CJ::COMPRESSED;
      cj.compressed.signal();
      job.compressors.signal();
      release(job.mutex);
    }
  }
  catch (std::exception& e) {
    lock(job.mutex);
    fflush(stdout);
    fprintf(stderr, "job %d: %s\n", jobNumber+1, e.what());
    release(job.mutex);
    exit(1);
  }
  return 0;
}

// Write compressed data to the archive in the background
ThreadReturn writeThread(void* arg) {
  CompressJob& job=*(CompressJob*)arg;
  try {

    // work until done
    while (true) {

      // wait for something to write
      CJ& cj=job.q[job.front];  // no other threads move front
      cj.compressed.wait();

      // Quit if end of input
      lock(job.mutex);
      if (cj.method=="") {
        release(job.mutex);
        return 0;
      }

      // Write to archive
      assert(cj.state==CJ::COMPRESSED);
      cj.state=CJ::WRITING;
      job.csize.push_back(cj.out.size());
      if (job.out && cj.out.size()>0) {
        release(job.mutex);
        assert(cj.out.c_str());
        const char* p=cj.out.c_str();
        int64_t n=cj.out.size();
        const int64_t N=1<<30;
        while (n>N) {
          job.out->write(p, N);
          p+=N;
          n-=N;
        }
        job.out->write(p, n);
        lock(job.mutex);
      }
      cj.out.resize(0);
      cj.state=CJ::EMPTY;
      job.front=(job.front+1)%job.qsize;
      job.empty.signal();
      release(job.mutex);
    }
  }
  catch (std::exception& e) {
    fflush(stdout);
    fprintf(stderr, "zpaq exiting from writeThread: %s\n", e.what());
    exit(1);
  }
  return 0;
}

// Write a ZPAQ compressed JIDAC block header. Output size should not
// depend on input data.
void writeJidacHeader(libzpaq::Writer *out, int64_t date,
                      int64_t cdata, unsigned htsize) {
  if (!out) return;
  assert(date>=19000000000000LL && date<30000000000000LL);
  StringBuffer is;
  puti(is, cdata, 8);
  libzpaq::compressBlock(&is, out, "0",
      ("jDC"+itos(date, 14)+"c"+itos(htsize, 10)).c_str(), "jDC\x01");
}

// Maps sha1 -> fragment ID in ht with known size
class HTIndex {
  vector<HT>& htr;  // reference to ht
  libzpaq::Array<unsigned> t;  // sha1 prefix -> index into ht
  unsigned htsize;  // number of IDs in t

  // Compuate a hash index for sha1[20]
  unsigned hash(const char* sha1) {
    return (*(const unsigned*)sha1)&(t.size()-1);
  }

public:
  // r = ht, sz = estimated number of fragments needed
  HTIndex(vector<HT>& r, size_t sz): htr(r), t(0), htsize(1) {
    int b;
    for (b=1; sz*3>>b; ++b);
    t.resize(1, b-1);
    update();
  }

  // Find sha1 in ht. Return its index or 0 if not found.
  unsigned find(const char* sha1) {
    unsigned h=hash(sha1);
    for (unsigned i=0; i<t.size(); ++i) {
      if (t[h^i]==0) return 0;
      if (memcmp(sha1, htr[t[h^i]].sha1, 20)==0) return t[h^i];
    }
    return 0;
  }

  // Update index of ht. Do not index if fragment size is unknown.
  void update() {
    char zero[20]={0};
    while (htsize<htr.size()) {
      if (htsize>=t.size()/4*3) {
        t.resize(t.size(), 1);
        htsize=1;
      }
      if (htr[htsize].usize>=0 && memcmp(htr[htsize].sha1, zero, 20)!=0) {
        unsigned h=hash((const char*)htr[htsize].sha1);
        for (unsigned i=0; i<t.size(); ++i) {
          if (t[h^i]==0) {
            t[h^i]=htsize;
            break;
          }
        }
      }
      ++htsize;
    }
  }    
};

// Sort by sortkey, then by full path
bool compareFilename(DTMap::iterator ap, DTMap::iterator bp) {
  if (ap->second.data!=bp->second.data)
    return ap->second.data<bp->second.data;
  return ap->first<bp->first;
}

// For writing to two archives at once
struct WriterPair: public libzpaq::Writer {
  OutputArchive *a, *b;
  void put(int c) {
    if (a) a->put(c);
    if (b) b->put(c);
  }
  void write(const char* buf, int n) {
    if (a) a->write(buf, n);
    if (b) b->write(buf, n);
  }
  WriterPair(): a(0), b(0) {}
};

// Add or delete files from archive. Return 1 if error else 0.
int Jidac::add() {

  // Read archive or index into ht, dt, ver.
  int errors=0;
  const bool archive_exists=exists(subpart(archive, 1).c_str());
  string arcname=archive;  // input archive name
  if (index) arcname=index;
  int64_t header_pos=0;
  if (exists(subpart(arcname, 1).c_str()))
    header_pos=read_archive(arcname.c_str(), &errors);

  // Set arcname, offset, header_pos, and salt to open out archive
  arcname=archive;  // output file name
  int64_t offset=0;  // total size of existing parts
  char salt[32]={0};  // encryption salt
  if (password) libzpaq::random(salt, 32);

  // Remote archive
  if (index) {
    if (dcsize>0) error("index is a regular archive");
    if (version!=DEFAULT_VERSION) error("cannot truncate with an index");
    offset=header_pos+dhsize;
    header_pos=32*(password && offset==0);
    arcname=subpart(archive, ver.size());
    if (exists(arcname.c_str())) {
      printUTF8(arcname.c_str(), stderr);
      fprintf(stderr, ": archive exists\n");
      error("archive exists");
    }
    if (password) {  // derive archive salt from index
      FP fp=fopen(index, RB);
      if (fp!=FPNULL) {
        if (fread(salt, 1, 32, fp)!=32) error("cannot read salt from index");
        salt[0]^='7'^'z';
        fclose(fp);
      }
    }
  }

  // Local single or multi-part archive
  else {
    int parts=0;  // number of existing parts in multipart
    string part0=subpart(archive, 0);
    if (part0!=archive) {  // multi-part?
      for (int i=1;; ++i) {
        string partname=subpart(archive, i);
        if (partname==part0) error("too many archive parts");
        FP fp=fopen(partname.c_str(), RB);
        if (fp==FPNULL) break;
        ++parts;
        fseeko(fp, 0, SEEK_END);
        offset+=ftello(fp);
        fclose(fp);
      }
      header_pos=32*(password && parts==0);
      arcname=subpart(archive, parts+1);
      if (exists(arcname.c_str())) error("part exists");
    }

    // Get salt from first part if it exists
    if (password) {
      FP fp=fopen(subpart(archive, 1).c_str(), RB);
      if (fp==FPNULL) {
        if (header_pos>32) error("archive first part not found");
        header_pos=32;
      }
      else {
        if (fread(salt, 1, 32, fp)!=32) error("cannot read salt");
        fclose(fp);
      }
    }
  }
  if (exists(arcname.c_str())) printf("Updating ");
  else printf("Creating ");
  printUTF8(arcname.c_str());
  printf(" at offset %1.0f + %1.0f\n", double(header_pos), double(offset));

  // Set method
  if (method=="") method="1";
  if (method.size()==1) {  // set default blocksize
    if (method[0]>='2' && method[0]<='9') method+="6";
    else method+="4";
  }
  if (strchr("0123456789xs", method[0])==0)
    error("-method must begin with 0..5, x, s");
  assert(method.size()>=2);
  if (method[0]=='s' && index) error("cannot index in streaming mode");

  // Set block and fragment sizes
  if (fragment<0) fragment=0;
  const int log_blocksize=20+atoi(method.c_str()+1);
  if (log_blocksize<20 || log_blocksize>31) error("blocksize must be 0..11");
  const unsigned blocksize=(1u<<log_blocksize)-4096;
  const unsigned MAX_FRAGMENT=fragment>19 || (8128u<<fragment)>blocksize-12
      ? blocksize-12 : 8128u<<fragment;
  const unsigned MIN_FRAGMENT=fragment>25 || (64u<<fragment)>MAX_FRAGMENT
      ? MAX_FRAGMENT : 64u<<fragment;

  // Don't mix streaming and journaling
  for (unsigned i=0; i<block.size(); ++i) {
    if (method[0]=='s') {
      if (block[i].usize>=0)
        error("cannot update journaling archive in streaming format");
    }
    else if (block[i].usize<0)
      error("cannot update streaming archive in journaling format");
  }

  // Make list of files to add or delete
  for (unsigned i=0; i<files.size(); ++i)
    scandir(files[i].c_str());

  // Sort the files to be added by filename extension and decreasing size
  vector<DTMap::iterator> vf;
  int64_t total_size=0;  // size of all input
  int64_t total_done=0;  // input deduped so far
  for (DTMap::iterator p=edt.begin(); p!=edt.end(); ++p) {
    DTMap::iterator a=dt.find(rename(p->first));
    if (a!=dt.end()) a->second.data=1;  // keep
    if (p->second.date && p->first!="" && p->first[p->first.size()-1]!='/'
        && (force || a==dt.end()
            || p->second.date!=a->second.date
            || p->second.size!=a->second.size)) {
      total_size+=p->second.size;

      // Key by first 5 bytes of filename extension, case insensitive
      int sp=0;  // sortkey byte position
      for (string::const_iterator q=p->first.begin(); q!=p->first.end(); ++q){
        uint64_t c=*q&255;
        if (c>='A' && c<='Z') c+='a'-'A';
        if (c=='/') sp=0, p->second.data=0;
        else if (c=='.') sp=8, p->second.data=0;
        else if (sp>3) p->second.data+=c<<(--sp*8);
      }

      // Key by descending size rounded to 16K
      int64_t s=p->second.size>>14;
      if (s>=(1<<24)) s=(1<<24)-1;
      p->second.data+=(1<<24)-s-1;
      vf.push_back(p);
    }
  }
  std::sort(vf.begin(), vf.end(), compareFilename);

  // Test for reliable access to archive
  if (archive_exists!=exists(subpart(archive, 1).c_str()))
    error("archive access is intermittent");

  // Open output
  OutputArchive out(arcname.c_str(), password, salt, offset);
  out.seek(header_pos, SEEK_SET);

  // Start compress and write jobs
  vector<ThreadID> tid(threads*2-1);
  ThreadID wid;
  CompressJob job(threads, tid.size(), &out);
  printf(
      "Adding %1.6f MB in %d files -method %s -threads %d at %s.\n",
      total_size/1000000.0, int(vf.size()), method.c_str(), threads,
      dateToString(date).c_str());
  for (unsigned i=0; i<tid.size(); ++i) run(tid[i], compressThread, &job);
  run(wid, writeThread, &job);

  // Append in streaming mode. Each file is a separate block. Large files
  // are split into blocks of size blocksize.
  int64_t dedupesize=0;  // input size after dedupe
  if (method[0]=='s') {
    StringBuffer sb(blocksize+4096-128);
    for (unsigned fi=0; fi<vf.size(); ++fi) {
      DTMap::iterator p=vf[fi];
      print_progress(total_size, total_done, summary);
      if (summary<=0) {
        printf("+ ");
        printUTF8(p->first.c_str());
        printf(" %1.0f\n", p->second.size+0.0);
      }
      FP in=fopen(p->first.c_str(), RB);
      if (in==FPNULL) {
        printerr(p->first.c_str());
        total_size-=p->second.size;
        ++errors;
        continue;
      }
      uint64_t i=0;
      const int BUFSIZE=4096;
      char buf[BUFSIZE];
      while (true) {
        int r=fread(buf, 1, BUFSIZE, in);
        sb.write(buf, r);
        i+=r;
        if (r==0 || sb.size()+BUFSIZE>blocksize) {
          string filename="";
          string comment="";
          if (i==sb.size()) {  // first block?
            filename=rename(p->first);
            comment=itos(p->second.date);
            if ((p->second.attr&255)>0) {
              comment+=" ";
              comment+=char(p->second.attr&255);
              comment+=itos(p->second.attr>>8);
            }
          }
          total_done+=sb.size();
          job.write(sb, filename.c_str(), method, comment.c_str());
          assert(sb.size()==0);
        }
        if (r==0) break;
      }
      fclose(in);
    }

    // Wait for jobs to finish
    job.write(sb, 0, "");  // signal end of input
    for (unsigned i=0; i<tid.size(); ++i) join(tid[i]);
    join(wid);

    // Done
    const int64_t outsize=out.tell();
    printf("%1.0f + (%1.0f -> %1.0f) = %1.0f\n",
        double(header_pos),
        double(total_size),
        double(outsize-header_pos),
        double(outsize));
    out.close();
    return errors>0;
  }  // end if streaming

  // Adjust date to maintain sequential order
  if (ver.size() && ver.back().lastdate>=date) {
    const int64_t newdate=decimal_time(unix_time(ver.back().lastdate)+1);
    fflush(stdout);
    fprintf(stderr, "Warning: adjusting date from %s to %s\n",
      dateToString(date).c_str(), dateToString(newdate).c_str());
    assert(newdate>date);
    date=newdate;
  }

  // Build htinv for fast lookups of sha1 in ht
  HTIndex htinv(ht, ht.size()+(total_size>>(10+fragment))+vf.size());
  const unsigned htsize=ht.size();  // fragments at start of update

  // reserve space for the header block
  writeJidacHeader(&out, date, -1, htsize);
  const int64_t header_end=out.tell();

  // Compress until end of last file
  assert(method!="");
  StringBuffer sb(blocksize+4096-128);  // block to compress
  unsigned frags=0;    // number of fragments in sb
  unsigned redundancy=0;  // estimated bytes that can be compressed out of sb
  unsigned text=0;     // number of fragents containing text
  unsigned exe=0;      // number of fragments containing x86 (exe, dll)
  const int ON=4;      // number of order-1 tables to save
  unsigned char o1prev[ON*256]={0};  // last ON order 1 predictions
  libzpaq::Array<char> fragbuf(MAX_FRAGMENT);
  vector<unsigned> blocklist;  // list of starting fragments

  // For each file to be added
  for (unsigned fi=0; fi<=vf.size(); ++fi) {
    FP in=FPNULL;
    const int BUFSIZE=4096;  // input buffer
    char buf[BUFSIZE];
    int bufptr=0, buflen=0;  // read pointer and limit
    if (fi<vf.size()) {
      assert(vf[fi]->second.ptr.size()==0);
      DTMap::iterator p=vf[fi];

      // Open input file
      bufptr=buflen=0;
      in=fopen(p->first.c_str(), RB);
      if (in==FPNULL) {  // skip if not found
        p->second.date=0;
        total_size-=p->second.size;
        printerr(p->first.c_str());
        ++errors;
        continue;
      }
      p->second.data=1;  // add
    }

    // Read fragments
    int64_t fsize=0;  // file size after dedupe
    for (unsigned fj=0; true; ++fj) {
      int64_t sz=0;  // fragment size;
      unsigned hits=0;  // correct prediction count
      int c=EOF;  // current byte
      unsigned htptr=0;  // fragment index
      char sha1result[20]={0};  // fragment hash
      unsigned char o1[256]={0};  // order 1 context -> predicted byte
      if (fi<vf.size()) {
        int c1=0;  // previous byte
        unsigned h=0;  // rolling hash for finding fragment boundaries
        libzpaq::SHA1 sha1;
        assert(in!=FPNULL);
        while (true) {
          if (bufptr>=buflen) bufptr=0, buflen=fread(buf, 1, BUFSIZE, in);
          if (bufptr>=buflen) c=EOF;
          else c=(unsigned char)buf[bufptr++];
          if (c!=EOF) {
            if (c==o1[c1]) h=(h+c+1)*314159265u, ++hits;
            else h=(h+c+1)*271828182u;
            o1[c1]=c;
            c1=c;
            sha1.put(c);
            fragbuf[sz++]=c;
          }
          if (c==EOF
              || sz>=MAX_FRAGMENT
              || (fragment<=22 && h<(1u<<(22-fragment)) && sz>=MIN_FRAGMENT))
            break;
        }
        assert(sz<=MAX_FRAGMENT);
        total_done+=sz;

        // Look for matching fragment
        assert(uint64_t(sz)==sha1.usize());
        memcpy(sha1result, sha1.result(), 20);
        htptr=htinv.find(sha1result);
      }  // end if fi<vf.size()

      if (htptr==0) {  // not matched or last block

        // Analyze fragment for redundancy, x86, text.
        // Test for text: letters, digits, '.' and ',' followed by spaces
        //   and no invalid UTF-8.
        // Test for exe: 139 (mov reg, r/m) in lots of contexts.
        // 4 tests for redundancy, measured as hits/sz. Take the highest of:
        //   1. Successful prediction count in o1.
        //   2. Non-uniform distribution in o1 (counted in o2).
        //   3. Fraction of zeros in o1 (bytes never seen).
        //   4. Fraction of matches between o1 and previous o1 (o1prev).
        int text1=0, exe1=0;
        int64_t h1=sz;
        unsigned char o1ct[256]={0};  // counts of bytes in o1
        static const unsigned char dt[256]={  // 32768/((i+1)*204)
          160,80,53,40,32,26,22,20,17,16,14,13,12,11,10,10,
            9, 8, 8, 8, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 5, 5,
            4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        for (int i=0; i<256; ++i) {
          if (o1ct[o1[i]]<255) h1-=(sz*dt[o1ct[o1[i]]++])>>15;
          if (o1[i]==' ' && (isalnum(i) || i=='.' || i==',')) ++text1;
          if (o1[i] && (i<9 || i==11 || i==12 || (i>=14 && i<=31) || i>=240))
            --text1;
          if (i>=192 && i<240 && o1[i] && (o1[i]<128 || o1[i]>=192))
            --text1;
          if (o1[i]==139) ++exe1;
        }
        text1=(text1>=3);
        exe1=(exe1>=5);
        if (sz>0) h1=h1*h1/sz; // Test 2: near 0 if random.
        unsigned h2=h1;
        if (h2>hits) hits=h2;
        h2=o1ct[0]*sz/256;  // Test 3: bytes never seen or that predict 0.
        if (h2>hits) hits=h2;
        h2=0;
        for (int i=0; i<256*ON; ++i)  // Test 4: compare to previous o1.
          h2+=o1prev[i]==o1[i&255];
        h2=h2*sz/(256*ON);
        if (h2>hits) hits=h2;
        if (hits>sz) hits=sz;

        // Start a new block if the current block is almost full, or at
        // the start of a file that won't fit or doesn't share mutual
        // information with the current block, or last file.
        bool newblock=false;
        if (frags>0 && fj==0 && fi<vf.size()) {
          const int64_t esize=vf[fi]->second.size;
          const int64_t newsize=sb.size()+esize+(esize>>14)+4096+frags*4;
          if (newsize>blocksize/4 && redundancy<sb.size()/128) newblock=true;
          if (newblock) {  // test for mutual information
            unsigned ct=0;
            for (unsigned i=0; i<256*ON; ++i)
              if (o1prev[i] && o1prev[i]==o1[i&255]) ++ct;
            if (ct>ON*2) newblock=false;
          }
          if (newsize>=blocksize) newblock=true;  // won't fit?
        }
        if (sb.size()+sz+80+frags*4>=blocksize) newblock=true; // full?
        if (fi==vf.size()) newblock=true;  // last file?
        if (frags<1) newblock=false;  // block is empty?

        // Pad sb with fragment size list, then compress
        if (newblock) {
          assert(frags>0);
          assert(frags<ht.size());
          for (unsigned i=ht.size()-frags; i<ht.size(); ++i)
            puti(sb, ht[i].usize, 4);  // list of frag sizes
          puti(sb, 0, 4); // omit first frag ID to make block movable
          puti(sb, frags, 4);  // number of frags
          string m=method;
          if (isdigit(method[0]))
            m+=","+itos(redundancy/(sb.size()/256+1))
                 +","+itos((exe>frags)*2+(text>frags));
          string fn="jDC"+itos(date, 14)+"d"+itos(ht.size()-frags, 10);
          print_progress(total_size, total_done, summary);
          if (summary<=0)
            printf("[%u..%u] %u -method %s\n",
                unsigned(ht.size())-frags, unsigned(ht.size())-1,
                unsigned(sb.size()), m.c_str());
          if (method[0]!='i')
            job.write(sb, fn.c_str(), m.c_str());
          else {  // index: don't compress data
            job.csize.push_back(sb.size());
            sb.resize(0);
          }
          assert(sb.size()==0);
          blocklist.push_back(ht.size()-frags);  // mark block start
          frags=redundancy=text=exe=0;
          memset(o1prev, 0, sizeof(o1prev));
        }

        // Append fragbuf to sb and update block statistics
        assert(sz==0 || fi<vf.size());
        sb.write(&fragbuf[0], sz);
        ++frags;
        redundancy+=hits;
        exe+=exe1*4;
        text+=text1*2;
        if (sz>=MIN_FRAGMENT) {
          memmove(o1prev, o1prev+256, 256*(ON-1));
          memcpy(o1prev+256*(ON-1), o1, 256);
        }
      }  // end if frag not matched or last block

      // Update HT and ptr list
      if (fi<vf.size()) {
        if (htptr==0) {
          htptr=ht.size();
          ht.push_back(HT(sha1result, sz));
          htinv.update();
          fsize+=sz;
        }
        vf[fi]->second.ptr.push_back(htptr);
      }
      if (c==EOF) break;
    }  // end for each fragment fj
    if (fi<vf.size()) {
      dedupesize+=fsize;
      DTMap::iterator p=vf[fi];
      print_progress(total_size, total_done, summary);
      if (summary<=0) {
        string newname=rename(p->first.c_str());
        DTMap::iterator a=dt.find(newname);
        if (a==dt.end() || a->second.date==0) printf("+ ");
        else printf("# ");
        printUTF8(p->first.c_str());
        if (newname!=p->first) {
          printf(" -> ");
          printUTF8(newname.c_str());
        }
        printf(" %1.0f", p->second.size+0.0);
        if (fsize!=p->second.size) printf(" -> %1.0f", fsize+0.0);
        printf("\n");
      }
      assert(in!=FPNULL);
      fclose(in);
      in=FPNULL;
    }
  }  // end for each file fi
  assert(sb.size()==0);

  // Wait for jobs to finish
  job.write(sb, 0, "");  // signal end of input
  for (unsigned i=0; i<tid.size(); ++i) join(tid[i]);
  join(wid);

  // Open index
  salt[0]^='7'^'z';
  OutputArchive outi(index ? index : "", password, salt, 0);
  WriterPair wp;
  wp.a=&out;
  if (index) wp.b=&outi;
  writeJidacHeader(&outi, date, 0, htsize);

  // Append compressed fragment tables to archive
  int64_t cdatasize=out.tell()-header_end;
  StringBuffer is;
  assert(blocklist.size()==job.csize.size());
  blocklist.push_back(ht.size());
  for (unsigned i=0; i<job.csize.size(); ++i) {
    if (blocklist[i]<blocklist[i+1]) {
      puti(is, job.csize[i], 4);  // compressed size of block
      for (unsigned j=blocklist[i]; j<blocklist[i+1]; ++j) {
        is.write((const char*)ht[j].sha1, 20);
        puti(is, ht[j].usize, 4);
      }
      libzpaq::compressBlock(&is, &wp, "0",
          ("jDC"+itos(date, 14)+"h"+itos(blocklist[i], 10)).c_str(),
          "jDC\x01");
      is.resize(0);
    }
  }

  // Delete from archive
  int dtcount=0;  // index block header name
  int removed=0;  // count
  for (DTMap::iterator p=dt.begin(); p!=dt.end(); ++p) {
    if (p->second.date && !p->second.data) {
      puti(is, 0, 8);
      is.write(p->first.c_str(), strlen(p->first.c_str()));
      is.put(0);
      if (summary<=0) {
        printf("- ");
        printUTF8(p->first.c_str());
        printf("\n");
      }
      ++removed;
      if (is.size()>16000) {
        libzpaq::compressBlock(&is, &wp, "1",
            ("jDC"+itos(date)+"i"+itos(++dtcount, 10)).c_str(), "jDC\x01");
        is.resize(0);
      }
    }
  }

  // Append compressed index to archive
  int added=0;  // count
  for (DTMap::iterator p=edt.begin();; ++p) {
    if (p!=edt.end()) {
      string filename=rename(p->first);
      DTMap::iterator a=dt.find(filename);
      if (p->second.date && (a==dt.end() // new file
         || a->second.date!=p->second.date  // date change
         || (a->second.attr && a->second.attr!=p->second.attr)  // attr ch.
         || a->second.size!=p->second.size  // size change
         || (p->second.data && a->second.ptr!=p->second.ptr))) { // content
        if (summary<=0 && p->second.data==0) {  // not compressed?
          if (a==dt.end() || a->second.date==0) printf("+ ");
          else printf("# ");
          printUTF8(p->first.c_str());
          if (filename!=p->first) {
            printf(" -> ");
            printUTF8(filename.c_str());
          }
          printf("\n");
        }
        ++added;
        puti(is, p->second.date, 8);
        is.write(filename.c_str(), strlen(filename.c_str()));
        is.put(0);
        if ((p->second.attr&255)=='u') {  // unix attributes
          puti(is, 3, 4);
          puti(is, p->second.attr, 3);
        }
        else if ((p->second.attr&255)=='w') {  // windows attributes
          puti(is, 5, 4);
          puti(is, p->second.attr, 5);
        }
        else puti(is, 0, 4);  // no attributes
        if (a==dt.end() || p->second.data) a=p;  // use new frag pointers
        puti(is, a->second.ptr.size(), 4);  // list of frag pointers
        for (unsigned i=0; i<a->second.ptr.size(); ++i)
          puti(is, a->second.ptr[i], 4);
      }
    }
    if (is.size()>16000 || (is.size()>0 && p==edt.end())) {
      libzpaq::compressBlock(&is, &wp, "1",
          ("jDC"+itos(date)+"i"+itos(++dtcount, 10)).c_str(), "jDC\x01");
      is.resize(0);
    }
    if (p==edt.end()) break;
  }
  printf("%d +added, %d -removed.\n", added, removed);
  assert(is.size()==0);

  // Back up and write the header
  outi.close();
  int64_t archive_end=out.tell();
  out.seek(header_pos, SEEK_SET);
  writeJidacHeader(&out, date, cdatasize, htsize);
  out.seek(0, SEEK_END);
  int64_t archive_size=out.tell();
  out.close();

  // Truncate empty update from archive (if not indexed)
  if (!index) {
    if (added+removed==0 && archive_end-header_pos==104) // no update
      archive_end=header_pos;
    if (archive_end<archive_size) {
      if (archive_end>0) {
        printf("truncating archive from %1.0f to %1.0f\n",
            double(archive_size), double(archive_end));
        if (truncate(arcname.c_str(), archive_end)) printerr(archive.c_str());
      }
      else if (archive_end==0) {
        if (delete_file(arcname.c_str())) {
          printf("deleted ");
          printUTF8(arcname.c_str());
          printf("\n");
        }
      }
    }
  }
  fflush(stdout);
  fprintf(stderr, "\n%1.6f + (%1.6f -> %1.6f -> %1.6f) = %1.6f MB\n",
      header_pos/1000000.0, total_size/1000000.0, dedupesize/1000000.0,
      (archive_end-header_pos)/1000000.0, archive_end/1000000.0);
  return errors>0;
}

/////////////////////////////// extract ///////////////////////////////

// Return true if the internal file p
// and external file contents are equal or neither exists.
// If filename is 0 then return true if it is possible to compare.
bool Jidac::equal(DTMap::const_iterator p, const char* filename) {

  // test if all fragment sizes and hashes exist
  if (filename==0) {
    static const char zero[20]={0};
    for (unsigned i=0; i<p->second.ptr.size(); ++i) {
      unsigned j=p->second.ptr[i];
      if (j<1 || j>=ht.size()
          || ht[j].usize<0 || !memcmp(ht[j].sha1, zero, 20))
        return false;
    }
    return true;
  }

  // internal or neither file exists
  if (p->second.date==0) return !exists(filename);

  // directories always match
  if (p->first!="" && p->first[p->first.size()-1]=='/')
    return exists(filename);

  // compare sizes
  FP in=fopen(filename, RB);
  if (in==FPNULL) return false;
  fseeko(in, 0, SEEK_END);
  if (ftello(in)!=p->second.size) return fclose(in), false;

  // compare hashes
  fseeko(in, 0, SEEK_SET);
  libzpaq::SHA1 sha1;
  const int BUFSIZE=4096;
  char buf[BUFSIZE];
  for (unsigned i=0; i<p->second.ptr.size(); ++i) {
    unsigned f=p->second.ptr[i];
    if (f<1 || f>=ht.size() || ht[f].usize<0) return fclose(in), false;
    for (int j=0; j<ht[f].usize;) {
      int n=ht[f].usize-j;
      if (n>BUFSIZE) n=BUFSIZE;
      int r=fread(buf, 1, n, in);
      if (r!=n) return fclose(in), false;
      sha1.write(buf, n);
      j+=n;
    }
    if (memcmp(sha1.result(), ht[f].sha1, 20)!=0) return fclose(in), false;
  }
  if (fread(buf, 1, BUFSIZE, in)!=0) return fclose(in), false;
  fclose(in);
  return true;
}

// An extract job is a set of blocks with at least one file pointing to them.
// Blocks are extracted in separate threads, set READY -> WORKING.
// A block is extracted to memory up to the last fragment that has a file
// pointing to it. Then the checksums are verified. Then for each file
// pointing to the block, each of the fragments that it points to within
// the block are written in order.

struct ExtractJob {         // list of jobs
  Mutex mutex;              // protects state
  Mutex write_mutex;        // protects writing to disk
  int job;                  // number of jobs started
  Jidac& jd;                // what to extract
  FP outf;                  // currently open output file
  DTMap::iterator lastdt;   // currently open output file name
  double maxMemory;         // largest memory used by any block (test mode)
  int64_t total_size;       // bytes to extract
  int64_t total_done;       // bytes extracted so far
  ExtractJob(Jidac& j): job(0), jd(j), outf(FPNULL), lastdt(j.dt.end()),
      maxMemory(0), total_size(0), total_done(0) {
    init_mutex(mutex);
    init_mutex(write_mutex);
  }
  ~ExtractJob() {
    destroy_mutex(mutex);
    destroy_mutex(write_mutex);
  }
};

// Decompress blocks in a job until none are READY
ThreadReturn decompressThread(void* arg) {
  ExtractJob& job=*(ExtractJob*)arg;
  int jobNumber=0;

  // Get job number
  lock(job.mutex);
  jobNumber=++job.job;
  release(job.mutex);

  // Open archive for reading
  InputArchive in(job.jd.archive.c_str(), job.jd.password);
  if (!in.isopen()) return 0;
  StringBuffer out;

  // Look for next READY job.
  int next=0;  // current job
  while (true) {
    lock(job.mutex);
    for (unsigned i=0; i<=job.jd.block.size(); ++i) {
      unsigned k=i+next;
      if (k>=job.jd.block.size()) k-=job.jd.block.size();
      if (i==job.jd.block.size()) {  // no more jobs?
        release(job.mutex);
        return 0;
      }
      Block& b=job.jd.block[k];
      if (b.state==Block::READY && b.size>0 && b.usize>=0) {
        b.state=Block::WORKING;
        release(job.mutex);
        next=k;
        break;
      }
    }
    Block& b=job.jd.block[next];

    // Get uncompressed size of block
    unsigned output_size=0;  // minimum size to decompress
    assert(b.start>0);
    for (unsigned j=0; j<b.size; ++j) {
      assert(b.start+j<job.jd.ht.size());
      assert(job.jd.ht[b.start+j].usize>=0);
      output_size+=job.jd.ht[b.start+j].usize;
    }

    // Decompress
    double mem=0;  // how much memory used to decompress
    try {
      assert(b.start>0);
      assert(b.start<job.jd.ht.size());
      assert(b.size>0);
      assert(b.start+b.size<=job.jd.ht.size());
      in.seek(b.offset, SEEK_SET);
      libzpaq::Decompresser d;
      d.setInput(&in);
      out.resize(0);
      assert(b.usize>=0);
      assert(b.usize<=0xffffffffu);
      out.setLimit(b.usize);
      d.setOutput(&out);
      if (!d.findBlock(&mem)) error("archive block not found");
      if (mem>job.maxMemory) job.maxMemory=mem;
      while (d.findFilename()) {
        d.readComment();
        while (out.size()<output_size && d.decompress(1<<14));
        lock(job.mutex);
        print_progress(job.total_size, job.total_done, job.jd.summary);
        if (job.jd.summary<=0)
          printf("[%d..%d] -> %1.0f\n", b.start, b.start+b.size-1,
              out.size()+0.0);
        release(job.mutex);
        if (out.size()>=output_size) break;
        d.readSegmentEnd();
      }
      if (out.size()<output_size) {
        lock(job.mutex);
        fflush(stdout);
        fprintf(stderr, "output [%d..%d] %d of %d bytes\n",
             b.start, b.start+b.size-1, int(out.size()), output_size);
        release(job.mutex);
        error("unexpected end of compressed data");
      }

      // Verify fragment checksums if present
      uint64_t q=0;  // fragment start
      libzpaq::SHA1 sha1;
      assert(b.extracted==0);
      for (unsigned j=b.start; j<b.start+b.size; ++j) {
        assert(j>0 && j<job.jd.ht.size());
        assert(job.jd.ht[j].usize>=0);
        assert(job.jd.ht[j].usize<=0x7fffffff);
        if (q+job.jd.ht[j].usize>out.size())
          error("Incomplete decompression");
        char sha1result[20];
        sha1.write(out.c_str()+q, job.jd.ht[j].usize);
        memcpy(sha1result, sha1.result(), 20);
        q+=job.jd.ht[j].usize;
        if (memcmp(sha1result, job.jd.ht[j].sha1, 20)) {
          lock(job.mutex);
          fflush(stdout);
          fprintf(stderr, "Job %d: fragment %u size %d checksum failed\n",
                 jobNumber, j, job.jd.ht[j].usize);
          release(job.mutex);
          error("bad checksum");
        }
        ++b.extracted;
      }
    }

    // If out of memory, let another thread try
    catch (std::bad_alloc& e) {
      lock(job.mutex);
      fflush(stdout);
      fprintf(stderr, "Job %d killed: %s\n", jobNumber, e.what());
      b.state=Block::READY;
      b.extracted=0;
      out.resize(0);
      release(job.mutex);
      return 0;
    }

    // Other errors: assume bad input
    catch (std::exception& e) {
      lock(job.mutex);
      fflush(stdout);
      fprintf(stderr, "Job %d: skipping [%u..%u] at %1.0f: %s\n",
              jobNumber, b.start+b.extracted, b.start+b.size-1,
              b.offset+0.0, e.what());
      release(job.mutex);
      continue;
    }

    // Write the files in dt that point to this block
    lock(job.write_mutex);
    for (unsigned ip=0; ip<b.files.size(); ++ip) {
      DTMap::iterator p=b.files[ip];
      if (p->second.date==0 || p->second.data<0
          || p->second.data>=int64_t(p->second.ptr.size()))
        continue;  // don't write

      // Look for pointers to this block
      const vector<unsigned>& ptr=p->second.ptr;
      int64_t offset=0;  // write offset
      for (unsigned j=0; j<ptr.size(); ++j) {
        if (ptr[j]<b.start || ptr[j]>=b.start+b.extracted) {
          offset+=job.jd.ht[ptr[j]].usize;
          continue;
        }

        // Close last opened file if different
        if (p!=job.lastdt) {
          if (job.outf!=FPNULL) {
            assert(job.lastdt!=job.jd.dt.end());
            assert(job.lastdt->second.date);
            assert(job.lastdt->second.data
                   <int64_t(job.lastdt->second.ptr.size()));
            fclose(job.outf);
            job.outf=FPNULL;
          }
          job.lastdt=job.jd.dt.end();
        }

        // Open file for output
        if (job.lastdt==job.jd.dt.end()) {
          string filename=job.jd.rename(p->first);
          assert(job.outf==FPNULL);
          if (p->second.data==0) {
            if (!job.jd.dotest) makepath(filename);
            if (job.jd.summary<=0) {
              lock(job.mutex);
              print_progress(job.total_size, job.total_done, job.jd.summary);
              if (job.jd.summary<=0) {
                printf("> ");
                printUTF8(filename.c_str());
                printf("\n");
              }
              release(job.mutex);
            }
            if (!job.jd.dotest) {
              job.outf=fopen(filename.c_str(), WB);
              if (job.outf==FPNULL) {
                lock(job.mutex);
                printerr(filename.c_str());
                release(job.mutex);
              }
#ifndef unix
              else if ((p->second.attr&0x200ff)==0x20000+'w') {  // sparse?
                DWORD br=0;
                if (!DeviceIoControl(job.outf, FSCTL_SET_SPARSE,
                    NULL, 0, NULL, 0, &br, NULL))  // set sparse attribute
                  printerr(filename.c_str());
              }
#endif
            }
          }
          else if (!job.jd.dotest)
            job.outf=fopen(filename.c_str(), RBPLUS);  // update existing file
          if (!job.jd.dotest && job.outf==FPNULL) break;  // skip errors
          job.lastdt=p;
          assert(job.jd.dotest || job.outf!=FPNULL);
        }
        assert(job.lastdt==p);

        // Find block offset of fragment
        uint64_t q=0;  // fragment offset from start of block
        for (unsigned k=b.start; k<ptr[j]; ++k) {
          assert(k>0);
          assert(k<job.jd.ht.size());
          if (job.jd.ht[k].usize<0) error("streaming fragment in file");
          assert(job.jd.ht[k].usize>=0);
          q+=job.jd.ht[k].usize;
        }
        assert(q+job.jd.ht[ptr[j]].usize<=out.size());

        // Combine consecutive fragments into a single write
        assert(offset>=0);
        ++p->second.data;
        uint64_t usize=job.jd.ht[ptr[j]].usize;
        assert(usize<=0x7fffffff);
        assert(b.start+b.size<=job.jd.ht.size());
        while (j+1<ptr.size() && ptr[j+1]==ptr[j]+1
               && ptr[j+1]<b.start+b.size
               && job.jd.ht[ptr[j+1]].usize>=0
               && usize+job.jd.ht[ptr[j+1]].usize<=0x7fffffff) {
          ++p->second.data;
          assert(p->second.data<=int64_t(ptr.size()));
          assert(job.jd.ht[ptr[j+1]].usize>=0);
          usize+=job.jd.ht[ptr[++j]].usize;
        }
        assert(usize<=0x7fffffff);
        assert(q+usize<=out.size());

        // Write the merged fragment unless they are all zeros and it
        // does not include the last fragment.
        uint64_t nz=q;  // first nonzero byte in fragments to be written
        while (nz<q+usize && out.c_str()[nz]==0) ++nz;
        if (!job.jd.dotest && (nz<q+usize || j+1==ptr.size())) {
          fseeko(job.outf, offset, SEEK_SET);
          fwrite(out.c_str()+q, 1, usize, job.outf);
        }
        offset+=usize;
        lock(job.mutex);
        job.total_done+=usize;
        release(job.mutex);

        // Close file. If this is the last fragment then set date and attr.
        // Do not set read-only attribute in Windows yet.
        if (p->second.data==int64_t(ptr.size())) {
          assert(p->second.date);
          assert(job.lastdt!=job.jd.dt.end());
          assert(job.jd.dotest || job.outf!=FPNULL);
          if (!job.jd.dotest) {
            assert(job.outf!=FPNULL);
            string fn=job.jd.rename(p->first);
            int64_t attr=p->second.attr;
            int64_t date=p->second.date;
            if ((p->second.attr&0x1ff)=='w'+256) attr=0;  // read-only?
            if (p->second.data!=int64_t(p->second.ptr.size()))
              date=attr=0;  // not last frag
            close(fn.c_str(), date, attr, job.outf);
            job.outf=FPNULL;
          }
          job.lastdt=job.jd.dt.end();
        }
      } // end for j
    } // end for ip

    // Last file
    release(job.write_mutex);
  } // end while true

  // Last block
  return 0;
}

// Streaming output destination
struct OutputFile: public libzpaq::Writer {
  FP f;
  void put(int c) {
    char ch=c;
    if (f!=FPNULL) fwrite(&ch, 1, 1, f);
  }
  void write(const char* buf, int n) {if (f!=FPNULL) fwrite(buf, 1, n, f);}
  OutputFile(FP out=FPNULL): f(out) {}
};

// Copy at most n bytes from in to out (default all). Return how many copied.
int64_t copy(libzpaq::Reader& in, libzpaq::Writer& out, uint64_t n=~0ull) {
  const unsigned BUFSIZE=4096;
  int64_t result=0;
  char buf[BUFSIZE];
  while (n>0) {
    int nc=n>BUFSIZE ? BUFSIZE : n;
    int nr=in.read(buf, nc);
    if (nr<1) break;
    out.write(buf, nr);
    result+=nr;
    n-=nr;
  }
  return result;
}

// Extract files from archive. If force is true then overwrite
// existing files and set the dates and attributes of exising directories.
// Otherwise create only new files and directories. Return 1 if error else 0.
int Jidac::extract() {

  // Encrypt or decrypt whole archive
  if (repack && all) {
    if (files.size()>0 || tofiles.size()>0 || onlyfiles.size()>0
        || noattributes || version!=DEFAULT_VERSION || method!="")
      error("-repack -all does not allow partial copy");
    InputArchive in(archive.c_str(), password);
    if (force) delete_file(repack);
    if (exists(repack)) error("output file exists");

    // Get key and salt
    char salt[32]={0};
    if (new_password) libzpaq::random(salt, 32);

    // Copy
    OutputArchive out(repack, new_password, salt, 0);
    copy(in, out);
    printUTF8(archive.c_str());
    printf(" %1.0f ", in.tell()+.0);
    printUTF8(repack);
    printf(" -> %1.0f\n", out.tell()+.0);
    out.close();
    return 0;
  }

  // Read archive
  const int64_t sz=read_archive(archive.c_str());
  if (sz<1) error("archive not found");

  // test blocks
  for (unsigned i=0; i<block.size(); ++i) {
    if (block[i].bsize<0) error("negative block size");
    if (block[i].start<1) error("block starts at fragment 0");
    if (block[i].start>=ht.size()) error("block start too high");
    if (i>0 && block[i].start<block[i-1].start) error("unordered frags");
    if (i>0 && block[i].start==block[i-1].start) error("empty block");
    if (i>0 && block[i].offset<block[i-1].offset+block[i-1].bsize)
      error("unordered blocks");
    if (i>0 && block[i-1].offset+block[i-1].bsize>block[i].offset)
      error("overlapping blocks");
  }

  // Create index instead of extract files
  if (index) {
    if (ver.size()<2) error("no journaling data");
    if (force) delete_file(index);
    if (exists(index)) error("index file exists");

    // Get salt
    char salt[32];
    if (ver[1].offset==32) {  // encrypted?
      FP fp=fopen(subpart(archive, 1).c_str(), RB);
      if (fp==FPNULL) error("cannot read part 1");
      if (fread(salt, 1, 32, fp)!=32) error("cannot read salt");
      salt[0]^='7'^'z';  // for index
      fclose(fp);
    }
    InputArchive in(archive.c_str(), password);
    OutputArchive out(index, password, salt, 0);
    for (unsigned i=1; i<ver.size(); ++i) {
      if (in.tell()!=ver[i].offset) error("I'm lost");

      // Read C block. Assume uncompressed and hash is present
      static char hdr[256]={0};  // Read C block
      int hsize=ver[i].data_offset-ver[i].offset;
      if (hsize<70 || hsize>255) error("bad C block size");
      if (in.read(hdr, hsize)!=hsize) error("EOF in header");
      if (hdr[hsize-36]!=9  // size of uncompressed block low byte
          || (hdr[hsize-22]&255)!=253  // start of SHA1 marker
          || (hdr[hsize-1]&255)!=255) {  // end of block marker
        for (int j=0; j<hsize; ++j)
          printf("%d%c", hdr[j]&255, j%10==9 ? '\n' : ' ');
        printf("at %1.0f\n", ver[i].offset+.0);
        error("C block in weird format");
      }
      memcpy(hdr+hsize-34, 
          "\x00\x00\x00\x00\x00\x00\x00\x00"  // csize = 0
          "\x00\x00\x00\x00"  // compressed data terminator
          "\xfd"  // start of hash marker
          "\x05\xfe\x40\x57\x53\x16\x6f\x12\x55\x59\xe7\xc9\xac\x55\x86"
          "\x54\xf1\x07\xc7\xe9"  // SHA-1('0'*8)
          "\xff", 34);  // EOB
      out.write(hdr, hsize);
      in.seek(ver[i].csize, SEEK_CUR);  // skip D blocks
      int64_t end=sz;
      if (i+1<ver.size()) end=ver[i+1].offset;
      int64_t n=end-in.tell();
      if (copy(in, out, n)!=n) error("EOF");  // copy H and I blocks
    }
    printUTF8(index);
    printf(" -> %1.0f\n", out.tell()+.0);
    out.close();
    return 0;
  }

  // Label files to extract with data=0.
  // Skip existing output files. If force then skip only if equal
  // and set date and attributes.
  ExtractJob job(*this);
  int total_files=0, skipped=0;
  for (DTMap::iterator p=dt.begin(); p!=dt.end(); ++p) {
    p->second.data=-1;  // skip
    if (p->second.date && p->first!="") {
      const string fn=rename(p->first);
      const bool isdir=p->first[p->first.size()-1]=='/';
      if (!repack && !dotest && force && !isdir && equal(p, fn.c_str())) {
        if (summary<=0) {  // identical
          printf("= ");
          printUTF8(fn.c_str());
          printf("\n");
        }
        close(fn.c_str(), p->second.date, p->second.attr);
        ++skipped;
      }
      else if (!repack && !dotest && !force && exists(fn)) {  // exists, skip
        if (summary<=0) {
          printf("? ");
          printUTF8(fn.c_str());
          printf("\n");
        }
        ++skipped;
      }
      else if (isdir)  // update directories later
        p->second.data=0;
      else if (block.size()>0) {  // files to decompress
        p->second.data=0;
        unsigned lo=0, hi=block.size()-1;  // block indexes for binary search
        for (unsigned i=0; p->second.data>=0 && i<p->second.ptr.size(); ++i) {
          unsigned j=p->second.ptr[i];  // fragment index
          if (j==0 || j>=ht.size() || ht[j].usize<-1) {
            fflush(stdout);
            printUTF8(p->first.c_str(), stderr);
            fprintf(stderr, ": bad frag IDs, skipping...\n");
            p->second.data=-1;  // skip
            continue;
          }
          assert(j>0 && j<ht.size());
          if (lo!=hi || lo>=block.size() || j<block[lo].start
              || (lo+1<block.size() && j>=block[lo+1].start)) {
            lo=0;  // find block with fragment j by binary search
            hi=block.size()-1;
            while (lo<hi) {
              unsigned mid=(lo+hi+1)/2;
              assert(mid>lo);
              assert(mid<=hi);
              if (j<block[mid].start) hi=mid-1;
              else (lo=mid);
            }
          }
          assert(lo==hi);
          assert(lo>=0 && lo<block.size());
          assert(j>=block[lo].start);
          assert(lo+1==block.size() || j<block[lo+1].start);
          unsigned c=j-block[lo].start+1;
          if (block[lo].size<c) block[lo].size=c;
          if (block[lo].files.size()==0 || block[lo].files.back()!=p)
            block[lo].files.push_back(p);
        }
        ++total_files;
        job.total_size+=p->second.size;
      }
    }  // end if selected
  }  // end for
  if (!force && skipped>0)
    printf("%d ?existing files skipped (-force overwrites).\n", skipped);
  if (force && skipped>0)
    printf("%d =identical files skipped.\n", skipped);

  // Repack to new archive
  if (repack) {

    // Get total D block size
    if (ver.size()<2) error("cannot repack streaming archive");
    int64_t csize=0;  // total compressed size of D blocks
    for (unsigned i=0; i<block.size(); ++i) {
      if (block[i].bsize<1) error("empty block");
      if (block[i].size>0) csize+=block[i].bsize;
    }

    // Open input
    InputArchive in(archive.c_str(), password);

    // Open output
    if (!force && exists(repack)) error("repack output exists");
    delete_file(repack);
    char salt[32]={0};
    if (new_password) libzpaq::random(salt, 32);
    OutputArchive out(repack, new_password, salt, 0);
    int64_t cstart=out.tell();

    // Write C block using first version date
    writeJidacHeader(&out, ver[1].date, -1, 1);
    int64_t dstart=out.tell();

    // Copy only referenced D blocks. If method then recompress.
    for (unsigned i=0; i<block.size(); ++i) {
      if (block[i].size>0) {
        in.seek(block[i].offset, SEEK_SET);
        copy(in, out, block[i].bsize);
      }
    }
    printf("Data %1.0f -> ", csize+.0);
    csize=out.tell()-dstart;
    printf("%1.0f\n", csize+.0);

    // Re-create referenced H blocks using latest date
    for (unsigned i=0; i<block.size(); ++i) {
      if (block[i].size>0) {
        StringBuffer is;
        puti(is, block[i].bsize, 4);
        for (unsigned j=0; j<block[i].frags; ++j) {
          const unsigned k=block[i].start+j;
          if (k<1 || k>=ht.size()) error("frag out of range");
          is.write((const char*)ht[k].sha1, 20);
          puti(is, ht[k].usize, 4);
        }
        libzpaq::compressBlock(&is, &out, "0",
            ("jDC"+itos(ver.back().date, 14)+"h"
            +itos(block[i].start, 10)).c_str(),
            "jDC\x01");
      }
    }

    // Append I blocks of selected files
    unsigned dtcount=0;
    StringBuffer is;
    for (DTMap::iterator p=dt.begin();; ++p) {
      if (p!=dt.end() && p->second.date>0 && p->second.data>=0) {
        string filename=rename(p->first);
        puti(is, p->second.date, 8);
        is.write(filename.c_str(), strlen(filename.c_str()));
        is.put(0);
        if ((p->second.attr&255)=='u') {  // unix attributes
          puti(is, 3, 4);
          puti(is, p->second.attr, 3);
        }
        else if ((p->second.attr&255)=='w') {  // windows attributes
          puti(is, 5, 4);
          puti(is, p->second.attr, 5);
        }
        else puti(is, 0, 4);  // no attributes
        puti(is, p->second.ptr.size(), 4);  // list of frag pointers
        for (unsigned i=0; i<p->second.ptr.size(); ++i)
          puti(is, p->second.ptr[i], 4);
      }
      if (is.size()>16000 || (is.size()>0 && p==dt.end())) {
        libzpaq::compressBlock(&is, &out, "1",
            ("jDC"+itos(ver.back().date)+"i"+itos(++dtcount, 10)).c_str(),
            "jDC\x01");
        is.resize(0);
      }
      if (p==dt.end()) break;
    }

    // Summarize result
    printUTF8(archive.c_str());
    printf(" %1.0f -> ", sz+.0);
    printUTF8(repack);
    printf(" %1.0f\n", out.tell()+.0);

    // Rewrite C block
    out.seek(cstart, SEEK_SET);
    writeJidacHeader(&out, ver[1].date, csize, 1);
    out.close();
    return 0;
  }

  // Decompress archive in parallel
  printf("Extracting %1.6f MB in %d files -threads %d\n",
      job.total_size/1000000.0, total_files, threads);
  vector<ThreadID> tid(threads);
  for (unsigned i=0; i<tid.size(); ++i) run(tid[i], decompressThread, &job);

  // Extract streaming files
  unsigned segments=0;  // count
  InputArchive in(archive.c_str(), password);
  if (in.isopen()) {
    FP outf=FPNULL;
    DTMap::iterator dtptr=dt.end();
    for (unsigned i=0; i<block.size(); ++i) {
      if (block[i].usize<0 && block[i].size>0) {
        Block& b=block[i];
        try {
          in.seek(b.offset, SEEK_SET);
          libzpaq::Decompresser d;
          d.setInput(&in);
          if (!d.findBlock()) error("block not found");
          StringWriter filename;
          for (unsigned j=0; j<b.size; ++j) {
            if (!d.findFilename(&filename)) error("segment not found");
            d.readComment();

            // Start of new output file
            if (filename.s!="" || segments==0) {
              unsigned k;
              for (k=0; k<b.files.size(); ++k) {  // find in dt
                if (b.files[k]->second.ptr.size()>0
                    && b.files[k]->second.ptr[0]==b.start+j
                    && b.files[k]->second.date>0
                    && b.files[k]->second.data==0)
                  break;
              }
              if (k<b.files.size()) {  // found new file
                if (outf!=FPNULL) fclose(outf);
                outf=FPNULL;
                string outname=rename(b.files[k]->first);
                dtptr=b.files[k];
                lock(job.mutex);
                if (summary<=0) {
                  printf("> ");
                  printUTF8(outname.c_str());
                  printf("\n");
                }
                if (!dotest) {
                  makepath(outname);
                  outf=fopen(outname.c_str(), WB);
                  if (outf==FPNULL) printerr(outname.c_str());
                }
                release(job.mutex);
              }
              else {  // end of file
                if (outf!=FPNULL) fclose(outf);
                outf=FPNULL;
                dtptr=dt.end();
              }
            }

            // Decompress segment
            libzpaq::SHA1 sha1;
            d.setSHA1(&sha1);
            OutputFile o(outf);
            d.setOutput(&o);
            d.decompress();

            // Verify checksum
            char sha1result[21];
            d.readSegmentEnd(sha1result);
            if (sha1result[0]==1) {
              if (memcmp(sha1result+1, sha1.result(), 20)!=0)
                error("checksum failed");
            }
            else if (sha1result[0]!=0)
              error("unknown checksum type");
            ++b.extracted;
            if (dtptr!=dt.end()) ++dtptr->second.data;
            filename.s="";
            ++segments;
          }
        }
        catch(std::exception& e) {
          lock(job.mutex);
          printf("Skipping block: %s\n", e.what());
          release(job.mutex);
        }
      }
    }
    if (outf!=FPNULL) fclose(outf);
  }
  if (segments>0) printf("%u streaming segments extracted\n", segments);

  // Wait for threads to finish
  for (unsigned i=0; i<tid.size(); ++i) join(tid[i]);

  // Create empty directories and set file dates and attributes
  if (!dotest) {
    for (DTMap::reverse_iterator p=dt.rbegin(); p!=dt.rend(); ++p) {
      if (p->second.data>=0 && p->second.date && p->first!="") {
        string s=rename(p->first);
        if (p->first[p->first.size()-1]=='/')
          makepath(s, p->second.date, p->second.attr);
        else if ((p->second.attr&0x1ff)=='w'+256)  // read-only?
          close(s.c_str(), 0, p->second.attr);
      }
    }
  }

  // Report failed extractions
  unsigned extracted=0, errors=0;
  for (DTMap::iterator p=dt.begin(); p!=dt.end(); ++p) {
    string fn=rename(p->first);
    if (p->second.data>=0 && p->second.date
        && fn!="" && fn[fn.size()-1]!='/') {
      ++extracted;
      if (p->second.ptr.size()!=unsigned(p->second.data)) {
        fflush(stdout);
        if (++errors==1)
          fprintf(stderr,
          "\nFailed (extracted/total fragments, file):\n");
        fprintf(stderr, "%u/%u ",
                int(p->second.data), int(p->second.ptr.size()));
        printUTF8(fn.c_str(), stderr);
        fprintf(stderr, "\n");
      }
    }
  }
  if (errors>0) {
    fflush(stdout);
    fprintf(stderr,
        "\nExtracted %u of %u files OK (%u errors)"
        " using %1.3f MB x %d threads\n",
        extracted-errors, extracted, errors, job.maxMemory/1000000,
        int(tid.size()));
  }
  return errors>0;
}

/////////////////////////////// list //////////////////////////////////

// Return p<q for sorting files by decreasing size, then fragment ID list
bool compareFragmentList(DTMap::const_iterator p, DTMap::const_iterator q) {
  if (p->second.size!=q->second.size) return p->second.size>q->second.size;
  if (p->second.ptr<q->second.ptr) return true;
  if (q->second.ptr<p->second.ptr) return false;
  if (p->second.data!=q->second.data) return p->second.data<q->second.data;
  return p->first<q->first;
}

// Return p<q for sort by name and comparison result
bool compareName(DTMap::const_iterator p, DTMap::const_iterator q) {
  if (p->first!=q->first) return p->first<q->first;
  return p->second.data<q->second.data;
}

// List contents
int Jidac::list() {

  // Read archive into dt, which may be "" for empty.
  int64_t csize=0;
  if (archive!="") csize=read_archive(archive.c_str());

  // Read external files into edt
  for (unsigned i=0; i<files.size(); ++i)
    scandir(files[i].c_str());
  if (files.size()) printf("%d external files.\n", int(edt.size()));
  printf("\n");

  // Compute directory sizes as the sum of their contents
  DTMap* dp[2]={&dt, &edt};
  for (int i=0; i<2; ++i) {
    for (DTMap::iterator p=dp[i]->begin(); p!=dp[i]->end(); ++p) {
      int len=p->first.size();
      if (len>0 && p->first[len]!='/') {
        for (int j=0; j<len; ++j) {
          if (p->first[j]=='/') {
            DTMap::iterator q=dp[i]->find(p->first.substr(0, j+1));
            if (q!=dp[i]->end())
              q->second.size+=p->second.size;
          }
        }
      }
    }
  }

  // Make list of files to list. List each external file preceded
  // by the matching internal file, if any. Then list any unmatched
  // internal files at the end.
  vector<DTMap::iterator> filelist;
  for (DTMap::iterator p=edt.begin(); p!=edt.end(); ++p) {
    DTMap::iterator a=dt.find(rename(p->first));
    if (a!=dt.end() && (all || a->second.date)) {
      a->second.data='-';
      filelist.push_back(a);
    }
    p->second.data='+';
    filelist.push_back(p);
  }
  for (DTMap::iterator a=dt.begin(); a!=dt.end(); ++a) {
    if (a->second.data!='-' && (all || a->second.date)) {
      a->second.data='-';
      filelist.push_back(a);
    }
  }

  // Sort
  if (summary>0)
    sort(filelist.begin(), filelist.end(), compareFragmentList);

  // List
  int64_t usize=0;
  unsigned matches=0, mismatches=0, internal=0, external=0,
           duplicates=0;  // counts
  for (unsigned fi=0;
       fi<filelist.size() && (summary<=0 || int(fi)<summary); ++fi) {
    DTMap::iterator p=filelist[fi];

    // Compare external files
    if (summary<=0 && p->second.data=='-' && fi+1<filelist.size()
        && filelist[fi+1]->second.data=='+') {
      DTMap::const_iterator p1=filelist[fi+1];
      if ((force && equal(p, p1->first.c_str()))
          || (!force && p->second.date==p1->second.date
              && p->second.size==p1->second.size
              && (!p->second.attr || !p1->second.attr
                  || p->second.attr==p1->second.attr))) {
        p->second.data='=';
        ++fi;
      }
      else
        p->second.data='#';
    }

    // Compare with previous file in summary
    if (summary>0 && fi>0 && p->second.date && p->first!=""
        && p->first[p->first.size()-1]!='/'
        && p->second.ptr.size()
        && filelist[fi-1]->second.ptr==p->second.ptr)
      p->second.data='^';

    if (p->second.data=='=') ++matches;
    if (p->second.data=='#') ++mismatches;
    if (p->second.data=='-') ++internal;
    if (p->second.data=='+') ++external;
    if (p->second.data=='^') ++duplicates;

    // List selected comparison results
    if (!strchr(nottype.c_str(), p->second.data)) {
      if (p->first!="" && p->first[p->first.size()-1]!='/')
        usize+=p->second.size;
      printf("%c %s %12.0f ", char(p->second.data),
          dateToString(p->second.date).c_str(), p->second.size+0.0);
      if (!noattributes)
        printf("%s ", attrToString(p->second.attr).c_str());
      printUTF8(p->first.c_str());
      if (summary<0) {  // frag pointers
        const vector<unsigned>& ptr=p->second.ptr;
        bool hyphen=false;
        for (int j=0; j<int(ptr.size()); ++j) {
          if (j==0 || j==int(ptr.size())-1 || ptr[j]!=ptr[j-1]+1
              || ptr[j]!=ptr[j+1]-1) {
            if (!hyphen) printf(" ");
            hyphen=false;
            printf("%d", ptr[j]);
          }
          else {
            if (!hyphen) printf("-");
            hyphen=true;
          }
        }
      }
      unsigned v;  // list version updates, deletes, compressed size
      if (all>0 && p->first.size()==all+1u && (v=atoi(p->first.c_str()))>0
          && v<ver.size()) {  // version info
        printf(" +%d -%d -> %1.0f", ver[v].updates, ver[v].deletes,
            (v+1<ver.size() ? ver[v+1].offset : csize)-ver[v].offset+0.0);
        if (summary<0)  // print fragment range
          printf(" %u-%u", ver[v].firstFragment,
              v+1<ver.size()?ver[v+1].firstFragment-1:unsigned(ht.size())-1);
      }
      printf("\n");
    }
  }  // end for i = each file version

  // Compute dedupe size
  int64_t ddsize=0, allsize=0;
  unsigned nfiles=0, nfrags=0, unknown_frags=0, refs=0;
  vector<bool> ref(ht.size());
  for (DTMap::const_iterator p=dt.begin(); p!=dt.end(); ++p) {
    if (p->second.date) {
      ++nfiles;
      for (unsigned j=0; j<p->second.ptr.size(); ++j) {
        unsigned k=p->second.ptr[j];
        if (k>0 && k<ht.size()) {
          ++refs;
          if (ht[k].usize>=0) allsize+=ht[k].usize;
          if (!ref[k]) {
            ref[k]=true;
            ++nfrags;
            if (ht[k].usize>=0) ddsize+=ht[k].usize;
            else ++unknown_frags;
          }
        }
      }
    }
  }

  // Print archive statistics
  printf("\n"
      "%1.6f MB of %1.6f MB (%d files) shown\n"
      "  -> %1.6f MB (%u refs to %u of %u frags) after dedupe\n"
      "  -> %1.6f MB compressed.\n",
       usize/1000000.0, allsize/1000000.0, nfiles, 
       ddsize/1000000.0, refs, nfrags, unsigned(ht.size())-1,
       (csize+dhsize-dcsize)/1000000.0);
  if (unknown_frags)
    printf("%d fragments have unknown size\n", unknown_frags);
  if (files.size())
    printf(
       "%d =same, %d #different, %d +external, %d -internal\n",
        matches, mismatches, external, internal);
  if (summary>0)
    printf("%d of largest %d files are ^duplicates\n",
        duplicates, summary);
  if (dhsize!=dcsize)  // index?
    printf("Note: %1.0f of %1.0f compressed bytes are in archive\n",
        dcsize+0.0, dhsize+0.0);
  return 0;
}

#include"td/utils/buffer.h"
#include"td/utils/misc.h"

namespace zpaq {
  td::BufferSlice compress(td::Slice data) {
    int size = td::narrow_cast<int>(data.size());

    StringBuffer in, out;
    in.write(data.data(), size);

    libzpaq::compressBlock(&in, &out, "6", 0, 0, false);

    return td::BufferSlice(out.c_str(), out.size());
  }

  td::BufferSlice decompress(td::Slice data) {
      StringBuffer in, out;
      in.write(data.data(), data.size());
      libzpaq::decompress(&in, &out);
      return td::BufferSlice(out.c_str(), out.size());
  }
}