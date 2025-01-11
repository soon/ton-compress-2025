// minify-remove:start
#include<iostream>
#include"td/utils/buffer.h"
#include"td/utils/misc.h"
#include"vm/boc.h"
#include<stdio.h>
#include<string>
#include<vector>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<algorithm>
// minify-remove:end

#define Y else
#define O if
#define Q for
#define F(x) for(int i=0;i<(x);++i)

using I=int;

// minify-remove
using namespace std;


namespace LQ {

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

void E(const char* msg=0) {
  throw runtime_error(msg);
}

I lg(unsigned x) {
  unsigned r=0;
  O (x>=65536) r=16, x>>=16;
  O (x>=256) r+=8, x>>=8;
  O (x>=16) r+=4, x>>=4;
  return
    "\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;
}

I tolower(I c){return(c>='A'&&c<='Z')?c+'a'-'A':c;}

string itos(int64_t x, I n=1) {
  string r;
  Q (; x || n>0; x/=10, --n) r=string(1, '0'+x%10)+r;
  return r;
}

struct Reader {
  virtual I get() = 0;  // should return 0..255, or -1 at EOF
  virtual I read(char* buf, I n) {
    I i=0, c;
    while (i<n && (c=get())>=0)
      buf[i++]=c;
    return i;
  }
  virtual ~Reader() {}
};

struct Writer {
  virtual void put(I c) = 0;
  virtual void write(const char* buf, I n) {
    F(n) put(U8(buf[i]));
  }
  virtual ~Writer() {}
};

I toU16(const char* p) {
  return (p[0]&255)+256*(p[1]&255);
}

const I compsize[256]={0,2,3,2,3,4,6,6,3,5};

template <class T>
struct Array {
  T *data;  
  size_t n;  
  I offset; 
  void operator=(const Array&);
  Array(const Array&); 
  Array(size_t sz=0, I ex=0): data(0), n(0), offset(0) { resize(sz, ex); }
  void resize(size_t sz, I ex=0) {
    while (ex>0) {
      O (sz>sz*2) E();
      sz*=2, --ex;
    }
    O (n>0) {
      ::free((char*)data-offset);
    }
    n=0;
    offset=0;
    O (sz==0) return;
    n=sz;
    const size_t nb=128+n*sizeof(T); 
    O (nb<=128 || (nb-128)/sizeof(T)!=n) n=0, E();
    data=(T*)::calloc(nb, 1);
    O (!data) n=0, E(); // E("Out of memory");
    offset=64-(((char*)data-(char*)0)&63);
    //   assert(offset>0 && offset<=64);
    data=(T*)((char*)data+offset);
  } // change size, erase content to zeros
  ~Array() {resize(0);}  // free memory
  size_t size() {return n;}  // get size
  I isize() {return I(n);}  // get size as an I
  T& operator[](size_t i) {
    // assert(n>0 && i<n); 
    return data[i];}
  T& operator()(size_t i) {
    // assert(n>0 && (n&(n-1))==0); 
    return data[i&(n-1)];}
};

typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE} CompType;
struct Decoder;  // Qward
struct ZP {
  ZP() {
    output=0;
    rcode=0;
    rcode_size=0;
    clear();
    outbuf.resize(1<<14);
    bufptr=0;
  }
  void clear() {
    cend=hbegin=hend=0;  // COMP and HCOMP locations
    a=b=c=d=f=pc=0;      // machine state
    D.resize(0);
    h.resize(0);
    m.resize(0);
    r.resize(0);
  }
  void inith() { init(D[2], D[3]); }
  void initp() { init(D[4], D[5]); }
  void run(U32 input);    // Execute with input
  I read(Reader* in2) {
    I hsize=in2->get();
    hsize+=in2->get()*256;
    D.resize(hsize+300);
    cend=hbegin=hend=0;
    D[cend++]=hsize&255;
    D[cend++]=hsize>>8;
    while (cend<7) D[cend++]=in2->get(); // hh hm ph pm n

    I n=D[cend-1];
    F(n){
      I type=in2->get();  // component type
      D[cend++]=type;  // component type
      I size=compsize[type];
      Q (I j=1; j<size; ++j)
        D[cend++]=in2->get();
    }
    O ((D[cend++]=in2->get())!=0) E();

    hbegin=hend=cend+128;
    while (hend<hsize+129) {
      I op=in2->get();
      D[hend++]=op;
    }
    O ((D[hend++]=in2->get())!=0) E();
    return cend+hend-hbegin;
  }
  bool write(Writer* out2, bool pp) {
    O (D.size()<=6) return false;
    O (!pp) {  // O not a postprocessor then write COMP
      F(cend)out2->put(D[i]);
    }
    Y {  // write PCOMP size only
      out2->put((hend-hbegin)&255);
      out2->put((hend-hbegin)>>8);
    }
    Q (I i=hbegin; i<hend; ++i)
      out2->put(D[i]);
    return true;
  }
  I step(U32 input, I mode);  // Trace execution (defined externally)

  Writer* output;         // Destination Q OUT instruction, or 0 to suppress
  U32 H(I i) {return h(i);}  // get element of h

  void flush() {
    O (output) output->write(&outbuf[0], bufptr);
    bufptr=0;
  }
  void outc(I ch) {     // output byte ch (0..255) or -1 at EOS
    O (ch<0 || (outbuf[bufptr]=ch, ++bufptr==outbuf.isize())) flush();
  }

  Array<U8> D;   // hsize[2] hh hm ph pm n COMP (guard) HCOMP (guard)
  I cend, hbegin, hend;   // HCOMP/PCOMP in D[hbegin...hend-1]

  Array<U8> m;        // memory array M Q HCOMP
  Array<U32> h, r;       // hash array H Q HCOMP
  Array<char> outbuf; // output buffer
  I bufptr;         // number of bytes in outbuf
  U32 a, b, c, d;     // machine registers
  I f, pc, rcode_size;              // condition flag
  U8* rcode;          // JIT code Q run()

  void init(I hbits, I mbits) {
    h.resize(1, hbits);
    m.resize(1, mbits);
    r.resize(256);
    a=b=c=d=pc=f=0;
  }
  
