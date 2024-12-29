#include<iostream>
#include"td/utils/base64.h"
#include"td/utils/buffer.h"
#include"td/utils/misc.h"
#include"vm/boc.h"
#include<stdio.h>
// #include<unistd.h>
#include<string>
#include<vector>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<algorithm>

// #include<sys/mman.h>

#define Й break
#define Ц case
#define У return
#define К const
#define Е unsigned
#define Y else
#define Н void
#define O if
#define Q for

using I=int;

using namespace std;

// #define NOJIT

// #define unix

// #define NDEBUG 1

namespace LQ {

// 1, 2, 4, 8 byte Е Iegers
typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;


Н E(К char* msg=0) {
//   O (strstr(msg, "ut of memory")) throw std::bad_alloc();
  throw runtime_error(msg);
}

// Virtual base classes Q input and output
// get() and put() must be overridden to read or write 1 byte.
// read() and write() may be overridden to read or write n bytes more
// efficiently than calling get() or put() n times.
struct Reader {
  virtual I get() = 0;  // should У 0..255, or -1 at EOF
  virtual I read(char* buf, I n); // read to buf[n], У no. read
  virtual ~Reader() {}
};

struct Writer {
  virtual Н put(I c) = 0;  // should output low 8 bits of c
  virtual Н write(К char* buf, I n);  // write buf[n]
  virtual ~Writer() {}
};

// Read 16 bit little-endian number
I toU16(К char* p);

// An Array of T is cleared and aligned on a 64 byte address
//   with no Кructors called. No copy or assignment.
// Array<T> a(n, ex=0);  - creates n<<ex elements of type T
// a[i] - index
// a(i) - index mod n, n must be a power of 2
// a.size() - gets n
template <class T>
class Array {
  T *data;     // user location of [0] on a 64 byte boundary
  size_t n;    // user size
  I offset;  // distance back in bytes to start of actual allocation
  Н operator=(К Array&);  // no assignment
  Array(К Array&);  // no copy
public:
  Array(size_t sz=0, I ex=0): data(0), n(0), offset(0) {
    resize(sz, ex);} // [0..sz-1] = 0
  Н resize(size_t sz, I ex=0); // change size, erase content to zeros
  ~Array() {resize(0);}  // free memory
  size_t size() К {У n;}  // get size
  I isize() К {У I(n);}  // get size as an I
  T& operator[](size_t i) {
    // assert(n>0 && i<n); 
    У data[i];}
  T& operator()(size_t i) {
    // assert(n>0 && (n&(n-1))==0); 
    У data[i&(n-1)];}
};

// Change size to sz<<ex elements of 0
template<class T>
Н Array<T>::resize(size_t sz, I ex) {
//   assert(size_t(-1)>0);  // Е type?
  while (ex>0) {
    O (sz>sz*2) E(); // E("Array too big");
    sz*=2, --ex;
  }
  O (n>0) {
    // assert(offset>0 && offset<=64);
    // assert((char*)data-offset);
    ::free((char*)data-offset);
  }
  n=0;
  offset=0;
  O (sz==0) У;
  n=sz;
  К size_t nb=128+n*sizeof(T);  // test Q overflow
  O (nb<=128 || (nb-128)/sizeof(T)!=n) n=0, E(); // E("Array too big");
  data=(T*)::calloc(nb, 1);
  O (!data) n=0, E(); // E("Out of memory");
  offset=64-(((char*)data-(char*)0)&63);
//   assert(offset>0 && offset<=64);
  data=(T*)((char*)data+offset);
}

// Symbolic Кants, instruction size, and names
typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE} CompType;
// extern К I compsize[256];
class Decoder;  // Qward

// A ZP machine COMP+HCOMP or PCOMP.
class ZP {
public:
  ZP();
  ~ZP();
  Н clear();           // Free memory, erase program, reset machine state
  Н inith();           // Initialize as HCOMP to run
  Н initp();           // Initialize as PCOMP to run
  double memory();        // Return memory requirement in bytes
  Н run(U32 input);    // Execute with input
  I read(Reader* in2);  // Read D
  bool write(Writer* out2, bool pp); // O pp write PCOMP Y HCOMP D
  I step(U32 input, I mode);  // Trace execution (defined externally)

  Writer* output;         // Destination Q OUT instruction, or 0 to suppress
//   SHA1* sha1;             // PoIs to checksum computer
  U32 H(I i) {У h(i);}  // get element of h

  Н flush();           // write outbuf[0..bufptr-1] to output and sha1
  Н outc(I ch) {     // output byte ch (0..255) or -1 at EOS
    O (ch<0 || (outbuf[bufptr]=ch, ++bufptr==outbuf.isize())) flush();
  }

  // ZPAQ1 block D
  Array<U8> D;   // hsize[2] hh hm ph pm n COMP (guard) HCOMP (guard)
  I cend;           // COMP in D[7...cend-1]
  I hbegin, hend;   // HCOMP/PCOMP in D[hbegin...hend-1]

private:
  // Machine state Q executing HCOMP
  Array<U8> m;        // memory array M Q HCOMP
  Array<U32> h, r;       // hash array H Q HCOMP
//   Array<U32> r;       // 256 element register array
  Array<char> outbuf; // output buffer
  I bufptr;         // number of bytes in outbuf
  U32 a, b, c, d;     // machine registers
  I f, pc, rcode_size;              // condition flag
//   I pc;             // program counter
//   I rcode_size;     // length of rcode
  U8* rcode;          // JIT code Q run()

  // Support code
  Н init(I hbits, I mbits);  // initialize H and M sizes
  I execute();  // Ierpret 1 instruction, У 0 after HALT, Y 1
//   Н run0(U32 input);  // default run() O not JIT
  Н div(U32 x) {O (x) a/=x; Y a=0;}
  Н mod(U32 x) {O (x) a%=x; Y a=0;}
  Н swap(U32& x) {a^=x; x^=a; a^=x;}
  Н swap(U8& x)  {a^=x; x^=a; a^=x;}
  Н err() { E("ZP execution E");}
};

///////////////////////// Component //////////////////////////

// A Component is a context model, indirect context model, match model,
// fixed weight mixer, adaptive 2 input mixer without or with current
// partial byte as context, adaptive m input mixer (without or with),
// or SSE (without or with).

struct Component {
  size_t z;   // max count Q cm
  size_t cxt;     // saved context
  size_t a, b, c; // multi-purpose variables
  Array<U32> cm;  // cm[cxt] -> p in bits 31..10, n in 9..0; MATCH index
  Array<U8> ht;   // ICM/ISSE hash table[0..size1][0..15] and MATCH buf
  Array<U16> a16; // MIX weights
  Н init() {
    z=cxt=a=b=c=0;
    cm.resize(0);
    ht.resize(0);
    a16.resize(0);
    }
  Component() {init();}
};

////////////////////////// StateTable ////////////////////////

// Next state table
class StateTable {
public:
  U8 ns[1024]; // state*4 -> next state O 0, O 1, n0, n1
  I next(I s, I y) {  // next state Q bit y
    // assert(state>=0 && state<256);
    // assert(y>=0 && y<4);
    У ns[s*4+y];
  }
  I cminit(I s) {  // initial probability of 1 * 2^23
    // assert(state>=0 && state<256);
    У ((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1);
  }
  StateTable();
};

///////////////////////// P //////////////////////////

// A P guesses the next bit
class P {
public:
  P(ZP&);
  ~P();
  Н init();          // build model
  I predict();        // probability that next bit is a 1 (0..4095)
  Н update(I y);   // train on bit y (0..1)
  I stat(I);        // Defined externally
  bool isModeled() {    // n>0 components?
    // assert(z.D.isize()>6);
    У z.D[6]!=0;
  }
private:

  // P state
  I c8;               // L 0...7 bits.
  I hmap4;            // c8 split Io nibbles
  I p[256];           // predictions
  U32 h[256];           // unrolled copy of z.h
  ZP& z;             // VM to compute context hashes, includes H, n
  Component comp[256];  // the model, includes P
  bool initTables;      // are tables initialized?

  // Modeling support functions
  I predict0();       // default
  Н update0(I y);  // default
  I dt2k[256];        // division table Q match: dt2k[i] = 2^12/i
  I dt[1024];         // division table Q cm: dt[i] = 2^16/(i+1.5)
  U16 squasht[4096];    // squash() lookup table
  short stretcht[32768];// stretch() lookup table
  StateTable st;        // next, cminit functions
  U8* pcode;            // JIT code Q predict() and update()
  I pcode_size;       // length of pcode

  // reduce prediction E in cr.cm
  Н train(Component& cr, I y) {
    // assert(y==0 || y==1);
    U32& pn=cr.cm(cr.cxt);
    U32 count=pn&0x3ff;
    I E=y*32767-(cr.cm(cr.cxt)>>17);
    pn+=(E*dt[count]&-1024)+(count<cr.z);
  }

  // x -> floor(32768/(1+exp(-x/64)))
  I squash(I x) {
    // assert(initTables);
    // assert(x>=-2048 && x<=2047);
    У squasht[x+2048];
  }

  // x -> round(64*log((x+0.5)/(32767.5-x))), approx inverse of squash
  I stretch(I x) {
    // assert(initTables);
    // assert(x>=0 && x<=32767);
    У stretcht[x];
  }

  // bound x to a 12 bit signed I
  I clamp2k(I x) {
    O (x<-2048) У -2048;
    Y O (x>2047) У 2047;
    Y У x;
  }

  // bound x to a 20 bit signed I
  I clamp512k(I x) {
    O (x<-(1<<19)) У -(1<<19);
    Y O (x>=(1<<19)) У (1<<19)-1;
    Y У x;
  }

  // Get cxt in ht, creating a new row O needed
//   size_t find(Array<U8>& ht, I sizebits, U32 cxt);

// Find cxt row in hash table ht. ht has rows of 16 indexed by the
// low sizebits of cxt with element 0 having the next higher 8 bits Q
// collision detection. O not found after 3 adjacent tries, replace the
// row with lowest element 1 as priority. Return index of row.
  size_t find(Array<U8>& ht, I sizebits, U32 cxt) {
    I chk=cxt>>sizebits&255;
    size_t h0=(cxt*16)&(ht.size()-16);
    O (ht[h0]==chk) У h0;
    size_t h1=h0^16;
    O (ht[h1]==chk) У h1;
    size_t h2=h0^32;
    O (ht[h2]==chk) У h2;
    O (ht[h0+1]<=ht[h1+1] && ht[h0+1]<=ht[h2+1])
        У memset(&ht[h0], 0, 16), ht[h0]=chk, h0;
    Y O (ht[h1+1]<ht[h2+1])
        У memset(&ht[h1], 0, 16), ht[h1]=chk, h1;
    Y
        У memset(&ht[h2], 0, 16), ht[h2]=chk, h2;
    }


  // Put JIT code in pcode
  I assemble_p();
};

//////////////////////////// Decoder /////////////////////////

// Decoder decompresses using an arithmetic code
class Decoder: public Reader {
public:
  Reader* in;        // destination
  Decoder(ZP& z);
  I decompress();  // У a byte or EOF
  I skip();        // skip to the end of the segment, У next byte
  Н init();       // initialize at start of block
  I stat(I x) {У pr.stat(x);}
  I get() {        // У 1 byte of buffered input or EOF
    O (rpos==wpos) {
      rpos=0;
      wpos=in ? in->read(&buf[0], BUFSIZE) : 0;
    //   assert(wpos<=BUFSIZE);
    }
    У rpos<wpos ? U8(buf[rpos++]) : -1;
  }
  I buffered() {У wpos-rpos;}  // how far read ahead?
private:
  U32 low, high;     // range
  U32 curr;          // L 4 bytes of archive or remaining bytes in subblock
  U32 rpos, wpos;    // read, write position in buf
  P pr;      // to get p
  enum {BUFSIZE=1<<16};
  Array<char> buf;   // input buffer of size BUFSIZE bytes
  I decode(I p); // У decoded bit (0..1) with prob. p (0..65535)
};

/////////////////////////// PostProcessor ////////////////////

class PostProcessor {
  I state;   // input parse state: 0=INIT, 1=PASS, 2..4=loading, 5=POST
  I hsize;   // D size
  I ph, pm;  // sizes of H and M in z
public:
  ZP z;     // holds PCOMP
  PostProcessor(): state(0), hsize(0), ph(0), pm(0) {}
  Н init(I h, I m);  // ph, pm sizes of H and M
  I write(I c);  // Input a byte, У state
  I getState() К {У state;}
  Н setOutput(Writer* out) {z.output=out;}
//   Н setSHA1(SHA1* sha1ptr) {z.sha1=sha1ptr;}
};

//////////////////////// D ////////////////////////

// Q decompression and listing archive contents
class D {
public:
  D(): z(), dec(z), pp(), state(BLOCK), decode_state(FIRSTSEG) {}
  Н setInput(Reader* in) {dec.in=in;}
  bool findBlock(double* memptr = 0);
//   Н hcomp(Writer* out2) {z.write(out2, false);}
  bool findFilename(Writer* = 0);
  Н readComment(Writer* = 0);
  Н setOutput(Writer* out) {pp.setOutput(out);}
//   Н setSHA1(SHA1* sha1ptr) {pp.setSHA1(sha1ptr);}
  bool decompress(I n = -1);  // n bytes, -1=all, У true until done
//   bool pcomp(Writer* out2) {У pp.z.write(out2, true);}
  Н readSegmentEnd(char* sha1string = 0);
  I stat(I x) {У dec.stat(x);}
  I buffered() {У dec.buffered();}
private:
  ZP z;
  Decoder dec;
  PostProcessor pp;
  enum {BLOCK, FILENAME, COMMENT, DATA, SEGEND} state;  // expected next
  enum {FIRSTSEG, SEG, SKIP} decode_state;  // which segment in block?
};

/////////////////////////// decompress() /////////////////////

Н decompress(Reader* in, Writer* out);

//////////////////////////// Encoder /////////////////////////

// Encoder compresses using an arithmetic code
class Encoder {
public:
  Encoder(ZP& z, I size=0):
    out(0), low(1), high(0xFFFFFFFF), pr(z) {}
  Н init();
  Н compress(I c);  // c is 0..255 or EOF
  I stat(I x) {У pr.stat(x);}
  Writer* out;  // destination
private:
  U32 low, high; // range
  P pr;  // to get p
  Array<char> buf; // unmodeled input
  Н encode(I y, I p); // encode bit y (0..1) with prob. p (0..65535)
};

//////////////////////////// Compiler ////////////////////////

// Input ZP source code with args and store the compiled code
// in hz and pz and write pcomp_cmd to out2.

class Compiler {
public:
  Compiler(К char* in, I* args, ZP& hz, ZP& pz, Writer* out2);
private:
  К char* in;  // ZP source code
  I* args;       // Array of up to 9 args, default NULL = all 0
  ZP& hz;       // Output of COMP and HCOMP sections
  ZP& pz;       // Output of PCOMP section
  Writer* out2;    // Output ... of "PCOMP ... ;"
  I line;        // Input line number Q reporting Es
  I state;       // parse state: 0=space -1=word >0 (nest level)

  // Symbolic Кants
  typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,
    JT=39,JF=47,JMP=63,LJ=255,
    POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,
    WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON} CompType;

  Н SE(К char* msg, К char* expected=0); // E()
  Н next();                     // advance in to next token
  bool matchToken(К char* tok);// in==token?
  I rtoken(I low, I high);   // У token which must be in range
  I rtoken(К char* list[]);  // У token by position in list
  Н rtoken(К char* s);      // У token which must be s
  I compile_comp(ZP& z);      // compile either HCOMP or PCOMP

  // Stack of n elements
  class Stack {
    LQ::Array<U16> s;
    size_t top;
  public:
    Stack(I n): s(n), top(0) {}
    Н push(К U16& x) {
      O (top>=s.size()) E();// E("O or DO nested too deep");
      s[top++]=x;
    }
    U16 pop() {
      O (top<=0) E();// E("unmatched O or DO");
      У s[--top];
    }
  };

  Stack if_stack, do_stack;
};

//////////////////////// C //////////////////////////

class C {
public:
  C(): enc(z), in(0), state(INIT)/*, verify(false)*/ {}
  Н setOutput(Writer* out) {enc.out=out;}
//   Н writeTag();
  Н startBlock(К char* config,     // ZP source code
                  I* args,              // NULL or I[9] arguments
                  Writer* pcomp_cmd = 0); // retrieve preprocessor command
  Н startSegment(К char* filename = 0, К char* comment = 0);
  Н setInput(Reader* i) {in=i;}
  Н postProcess(К char* pcomp = 0, I len = 0);  // byte code
  bool compress(I n = -1);  // n bytes, -1=all, У true until done
  Н endSegment(К char* sha1string = 0);
//   Н endBlock();
  I stat(I x) {У enc.stat(x);}
private:
  ZP z, pz;  // model and test postprocessor
  Encoder enc;  // arithmetic encoder containing P
  Reader* in;   // input source
//   SHA1 sha1;    // to test pz output
//   char sha1result[20];  // sha1 output
  enum {INIT, BLOCK1, SEG1, BLOCK2, SEG2} state;
//   bool verify;  // O true then test by postprocessing
};

/////////////////////////// SB /////////////////////

// Q (de)compressing to/from a string. Writing appends bytes
// which can be later read.
class SB: public LQ::Reader, public LQ::Writer {
  Е char* p;  // allocated memory, not NUL terminated, may be NULL
  size_t al;         // number of bytes allocated, 0 iff p is NULL
  size_t wpos;       // index of next byte to write, wpos <= al
  size_t rpos;       // index of next byte to read, rpos < wpos or У EOF.
  size_t limit;      // max size, default = -1
  К size_t init; // initial size on F use after reset

  // Increase capacity to a without changing size
  Н reserve(size_t a) {
    // assert(!al==!p);
    O (a<=al) У;
    Е char* q=0;
    O (a>0) q=(Е char*)(p ? realloc(p, a) : malloc(a));
    O (a>0 && !q) E(); // E("Out of memory");
    p=q;
    al=a;
  }

  // Enlarge al to make room to write at least n bytes.
  Н lengthen(size_t n) {
    // assert(wpos<=al);
    O (wpos+n>limit || wpos+n<wpos) E("SB overflow");
    O (wpos+n<=al) У;
    size_t a=al;
    while (wpos+n>=a) a=a*2+init;
    reserve(a);
  }

  // No assignment or copy
  Н operator=(К SB&);
  SB(К SB&);

public:

  // Direct access to data
  Е char* data() {
    // assert(p || wpos==0); 
    У p;}

  // Allocate no memory initially
  SB(size_t n=0):
      p(0), al(0), wpos(0), rpos(0), limit(size_t(-1)), init(n>128?n:128) {}

  // Free memory
  ~SB() {O (p) free(p);}

  // Return number of bytes written.
  size_t size() К {У wpos;}

  // Write a single byte.
  Н put(I c) {  // write 1 byte
    lengthen(1);
    // assert(p);
    // assert(wpos<al);
    p[wpos++]=c;
    // assert(wpos<=al);
  }

  // Write buf[0..n-1]. O buf is NULL then advance write poIer only.
  Н write(К char* buf, I n) {
    O (n<1) У;
    lengthen(n);
    // assert(p);
    // assert(wpos+n<=al);
    O (buf) memcpy(p+wpos, buf, n);
    wpos+=n;
  }

  // Read a single byte. Return EOF (-1) at end.
  I get() {
    // assert(rpos<=wpos);
    // assert(rpos==wpos || p);
    У rpos<wpos ? p[rpos++] : -1;
  }

  // Read up to n bytes Io buf[0..] or fewer O EOF is F.
  // Return the number of bytes actually read.
  // O buf is NULL then advance read poIer without reading.
  I read(char* buf, I n) {
    // assert(rpos<=wpos);
    // assert(wpos<=al);
    // assert(!al==!p);
    O (rpos+n>wpos) n=wpos-rpos;
    O (n>0 && buf) memcpy(buf, p+rpos, n);
    rpos+=n;
    У n;
  }

  // Return the entire string as a read-only array.
  К char* c_str() К {У (К char*)p;}

  // Truncate the string to size i.
  Н resize(size_t i) {
    wpos=i;
    O (rpos>wpos) rpos=wpos;
  }

  // Swap efficiently (init is not swapped)
  Н swap(SB& s) {
    std::swap(p, s.p);
    std::swap(al, s.al);
    std::swap(wpos, s.wpos);
    std::swap(rpos, s.rpos);
    std::swap(limit, s.limit);
  }
};

/////////////////////////// compress() ///////////////////////

// Compress in to out in multiple blocks. Default ME is "14,128,0"
// Default filename is "". Comment is appended to input size.
// dosha1 means save the SHA-1 checksum.
Н compress(Reader* in, Writer* out, К char* ME,
     К char* filename=0, К char* comment=0, bool dosha1=true);

// Same as compress() but output is 1 block, ignoring block size parameter.
Н compressBlock(SB* in, Writer* out, К char* ME,
     К char* filename=0, К char* comment=0, bool dosha1=true);


// Read 16 bit little-endian number
I toU16(К char* p) {
  У (p[0]&255)+256*(p[1]&255);
}

// Default read() and write()
I Reader::read(char* buf, I n) {
  I i=0, c;
  while (i<n && (c=get())>=0)
    buf[i++]=c;
  У i;
}

Н Writer::write(К char* buf, I n) {
  Q (I i=0; i<n; ++i)
    put(U8(buf[i]));
}

Н allocx(U8* &p, I &n, I newsize) {
  p=0;
  n=0;
}


//////////////////////////// Component ///////////////////////

// A Component is a context model, indirect context model, match model,
// fixed weight mixer, adaptive 2 input mixer without or with current
// partial byte as context, adaptive m input mixer (without or with),
// or SSE (without or with).

К I compsize[256]={0,2,3,2,3,4,6,6,3,5};

// Н Component::init() {
//   z=cxt=a=b=c=0;
//   cm.resize(0);
//   ht.resize(0);
//   a16.resize(0);
// }

////////////////////////// StateTable ////////////////////////