  I execute() {
    switch(D[pc++]) {
      // case 0:err(); break; // E
      case 1:++a; break; // A++
      case 2:--a; break; // A--
      case 3:a = ~a; break; // A!
      case 4:a = 0; break; // A=0
      case 7:a = r[D[pc++]]; break; // A=R N
      case 8:swap(b); break; // B<>A
      case 9:++b; break; // B++
      case 10:--b; break; // B--
      case 11:b = ~b; break; // B!
      case 12:b = 0; break; // B=0
      case 15:b = r[D[pc++]]; break; // B=R N
      case 16:swap(c); break; // C<>A
      case 17:++c; break; // C++
      case 18:--c; break; // C--
      case 19:c = ~c; break; // C!
      case 20:c = 0; break; // C=0
      case 23:c = r[D[pc++]]; break; // C=R N
      case 24:swap(d); break; // D<>A
      case 25:++d; break; // D++
      case 26:--d; break; // D--
      case 27:d = ~d; break; // D!
      case 28:d = 0; break; // D=0
      case 31:d = r[D[pc++]]; break; // D=R N
      case 32:swap(m(b)); break; // *B<>A
      case 33:++m(b); break; // *B++
      case 34:--m(b); break; // *B--
      case 35:m(b) = ~m(b); break; // *B!
      case 36:m(b) = 0; break; // *B=0
      case 39:O (f) pc+=((D[pc]+128)&255)-127; Y ++pc; break; // JT N
      case 40:swap(m(c)); break; // *C<>A
      case 41:++m(c); break; // *C++
      case 42:--m(c); break; // *C--
      case 43:m(c) = ~m(c); break; // *C!
      case 44:m(c) = 0; break; // *C=0
      case 47:O (!f) pc+=((D[pc]+128)&255)-127; Y ++pc; break; // JF N
      case 48:swap(h(d)); break; // *D<>A
      case 49:++h(d); break; // *D++
      case 50:--h(d); break; // *D--
      case 51:h(d) = ~h(d); break; // *D!
      case 52:h(d) = 0; break; // *D=0
      case 55:r[D[pc++]] = a; break; // R=A N
      case 56:return 0  ; // HALT
      case 57:outc(a&255); break; // OUT
      case 59:a = (a+m(b)+512)*773; break; // HASH
      case 60:h(d) = (h(d)+a+512)*773; break; // HASHD
      case 63:pc+=((D[pc]+128)&255)-127; break; // JMP N
      case 64:break; // A=A
      case 65:a = b; break; // A=B
      case 66:a = c; break; // A=C
      case 67:a = d; break; // A=D
      case 68:a = m(b); break; // A=*B
      case 69:a = m(c); break; // A=*C
      case 70:a = h(d); break; // A=*D
      case 71:a = D[pc++]; break; // A= N
      case 72:b = a; break; // B=A
      case 73:break; // B=B
      case 74:b=c; break; // B=C
      case 75:b=d; break; // B=D
      case 76:b=m(b); break; // B=*B
      case 77:b=m(c); break; // B=*C
      case 78:b=h(d); break; // B=*D
      case 79:b=D[pc++]; break; // B= N
      case 80:c=a; break; // C=A
      case 81:c=b; break; // C=B
      case 82:break; // C=C
      case 83:c=d; break; // C=D
      case 84:c=m(b); break; // C=*B
      case 85:c=m(c); break; // C=*C
      case 86:c=h(d); break; // C=*D
      case 87:c=D[pc++]; break; // C= N
      case 88:d=a; break; // D=A
      case 89:d=b; break; // D=B
      case 90:d=c; break; // D=C
      case 91:break; // D=D
      case 92:d=m(b); break; // D=*B
      case 93:d=m(c); break; // D=*C
      case 94:d=h(d); break; // D=*D
      case 95:d=D[pc++]; break; // D= N
      case 96:m(b)=a; break; // *B=A
      case 97:m(b)=b; break; // *B=B
      case 98:m(b)=c; break; // *B=C
      case 99:m(b)=d; break; // *B=D
      case 100:break; // *B=*B
      case 101:m(b)=m(c); break; // *B=*C
      case 102:m(b)=h(d); break; // *B=*D
      case 103:m(b)=D[pc++]; break; // *B= N
      case 104:m(c)=a; break; // *C=A
      case 105:m(c)=b; break; // *C=B
      case 106:m(c)=c; break; // *C=C
      case 107:m(c)=d; break; // *C=D
      case 108:m(c)=m(b); break; // *C=*B
      case 109:break; // *C=*C
      case 110:m(c)=h(d); break; // *C=*D
      case 111:m(c)=D[pc++]; break; // *C= N
      case 112:h(d)=a; break; // *D=A
      case 113:h(d)=b; break; // *D=B
      case 114:h(d)=c; break; // *D=C
      case 115:h(d)=d; break; // *D=D
      case 116:h(d)=m(b); break; // *D=*B
      case 117:h(d)=m(c); break; // *D=*C
      case 118:break; // *D=*D
      case 119:h(d)=D[pc++]; break; // *D= N
      case 128:a+=a; break; // A+=A
      case 129:a+=b; break; // A+=B
      case 130:a+=c; break; // A+=C
      case 131:a+=d; break; // A+=D
      case 132:a+=m(b); break; // A+=*B
      case 133:a+=m(c); break; // A+=*C
      case 134:a+=h(d); break; // A+=*D
      case 135:a+=D[pc++]; break; // A+= N
      case 136:a-=a; break; // A-=A
      case 137:a-=b; break; // A-=B
      case 138:a-=c; break; // A-=C
      case 139:a-=d; break; // A-=D
      case 140:a-=m(b); break; // A-=*B
      case 141:a-=m(c); break; // A-=*C
      case 142:a-=h(d); break; // A-=*D
      case 143:a-=D[pc++]; break; // A-= N
      case 144:a*=a; break; // A*=A
      case 145:a*=b; break; // A*=B
      case 146:a*=c; break; // A*=C
      case 147:a*=d; break; // A*=D
      case 148:a*=m(b); break; // A*=*B
      case 149:a*=m(c); break; // A*=*C
      case 150:a*=h(d); break; // A*=*D
      case 151:a*=D[pc++]; break; // A*= N
      case 152:div(a); break; // A/=A
      case 153:div(b); break; // A/=B
      case 154:div(c); break; // A/=C
      case 155:div(d); break; // A/=D
      case 156:div(m(b)); break; // A/=*B
      case 157:div(m(c)); break; // A/=*C
      case 158:div(h(d)); break; // A/=*D
      case 159:div(D[pc++]); break; // A/= N
      case 160:mod(a); break; // A%=A
      case 161:mod(b); break; // A%=B
      case 162:mod(c); break; // A%=C
      case 163:mod(d); break; // A%=D
      case 164:mod(m(b)); break; // A%=*B
      case 165:mod(m(c)); break; // A%=*C
      case 166:mod(h(d)); break; // A%=*D
      case 167:mod(D[pc++]); break; // A%= N
      case 168:a&=a; break; // A&=A
      case 169:a&=b; break; // A&=B
      case 170:a&=c; break; // A&=C
      case 171:a&=d; break; // A&=D
      case 172:a&=m(b); break; // A&=*B
      case 173:a&=m(c); break; // A&=*C
      case 174:a&=h(d); break; // A&=*D
      case 175:a&=D[pc++]; break; // A&= N
      case 176:a&=~a; break; // A&~A
      case 177:a&=~b; break; // A&~B
      case 178:a&=~c; break; // A&~C
      case 179:a&=~d; break; // A&~D
      case 180:a&=~m(b); break; // A&~*B
      case 181:a&=~m(c); break; // A&~*C
      case 182:a&=~h(d); break; // A&~*D
      case 183:a&=~D[pc++]; break; // A&~ N
      case 184:a|=a; break; // A|=A
      case 185:a|=b; break; // A|=B
      case 186:a|=c; break; // A|=C
      case 187:a|=d; break; // A|=D
      case 188:a|=m(b); break; // A|=*B
      case 189:a|=m(c); break; // A|=*C
      case 190:a|=h(d); break; // A|=*D
      case 191:a|=D[pc++]; break; // A|= N
      case 192:a^=a; break; // A^=A
      case 193:a^=b; break; // A^=B
      case 194:a^=c; break; // A^=C
      case 195:a^=d; break; // A^=D
      case 196:a^=m(b); break; // A^=*B
      case 197:a^=m(c); break; // A^=*C
      case 198:a^=h(d); break; // A^=*D
      case 199:a^=D[pc++]; break; // A^= N
      case 200:a<<=(a&31); break; // A<<=A
      case 201:a<<=(b&31); break; // A<<=B
      case 202:a<<=(c&31); break; // A<<=C
      case 203:a<<=(d&31); break; // A<<=D
      case 204:a<<=(m(b)&31); break; // A<<=*B
      case 205:a<<=(m(c)&31); break; // A<<=*C
      case 206:a<<=(h(d)&31); break; // A<<=*D
      case 207:a<<=(D[pc++]&31); break; // A<<= N
      case 208:a>>=(a&31); break; // A>>=A
      case 209:a>>=(b&31); break; // A>>=B
      case 210:a>>=(c&31); break; // A>>=C
      case 211:a>>=(d&31); break; // A>>=D
      case 212:a>>=(m(b)&31); break; // A>>=*B
      case 213:a>>=(m(c)&31); break; // A>>=*C
      case 214:a>>=(h(d)&31); break; // A>>=*D
      case 215:a>>=(D[pc++]&31); break; // A>>= N
      case 216:f=1; break; // A==A
      case 217:f=(a==b); break; // A==B
      case 218:f=(a==c); break; // A==C
      case 219:f=(a==d); break; // A==D
      case 220:f=(a==U32(m(b))); break; // A==*B
      case 221:f=(a==U32(m(c))); break; // A==*C
      case 222:f=(a==h(d)); break; // A==*D
      case 223:f=(a==U32(D[pc++])); break; // A== N
      case 224:f=0;break; // A<A
      case 225:f=(a<b); break; // A<B
      case 226:f=(a<c); break; // A<C
      case 227:f=(a<d); break; // A<D
      case 228:f=(a<U32(m(b))); break; // A<*B
      case 229:f=(a<U32(m(c))); break; // A<*C
      case 230:f=(a<h(d)); break; // A<*D
      case 231:f=(a<U32(D[pc++])); break; // A< N
      case 232:f=0;break; // A>A
      case 233:f=(a>b); break; // A>B
      case 234:f=(a>c); break; // A>C
      case 235:f=(a>d); break; // A>D
      case 236:f=(a>U32(m(b))); break; // A>*B
      case 237:f=(a>U32(m(c))); break; // A>*C
      case 238:f=(a>h(d)); break; // A>*D
      case 239:f=(a>U32(D[pc++])); break; // A> N
      case 255:O((pc=hbegin+D[pc]+256*D[pc+1])>=hend)E();break;//LJ
      default: E();
    }
    return 1;
  }
  void div(U32 x) {O (x) a/=x; Y a=0;}
  void mod(U32 x) {O (x) a%=x; Y a=0;}
  void swap(U32& x) {a^=x; x^=a; a^=x;}
  void swap(U8& x)  {a^=x; x^=a; a^=x;}
};


struct Component {
  size_t z, cxt, a, b, c; 
  Array<U32> cm;
  Array<U8> ht;
  Array<U16> a16;
  void init() {
    z=cxt=a=b=c=0;
    cm.resize(0);
    ht.resize(0);
    a16.resize(0);
    }
  Component() {init();}
};

const U8 sns[1024]={
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

struct StateTable {
  U8 ns[1024]; 
  I next(I s, I y) { return ns[s*4+y]; }
  I cminit(I s) { return ((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1); }
  StateTable() {memcpy(ns, sns, sizeof(ns));}
};

struct P {
  P(ZP& zr): c8(1), hmap4(1), z(zr) {
    pcode=0;
    pcode_size=0;
    initTables=false;
  }
  void init();   
  I predict();     
  void update(I y); 
  I stat(I);   
  bool isModeled() {  
    return z.D[6]!=0;
  }

  I c8, hmap4, p[256]; 
  U32 h[256];           // unrolled copy of z.h
  ZP& z;             // VM to compute context hashes, includes H, n
  Component comp[256];  // the model, includes P
  bool initTables;      // are tables initialized?

  I dt2k[256];        // division table Q match: dt2k[i] = 2^12/i
  I dt[1024];         // division table Q cm: dt[i] = 2^16/(i+1.5)
  U16 squasht[4096];    // squash() lookup table
  short stretcht[32768];// stretch() lookup table
  StateTable st;        // next, cminit functions
  U8* pcode;            // JIT code Q predict() and update()
  I pcode_size;       // length of pcode

  void train(Component& cr, I y) {
    // assert(y==0 || y==1);
    U32& pn=cr.cm(cr.cxt);
    U32 count=pn&0x3ff;
    I E=y*32767-(cr.cm(cr.cxt)>>17);
    pn+=(E*dt[count]&-1024)+(count<cr.z);
  }

  I squash(I x) { return squasht[x+2048]; }

  I stretch(I x) { return stretcht[x]; }

  I clamp2k(I x) {
    O (x<-2048) return -2048;
    Y O (x>2047) return 2047;
    Y return x;
  }

  I clamp512k(I x) {
    O (x<-(1<<19)) return -(1<<19);
    Y O (x>=(1<<19)) return (1<<19)-1;
    Y return x;
  }

  size_t find(Array<U8>& ht, I sizebits, U32 cxt) {
    I chk=cxt>>sizebits&255;
    size_t h0=(cxt*16)&(ht.size()-16);
    O (ht[h0]==chk) return h0;
    size_t h1=h0^16;
    O (ht[h1]==chk) return h1;
    size_t h2=h0^32;
    O (ht[h2]==chk) return h2;
    O (ht[h0+1]<=ht[h1+1] && ht[h0+1]<=ht[h2+1])
        return memset(&ht[h0], 0, 16), ht[h0]=chk, h0;
    Y O (ht[h1+1]<ht[h2+1])
        return memset(&ht[h1], 0, 16), ht[h1]=chk, h1;
    Y
        return memset(&ht[h2], 0, 16), ht[h2]=chk, h2;
    }
};

struct Decoder: Reader {
  Reader* in;        // destination
  Decoder(ZP& z):
    in(0), low(1), high(0xFFFFFFFF), curr(0), rpos(0), wpos(0),
    pr(z), buf(BUFSIZE) {}

  I decompress() {
    O (pr.isModeled()) {  // n>0 components?
      O (curr==0) {  // segment initialization
        F(4)curr=curr<<8|get();
      }
      O (decode(0)) {
        // O (curr!=0) E("decoding end of stream");
        return -1;
      }
      Y {
        I c=1;
        while (c<256) {  // get 8 bits
          I p=pr.predict()*2+1;
          c+=c+decode(p);
          pr.update(c&1);
        }
        return c-256;
      }
    }
    Y {
      O (curr==0) {
        F(4)curr=curr<<8|get();
        O (curr==0) return -1;
      }
      --curr;
      return get();
    }
  }

  I skip() {
    I c=-1;
    O (pr.isModeled()) {
      while (curr==0)  // at start?
        curr=get();
      while (curr && (c=get())>=0)  // find 4 zeros
        curr=curr<<8|c;
      while ((c=get())==0) ;  // might be more than 4
      return c;
    }
    Y {
      O (curr==0)  // at start?
        Q (I i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
      while (curr>0) {
        while (curr>0) {
          --curr;
          O (get()<0) return E("skipped to EOF"), -1;
        }
        Q (I i=0; i<4 && (c=get())>=0; ++i) curr=curr<<8|c;
      }
      O (c>=0) c=get();
      return c;
    }
  }

  void init() {
    pr.init();
    O (pr.isModeled()) low=1, high=0xFFFFFFFF, curr=0;
    Y low=high=curr=0;
  }
  I stat(I x) {return pr.stat(x);}
  I get() {        // return 1 byte of buffered input or EOF
    O (rpos==wpos) {
      rpos=0;
      wpos=in ? in->read(&buf[0], BUFSIZE) : 0;
    //   assert(wpos<=BUFSIZE);
    }
    return rpos<wpos ? U8(buf[rpos++]) : -1;
  }
  I buffered() {return wpos-rpos;}  // how far read ahead?
  U32 low, high;     // range
  U32 curr;          // L 4 bytes of archive or remaining bytes in subblock
  U32 rpos, wpos;    // read, write position in buf
  P pr;      // to get p
  enum {BUFSIZE=1<<16};
  Array<char> buf;   // input buffer of size BUFSIZE bytes
  I decode(I p) {
    U32 mid=low+U32(((high-low)*U64(U32(p)))>>16); 
    I y;
    O (curr<=mid) y=1, high=mid; 
    Y y=0, low=mid+1;
    while ((high^low)<0x1000000) { 
      high=high<<8|255;
      low=low<<8;
      low+=(low==0);
      I c=get();
      curr=curr<<8|c;
    }
    return y;
  }
};

struct PostProcessor {
  I state, hsize, ph, pm;
  ZP z;     // holds PCOMP
  PostProcessor(Writer* out): state(0), hsize(0), ph(0), pm(0) {z.output=out;}
  void init(I h, I m) {
    state=hsize=0;
    ph=h;
    pm=m;
    z.clear();
  }
  
  I write(I c) {
    switch (state) {
      case 0: 
        state=c+1; 
        O (state==1) z.clear();
        break;
      case 1: 
        z.outc(c);
        break;
      case 2:
        hsize=c;
        state=3;
        break;
      case 3:
        hsize+=c*256; 
        z.D.resize(hsize+300);
        z.cend=8;
        z.hbegin=z.hend=z.cend+128;
        z.D[4]=ph;
        z.D[5]=pm;
        state=4;
        break;
      case 4: 
        z.D[z.hend++]=c;  
        O (z.hend-z.hbegin==hsize) {
          hsize=z.cend-2+z.hend-z.hbegin;
          z.D[0]=hsize&255; 
          z.D[1]=hsize>>8;
          z.initp();
          state=5;
        }
        break;
      case 5: 
        z.run(c);
        O (c<0) z.flush();
        break;
    }
    return state;
  }
  I getState() const {return state;}
};

struct D {
  D(Reader* in, Writer* out): z(), dec(z), pp(out) {
    dec.in=in;
  }
  void findBlock() {
    I c = dec.get();
    z.read(&dec);
  }
  void decompress() {
    dec.init();
    pp.init(z.D[4], z.D[5]);

    while ((pp.getState()&3)!=1)
      pp.write(dec.decompress());

    while (1) {
      I c=dec.decompress();
      pp.write(c);
      O (c==-1) {
        return;
      }
    }
  }
  ZP z;
  Decoder dec;
  PostProcessor pp;
};

void decompress(Reader* in, Writer* out) {
  D d{in,out};
  d.findBlock();      // don't calculate memory
  d.decompress();           // to end of segment
}

struct Encoder {
  Encoder(ZP& z, I size=0): out(0), low(1), high(0xFFFFFFFF), pr(z) {}
  void init() {
    low=1;
    high=0xFFFFFFFF;
    pr.init();
    O (!pr.isModeled()) low=0, buf.resize(1<<16);
  }
  void compress(I c) {
    O (c==-1)
      encode(1, 0);
    Y {
      encode(0, 0);
      Q (I i=7; i>=0; --i) {
        I p=pr.predict()*2+1;
        I y=c>>i&1;
        encode(y, p);
        pr.update(y);
      }
    }
  }
  I stat(I x) {return pr.stat(x);}
  Writer* out; 
  U32 low, high;
  P pr;  
  Array<char> buf; 
  void encode(I y, I p) {
    U32 mid=low+U32(((high-low)*U64(U32(p)))>>16); 
    O (y) high=mid; Y low=mid+1; 
    while ((high^low)<0x1000000) {
      out->put(high>>24);  
      high=high<<8|255;
      low=low<<8;
      low+=(low==0); 
    }
  }
};

struct Compiler {
  Compiler(const char* in, I* args, ZP& hz, ZP& pz);
  const char* in;  // ZP source code
  I* args;       // Array of up to 9 args, default NULL = all 0
  ZP &hz, &pz;       // Output of PCOMP section
  // Writer* out2;    // Output ... of "PCOMP ... ;"
  I line, state;       // parse state: 0=space -1=word >0 (nest level)

  // Symbolic constants
  typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,
    JT=39,JF=47,JMP=63,LJ=255,
    POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,
    WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON} CompType;

  void next() {
    Q (; *in; ++in) {
      O (*in=='\n') ++line;
      O (*in=='(') state+=1+(state<0);
      Y O (state>0 && *in==')') --state;
      Y O (state<0 && *in<=' ') state=0;
      Y O (state==0 && *in>' ') {state=-1; break;}
    }
  }
  bool matchToken(const char* word) {
    const char* a=in;
    Q (; (*a>' ' && *a!='(' && *word); ++a, ++word)
      O (tolower(*a)!=tolower(*word)) return false;
    return !*word && (*a<=' ' || *a=='(');
  }
  I rtoken(I low, I high) {
    next();
    I r=0;
    O (in[0]=='$' && in[1]>='1' && in[1]<='9') {
      O (in[2]=='+') r=atoi(in+3);
      O (args) r+=args[in[1]-'1'];
    }
    Y O (in[0]=='-' || (in[0]>='0' && in[0]<='9')) r=atoi(in);
    return r;
  }
  I rtoken(const char* list[]) {
    next();
    Q (I i=0; list[i]; ++i)
      O (matchToken(list[i]))
        return i;
    E();
    return -1; // not reached
  }
  I compile_comp(ZP& z);      // compile either HCOMP or PCOMP

  // Stack of n elements
  struct Stack {
    LQ::Array<U16> s;
    size_t top;
    Stack(I n): s(n), top(0) {}
    void push(const U16& x) {
      O (top>=s.size()) E();// E("O or DO nested too deep");
      s[top++]=x;
    }
    U16 pop() {
      O (top<=0) E();// E("unmatched O or DO");
      return s[--top];
    }
  };

  Stack if_stack, do_stack;
};

struct C {
  C(Reader* i, Writer* out): enc(z), in(i), state(INIT) {enc.out=out;}
  void startBlock(const char* config,     // ZP source code
                  I* args) {
    Compiler(config, args, z, pz);
    enc.out->put(1+(z.D[6]==0));  // level 1 or 2
    z.write(enc.out, false);
    state=BLOCK1;
  }
  void startSegment() {
    O (state==BLOCK1) state=SEG1;
    O (state==BLOCK2) state=SEG2;
  }
  void postProcess(const char* pcomp = 0, I len = 0); 
  bool compress(I n = -1); 
  void endSegment() {
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
  I stat(I x) {return enc.stat(x);}
  ZP z, pz; 
  Encoder enc; 
  Reader* in; 
  enum {INIT, BLOCK1, SEG1, BLOCK2, SEG2} state;
};

struct SB: Reader, Writer {
  unsigned char* p;
  size_t al,wpos,rpos,limit,init;

  // Increase capacity to a without changing size
  void reserve(size_t a) {
    O (a<=al) return;
    unsigned char* q=0;
    O (a>0) q=(unsigned char*)(p ? realloc(p, a) : malloc(a));
    O (a>0 && !q) E(); // E("Out of memory");
    p=q;
    al=a;
  }

  // Enlarge al to make room to write at least n bytes.
  void lengthen(size_t n) {
    // assert(wpos<=al);
    // O (wpos+n>limit || wpos+n<wpos) E("SB overflow");
    O (wpos+n<=al) return;
    size_t a=al;
    while (wpos+n>=a) a=a*2+init;
    reserve(a);
  }

  void operator=(const SB&);
  SB(const SB&);

  unsigned char* data() {return p;}

  SB(size_t n=0): p(0), al(0), wpos(0), rpos(0), limit(size_t(-1)), init(n>128?n:128) {}

  ~SB() {O (p) free(p);}

  size_t size() const {return wpos;}

  void put(I c) {  // write 1 byte
    lengthen(1);
    p[wpos++]=c;
  }

  void write(const char* buf, I n) {
    O (n<1) return;
    lengthen(n);
    O (buf) memcpy(p+wpos, buf, n);
    wpos+=n;
  }

  I get() {
    return rpos<wpos ? p[rpos++] : -1;
  }

  I read(char* buf, I n) {
    O (rpos+n>wpos) n=wpos-rpos;
    O (n>0 && buf) memcpy(buf, p+rpos, n);
    rpos+=n;
    return n;
  }

  const char* c_str() const {return (const char*)p;}

  void resize(size_t i) {
    wpos=i;
    O (rpos>wpos) rpos=wpos;
  }

  void swap(SB& s) {
    ::swap(p, s.p);
    ::swap(al, s.al);
    ::swap(wpos, s.wpos);
    ::swap(rpos, s.rpos);
    ::swap(limit, s.limit);
  }
};


double pow2(I x) {
  double r=1;
  Q (; x>0; x--) r+=r;
  return r;
}

U8 stdt[712]={
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

// Initialize the P with a new model in z
void P::init() {
  z.inith();

  O (!initTables && isModeled()) {
    initTables=true;

    dt2k[0] = 0;
    Q (I i = 1; i < 256; ++i) {
        dt2k[i]=2048/i;
    }

    F(1024)dt[i]=(1<<17)/(i*2+3)*2;

    memset(squasht, 0, (1376 + 7)*2);

    Q (I i=1376 + 7; i < 1376 + 1344; ++i) {
        // todo division by 0
        // ??
        squasht[i]=(U16)(32768.0 / (1 + exp((i - 2048) * (-1.0 / 64))));
    }
    Q (I i=2720; i<4096; ++i) squasht[i]=32767;

    I k=16384;
    F(712)
      Q (I j=stdt[i]; j>0; --j)
        stretcht[k++]=i;

    F(16384)stretcht[i]=-stretcht[32767-i];
  }

  F(256)h[i]=p[i]=0;

  F(256)comp[i].init();
  I n=z.D[6];
  const U8* cp=&z.D[7]; 
  F(n){
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  // c
        p[i]=(cp[1]-128)*4;
        break;
      case CM:
        cr.cm.resize(1, cp[1]); 
        cr.z=cp[2]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=0x80000000;
        break;
      case ICM:
        cr.z=1023;
        cr.cm.resize(256);
        cr.ht.resize(64, cp[1]);
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=st.cminit(j);
        break;
      case MATCH:
        cr.cm.resize(1, cp[1]);  // index
        cr.ht.resize(1, cp[2]);  // buf
        cr.ht(0)=1;
        break;
      case AVG: // j k wt
        break;
      case MIX2:  // sizebits j k rate mask
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.a16.resize(1, cp[1]);  // wt[size][m]
        Q (size_t j=0; j<cr.a16.size(); ++j)
          cr.a16[j]=32768;
        break;
      case MIX: {  // sizebits j m rate mask
        I m=cp[3];  // number of inputs
        cr.c=(size_t(1)<<cp[1]); // size (number of contexts)
        cr.cm.resize(m, cp[1]);  // wt[size][m]
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=65536/m;
        break;
      }
      case ISSE:  // sizebits j
        cr.ht.resize(64, cp[1]);
        cr.cm.resize(512);
        Q (I j=0; j<256; ++j) {
          cr.cm[j*2]=1<<15;
          cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);
        }
        break;
      case SSE: // sizebits j start limit
        cr.cm.resize(32, cp[1]);
        cr.z=cp[4]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];
        break;
      default: E();
    }
    cp+=compsize[*cp];
  }
}

// Return next bit prediction using Ierpreted COMP code
I P::predict() {

  // Predict next bit
  I n=z.D[6];
  const U8* cp=&z.D[7];
  F(n){
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  // c
        break;
      case CM:  // sizebits limit
        cr.cxt=h[i]^hmap4;
        p[i]=stretch(cr.cm(cr.cxt)>>17);
        break;
      case ICM: // sizebits
        // assert((hmap4&15)>0);
        O (c8==1 || (c8&0xf0)==16) cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];
        p[i]=stretch(cr.cm(cr.cxt)>>8);
        break;
      case MATCH: // sizebits bufbits: a=len, b=offset, c=bit, cxt=bitpos,
                  //                   ht=buf, limit=pos
        O (cr.a==0) p[i]=0;
        Y {
          cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1; // predicted bit
          p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        }
        break;
      case AVG: // j k wt
        p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;
        break;
      case MIX2: { // sizebits j k rate mask
                   // c=size cm=wt[size] cxt=input
        cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));
        // assert(cr.cxt<cr.a16.size());
        I w=cr.a16[cr.cxt];
        // assert(w>=0 && w<65536);
        p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;
        // assert(p[i]>=-2048 && p[i]<2048);
      }
        break;
      case MIX: {  // sizebits j m rate mask
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
        break;
      case ISSE: { // sizebits j -- c=hi, cxt=bh
        // assert((hmap4&15)>0);
        O (c8==1 || (c8&0xf0)==16)
          cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];  // bit history
        I *wt=(I*)&cr.cm[cr.cxt*2];
        p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);
      }
        break;
      case SSE: { // sizebits j start limit
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
        break;
      default:
        E("component predict not implemented");
    }
    cp+=compsize[cp[0]];
  }
  return squash(p[n-1]);
}