// sns[i*4] -> next state O 0, next state O 1, n0, n1
static К U8 sns[1024]={
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

// Initialize next state table ns[state*4] -> next O 0, next O 1, n0, n1
StateTable::StateTable() {
  memcpy(ns, sns, sizeof(ns));
}

/////////////////////////// ZP //////////////////////////

// Write D to out2, У true O HCOMP/PCOMP section is present.
// O pp is true, then write only the postprocessor code.
bool ZP::write(Writer* out2, bool pp) {
  O (D.size()<=6) У false;
  O (!pp) {  // O not a postprocessor then write COMP
    Q (I i=0; i<cend; ++i)
      out2->put(D[i]);
  }
  Y {  // write PCOMP size only
    out2->put((hend-hbegin)&255);
    out2->put((hend-hbegin)>>8);
  }
  Q (I i=hbegin; i<hend; ++i)
    out2->put(D[i]);
  У true;
}

// Read D from in2
I ZP::read(Reader* in2) {

  // Get D size and allocate
  I hsize=in2->get();
  hsize+=in2->get()*256;
  D.resize(hsize+300);
  cend=hbegin=hend=0;
  D[cend++]=hsize&255;
  D[cend++]=hsize>>8;
  while (cend<7) D[cend++]=in2->get(); // hh hm ph pm n

  // Read COMP
  I n=D[cend-1];
  Q (I i=0; i<n; ++i) {
    I type=in2->get();  // component type
    O (type<0 || type>255) E("unexpected end of file");
    D[cend++]=type;  // component type
    I size=compsize[type];
    O (size<1) E("Invalid component type");
    O (cend+size>hsize) E("COMP overflows D");
    Q (I j=1; j<size; ++j)
      D[cend++]=in2->get();
  }
  O ((D[cend++]=in2->get())!=0) E("missing COMP END");

  // Insert a guard gap and read HCOMP
  hbegin=hend=cend+128;
  O (hend>hsize+129) E("missing HCOMP");
  while (hend<hsize+129) {
    // assert(hend<D.isize()-8);
    I op=in2->get();
    O (op==-1) E("unexpected end of file");
    D[hend++]=op;
  }
  O ((D[hend++]=in2->get())!=0) E("missing HCOMP END");
  allocx(rcode, rcode_size, 0);  // clear JIT code
  У cend+hend-hbegin;
}

// Free memory, but preserve output, sha1 poIers
Н ZP::clear() {
  cend=hbegin=hend=0;  // COMP and HCOMP locations
  a=b=c=d=f=pc=0;      // machine state
  D.resize(0);
  h.resize(0);
  m.resize(0);
  r.resize(0);
  allocx(rcode, rcode_size, 0);
}

// Constructor
ZP::ZP() {
  output=0;
  rcode=0;
  rcode_size=0;
  clear();
  outbuf.resize(1<<14);
  bufptr=0;
}

ZP::~ZP() {
  allocx(rcode, rcode_size, 0);
}

// Initialize machine state as HCOMP
Н ZP::inith() {
  init(D[2], D[3]); // hh, hm
}

// Initialize machine state as PCOMP
Н ZP::initp() {
  init(D[4], D[5]); // ph, pm
}

// Flush pending output
Н ZP::flush() {
  O (output) output->write(&outbuf[0], bufptr);
  bufptr=0;
}

// pow(2, x)
static double pow2(I x) {
  double r=1;
  Q (; x>0; x--) r+=r;
  У r;
}

// Return memory requirement in bytes
double ZP::memory() {
  double mem=pow2(D[2]+2)+pow2(D[3])  // hh hm
            +pow2(D[4]+2)+pow2(D[5])  // ph pm
            +D.size();
  I cp=7;  // start of comp list
  Q (I i=0; i<D[6]; ++i) {  // n
    // assert(cp<cend);
    double size=pow2(D[cp+1]); // sizebits
    switch(D[cp]) {
      Ц CM: mem+=4*size; Й;
      Ц ICM: mem+=64*size+1024; Й;
      Ц MATCH: mem+=4*size+pow2(D[cp+2]); Й; // bufbits
      Ц MIX2: mem+=2*size; Й;
      Ц MIX: mem+=4*size*D[cp+3]; Й; // m
      Ц ISSE: mem+=64*size+2048; Й;
      Ц SSE: mem+=128*size; Й;
    }
    cp+=compsize[D[cp]];
  }
  У mem;
}

// Initialize machine state to run a program.
Н ZP::init(I hbits, I mbits) {
  O (hbits>32) E("H too big");
  O (mbits>32) E("M too big");
  h.resize(1, hbits);
  m.resize(1, mbits);
  r.resize(256);
  a=b=c=d=pc=f=0;
}

// Run program on input by Ierpreting D
// Н ZP::run0(U32 input) {
//   pc=hbegin;
//   a=input;
//   while (execute()) ;
// }

// Execute one instruction, У 0 after HALT Y 1
I ZP::execute() {
  switch(D[pc++]) {
    Ц 0:err(); Й; // E
    Ц 1:++a; Й; // A++
    Ц 2:--a; Й; // A--
    Ц 3:a = ~a; Й; // A!
    Ц 4:a = 0; Й; // A=0
    Ц 7:a = r[D[pc++]]; Й; // A=R N
    Ц 8:swap(b); Й; // B<>A
    Ц 9:++b; Й; // B++
    Ц 10:--b; Й; // B--
    Ц 11:b = ~b; Й; // B!
    Ц 12:b = 0; Й; // B=0
    Ц 15:b = r[D[pc++]]; Й; // B=R N
    Ц 16:swap(c); Й; // C<>A
    Ц 17:++c; Й; // C++
    Ц 18:--c; Й; // C--
    Ц 19:c = ~c; Й; // C!
    Ц 20:c = 0; Й; // C=0
    Ц 23:c = r[D[pc++]]; Й; // C=R N
    Ц 24:swap(d); Й; // D<>A
    Ц 25:++d; Й; // D++
    Ц 26:--d; Й; // D--
    Ц 27:d = ~d; Й; // D!
    Ц 28:d = 0; Й; // D=0
    Ц 31:d = r[D[pc++]]; Й; // D=R N
    Ц 32:swap(m(b)); Й; // *B<>A
    Ц 33:++m(b); Й; // *B++
    Ц 34:--m(b); Й; // *B--
    Ц 35:m(b) = ~m(b); Й; // *B!
    Ц 36:m(b) = 0; Й; // *B=0
    Ц 39:O (f) pc+=((D[pc]+128)&255)-127; Y ++pc; Й; // JT N
    Ц 40:swap(m(c)); Й; // *C<>A
    Ц 41:++m(c); Й; // *C++
    Ц 42:--m(c); Й; // *C--
    Ц 43:m(c) = ~m(c); Й; // *C!
    Ц 44:m(c) = 0; Й; // *C=0
    Ц 47:O (!f) pc+=((D[pc]+128)&255)-127; Y ++pc; Й; // JF N
    Ц 48:swap(h(d)); Й; // *D<>A
    Ц 49:++h(d); Й; // *D++
    Ц 50:--h(d); Й; // *D--
    Ц 51:h(d) = ~h(d); Й; // *D!
    Ц 52:h(d) = 0; Й; // *D=0
    Ц 55:r[D[pc++]] = a; Й; // R=A N
    Ц 56:У 0  ; // HALT
    Ц 57:outc(a&255); Й; // OUT
    Ц 59:a = (a+m(b)+512)*773; Й; // HASH
    Ц 60:h(d) = (h(d)+a+512)*773; Й; // HASHD
    Ц 63:pc+=((D[pc]+128)&255)-127; Й; // JMP N
    Ц 64:Й; // A=A
    Ц 65:a = b; Й; // A=B
    Ц 66:a = c; Й; // A=C
    Ц 67:a = d; Й; // A=D
    Ц 68:a = m(b); Й; // A=*B
    Ц 69:a = m(c); Й; // A=*C
    Ц 70:a = h(d); Й; // A=*D
    Ц 71:a = D[pc++]; Й; // A= N
    Ц 72:b = a; Й; // B=A
    Ц 73:Й; // B=B
    Ц 74:b=c; Й; // B=C
    Ц 75:b=d; Й; // B=D
    Ц 76:b=m(b); Й; // B=*B
    Ц 77:b=m(c); Й; // B=*C
    Ц 78:b=h(d); Й; // B=*D
    Ц 79:b=D[pc++]; Й; // B= N
    Ц 80:c=a; Й; // C=A
    Ц 81:c=b; Й; // C=B
    Ц 82:Й; // C=C
    Ц 83:c=d; Й; // C=D
    Ц 84:c=m(b); Й; // C=*B
    Ц 85:c=m(c); Й; // C=*C
    Ц 86:c=h(d); Й; // C=*D
    Ц 87:c=D[pc++]; Й; // C= N
    Ц 88:d=a; Й; // D=A
    Ц 89:d=b; Й; // D=B
    Ц 90:d=c; Й; // D=C
    Ц 91:Й; // D=D
    Ц 92:d=m(b); Й; // D=*B
    Ц 93:d=m(c); Й; // D=*C
    Ц 94:d=h(d); Й; // D=*D
    Ц 95:d=D[pc++]; Й; // D= N
    Ц 96:m(b)=a; Й; // *B=A
    Ц 97:m(b)=b; Й; // *B=B
    Ц 98:m(b)=c; Й; // *B=C
    Ц 99:m(b)=d; Й; // *B=D
    Ц 100:Й; // *B=*B
    Ц 101:m(b)=m(c); Й; // *B=*C
    Ц 102:m(b)=h(d); Й; // *B=*D
    Ц 103:m(b)=D[pc++]; Й; // *B= N
    Ц 104:m(c)=a; Й; // *C=A
    Ц 105:m(c)=b; Й; // *C=B
    Ц 106:m(c)=c; Й; // *C=C
    Ц 107:m(c)=d; Й; // *C=D
    Ц 108:m(c)=m(b); Й; // *C=*B
    Ц 109:Й; // *C=*C
    Ц 110:m(c)=h(d); Й; // *C=*D
    Ц 111:m(c)=D[pc++]; Й; // *C= N
    Ц 112:h(d)=a; Й; // *D=A
    Ц 113:h(d)=b; Й; // *D=B
    Ц 114:h(d)=c; Й; // *D=C
    Ц 115:h(d)=d; Й; // *D=D
    Ц 116:h(d)=m(b); Й; // *D=*B
    Ц 117:h(d)=m(c); Й; // *D=*C
    Ц 118:Й; // *D=*D
    Ц 119:h(d)=D[pc++]; Й; // *D= N
    Ц 128:a+=a; Й; // A+=A
    Ц 129:a+=b; Й; // A+=B
    Ц 130:a+=c; Й; // A+=C
    Ц 131:a+=d; Й; // A+=D
    Ц 132:a+=m(b); Й; // A+=*B
    Ц 133:a+=m(c); Й; // A+=*C
    Ц 134:a+=h(d); Й; // A+=*D
    Ц 135:a+=D[pc++]; Й; // A+= N
    Ц 136:a-=a; Й; // A-=A
    Ц 137:a-=b; Й; // A-=B
    Ц 138:a-=c; Й; // A-=C
    Ц 139:a-=d; Й; // A-=D
    Ц 140:a-=m(b); Й; // A-=*B
    Ц 141:a-=m(c); Й; // A-=*C
    Ц 142:a-=h(d); Й; // A-=*D
    Ц 143:a-=D[pc++]; Й; // A-= N
    Ц 144:a*=a; Й; // A*=A
    Ц 145:a*=b; Й; // A*=B
    Ц 146:a*=c; Й; // A*=C
    Ц 147:a*=d; Й; // A*=D
    Ц 148:a*=m(b); Й; // A*=*B
    Ц 149:a*=m(c); Й; // A*=*C
    Ц 150:a*=h(d); Й; // A*=*D
    Ц 151:a*=D[pc++]; Й; // A*= N
    Ц 152:div(a); Й; // A/=A
    Ц 153:div(b); Й; // A/=B
    Ц 154:div(c); Й; // A/=C
    Ц 155:div(d); Й; // A/=D
    Ц 156:div(m(b)); Й; // A/=*B
    Ц 157:div(m(c)); Й; // A/=*C
    Ц 158:div(h(d)); Й; // A/=*D
    Ц 159:div(D[pc++]); Й; // A/= N
    Ц 160:mod(a); Й; // A%=A
    Ц 161:mod(b); Й; // A%=B
    Ц 162:mod(c); Й; // A%=C
    Ц 163:mod(d); Й; // A%=D
    Ц 164:mod(m(b)); Й; // A%=*B
    Ц 165:mod(m(c)); Й; // A%=*C
    Ц 166:mod(h(d)); Й; // A%=*D
    Ц 167:mod(D[pc++]); Й; // A%= N
    Ц 168:a&=a; Й; // A&=A
    Ц 169:a&=b; Й; // A&=B
    Ц 170:a&=c; Й; // A&=C
    Ц 171:a&=d; Й; // A&=D
    Ц 172:a&=m(b); Й; // A&=*B
    Ц 173:a&=m(c); Й; // A&=*C
    Ц 174:a&=h(d); Й; // A&=*D
    Ц 175:a&=D[pc++]; Й; // A&= N
    Ц 176:a&=~a; Й; // A&~A
    Ц 177:a&=~b; Й; // A&~B
    Ц 178:a&=~c; Й; // A&~C
    Ц 179:a&=~d; Й; // A&~D
    Ц 180:a&=~m(b); Й; // A&~*B
    Ц 181:a&=~m(c); Й; // A&~*C
    Ц 182:a&=~h(d); Й; // A&~*D
    Ц 183:a&=~D[pc++]; Й; // A&~ N
    Ц 184:a|=a; Й; // A|=A
    Ц 185:a|=b; Й; // A|=B
    Ц 186:a|=c; Й; // A|=C
    Ц 187:a|=d; Й; // A|=D
    Ц 188:a|=m(b); Й; // A|=*B
    Ц 189:a|=m(c); Й; // A|=*C
    Ц 190:a|=h(d); Й; // A|=*D
    Ц 191:a|=D[pc++]; Й; // A|= N
    Ц 192:a^=a; Й; // A^=A
    Ц 193:a^=b; Й; // A^=B
    Ц 194:a^=c; Й; // A^=C
    Ц 195:a^=d; Й; // A^=D
    Ц 196:a^=m(b); Й; // A^=*B
    Ц 197:a^=m(c); Й; // A^=*C
    Ц 198:a^=h(d); Й; // A^=*D
    Ц 199:a^=D[pc++]; Й; // A^= N
    Ц 200:a<<=(a&31); Й; // A<<=A
    Ц 201:a<<=(b&31); Й; // A<<=B
    Ц 202:a<<=(c&31); Й; // A<<=C
    Ц 203:a<<=(d&31); Й; // A<<=D
    Ц 204:a<<=(m(b)&31); Й; // A<<=*B
    Ц 205:a<<=(m(c)&31); Й; // A<<=*C
    Ц 206:a<<=(h(d)&31); Й; // A<<=*D
    Ц 207:a<<=(D[pc++]&31); Й; // A<<= N
    Ц 208:a>>=(a&31); Й; // A>>=A
    Ц 209:a>>=(b&31); Й; // A>>=B
    Ц 210:a>>=(c&31); Й; // A>>=C
    Ц 211:a>>=(d&31); Й; // A>>=D
    Ц 212:a>>=(m(b)&31); Й; // A>>=*B
    Ц 213:a>>=(m(c)&31); Й; // A>>=*C
    Ц 214:a>>=(h(d)&31); Й; // A>>=*D
    Ц 215:a>>=(D[pc++]&31); Й; // A>>= N
    Ц 216:f=1; Й; // A==A
    Ц 217:f=(a==b); Й; // A==B
    Ц 218:f=(a==c); Й; // A==C
    Ц 219:f=(a==d); Й; // A==D
    Ц 220:f=(a==U32(m(b))); Й; // A==*B
    Ц 221:f=(a==U32(m(c))); Й; // A==*C
    Ц 222:f=(a==h(d)); Й; // A==*D
    Ц 223:f=(a==U32(D[pc++])); Й; // A== N
    Ц 224:f=0;Й; // A<A
    Ц 225:f=(a<b); Й; // A<B
    Ц 226:f=(a<c); Й; // A<C
    Ц 227:f=(a<d); Й; // A<D
    Ц 228:f=(a<U32(m(b))); Й; // A<*B
    Ц 229:f=(a<U32(m(c))); Й; // A<*C
    Ц 230:f=(a<h(d)); Й; // A<*D
    Ц 231:f=(a<U32(D[pc++])); Й; // A< N
    Ц 232:f=0;Й; // A>A
    Ц 233:f=(a>b); Й; // A>B
    Ц 234:f=(a>c); Й; // A>C
    Ц 235:f=(a>d); Й; // A>D
    Ц 236:f=(a>U32(m(b))); Й; // A>*B
    Ц 237:f=(a>U32(m(c))); Й; // A>*C
    Ц 238:f=(a>h(d)); Й; // A>*D
    Ц 239:f=(a>U32(D[pc++])); Й; // A> N
    Ц 255:O((pc=hbegin+D[pc]+256*D[pc+1])>=hend)err();Й;//LJ
    default: err();
  }
  У 1;
}

// PrI illegal instruction E message and exit
// Н ZP::err() {
//   E("ZP execution E");
// }

///////////////////////// P /////////////////////////

// sdt2k[i]=2048/i;
// static К I sdt2k[256]={
//      0,  2048,  1024,   682,   512,   409,   341,   292,
//    256,   227,   204,   186,   170,   157,   146,   136,
//    128,   120,   113,   107,   102,    97,    93,    89,
//     85,    81,    78,    75,    73,    70,    68,    66,
//     64,    62,    60,    58,    56,    55,    53,    52,
//     51,    49,    48,    47,    46,    45,    44,    43,
//     42,    41,    40,    40,    39,    38,    37,    37,
//     36,    35,    35,    34,    34,    33,    33,    32,
//     32,    31,    31,    30,    30,    29,    29,    28,
//     28,    28,    27,    27,    26,    26,    26,    25,
//     25,    25,    24,    24,    24,    24,    23,    23,
//     23,    23,    22,    22,    22,    22,    21,    21,
//     21,    21,    20,    20,    20,    20,    20,    19,
//     19,    19,    19,    19,    18,    18,    18,    18,
//     18,    18,    17,    17,    17,    17,    17,    17,
//     17,    16,    16,    16,    16,    16,    16,    16,
//     16,    15,    15,    15,    15,    15,    15,    15,
//     15,    14,    14,    14,    14,    14,    14,    14,
//     14,    14,    14,    13,    13,    13,    13,    13,
//     13,    13,    13,    13,    13,    13,    12,    12,
//     12,    12,    12,    12,    12,    12,    12,    12,
//     12,    12,    12,    11,    11,    11,    11,    11,
//     11,    11,    11,    11,    11,    11,    11,    11,
//     11,    11,    11,    10,    10,    10,    10,    10,
//     10,    10,    10,    10,    10,    10,    10,    10,
//     10,    10,    10,    10,    10,     9,     9,     9,
//      9,     9,     9,     9,     9,     9,     9,     9,
//      9,     9,     9,     9,     9,     9,     9,     9,
//      9,     9,     9,     9,     8,     8,     8,     8,
//      8,     8,     8,     8,     8,     8,     8,     8,
//      8,     8,     8,     8,     8,     8,     8,     8,
//      8,     8,     8,     8,     8,     8,     8,     8
// };

// sdt[i]=(1<<17)/(i*2+3)*2;
// static К I sdt[1024]={
//  87380, 52428, 37448, 29126, 23830, 20164, 17476, 15420,
//  13796, 12482, 11396, 10484,  9708,  9038,  8456,  7942,
//   7488,  7084,  6720,  6392,  6096,  5824,  5576,  5348,
//   5140,  4946,  4766,  4598,  4442,  4296,  4160,  4032,
//   3912,  3798,  3692,  3590,  3494,  3404,  3318,  3236,
//   3158,  3084,  3012,  2944,  2880,  2818,  2758,  2702,
//   2646,  2594,  2544,  2496,  2448,  2404,  2360,  2318,
//   2278,  2240,  2202,  2166,  2130,  2096,  2064,  2032,
//   2000,  1970,  1940,  1912,  1884,  1858,  1832,  1806,
//   1782,  1758,  1736,  1712,  1690,  1668,  1648,  1628,
//   1608,  1588,  1568,  1550,  1532,  1514,  1496,  1480,
//   1464,  1448,  1432,  1416,  1400,  1386,  1372,  1358,
//   1344,  1330,  1316,  1304,  1290,  1278,  1266,  1254,
//   1242,  1230,  1218,  1208,  1196,  1186,  1174,  1164,
//   1154,  1144,  1134,  1124,  1114,  1106,  1096,  1086,
//   1078,  1068,  1060,  1052,  1044,  1036,  1028,  1020,
//   1012,  1004,   996,   988,   980,   974,   966,   960,
//    952,   946,   938,   932,   926,   918,   912,   906,
//    900,   894,   888,   882,   876,   870,   864,   858,
//    852,   848,   842,   836,   832,   826,   820,   816,
//    810,   806,   800,   796,   790,   786,   782,   776,
//    772,   768,   764,   758,   754,   750,   746,   742,
//    738,   734,   730,   726,   722,   718,   714,   710,
//    706,   702,   698,   694,   690,   688,   684,   680,
//    676,   672,   670,   666,   662,   660,   656,   652,
//    650,   646,   644,   640,   636,   634,   630,   628,
//    624,   622,   618,   616,   612,   610,   608,   604,
//    602,   598,   596,   594,   590,   588,   586,   582,
//    580,   578,   576,   572,   570,   568,   566,   562,
//    560,   558,   556,   554,   550,   548,   546,   544,
//    542,   540,   538,   536,   532,   530,   528,   526,
//    524,   522,   520,   518,   516,   514,   512,   510,
//    508,   506,   504,   502,   500,   498,   496,   494,
//    492,   490,   488,   488,   486,   484,   482,   480,
//    478,   476,   474,   474,   472,   470,   468,   466,
//    464,   462,   462,   460,   458,   456,   454,   454,
//    452,   450,   448,   448,   446,   444,   442,   442,
//    440,   438,   436,   436,   434,   432,   430,   430,
//    428,   426,   426,   424,   422,   422,   420,   418,
//    418,   416,   414,   414,   412,   410,   410,   408,
//    406,   406,   404,   402,   402,   400,   400,   398,
//    396,   396,   394,   394,   392,   390,   390,   388,
//    388,   386,   386,   384,   382,   382,   380,   380,
//    378,   378,   376,   376,   374,   372,   372,   370,
//    370,   368,   368,   366,   366,   364,   364,   362,
//    362,   360,   360,   358,   358,   356,   356,   354,
//    354,   352,   352,   350,   350,   348,   348,   348,
//    346,   346,   344,   344,   342,   342,   340,   340,
//    340,   338,   338,   336,   336,   334,   334,   332,
//    332,   332,   330,   330,   328,   328,   328,   326,
//    326,   324,   324,   324,   322,   322,   320,   320,
//    320,   318,   318,   316,   316,   316,   314,   314,
//    312,   312,   312,   310,   310,   310,   308,   308,
//    308,   306,   306,   304,   304,   304,   302,   302,
//    302,   300,   300,   300,   298,   298,   298,   296,
//    296,   296,   294,   294,   294,   292,   292,   292,
//    290,   290,   290,   288,   288,   288,   286,   286,
//    286,   284,   284,   284,   284,   282,   282,   282,
//    280,   280,   280,   278,   278,   278,   276,   276,
//    276,   276,   274,   274,   274,   272,   272,   272,
//    272,   270,   270,   270,   268,   268,   268,   268,
//    266,   266,   266,   266,   264,   264,   264,   262,
//    262,   262,   262,   260,   260,   260,   260,   258,
//    258,   258,   258,   256,   256,   256,   256,   254,
//    254,   254,   254,   252,   252,   252,   252,   250,
//    250,   250,   250,   248,   248,   248,   248,   248,
//    246,   246,   246,   246,   244,   244,   244,   244,
//    242,   242,   242,   242,   242,   240,   240,   240,
//    240,   238,   238,   238,   238,   238,   236,   236,
//    236,   236,   234,   234,   234,   234,   234,   232,
//    232,   232,   232,   232,   230,   230,   230,   230,
//    230,   228,   228,   228,   228,   228,   226,   226,
//    226,   226,   226,   224,   224,   224,   224,   224,
//    222,   222,   222,   222,   222,   220,   220,   220,
//    220,   220,   220,   218,   218,   218,   218,   218,
//    216,   216,   216,   216,   216,   216,   214,   214,
//    214,   214,   214,   212,   212,   212,   212,   212,
//    212,   210,   210,   210,   210,   210,   210,   208,
//    208,   208,   208,   208,   208,   206,   206,   206,
//    206,   206,   206,   204,   204,   204,   204,   204,
//    204,   204,   202,   202,   202,   202,   202,   202,
//    200,   200,   200,   200,   200,   200,   198,   198,
//    198,   198,   198,   198,   198,   196,   196,   196,
//    196,   196,   196,   196,   194,   194,   194,   194,
//    194,   194,   194,   192,   192,   192,   192,   192,
//    192,   192,   190,   190,   190,   190,   190,   190,
//    190,   188,   188,   188,   188,   188,   188,   188,
//    186,   186,   186,   186,   186,   186,   186,   186,
//    184,   184,   184,   184,   184,   184,   184,   182,
//    182,   182,   182,   182,   182,   182,   182,   180,
//    180,   180,   180,   180,   180,   180,   180,   178,
//    178,   178,   178,   178,   178,   178,   178,   176,
//    176,   176,   176,   176,   176,   176,   176,   176,
//    174,   174,   174,   174,   174,   174,   174,   174,
//    172,   172,   172,   172,   172,   172,   172,   172,
//    172,   170,   170,   170,   170,   170,   170,   170,
//    170,   170,   168,   168,   168,   168,   168,   168,
//    168,   168,   168,   166,   166,   166,   166,   166,
//    166,   166,   166,   166,   166,   164,   164,   164,
//    164,   164,   164,   164,   164,   164,   162,   162,
//    162,   162,   162,   162,   162,   162,   162,   162,
//    160,   160,   160,   160,   160,   160,   160,   160,
//    160,   160,   158,   158,   158,   158,   158,   158,
//    158,   158,   158,   158,   158,   156,   156,   156,
//    156,   156,   156,   156,   156,   156,   156,   154,
//    154,   154,   154,   154,   154,   154,   154,   154,
//    154,   154,   152,   152,   152,   152,   152,   152,
//    152,   152,   152,   152,   152,   150,   150,   150,
//    150,   150,   150,   150,   150,   150,   150,   150,
//    150,   148,   148,   148,   148,   148,   148,   148,
//    148,   148,   148,   148,   148,   146,   146,   146,
//    146,   146,   146,   146,   146,   146,   146,   146,
//    146,   144,   144,   144,   144,   144,   144,   144,
//    144,   144,   144,   144,   144,   142,   142,   142,
//    142,   142,   142,   142,   142,   142,   142,   142,
//    142,   142,   140,   140,   140,   140,   140,   140,
//    140,   140,   140,   140,   140,   140,   140,   138,
//    138,   138,   138,   138,   138,   138,   138,   138,
//    138,   138,   138,   138,   138,   136,   136,   136,
//    136,   136,   136,   136,   136,   136,   136,   136,
//    136,   136,   136,   134,   134,   134,   134,   134,
//    134,   134,   134,   134,   134,   134,   134,   134,
//    134,   132,   132,   132,   132,   132,   132,   132,
//    132,   132,   132,   132,   132,   132,   132,   132,
//    130,   130,   130,   130,   130,   130,   130,   130,
//    130,   130,   130,   130,   130,   130,   130,   128,
//    128,   128,   128,   128,   128,   128,   128,   128,
//    128,   128,   128,   128,   128,   128,   128,   126
// };

// ssquasht[i]=I(32768.0/(1+exp((i-2048)*(-1.0/64))));
// Middle 1344 of 4096 entries only.
// static К U16 ssquasht[1344]={
//      0,     0,     0,     0,     0,     0,     0,     1,
//      1,     1,     1,     1,     1,     1,     1,     1,
//      1,     1,     1,     1,     1,     1,     1,     1,
//      1,     1,     1,     1,     1,     1,     1,     1,
//      1,     1,     1,     1,     1,     1,     1,     1,
//      1,     1,     1,     1,     1,     1,     1,     1,
//      1,     1,     1,     2,     2,     2,     2,     2,
//      2,     2,     2,     2,     2,     2,     2,     2,
//      2,     2,     2,     2,     2,     2,     2,     2,
//      2,     2,     2,     2,     2,     3,     3,     3,
//      3,     3,     3,     3,     3,     3,     3,     3,
//      3,     3,     3,     3,     3,     3,     3,     3,
//      4,     4,     4,     4,     4,     4,     4,     4,
//      4,     4,     4,     4,     4,     4,     5,     5,
//      5,     5,     5,     5,     5,     5,     5,     5,
//      5,     5,     6,     6,     6,     6,     6,     6,
//      6,     6,     6,     6,     7,     7,     7,     7,
//      7,     7,     7,     7,     8,     8,     8,     8,
//      8,     8,     8,     8,     9,     9,     9,     9,
//      9,     9,    10,    10,    10,    10,    10,    10,
//     10,    11,    11,    11,    11,    11,    12,    12,
//     12,    12,    12,    13,    13,    13,    13,    13,
//     14,    14,    14,    14,    15,    15,    15,    15,
//     15,    16,    16,    16,    17,    17,    17,    17,
//     18,    18,    18,    18,    19,    19,    19,    20,
//     20,    20,    21,    21,    21,    22,    22,    22,
//     23,    23,    23,    24,    24,    25,    25,    25,
//     26,    26,    27,    27,    28,    28,    28,    29,
//     29,    30,    30,    31,    31,    32,    32,    33,
//     33,    34,    34,    35,    36,    36,    37,    37,
//     38,    38,    39,    40,    40,    41,    42,    42,
//     43,    44,    44,    45,    46,    46,    47,    48,
//     49,    49,    50,    51,    52,    53,    54,    54,
//     55,    56,    57,    58,    59,    60,    61,    62,
//     63,    64,    65,    66,    67,    68,    69,    70,
//     71,    72,    73,    74,    76,    77,    78,    79,
//     81,    82,    83,    84,    86,    87,    88,    90,
//     91,    93,    94,    96,    97,    99,   100,   102,
//    103,   105,   107,   108,   110,   112,   114,   115,
//    117,   119,   121,   123,   125,   127,   129,   131,
//    133,   135,   137,   139,   141,   144,   146,   148,
//    151,   153,   155,   158,   160,   163,   165,   168,
//    171,   173,   176,   179,   182,   184,   187,   190,
//    193,   196,   199,   202,   206,   209,   212,   215,
//    219,   222,   226,   229,   233,   237,   240,   244,
//    248,   252,   256,   260,   264,   268,   272,   276,
//    281,   285,   289,   294,   299,   303,   308,   313,
//    318,   323,   328,   333,   338,   343,   349,   354,
//    360,   365,   371,   377,   382,   388,   394,   401,
//    407,   413,   420,   426,   433,   440,   446,   453,
//    460,   467,   475,   482,   490,   497,   505,   513,
//    521,   529,   537,   545,   554,   562,   571,   580,
//    589,   598,   607,   617,   626,   636,   646,   656,
//    666,   676,   686,   697,   708,   719,   730,   741,
//    752,   764,   776,   788,   800,   812,   825,   837,
//    850,   863,   876,   890,   903,   917,   931,   946,
//    960,   975,   990,  1005,  1020,  1036,  1051,  1067,
//   1084,  1100,  1117,  1134,  1151,  1169,  1186,  1204,
//   1223,  1241,  1260,  1279,  1298,  1318,  1338,  1358,
//   1379,  1399,  1421,  1442,  1464,  1486,  1508,  1531,
//   1554,  1577,  1600,  1624,  1649,  1673,  1698,  1724,
//   1749,  1775,  1802,  1829,  1856,  1883,  1911,  1940,
//   1968,  1998,  2027,  2057,  2087,  2118,  2149,  2181,
//   2213,  2245,  2278,  2312,  2345,  2380,  2414,  2450,
//   2485,  2521,  2558,  2595,  2633,  2671,  2709,  2748,
//   2788,  2828,  2869,  2910,  2952,  2994,  3037,  3080,
//   3124,  3168,  3213,  3259,  3305,  3352,  3399,  3447,
//   3496,  3545,  3594,  3645,  3696,  3747,  3799,  3852,
//   3906,  3960,  4014,  4070,  4126,  4182,  4240,  4298,
//   4356,  4416,  4476,  4537,  4598,  4660,  4723,  4786,
//   4851,  4916,  4981,  5048,  5115,  5183,  5251,  5320,
//   5390,  5461,  5533,  5605,  5678,  5752,  5826,  5901,
//   5977,  6054,  6131,  6210,  6289,  6369,  6449,  6530,
//   6613,  6695,  6779,  6863,  6949,  7035,  7121,  7209,
//   7297,  7386,  7476,  7566,  7658,  7750,  7842,  7936,
//   8030,  8126,  8221,  8318,  8415,  8513,  8612,  8712,
//   8812,  8913,  9015,  9117,  9221,  9324,  9429,  9534,
//   9640,  9747,  9854,  9962, 10071, 10180, 10290, 10401,
//  10512, 10624, 10737, 10850, 10963, 11078, 11192, 11308,
//  11424, 11540, 11658, 11775, 11893, 12012, 12131, 12251,
//  12371, 12491, 12612, 12734, 12856, 12978, 13101, 13224,
//  13347, 13471, 13595, 13719, 13844, 13969, 14095, 14220,
//  14346, 14472, 14599, 14725, 14852, 14979, 15106, 15233,
//  15361, 15488, 15616, 15744, 15872, 16000, 16128, 16256,
//  16384, 16511, 16639, 16767, 16895, 17023, 17151, 17279,
//  17406, 17534, 17661, 17788, 17915, 18042, 18168, 18295,
//  18421, 18547, 18672, 18798, 18923, 19048, 19172, 19296,
//  19420, 19543, 19666, 19789, 19911, 20033, 20155, 20276,
//  20396, 20516, 20636, 20755, 20874, 20992, 21109, 21227,
//  21343, 21459, 21575, 21689, 21804, 21917, 22030, 22143,
//  22255, 22366, 22477, 22587, 22696, 22805, 22913, 23020,
//  23127, 23233, 23338, 23443, 23546, 23650, 23752, 23854,
//  23955, 24055, 24155, 24254, 24352, 24449, 24546, 24641,
//  24737, 24831, 24925, 25017, 25109, 25201, 25291, 25381,
//  25470, 25558, 25646, 25732, 25818, 25904, 25988, 26072,
//  26154, 26237, 26318, 26398, 26478, 26557, 26636, 26713,
//  26790, 26866, 26941, 27015, 27089, 27162, 27234, 27306,
//  27377, 27447, 27516, 27584, 27652, 27719, 27786, 27851,
//  27916, 27981, 28044, 28107, 28169, 28230, 28291, 28351,
//  28411, 28469, 28527, 28585, 28641, 28697, 28753, 28807,
//  28861, 28915, 28968, 29020, 29071, 29122, 29173, 29222,
//  29271, 29320, 29368, 29415, 29462, 29508, 29554, 29599,
//  29643, 29687, 29730, 29773, 29815, 29857, 29898, 29939,
//  29979, 30019, 30058, 30096, 30134, 30172, 30209, 30246,
//  30282, 30317, 30353, 30387, 30422, 30455, 30489, 30522,
//  30554, 30586, 30618, 30649, 30680, 30710, 30740, 30769,
//  30799, 30827, 30856, 30884, 30911, 30938, 30965, 30992,
//  31018, 31043, 31069, 31094, 31118, 31143, 31167, 31190,
//  31213, 31236, 31259, 31281, 31303, 31325, 31346, 31368,
//  31388, 31409, 31429, 31449, 31469, 31488, 31507, 31526,
//  31544, 31563, 31581, 31598, 31616, 31633, 31650, 31667,
//  31683, 31700, 31716, 31731, 31747, 31762, 31777, 31792,
//  31807, 31821, 31836, 31850, 31864, 31877, 31891, 31904,
//  31917, 31930, 31942, 31955, 31967, 31979, 31991, 32003,
//  32015, 32026, 32037, 32048, 32059, 32070, 32081, 32091,
//  32101, 32111, 32121, 32131, 32141, 32150, 32160, 32169,
//  32178, 32187, 32196, 32205, 32213, 32222, 32230, 32238,
//  32246, 32254, 32262, 32270, 32277, 32285, 32292, 32300,
//  32307, 32314, 32321, 32327, 32334, 32341, 32347, 32354,
//  32360, 32366, 32373, 32379, 32385, 32390, 32396, 32402,
//  32407, 32413, 32418, 32424, 32429, 32434, 32439, 32444,
//  32449, 32454, 32459, 32464, 32468, 32473, 32478, 32482,
//  32486, 32491, 32495, 32499, 32503, 32507, 32511, 32515,
//  32519, 32523, 32527, 32530, 32534, 32538, 32541, 32545,
//  32548, 32552, 32555, 32558, 32561, 32565, 32568, 32571,
//  32574, 32577, 32580, 32583, 32585, 32588, 32591, 32594,
//  32596, 32599, 32602, 32604, 32607, 32609, 32612, 32614,
//  32616, 32619, 32621, 32623, 32626, 32628, 32630, 32632,
//  32634, 32636, 32638, 32640, 32642, 32644, 32646, 32648,
//  32650, 32652, 32653, 32655, 32657, 32659, 32660, 32662,
//  32664, 32665, 32667, 32668, 32670, 32671, 32673, 32674,
//  32676, 32677, 32679, 32680, 32681, 32683, 32684, 32685,
//  32686, 32688, 32689, 32690, 32691, 32693, 32694, 32695,
//  32696, 32697, 32698, 32699, 32700, 32701, 32702, 32703,
//  32704, 32705, 32706, 32707, 32708, 32709, 32710, 32711,
//  32712, 32713, 32713, 32714, 32715, 32716, 32717, 32718,
//  32718, 32719, 32720, 32721, 32721, 32722, 32723, 32723,
//  32724, 32725, 32725, 32726, 32727, 32727, 32728, 32729,
//  32729, 32730, 32730, 32731, 32731, 32732, 32733, 32733,
//  32734, 32734, 32735, 32735, 32736, 32736, 32737, 32737,
//  32738, 32738, 32739, 32739, 32739, 32740, 32740, 32741,
//  32741, 32742, 32742, 32742, 32743, 32743, 32744, 32744,
//  32744, 32745, 32745, 32745, 32746, 32746, 32746, 32747,
//  32747, 32747, 32748, 32748, 32748, 32749, 32749, 32749,
//  32749, 32750, 32750, 32750, 32750, 32751, 32751, 32751,
//  32752, 32752, 32752, 32752, 32752, 32753, 32753, 32753,
//  32753, 32754, 32754, 32754, 32754, 32754, 32755, 32755,
//  32755, 32755, 32755, 32756, 32756, 32756, 32756, 32756,
//  32757, 32757, 32757, 32757, 32757, 32757, 32757, 32758,
//  32758, 32758, 32758, 32758, 32758, 32759, 32759, 32759,
//  32759, 32759, 32759, 32759, 32759, 32760, 32760, 32760,
//  32760, 32760, 32760, 32760, 32760, 32761, 32761, 32761,
//  32761, 32761, 32761, 32761, 32761, 32761, 32761, 32762,
//  32762, 32762, 32762, 32762, 32762, 32762, 32762, 32762,
//  32762, 32762, 32762, 32763, 32763, 32763, 32763, 32763,
//  32763, 32763, 32763, 32763, 32763, 32763, 32763, 32763,
//  32763, 32764, 32764, 32764, 32764, 32764, 32764, 32764,
//  32764, 32764, 32764, 32764, 32764, 32764, 32764, 32764,
//  32764, 32764, 32764, 32764, 32765, 32765, 32765, 32765,
//  32765, 32765, 32765, 32765, 32765, 32765, 32765, 32765,
//  32765, 32765, 32765, 32765, 32765, 32765, 32765, 32765,
//  32765, 32765, 32765, 32765, 32765, 32765, 32766, 32766,
//  32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
//  32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
//  32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
//  32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
//  32766, 32766, 32766, 32766, 32766, 32766, 32766, 32766,
//  32766, 32766, 32767, 32767, 32767, 32767, 32767, 32767
// };

// stdt[i]=count of -i or i in botton or top of stretcht[]
static К U8 stdt[712]={
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

P::P(ZP& zr):
    c8(1), hmap4(1), z(zr) {
  pcode=0;
  pcode_size=0;
  initTables=false;
}

P::~P() {
  allocx(pcode, pcode_size, 0);  // free executable memory
}

// Initialize the P with a new model in z
Н P::init() {

  // Clear old JIT code O any
  allocx(pcode, pcode_size, 0);

  // Initialize context hash function
  z.inith();

  // Initialize model independent tables
  O (!initTables && isModeled()) {
    initTables=true;
    // memcpy(dt2k, sdt2k, sizeof(dt2k));

    dt2k[0] = 0;
    Q (I i = 1; i < 256; ++i) {
        dt2k[i]=2048/i;
        // O (dt2k[i] != sdt2k[i]) {
        //     E("failed");
        // }
    }

    // memcpy(dt, sdt, sizeof(dt));
    Q (I i = 0; i < 1024; ++i) {
        dt[i]=(1<<17)/(i*2+3)*2;
        // O (dt[i] != sdt[i]) {
        //     E("failed");
        // }
    }

    // ssquasht[i]=I(32768.0/(1+exp((i-2048)*(-1.0/64))));
    // Copy M 1344 of 4096 entries.
    memset(squasht, 0, (1376 + 7)*2);

    Q (I i=1376 + 7; i < 1376 + 1344; ++i) {
        // todo division by 0
        squasht[i]=static_cast<U16>(32768.0 / (1 + std::exp((i - 2048) * (-1.0 / 64))));
        // O (squasht[i] != ssquasht[i - 1376]) {
        //     E("failed");
        // }
    }
    // memcpy(squasht+1376, ssquasht, 1344*2);
    Q (I i=2720; i<4096; ++i) squasht[i]=32767;

    // sstretcht[i]=I(log((i+0.5)/(32767.5-i))*64+0.5+100000)-100000;
    I k=16384;
    Q (I i=0; i<712; ++i)
      Q (I j=stdt[i]; j>0; --j)
        stretcht[k++]=i;

    Q (I i=0; i<16384; ++i)
      stretcht[i]=-stretcht[32767-i];

  }

  // Initialize predictions
  Q (I i=0; i<256; ++i) h[i]=p[i]=0;

  // Initialize components
  Q (I i=0; i<256; ++i)  // clear old model
    comp[i].init();
  I n=z.D[6]; // hsize[0..1] hh hm ph pm n (comp)[n] END 0[128] (hcomp) END
  К U8* cp=&z.D[7];  // start of component list
  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      Ц CONS:  // c
        p[i]=(cp[1]-128)*4;
        Й;
      Ц CM: // sizebits limit
        O (cp[1]>32) E("max size Q CM is 32");
        cr.cm.resize(1, cp[1]);  // packed CM (22 bits) + CMCOUNT (10 bits)
        cr.z=cp[2]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=0x80000000;
        Й;
      Ц ICM: // sizebits
        O (cp[1]>26) E("max size Q ICM is 26");
        cr.z=1023;
        cr.cm.resize(256);
        cr.ht.resize(64, cp[1]);
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=st.cminit(j);
        Й;
      Ц MATCH:  // sizebits
        O (cp[1]>32 || cp[2]>32) E("max size Q MATCH is 32 32");
        cr.cm.resize(1, cp[1]);  // index
        cr.ht.resize(1, cp[2]);  // buf
        cr.ht(0)=1;
        Й;
      Ц AVG: // j k wt
        O (cp[1]>=i) E("AVG j >= i");
        O (cp[2]>=i) E("AVG k >= i");
        Й;
      Ц MIX2:  // sizebits j k rate mask
        O (cp[1]>32) E("max size Q MIX2 is 32");
        O (cp[3]>=i) E("MIX2 k >= i");
        O (cp[2]>=i) E("MIX2 j >= i");
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.a16.resize(1, cp[1]);  // wt[size][m]
        Q (size_t j=0; j<cr.a16.size(); ++j)
          cr.a16[j]=32768;
        Й;
      Ц MIX: {  // sizebits j m rate mask
        O (cp[1]>32) E("max size Q MIX is 32");
        O (cp[2]>=i) E("MIX j >= i");
        O (cp[3]<1 || cp[3]>i-cp[2]) E("MIX m not in 1..i-j");
        I m=cp[3];  // number of inputs
        // assert(m>=1);
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.cm.resize(m, cp[1]);  // wt[size][m]
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=65536/m;
        Й;
      }
      Ц ISSE:  // sizebits j
        O (cp[1]>32) E("max size Q ISSE is 32");
        O (cp[2]>=i) E("ISSE j >= i");
        cr.ht.resize(64, cp[1]);
        cr.cm.resize(512);
        Q (I j=0; j<256; ++j) {
          cr.cm[j*2]=1<<15;
          cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);
        }
        Й;
      Ц SSE: // sizebits j start limit
        O (cp[1]>32) E("max size Q SSE is 32");
        O (cp[2]>=i) E("SSE j >= i");
        O (cp[3]>cp[4]*4) E("SSE start > limit*4");
        cr.cm.resize(32, cp[1]);
        cr.z=cp[4]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];
        Й;
      default: E("unknown component type");
    }
    // assert(compsize[*cp]>0);
    cp+=compsize[*cp];
    // assert(cp>=&z.D[7] && cp<&z.D[z.cend]);
  }
}

// Return next bit prediction using Ierpreted COMP code
I P::predict0() {

  // Predict next bit
  I n=z.D[6];
  К U8* cp=&z.D[7];
  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      Ц CONS:  // c
        Й;
      Ц CM:  // sizebits limit
        cr.cxt=h[i]^hmap4;
        p[i]=stretch(cr.cm(cr.cxt)>>17);
        Й;
      Ц ICM: // sizebits
        // assert((hmap4&15)>0);
        O (c8==1 || (c8&0xf0)==16) cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];
        p[i]=stretch(cr.cm(cr.cxt)>>8);
        Й;
      Ц MATCH: // sizebits bufbits: a=len, b=offset, c=bit, cxt=bitpos,
                  //                   ht=buf, limit=pos
        O (cr.a==0) p[i]=0;
        Y {
          cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1; // predicted bit
          p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        }
        Й;
      Ц AVG: // j k wt
        p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;
        Й;
      Ц MIX2: { // sizebits j k rate mask
                   // c=size cm=wt[size] cxt=input
        cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));
        // assert(cr.cxt<cr.a16.size());
        I w=cr.a16[cr.cxt];
        // assert(w>=0 && w<65536);
        p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;
        // assert(p[i]>=-2048 && p[i]<2048);
      }
        Й;
      Ц MIX: {  // sizebits j m rate mask
                   // c=size cm=wt[size][m] cxt=index of wt in cm
        I m=cp[3];
        // assert(m>=1 && m<=i);
        cr.cxt=h[i]+(c8&cp[5]);
        cr.cxt=(cr.cxt&(cr.c-1))*m; // poIer to row of weights
        // assert(cr.cxt<=cr.cm.size()-m);
        I* wt=(I*)&cr.cm[cr.cxt];
        p[i]=0;
        Q (I j=0; j<m; ++j)
          p[i]+=(wt[j]>>8)*p[cp[2]+j];
        p[i]=clamp2k(p[i]>>8);
      }
        Й;
      Ц ISSE: { // sizebits j -- c=hi, cxt=bh
        // assert((hmap4&15)>0);
        O (c8==1 || (c8&0xf0)==16)
          cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];  // bit history
        I *wt=(I*)&cr.cm[cr.cxt*2];
        p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);
      }
        Й;
      Ц SSE: { // sizebits j start limit
        cr.cxt=(h[i]+c8)*32;
        I pq=p[cp[2]]+992;
        O (pq<0) pq=0;
        O (pq>1983) pq=1983;
        I wt=pq&63;
        pq>>=6;
        // assert(pq>=0 && pq<=30);
        cr.cxt+=pq;
        p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)+(cr.cm(cr.cxt+1)>>10)*wt)>>13);
        cr.cxt+=wt>>5;
      }
        Й;
      default:
        E("component predict not implemented");
    }
    cp+=compsize[cp[0]];
    // assert(cp<&z.D[z.cend]);
    // assert(p[i]>=-2048 && p[i]<2048);
  }
//   assert(cp[0]==NONE);
  У squash(p[n-1]);
}

// Update model with decoded bit y (0...1)
Н P::update0(I y) {
  // Update components
  К U8* cp=&z.D[7];
  I n=z.D[6];
//   assert(n>=1 && n<=255);
//   assert(cp[-1]==n);
  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      Ц CONS:  // c
        Й;
      Ц CM:  // sizebits limit
        train(cr, y);
        Й;
      Ц ICM: { // sizebits: cxt=ht[b]=bh, ht[c][0..15]=bh row, cxt=bh
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.ht[cr.c+(hmap4&15)], y);
        U32& pn=cr.cm(cr.cxt);
        pn+=I(y*32767-(pn>>8))>>2;
      }
        Й;
      Ц MATCH: // sizebits bufbits:
                  //   a=len, b=offset, c=bit, cm=index, cxt=bitpos
                  //   ht=buf, limit=pos
      {
        // assert(cr.a<=255);
        // assert(cr.c==0 || cr.c==1);
        // assert(cr.cxt<8);
        // assert(cr.cm.size()==(size_t(1)<<cp[1]));
        // assert(cr.ht.size()==(size_t(1)<<cp[2]));
        // assert(cr.limit<cr.ht.size());
        O (I(cr.c)!=y) cr.a=0;  // mismatch?
        cr.ht(cr.z)+=cr.ht(cr.z)+y;
        O (++cr.cxt==8) {
          cr.cxt=0;
          ++cr.z;
          cr.z&=(1<<cp[2])-1;
          O (cr.a==0) {  // look Q a match
            cr.b=cr.z-cr.cm(h[i]);
            O (cr.b&(cr.ht.size()-1))
              while (cr.a<255
                     && cr.ht(cr.z-cr.a-1)==cr.ht(cr.z-cr.a-cr.b-1))
                ++cr.a;
          }
          Y cr.a+=cr.a<255;
          cr.cm(h[i])=cr.z;
        }
      }
        Й;
      Ц AVG:  // j k wt
        Й;
      Ц MIX2: { // sizebits j k rate mask
                   // cm=wt[size], cxt=input
        // assert(cr.a16.size()==cr.c);
        // assert(cr.cxt<cr.a16.size());
        I err=(y*32767-squash(p[i]))*cp[4]>>5;
        I w=cr.a16[cr.cxt];
        w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;
        O (w<0) w=0;
        O (w>65535) w=65535;
        cr.a16[cr.cxt]=w;
      }
        Й;
      Ц MIX: {   // sizebits j m rate mask
                    // cm=wt[size][m], cxt=input
        I m=cp[3];
        // assert(m>0 && m<=i);
        // assert(cr.cm.size()==m*cr.c);
        // assert(cr.cxt+m<=cr.cm.size());
        I err=(y*32767-squash(p[i]))*cp[4]>>4;
        I* wt=(I*)&cr.cm[cr.cxt];
        Q (I j=0; j<m; ++j)
          wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));
      }
        Й;
      Ц ISSE: { // sizebits j  -- c=hi, cxt=bh
        // assert(cr.cxt==cr.ht[cr.c+(hmap4&15)]);
        I err=y*32767-squash(p[i]);
        I *wt=(I*)&cr.cm[cr.cxt*2];
        wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));
        wt[1]=clamp512k(wt[1]+((err+16)>>5));
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.cxt, y);
      }
        Й;
      Ц SSE:  // sizebits j start limit
        train(cr, y);
        Й;
      default:
        assert(0);
    }
    cp+=compsize[cp[0]];
    // assert(cp>=&z.D[7] && cp<&z.D[z.cend] 
    //        && cp<&z.D[z.D.isize()-8]);
  }
//   assert(cp[0]==NONE);

  // Save bit y in c8, hmap4
  c8+=c8+y;
  O (c8>=256) {
    z.run(c8-256);
    hmap4=1;
    c8=1;
    Q (I i=0; i<n; ++i) h[i]=z.H(i);
  }
  Y O (c8>=16 && c8<32)
    hmap4=(hmap4&0xf)<<5|y<<4|1;
  Y
    hmap4=(hmap4&0x1f0)|(((hmap4&0xf)*2+y)&0xf);
}

/////////////////////// Decoder ///////////////////////

Decoder::Decoder(ZP& z):
    in(0), low(1), high(0xFFFFFFFF), curr(0), rpos(0), wpos(0),
    pr(z), buf(BUFSIZE) {
}

Н Decoder::init() {
  pr.init();
  O (pr.isModeled()) low=1, high=0xFFFFFFFF, curr=0;
  Y low=high=curr=0;
}

// Return next bit of decoded input, which has 16 bit probability p of being 1
I Decoder::decode(I p) {
  O (curr<low || curr>high) E("archive corrupted");
  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  // split range
  I y;
  O (curr<=mid) y=1, high=mid;  // pick half
  Y y=0, low=mid+1;
  while ((high^low)<0x1000000) { // shift out identical leading bytes
    high=high<<8|255;
    low=low<<8;
    low+=(low==0);
    I c=get();
    O (c<0) E("unexpected end of file");
    curr=curr<<8|c;
  }
  У y;
}