// Update model with decoded bit y (0...1)
void P::update(I y) {
  // Update components
  const U8* cp=&z.D[7];
  I n=z.D[6];
//   assert(n>=1 && n<=255);
//   assert(cp[-1]==n);
  F(n){
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
        pn+=I(y*32767-(pn>>8))>>2;
      }
        break;
      case MATCH: // sizebits bufbits:
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
        break;
      case AVG:  // j k wt
        break;
      case MIX2: { // sizebits j k rate mask
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
        break;
      case MIX: {   // sizebits j m rate mask
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
        break;
      case ISSE: { // sizebits j  -- c=hi, cxt=bh
        // assert(cr.cxt==cr.ht[cr.c+(hmap4&15)]);
        I err=y*32767-squash(p[i]);
        I *wt=(I*)&cr.cm[cr.cxt*2];
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
    F(n)h[i]=z.H(i);
  }
  Y O (c8>=16 && c8<32)
    hmap4=(hmap4&0xf)<<5|y<<4|1;
  Y
    hmap4=(hmap4&0x1f0)|(((hmap4&0xf)*2+y)&0xf);
}

const char*compname[256]=
  {"","const","cm","icm","match","avg","mix2","mix","isse","sse",0};

const char*opcodelist[272]={
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

// Compile HCOMP or PCOMP code. Exit on E. Return
// code Q end token (POST, PCOMP, END)
I Compiler::compile_comp(ZP& z) {
  I op=0;
  const I comp_begin=z.hend;
  while (true) {
    op=rtoken(opcodelist);
    O (op==POST || op==PCOMP || op==END) break;
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
      O (z.D[a-1]!=LJ) {  // O, IFNOT
        I j=z.hend-a+1+(op==LJ); // offset at O
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
      I j=z.hend-a-1;  // jump offset
      O (z.D[a-1]!=LJ) {
        z.D[a]=j;
      }
      Y {
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
    // O (z.hend>=z.D.isize()-130 || z.hend-z.hbegin+z.cend-2>65535)
  }
  z.D[z.hend++]=(0); // END
  return op;
}

// Compile a configuration file. Store COMP/HCOMP section in hcomp.
// O there is a PCOMP section, store it in pcomp and store the PCOMP
// command in pcomp_cmd. Replace "$1..$9+n" with args[0..8]+n

Compiler::Compiler(const char* in_, I* args_, ZP& hz_, ZP& pz_): in(in_), args(args_), hz(hz_), pz(pz_),
                   if_stack(1000), do_stack(1000) {
  line=1;
  state=0;
  hz.clear();
  pz.clear();
  hz.D.resize(68000); 

  // Compile the COMP section of D
  next();
  hz.D[2]=rtoken(0, 255);  // hh
  hz.D[3]=rtoken(0, 255);  // hm
  hz.D[4]=rtoken(0, 255);  // ph
  hz.D[5]=rtoken(0, 255);  // pm
  const I n=hz.D[6]=rtoken(0, 255);  // n
  hz.cend=7;
  F(n){
    rtoken(i, i);
    CompType type=CompType(rtoken(compname));
    hz.D[hz.cend++]=type;
    I clen=LQ::compsize[type&255];
    Q (I j=1; j<clen; ++j)
      hz.D[hz.cend++]=rtoken(0, 255);  // component arguments
  }
  hz.D[hz.cend++];  // end
  hz.hbegin=hz.hend=hz.cend+128;

  // Compile HCOMP
  next();
  I op=compile_comp(hz);

  // Compute D size
  I hsize=hz.cend-2+hz.hend-hz.hbegin;
  hz.D[0]=hsize&255;
  hz.D[1]=hsize>>8;

  // Compile POST 0 END
  O (op==POST) {
    rtoken(0, 0);
    next();
  }

  // Compile PCOMP pcomp_cmd ; program... END
  Y O (op==PCOMP) {
    pz.D.resize(68000);
    pz.D[4]=hz.D[4];  // ph
    pz.D[5]=hz.D[5];  // pm
    pz.cend=8;
    pz.hbegin=pz.hend=pz.cend+128;

    // get pcomp_cmd ending with ";" (case sensitive)
    next();
    while (*in && *in!=';') {
      ++in;
    }
    O (*in) ++in;

    // Compile PCOMP
    op=compile_comp(pz);
    I len=pz.cend-2+pz.hend-pz.hbegin;  // insert D size
    pz.D[0]=len&255;
    pz.D[1]=len>>8;
  }
}

struct MemoryReader: Reader {
  const char* p;
  MemoryReader(const char* p_): p(p_) {}
  I get() {return *p++&255;}
};

void C::postProcess(const char* pcomp, I len) {
  O (state==SEG2) return;
  enc.init();
  O (!pcomp) {
    len=pz.hend-pz.hbegin;
    O (len>0) {
      pcomp=(const char*)&pz.D[pz.hbegin];
    }
  }
  Y O (len==0) {
    len=toU16(pcomp);
    pcomp+=2;
  }
  O (len>0) {
    enc.compress(1);
    enc.compress(len&255);
    enc.compress((len>>8)&255);
    F(len)enc.compress(pcomp[i]&255);
  }
  Y
    enc.compress(0);
  state=SEG2;
}

bool C::compress(I n) {
  O (state==SEG1)
    postProcess();

  const I BUFSIZE=1<<14;
  char buf[BUFSIZE];  // input buffer
  while (n) {
    I nbuf=BUFSIZE;  // bytes read Io buf
    O (n>=0 && n<nbuf) nbuf=n;
    I nr=in->read(buf, nbuf);
    O (nr<=0) return false;
    O (n>=0) n-=nr;
    F(nr){
      I ch=U8(buf[i]);
      enc.compress(ch);
    }
  }
  return true;
}

void ZP::run(U32 input) {
    pc=hbegin;
  a=input;
  while (execute()) ;
}

#define AS 256
#define BS (AS*AS)
#define SSIT 8
#define SSB 1024
#define TR_STACKSIZE 64


#define SWAP(_a,_b)do{t=(_a);(_a)=(_b);(_b)=t;}while(0)
#define MIN(_a,_b)(((_a)<(_b))?(_a):(_b))
#define MAX(_a,_b)(((_a)>(_b))?(_a):(_b))
#define SP(_a,_b,_c,_d)do{assert(ssize<STACK_SIZE);stack[ssize].a=(_a),stack[ssize].b=(_b),stack[ssize].c=(_c),stack[ssize++].d=(_d);}while(0)
#define S5(_a,_b,_c,_d,_e)do{assert(ssize < STACK_SIZE);stack[ssize].a = (_a), stack[ssize].b = (_b),stack[ssize].c = (_c), stack[ssize].d = (_d), stack[ssize++].e = (_e);}while(0)
#define STACK_POP(_a,_b,_c,_d)do{assert(0<=ssize);O(ssize==0){return;}(_a) = stack[--ssize].a, (_b) = stack[ssize].b,(_c) = stack[ssize].c, (_d) = stack[ssize].d;}while(0)
#define STACK_POP5(_a,_b,_c,_d,_e)do{assert(0<=ssize);O(ssize==0){return;}(_a) = stack[--ssize].a, (_b) = stack[ssize].b,(_c) = stack[ssize].c, (_d) = stack[ssize].d, (_e) = stack[ssize].e;}while(0)
#define BUCKET_A(_c0)bucket_A[(_c0)]
#define BUCKET_B(_c0,_c1)(bucket_B[((_c1)<<8)|(_c0)])
#define BB(_c0, _c1)(bucket_B[((_c0)<<8)|(_c1)])


#define R(v) v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
const I lg_table[256]= {
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

I ss_ilg(I n) {
  return (n & 0xff00) ?
          8 + lg_table[(n >> 8) & 0xff] :
          0 + lg_table[(n >> 0) & 0xff];
}

I dqq_table[256] = {0};


I ss_isqrt(I x) {
  I y, e;

  O(x >= (SSB * SSB)) { return SSB; }
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
    }
  }

  O(e >= 16) {
    y = dqq_table[x >> ((e - 6) - (e & 1))] << ((e >> 1) - 7);
    O(e >= 24) { y = (y + 1 + x / y) >> 1; }
    y = (y + 1 + x / y) >> 1;
  } Y O(e >= 8) {
    y = (dqq_table[x >> ((e - 6) - (e & 1))] >> (7 - (e >> 1))) + 1;
  } Y {
    return dqq_table[x] >> 4;
  }

  return (x < (y * y)) ? y - 1 : y;
}

I ss_compare(const unsigned char *T,
           const I *p1, const I *p2,
           I depth) {
  const unsigned char *U1, *U2, *U1n, *U2n;

  Q(U1 = T + depth + *p1,
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


void ss_insertionsort(const unsigned char *T, const I *PA,
                 I *F, I *L, I depth) {
  I *i, *j, t, r;

  Q(i = L - 2; F <= i; --i) {
    Q(t = *i, j = i + 1; 0 < (r = ss_compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < L) && (*j < 0));
      O(L <= j) { break; }
    }
    O(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}

void ss_fixdown(const unsigned char *Td, const I *PA,
           I *SA, I i, I size) {
  I j, k, v, c, d, e;

  Q(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    O(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    O(d <= c) { break; }
  }
  SA[i] = v;
}

void ss_heapsort(const unsigned char *Td, const I *PA, I *SA, I size) {
  I i, m, t;

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

I *
ss_median3(const unsigned char *Td, const I *PA,
           I *v1, I *v2, I *v3) {
  I *t;
  O(Td[PA[*v1]] > Td[PA[*v2]]) { SWAP(v1, v2); }
  O(Td[PA[*v2]] > Td[PA[*v3]]) {
    O(Td[PA[*v1]] > Td[PA[*v3]]) { return v1; }
    Y { return v3; }
  }
  return v2;
}

I*ss_median5(const unsigned char *Td, const I *PA,
           I *v1, I *v2, I *v3, I *v4, I *v5) {
  I *t;
  O(Td[PA[*v2]] > Td[PA[*v3]]) { SWAP(v2, v3); }
  O(Td[PA[*v4]] > Td[PA[*v5]]) { SWAP(v4, v5); }
  O(Td[PA[*v2]] > Td[PA[*v4]]) { SWAP(v2, v4); SWAP(v3, v5); }
  O(Td[PA[*v1]] > Td[PA[*v3]]) { SWAP(v1, v3); }
  O(Td[PA[*v1]] > Td[PA[*v4]]) { SWAP(v1, v4); SWAP(v3, v5); }
  O(Td[PA[*v3]] > Td[PA[*v4]]) { return v4; }
  return v3;
}

/* Returns the pivot element. */
I*ss_pivot(const unsigned char *Td, const I *PA, I *F, I *L) {
  I *M;
  I t;

  t = L - F;
  M = F + t / 2;

  O(t <= 512) {
    O(t <= 32) {
      return ss_median3(Td, PA, F, M, L - 1);
    } Y {
      t >>= 2;
      return ss_median5(Td, PA, F, F + t, M, L - 1 - t, L - 1);
    }
  }
  t >>= 3;
  F  = ss_median3(Td, PA, F, F + t, F + (t << 1));
  M = ss_median3(Td, PA, M - t, M, M + t);
  L   = ss_median3(Td, PA, L - 1 - (t << 1), L - 1 - t, L - 1);
  return ss_median3(Td, PA, F, M, L);
}


/*---------------------------------------------------------------------------*/

/* Binary partition Q substrings. */
I*ss_partition(const I *PA,
                    I *F, I *L, I depth) {
  I *a, *b, t;
  Q(a = F - 1, b = L;;) {
    Q(; (++a < b) && ((PA[*a] + depth) >= (PA[*a + 1] + 1));) { *a = ~*a; }
    Q(; (a < --b) && ((PA[*b] + depth) <  (PA[*b + 1] + 1));) { }
    O(b <= a) { break; }
    t = ~*b;
    *b = *a;
    *a = t;
  }
  O(F < a) { *F = ~*F; }
  return a;
}

/* Multikey Irosort Q medium size groups. */
void ss_mIrosort(const unsigned char*T, const I*PA,
              I*F, I*L,
              I depth) {
#define STACK_SIZE 16
  struct { I *a, *b, c, d; } stack[STACK_SIZE];
  const unsigned char *Td;
  I *a, *b, *c, *d, *e, *f, s, t, ssize, limit, v, x = 0;

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
          O(1 < (a - F)) { break; }
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

void
ss_blockswap(I *a, I *b, I n) {
  I t;
  Q(; 0 < n; --n, ++a, ++b) {
    t = *a, *a = *b, *b = t;
  }
}

void
ss_rotate(I *F, I *M, I *L) {
  I *a, *b, t;
  I l, r;
  l = M - F, r = L - M;
  Q(; (0 < l) && (0 < r);) {
    O(l == r) { ss_blockswap(F, M, l); break; }
    O(l < r) {
      a = L - 1, b = M - 1;
      t = *a;
      do {
        *a-- = *b, *b-- = *a;
        O(b < F) {
          *a = t;
          L = a;
          O((r -= l + 1) <= l) { break; }
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
          O((l -= r + 1) <= r) { break; }
          a += 1, b = M;
          t = *a;
        }
      } while(1);
    }
  }
}


/*---------------------------------------------------------------------------*/

void
ss_inplacemerge(const unsigned char*T, const I*PA,
                I*F, I*M, I*L,
                I depth) {
  const I*p;
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
      O(F == M) { break; }
    }
    --L;
    O(x != 0) { while(*--L < 0) { } }
    O(M == L) { break; }
  }
}


/*---------------------------------------------------------------------------*/

/* Merge-Qward with Iernal buffer. */
void
ss_mergeQward(const unsigned char *T, const I *PA,
                I *F, I *M, I *L,
                I *buf, I depth) {
  I *a, *b, *c, *bufend;
  I t,r;

  bufend = buf + (M - F) - 1;
  ss_blockswap(buf, F, M - F);

  Q(t = *(a = F), b = buf, c = M;;) {
    r = ss_compare(T, PA + *b, PA + *c, depth);
    O(r < 0) {
      do {
        *a++ = *b;
        O(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);
    } Y O(r > 0) {
      do {
        *a++ = *c, *c++ = *a;
        O(L <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    } Y {
      *c = ~*c;
      do {
        *a++ = *b;
        O(bufend <= b) { *bufend = t; return; }
        *b++ = *a;
      } while(*b < 0);

      do {
        *a++ = *c, *c++ = *a;
        O(L <= c) {
          while(b < bufend) { *a++ = *b, *b++ = *a; }
          *a = *b, *b = t;
          return;
        }
      } while(*c < 0);
    }
  }
}

/* Merge-backward with Iernal buffer. */
static
void
ss_mergebackward(const unsigned char *T, const I *PA,
                 I *F, I *M, I *L,
                 I *buf, I depth) {
  const I *p1, *p2;
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
      O(b <= buf) { *buf = t; break; }
      *b-- = *a;
      O(*b < 0) { p1 = PA + ~*b; x |= 1; }
      Y       { p1 = PA +  *b; }
    } Y O(r < 0) {
      O(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      O(c < F) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      O(*c < 0) { p2 = PA + ~*c; x |= 2; }
      Y       { p2 = PA +  *c; }
    } Y {
      O(x & 1) { do { *a-- = *b, *b-- = *a; } while(*b < 0); x ^= 1; }
      *a-- = ~*b;
      O(b <= buf) { *buf = t; break; }
      *b-- = *a;
      O(x & 2) { do { *a-- = *c, *c-- = *a; } while(*c < 0); x ^= 2; }
      *a-- = *c, *c-- = *a;
      O(c < F) {
        while(buf < b) { *a-- = *b, *b-- = *a; }
        *a = *b, *b = t;
        break;
      }
      O(*b < 0) { p1 = PA + ~*b; x |= 1; }
      Y       { p1 = PA +  *b; }
      O(*c < 0) { p2 = PA + ~*c; x |= 2; }
      Y       { p2 = PA +  *c; }
    }
  }
}

/* D&C based merge. */
void
ss_swapmerge(const unsigned char *T, const I *PA,
             I *F, I *M, I *L,
             I *buf, I bufsize, I depth) {
#define STACK_SIZE 32
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
  I m, len, half,ssize,check, next;

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

void sssort(const unsigned char *T, const I *PA,
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

  O((bufsize < SSB) &&
      (bufsize < (L - F)) &&
      (bufsize < (Z = ss_isqrt(L - F)))) {
    O(SSB < Z) { Z = SSB; }
    buf = M = L - Z, bufsize = Z;
  } Y {
    M = L, Z = 0;
  }
  Q(a = F, i = 0; SSB < (M - a); a += SSB, ++i) {
    ss_mIrosort(T, PA, a, a + SSB, depth);
    curbufsize = L - (a + SSB);
    curbuf = a + SSB;
    O(curbufsize <= bufsize) { curbufsize = bufsize, curbuf = buf; }
    Q(b = a, k = SSB, j = i; j & 1; b -= k, k <<= 1, j >>= 1) {
      ss_swapmerge(T, PA, b - k, b, b + k, curbuf, curbufsize, depth);
    }
  }
  ss_mIrosort(T, PA, a, M, depth);
  Q(k = SSB; i != 0; k <<= 1, i >>= 1) {
    O(i & 1) {
      ss_swapmerge(T, PA, a - k, a, M, buf, bufsize, depth);
      a -= k;
    }
  }
  O(Z != 0) {
    ss_mIrosort(T, PA, M, L, depth);
    ss_inplacemerge(T, PA, F, M, L, depth);
  }

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

I tr_ilg(I n) {
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
}

void tr_insertionsort(const I *X, I *F, I *L) {
  I *a, *b;
  I t, r;

  Q(a = F + 1; a < L; ++a) {
    Q(t = *a, b = a - 1; 0 > (r = X[t] - X[*b]);) {
      do { *(b + 1) = *b; } while((F <= --b) && (*b < 0));
      O(b < F) { break; }
    }
    O(r == 0) { *b = ~*b; }
    *(b + 1) = t;
  }
}

void tr_fixdown(const I *X, I *SA, I i, I size) {
  I j, k, v, c, d, e;

  Q(v = SA[i], c = X[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = X[SA[k = j++]];
    O(d < (e = X[SA[j]])) { k = j; d = e; }
    O(d <= c) { break; }
  }
  SA[i] = v;
}

void tr_heapsort(const I *X, I *SA, I size) {
  I i, m, t;

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

I*tr_median3(const I *X, I *v1, I *v2, I *v3) {
  I *t;
  O(X[*v1] > X[*v2]) { SWAP(v1, v2); }
  O(X[*v2] > X[*v3]) {
    O(X[*v1] > X[*v3]) { return v1; }
    Y { return v3; }
  }
  return v2;
}

I*tr_median5(const I *X,
           I *v1, I *v2, I *v3, I *v4, I *v5) {
  I *t;
  O(X[*v2] > X[*v3]) { SWAP(v2, v3); }
  O(X[*v4] > X[*v5]) { SWAP(v4, v5); }
  O(X[*v2] > X[*v4]) { SWAP(v2, v4); SWAP(v3, v5); }
  O(X[*v1] > X[*v3]) { SWAP(v1, v3); }
  O(X[*v1] > X[*v4]) { SWAP(v1, v4); SWAP(v3, v5); }
  O(X[*v3] > X[*v4]) { return v4; }
  return v3;
}

I*tr_pivot(const I *X, I *F, I *L) {
  I *M, t;

  t = L - F;
  M = F + t / 2;

  O(t <= 512) {
    O(t <= 32) {
      return tr_median3(X, F, M, L - 1);
    } Y {
      t >>= 2;
      return tr_median5(X, F, F + t, M, L - 1 - t, L - 1);
    }
  }
  t >>= 3;
  F  = tr_median3(X, F, F + t, F + (t << 1));
  M = tr_median3(X, M - t, M, M + t);
  L   = tr_median3(X, L - 1 - (t << 1), L - 1 - t, L - 1);
  return tr_median3(X, F, M, L);
}


struct tr {
  I c,r,i,n;
};

void trbudget_init(tr *b, I c, I i) {
  b->c = c;
  b->r = b->i = i;
}

I trbudget_check(tr *b, I size) {
  O(size <= b->r) { b->r -= size; return 1; }
  O(b->c == 0) { b->n += size; return 0; }
  b->r += b->i - size;
  b->c -= 1;
  return 1;
}


/*---------------------------------------------------------------------------*/

void tr_partition(const I *X,
             I *F, I *M, I *L,
             I **pa, I **pb, I v) {
  I *a, *b, *c, *d, *e, *f, t, s, x = 0;

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

void tr_copy(I *ISA, const I *SA,
        I *F, I *a, I *b, I *L,
        I depth) {
  I *c, *d, *e,s, v;

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

void tr_partialcopy(I *ISA, const I *SA,
               I *F, I *a, I *b, I *L,
               I depth) {
  I *c, *d, *e,s, v,rank, Lrank, newrank = -1;

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

void tr_Irosort(I *ISA, const I *X,
             I *SA, I *F, I *L,
             tr *T) {
#define STACK_SIZE TR_STACKSIZE
  struct { const I *a; I *b, *c; I d, e; }stack[STACK_SIZE];
  I *a, *b, *c,t,v, x = 0,incr = X - ISA,Z, next,ssize, TR = -1;

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

    O((L - F) <= 8) {
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


void trsort(I *ISA, I *SA, I n, I depth) {
  I *X;
  I *F, *L;
  tr b;
  I t, skip, unsorted;

  trbudget_init(&b, tr_ilg(n) * 2 / 3, n);
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
    O(unsorted == 0) { break; }
  }
}


/*---------------------------------------------------------------------------*/

/* Sorts suffixes of type B*. */
I sort_typeBstar(const unsigned char *T, I *SA,
               I *bucket_A, I *bucket_B,
               I n) {
  I *PAb, *ISAb, *buf, i, j, k, t, m, bufsize, c0, c1;

  /* Initialize bucket arrays. */
  Q(i = 0; i < AS; ++i) { bucket_A[i] = 0; }
  Q(i = 0; i < BS; ++i) { bucket_B[i] = 0; }

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

    /* Compute ranks of type B* substrings. */
    Q(i = m - 1; 0 <= i; --i) {
      O(0 <= SA[i]) {
        j = i;
        do { ISAb[SA[i]] = i; } while((0 <= --i) && (0 <= SA[i]));
        SA[i + 1] = i - j;
        O(i <= 0) { break; }
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

  return m;
}

void construct_SA(const unsigned char *T, I *SA,
             I *bucket_A, I *bucket_B,
             I n, I m) {
  I *i, *j, *k, s, c0, c1, c2;

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

I divsufsort(const unsigned char *T, I *SA, I n) {
  I *bucket_A, *bucket_B;
  I m;
  I err = 0;

  /* Check arguments. */
  O((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  Y O(n == 0) { return 0; }
  Y O(n == 1) { SA[0] = 0; return 0; }
  Y O(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; return 0; }

  bucket_A = (I *)malloc(AS * sizeof(I));
  bucket_B = (I *)malloc(BS * sizeof(I));

  /* Suffixsort. */
  O((bucket_A != NULL) && (bucket_B != NULL)) {
    m = sort_typeBstar(T, SA, bucket_A, bucket_B, n);
    construct_SA(T, SA, bucket_A, bucket_B, n, m);
  } Y {
    err = -2;
  }

  free(bucket_B);
  free(bucket_A);

  return err;
}



// E8E9 transQm of buf[0..n-1] to improve compression of .exe and .dll.
// Patterns (E8|E9 xx xx xx 00|FF) at offset i replace the 3 M
// bytes with x+i mod 2^24, LSB F, reading backward.
// void e8e9(unsigned char* buf, I n) {
//     E("unsupported");

//   Q (I i=n-5; i>=0; --i) {
//     O (((buf[i]&254)==0xe8) && ((buf[i+4]+1)&254)==0) {
//       unsigned a=(buf[i+1]|buf[i+2]<<8|buf[i+3]<<16)+i;
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

struct LZBuffer: LQ::Reader {
  LQ::Array<unsigned> ht;// hash table, confirm in low bits, or SA+ISA
  const unsigned char* in;    // input poIer
  I checkbits, level, mBoth;       
  unsigned htsize, n, i, m, m2, maxMatch, maxLiteral, lookahead, h1, h2, bucket, shift1, shift2, rb, bits, nbits, rpos, wpos, idx;               // BWT index
  const unsigned* sa;         // suffix array Q BWT or LZ77-SA
  unsigned* isa;              // inverse suffix array Q LZ77-SA
  enum {BUFSIZE=1<<14};       // output buffer size
  unsigned char buf[BUFSIZE]; // output buffer

  void write_literal(unsigned i, unsigned& lit) {
    O (level==1) {
      O (lit<1) return;
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
        unsigned lit1=lit;
        O (lit1>64) lit1=64;
        put(lit1-1);
        Q (unsigned j=i-lit; j<i-lit+lit1; ++j) put(in[j]);
        lit-=lit1;
      }
    }
  }
  void write_match(unsigned len, unsigned off);
  void fill();  // encode to buf

  // write k bits of x
  void putb(unsigned x, I k) {
    x&=(1<<k)-1;
    bits|=x<<nbits;
    nbits+=k;
    while (nbits>7) {
    //   assert(wpos<BUFSIZE);
      buf[wpos++]=bits, bits>>=8, nbits-=8;
    }
  }

  // write L byte
  void flush() {
    // assert(wpos<BUFSIZE);
    O (nbits>0) buf[wpos++]=bits;
    bits=nbits=0;
  }

  // write 1 byte
  void put(I c) {
    // assert(wpos<BUFSIZE);
    buf[wpos++]=c;
  }

  LZBuffer(SB& inbuf, I args[], const unsigned* sap=0):
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

    O (args[5]-args[0]>=21 || level==3) {
      O (sap)
        sa=sap;
      Y {
        sa=&ht[0];
        O (n>0) divsufsort((const unsigned char*)in, (I*)sa, n);
      }
      O (level<3) {
        isa=&ht[n*(sap==0)];
      }
    }
  }

  I get() {
    I c=-1;
    O (rpos==wpos) fill();
    O (rpos<wpos) c=buf[rpos++];
    O (rpos==wpos) rpos=wpos=0;
    return c;
  }

  I read(char* p, I n) {
    O (rpos==wpos) fill();
    I nr=n;
    O (nr>I(wpos-rpos)) nr=wpos-rpos;
    O (nr) memcpy(p, buf+rpos, nr);
    rpos+=nr;
    O (rpos==wpos) rpos=wpos=0;
    return nr;
  }
};

I nbits(unsigned x) {
  I r;
  Q (r=0; x; x>>=1) r+=x&1;
  return r;
}

void LZBuffer::fill() {
  // LZ77: scan the input
  unsigned lit=0;  // number of output literals pending
  const unsigned mask=(1<<checkbits)-1;
  while (i<n && wpos*2<BUFSIZE) {

    // Search Q longest match, or pick closest in case of tie
    unsigned blen=m-1;  // best match length
    unsigned bp=0;  // poIer to best match
    unsigned blit=0;  // literals beQe best match
    I bscore=0;  // best cost

    // Look up contexts in suffix array
    O (isa) {
      O (sa[isa[i&mask]]!=i) // rebuild ISA
        Q (unsigned j=0; j<n; ++j)
          O ((sa[j]&~mask)==(i&~mask))
            isa[sa[j]&mask]=j;
      Q (unsigned h=0; h<=lookahead; ++h) {
        unsigned q=isa[(h+i)&mask];  // location of h+i in SA
        // assert(q<n);
        O (sa[q]!=h+i) continue;
        Q (I j=-1; j<=1; j+=2) {  // search backward and Qward
          Q (unsigned k=1; k<=bucket; ++k) {
            unsigned p;  // match to be tested
            O (q+j*k<n && (p=sa[q+j*k]-h)<i) {
            //   assert(p<n);
              unsigned l, l1;  // length of match, leading literals
              Q (l=h; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              Q (l1=h; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
              I score=I(l-l1)*8-lg(i-p)-4*(lit==0 && l1>0)-11;
              Q (unsigned a=0; a<h; ++a) score=score*5/8;
              O (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              O (l<blen || l<m || l>255) break;
            }
          }
        }
        O (bscore<=0 || blen<m) break;
      }
    }

    // Look up contexts in a hash table.
    // Try the longest context orders F. O a match is found, then
    // skip the lower order as a speed optimization.
    Y O (level==1 || m<=64) {
      O (m2>0) {
        Q (unsigned k=0; k<=bucket; ++k) {
          unsigned p=ht[h2^k];
          O (p && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            O (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              unsigned l;  // match length from lookahead
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
          O (blen>=128) break;
        }
      }

      // Search the lower order context
      O (!m2 || blen<m2) {
        Q (unsigned k=0; k<=bucket; ++k) {
          unsigned p=ht[h1^k];
          O (p && i+3<n && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            O (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              unsigned l;
              Q (l=0; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              I score=l*8-lg(i-p)-2*(lit>0)-11;
              O (score>bscore) blen=l, bp=p, blit=0, bscore=score;
            }
          }
          O (blen>=128) break;
        }
      }
    }

    // O match is long enough, then output any pending literals F,
    // and then the match. blen is the length of the match.
    // assert(i>=bp);
    const unsigned off=i-bp;  // offset
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
          unsigned ih=((i*1234547)>>19)&bucket;
          const unsigned p=(i<<checkbits)|(in[i+3]&mask);
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

// Write match sequence of given length and offset
void LZBuffer::write_match(unsigned len, unsigned off) {
  O (level==1) {
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
      const unsigned len1=len>m*2+63 ? m+63 :
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
string makeConfig(const char* ME, I args[]) {
//   assert(ME);
  const char type=ME[0];
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

  string hdr, pcomp;
  const I level=args[1]&3;
  const bool doe8=args[1]>=4 && args[1]<=7;

  hdr="comp 9 16 0 $1+20 ";
  pcomp="pcomp lzpre c ;\n  a> 255 O\nb=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n out a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++ out d-- a=d a> 0 while endif endif endif halt end ";
  
  I ncomp=0;  // number of components
  const I membits=args[0]+20;
  I sb=5;  // bits in L context
  string comp;
  string hcomp="hcomp\nc-- *c=a a+= 255 d=a *d=c\na=r 1 a== 0 O\na= 111\nY a== 1 O\na=*c r=a 2\na> 63 O a>>= 6 a++ a++\nY a++ a++ endif\nY\na--\nendif endif\nr=a 1\n";

  // Generate the context model
  while (*ME && ncomp<254) {
    // parse command C[N1[,N2]...] Io v = {C, N1, N2...}
    vector<I> v;
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
      Q (unsigned i=3; i<v.size(); ++i)
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
      Q (unsigned i=3; i<v.size(); ++i) {
        O (i==3) hcomp+="b=c ";
        O (v[i]>=256 && v[i]<512) { // lz77 state or masked literal byte
          hcomp+="a=r 1 a> 1 O\na=r 2 a< 64 O\na=*b ";
          O (v[i]<511) hcomp+="a&= "+itos(v[i]-256);
          hcomp+=" hashd\nY\na>>= 6 hashd a=r 1 hashd\nendif\nY\na= 255 hashd a=r 2 hashd\nendif\n";
        }
      }
      ++ncomp;
    }

    O (v[0]=='i' && ncomp>0) {
      hcomp+="d= "+itos(ncomp-1)+" b=c a=*d d++\n";
      Q (unsigned i=1; i<v.size() && ncomp<254; ++i) {
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
  }
  return hdr+itos(ncomp)+"\n"+comp+hcomp+"halt\n"+pcomp;
}

void compressBlock(SB* in, Writer* out) {
  string ME="3";
  const unsigned n=in->size();  // input size
  const I arg0=MAX(lg(n+4095)-20, 0);  // block size

  unsigned type=0;
  I commas=0, arg[4]={0};
  Q (I i=1; i<I(ME.size()) && commas<4; ++i) {
    O (ME[i]==',' || ME[i]=='.') ++commas;
    Y O (isdigit(ME[i])) arg[commas]=arg[commas]*10+ME[i]-'0';
  }
  O (commas==0) type=512;
  Y type=arg[1]*4+arg[2];

  // Expand default methods
  const I level=3;
  // assert(level>=0 && level<=9);

  // build models
  const I doe8=(type&2)*2;
  ME="x"+itos(arg0);
  string htsz=","+itos(19+arg0+(arg0<=6));  // lz77 hash table size
  string sasz=","+itos(21+arg0);            // lz77 suffix array size

  ME+=","+itos(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";

  // Compress
  string config;
  I args[9]={0};
  config=makeConfig(ME.c_str(), args);

  LZBuffer lz(*in, args);
  LQ::C co{&lz,out};

  co.startBlock(config.c_str(), args);
  string cs=itos(n);
  co.startSegment();
  co.compress();
  co.endSegment();
}

}  // end namespace LQ

namespace zpaq {
  td::BufferSlice compress(td::Slice data) {
    I size = td::narrow_cast<int>(data.size());

    LQ::SB in, out;
    in.write(data.data(), size);

    LQ::compressBlock(&in, &out);

    return td::BufferSlice(out.c_str(), out.size());
  }

  td::BufferSlice decompress(td::Slice data) {
      LQ::SB in, out;
      in.write(data.data(), data.size());
      LQ::decompress(&in, &out);
      return td::BufferSlice(out.c_str(), out.size());
  }
}

#undef Y
#undef O
#undef Q
#undef F