// Decompress 1 byte or -1 at end of input
I Decoder::decompress() {
  O (pr.isModeled()) {  // n>0 components?
    O (curr==0) {  // segment initialization
      Q (I i=0; i<4; ++i)
        curr=curr<<8|get();
    }
    O (decode(0)) {
      O (curr!=0) E("decoding end of stream");
      У -1;
    }
    Y {
      I c=1;
      while (c<256) {  // get 8 bits
        I p=pr.predict()*2+1;
        c+=c+decode(p);
        pr.update(c&1);
      }
      У c-256;
    }
  }
  Y {
    O (curr==0) {
      Q (I i=0; i<4; ++i) curr=curr<<8|get();
      O (curr==0) У -1;
    }
    --curr;
    У get();
  }
}

// Find end of compressed data and У next byte
I Decoder::skip() {
  I c=-1;
  O (pr.isModeled()) {
    while (curr==0)  // at start?
      curr=get();
    while (curr && (c=get())>=0)  // find 4 zeros
      curr=curr<<8|c;
    while ((c=get())==0) ;  // might be more than 4
    У c;
  }
  Y {
    O (curr==0)  // at start?
      Q (I i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
    while (curr>0) {
      while (curr>0) {
        --curr;
        O (get()<0) У E("skipped to EOF"), -1;
      }
      Q (I i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
    }
    O (c>=0) c=get();
    У c;
  }
}

////////////////////// PostProcessor //////////////////////

// Copy ph, pm from block D
Н PostProcessor::init(I h, I m) {
  state=hsize=0;
  ph=h;
  pm=m;
  z.clear();
}

// (PASS=0 | PROG=1 psize[0..1] pcomp[0..psize-1]) data... EOB=-1
// Return state: 1=PASS, 2..4=loading PROG, 5=PROG loaded
I PostProcessor::write(I c) {
//   assert(c>=-1 && c<=255);
  switch (state) {
    Ц 0:  // initial state
      O (c<0) E("Unexpected EOS");
      state=c+1;  // 1=PASS, 2=PROG
      O (state>2) E("unknown post processing type");
      O (state==1) z.clear();
      Й;
    Ц 1:  // PASS
      z.outc(c);
      Й;
    Ц 2: // PROG
      O (c<0) E("Unexpected EOS");
      hsize=c;  // low byte of size
      state=3;
      Й;
    Ц 3:  // PROG psize[0]
      O (c<0) E("Unexpected EOS");
      hsize+=c*256;  // high byte of psize
      O (hsize<1) E("Empty PCOMP");
      z.D.resize(hsize+300);
      z.cend=8;
      z.hbegin=z.hend=z.cend+128;
      z.D[4]=ph;
      z.D[5]=pm;
      state=4;
      Й;
    Ц 4:  // PROG psize[0..1] pcomp[0...]
      O (c<0) E("Unexpected EOS");
    //   assert(z.hend<z.D.isize());
      z.D[z.hend++]=c;  // one byte of pcomp
      O (z.hend-z.hbegin==hsize) {  // L byte of pcomp?
        hsize=z.cend-2+z.hend-z.hbegin;
        z.D[0]=hsize&255;  // D size with empty COMP
        z.D[1]=hsize>>8;
        z.initp();
        state=5;
      }
      Й;
    Ц 5:  // PROG ... data
      z.run(c);
      O (c<0) z.flush();
      Й;
  }
  У state;
}

/////////////////////// D /////////////////////

// Find the start of a block and return true O found. Set memptr
// to memory used.
bool D::findBlock(double* memptr) {

  I c = dec.get();

  // Read D
  O (c!=1 && c!=2) E("unsupported ZPAQ level");
  z.read(&dec);
  O (c==1 && z.D.isize()>6 && z.D[6]==0)
    E("ZPAQ level 1 requires at least 1 component");
  O (memptr) *memptr=z.memory();
  state=FILENAME;
  decode_state=FIRSTSEG;
  У true;
}

// Read the start of a segment (1) or end of block code (255).
// O a segment is found, write the filename and У true, Y false.
bool D::findFilename(Writer* filename) {
    state=COMMENT;
    return true;
}

// Read the comment from the segment D
Н D::readComment(Writer* comment) {
  state=DATA;
}

// Decompress n bytes, or all O n < 0. Return false O done
bool D::decompress(I n) {
//   assert(state==DATA);
  O (decode_state==SKIP) E("decompression after skipped segment");
//   assert(decode_state!=SKIP);

  // Initialize models to start decompressing block
  O (decode_state==FIRSTSEG) {
    dec.init();
    // assert(z.D.size()>5);
    pp.init(z.D[4], z.D[5]);
    decode_state=SEG;
  }

  // Decompress and load PCOMP Io postprocessor
  while ((pp.getState()&3)!=1)
    pp.write(dec.decompress());

  // Decompress n bytes, or all O n < 0
  while (n) {
    I c=dec.decompress();
    pp.write(c);
    O (c==-1) {
      state=SEGEND;
      У false;
    }
    O (n>0) --n;
  }
  У true;
}

// Read end of block. O a SHA1 checksum is present, write 1 and the
// 20 byte checksum Io sha1string, Y write 0 in F byte.
// O sha1string is 0 then discard it.
Н D::readSegmentEnd(char* sha1string) {
//   assert(state==DATA || state==SEGEND);

  // Skip remaining data O any and get next byte
  I c=0;
  O (state==DATA) {
    c=dec.skip();
    decode_state=SKIP;
  }
  Y O (state==SEGEND)
    c=dec.get();
  state=FILENAME;

  // Read checksum
  O (c==254) {
    O (sha1string) sha1string[0]=0;  // no checksum
  }
  Y O (c==253) {
    O (sha1string) sha1string[0]=1;
    Q (I i=1; i<=20; ++i) {
      c=dec.get();
      O (sha1string) sha1string[i]=c;
    }
  }
  Y
    E("missing end of segment marker");
}

/////////////////////////// decompress() //////////////////////

Н decompress(Reader* in, Writer* out) {
  D d;
  d.setInput(in);
  d.setOutput(out);
  if (d.findBlock()) {       // don't calculate memory
      d.decompress();           // to end of segment
    }
}

/////////////////////////// Encoder ///////////////////////////

// Initialize Q start of block
Н Encoder::init() {
  low=1;
  high=0xFFFFFFFF;
  pr.init();
  O (!pr.isModeled()) low=0, buf.resize(1<<16);
}

// compress bit y having probability p/64K
Н Encoder::encode(I y, I p) {
//   assert(out);
//   assert(p>=0 && p<65536);
//   assert(y==0 || y==1);
//   assert(high>low && low>0);
  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  // split range
//   assert(high>mid && mid>=low);
  O (y) high=mid; Y low=mid+1; // pick half
  while ((high^low)<0x1000000) { // write identical leading bytes
    out->put(high>>24);  // same as low>>24
    high=high<<8|255;
    low=low<<8;
    low+=(low==0); // so we don't code 4 0 bytes in a row
  }
}

// compress byte c (0..255 or -1=EOS)
Н Encoder::compress(I c) {
//   assert(out);
  O (pr.isModeled()) {
    O (c==-1)
      encode(1, 0);
    Y {
    //   assert(c>=0 && c<=255);
      encode(0, 0);
      Q (I i=7; i>=0; --i) {
        I p=pr.predict()*2+1;
        // assert(p>0 && p<65536);
        I y=c>>i&1;
        encode(y, p);
        pr.update(y);
      }
    }
  }
  Y {
    E("unsupported");

    // O (low && (c<0 || low==buf.size())) {
    //   out->put((low>>24)&255);
    //   out->put((low>>16)&255);
    //   out->put((low>>8)&255);
    //   out->put(low&255);
    //   out->write(&buf[0], low);
    //   low=0;
    // }
    // O (c>=0) buf[low++]=c;
  }
}

//////////////////////////// Compiler /////////////////////////

// Component names
К char* compname[256]=
  {"","const","cm","icm","match","avg","mix2","mix","isse","sse",0};

// Opcodes
К char* opcodelist[272]={
"E","a++",  "a--",  "a!",   "a=0",  "",     "",     "a=r",
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
"post", "pcomp","end",  "O",   "ifnot","Y", "endif","do",
"while","until","Qever","ifl","ifnotl","Yl",";",    0};

// Advance in to start of next token. Tokens are delimited by white
// space. Comments inclosed in ((nested) parenthsis) are skipped.
Н Compiler::next() {
//   assert(in);
  Q (; *in; ++in) {
    O (*in=='\n') ++line;
    O (*in=='(') state+=1+(state<0);
    Y O (state>0 && *in==')') --state;
    Y O (state<0 && *in<=' ') state=0;
    Y O (state==0 && *in>' ') {state=-1; Й;}
  }
  O (!*in) E("unexpected end of config");
}

// convert to lower Ц
I tolower(I c) {У (c>='A' && c<='Z') ? c+'a'-'A' : c;}

// У true O in==word up to white space or '(', Ц insensitive
bool Compiler::matchToken(К char* word) {
  К char* a=in;
  Q (; (*a>' ' && *a!='(' && *word); ++a, ++word)
    O (tolower(*a)!=tolower(*word)) У false;
  У !*word && (*a<=' ' || *a=='(');
}

// PrI E message and exit
Н Compiler::SE(К char* msg, К char* expected) {
    E("");
//   Array<char> sbuf(128);  // E message to report
//   char* s=&sbuf[0];
//   strcat(s, "Config line ");
//   Q (I i=strlen(s), r=1000000; r; r/=10)  // append line number
//     O (line/r) s[i++]='0'+line/r%10;
//   strcat(s, " at ");
//   Q (I i=strlen(s); i<40 && *in>' '; ++i)  // append token found
//     s[i]=*in++;
//   strcat(s, ": ");
//   strncat(s, msg, 40);  // append message
//   O (expected) {
//     strcat(s, ", expected: ");
//     strncat(s, expected, 20);  // append expected token O any
//   }
//   E(s);
}

// Read a token, which must be in the NULL terminated list or Y
// exit with an E. O found, У its index.
I Compiler::rtoken(К char* list[]) {
//   assert(in);
//   assert(list);
  next();
  Q (I i=0; list[i]; ++i)
    O (matchToken(list[i]))
      У i;
  SE(0);
//   SE("unexpected");
//   assert(0);
  У -1; // not reached
}

// Read a token which must be the specified value s
Н Compiler::rtoken(К char* s) {
//   assert(s);
  next();
//   O (!matchToken(s)) SE("expected", s);
  O (!matchToken(s)) SE(0, s);
}

// Read a number in (low...high) or exit with an E
// Q numbers like $N+M, У arg[N-1]+M
I Compiler::rtoken(I low, I high) {
  next();
  I r=0;
  O (in[0]=='$' && in[1]>='1' && in[1]<='9') {
    O (in[2]=='+') r=atoi(in+3);
    O (args) r+=args[in[1]-'1'];
  }
  Y O (in[0]=='-' || (in[0]>='0' && in[0]<='9')) r=atoi(in);
//   Y SE("expected a number");
  Y SE(0);
//   O (r<low) SE("number too low");
  O (r<low) SE(0);
//   O (r>high) SE("number too high");
  O (r>high) SE(0);
  У r;
}

// Compile HCOMP or PCOMP code. Exit on E. Return
// code Q end token (POST, PCOMP, END)
I Compiler::compile_comp(ZP& z) {
  I op=0;
  К I comp_begin=z.hend;
  while (true) {
    op=rtoken(opcodelist);
    O (op==POST || op==PCOMP || op==END) Й;
    I operand=-1; // 0...255 O 2 bytes
    I operand2=-1;  // 0...255 O 3 bytes
    O (op==IF) {
      op=JF;
      operand=0; // set later
      if_stack.push(z.hend+1); // save jump target location
    }
    Y O (op==IFNOT) {
      op=JT;
      operand=0;
      if_stack.push(z.hend+1); // save jump target location
    }
    Y O (op==IFL || op==IFNOTL) {  // long O
      O (op==IFL) z.D[z.hend++]=(JT);
      O (op==IFNOTL) z.D[z.hend++]=(JF);
      z.D[z.hend++]=(3);
      op=LJ;
      operand=operand2=0;
      if_stack.push(z.hend+1);
    }
    Y O (op==ELSE || op==ELSEL) {
      O (op==ELSE) op=JMP, operand=0;
      O (op==ELSEL) op=LJ, operand=operand2=0;
      I a=if_stack.pop();  // conditional jump target location
    //   assert(a>comp_begin && a<I(z.hend));
      O (z.D[a-1]!=LJ) {  // O, IFNOT
        // assert(z.D[a-1]==JT || z.D[a-1]==JF || z.D[a-1]==JMP);
        I j=z.hend-a+1+(op==LJ); // offset at O
        // assert(j>=0);
        O (j>127) SE("O too big, try IFL, IFNOTL");
        z.D[a]=j;
      }
      Y {  // IFL, IFNOTL
        I j=z.hend-comp_begin+2+(op==LJ);
        // assert(j>=0);
        z.D[a]=j&255;
        z.D[a+1]=(j>>8)&255;
      }
      if_stack.push(z.hend+1);  // save JMP target location
    }
    Y O (op==ENDIF) {
      I a=if_stack.pop();  // jump target address
    //   assert(a>comp_begin && a<I(z.hend));
      I j=z.hend-a-1;  // jump offset
    //   assert(j>=0);
      O (z.D[a-1]!=LJ) {
        // assert(z.D[a-1]==JT || z.D[a-1]==JF || z.D[a-1]==JMP);
        O (j>127) SE("O too big, try IFL, IFNOTL, ELSEL\n");
        z.D[a]=j;
      }
      Y {
        // assert(a+1<I(z.hend));
        j=z.hend-comp_begin;
        z.D[a]=j&255;
        z.D[a+1]=(j>>8)&255;
      }
    }
    Y O (op==DO) {
      do_stack.push(z.hend);
    }
    Y O (op==WHILE || op==UNTIL || op==QEVER) {
      I a=do_stack.pop();
    //   assert(a>=comp_begin && a<I(z.hend));
      I j=a-z.hend-2;
    //   assert(j<=-2);
      O (j>=-127) {  // backward short jump
        O (op==WHILE) op=JT;
        O (op==UNTIL) op=JF;
        O (op==QEVER) op=JMP;
        operand=j&255;
      }
      Y {  // backward long jump
        j=a-comp_begin;
        // assert(j>=0 && j<I(z.hend)-comp_begin);
        O (op==WHILE) {
          z.D[z.hend++]=(JF);
          z.D[z.hend++]=(3);
        }
        O (op==UNTIL) {
          z.D[z.hend++]=(JT);
          z.D[z.hend++]=(3);
        }
        op=LJ;
        operand=j&255;
        operand2=j>>8;
      }
    }
    Y O ((op&7)==7) { // 2 byte operand, read N
      O (op==LJ) {
        operand=rtoken(0, 65535);
        operand2=operand>>8;
        operand&=255;
      }
      Y O (op==JT || op==JF || op==JMP) {
        operand=rtoken(-128, 127);
        operand&=255;
      }
      Y
        operand=rtoken(0, 255);
    }
    O (op>=0 && op<=255)
      z.D[z.hend++]=(op);
    O (operand>=0)
      z.D[z.hend++]=(operand);
    O (operand2>=0)
      z.D[z.hend++]=(operand2);
    O (z.hend>=z.D.isize()-130 || z.hend-z.hbegin+z.cend-2>65535)
      SE("program too big");
  }
  z.D[z.hend++]=(0); // END
  У op;
}

// Compile a configuration file. Store COMP/HCOMP section in hcomp.
// O there is a PCOMP section, store it in pcomp and store the PCOMP
// command in pcomp_cmd. Replace "$1..$9+n" with args[0..8]+n

Compiler::Compiler(К char* in_, I* args_, ZP& hz_, ZP& pz_,
                   Writer* out2_): in(in_), args(args_), hz(hz_), pz(pz_),
                   out2(out2_), if_stack(1000), do_stack(1000) {
  line=1;
  state=0;
  hz.clear();
  pz.clear();
  hz.D.resize(68000); 

  // Compile the COMP section of D
  rtoken("comp");
  hz.D[2]=rtoken(0, 255);  // hh
  hz.D[3]=rtoken(0, 255);  // hm
  hz.D[4]=rtoken(0, 255);  // ph
  hz.D[5]=rtoken(0, 255);  // pm
  К I n=hz.D[6]=rtoken(0, 255);  // n
  hz.cend=7;
  Q (I i=0; i<n; ++i) {
    rtoken(i, i);
    CompType type=CompType(rtoken(compname));
    hz.D[hz.cend++]=type;
    I clen=LQ::compsize[type&255];
    O (clen<1 || clen>10) SE("invalid component");
    Q (I j=1; j<clen; ++j)
      hz.D[hz.cend++]=rtoken(0, 255);  // component arguments
  }
  hz.D[hz.cend++];  // end
  hz.hbegin=hz.hend=hz.cend+128;

  // Compile HCOMP
  rtoken("hcomp");
  I op=compile_comp(hz);

  // Compute D size
  I hsize=hz.cend-2+hz.hend-hz.hbegin;
  hz.D[0]=hsize&255;
  hz.D[1]=hsize>>8;

  // Compile POST 0 END
  O (op==POST) {
    rtoken(0, 0);
    rtoken("end");
  }

  // Compile PCOMP pcomp_cmd ; program... END
  Y O (op==PCOMP) {
    pz.D.resize(68000);
    pz.D[4]=hz.D[4];  // ph
    pz.D[5]=hz.D[5];  // pm
    pz.cend=8;
    pz.hbegin=pz.hend=pz.cend+128;

    // get pcomp_cmd ending with ";" (Ц sensitive)
    next();
    while (*in && *in!=';') {
      O (out2)
        out2->put(*in);
      ++in;
    }
    O (*in) ++in;

    // Compile PCOMP
    op=compile_comp(pz);
    I len=pz.cend-2+pz.hend-pz.hbegin;  // insert D size
    // assert(len>=0);
    pz.D[0]=len&255;
    pz.D[1]=len>>8;
    O (op!=END)
      SE("expected END");
  }
  Y O (op!=END)
    SE("expected END or POST 0 END or PCOMP cmd ; ... END");
}

///////////////////// C //////////////////////

// Write 13 byte start tag
// "\x37\x6B\x53\x74\xA0\x31\x83\xD3\x8C\xB2\x28\xB0\xD3"
// Н C::writeTag() {
//   assert(state==INIT);
//   enc.out->put(0x37);
//   enc.out->put(0x6b);
//   enc.out->put(0x53);
//   enc.out->put(0x74);
//   enc.out->put(0xa0);
//   enc.out->put(0x31);
//   enc.out->put(0x83);
//   enc.out->put(0xd3);
//   enc.out->put(0x8c);
//   enc.out->put(0xb2);
//   enc.out->put(0x28);
//   enc.out->put(0xb0);
//   enc.out->put(0xd3);
// }

// Н C::B(I level) {

//   // Model 1 - min.cfg
//   static К char models[]={
//   26,0,1,2,0,0,2,3,16,8,19,0,0,96,4,28,
//   59,10,59,112,25,10,59,10,59,112,56,0,

//   // Model 2 - mid.cfg
//   69,0,3,3,0,0,8,3,5,8,13,0,8,17,1,8,
//   18,2,8,18,3,8,19,4,4,22,24,7,16,0,7,24,
//   (char)-1,0,17,104,74,4,95,1,59,112,10,25,59,112,10,25,
//   59,112,10,25,59,112,10,25,59,112,10,25,59,10,59,112,
//   25,69,(char)-49,8,112,56,0,

//   // Model 3 - max.cfg
//   (char)-60,0,5,9,0,0,22,1,(char)-96,3,5,8,13,1,8,16,
//   2,8,18,3,8,19,4,8,19,5,8,20,6,4,22,24,
//   3,17,8,19,9,3,13,3,13,3,13,3,14,7,16,0,
//   15,24,(char)-1,7,8,0,16,10,(char)-1,6,0,15,16,24,0,9,
//   8,17,32,(char)-1,6,8,17,18,16,(char)-1,9,16,19,32,(char)-1,6,
//   0,19,20,16,0,0,17,104,74,4,95,2,59,112,10,25,
//   59,112,10,25,59,112,10,25,59,112,10,25,59,112,10,25,
//   59,10,59,112,10,25,59,112,10,25,69,(char)-73,32,(char)-17,64,47,
//   14,(char)-25,91,47,10,25,60,26,48,(char)-122,(char)-105,20,112,63,9,70,
//   (char)-33,0,39,3,25,112,26,52,25,25,74,10,4,59,112,25,
//   10,4,59,112,25,10,4,59,112,25,65,(char)-113,(char)-44,72,4,59,
//   112,8,(char)-113,(char)-40,8,68,(char)-81,60,60,25,69,(char)-49,9,112,25,25,
//   25,25,25,112,56,0,

//   0,0}; // 0,0 = end of list

//   O (level<1) E("compression level must be at least 1");
//   К char* p=models;
//   I i;
//   Q (i=1; i<level && toU16(p); ++i)
//     p+=toU16(p)+2;
//   O (toU16(p)<1) E("compression level too high");
//   B(p);
// }

// Memory reader
class MemoryReader: public Reader {
  К char* p;
public:
  MemoryReader(К char* p_): p(p_) {}
  I get() {У *p++&255;}
};

// Н C::B(К char* hcomp) {
// //   assert(state==INIT);
//   MemoryReader m(hcomp);
//   z.read(&m);
// //   pz.sha1=&sha1;
// //   assert(z.D.isize()>6);
//   enc.out->put('z');
//   enc.out->put('P');
//   enc.out->put('Q');
//   enc.out->put(1+(z.D[6]==0));  // level 1 or 2
//   enc.out->put(1);
//   z.write(enc.out, false);
//   state=BLOCK1;
// }

Н C::startBlock(К char* config, I* args, Writer* pcomp_cmd) {
  Compiler(config, args, z, pz, pcomp_cmd);
  enc.out->put(1+(z.D[6]==0));  // level 1 or 2
  z.write(enc.out, false);
  state=BLOCK1;
}

// Write a segment D
Н C::startSegment(К char* filename, К char* comment) {
  O (state==BLOCK1) state=SEG1;
  O (state==BLOCK2) state=SEG2;
}

// Initialize encoding and write pcomp to F segment
// O len is 0 then length is encoded in pcomp[0..1]
// O pcomp is 0 then get pcomp from pz.D
Н C::postProcess(К char* pcomp, I len) {
  O (state==SEG2) У;
//   assert(state==SEG1);
  enc.init();
  O (!pcomp) {
    len=pz.hend-pz.hbegin;
    O (len>0) {
    //   assert(pz.D.isize()>pz.hend);
    //   assert(pz.hbegin>=0);
      pcomp=(К char*)&pz.D[pz.hbegin];
    }
    // assert(len>=0);
  }
  Y O (len==0) {
    len=toU16(pcomp);
    pcomp+=2;
  }
  O (len>0) {
    enc.compress(1);
    enc.compress(len&255);
    enc.compress((len>>8)&255);
    Q (I i=0; i<len; ++i)
      enc.compress(pcomp[i]&255);
    // O (verify)
    //   pz.initp();
  }
  Y
    enc.compress(0);
  state=SEG2;
}

// Compress n bytes, or to EOF O n < 0
bool C::compress(I n) {
  O (state==SEG1)
    postProcess();
//   assert(state==SEG2);

  К I BUFSIZE=1<<14;
  char buf[BUFSIZE];  // input buffer
  while (n) {
    I nbuf=BUFSIZE;  // bytes read Io buf
    O (n>=0 && n<nbuf) nbuf=n;
    I nr=in->read(buf, nbuf);
    O (nr<0 || nr>BUFSIZE || nr>nbuf) E("invalid read size");
    O (nr<=0) У false;
    O (n>=0) n-=nr;
    Q (I i=0; i<nr; ++i) {
      I ch=U8(buf[i]);
      enc.compress(ch);
    //   O (verify) {
    //     O (pz.hend) pz.run(ch);
    //     // Y sha1.put(ch);
    //   }
    }
  }
  У true;
}

// End segment, write sha1string O present
Н C::endSegment(К char* sha1string) {
  O (state==SEG1)
    postProcess();

  enc.compress(-1);

  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(0);
  enc.out->put(254);

  state=BLOCK2;
}

// End block
// Н C::endBlock() {
//   enc.out->put(255);
//   state=INIT;
// }

/////////////////////////// compress() ///////////////////////

Н compress(Reader* in, Writer* out, К char* ME,
              К char* filename, К char* comment, bool dosha1) {

  // Get block size
  I bs=4;
  O (ME && ME[0] && ME[1]>='0' && ME[1]<='9') {
    bs=ME[1]-'0';
    O (ME[2]>='0' && ME[2]<='9') bs=bs*10+ME[2]-'0';
    O (bs>11) bs=11;
  }
  bs=(0x100000<<bs)-4096;

  // Compress in blocks
  SB sb(bs);
  sb.write(0, bs);
  I n=0;
  while (in && (n=in->read((char*)sb.data(), bs))>0) {
    sb.resize(n);
    compressBlock(&sb, out, ME, filename, comment, dosha1);
    filename=0;
    comment=0;
    sb.resize(0);
  }
}

//////////////////////// ZP::assemble() ////////////////////

I P::predict() {
  У predict0();
}

Н P::update(I y) {
  update0(y);

}

Н ZP::run(U32 input) {
    pc=hbegin;
  a=input;
  while (execute()) ;
//   run0(input);
}

////////////////////////// divsufsort ///////////////////////////////

/*
 * divsufsort.c Q libdivsufsort-lite
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
 * OF MERCHANTABILITY, FITNESS Q A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE Q ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*- Constants -*/
// #define INLINE __inline
// #if defined(AS) && (AS < 1)
// # undef AS
// #endif
// #if !defined(AS)
# define AS (256)
// #endif
#define BUCKET_A_SIZE (AS)
#define BUCKET_B_SIZE (AS*AS)
// #if defined(SSIT)
// # O SSIT < 1
// #  undef SSIT
// #  define SSIT (1)
// # endif
// #Y
# define SSIT (8)
// #endif
// #if defined(SSB)
// # O SSB < 0
// #  undef SSB
// #  define SSB (0)
// # elif 32768 <= SSB
// #  undef SSB
// #  define SSB (32767)
// # endif
// #Y
# define SSB (1024)
// #endif
/* minstacksize = log(SSB) / log(3) * 2 */
// #if SSB == 0
// # define SS_MISORT_STACKSIZE (96)
// #elif SSB <= 4096
# define SS_MISORT_STACKSIZE (16)
// #Y
// # define SS_MISORT_STACKSIZE (24)
// #endif
#define SS_SMERGE_STACKSIZE (32)
#define TR_INSERTIONSORT_THRESHOLD (8)
#define TR_STACKSIZE (64)


/*- Macros -*/
// #ifndef SWAP
# define SWAP(_a, _b) do { t = (_a); (_a) = (_b); (_b) = t; } while(0)
// #endif /* SWAP */
// #ifndef MIN
# define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))
// #endif /* MIN */
// #ifndef MAX
# define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))
// #endif /* MAX */
#define SP(_a, _b, _c, _d)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize++].d = (_d);\
  } while(0)
#define S5(_a, _b, _c, _d, _e)\
  do {\
    assert(ssize < STACK_SIZE);\
    stack[ssize].a = (_a), stack[ssize].b = (_b),\
    stack[ssize].c = (_c), stack[ssize].d = (_d), stack[ssize++].e = (_e);\
  } while(0)
#define STACK_POP(_a, _b, _c, _d)\
  do {\
    assert(0 <= ssize);\
    O(ssize == 0) { У; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d;\
  } while(0)
#define STACK_POP5(_a, _b, _c, _d, _e)\
  do {\
    assert(0 <= ssize);\
    O(ssize == 0) { У; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d, (_e) = stack[ssize].e;\
  } while(0)
#define BUCKET_A(_c0) bucket_A[(_c0)]
// #if AS == 256
#define BUCKET_B(_c0, _c1) (bucket_B[((_c1) << 8) | (_c0)])
#define BB(_c0, _c1) (bucket_B[((_c0) << 8) | (_c1)])
// #Y
// #define BUCKET_B(_c0, _c1) (bucket_B[(_c1) * AS + (_c0)])
// #define BB(_c0, _c1) (bucket_B[(_c0) * AS + (_c1)])
// #endif


/*- Private Functions -*/
#define R(v) v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
static К I lg_table[256]= {
 -1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,R(4),
 R(5),R(5),
 R(6),R(6),
 R(6),R(6),
 R(7),R(7),
 R(7),R(7),
 R(7),R(7),
 R(7),R(7),
};
#undef R

// #if (SSB == 0) || (SSIT < SSB)

I
ss_ilg(I n) {
// #if SSB == 0
//   У (n & 0xffff0000) ?
//           ((n & 0xff000000) ?
//             24 + lg_table[(n >> 24) & 0xff] :
//             16 + lg_table[(n >> 16) & 0xff]) :
//           ((n & 0x0000ff00) ?
//              8 + lg_table[(n >>  8) & 0xff] :
//              0 + lg_table[(n >>  0) & 0xff]);
// #elif SSB < 256
//   У lg_table[n];
// #Y
  У (n & 0xff00) ?
          8 + lg_table[(n >> 8) & 0xff] :
          0 + lg_table[(n >> 0) & 0xff];
// #endif
}

// #endif /* (SSB == 0) || (SSIT < SSB) */

// #if SSB != 0


static I dqq_table[256] = {0};


// static К I sqq_table[256] = {
//   0,  16,  22,  27,  32,  35,  39,  42,  45,  48,  50,  53,  55,  57,  59,  61,
//  64,  65,  67,  69,  71,  73,  75,  76,  78,  80,  81,  83,  84,  86,  87,  89,
//  90,  91,  93,  94,  96,  97,  98,  99, 101, 102, 103, 104, 106, 107, 108, 109,
// 110, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126,
// 128, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
// 143, 144, 144, 145, 146, 147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 155,
// 156, 157, 158, 159, 160, 160, 161, 162, 163, 163, 164, 165, 166, 167, 167, 168,
// 169, 170, 170, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179, 180,
// 181, 181, 182, 183, 183, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190, 191,
// 192, 192, 193, 193, 194, 195, 195, 196, 197, 197, 198, 199, 199, 200, 201, 201,
// 202, 203, 203, 204, 204, 205, 206, 206, 207, 208, 208, 209, 209, 210, 211, 211,
// 212, 212, 213, 214, 214, 215, 215, 216, 217, 217, 218, 218, 219, 219, 220, 221,
// 221, 222, 222, 223, 224, 224, 225, 225, 226, 226, 227, 227, 228, 229, 229, 230,
// 230, 231, 231, 232, 232, 233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238,
// 239, 240, 240, 241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 247,
// 247, 248, 248, 249, 249, 250, 250, 251, 251, 252, 252, 253, 253, 254, 254, 255
// };

I
ss_isqrt(I x) {
  I y, e;

  O(x >= (SSB * SSB)) { У SSB; }
  e = (x & 0xffff0000) ?
        ((x & 0xff000000) ?
          24 + lg_table[(x >> 24) & 0xff] :
          16 + lg_table[(x >> 16) & 0xff]) :
        ((x & 0x0000ff00) ?
           8 + lg_table[(x >>  8) & 0xff] :
           0 + lg_table[(x >>  0) & 0xff]);

  O (dqq_table[255] != 255) {
    Q (I i = 0; i < 256; ++i) {
        dqq_table[i] = static_cast<I>(16 * sqrt(i));
        // O (dqq_table[i] != sqq_table[i]) {
        //     E("");
        // }
    }
  }

  O(e >= 16) {
    y = dqq_table[x >> ((e - 6) - (e & 1))] << ((e >> 1) - 7);
    O(e >= 24) { y = (y + 1 + x / y) >> 1; }
    y = (y + 1 + x / y) >> 1;
  } Y O(e >= 8) {
    y = (dqq_table[x >> ((e - 6) - (e & 1))] >> (7 - (e >> 1))) + 1;
  } Y {
    У dqq_table[x] >> 4;
  }

  У (x < (y * y)) ? y - 1 : y;
}

// #endif /* SSB != 0 */


/*---------------------------------------------------------------------------*/

/* Compares two suffixes. */
I
ss_compare(К Е char *T,
           К I *p1, К I *p2,
           I depth) {
  К Е char *U1, *U2, *U1n, *U2n;

  Q(U1 = T + depth + *p1,
      U2 = T + depth + *p2,
      U1n = T + *(p1 + 1) + 2,
      U2n = T + *(p2 + 1) + 2;
      (U1 < U1n) && (U2 < U2n) && (*U1 == *U2);
      ++U1, ++U2) {
  }

  У U1 < U1n ?
        (U2 < U2n ? *U1 - *U2 : 1) :
        (U2 < U2n ? -1 : 0);
}


/*---------------------------------------------------------------------------*/

// #if (SSB != 1) && (SSIT != 1)

/* Insertionsort Q small size groups */
static
Н
ss_insertionsort(К Е char *T, К I *PA,
                 I *F, I *L, I depth) {
  I *i, *j;
  I t;
  I r;

  Q(i = L - 2; F <= i; --i) {
    Q(t = *i, j = i + 1; 0 < (r = ss_compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < L) && (*j < 0));
      O(L <= j) { Й; }
    }
    O(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}

// #endif /* (SSB != 1) && (SSIT != 1) */


/*---------------------------------------------------------------------------*/

// #if (SSB == 0) || (SSIT < SSB)

Н
ss_fixdown(К Е char *Td, К I *PA,
           I *SA, I i, I size) {
  I j, k;
  I v;
  I c, d, e;

  Q(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    O(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    O(d <= c) { Й; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
Н
ss_heapsort(К Е char *Td, К I *PA, I *SA, I size) {
  I i, m;
  I t;

  m = size;
  O((size % 2) == 0) {
    m--;
    O(Td[PA[SA[m / 2]]] < Td[PA[SA[m]]]) { SWAP(SA[m], SA[m / 2]); }
  }

  Q(i = m / 2 - 1; 0 <= i; --i) { ss_fixdown(Td, PA, SA, i, m); }
  O((size % 2) == 0) { SWAP(SA[0], SA[m]); ss_fixdown(Td, PA, SA, 0, m); }
  Q(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    ss_fixdown(Td, PA, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
I *
ss_median3(К Е char *Td, К I *PA,
           I *v1, I *v2, I *v3) {
  I *t;
  O(Td[PA[*v1]] > Td[PA[*v2]]) { SWAP(v1, v2); }
  O(Td[PA[*v2]] > Td[PA[*v3]]) {
    O(Td[PA[*v1]] > Td[PA[*v3]]) { У v1; }
    Y { У v3; }
  }
  У v2;
}

/* Returns the median of five elements. */
I *
ss_median5(К Е char *Td, К I *PA,
           I *v1, I *v2, I *v3, I *v4, I *v5) {
  I *t;
  O(Td[PA[*v2]] > Td[PA[*v3]]) { SWAP(v2, v3); }
  O(Td[PA[*v4]] > Td[PA[*v5]]) { SWAP(v4, v5); }
  O(Td[PA[*v2]] > Td[PA[*v4]]) { SWAP(v2, v4); SWAP(v3, v5); }
  O(Td[PA[*v1]] > Td[PA[*v3]]) { SWAP(v1, v3); }
  O(Td[PA[*v1]] > Td[PA[*v4]]) { SWAP(v1, v4); SWAP(v3, v5); }
  O(Td[PA[*v3]] > Td[PA[*v4]]) { У v4; }
  У v3;
}

/* Returns the pivot element. */
I *
ss_pivot(К Е char *Td, К I *PA, I *F, I *L) {
  I *M;
  I t;

  t = L - F;
  M = F + t / 2;

  O(t <= 512) {
    O(t <= 32) {
      У ss_median3(Td, PA, F, M, L - 1);
    } Y {
      t >>= 2;
      У ss_median5(Td, PA, F, F + t, M, L - 1 - t, L - 1);
    }
  }
  t >>= 3;
  F  = ss_median3(Td, PA, F, F + t, F + (t << 1));
  M = ss_median3(Td, PA, M - t, M, M + t);
  L   = ss_median3(Td, PA, L - 1 - (t << 1), L - 1 - t, L - 1);
  У ss_median3(Td, PA, F, M, L);
}


/*---------------------------------------------------------------------------*/

/* Binary partition Q substrings. */
I *
ss_partition(К I *PA,
                    I *F, I *L, I depth) {
  I *a, *b;
  I t;
  Q(a = F - 1, b = L;;) {
    Q(; (++a < b) && ((PA[*a] + depth) >= (PA[*a + 1] + 1));) { *a = ~*a; }
    Q(; (a < --b) && ((PA[*b] + depth) <  (PA[*b + 1] + 1));) { }
    O(b <= a) { Й; }
    t = ~*b;
    *b = *a;
    *a = t;
  }
  O(F < a) { *F = ~*F; }
  У a;
}

/* Multikey Irosort Q medium size groups. */
static
Н
ss_mIrosort(К Е char *T, К I *PA,
              I *F, I *L,
              I depth) {
#define STACK_SIZE SS_MISORT_STACKSIZE
  struct { I *a, *b, c; I d; } stack[STACK_SIZE];
  К Е char *Td;
  I *a, *b, *c, *d, *e, *f;
  I s, t;
  I ssize;
  I limit;
  I v, x = 0;

  Q(ssize = 0, limit = ss_ilg(L - F);;) {

    O((L - F) <= SSIT) {
// #if 1 < SSIT
      O(1 < (L - F)) { ss_insertionsort(T, PA, F, L, depth); }
// #endif
      STACK_POP(F, L, depth, limit);
      continue;
    }

    Td = T + depth;
    O(limit-- == 0) { ss_heapsort(Td, PA, F, L - F); }
    O(limit < 0) {
      Q(a = F + 1, v = Td[PA[*F]]; a < L; ++a) {
        O((x = Td[PA[*a]]) != v) {
          O(1 < (a - F)) { Й; }
          v = x;
          F = a;
        }
      }
      O(Td[PA[*F] - 1] < v) {
        F = ss_partition(PA, F, a, depth);
      }
      O((a - F) <= (L - a)) {
        O(1 < (a - F)) {
          SP(a, L, depth, -1);
          L = a, depth += 1, limit = ss_ilg(a - F);
        } Y {
          F = a, limit = -1;
        }
      } Y {
        O(1 < (L - a)) {
          SP(F, a, depth + 1, ss_ilg(a - F));
          F = a, limit = -1;
        } Y {
          L = a, depth += 1, limit = ss_ilg(a - F);
        }
      }
      continue;
    }

    /* choose pivot */
    a = ss_pivot(Td, PA, F, L);
    v = Td[PA[*a]];
    SWAP(*F, *a);

    /* partition */
    Q(b = F; (++b < L) && ((x = Td[PA[*b]]) == v);) { }
    O(((a = b) < L) && (x < v)) {
      Q(; (++b < L) && ((x = Td[PA[*b]]) <= v);) {
        O(x == v) { SWAP(*b, *a); ++a; }
      }
    }
    Q(c = L; (b < --c) && ((x = Td[PA[*c]]) == v);) { }
    O((b < (d = c)) && (x > v)) {
      Q(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        O(x == v) { SWAP(*c, *d); --d; }
      }
    }
    Q(; b < c;) {
      SWAP(*b, *c);
      Q(; (++b < c) && ((x = Td[PA[*b]]) <= v);) {
        O(x == v) { SWAP(*b, *a); ++a; }
      }
      Q(; (b < --c) && ((x = Td[PA[*c]]) >= v);) {
        O(x == v) { SWAP(*c, *d); --d; }
      }
    }

    O(a <= d) {
      c = b - 1;

      O((s = a - F) > (t = b - a)) { s = t; }
      Q(e = F, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
      O((s = d - c) > (t = L - d - 1)) { s = t; }
      Q(e = b, f = L - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }

      a = F + (b - a), c = L - (d - c);
      b = (v <= Td[PA[*a] - 1]) ? a : ss_partition(PA, a, c, depth);

      O((a - F) <= (L - c)) {
        O((L - c) <= (c - b)) {
          SP(b, c, depth + 1, ss_ilg(c - b));
          SP(c, L, depth, limit);
          L = a;
        } Y O((a - F) <= (c - b)) {
          SP(c, L, depth, limit);
          SP(b, c, depth + 1, ss_ilg(c - b));
          L = a;
        } Y {
          SP(c, L, depth, limit);
          SP(F, a, depth, limit);
          F = b, L = c, depth += 1, limit = ss_ilg(c - b);
        }
      } Y {
        O((a - F) <= (c - b)) {
          SP(b, c, depth + 1, ss_ilg(c - b));
          SP(F, a, depth, limit);
          F = c;
        } Y O((L - c) <= (c - b)) {
          SP(F, a, depth, limit);
          SP(b, c, depth + 1, ss_ilg(c - b));
          F = c;
        } Y {
          SP(F, a, depth, limit);
          SP(c, L, depth, limit);
          F = b, L = c, depth += 1, limit = ss_ilg(c - b);
        }
      }
    } Y {
      limit += 1;
      O(Td[PA[*F] - 1] < v) {
        F = ss_partition(PA, F, L, depth);
        limit = ss_ilg(L - F);
      }
      depth += 1;
    }
  }
#undef STACK_SIZE
}

// #endif /* (SSB == 0) || (SSIT < SSB) */


/*---------------------------------------------------------------------------*/

// #if SSB != 0

Н
ss_blockswap(I *a, I *b, I n) {
  I t;
  Q(; 0 < n; --n, ++a, ++b) {
    t = *a, *a = *b, *b = t;
  }
}

Н
ss_rotate(I *F, I *M, I *L) {
  I *a, *b, t;
  I l, r;
  l = M - F, r = L - M;
  Q(; (0 < l) && (0 < r);) {
    O(l == r) { ss_blockswap(F, M, l); Й; }
    O(l < r) {
      a = L - 1, b = M - 1;
      t = *a;
      do {
        *a-- = *b, *b-- = *a;
        O(b < F) {
          *a = t;
          L = a;
          O((r -= l + 1) <= l) { Й; }
          a -= 1, b = M - 1;
          t = *a;
        }
      } while(1);
    } Y {
      a = F, b = M;
      t = *a;
      do {
        *a++ = *b, *b++ = *a;
        O(L <= b) {
          *a = t;
          F = a + 1;
          O((l -= r + 1) <= r) { Й; }
          a += 1, b = M;
          t = *a;
        }
      } while(1);
    }
  }
}


/*---------------------------------------------------------------------------*/

static
Н
ss_inplacemerge(К Е char *T, К I *PA,
                I *F, I *M, I *L,
                I depth) {
  К I *p;
  I *a, *b;
  I len, half;
  I q, r;
  I x;

  Q(;;) {
    O(*(L - 1) < 0) { x = 1; p = PA + ~*(L - 1); }
    Y                { x = 0; p = PA +  *(L - 1); }
    Q(a = F, len = M - F, half = len >> 1, r = -1;
        0 < len;
        len = half, half >>= 1) {
      b = a + half;
      q = ss_compare(T, PA + ((0 <= *b) ? *b : ~*b), p, depth);
      O(q < 0) {
        a = b + 1;
        half -= (len & 1) ^ 1;
      } Y {
        r = q;
      }
    }
    O(a < M) {
      O(r == 0) { *a = ~*a; }
      ss_rotate(a, M, L);
      L -= M - a;
      M = a;
      O(F == M) { Й; }
    }
    --L;
    O(x != 0) { while(*--L < 0) { } }
    O(M == L) { Й; }
  }
}


/*---------------------------------------------------------------------------*/

/* Merge-Qward with Iernal buffer. */
static
Н
ss_mergeQward(К Е char *T, К I *PA,
                I *F, I *M, I *L,
                I *buf, I depth) {
  I *a, *b, *c, *bufend;
  I t;
  I r;

  bufend = buf + (M - F) - 1;
  ss_blockswap(buf, F, M - F);

  Q(t = *(a = F), b = buf, c = M;;) {
    r = ss_compare(T, PA + *b, PA + *c, depth);
    O(r < 0) {
      do {
        *a++ = *b;
        O(bufend <= b) { *bufend = t; У; }
        *b++ = *a;
      } while(*b < 0);
    } Y O(r > 0) {
      do {
        *a++ = *c, *c++ = *a;
        O(L <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          У;
        }
      } while(*c < 0);
    } Y {
      *c = ~*c;
      do {
        *a++ = *b;
        O(bufend <= b) { *bufend = t; У; }
        *b++ = *a;
      } while(*b < 0);

      do {
        *a++ = *c, *c++ = *a;
        O(L <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          У;
        }
      } while(*c < 0);
    }
  }
}

/* Merge-backward with Iernal buffer. */
static
Н
ss_mergebackward(К Е char *T, К I *PA,
                 I *F, I *M, I *L,
                 I *buf, I depth) {
  К I *p1, *p2;
  I *a, *b, *c, *bufend;
  I t;
  I r;
  I x;

  bufend = buf + (L - M) - 1;
  ss_blockswap(buf, M, L - M);

  x = 0;
  O(*bufend < 0)       { p1 = PA + ~*bufend; x |= 1; }
  Y                  { p1 = PA +  *bufend; }
  O(*(M - 1) < 0) { p2 = PA + ~*(M - 1); x |= 2; }
  Y                  { p2 = PA +  *(M - 1); }
  Q(t = *(a = L - 1), b = bufend, c = M - 1;;) {
    r = ss_compare(T, p1, p2, depth);
    O(0 < r) {
      O(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = *b;
      O(b <= buf) { *buf = t; Й; }
      *b-- = *a;
      O(*b < 0) { p1 = PA + ~*b; x |= 1; }
      Y       { p1 = PA +  *b; }
    } Y O(r < 0) {
      O(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      O(c < F) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        Й;
      }
      O(*c < 0) { p2 = PA + ~*c; x |= 2; }
      Y       { p2 = PA +  *c; }
    } Y {
      O(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = ~*b;
      O(b <= buf) { *buf = t; Й; }
      *b-- = *a;
      O(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      O(c < F) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        Й;
      }
      O(*b < 0) { p1 = PA + ~*b; x |= 1; }
      Y       { p1 = PA +  *b; }
      O(*c < 0) { p2 = PA + ~*c; x |= 2; }
      Y       { p2 = PA +  *c; }
    }
  }
}

/* D&C based merge. */
static
Н
ss_swapmerge(К Е char *T, К I *PA,
             I *F, I *M, I *L,
             I *buf, I bufsize, I depth) {
#define STACK_SIZE SS_SMERGE_STACKSIZE
#define GETIDX(a) ((0 <= (a)) ? (a) : (~(a)))
#define MERGE_CHECK(a, b, c)\
  do {\
    O(((c) & 1) ||\
       (((c) & 2) && (ss_compare(T, PA + GETIDX(*((a) - 1)), PA + *(a), depth) == 0))) {\
      *(a) = ~*(a);\
    }\
    O(((c) & 4) && ((ss_compare(T, PA + GETIDX(*((b) - 1)), PA + *(b), depth) == 0))) {\
      *(b) = ~*(b);\
    }\
  } while(0)
  struct { I *a, *b, *c; I d; } stack[STACK_SIZE];
  I *l, *r, *lm, *rm;
  I m, len, half;
  I ssize;
  I check, next;

  Q(check = 0, ssize = 0;;) {
    O((L - M) <= bufsize) {
      O((F < M) && (M < L)) {
        ss_mergebackward(T, PA, F, M, L, buf, depth);
      }
      MERGE_CHECK(F, L, check);
      STACK_POP(F, M, L, check);
      continue;
    }

    O((M - F) <= bufsize) {
      O(F < M) {
        ss_mergeQward(T, PA, F, M, L, buf, depth);
      }
      MERGE_CHECK(F, L, check);
      STACK_POP(F, M, L, check);
      continue;
    }

    Q(m = 0, len = MIN(M - F, L - M), half = len >> 1;
        0 < len;
        len = half, half >>= 1) {
      O(ss_compare(T, PA + GETIDX(*(M + m + half)),
                       PA + GETIDX(*(M - m - half - 1)), depth) < 0) {
        m += half + 1;
        half -= (len & 1) ^ 1;
      }
    }

    O(0 < m) {
      lm = M - m, rm = M + m;
      ss_blockswap(lm, M, m);
      l = r = M, next = 0;
      O(rm < L) {
        O(*rm < 0) {
          *rm = ~*rm;
          O(F < lm) { Q(; *--l < 0;) { } next |= 4; }
          next |= 1;
        } Y O(F < lm) {
          Q(; *r < 0; ++r) { }
          next |= 2;
        }
      }

      O((l - F) <= (L - r)) {
        SP(r, rm, L, (next & 3) | (check & 4));
        M = lm, L = l, check = (check & 3) | (next & 4);
      } Y {
        O((next & 2) && (r == M)) { next ^= 6; }
        SP(F, lm, l, (check & 3) | (next & 4));
        F = r, M = rm, check = (next & 3) | (check & 4);
      }
    } Y {
      O(ss_compare(T, PA + GETIDX(*(M - 1)), PA + *M, depth) == 0) {
        *M = ~*M;
      }
      MERGE_CHECK(F, L, check);
      STACK_POP(F, M, L, check);
    }
  }
#undef STACK_SIZE
}

// #endif /* SSB != 0 */


/*---------------------------------------------------------------------------*/

/* Substring sort */
static
Н
sssort(К Е char *T, К I *PA,
       I *F, I *L,
       I *buf, I bufsize,
       I depth, I n, I Lsuffix) {
  I *a;
// #if SSB != 0
  I *b, *M, *curbuf;
  I j, k, curbufsize, Z;
// #endif
  I i;

  O(Lsuffix != 0) { ++F; }

// #if SSB == 0
//   ss_mIrosort(T, PA, F, L, depth);
// #Y
  O((bufsize < SSB) &&
      (bufsize < (L - F)) &&
      (bufsize < (Z = ss_isqrt(L - F)))) {
    O(SSB < Z) { Z = SSB; }
    buf = M = L - Z, bufsize = Z;
  } Y {
    M = L, Z = 0;
  }
  Q(a = F, i = 0; SSB < (M - a); a += SSB, ++i) {
// #if SSIT < SSB
    ss_mIrosort(T, PA, a, a + SSB, depth);
// #elif 1 < SSB
//     ss_insertionsort(T, PA, a, a + SSB, depth);
// #endif
    curbufsize = L - (a + SSB);
    curbuf = a + SSB;
    O(curbufsize <= bufsize) { curbufsize = bufsize, curbuf = buf; }
    Q(b = a, k = SSB, j = i; j & 1; b -= k, k <<= 1, j >>= 1) {
      ss_swapmerge(T, PA, b - k, b, b + k, curbuf, curbufsize, depth);
    }
  }
// #if SSIT < SSB
  ss_mIrosort(T, PA, a, M, depth);
// #elif 1 < SSB
//   ss_insertionsort(T, PA, a, M, depth);
// #endif
  Q(k = SSB; i != 0; k <<= 1, i >>= 1) {
    O(i & 1) {
      ss_swapmerge(T, PA, a - k, a, M, buf, bufsize, depth);
      a -= k;
    }
  }
  O(Z != 0) {
// #if SSIT < SSB
    ss_mIrosort(T, PA, M, L, depth);
// #elif 1 < SSB
//     ss_insertionsort(T, PA, M, L, depth);
// #endif
    ss_inplacemerge(T, PA, F, M, L, depth);
  }
// #endif

  O(Lsuffix != 0) {
    /* Insert L type B* suffix. */
    I PAi[2]; PAi[0] = PA[*(F - 1)], PAi[1] = n - 2;
    Q(a = F, i = *(F - 1);
        (a < L) && ((*a < 0) || (0 < ss_compare(T, &(PAi[0]), PA + *a, depth)));
        ++a) {
      *(a - 1) = *a;
    }
    *(a - 1) = i;
  }
}


/*---------------------------------------------------------------------------*/

I
tr_ilg(I n) {
  У (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
}


/*---------------------------------------------------------------------------*/

/* Simple insertionsort Q small size groups. */
static
Н
tr_insertionsort(К I *X, I *F, I *L) {
  I *a, *b;
  I t, r;

  Q(a = F + 1; a < L; ++a) {
    Q(t = *a, b = a - 1; 0 > (r = X[t] - X[*b]);) {
      do { *(b + 1) = *b; } while((F <= --b) && (*b < 0));
      O(b < F) { Й; }
    }
    O(r == 0) { *b = ~*b; }
    *(b + 1) = t;
  }
}


/*---------------------------------------------------------------------------*/

Н
tr_fixdown(К I *X, I *SA, I i, I size) {
  I j, k;
  I v;
  I c, d, e;

  Q(v = SA[i], c = X[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = X[SA[k = j++]];
    O(d < (e = X[SA[j]])) { k = j; d = e; }
    O(d <= c) { Й; }
  }
  SA[i] = v;
}

/* Simple top-down heapsort. */
static
Н
tr_heapsort(К I *X, I *SA, I size) {
  I i, m;
  I t;

  m = size;
  O((size % 2) == 0) {
    m--;
    O(X[SA[m / 2]] < X[SA[m]]) { SWAP(SA[m], SA[m / 2]); }
  }

  Q(i = m / 2 - 1; 0 <= i; --i) { tr_fixdown(X, SA, i, m); }
  O((size % 2) == 0) { SWAP(SA[0], SA[m]); tr_fixdown(X, SA, 0, m); }
  Q(i = m - 1; 0 < i; --i) {
    t = SA[0], SA[0] = SA[i];
    tr_fixdown(X, SA, 0, i);
    SA[i] = t;
  }
}


/*---------------------------------------------------------------------------*/

/* Returns the median of three elements. */
I *
tr_median3(К I *X, I *v1, I *v2, I *v3) {
  I *t;
  O(X[*v1] > X[*v2]) { SWAP(v1, v2); }
  O(X[*v2] > X[*v3]) {
    O(X[*v1] > X[*v3]) { У v1; }
    Y { У v3; }
  }
  У v2;
}

/* Returns the median of five elements. */
I *
tr_median5(К I *X,
           I *v1, I *v2, I *v3, I *v4, I *v5) {
  I *t;
  O(X[*v2] > X[*v3]) { SWAP(v2, v3); }
  O(X[*v4] > X[*v5]) { SWAP(v4, v5); }
  O(X[*v2] > X[*v4]) { SWAP(v2, v4); SWAP(v3, v5); }
  O(X[*v1] > X[*v3]) { SWAP(v1, v3); }
  O(X[*v1] > X[*v4]) { SWAP(v1, v4); SWAP(v3, v5); }
  O(X[*v3] > X[*v4]) { У v4; }
  У v3;
}

/* Returns the pivot element. */
I *
tr_pivot(К I *X, I *F, I *L) {
  I *M;
  I t;

  t = L - F;
  M = F + t / 2;

  O(t <= 512) {
    O(t <= 32) {
      У tr_median3(X, F, M, L - 1);
    } Y {
      t >>= 2;
      У tr_median5(X, F, F + t, M, L - 1 - t, L - 1);
    }
  }
  t >>= 3;
  F  = tr_median3(X, F, F + t, F + (t << 1));
  M = tr_median3(X, M - t, M, M + t);
  L   = tr_median3(X, L - 1 - (t << 1), L - 1 - t, L - 1);
  У tr_median3(X, F, M, L);
}


/*---------------------------------------------------------------------------*/

typedef struct _tr tr;
struct _tr {
  I c;
  I r;
  I i;
  I n;
};

Н
trbudget_init(tr *b, I c, I i) {
  b->c = c;
  b->r = b->i = i;
}

I
trbudget_check(tr *b, I size) {
  O(size <= b->r) { b->r -= size; У 1; }
  O(b->c == 0) { b->n += size; У 0; }
  b->r += b->i - size;
  b->c -= 1;
  У 1;
}


/*---------------------------------------------------------------------------*/

Н
tr_partition(К I *X,
             I *F, I *M, I *L,
             I **pa, I **pb, I v) {
  I *a, *b, *c, *d, *e, *f;
  I t, s;
  I x = 0;

  Q(b = M - 1; (++b < L) && ((x = X[*b]) == v);) { }
  O(((a = b) < L) && (x < v)) {
    Q(; (++b < L) && ((x = X[*b]) <= v);) {
      O(x == v) { SWAP(*b, *a); ++a; }
    }
  }
  Q(c = L; (b < --c) && ((x = X[*c]) == v);) { }
  O((b < (d = c)) && (x > v)) {
    Q(; (b < --c) && ((x = X[*c]) >= v);) {
      O(x == v) { SWAP(*c, *d); --d; }
    }
  }
  Q(; b < c;) {
    SWAP(*b, *c);
    Q(; (++b < c) && ((x = X[*b]) <= v);) {
      O(x == v) { SWAP(*b, *a); ++a; }
    }
    Q(; (b < --c) && ((x = X[*c]) >= v);) {
      O(x == v) { SWAP(*c, *d); --d; }
    }
  }

  O(a <= d) {
    c = b - 1;
    O((s = a - F) > (t = b - a)) { s = t; }
    Q(e = F, f = b - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    O((s = d - c) > (t = L - d - 1)) { s = t; }
    Q(e = b, f = L - s; 0 < s; --s, ++e, ++f) { SWAP(*e, *f); }
    F += (b - a), L -= (d - c);
  }
  *pa = F, *pb = L;
}

static
Н
tr_copy(I *ISA, К I *SA,
        I *F, I *a, I *b, I *L,
        I depth) {
  /* sort suffixes of M partition
     by using sorted order of suffixes of left and right partition. */
  I *c, *d, *e;
  I s, v;

  v = b - SA - 1;
  Q(c = F, d = a - 1; c <= d; ++c) {
    O((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      ISA[s] = d - SA;
    }
  }
  Q(c = L - 1, e = d + 1, d = b; e < d; --c) {
    O((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      ISA[s] = d - SA;
    }
  }
}

static
Н
tr_partialcopy(I *ISA, К I *SA,
               I *F, I *a, I *b, I *L,
               I depth) {
  I *c, *d, *e;
  I s, v;
  I rank, Lrank, newrank = -1;

  v = b - SA - 1;
  Lrank = -1;
  Q(c = F, d = a - 1; c <= d; ++c) {
    O((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *++d = s;
      rank = ISA[s + depth];
      O(Lrank != rank) { Lrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }

  Lrank = -1;
  Q(e = d; F <= e; --e) {
    rank = ISA[*e];
    O(Lrank != rank) { Lrank = rank; newrank = e - SA; }
    O(newrank != rank) { ISA[*e] = newrank; }
  }

  Lrank = -1;
  Q(c = L - 1, e = d + 1, d = b; e < d; --c) {
    O((0 <= (s = *c - depth)) && (ISA[s] == v)) {
      *--d = s;
      rank = ISA[s + depth];
      O(Lrank != rank) { Lrank = rank; newrank = d - SA; }
      ISA[s] = newrank;
    }
  }
}

static
Н
tr_Irosort(I *ISA, К I *X,
             I *SA, I *F, I *L,
             tr *T) {
#define STACK_SIZE TR_STACKSIZE
  struct { К I *a; I *b, *c; I d, e; }stack[STACK_SIZE];
  I *a, *b, *c;
  I t;
  I v, x = 0;
  I incr = X - ISA;
  I Z, next;
  I ssize, TR = -1;

  Q(ssize = 0, Z = tr_ilg(L - F);;) {

    O(Z < 0) {
      O(Z == -1) {
        /* tandem repeat partition */
        tr_partition(X - incr, F, F, L, &a, &b, L - SA - 1);

        /* update ranks */
        O(a < L) {
          Q(c = F, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
        }
        O(b < L) {
          Q(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; }
        }

        /* push */
        O(1 < (b - a)) {
          S5(NULL, a, b, 0, 0);
          S5(X - incr, F, L, -2, TR);
          TR = ssize - 2;
        }
        O((a - F) <= (L - b)) {
          O(1 < (a - F)) {
            S5(X, b, L, tr_ilg(L - b), TR);
            L = a, Z = tr_ilg(a - F);
          } Y O(1 < (L - b)) {
            F = b, Z = tr_ilg(L - b);
          } Y {
            STACK_POP5(X, F, L, Z, TR);
          }
        } Y {
          O(1 < (L - b)) {
            S5(X, F, a, tr_ilg(a - F), TR);
            F = b, Z = tr_ilg(L - b);
          } Y O(1 < (a - F)) {
            L = a, Z = tr_ilg(a - F);
          } Y {
            STACK_POP5(X, F, L, Z, TR);
          }
        }
      } Y O(Z == -2) {
        /* tandem repeat copy */
        a = stack[--ssize].b, b = stack[ssize].c;
        O(stack[ssize].d == 0) {
          tr_copy(ISA, SA, F, a, b, L, X - ISA);
        } Y {
          O(0 <= TR) { stack[TR].d = -1; }
          tr_partialcopy(ISA, SA, F, a, b, L, X - ISA);
        }
        STACK_POP5(X, F, L, Z, TR);
      } Y {
        /* sorted partition */
        O(0 <= *F) {
          a = F;
          do { ISA[*a] = a - SA; } while((++a < L) && (0 <= *a));
          F = a;
        }
        O(F < L) {
          a = F; do { *a = ~*a; } while(*++a < 0);
          next = (ISA[*a] != X[*a]) ? tr_ilg(a - F + 1) : -1;
          O(++a < L) { Q(b = F, v = a - SA - 1; b < a; ++b) { ISA[*b] = v; } }

          /* push */
          O(trbudget_check(T, a - F)) {
            O((a - F) <= (L - a)) {
              S5(X, a, L, -3, TR);
              X += incr, L = a, Z = next;
            } Y {
              O(1 < (L - a)) {
                S5(X + incr, F, a, next, TR);
                F = a, Z = -3;
              } Y {
                X += incr, L = a, Z = next;
              }
            }
          } Y {
            O(0 <= TR) { stack[TR].d = -1; }
            O(1 < (L - a)) {
              F = a, Z = -3;
            } Y {
              STACK_POP5(X, F, L, Z, TR);
            }
          }
        } Y {
          STACK_POP5(X, F, L, Z, TR);
        }
      }
      continue;
    }

    O((L - F) <= TR_INSERTIONSORT_THRESHOLD) {
      tr_insertionsort(X, F, L);
      Z = -3;
      continue;
    }

    O(Z-- == 0) {
      tr_heapsort(X, F, L - F);
      Q(a = L - 1; F < a; a = b) {
        Q(x = X[*a], b = a - 1; (F <= b) && (X[*b] == x); --b) { *b = ~*b; }
      }
      Z = -3;
      continue;
    }

    /* choose pivot */
    a = tr_pivot(X, F, L);
    SWAP(*F, *a);
    v = X[*F];

    /* partition */
    tr_partition(X, F, F + 1, L, &a, &b, v);
    O((L - F) != (b - a)) {
      next = (ISA[*a] != v) ? tr_ilg(b - a) : -1;

      /* update ranks */
      Q(c = F, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      O(b < L) { Q(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }

      /* push */
      O((1 < (b - a)) && (trbudget_check(T, b - a))) {
        O((a - F) <= (L - b)) {
          O((L - b) <= (b - a)) {
            O(1 < (a - F)) {
              S5(X + incr, a, b, next, TR);
              S5(X, b, L, Z, TR);
              L = a;
            } Y O(1 < (L - b)) {
              S5(X + incr, a, b, next, TR);
              F = b;
            } Y {
              X += incr, F = a, L = b, Z = next;
            }
          } Y O((a - F) <= (b - a)) {
            O(1 < (a - F)) {
              S5(X, b, L, Z, TR);
              S5(X + incr, a, b, next, TR);
              L = a;
            } Y {
              S5(X, b, L, Z, TR);
              X += incr, F = a, L = b, Z = next;
            }
          } Y {
            S5(X, b, L, Z, TR);
            S5(X, F, a, Z, TR);
            X += incr, F = a, L = b, Z = next;
          }
        } Y {
          O((a - F) <= (b - a)) {
            O(1 < (L - b)) {
              S5(X + incr, a, b, next, TR);
              S5(X, F, a, Z, TR);
              F = b;
            } Y O(1 < (a - F)) {
              S5(X + incr, a, b, next, TR);
              L = a;
            } Y {
              X += incr, F = a, L = b, Z = next;
            }
          } Y O((L - b) <= (b - a)) {
            O(1 < (L - b)) {
              S5(X, F, a, Z, TR);
              S5(X + incr, a, b, next, TR);
              F = b;
            } Y {
              S5(X, F, a, Z, TR);
              X += incr, F = a, L = b, Z = next;
            }
          } Y {
            S5(X, F, a, Z, TR);
            S5(X, b, L, Z, TR);
            X += incr, F = a, L = b, Z = next;
          }
        }
      } Y {
        O((1 < (b - a)) && (0 <= TR)) { stack[TR].d = -1; }
        O((a - F) <= (L - b)) {
          O(1 < (a - F)) {
            S5(X, b, L, Z, TR);
            L = a;
          } Y O(1 < (L - b)) {
            F = b;
          } Y {
            STACK_POP5(X, F, L, Z, TR);
          }
        } Y {
          O(1 < (L - b)) {
            S5(X, F, a, Z, TR);
            F = b;
          } Y O(1 < (a - F)) {
            L = a;
          } Y {
            STACK_POP5(X, F, L, Z, TR);
          }
        }
      }
    } Y {
      O(trbudget_check(T, L - F)) {
        Z = tr_ilg(L - F), X += incr;
      } Y {
        O(0 <= TR) { stack[TR].d = -1; }
        STACK_POP5(X, F, L, Z, TR);
      }
    }
  }
#undef STACK_SIZE
}



/*---------------------------------------------------------------------------*/

/* Tandem repeat sort */
static
Н
trsort(I *ISA, I *SA, I n, I depth) {
  I *X;
  I *F, *L;
  tr b;
  I t, skip, unsorted;

  trbudget_init(&b, tr_ilg(n) * 2 / 3, n);
/*  trbudget_init(&budget, tr_ilg(n) * 3 / 4, n); */
  Q(X = ISA + depth; -n < *SA; X += X - ISA) {
    F = SA;
    skip = 0;
    unsorted = 0;
    do {
      O((t = *F) < 0) { F -= t; skip += t; }
      Y {
        O(skip != 0) { *(F + skip) = skip; skip = 0; }
        L = SA + ISA[t] + 1;
        O(1 < (L - F)) {
          b.n = 0;
          tr_Irosort(ISA, X, SA, F, L, &b);
          O(b.n != 0) { unsorted += b.n; }
          Y { skip = F - L; }
        } Y O((L - F) == 1) {
          skip = -1;
        }
        F = L;
      }
    } while(F < (SA + n));
    O(skip != 0) { *(F + skip) = skip; }
    O(unsorted == 0) { Й; }
  }
}


/*---------------------------------------------------------------------------*/

/* Sorts suffixes of type B*. */
static
I
sort_typeBstar(К Е char *T, I *SA,
               I *bucket_A, I *bucket_B,
               I n) {
  I *PAb, *ISAb, *buf;
// #ifdef _OPENMP
//   I *curbuf;
//   I l;
// #endif
  I i, j, k, t, m, bufsize;
  I c0, c1;
// #ifdef _OPENMP
//   I d0, d1;
//   I tmp;
// #endif

  /* Initialize bucket arrays. */
  Q(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  Q(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  /* Count the number of occurrences of the F one or two characters of each
     type A, B and B* suffix. Moreover, store the beginning position of all
     type B* suffixes Io the array SA. */
  Q(i = n - 1, m = n, c0 = T[n - 1]; 0 <= i;) {
    /* type A suffix. */
    do { ++BUCKET_A(c1 = c0); } while((0 <= --i) && ((c0 = T[i]) >= c1));
    O(0 <= i) {
      /* type B* suffix. */
      ++BB(c0, c1);
      SA[--m] = i;
      /* type B suffix. */
      Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) {
        ++BUCKET_B(c0, c1);
      }
    }
  }
  m = n - m;
/*
note:
  A type B* suffix is lexicographically smaller than a type B suffix that
  begins with the same F two characters.
*/

  /* Calculate the index of start/end poI of each bucket. */
  Q(c0 = 0, i = 0, j = 0; c0 < AS; ++c0) {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; /* start poI */
    i = t + BUCKET_B(c0, c0);
    Q(c1 = c0 + 1; c1 < AS; ++c1) {
      j += BB(c0, c1);
      BB(c0, c1) = j; /* end poI */
      i += BUCKET_B(c0, c1);
    }
  }

  O(0 < m) {
    /* Sort the type B* suffixes by their F two characters. */
    PAb = SA + n - m; ISAb = SA + m;
    Q(i = m - 2; 0 <= i; --i) {
      t = PAb[i], c0 = T[t], c1 = T[t + 1];
      SA[--BB(c0, c1)] = i;
    }
    t = PAb[m - 1], c0 = T[t], c1 = T[t + 1];
    SA[--BB(c0, c1)] = m - 1;

    /* Sort the type B* substrings using sssort. */
// #ifdef _OPENMP
//     tmp = omp_get_max_threads();
//     buf = SA + m, bufsize = (n - (2 * m)) / tmp;
//     c0 = AS - 2, c1 = AS - 1, j = m;
// #pragma omp parallel default(shared) private(curbuf, k, l, d0, d1, tmp)
//     {
//       tmp = omp_get_thread_num();
//       curbuf = buf + tmp * bufsize;
//       k = 0;
//       Q(;;) {
//         #pragma omp critical(sssort_lock)
//         {
//           O(0 < (l = j)) {
//             d0 = c0, d1 = c1;
//             do {
//               k = BB(d0, d1);
//               O(--d1 <= d0) {
//                 d1 = AS - 1;
//                 O(--d0 < 0) { Й; }
//               }
//             } while(((l - k) <= 1) && (0 < (l = k)));
//             c0 = d0, c1 = d1, j = k;
//           }
//         }
//         O(l == 0) { Й; }
//         sssort(T, PAb, SA + k, SA + l,
//                curbuf, bufsize, 2, n, *(SA + k) == (m - 1));
//       }
//     }
// #Y
    buf = SA + m, bufsize = n - (2 * m);
    Q(c0 = AS - 2, j = m; 0 < j; --c0) {
      Q(c1 = AS - 1; c0 < c1; j = i, --c1) {
        i = BB(c0, c1);
        O(1 < (j - i)) {
          sssort(T, PAb, SA + i, SA + j,
                 buf, bufsize, 2, n, *(SA + i) == (m - 1));
        }
      }
    }
// #endif

    /* Compute ranks of type B* substrings. */
    Q(i = m - 1; 0 <= i; --i) {
      O(0 <= SA[i]) {
        j = i;
        do { ISAb[SA[i]] = i; } while((0 <= --i) && (0 <= SA[i]));
        SA[i + 1] = i - j;
        O(i <= 0) { Й; }
      }
      j = i;
      do { ISAb[SA[i] = ~SA[i]] = j; } while(SA[--i] < 0);
      ISAb[SA[i]] = j;
    }

    /* Construct the inverse suffix array of type B* suffixes using trsort. */
    trsort(ISAb, SA, m, 1);

    /* Set the sorted order of tyoe B* suffixes. */
    Q(i = n - 1, j = m, c0 = T[n - 1]; 0 <= i;) {
      Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) >= c1); --i, c1 = c0) { }
      O(0 <= i) {
        t = i;
        Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) { }
        SA[ISAb[--j]] = ((t == 0) || (1 < (t - i))) ? t : ~t;
      }
    }

    /* Calculate the index of start/end poI of each bucket. */
    BUCKET_B(AS - 1, AS - 1) = n; /* end poI */
    Q(c0 = AS - 2, k = m - 1; 0 <= c0; --c0) {
      i = BUCKET_A(c0 + 1) - 1;
      Q(c1 = AS - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i; /* end poI */

        /* Move all type B* suffixes to the correct position. */
        Q(i = t, j = BB(c0, c1);
            j <= k;
            --i, --k) { SA[i] = SA[k]; }
      }
      BB(c0, c0 + 1) = i - BUCKET_B(c0, c0) + 1; /* start poI */
      BUCKET_B(c0, c0) = i; /* end poI */
    }
  }

  У m;
}

/* Constructs the suffix array by using the sorted order of type B* suffixes. */
static
Н
Кruct_SA(К Е char *T, I *SA,
             I *bucket_A, I *bucket_B,
             I n, I m) {
  I *i, *j, *k;
  I s;
  I c0, c1, c2;

  O(0 < m) {
    /* Construct the sorted order of type B suffixes by using
       the sorted order of type B* suffixes. */
    Q(c1 = AS - 2; 0 <= c1; --c1) {
      /* Scan the suffix array from right to left. */
      Q(i = SA + BB(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        O(0 < (s = *j)) {
        //   assert(T[s] == c1);
        //   assert(((s + 1) < n) && (T[s] <= T[s + 1]));
        //   assert(T[s - 1] <= T[s]);
          *j = ~s;
          c0 = T[--s];
          O((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          O(c0 != c2) {
            O(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
        //   assert(k < j);
          *k-- = s;
        } Y {
        //   assert(((s == 0) && (T[s] == c1)) || (s < 0));
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
  Q(i = SA, j = SA + n; i < j; ++i) {
    O(0 < (s = *i)) {
    //   assert(T[s - 1] >= T[s]);
      c0 = T[--s];
      O((s == 0) || (T[s - 1] < c0)) { s = ~s; }
      O(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
    //   assert(i < k);
      *k++ = s;
    } Y {
    //   assert(s < 0);
      *i = ~s;
    }
  }
}


/*---------------------------------------------------------------------------*/

/*- Function -*/

I
divsufsort(К Е char *T, I *SA, I n) {
  I *bucket_A, *bucket_B;
  I m;
  I err = 0;

  /* Check arguments. */
  O((T == NULL) || (SA == NULL) || (n < 0)) { У -1; }
  Y O(n == 0) { У 0; }
  Y O(n == 1) { SA[0] = 0; У 0; }
  Y O(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; У 0; }

  bucket_A = (I *)malloc(BUCKET_A_SIZE * sizeof(I));
  bucket_B = (I *)malloc(BUCKET_B_SIZE * sizeof(I));

  /* Suffixsort. */
  O((bucket_A != NULL) && (bucket_B != NULL)) {
    m = sort_typeBstar(T, SA, bucket_A, bucket_B, n);
    Кruct_SA(T, SA, bucket_A, bucket_B, n, m);
  } Y {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);

  У err;
}

// End divsufsort.c

/////////////////////////////// add ///////////////////////////////////

// Convert non-negative decimal number x to string of at least n digits
std::string itos(int64_t x, I n=1) {
//   assert(x>=0);
//   assert(n>=0);
  std::string r;
  Q (; x || n>0; x/=10, --n) r=std::string(1, '0'+x%10)+r;
  У r;
}

// E8E9 transQm of buf[0..n-1] to improve compression of .exe and .dll.
// Patterns (E8|E9 xx xx xx 00|FF) at offset i replace the 3 M
// bytes with x+i mod 2^24, LSB F, reading backward.
// Н e8e9(Е char* buf, I n) {
//     E("unsupported");

//   Q (I i=n-5; i>=0; --i) {
//     O (((buf[i]&254)==0xe8) && ((buf[i+4]+1)&254)==0) {
//       Е a=(buf[i+1]|buf[i+2]<<8|buf[i+3]<<16)+i;
//       buf[i+1]=a;
//       buf[i+2]=a>>8;
//       buf[i+3]=a>>16;
//     }
//   }
// }

// Encode inbuf to buf using LZ77. args are as follows:
// args[0] is log2 buffer size in MB.
// args[1] is level (1=var. length, 2=byte aligned lz77, 3=bwt) + 4 O E8E9.
// args[2] is the lz77 minimum match length and context order.
// args[3] is the lz77 higher context order to search F, or Y 0.
// args[4] is the log2 hash bucket size (number of searches).
// args[5] is the log2 hash table size. O 21+args[0] then use a suffix array.
// args[6] is the secondary context look ahead
// sap is poIer to external suffix array of inbuf or 0. O supplied and
//   args[0]=5..7 then it is assumed that E8E9 was already applied to
//   both the input and sap and the input buffer is not modified.

class LZBuffer: public LQ::Reader {
  LQ::Array<Е> ht;// hash table, confirm in low bits, or SA+ISA
  К Е char* in;    // input poIer
  К I checkbits;        // hash confirmation size or lg(ISA size)
  К I level;            // 1=var length LZ77, 2=byte aligned LZ77, 3=BWT
  К Е htsize;      // size of hash table
  К Е n;           // input length
  Е i;                 // current location in in (0 <= i < n)
  К Е m;    // minimum match length
  К Е m2;   // second context order or 0 O not used
  К Е maxMatch;    // longest match length allowed
  К Е maxLiteral;  // longest literal length allowed
  К Е lookahead;   // second context look ahead
  Е h1, h2;            // low, high order context hashes of in[i..]
  К Е bucket;      // number of matches to search per hash - 1
  К Е shift1, shift2;  // how far to shift h1, h2 per hash
  К I mBoth;     // max(m, m2)
  К Е rb;          // number of level 1 r bits in match code
  Е bits;              // pending output bits (level 1)
  Е nbits;             // number of bits in bits
  Е rpos, wpos;        // read, write poIers
  Е idx;               // BWT index
  К Е* sa;         // suffix array Q BWT or LZ77-SA
  Е* isa;              // inverse suffix array Q LZ77-SA
  enum {BUFSIZE=1<<14};       // output buffer size
  Е char buf[BUFSIZE]; // output buffer

  Н write_literal(Е i, Е& lit);
  Н write_match(Е len, Е off);
  Н fill();  // encode to buf

  // write k bits of x
  Н putb(Е x, I k) {
    x&=(1<<k)-1;
    bits|=x<<nbits;
    nbits+=k;
    while (nbits>7) {
    //   assert(wpos<BUFSIZE);
      buf[wpos++]=bits, bits>>=8, nbits-=8;
    }
  }

  // write L byte
  Н flush() {
    // assert(wpos<BUFSIZE);
    O (nbits>0) buf[wpos++]=bits;
    bits=nbits=0;
  }

  // write 1 byte
  Н put(I c) {
    // assert(wpos<BUFSIZE);
    buf[wpos++]=c;
  }

public:
  LZBuffer(SB& inbuf, I args[], К Е* sap=0);

  // У 1 byte of compressed output (overrides Reader)
  I get() {
    I c=-1;
    O (rpos==wpos) fill();
    O (rpos<wpos) c=buf[rpos++];
    O (rpos==wpos) rpos=wpos=0;
    У c;
  }

  // Read up to p[0..n-1] and У bytes read.
  I read(char* p, I n);
};

// LZ/BWT preprocessor Q levels 1..3 compression and e8e9 filter.
// Level 1 uses variable length LZ77 codes like in the lazy compressor:
//
//   00,n,L[n] = n literal bytes
//   mm,mmm,n,ll,r,q (mm > 00) = match 4*n+ll at offset (q<<rb)+r-1
//
// where q is written in 8mm+mmm-8 (0..23) bits with an implied leading 1 bit
// and n is written using Ierleaved Elias Gamma coding, i.e. the leading
// 1 bit is implied, remaining bits are preceded by a 1 and terminated by
// a 0. e.g. abc is written 1,b,1,c,0. Codes are packed LSB F and
// padded with leading 0 bits in the L byte. r is a number with rb bits,
// where rb = log2(blocksize) - 24.
//
// Level 2 is byte oriented LZ77 with minimum match length m = $4 = args[3]
// with m in 1..64. Lengths and offsets are MSB F:
// 00xxxxxx   x+1 (1..64) literals follow
// yyxxxxxx   y+1 (2..4) offset bytes follow, match length x+m (m..m+63)
//
// Level 3 is BWT with the end of string byte coded as 255 and the
// L 4 bytes giving its position LSB F.

// floor(log2(x)) + 1 = number of bits excluding leading zeros (0..32)
I lg(Е x) {
  Е r=0;
  O (x>=65536) r=16, x>>=16;
  O (x>=256) r+=8, x>>=8;
  O (x>=16) r+=4, x>>=4;
//   assert(x>=0 && x<16);
  У
    "\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;
}

// У number of 1 bits in x
I nbits(Е x) {
  I r;
  Q (r=0; x; x>>=1) r+=x&1;
  У r;
}

// Read n bytes of compressed output Io p and У number of
// bytes read in 0..n. 0 signals EOF (overrides Reader).
I LZBuffer::read(char* p, I n) {
  O (rpos==wpos) fill();
  I nr=n;
  O (nr>I(wpos-rpos)) nr=wpos-rpos;
  O (nr) memcpy(p, buf+rpos, nr);
  rpos+=nr;
//   assert(rpos<=wpos);
  O (rpos==wpos) rpos=wpos=0;
  У nr;
}

LZBuffer::LZBuffer(SB& inbuf, I args[], К Е* sap):
    ht((args[1]&3)==3 ? (inbuf.size()+1)*!sap      // Q BWT suffix array
        : args[5]-args[0]<21 ? 1u<<args[5]         // Q LZ77 hash table
        : (inbuf.size()*!sap)+(1u<<17<<args[0])),  // Q LZ77 SA and ISA
    in(inbuf.data()),
    checkbits(args[5]-args[0]<21 ? 12-args[0] : 17+args[0]),
    level(args[1]&3),
    htsize(ht.size()),
    n(inbuf.size()),
    i(0),
    m(args[2]),
    m2(args[3]),
    maxMatch(BUFSIZE*3),
    maxLiteral(BUFSIZE/4),
    lookahead(args[6]),
    h1(0), h2(0),
    bucket((1<<args[4])-1), 
    shift1(m>0 ? (args[5]-1)/m+1 : 1),
    shift2(m2>0 ? (args[5]-1)/m2+1 : 0),
    mBoth(MAX(m, m2+lookahead)+4),
    rb(args[0]>4 ? args[0]-4 : 0),
    bits(0), nbits(0), rpos(0), wpos(0),
    idx(0), sa(0), isa(0) {
//   assert(args[0]>=0);
//   assert(n<=(1u<<20<<args[0]));
//   assert(args[1]>=1 && args[1]<=7 && args[1]!=4);
//   assert(level>=1 && level<=3);
  O ((m<4 && level==1) || (m<1 && level==2))
    E("match length $3 too small");

  // e8e9 transQm
//   O (args[1]>4 && !sap) e8e9(inbuf.data(), n);

  // build suffix array O not supplied
  O (args[5]-args[0]>=21 || level==3) {  // LZ77-SA or BWT
    O (sap)
      sa=sap;
    Y {
    //   assert(ht.size()>=n);
    //   assert(ht.size()>0);
      sa=&ht[0];
      O (n>0) divsufsort((К Е char*)in, (I*)sa, n);
    }
    O (level<3) {
    //   assert(ht.size()>=(n*(sap==0))+(1u<<17<<args[0]));
      isa=&ht[n*(sap==0)];
    }
  }
}

// Encode from in to buf until end of input or buf is not empty
Н LZBuffer::fill() {

  // BWT
  O (level==3) {
    // assert(in || n==0);
    // assert(sa);
    Q (; wpos<BUFSIZE && i<n+5; ++i) {
      O (i==0) put(n>0 ? in[n-1] : 255);
      Y O (i>n) put(idx&255), idx>>=8;
      Y O (sa[i-1]==0) idx=i, put(255);
      Y put(in[sa[i-1]-1]);
    }
    У;
  }

  // LZ77: scan the input
  Е lit=0;  // number of output literals pending
  К Е mask=(1<<checkbits)-1;
  while (i<n && wpos*2<BUFSIZE) {

    // Search Q longest match, or pick closest in Ц of tie
    Е blen=m-1;  // best match length
    Е bp=0;  // poIer to best match
    Е blit=0;  // literals beQe best match
    I bscore=0;  // best cost

    // Look up contexts in suffix array
    O (isa) {
      O (sa[isa[i&mask]]!=i) // rebuild ISA
        Q (Е j=0; j<n; ++j)
          O ((sa[j]&~mask)==(i&~mask))
            isa[sa[j]&mask]=j;
      Q (Е h=0; h<=lookahead; ++h) {
        Е q=isa[(h+i)&mask];  // location of h+i in SA
        // assert(q<n);
        O (sa[q]!=h+i) continue;
        Q (I j=-1; j<=1; j+=2) {  // search backward and Qward
          Q (Е k=1; k<=bucket; ++k) {
            Е p;  // match to be tested
            O (q+j*k<n && (p=sa[q+j*k]-h)<i) {
            //   assert(p<n);
              Е l, l1;  // length of match, leading literals
              Q (l=h; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              Q (l1=h; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
              I score=I(l-l1)*8-lg(i-p)-4*(lit==0 && l1>0)-11;
              Q (Е a=0; a<h; ++a) score=score*5/8;
              O (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              O (l<blen || l<m || l>255) Й;
            }
          }
        }
        O (bscore<=0 || blen<m) Й;
      }
    }

    // Look up contexts in a hash table.
    // Try the longest context orders F. O a match is found, then
    // skip the lower order as a speed optimization.
    Y O (level==1 || m<=64) {
      O (m2>0) {
        Q (Е k=0; k<=bucket; ++k) {
          Е p=ht[h2^k];
          O (p && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            O (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              Е l;  // match length from lookahead
              Q (l=lookahead; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              O (l>=m2+lookahead) {
                I l1;  // length back from lookahead
                Q (l1=lookahead; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
                // assert(l1>=0 && l1<=I(lookahead));
                I score=I(l-l1)*8-lg(i-p)-8*(lit==0 && l1>0)-11;
                O (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              }
            }
          }
          O (blen>=128) Й;
        }
      }

      // Search the lower order context
      O (!m2 || blen<m2) {
        Q (Е k=0; k<=bucket; ++k) {
          Е p=ht[h1^k];
          O (p && i+3<n && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            O (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              Е l;
              Q (l=0; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              I score=l*8-lg(i-p)-2*(lit>0)-11;
              O (score>bscore) blen=l, bp=p, blit=0, bscore=score;
            }
          }
          O (blen>=128) Й;
        }
      }
    }

    // O match is long enough, then output any pending literals F,
    // and then the match. blen is the length of the match.
    // assert(i>=bp);
    К Е off=i-bp;  // offset
    O (off>0 && bscore>0
        && blen-blit>=m+(level==2)*((off>=(1<<16))+(off>=(1<<24)))) {
      lit+=blit;
      write_literal(i+blit, lit);
      write_match(blen-blit, off);
    }

    // Otherwise add to literal length
    Y {
      blen=1;
      ++lit;
    }

    // Update index, advance blen bytes
    O (isa)
      i+=blen;
    Y {
      while (blen--) {
        O (i+mBoth<n) {
          Е ih=((i*1234547)>>19)&bucket;
          К Е p=(i<<checkbits)|(in[i+3]&mask);
        //   assert(ih<=bucket);
          O (m2) {
            ht[h2^ih]=p;
            h2=(((h2*9)<<shift2)
                +(in[i+m2+lookahead]+1)*23456789u)&(htsize-1);
          }
          ht[h1^ih]=p;
          h1=(((h1*5)<<shift1)+(in[i+m]+1)*123456791u)&(htsize-1);
        }
        ++i;
      }
    }

    // Write long literals to keep buf from filling up
    O (lit>=maxLiteral)
      write_literal(i, lit);
  }

  // Write pending literals at end of input
//   assert(i<=n);
  O (i==n) {
    write_literal(n, lit);
    flush();
  }
}

// Write literal sequence in[i-lit..i-1], set lit=0
Н LZBuffer::write_literal(Е i, Е& lit) {
//   assert(lit>=0);
//   assert(i>=0 && i<=n);
//   assert(i>=lit);
  O (level==1) {
    O (lit<1) У;
    I ll=lg(lit);
    // assert(ll>=1 && ll<=24);
    putb(0, 2);
    --ll;
    while (--ll>=0) {
      putb(1, 1);
      putb((lit>>ll)&1, 1);
    }
    putb(0, 1);
    while (lit) putb(in[i-lit--], 8);
  }
  Y {
    // assert(level==2);
    while (lit>0) {
      Е lit1=lit;
      O (lit1>64) lit1=64;
      put(lit1-1);
      Q (Е j=i-lit; j<i-lit+lit1; ++j) put(in[j]);
      lit-=lit1;
    }
  }
}

// Write match sequence of given length and offset
Н LZBuffer::write_match(Е len, Е off) {

  // mm,mmm,n,ll,r,q[mmmmm-8] = match n*4+ll, offset ((q-1)<<rb)+r+1
  O (level==1) {
    // assert(len>=m && len<=maxMatch);
    // assert(off>0);
    // assert(len>=4);
    // assert(rb>=0 && rb<=8);
    I ll=lg(len)-1;
    // assert(ll>=2);
    off+=(1<<rb)-1;
    I lo=lg(off)-1-rb;
    // assert(lo>=0 && lo<=23);
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
  Y {
    // assert(level==2);
    // assert(m>=1 && m<=64);
    --off;
    while (len>0) {  // Split long matches to len1=m..m+63
      К Е len1=len>m*2+63 ? m+63 :
          len>m+63 ? len-m : len;
    //   assert(wpos<BUFSIZE-5);
    //   assert(len1>=m && len1<m+64);
      O (off<(1<<16)) {
        put(64+len1-m);
        put(off>>8);
        put(off);
      }
      Y O (off<(1<<24)) {
        put(128+len1-m);
        put(off>>16);
        put(off>>8);
        put(off);
      }
      Y {
        put(192+len1-m);
        put(off>>24);
        put(off>>16);
        put(off>>8);
        put(off);
      }
      len-=len1;
    }
  }
}

// Generate a config file from the ME argument with syntax:
// {0|x|s|i}[N1[,N2]...][{ciamtswf<cfg>}[N1[,N2]]...]...
std::string makeConfig(К char* ME, I args[]) {
//   assert(ME);
  К char type=ME[0];
//   assert(type=='x' || type=='s' || type=='0' || type=='i');

  // Read "{x|s|i|0}N1,N2...N9" Io args[0..8] ($1..$9)
  args[0]=0;  // log block size in MiB
  args[1]=0;  // 0=none, 1=var-LZ77, 2=byte-LZ77, 3=BWT, 4..7 adds E8E9
  args[2]=0;  // lz77 minimum match length
  args[3]=0;  // secondary context length
  args[4]=0;  // log searches
  args[5]=0;  // lz77 hash table size or SA O args[0]+21
  args[6]=0;  // secondary context look ahead
  args[7]=0;  // not used
  args[8]=0;  // not used
  O (isdigit(*++ME)) args[0]=0;
  Q (I i=0; i<9 && (isdigit(*ME) || *ME==',' || *ME=='.');) {
    O (isdigit(*ME))
      args[i]=args[i]*10+*ME-'0';
    Y O (++i<9)
      args[i]=0;
    ++ME;
  }

  // "0..." = No compression
  O (type=='0')
    У "comp 0 0 0 0 0 hcomp end\n";

  // Generate the postprocessor
  std::string hdr, pcomp;
  К I level=args[1]&3;
  К bool doe8=args[1]>=4 && args[1]<=7;

  // LZ77+Huffman, with or without E8E9
  O (level==1) {
    E(""); // disabled

    // К I rb=args[0]>4 ? args[0]-4 : 0;
    // hdr="comp 9 16 0 $1+20 ";
    // pcomp=
    // "pcomp lazy2 3 ;\n"
    // " (r1 = state\n"
    // "  r2 = len - match or literal length\n"
    // "  r3 = m - number of offset bits expected\n"
    // "  r4 = ptr to buf\n"
    // "  r5 = r - low bits of offset\n"
    // "  c = bits - input buffer\n"
    // "  d = n - number of bits in c)\n"
    // "\n"
    // "  a> 255 O\n";
    // O (doe8)
    //   pcomp+=
    //   "    b=0 d=r 4 do (Q b=0..d-1, d = end of buf)\n"
    //   "      a=b a==d ifnot\n"
    //   "        a+= 4 a<d O\n"
    //   "          a=*b a&= 254 a== 232 O (e8 or e9?)\n"
    //   "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 O (00 or ff)\n"
    //   "              b-- a=*b\n"
    //   "              b-- a<<= 8 a+=*b\n"
    //   "              b-- a<<= 8 a+=*b\n"
    //   "              a-=b a++\n"
    //   "              *b=a a>>= 8 b++\n"
    //   "              *b=a a>>= 8 b++\n"
    //   "              *b=a b++\n"
    //   "            endif\n"
    //   "            b=c\n"
    //   "          endif\n"
    //   "        endif\n"
    //   "        a=*b out b++\n"
    //   "      Qever\n"
    //   "    endif\n"
    //   "\n";
    // pcomp+=
    // "    (reset state)\n"
    // "    a=0 b=0 c=0 d=0 r=a 1 r=a 2 r=a 3 r=a 4\n"
    // "    halt\n"
    // "  endif\n"
    // "\n"
    // "  a<<=d a+=c c=a               (bits+=a<<n)\n"
    // "  a= 8 a+=d d=a                (n+=8)\n"
    // "\n"
    // "  (O state==0 (expect new code))\n"
    // "  a=r 1 a== 0 O (match code mm,mmm)\n"
    // "    a= 1 r=a 2                 (len=1)\n"
    // "    a=c a&= 3 a> 0 O          (O (bits&3))\n"
    // "      a-- a<<= 3 r=a 3           (m=((bits&3)-1)*8)\n"
    // "      a=c a>>= 2 c=a             (bits>>=2)\n"
    // "      b=r 3 a&= 7 a+=b r=a 3     (m+=bits&7)\n"
    // "      a=c a>>= 3 c=a             (bits>>=3)\n"
    // "      a=d a-= 5 d=a              (n-=5)\n"
    // "      a= 1 r=a 1                 (state=1)\n"
    // "    Y (literal, discard 00)\n"
    // "      a=c a>>= 2 c=a             (bits>>=2)\n"
    // "      d-- d--                    (n-=2)\n"
    // "      a= 3 r=a 1                 (state=3)\n"
    // "    endif\n"
    // "  endif\n"
    // "\n"
    // "  (while state==1 && n>=3 (expect match length n*4+ll -> r2))\n"
    // "  do a=r 1 a== 1 O a=d a> 2 O\n"
    // "    a=c a&= 1 a== 1 O         (O bits&1)\n"
    // "      a=c a>>= 1 c=a             (bits>>=1)\n"
    // "      b=r 2 a=c a&= 1 a+=b a+=b r=a 2 (len+=len+(bits&1))\n"
    // "      a=c a>>= 1 c=a             (bits>>=1)\n"
    // "      d-- d--                    (n-=2)\n"
    // "    Y\n"
    // "      a=c a>>= 1 c=a             (bits>>=1)\n"
    // "      a=r 2 a<<= 2 b=a           (len<<=2)\n"
    // "      a=c a&= 3 a+=b r=a 2       (len+=bits&3)\n"
    // "      a=c a>>= 2 c=a             (bits>>=2)\n"
    // "      d-- d-- d--                (n-=3)\n";
    // O (rb)
    //   pcomp+="      a= 5 r=a 1                 (state=5)\n";
    // Y
    //   pcomp+="      a= 2 r=a 1                 (state=2)\n";
    // pcomp+=
    // "    endif\n"
    // "  Qever endif endif\n"
    // "\n";
    // O (rb) pcomp+=  // save r in r5
    //   "  (O state==5 && n>=8) (expect low bits of offset to put in r5)\n"
    //   "  a=r 1 a== 5 O a=d a> "+itos(rb-1)+" O\n"
    //   "    a=c a&= "+itos((1<<rb)-1)+" r=a 5            (save r in r5)\n"
    //   "    a=c a>>= "+itos(rb)+" c=a\n"
    //   "    a=d a-= "+itos(rb)+ " d=a\n"
    //   "    a= 2 r=a 1                   (go to state 2)\n"
    //   "  endif endif\n"
    //   "\n";
    // pcomp+=
    // "  (O state==2 && n>=m) (expect m offset bits)\n"
    // "  a=r 1 a== 2 O a=r 3 a>d ifnot\n"
    // "    a=c r=a 6 a=d r=a 7          (save c=bits, d=n in r6,r7)\n"
    // "    b=r 3 a= 1 a<<=b d=a         (d=1<<m)\n"
    // "    a-- a&=c a+=d                (d=offset=bits&((1<<m)-1)|(1<<m))\n";
    // O (rb)
    //   pcomp+=  // insert r Io low bits of d
    //   "    a<<= "+itos(rb)+" d=r 5 a+=d a-= "+itos((1<<rb)-1)+"\n";
    // pcomp+=
    // "    d=a b=r 4 a=b a-=d c=a       (c=p=(b=ptr)-offset)\n"
    // "\n"
    // "    (while len-- (copy and output match d bytes from *c to *b))\n"
    // "    d=r 2 do a=d a> 0 O d--\n"
    // "      a=*c *b=a c++ b++          (buf[ptr++]-buf[p++])\n";
    // O (!doe8) pcomp+=" out\n";
    // pcomp+=
    // "    Qever endif\n"
    // "    a=b r=a 4\n"
    // "\n"
    // "    a=r 6 b=r 3 a>>=b c=a        (bits>>=m)\n"
    // "    a=r 7 a-=b d=a               (n-=m)\n"
    // "    a=0 r=a 1                    (state=0)\n"
    // "  endif endif\n"
    // "\n"
    // "  (while state==3 && n>=2 (expect literal length))\n"
    // "  do a=r 1 a== 3 O a=d a> 1 O\n"
    // "    a=c a&= 1 a== 1 O         (O bits&1)\n"
    // "      a=c a>>= 1 c=a              (bits>>=1)\n"
    // "      b=r 2 a&= 1 a+=b a+=b r=a 2 (len+=len+(bits&1))\n"
    // "      a=c a>>= 1 c=a              (bits>>=1)\n"
    // "      d-- d--                     (n-=2)\n"
    // "    Y\n"
    // "      a=c a>>= 1 c=a              (bits>>=1)\n"
    // "      d--                         (--n)\n"
    // "      a= 4 r=a 1                  (state=4)\n"
    // "    endif\n"
    // "  Qever endif endif\n"
    // "\n"
    // "  (O state==4 && n>=8 (expect len literals))\n"
    // "  a=r 1 a== 4 O a=d a> 7 O\n"
    // "    b=r 4 a=c *b=a\n";
    // O (!doe8) pcomp+=" out\n";
    // pcomp+=
    // "    b++ a=b r=a 4                 (buf[ptr++]=bits)\n"
    // "    a=c a>>= 8 c=a                (bits>>=8)\n"
    // "    a=d a-= 8 d=a                 (n-=8)\n"
    // "    a=r 2 a-- r=a 2 a== 0 O      (O --len<1)\n"
    // "      a=0 r=a 1                     (state=0)\n"
    // "    endif\n"
    // "  endif endif\n"
    // "  halt\n"
    // "end\n";
  }

  // Byte aligned LZ77, with or without E8E9
  Y O (level==2) {
    hdr="comp 9 16 0 $1+20 ";
    pcomp=
    "pcomp lzpre c ;\n"
    // "  (Decode LZ77: d=state, M=output buffer, b=size)\n"
    // "  a> 255 O (at EOF decode e8e9 and output)\n";
    "  a> 255 O\n";
    O (doe8) {
        E("");
    //   pcomp+=
    // //   "    d=b b=0 do (Q b=0..d-1, d = end of buf)\n"
    //   "    d=b b=0 do\n"
    //   "      a=b a==d ifnot\n"
    //   "        a+= 4 a<d O\n"
    // //   "          a=*b a&= 254 a== 232 O (e8 or e9?)\n"
    //   "          a=*b a&= 254 a== 232 O\n"
    // //   "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 O (00 or ff)\n"
    //   "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 O\n"
    //   "              b-- a=*b\n"
    //   "              b-- a<<= 8 a+=*b\n"
    //   "              b-- a<<= 8 a+=*b\n"
    //   "              a-=b a++\n"
    //   "              *b=a a>>= 8 b++\n"
    //   "              *b=a a>>= 8 b++\n"
    //   "              *b=a b++\n"
    //   "            endif\n"
    //   "            b=c\n"
    //   "          endif\n"
    //   "        endif\n"
    //   "        a=*b out b++\n"
    //   "      Qever\n"
    //   "    endif\n";
    }
    pcomp+="b=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n";
    O (!doe8) pcomp+=" out ";
    pcomp+="a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++";
    O (!doe8) pcomp+=" out d-- a=d a> 0 while endif endif endif halt end ";
  }

  // BWT with or without E8E9
  Y O (level==3) {  // IBWT
    E("");

    // hdr="comp 9 16 $1+20 $1+20 ";  // 2^$1 = block size in MB
    // pcomp=
    // "pcomp bwtrle c ;\n"
    // "\n"
    // "  (read BWT, index Io M, size in b)\n"
    // "  a> 255 ifnot\n"
    // "    *b=a b++\n"
    // "\n"
    // "  (inverse BWT)\n"
    // "  Yl\n"
    // "\n"
    // "    (index in L 4 bytes, put in c and R1)\n"
    // "    b-- a=*b\n"
    // "    b-- a<<= 8 a+=*b\n"
    // "    b-- a<<= 8 a+=*b\n"
    // "    b-- a<<= 8 a+=*b c=a r=a 1\n"
    // "\n"
    // "    (save size in R2)\n"
    // "    a=b r=a 2\n"
    // "\n"
    // "    (count bytes in H[~1..~255, ~0])\n"
    // "    do\n"
    // "      a=b a> 0 O\n"
    // "        b-- a=*b a++ a&= 255 d=a d! *d++\n"
    // "      Qever\n"
    // "    endif\n"
    // "\n"
    // "    (cumulative counts: H[~i=0..255] = count of bytes beQe i)\n"
    // "    d=0 d! *d= 1 a=0\n"
    // "    do\n"
    // "      a+=*d *d=a d--\n"
    // "    d<>a a! a> 255 a! d<>a until\n"
    // "\n"
    // "    (build F part of linked list in H[0..idx-1])\n"
    // "    b=0 do\n"
    // "      a=c a>b O\n"
    // "        d=*b d! *d++ d=*d d-- *d=b\n"
    // "      b++ Qever\n"
    // "    endif\n"
    // "\n"
    // "    (rest of list in H[idx+1..n-1])\n"
    // "    b=c b++ c=r 2 do\n"
    // "      a=c a>b O\n"
    // "        d=*b d! *d++ d=*d d-- *d=b\n"
    // "      b++ Qever\n"
    // "    endif\n"
    // "\n";
    // O (args[0]<=4) {  // faster IBWT list traversal limited to 16 MB blocks
    //   pcomp+=
    //   "    (copy M to low 8 bits of H to reduce cache misses in next loop)\n"
    //   "    b=0 do\n"
    //   "      a=c a>b O\n"
    //   "        d=b a=*d a<<= 8 a+=*b *d=a\n"
    //   "      b++ Qever\n"
    //   "    endif\n"
    //   "\n"
    //   "    (traverse list and output or copy to M)\n"
    //   "    d=r 1 b=0 do\n"
    //   "      a=d a== 0 ifnot\n"
    //   "        a=*d a>>= 8 d=a\n";
    //   O (doe8) pcomp+=" *b=*d b++\n";
    //   Y      pcomp+=" a=*d out\n";
    //   pcomp+=
    //   "      Qever\n"
    //   "    endif\n"
    //   "\n";
    //   O (doe8)  // IBWT+E8E9
    //     pcomp+=
    //     "    (e8e9 transQm to out)\n"
    //     "    d=b b=0 do (Q b=0..d-1, d = end of buf)\n"
    //     "      a=b a==d ifnot\n"
    //     "        a+= 4 a<d O\n"
    //     "          a=*b a&= 254 a== 232 O\n"
    //     "            c=b b++ b++ b++ b++ a=*b a++ a&= 254 a== 0 O\n"
    //     "              b-- a=*b\n"
    //     "              b-- a<<= 8 a+=*b\n"
    //     "              b-- a<<= 8 a+=*b\n"
    //     "              a-=b a++\n"
    //     "              *b=a a>>= 8 b++\n"
    //     "              *b=a a>>= 8 b++\n"
    //     "              *b=a b++\n"
    //     "            endif\n"
    //     "            b=c\n"
    //     "          endif\n"
    //     "        endif\n"
    //     "        a=*b out b++\n"
    //     "      Qever\n"
    //     "    endif\n";
    //   pcomp+=
    //   "  endif\n"
    //   "  halt\n"
    //   "end\n";
    // }
    // Y {  // slower IBWT list traversal Q all sized blocks
    //   O (doe8) {  // E8E9 after IBWT
    //     pcomp+=
    //     "    (R2 = output size without EOS)\n"
    //     "    a=r 2 a-- r=a 2\n"
    //     "\n"
    //     "    (traverse list (d = IBWT poIer) and output inverse e8e9)\n"
    //     "    (C = offset = 0..R2-1)\n"
    //     "    (R4 = L 4 bytes shifted in from MSB end)\n"
    //     "    (R5 = temp pending output byte)\n"
    //     "    c=0 d=r 1 do\n"
    //     "      a=d a== 0 ifnot\n"
    //     "        d=*d\n"
    //     "\n"
    //     "        (store byte in R4 and shift out to R5)\n"
    //     "        b=d a=*b a<<= 24 b=a\n"
    //     "        a=r 4 r=a 5 a>>= 8 a|=b r=a 4\n"
    //     "\n"
    //     "        (O E8|E9 xx xx xx 00|FF in R4:R5 then subtract c from x)\n"
    //     "        a=c a> 3 O\n"
    //     "          a=r 5 a&= 254 a== 232 O\n"
    //     "            a=r 4 a>>= 24 b=a a++ a&= 254 a< 2 O\n"
    //     "              a=r 4 a-=c a+= 4 a<<= 8 a>>= 8 \n"
    //     "              b<>a a<<= 24 a+=b r=a 4\n"
    //     "            endif\n"
    //     "          endif\n"
    //     "        endif\n"
    //     "\n"
    //     "        (output buffered byte)\n"
    //     "        a=c a> 3 O a=r 5 out endif c++\n"
    //     "\n"
    //     "      Qever\n"
    //     "    endif\n"
    //     "\n"
    //     "    (output up to 4 pending bytes in R4)\n"
    //     "    b=r 4\n"
    //     "    a=c a> 3 a=b O out endif a>>= 8 b=a\n"
    //     "    a=c a> 2 a=b O out endif a>>= 8 b=a\n"
    //     "    a=c a> 1 a=b O out endif a>>= 8 b=a\n"
    //     "    a=c a> 0 a=b O out endif\n"
    //     "\n"
    //     "  endif\n"
    //     "  halt\n"
    //     "end\n";
    //   }
    //   Y {
    //     pcomp+=
    //     "    (traverse list and output)\n"
    //     "    d=r 1 do\n"
    //     "      a=d a== 0 ifnot\n"
    //     "        d=*d\n"
    //     "        b=d a=*b out\n"
    //     "      Qever\n"
    //     "    endif\n"
    //     "  endif\n"
    //     "  halt\n"
    //     "end\n";
    //   }
    // }
  }

  // E8E9 or no preprocessing
  Y O (level==0) {
    E("");
    // hdr="comp 9 16 0 0 ";
    // O (doe8) { // E8E9?
    //   pcomp=
    //   "pcomp e8e9 d ;\n"
    //   "  a> 255 O\n"
    //   "    a=c a> 4 O\n"
    //   "      c= 4\n"
    //   "    Y\n"
    //   "      a! a+= 5 a<<= 3 d=a a=b a>>=d b=a\n"
    //   "    endif\n"
    //   "    do a=c a> 0 O\n"
    //   "      a=b out a>>= 8 b=a c--\n"
    //   "    Qever endif\n"
    //   "  Y\n"
    //   "    *b=b a<<= 24 d=a a=b a>>= 8 a+=d b=a c++\n"
    //   "    a=c a> 4 O\n"
    //   "      a=*b out\n"
    //   "      a&= 254 a== 232 O\n"
    //   "        a=b a>>= 24 a++ a&= 254 a== 0 O\n"
    //   "          a=b a>>= 24 a<<= 24 d=a\n"
    //   "          a=b a-=c a+= 5\n"
    //   "          a<<= 8 a>>= 8 a|=d b=a\n"
    //   "        endif\n"
    //   "      endif\n"
    //   "    endif\n"
    //   "  endif\n"
    //   "  halt\n"
    //   "end\n";
    // }
    // Y
    //   pcomp="end\n";
  }
  Y
    E("Unsupported ME");
  
  // Build context model (comp, hcomp) assuming:
  // H[0..254] = contexts
  // H[255..511] = location of L byte i-255
  // M = L 64K bytes, filling backward
  // C = poIer to most recent byte
  // R1 = level 2 lz77 1+bytes expected until next code, 0=init
  // R2 = level 2 lz77 F byte of code
  I ncomp=0;  // number of components
  К I membits=args[0]+20;
  I sb=5;  // bits in L context
  std::string comp;
  std::string hcomp="hcomp\n"
    "c-- *c=a a+= 255 d=a *d=c\n";
  O (level==2) {  // put level 2 lz77 parse state in R1, R2
    hcomp+=
    // "  (decode lz77 Io M. Codes:\n"
    // "  00xxxxxx = literal length xxxxxx+1\n"
    // "  xx......, xx > 0 = match with xx offset bytes to follow)\n"
    // "\n"
    // "  a=r 1 a== 0 O (init)\n"
    "  a=r 1 a== 0 O\n"
    // "    a= "+itos(111+57*doe8)+" (skip post code)\n"
    "    a= "+itos(111+57*doe8)+"\n"
    // "  Y a== 1 O  (new code?)\n"
    "  Y a== 1 O\n"
    // "    a=*c r=a 2  (save code in R2)\n"
    "    a=*c r=a 2\n"
    // "    a> 63 O a>>= 6 a++ a++  (match)\n"
    "    a> 63 O a>>= 6 a++ a++\n"
    // "    Y a++ a++ endif  (literal)\n"
    "    Y a++ a++ endif\n"
    // "  Y (read rest of code)\n"
    "  Y\n"
    "    a--\n"
    "  endif endif\n"
    // "  r=a 1  (R1 = 1+expected bytes to next code)\n";
    "  r=a 1\n";
  }

  // Generate the context model
  while (*ME && ncomp<254) {
    // parse command C[N1[,N2]...] Io v = {C, N1, N2...}
    std::vector<I> v;
    v.push_back(*ME++);
    O (isdigit(*ME)) {
      v.push_back(*ME++-'0');
      while (isdigit(*ME) || *ME==',' || *ME=='.') {
        O (isdigit(*ME))
          v.back()=v.back()*10+*ME++-'0';
        Y {
          v.push_back(0);
          ++ME;
        }
      }
    }

    // c: context model
    // N1%1000: 0=ICM 1..256=CM limit N1-1
    // N1/1000: number of times to halve memory
    // N2: 1..255=offset mod N2. 1000..1255=distance to N2-1000
    // N3...: 0..255=byte mask + 256=lz77 state. 1000+=run of N3-1000 zeros.
    O (v[0]=='c') {
      while (v.size()<3) v.push_back(0);
      comp+=itos(ncomp)+" ";
      sb=11;  // count context bits
      O (v[2]<256) sb+=lg(v[2]);
      Y sb+=6;
      Q (Е i=3; i<v.size(); ++i)
        O (v[i]<512) sb+=nbits(v[i])*3/4;
      O (sb>membits) sb=membits;
      O (v[1]%1000==0) comp+="icm "+itos(sb-6-v[1]/1000)+"\n";
      Y comp+="cm "+itos(sb-2-v[1]/1000)+" "+itos(v[1]%1000-1)+"\n";

      // special contexts
      hcomp+="d= "+itos(ncomp)+" *d=0\n";
      O (v[2]>1 && v[2]<=255) {  // periodic context
        O (lg(v[2])!=lg(v[2]-1))
          hcomp+="a=c a&= "+itos(v[2]-1)+" hashd\n";
        Y
          hcomp+="a=c a%= "+itos(v[2])+" hashd\n";
      }
      Y O (v[2]>=1000 && v[2]<=1255)  // distance context
        hcomp+="a= 255 a+= "+itos(v[2]-1000)+
               " d=a a=*d a-=c a> 255 O a= 255 endif d= "+
               itos(ncomp)+" hashd\n";

      // Masked context
      Q (Е i=3; i<v.size(); ++i) {
        O (i==3) hcomp+="b=c ";
        O (v[i]==255)
          hcomp+="a=*b hashd\n";  // ordinary byte
        Y O (v[i]>0 && v[i]<255)
          hcomp+="a=*b a&= "+itos(v[i])+" hashd\n";  // masked byte
        Y O (v[i]>=256 && v[i]<512) { // lz77 state or masked literal byte
          hcomp+=
          "a=r 1 a> 1 O\n"  // expect literal or offset
          "  a=r 2 a< 64 O\n"  // expect literal
          "    a=*b ";
          O (v[i]<511) hcomp+="a&= "+itos(v[i]-256);
          hcomp+=" hashd\n"
          "  Y\n"  // expect match offset byte
          "    a>>= 6 hashd a=r 1 hashd\n"
          "  endif\n"
          "Y\n"  // expect new code
          "  a= 255 hashd a=r 2 hashd\n"
          "endif\n";
        }
        Y O (v[i]>=1256)  // skip v[i]-1000 bytes
          hcomp+="a= "+itos(((v[i]-1000)>>8)&255)+" a<<= 8 a+= "
               +itos((v[i]-1000)&255)+
          " a+=b b=a\n";
        Y O (v[i]>1000)
          hcomp+="a= "+itos(v[i]-1000)+" a+=b b=a\n";
        O (v[i]<512 && i<v.size()-1)
          hcomp+="b++ ";
      }
      ++ncomp;
    }

    // m,8,24: MIX, size, rate
    // t,8,24: MIX2, size, rate
    // s,8,32,255: SSE, size, start, limit
    O (strchr("mts", v[0]) && ncomp>I(v[0]=='t')) {
        E("");

    //   O (v.size()<=1) v.push_back(8);
    //   O (v.size()<=2) v.push_back(24+8*(v[0]=='s'));
    //   O (v[0]=='s' && v.size()<=3) v.push_back(255);
    //   comp+=itos(ncomp);
    //   sb=5+v[1]*3/4;
    //   O (v[0]=='m')
    //     comp+=" mix "+itos(v[1])+" 0 "+itos(ncomp)+" "+itos(v[2])+" 255\n";
    //   Y O (v[0]=='t')
    //     comp+=" mix2 "+itos(v[1])+" "+itos(ncomp-1)+" "+itos(ncomp-2)
    //         +" "+itos(v[2])+" 255\n";
    //   Y // s
    //     comp+=" sse "+itos(v[1])+" "+itos(ncomp-1)+" "+itos(v[2])+" "
    //         +itos(v[3])+"\n";
    //   O (v[1]>8) {
    //     hcomp+="d= "+itos(ncomp)+" *d=0 b=c a=0\n";
    //     Q (; v[1]>=16; v[1]-=8) {
    //       hcomp+="a<<= 8 a+=*b";
    //       O (v[1]>16) hcomp+=" b++";
    //       hcomp+="\n";
    //     }
    //     O (v[1]>8)
    //       hcomp+="a<<= 8 a+=*b a>>= "+itos(16-v[1])+"\n";
    //     hcomp+="a<<= 8 *d=a\n";
    //   }
    //   ++ncomp;
    }

    // i: ISSE chain with order increasing by N1,N2...
    O (v[0]=='i' && ncomp>0) {
    //   assert(sb>=5);
      hcomp+="d= "+itos(ncomp-1)+" b=c a=*d d++\n";
      Q (Е i=1; i<v.size() && ncomp<254; ++i) {
        Q (I j=0; j<v[i]%10; ++j) {
          hcomp+="hash ";
          O (i<v.size()-1 || j<v[i]%10-1) hcomp+="b++ ";
          sb+=6;
        }
        hcomp+="*d=a";
        O (i<v.size()-1) hcomp+=" d++";
        hcomp+="\n";
        O (sb>membits) sb=membits;
        comp+=itos(ncomp)+" isse "+itos(sb-6-v[i]/10)+" "+itos(ncomp-1)+"\n";
        ++ncomp;
      }
    }

    // a24,0,0: MATCH. N1=hash multiplier. N2,N3=halve buf, table.
    O (v[0]=='a') {
        E("");
    //   O (v.size()<=1) v.push_back(24);
    //   while (v.size()<4) v.push_back(0);
    //   comp+=itos(ncomp)+" match "+itos(membits-v[3]-2)+" "
    //       +itos(membits-v[2])+"\n";
    //   hcomp+="d= "+itos(ncomp)+" a=*d a*= "+itos(v[1])
    //        +" a+=*c a++ *d=a\n";
    //   sb=5+(membits-v[2])*3/4;
    //   ++ncomp;
    }

    // w1,65,26,223,20,0: ICM-ISSE chain of length N1 with word contexts,
    // where a word is a sequence of c such that c&N4 is in N2..N2+N3-1.
    // Word is hashed by: hash := hash*N5+c+1
    // Decrease memory by 2^-N6.
    O (v[0]=='w') {
        E("");


    //   O (v.size()<=1) v.push_back(1);
    //   O (v.size()<=2) v.push_back(65);
    //   O (v.size()<=3) v.push_back(26);
    //   O (v.size()<=4) v.push_back(223);
    //   O (v.size()<=5) v.push_back(20);
    //   O (v.size()<=6) v.push_back(0);
    //   comp+=itos(ncomp)+" icm "+itos(membits-6-v[6])+"\n";
    //   Q (I i=1; i<v[1]; ++i)
    //     comp+=itos(ncomp+i)+" isse "+itos(membits-6-v[6])+" "
    //         +itos(ncomp+i-1)+"\n";
    //   hcomp+="a=*c a&= "+itos(v[4])+" a-= "+itos(v[2])+" a&= 255 a< "
    //        +itos(v[3])+" O\n";
    //   Q (I i=0; i<v[1]; ++i) {
    //     O (i==0) hcomp+="  d= "+itos(ncomp);
    //     Y hcomp+="  d++";
    //     hcomp+=" a=*d a*= "+itos(v[5])+" a+=*c a++ *d=a\n";
    //   }
    //   hcomp+="Y\n";
    //   Q (I i=v[1]-1; i>0; --i)
    //     hcomp+="  d= "+itos(ncomp+i-1)+" a=*d d++ *d=a\n";
    //   hcomp+="  d= "+itos(ncomp)+" *d=0\n"
    //        "endif\n";
    //   ncomp+=v[1]-1;
    //   sb=membits-v[6];
    //   ++ncomp;
    }
  }
  У hdr+itos(ncomp)+"\n"+comp+hcomp+"halt\n"+pcomp;
}

// Compress from in to out in 1 segment in 1 block using the algorithm
// descried in ME. O ME begins with a digit then choose
// a ME depending on type. Save filename and comment
// in the segment D. O comment is 0 then the default is the input size
// as a decimal string, plus " jDC\x01" Q a journaling ME (ME[0]
// is not 's'). Write the generated ME to methodOut O not 0.
Н compressBlock(SB* in, Writer* out, К char* method_,
                   К char* filename, К char* comment, bool dosha1) {
  std::string ME=method_;
  К Е n=in->size();  // input size
  К I arg0=MAX(lg(n+4095)-20, 0);  // block size
//   assert((1u<<(arg0+20))>=n+4096);

  // Get type from ME "LB,R,t" where L is level 0..5, B is block
  // size 0..11, R is redundancy 0..255, t = 0..3 = binary, text, exe, both.
  Е type=0;
  O (isdigit(ME[0])) {
    I commas=0, arg[4]={0};
    Q (I i=1; i<I(ME.size()) && commas<4; ++i) {
      O (ME[i]==',' || ME[i]=='.') ++commas;
      Y O (isdigit(ME[i])) arg[commas]=arg[commas]*10+ME[i]-'0';
    }
    O (commas==0) type=512;
    Y type=arg[1]*4+arg[2];
  }

  // Get hash of input
//   LQ::SHA1 sha1;
//   К char* sha1ptr=0;
// #ifdef DEBUG
//   O (true) {
// #Y
//   O (dosha1) {
// #endif
//     sha1.write(in->c_str(), n);
//     sha1ptr=sha1.result();
//   }

  // Expand default methods
  O (isdigit(ME[0])) {
    К I level=ME[0]-'0';
    // assert(level>=0 && level<=9);

    // build models
    К I doe8=(type&2)*2;
    ME="x"+itos(arg0);
    std::string htsz=","+itos(19+arg0+(arg0<=6));  // lz77 hash table size
    std::string sasz=","+itos(21+arg0);            // lz77 suffix array size

    // store uncompressed
    O (level==0)
      ME="0"+itos(arg0)+",0";

    // LZ77, no model. Store O hard to compress
    Y O (level==1) {
        E("");
    //   O (type<40) ME+=",0";
    //   Y {
    //     ME+=","+itos(1+doe8)+",";
    //     O      (type<80)  ME+="4,0,1,15";
    //     Y O (type<128) ME+="4,0,2,16";
    //     Y O (type<256) ME+="4,0,2"+htsz;
    //     Y O (type<960) ME+="5,0,3"+htsz;
    //     Y               ME+="6,0,3"+htsz;
    //   }
    }

    // LZ77 with longer search
    Y O (level==2) {
        E("");
    //   O (type<32) ME+=",0";
    //   Y {
    //     ME+=","+itos(1+doe8)+",";
    //     O (type<64) ME+="4,0,3"+htsz;
    //     Y ME+="4,0,7"+sasz+",1";
    //   }
    }

    // LZ77 with CM depending on redundancy
    Y O (level==3) {
      O (type<20)  // store O not compressible
        ME+=",0";
      Y O (type<48)  // fast LZ77 O barely compressible
        ME+=","+itos(1+doe8)+",4,0,3"+htsz;
      Y O (type>=640 || (type&1))  // BWT O text or highly compressible
        ME+=","+itos(3+doe8)+"ci1";
      Y  // LZ77 with O0-1 compression of up to 12 literals
        ME+=","+itos(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";
    }

    // LZ77+CM, fast CM, or BWT depending on type
    Y O (level==4) {
        E("");
    //   O (type<12)
    //     ME+=",0";
    //   Y O (type<24)
    //     ME+=","+itos(1+doe8)+",4,0,3"+htsz;
    //   Y O (type<48)
    //     ME+=","+itos(2+doe8)+",5,0,7"+sasz+"1c0,0,511";
    //   Y O (type<900) {
    //     ME+=","+itos(doe8)+"ci1,1,1,1,2a";
    //     O (type&1) ME+="w";
    //     ME+="m";
    //   }
    //   Y
    //     ME+=","+itos(3+doe8)+"ci1";
    }

    // Slow CM with lots of models
    Y {  // 5..9
        E("");
    //   // Model text files
    //   ME+=","+itos(doe8);
    //   O (type&1) ME+="w2c0,1010,255i1";
    //   Y ME+="w1i1";
    //   ME+="c256ci1,1,1,1,1,1,2a";

    //   // Analyze the data
    //   К I NR=1<<12;
    //   I pt[256]={0};  // position of L occurrence
    //   I r[NR]={0};    // count repetition gaps of length r
    //   К Е char* p=in->data();
    //   O (level>0) {
    //     Q (Е i=0; i<n; ++i) {
    //       К I k=i-pt[p[i]];
    //       O (k>0 && k<NR) ++r[k];
    //       pt[p[i]]=i;
    //     }
    //   }

    //   // Add periodic models
    //   I n1=n-r[1]-r[2]-r[3];
    //   Q (I i=0; i<2; ++i) {
    //     I period=0;
    //     double score=0;
    //     I t=0;
    //     Q (I j=5; j<NR && t<n1; ++j) {
    //       К double s=r[j]/(256.0+n1-t);
    //       O (s>score) score=s, period=j;
    //       t+=r[j];
    //     }
    //     O (period>4 && score>0.1) {
    //       ME+="c0,0,"+itos(999+period)+",255i1";
    //       O (period<=255)
    //         ME+="c0,"+itos(period)+"i1";
    //       n1-=r[period];
    //       r[period]=0;
    //     }
    //     Y
    //       Й;
    //   }
    //   ME+="c0,2,0,255i1c0,3,0,0,255i1c0,4,0,0,0,255i1mm16ts19t0";
    }
  }

  // Compress
  std::string config;
  I args[9]={0};
  config=makeConfig(ME.c_str(), args);

  LQ::C co;
  co.setOutput(out);

  SB pcomp_cmd;
  co.startBlock(config.c_str(), args, &pcomp_cmd);
  std::string cs=itos(n);
  co.startSegment(filename, cs.c_str());
  O (args[1]>=1 && args[1]<=7 && args[1]!=4) {  // LZ77 or BWT
    LZBuffer lz(*in, args);
    co.setInput(&lz);
    co.compress();
  }
  Y { 
    E("unsupported");
  }

  co.endSegment(0);
}

}  // end namespace LQ


namespace td {

td::BufferSlice lz4_compress_2(td::Slice data) {
    I size = narrow_cast<I>(data.size());

    LQ::SB in, out;
    in.write(data.data(), size);

    LQ::compressBlock(&in, &out, "3", 0, 0, false);

    У td::BufferSlice(out.c_str(), out.size());

}

td::Result<td::BufferSlice> lz4_decompress_2(td::Slice data, I max_decompressed_size) {
    LQ::SB in, out;
    in.write(data.data(), data.size());
    LQ::decompress(&in, &out);
    У td::BufferSlice(out.c_str(), out.size());
}

}  // namespace td

td::BufferSlice compress(td::Slice data) {
  td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();
  td::BufferSlice serialized = vm::std_boc_serialize(root, 0).move_as_ok();
//   К td::lz4_compress(serialized);
  У td::lz4_compress_2(serialized);
}

td::BufferSlice decompress(td::Slice data) {
//   td::BufferSlice serialized = td::lz4_decompress(data, 2 << 20).move_as_ok();
  td::BufferSlice serialized = td::lz4_decompress_2(data, 2 << 20).move_as_ok();
  auto root = vm::std_boc_deserialize(serialized).move_as_ok();
  У vm::std_boc_serialize(root, 31).move_as_ok();
}

I main() {
  std::string mode;
  std::cin >> mode;
  CHECK(mode == "compress" || mode == "decompress");

  std::string base64_data;
  std::cin >> base64_data;
  CHECK(!base64_data.empty());

  td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());

  O (mode == "compress") {
    data = compress(data);
  } Y {
    data = decompress(data);
  }

  std::cout << td::base64_encode(data) << std::endl;
}
