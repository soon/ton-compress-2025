
#include<iostream>
#include<sstream>
#include"block/block-auto.h"
#include"block/block-parse.h"
#include"common/util.h"
#include"vm/boc.h"
#include"vm/cells/CellSlice.h"
#include<fstream>
#include<set>
#include"td/utils/lz4.h"
#include"td/utils/misc.h"
#include"td/utils/buffer.h"
#include"td/utils/misc.h"
#include<stdio.h>
#include<string>
#include<vector>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<algorithm>
#define cW blen
#define cQ true
#define cM template
#define cL auto
#define cI fetch_ref
#define cF in_msg_descr
#define cD BufferSlice
#define cy out_msg_descr
#define cx store_long
#define cj bool
#define bU virtual
#define bS HashmapAugE
#define bP data
#define bO string
#define bG CurrencyCollection
#define bE static
#define bC extra
#define bw write
#define bu public
#define bq class
#define bj struct
#define bi store_ref
#define bd size_t
#define bc size
#define ba CellSlice
#define W CellBuilder
#define V while
#define S char
#define K void
#define B unsigned
#define A const
#define u return
#define o case
#define g break
using namespace vm;using namespace std;ba to_cs(Ref<Cell> cell){u load_cell_slice(std::move(cell));}bj J{ostream&out;};set<bO> enabled_optimizations{"Block","BlockExtra","bS","HashmapAug","HashmapAugNode","InMsgDescr","OutMsgDescr",};cM<bq T_TLB>bj N{ba incoming_cs;ba bT;bO name;T_TLB bf;N(bO name,T_TLB bf):name(name),bf(bf){}bU ~N(){}bU K br(J&ctx,ba&cs,int G=0){}bU K bV(J&ctx,W&cb,int G=0){bH(ctx,cb,G);}bU K bB(J&ctx,ba&cs,int G=0){br(ctx,cs,G);}bU K bH(J&ctx,W&cb,int G=0){}cj is_enabled(){u enabled_optimizations.count(name)> 0;}K bW(J&ctx,ba&cs,int G=0,cj cP=false){cL e=is_enabled();incoming_cs=cs;if(e){br(ctx,cs,G);}else{bT=bf.fetch(cs).bw();}}K cY(J&ctx,W&cb,int G=0){cL e=is_enabled();if(e){bV(ctx,cb,G);}else{cb.append_cellslice(bT);}}K cg(J&ctx,ba&cs,int G=0,cj cP=false){cL e=is_enabled();incoming_cs=cs;if(e){bB(ctx,cs,G);}else{bT=bf.fetch(cs).bw();}}K cz(J&ctx,W&cb,int G=0){cL e=is_enabled();if(e){bH(ctx,cb,G);}else{if(!bT.is_valid()){throw runtime_error(name+":optimization is disabled,but bT is empty,meaning it was never set");}cb.append_cellslice(bT);}}Ref<Cell> bt(J&ctx,int G=0){W cb;cz(ctx,cb,G);u cb.finalize();}Ref<Cell> bM(J&ctx,int G=0){W cb;cY(ctx,cb,G);u cb.finalize();}K bx(J&ctx,Ref<Cell> cell_ref,int G=0,cj cP=false){cL cs=to_cs(std::move(cell_ref));bW(ctx,cs,G,cP);}K by(J&ctx,Ref<Cell> cell_ref,int G=0,cj cP=false){cL cs=to_cs(std::move(cell_ref));cg(ctx,cs,G,cP);}};cM<bq cw>bj cZ{bU ~cZ(){}bU ba calc_aug_data(){throw runtime_error("aug bP requested but not implemented");}};cM<bq T_TLB>bj da{T_TLB add_type;da(A T_TLB&add_type):add_type(add_type){}bU ~da(){}bU ba add_values(ba&cs1,ba&cs2){W cb;(add_type.add_values(cb,cs1,cs2));u cb.as_cellslice();}};using namespace block::tlb;bj ck;bj FullInMsg:N<InMsg>,cZ<ck>{FullInMsg():N("InMsg",InMsg()){}ba calc_aug_data(){W cb;cL cs_copy=bT;(bf.get_import_fees(cb,cs_copy));u cb.as_cellslice();}};bj bD;bj FullOutMsg:N<OutMsg>,cZ<bD>{FullOutMsg():N("OutMsg",OutMsg()){}ba calc_aug_data(){W cb;cL cs_copy=bT;(bf.get_export_value(cb,cs_copy));u cb.as_cellslice();}};bj ck;cM<bq cH,bq cw>bj bm;bj bD:N<bG>,da<bG>{bD():N("bG",bG()),da(bG()){}};cM<bq cH,bq cw>bj FullHashmapAugNode:N<block::gen::HashmapAugNode>{int tag=-1;int n=-1;Ref<bm<cH,cw>> left;Ref<bm<cH,cw>> right;cH value;cw bC;FullHashmapAugNode(int m,A TLB&X,A TLB&Y):N("HashmapAugNode",block::gen::HashmapAugNode(m,X,Y)){}K br(J&ctx,ba&cs,int G=0){tag=bf.check_tag(cs);if(tag==0){bC.bW(ctx,cs,G+1);value.bW(ctx,cs,G+1,cQ);}else{int n;add_r1(n,1,bf.m_);left=Ref<bm<cH,cw>>(cQ,n,bf.X_,bf.Y_);left.bw().bx(ctx,cs.cI(),G+1);right=Ref<bm<cH,cw>>(cQ,n,bf.X_,bf.Y_);right.bw().bx(ctx,cs.cI(),G+1);bC.bW(ctx,cs,G+1);}}K bV(J&ctx,W&cb,int G=0){if(tag==0){value.cY(ctx,cb,G+1);}else{int n;(add_r1(n,1,bf.m_));cb.bi(left.bw().bM(ctx,G+1));cb.bi(right.bw().bM(ctx,G+1));}}K bB(J&ctx,ba&cs,int G=0){tag=bf.check_tag(cs);if(tag==0){value.cg(ctx,cs,G+1);cL extra_cs=value.calc_aug_data();bC.cg(ctx,extra_cs,G+1,cQ);}else{int n;add_r1(n,1,bf.m_);left=Ref<bm<cH,cw>>(cQ,n,bf.X_,bf.Y_);left.bw().by(ctx,cs.cI(),G+1);right=Ref<bm<cH,cw>>(cQ,n,bf.X_,bf.Y_);right.bw().by(ctx,cs.cI(),G+1);cL left_extra_cs=to_cs(left.bw().node.bC.bt(ctx));cL right_extra_cs=to_cs(right.bw().node.bC.bt(ctx));cL extra_cs=bC.add_values(left_extra_cs,right_extra_cs);bC.cg(ctx,extra_cs,G+1,cQ);}}K bH(J&ctx,W&cb,int G=0){if(tag==0){bC.cz(ctx,cb,G+1);value.cz(ctx,cb,G+1);}else{int n;(add_r1(n,1,bf.m_));cb.bi(left.bw().bt(ctx,G+1));cb.bi(right.bw().bt(ctx,G+1));bC.cz(ctx,cb,G+1);}}};cM<bq cH,bq cw>bj bm:N<block::gen::HashmapAug>,td::CntObject{block::gen::HashmapAug::Record bb;FullHashmapAugNode<cH,cw> node;bm(int n,A TLB&X,A TLB&Y):N("HashmapAug",block::gen::HashmapAug(n,X,Y)),node(n,X,Y){}K br(J&ctx,ba&cs,int G=0){(bf.unpack(cs,bb));node.bf.m_=bb.m;node.bW(ctx,bb.node.bw(),G+1);}K bV(J&ctx,W&cb,int G=0){int l,m;(tlb::store_from(cb,HmLabel{bf.m_},bb.label,l)&&add_r1(m,l,bf.m_));node.cY(ctx,cb,G+1);}K bB(J&ctx,ba&cs,int G=0){(
(bb.n=bf.m_)>= 0
&&block::gen::HmLabel{bf.m_}.fetch_to(cs,bb.label,bb.l)&&add_r1(bb.m,bb.l,bf.m_));node.bf.m_=bb.m;node.cg(ctx,cs,G+1);}K bH(J&ctx,W&cb,int G=0){int l,m;(tlb::store_from(cb,block::gen::HmLabel{bf.m_},bb.label,l)&&add_r1(m,l,bf.m_));node.cz(ctx,cb,G+1);}};cM<bq cH,bq cw>bj db:N<block::gen::bS>{block::gen::bS::Record_ahme_root r;int tag=-1;bm<cH,cw> root;cw bC;db(int n,A TLB&X,A TLB&Y):N("bS",block::gen::bS(n,X,Y)),root(n,X,Y){}K br(J&ctx,ba&cs,int G=0){tag=bf.check_tag(cs);if(tag==block::gen::bS::ahme_empty){(cs.fetch_ulong(1)== 0);bC.bW(ctx,cs,G+1);}else{(bf.unpack(cs,r));root.bx(ctx,r.root,G+1);bC.bW(ctx,r.bC.bw(),G+1);}}K bV(J&ctx,W&cb,int G=0){if(tag==bS::ahme_empty){cb.cx(0,1);bC.cY(ctx,cb,G+1);}else{cb.cx(1,1).bi(root.bM(ctx,G+1));bC.cY(ctx,cb,G+1);}}K bB(J&ctx,ba&cs,int G=0){tag=bf.check_tag(cs);if(tag==bS::ahme_empty){(cs.fetch_ulong(1)== 0);bC.cg(ctx,cs,G+1);}else{(cs.fetch_ulong(1)== 1&&(r.n=bf.m_)>= 0);cL root_ref=cs.cI();root.by(ctx,root_ref,G+1);bC=root.node.bC;}}K bH(J&ctx,W&cb,int G=0){if(tag==bS::ahme_empty){cb.cx(0,1);bC.cz(ctx,cb,G+1);}else{cb.cx(1,1).bi(root.bt(ctx,G+1));bC.cz(ctx,cb,G+1);}}};A bG tCC;A OutMsg tOM;bj FullOutMsgDescr:N<OutMsgDescr>{db<FullOutMsg,bD> x{256,tOM,tCC};FullOutMsgDescr():N("OutMsgDescr",OutMsgDescr()){}K br(J&ctx,ba&cs,int G=0){x.bW(ctx,cs,G+1);}K bV(J&ctx,W&cb,int G=0){x.cY(ctx,cb,G+1);}K bB(J&ctx,ba&cs,int G=0){x.cg(ctx,cs,G+1);}K bH(J&ctx,W&cb,int G=0){x.cz(ctx,cb,G+1);}};bj FullOutMsgQueue:N<OutMsgQueue>{FullOutMsgQueue():N("OutMsgQueue",OutMsgQueue()){}};A ImportFees tIF;A InMsg tIM;bj ck:N<ImportFees>,da<ImportFees>{ck():N("ImportFees",tIF),da(tIF){}};bj FullInMsgDescr:N<InMsgDescr>{db<FullInMsg,ck> x{256,tIM,tIF};FullInMsgDescr():N("InMsgDescr",InMsgDescr()){}K br(J&ctx,ba&cs,int G=0){x.bW(ctx,cs,G+1);}K bV(J&ctx,W&cb,int G=0){x.cY(ctx,cb,G+1);}K bB(J&ctx,ba&cs,int G=0){x.cg(ctx,cs,G+1);}K bH(J&ctx,W&cb,int G=0){x.cz(ctx,cb,G+1);}};A block::gen::McBlockExtra tMBE{};A block::gen::RefT tRMBE{tMBE};A block::gen::Maybe tMRMBE(tRMBE);bj FullBlockExtra:N<block::gen::BlockExtra>{block::gen::BlockExtra::Record bb;FullInMsgDescr cF;FullOutMsgDescr cy;FullBlockExtra():N("BlockExtra",block::gen::t_BlockExtra){}K br(J&ctx,ba&cs,int G=0){(bf.unpack(cs,bb));cF.bx(ctx,bb.cF,G+1);cy.bx(ctx,bb.cy,G+1);}K bV(J&ctx,W&cb,int G=0){cb.bi(cF.bM(ctx,G+1)).bi(cy.bM(ctx,G+1)).bi(bb.account_blocks).store_bits(bb.rand_seed.cbits(),256).store_bits(bb.created_by.cbits(),256);(tMRMBE.store_from(cb,bb.custom));}K bB(J&ctx,ba&cs,int G=0){cF.by(ctx,cs.cI(),G+1);cy.by(ctx,cs.cI(),G+1);(cs.fetch_ref_to(bb.account_blocks)&&cs.fetch_bits_to(bb.rand_seed.bits(),256)&&cs.fetch_bits_to(bb.created_by.bits(),256)&&tMRMBE.fetch_to(cs,bb.custom));}K bH(J&ctx,W&cb,int G=0){cb.cx(0x4a33f6fd,32).bi(cF.bt(ctx,G+1)).bi(cy.bt(ctx,G+1)).bi(bb.account_blocks).store_bits(bb.rand_seed.cbits(),256).store_bits(bb.created_by.cbits(),256);(tMRMBE.store_from(cb,bb.custom));}};bj FullBlock:N<block::gen::Block>{block::gen::Block::Record bb;FullBlockExtra bC;FullBlock():N("Block",block::gen::t_Block){}K br(J&ctx,ba&cs,int G=0){(bf.unpack(cs,bb));bC.bx(ctx,bb.bC,G+1,cQ);}K bV(J&ctx,W&cb,int G=0){cb.cx(bb.global_id,32).bi(bb.info).bi(bb.value_flow).bi(bb.state_update).bi(bC.bM(ctx,G+1));}K bB(J&ctx,ba&cs,int G=0){(
cs.fetch_int_to(32,bb.global_id)&&cs.fetch_ref_to(bb.info)&&cs.fetch_ref_to(bb.value_flow)&&cs.fetch_ref_to(bb.state_update));bC.by(ctx,cs.cI(),G+1,cQ);}K bH(J&ctx,W&cb,int G=0){cb.cx(0x11ef55aa,32).cx(bb.global_id,32).bi(bb.info).bi(bb.value_flow).bi(bb.state_update).bi(bC.bt(ctx,G+1));}};bq NullStream:bu ostream{bq NullBuffer:bu streambuf{bu:int overflow(int c){u c;}}m_nb;bu:NullStream():ostream(&m_nb){}};
#define Y else
#define O if
#define Q for
using I=int;using namespace std;namespace LQ{typedef uint8_t U8;typedef uint16_t U16;typedef uint32_t U32;typedef uint64_t U64;K E(A S* msg=0){throw runtime_error(msg);}bj cc{bU I get()= 0;bU I read(S* buf,I n);bU ~cc(){}};bj bn{bU K put(I c)= 0;bU K bw(A S* buf,I n);bU ~bn(){}};I toU16(A S* p);cM<bq T>
bq cv{T *bP;bd n;I offset;K operator=(A cv&);cv(A cv&);bu:cv(bd sz=0,I ex=0):bP(0),n(0),offset(0){bg(sz,ex);}
K bg(bd sz,I ex=0);~cv(){bg(0);}
bd bc()A{u n;}
I isize()A{u I(n);}
T&operator[](bd i){u bP[i];}
T&operator()(bd i){u bP[i&(n-1)];}};cM<bq T>
K cv<T>::bg(bd sz,I ex){V(ex>0){O(sz>sz*2)E(0);sz*=2,--ex;}O(n>0){::free((S*)bP-offset);}n=0;offset=0;O(sz==0)u;n=sz;A bd nb=128+n*sizeof(T);O(nb<=128||(nb-128)/sizeof(T)!=n)n=0,E(0);bP=(T*)::calloc(nb,1);O(!bP)n=0,E(0);offset=64-(((S*)bP-(S*)0)&63);bP=(T*)((S*)bP+offset);}typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE}CompType;bq cR;bq ZP{bu:ZP();~ZP();K clear();K inith();K initp();double memory();K run(U32 input);I read(cc* in2);cj bw(bn* out2,cj pp);I step(U32 input,I mode);bn* output;U32 H(I i){u h(i);}
K flush();K outc(I ch){O(ch<0||(outbuf[bufptr]=ch,++bufptr==outbuf.isize()))flush();}cv<U8> D;I cd;I bI,bk;
cv<U8> m;cv<U32> h,r;cv<S> outbuf;I bufptr;U32 a,b,c,d;I f,pc,rcode_size;U8* rcode;K cE(I hbits,I mbits);I execute();K div(U32 x){O(x)a/=x;Y a=0;}
K mod(U32 x){O(x)a%=x;Y a=0;}
K swap(U32&x){a^=x;x^=a;a^=x;}
K swap(U8&x){a^=x;x^=a;a^=x;}
K err(){E(0);}};bj dc{bd z;bd cxt;bd a,b,c;cv<U32> cm;cv<U8> ht;cv<U16> a16;K cE(){z=cxt=a=b=c=0;cm.bg(0);ht.bg(0);a16.bg(0);}dc(){cE();}};bq StateTable{bu:U8 ns[1024];I bv(I s,I y){u ns[s*4+y];}I cminit(I s){u((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1);}StateTable();};bq P{bu:P(ZP&);~P();K cE();I predict();K update(I y);I stat(I);cj dd(){u z.D[6]!=0;}
I c8;I hmap4;I p[256];U32 h[256];ZP&z;dc comp[256];cj initTables;I predict0();K update0(I y);I dt2k[256];I dt[1024];U16 squasht[4096];short stretcht[32768];StateTable st;U8* pcode;I pcode_size;K train(dc&cr,I y){U32&pn=cr.cm(cr.cxt);U32 count=pn&0x3ff;I E=y*32767-(cr.cm(cr.cxt)>>17);pn+=(E*dt[count]&-1024)+(count<cr.z);}I squash(I x){u squasht[x+2048];}I stretch(I x){u stretcht[x];}I clamp2k(I x){O(x<-2048)u -2048;Y O(x>2047)u 2047;Y u x;}I clamp512k(I x){O(x<-(1<<19))u -(1<<19);Y O(x>=(1<<19))u(1<<19)-1;Y u x;}bd find(cv<U8>&ht,I sizebits,U32 cxt){I chk=cxt>>sizebits&255;bd h0=(cxt*16)&(ht.bc()-16);O(ht[h0]==chk)u h0;bd h1=h0^16;O(ht[h1]==chk)u h1;bd h2=h0^32;O(ht[h2]==chk)u h2;O(ht[h0+1]<=ht[h1+1]&&ht[h0+1]<=ht[h2+1])
u memset(&ht[h0],0,16),ht[h0]=chk,h0;Y O(ht[h1+1]<ht[h2+1])
u memset(&ht[h1],0,16),ht[h1]=chk,h1;Y
u memset(&ht[h2],0,16),ht[h2]=chk,h2;}I assemble_p();};bq cR:bu cc{bu:cc* in;cR(ZP&z);I decompress();I skip();K cE();I stat(I x){u pr.stat(x);}
I get(){O(bK==bo){bK=0;bo=in ? in->read(&buf[0],cl):0;}u bK<bo ? U8(buf[bK++]):-1;}I buffered(){u bo-bK;}
U32 low,cG;U32 bQ;U32 bK,bo;P pr;enum{cl=1<<16};cv<S> buf;I decode(I p);};bq cS{I be;I bZ;I ph,pm;bu:ZP z;cS():be(0),bZ(0),ph(0),pm(0){}K cE(I h,I m);I bw(I c);I getState()A{u be;}
K setOutput(bn* out){z.output=out;}};bq D{bu:D():z(),dec(z),pp(),be(BLOCK),cn(FIRSTSEG){}K setInput(cc* in){dec.in=in;}
cj findBlock(double* memptr=0);cj findFilename(bn* = 0);K readComment(bn* = 0);K setOutput(bn* out){pp.setOutput(out);}
cj decompress(I n=-1);K readSegmentEnd(S* bX=0);I stat(I x){u dec.stat(x);}
I buffered(){u dec.buffered();}
ZP z;cR dec;cS pp;enum{BLOCK,FILENAME,COMMENT,DATA,SEGEND}be;enum{FIRSTSEG,SEG,SKIP}cn;};K decompress(cc* in,bn* out);bq Encoder{bu:Encoder(ZP&z,I bc=0):
out(0),low(1),cG(0xFFFFFFFF),pr(z){}K cE();K compress(I c);I stat(I x){u pr.stat(x);}
bn* out;
U32 low,cG;P pr;cv<S> buf;K encode(I y,I p);};bq ce{bu:ce(A S* in,I* bh,ZP&hz,ZP&pz,bn* out2);
A S* in;I* bh;ZP&hz;ZP&pz;bn* out2;I line;I be;typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,JT=39,JF=47,JMP=63,LJ=255,POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON}CompType;K SE(A S* msg,A S* expected=0);K bv();cj matchToken(A S* tok);I rtoken(I low,I cG);I rtoken(A S* list[]);K rtoken(A S* s);I compile_comp(ZP&z);bq Stack{LQ::cv<U16> s;bd top;bu:Stack(I n):s(n),top(0){}K push(A U16&x){O(top>=s.bc())E(0);s[top++]=x;}U16 pop(){O(top<=0)E(0);u s[--top];}};Stack if_stack,do_stack;};bq C{bu:C():enc(z),in(0),be(INIT){}K setOutput(bn* out){enc.out=out;}
K startBlock(A S* config,I* bh,bn* pcomp_cmd=0);K startSegment(A S* cA=0,A S* comment=0);K setInput(cc* i){in=i;}
K postProcess(A S* cJ=0,I len=0);cj compress(I n=-1);K endSegment(A S* bX=0);I stat(I x){u enc.stat(x);}
ZP z,pz;Encoder enc;cc* in;enum{INIT,BLOCK1,SEG1,BLOCK2,SEG2}be;};bq SB:bu LQ::cc,bu LQ::bn{B S* p;bd al;bd bo;bd bK;bd bF;A bd cE;K reserve(bd a){O(a<=al)u;B S* q=0;O(a>0)q=(B S*)(p ? realloc(p,a):malloc(a));O(a>0&&!q)E(0);p=q;al=a;}K lengthen(bd n){O(bo+n>bF||bo+n<bo)E(0);O(bo+n<=al)u;bd a=al;V(bo+n>=a)a=a*2+cE;reserve(a);}K operator=(A SB&);SB(A SB&);bu:B S* bP(){u p;}
SB(bd n=0):
p(0),al(0),bo(0),bK(0),bF(bd(-1)),cE(n>128?n:128){}~SB(){O(p)free(p);}
bd bc()A{u bo;}
K put(I c){lengthen(1);p[bo++]=c;}K bw(A S* buf,I n){O(n<1)u;lengthen(n);O(buf)memcpy(p+bo,buf,n);bo+=n;}I get(){u bK<bo ? p[bK++]:-1;}I read(S* buf,I n){O(bK+n>bo)n=bo-bK;O(n>0&&buf)memcpy(buf,p+bK,n);bK+=n;u n;}A S* c_str()A{u(A S*)p;}
K bg(bd i){bo=i;O(bK>bo)bK=bo;}K swap(SB&s){::swap(p,s.p);::swap(al,s.al);::swap(bo,s.bo);::swap(bK,s.bK);::swap(bF,s.bF);}};K compress(cc* in,bn* out,A S* ME,A S* cA=0,A S* comment=0,cj dosha1=cQ);K compressBlock(SB* in,bn* out,A S* ME,A S* cA=0,A S* comment=0,cj dosha1=cQ);I toU16(A S* p){u(p[0]&255)+256*(p[1]&255);}I cc::read(S* buf,I n){I i=0,c;V(i<n&&(c=get())>=0)
buf[i++]=c;u i;}K bn::bw(A S* buf,I n){Q(I i=0;i<n;++i)
put(U8(buf[i]));}K allocx(U8*&p,I&n,I newsize){p=0;n=0;}A I compsize[256]={0,2,3,2,3,4,6,6,3,5};bE A U8 sns[1024]={1,2,0,0,3,5,1,0,4,6,0,1,7,9,2,0,8,11,1,1,8,11,1,1,10,12,0,2,13,15,3,0,14,17,2,1,14,17,2,1,16,19,1,2,16,19,1,2,18,20,0,3,21,23,4,0,22,25,3,1,22,25,3,1,24,27,2,2,24,27,2,2,26,29,1,3,26,29,1,3,28,30,0,4,31,33,5,0,32,35,4,1,32,35,4,1,34,37,3,2,34,37,3,2,36,39,2,3,36,39,2,3,38,41,1,4,38,41,1,4,40,42,0,5,43,33,6,0,44,47,5,1,44,47,5,1,46,49,4,2,46,49,4,2,48,51,3,3,48,51,3,3,50,53,2,4,50,53,2,4,52,55,1,5,52,55,1,5,40,56,0,6,57,45,7,0,58,47,6,1,58,47,6,1,60,63,5,2,60,63,5,2,62,65,4,3,62,65,4,3,64,67,3,4,64,67,3,4,66,69,2,5,66,69,2,5,52,71,1,6,52,71,1,6,54,72,0,7,73,59,8,0,74,61,7,1,74,61,7,1,76,63,6,2,76,63,6,2,78,81,5,3,78,81,5,3,80,83,4,4,80,83,4,4,82,85,3,5,82,85,3,5,66,87,2,6,66,87,2,6,68,89,1,7,68,89,1,7,70,90,0,8,91,59,9,0,92,77,8,1,92,77,8,1,94,79,7,2,94,79,7,2,96,81,6,3,96,81,6,3,98,101,5,4,98,101,5,4,100,103,4,5,100,103,4,5,82,105,3,6,82,105,3,6,84,107,2,7,84,107,2,7,86,109,1,8,86,109,1,8,70,110,0,9,111,59,10,0,112,77,9,1,112,77,9,1,114,97,8,2,114,97,8,2,116,99,7,3,116,99,7,3,62,101,6,4,62,101,6,4,80,83,5,5,80,83,5,5,100,67,4,6,100,67,4,6,102,119,3,7,102,119,3,7,104,121,2,8,104,121,2,8,86,123,1,9,86,123,1,9,70,124,0,10,125,59,11,0,126,77,10,1,126,77,10,1,128,97,9,2,128,97,9,2,60,63,8,3,60,63,8,3,66,69,3,8,66,69,3,8,104,131,2,9,104,131,2,9,86,133,1,10,86,133,1,10,70,134,0,11,135,59,12,0,136,77,11,1,136,77,11,1,138,97,10,2,138,97,10,2,104,141,2,10,104,141,2,10,86,143,1,11,86,143,1,11,70,144,0,12,145,59,13,0,146,77,12,1,146,77,12,1,148,97,11,2,148,97,11,2,104,151,2,11,104,151,2,11,86,153,1,12,86,153,1,12,70,154,0,13,155,59,14,0,156,77,13,1,156,77,13,1,158,97,12,2,158,97,12,2,104,161,2,12,104,161,2,12,86,163,1,13,86,163,1,13,70,164,0,14,165,59,15,0,166,77,14,1,166,77,14,1,168,97,13,2,168,97,13,2,104,171,2,13,104,171,2,13,86,173,1,14,86,173,1,14,70,174,0,15,175,59,16,0,176,77,15,1,176,77,15,1,178,97,14,2,178,97,14,2,104,181,2,14,104,181,2,14,86,183,1,15,86,183,1,15,70,184,0,16,185,59,17,0,186,77,16,1,186,77,16,1,74,97,15,2,74,97,15,2,104,89,2,15,104,89,2,15,86,187,1,16,86,187,1,16,70,188,0,17,189,59,18,0,190,77,17,1,86,191,1,17,70,192,0,18,193,59,19,0,194,77,18,1,86,195,1,18,70,196,0,19,193,59,20,0,197,77,19,1,86,198,1,19,70,196,0,20,199,77,20,1,86,200,1,20,201,77,21,1,86,202,1,21,203,77,22,1,86,204,1,22,205,77,23,1,86,206,1,23,207,77,24,1,86,208,1,24,209,77,25,1,86,210,1,25,211,77,26,1,86,212,1,26,213,77,27,1,86,214,1,27,215,77,28,1,86,216,1,28,217,77,29,1,86,218,1,29,219,77,30,1,86,220,1,30,221,77,31,1,86,222,1,31,223,77,32,1,86,224,1,32,225,77,33,1,86,226,1,33,227,77,34,1,86,228,1,34,229,77,35,1,86,230,1,35,231,77,36,1,86,232,1,36,233,77,37,1,86,234,1,37,235,77,38,1,86,236,1,38,237,77,39,1,86,238,1,39,239,77,40,1,86,240,1,40,241,77,41,1,86,242,1,41,243,77,42,1,86,244,1,42,245,77,43,1,86,246,1,43,247,77,44,1,86,248,1,44,249,77,45,1,86,250,1,45,251,77,46,1,86,252,1,46,253,77,47,1,86,254,1,47,253,77,48,1,86,254,1,48,0,0,0,0
};StateTable::StateTable(){memcpy(ns,sns,sizeof(ns));}cj ZP::bw(bn* out2,cj pp){O(D.bc()<=6)u false;O(!pp){Q(I i=0;i<cd;++i)
out2->put(D[i]);}Y{out2->put((bk-bI)&255);out2->put((bk-bI)>>8);}Q(I i=bI;i<bk;++i)
out2->put(D[i]);u cQ;}I ZP::read(cc* in2){I bZ=in2->get();bZ+=in2->get()*256;D.bg(bZ+300);cd=bI=bk=0;D[cd++]=bZ&255;D[cd++]=bZ>>8;V(cd<7)D[cd++]=in2->get();I n=D[cd-1];Q(I i=0;i<n;++i){I bf=in2->get();O(bf<0||bf>255)E(0);D[cd++]=bf;I bc=compsize[bf];O(bc<1)E(0);O(cd+bc>bZ)E(0);Q(I j=1;j<bc;++j)
D[cd++]=in2->get();}O((D[cd++]=in2->get())!=0)E(0);bI=bk=cd+128;O(bk>bZ+129)E(0);V(bk<bZ+129){I op=in2->get();O(op==-1)E(0);D[bk++]=op;}O((D[bk++]=in2->get())!=0)E(0);allocx(rcode,rcode_size,0);u cd+bk-bI;}K ZP::clear(){cd=bI=bk=0;a=b=c=d=f=pc=0;D.bg(0);h.bg(0);m.bg(0);r.bg(0);allocx(rcode,rcode_size,0);}ZP::ZP(){output=0;rcode=0;rcode_size=0;clear();outbuf.bg(1<<14);bufptr=0;}ZP::~ZP(){allocx(rcode,rcode_size,0);}K ZP::inith(){cE(D[2],D[3]);}K ZP::initp(){cE(D[4],D[5]);}K ZP::flush(){O(output)output->bw(&outbuf[0],bufptr);bufptr=0;}bE double pow2(I x){double r=1;Q(;x>0;x--)r+=r;u r;}double ZP::memory(){double mem=pow2(D[2]+2)+pow2(D[3])
+pow2(D[4]+2)+pow2(D[5])
+D.bc();I cp=7;Q(I i=0;i<D[6];++i){double bc=pow2(D[cp+1]);switch(D[cp]){o CM:mem+=4*bc;g;o ICM:mem+=64*bc+1024;g;o MATCH:mem+=4*bc+pow2(D[cp+2]);g;o MIX2:mem+=2*bc;g;o MIX:mem+=4*bc*D[cp+3];g;o ISSE:mem+=64*bc+2048;g;o SSE:mem+=128*bc;g;}cp+=compsize[D[cp]];}u mem;}K ZP::cE(I hbits,I mbits){O(hbits>32)E(0);O(mbits>32)E(0);h.bg(1,hbits);m.bg(1,mbits);r.bg(256);a=b=c=d=pc=f=0;}I ZP::execute(){switch(D[pc++]){o 0:err();g;o 1:++a;g;o 2:--a;g;o 3:a=~a;g;o 4:a=0;g;o 7:a=r[D[pc++]];g;o 8:swap(b);g;o 9:++b;g;o 10:--b;g;o 11:b=~b;g;o 12:b=0;g;o 15:b=r[D[pc++]];g;o 16:swap(c);g;o 17:++c;g;o 18:--c;g;o 19:c=~c;g;o 20:c=0;g;o 23:c=r[D[pc++]];g;o 24:swap(d);g;o 25:++d;g;o 26:--d;g;o 27:d=~d;g;o 28:d=0;g;o 31:d=r[D[pc++]];g;o 32:swap(m(b));g;o 33:++m(b);g;o 34:--m(b);g;o 35:m(b)= ~m(b);g;o 36:m(b)= 0;g;o 39:O(f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;o 40:swap(m(c));g;o 41:++m(c);g;o 42:--m(c);g;o 43:m(c)= ~m(c);g;o 44:m(c)= 0;g;o 47:O(!f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;o 48:swap(h(d));g;o 49:++h(d);g;o 50:--h(d);g;o 51:h(d)= ~h(d);g;o 52:h(d)= 0;g;o 55:r[D[pc++]]=a;g;o 56:u 0 ;o 57:outc(a&255);g;o 59:a=(a+m(b)+512)*773;g;o 60:h(d)= (h(d)+a+512)*773;g;o 63:pc+=((D[pc]+128)&255)-127;g;o 64:g;o 65:a=b;g;o 66:a=c;g;o 67:a=d;g;o 68:a=m(b);g;o 69:a=m(c);g;o 70:a=h(d);g;o 71:a=D[pc++];g;o 72:b=a;g;o 73:g;o 74:b=c;g;o 75:b=d;g;o 76:b=m(b);g;o 77:b=m(c);g;o 78:b=h(d);g;o 79:b=D[pc++];g;o 80:c=a;g;o 81:c=b;g;o 82:g;o 83:c=d;g;o 84:c=m(b);g;o 85:c=m(c);g;o 86:c=h(d);g;o 87:c=D[pc++];g;o 88:d=a;g;o 89:d=b;g;o 90:d=c;g;o 91:g;o 92:d=m(b);g;o 93:d=m(c);g;o 94:d=h(d);g;o 95:d=D[pc++];g;o 96:m(b)=a;g;o 97:m(b)=b;g;o 98:m(b)=c;g;o 99:m(b)=d;g;o 100:g;o 101:m(b)=m(c);g;o 102:m(b)=h(d);g;o 103:m(b)=D[pc++];g;o 104:m(c)=a;g;o 105:m(c)=b;g;o 106:m(c)=c;g;o 107:m(c)=d;g;o 108:m(c)=m(b);g;o 109:g;o 110:m(c)=h(d);g;o 111:m(c)=D[pc++];g;o 112:h(d)=a;g;o 113:h(d)=b;g;o 114:h(d)=c;g;o 115:h(d)=d;g;o 116:h(d)=m(b);g;o 117:h(d)=m(c);g;o 118:g;o 119:h(d)=D[pc++];g;o 128:a+=a;g;o 129:a+=b;g;o 130:a+=c;g;o 131:a+=d;g;o 132:a+=m(b);g;o 133:a+=m(c);g;o 134:a+=h(d);g;o 135:a+=D[pc++];g;o 136:a-=a;g;o 137:a-=b;g;o 138:a-=c;g;o 139:a-=d;g;o 140:a-=m(b);g;o 141:a-=m(c);g;o 142:a-=h(d);g;o 143:a-=D[pc++];g;o 144:a*=a;g;o 145:a*=b;g;o 146:a*=c;g;o 147:a*=d;g;o 148:a*=m(b);g;o 149:a*=m(c);g;o 150:a*=h(d);g;o 151:a*=D[pc++];g;o 152:div(a);g;o 153:div(b);g;o 154:div(c);g;o 155:div(d);g;o 156:div(m(b));g;o 157:div(m(c));g;o 158:div(h(d));g;o 159:div(D[pc++]);g;o 160:mod(a);g;o 161:mod(b);g;o 162:mod(c);g;o 163:mod(d);g;o 164:mod(m(b));g;o 165:mod(m(c));g;o 166:mod(h(d));g;o 167:mod(D[pc++]);g;o 168:a&=a;g;o 169:a&=b;g;o 170:a&=c;g;o 171:a&=d;g;o 172:a&=m(b);g;o 173:a&=m(c);g;o 174:a&=h(d);g;o 175:a&=D[pc++];g;o 176:a&=~a;g;o 177:a&=~b;g;o 178:a&=~c;g;o 179:a&=~d;g;o 180:a&=~m(b);g;o 181:a&=~m(c);g;o 182:a&=~h(d);g;o 183:a&=~D[pc++];g;o 184:a|=a;g;o 185:a|=b;g;o 186:a|=c;g;o 187:a|=d;g;o 188:a|=m(b);g;o 189:a|=m(c);g;o 190:a|=h(d);g;o 191:a|=D[pc++];g;o 192:a^=a;g;o 193:a^=b;g;o 194:a^=c;g;o 195:a^=d;g;o 196:a^=m(b);g;o 197:a^=m(c);g;o 198:a^=h(d);g;o 199:a^=D[pc++];g;o 200:a<<=(a&31);g;o 201:a<<=(b&31);g;o 202:a<<=(c&31);g;o 203:a<<=(d&31);g;o 204:a<<=(m(b)&31);g;o 205:a<<=(m(c)&31);g;o 206:a<<=(h(d)&31);g;o 207:a<<=(D[pc++]&31);g;o 208:a>>=(a&31);g;o 209:a>>=(b&31);g;o 210:a>>=(c&31);g;o 211:a>>=(d&31);g;o 212:a>>=(m(b)&31);g;o 213:a>>=(m(c)&31);g;o 214:a>>=(h(d)&31);g;o 215:a>>=(D[pc++]&31);g;o 216:f=1;g;o 217:f=(a==b);g;o 218:f=(a==c);g;o 219:f=(a==d);g;o 220:f=(a==U32(m(b)));g;o 221:f=(a==U32(m(c)));g;o 222:f=(a==h(d));g;o 223:f=(a==U32(D[pc++]));g;o 224:f=0;g;o 225:f=(a<b);g;o 226:f=(a<c);g;o 227:f=(a<d);g;o 228:f=(a<U32(m(b)));g;o 229:f=(a<U32(m(c)));g;o 230:f=(a<h(d));g;o 231:f=(a<U32(D[pc++]));g;o 232:f=0;g;o 233:f=(a>b);g;o 234:f=(a>c);g;o 235:f=(a>d);g;o 236:f=(a>U32(m(b)));g;o 237:f=(a>U32(m(c)));g;o 238:f=(a>h(d));g;o 239:f=(a>U32(D[pc++]));g;o 255:O((pc=bI+D[pc]+256*D[pc+1])>=bk)err();g;default:err();}u 1;}bE A U8 stdt[712]={64,128,128,128,128,128,127,128,127,128,127,127,127,127,126,126,126,126,126,125,125,124,125,124,123,123,123,123,122,122,121,121,120,120,119,119,118,118,118,116,117,115,116,114,114,113,113,112,112,111,110,110,109,108,108,107,106,106,105,104,104,102,103,101,101,100,99,98,98,97,96,96,94,94,94,92,92,91,90,89,89,88,87,86,86,84,84,84,82,82,81,80,79,79,78,77,76,76,75,74,73,73,72,71,70,70,69,68,67,67,66,65,65,64,63,62,62,61,61,59,59,59,57,58,56,56,55,54,54,53,52,52,51,51,50,49,49,48,48,47,47,45,46,44,45,43,43,43,42,41,41,40,40,40,39,38,38,37,37,36,36,36,35,34,34,34,33,32,33,32,31,31,30,31,29,30,28,29,28,28,27,27,27,26,26,25,26,24,25,24,24,23,23,23,23,22,22,21,22,21,20,21,20,19,20,19,19,19,18,18,18,18,17,17,17,17,16,16,16,16,15,15,15,15,15,14,14,14,14,13,14,13,13,13,12,13,12,12,12,11,12,11,11,11,11,11,10,11,10,10,10,10,9,10,9,9,9,9,9,8,9,8,9,8,8,8,7,8,8,7,7,8,7,7,7,6,7,7,6,6,7,6,6,6,6,6,6,5,6,5,6,5,5,5,5,5,5,5,5,5,4,5,4,5,4,4,5,4,4,4,4,4,4,3,4,4,3,4,4,3,3,4,3,3,3,4,3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,2,2,3,2,2,3,2,2,2,2,3,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,1,2,1,2,2,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0
};P::P(ZP&zr):
c8(1),hmap4(1),z(zr){pcode=0;pcode_size=0;initTables=false;}P::~P(){allocx(pcode,pcode_size,0);}K P::cE(){allocx(pcode,pcode_size,0);z.inith();O(!initTables&&dd()){initTables=cQ;dt2k[0]=0;Q(I i=1;i<256;++i){dt2k[i]=2048/i;}Q(I i=0;i<1024;++i){dt[i]=(1<<17)/(i*2+3)*2;}memset(squasht,0,(1376+7)*2);Q(I i=1376+7;i<1376+1344;++i){squasht[i]=static_cast<U16>(32768.0 / (1+std::exp((i - 2048)* (-1.0 / 64))));}Q(I i=2720;i<4096;++i)squasht[i]=32767;I k=16384;Q(I i=0;i<712;++i)
Q(I j=stdt[i];j>0;--j)
stretcht[k++]=i;Q(I i=0;i<16384;++i)
stretcht[i]=-stretcht[32767-i];}Q(I i=0;i<256;++i)h[i]=p[i]=0;Q(I i=0;i<256;++i)
comp[i].cE();I n=z.D[6];A U8* cp=&z.D[7];Q(I i=0;i<n;++i){dc&cr=comp[i];switch(cp[0]){o CONS:
p[i]=(cp[1]-128)*4;g;o CM:
O(cp[1]>32)E(0);cr.cm.bg(1,cp[1]);cr.z=cp[2]*4;Q(bd j=0;j<cr.cm.bc();++j)
cr.cm[j]=0x80000000;g;o ICM:
O(cp[1]>26)E(0);cr.z=1023;cr.cm.bg(256);cr.ht.bg(64,cp[1]);Q(bd j=0;j<cr.cm.bc();++j)
cr.cm[j]=st.cminit(j);g;o MATCH:
O(cp[1]>32||cp[2]>32)E(0);cr.cm.bg(1,cp[1]);cr.ht.bg(1,cp[2]);cr.ht(0)=1;g;o AVG:
O(cp[1]>=i)E(0);O(cp[2]>=i)E(0);g;o MIX2:
O(cp[1]>32)E(0);O(cp[3]>=i)E(0);O(cp[2]>=i)E(0);cr.c=(bd(1)<<cp[1]);cr.a16.bg(1,cp[1]);Q(bd j=0;j<cr.a16.bc();++j)
cr.a16[j]=32768;g;o MIX:{O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]<1||cp[3]>i-cp[2])E(0);I m=cp[3];cr.c=(bd(1)<<cp[1]);cr.cm.bg(m,cp[1]);Q(bd j=0;j<cr.cm.bc();++j)
cr.cm[j]=65536/m;g;}o ISSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);cr.ht.bg(64,cp[1]);cr.cm.bg(512);Q(I j=0;j<256;++j){cr.cm[j*2]=1<<15;cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);}g;o SSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]>cp[4]*4)E(0);cr.cm.bg(32,cp[1]);cr.z=cp[4]*4;Q(bd j=0;j<cr.cm.bc();++j)
cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];g;default:E(0);}cp+=compsize[*cp];}}I P::predict0(){I n=z.D[6];A U8* cp=&z.D[7];Q(I i=0;i<n;++i){dc&cr=comp[i];switch(cp[0]){o CONS:
g;o CM:
cr.cxt=h[i]^hmap4;p[i]=stretch(cr.cm(cr.cxt)>>17);g;o ICM:
O(c8==1||(c8&0xf0)==16)cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(hmap4&15)];p[i]=stretch(cr.cm(cr.cxt)>>8);g;o MATCH:
O(cr.a==0)p[i]=0;Y{cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1;p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);}g;o AVG:
p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;g;o MIX2:{cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));I w=cr.a16[cr.cxt];p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;}g;o MIX:{I m=cp[3];cr.cxt=h[i]+(c8&cp[5]);cr.cxt=(cr.cxt&(cr.c-1))*m;I* wt=(I*)&cr.cm[cr.cxt];p[i]=0;Q(I j=0;j<m;++j)
p[i]+=(wt[j]>>8)*p[cp[2]+j];p[i]=clamp2k(p[i]>>8);}g;o ISSE:{O(c8==1||(c8&0xf0)==16)
cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(hmap4&15)];I *wt=(I*)&cr.cm[cr.cxt*2];p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);}g;o SSE:{cr.cxt=(h[i]+c8)*32;I pq=p[cp[2]]+992;O(pq<0)pq=0;O(pq>1983)pq=1983;I wt=pq&63;pq>>=6;cr.cxt+=pq;p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)+(cr.cm(cr.cxt+1)>>10)*wt)>>13);cr.cxt+=wt>>5;}g;default:
E(0);}cp+=compsize[cp[0]];}u squash(p[n-1]);}K P::update0(I y){A U8* cp=&z.D[7];I n=z.D[6];Q(I i=0;i<n;++i){dc&cr=comp[i];switch(cp[0]){o CONS:
g;o CM:
train(cr,y);g;o ICM:{cr.ht[cr.c+(hmap4&15)]=st.bv(cr.ht[cr.c+(hmap4&15)],y);U32&pn=cr.cm(cr.cxt);pn+=I(y*32767-(pn>>8))>>2;}g;o MATCH:{O(I(cr.c)!=y)cr.a=0;cr.ht(cr.z)+=cr.ht(cr.z)+y;O(++cr.cxt==8){cr.cxt=0;++cr.z;cr.z&=(1<<cp[2])-1;O(cr.a==0){cr.b=cr.z-cr.cm(h[i]);O(cr.b&(cr.ht.bc()-1))
V(cr.a<255
&&cr.ht(cr.z-cr.a-1)==cr.ht(cr.z-cr.a-cr.b-1))
++cr.a;}Y cr.a+=cr.a<255;cr.cm(h[i])=cr.z;}}g;o AVG:
g;o MIX2:{I err=(y*32767-squash(p[i]))*cp[4]>>5;I w=cr.a16[cr.cxt];w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;O(w<0)w=0;O(w>65535)w=65535;cr.a16[cr.cxt]=w;}g;o MIX:{I m=cp[3];I err=(y*32767-squash(p[i]))*cp[4]>>4;I* wt=(I*)&cr.cm[cr.cxt];Q(I j=0;j<m;++j)
wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));}g;o ISSE:{I err=y*32767-squash(p[i]);I *wt=(I*)&cr.cm[cr.cxt*2];wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));wt[1]=clamp512k(wt[1]+((err+16)>>5));cr.ht[cr.c+(hmap4&15)]=st.bv(cr.cxt,y);}g;o SSE:
train(cr,y);g;default:
assert(0);}cp+=compsize[cp[0]];}c8+=c8+y;O(c8>=256){z.run(c8-256);hmap4=1;c8=1;Q(I i=0;i<n;++i)h[i]=z.H(i);}Y O(c8>=16&&c8<32)
hmap4=(hmap4&0xf)<<5|y<<4|1;Y
hmap4=(hmap4&0x1f0)|(((hmap4&0xf)*2+y)&0xf);}cR::cR(ZP&z):
in(0),low(1),cG(0xFFFFFFFF),bQ(0),bK(0),bo(0),pr(z),buf(cl){}K cR::cE(){pr.cE();O(pr.dd())low=1,cG=0xFFFFFFFF,bQ=0;Y low=cG=bQ=0;}I cR::decode(I p){O(bQ<low||bQ>cG)E(0);U32 mid=low+U32(((cG-low)*U64(U32(p)))>>16);I y;O(bQ<=mid)y=1,cG=mid;Y y=0,low=mid+1;V((cG^low)<0x1000000){cG=cG<<8|255;low=low<<8;low+=(low==0);I c=get();O(c<0)E(0);bQ=bQ<<8|c;}u y;}I cR::decompress(){O(pr.dd()){O(bQ==0){Q(I i=0;i<4;++i)
bQ=bQ<<8|get();}O(decode(0)){O(bQ!=0)E(0);u -1;}Y{I c=1;V(c<256){I p=pr.predict()*2+1;c+=c+decode(p);pr.update(c&1);}u c-256;}}Y{O(bQ==0){Q(I i=0;i<4;++i)bQ=bQ<<8|get();O(bQ==0)u -1;}--bQ;u get();}}I cR::skip(){I c=-1;O(pr.dd()){V(bQ==0)
bQ=get();V(bQ&&(c=get())>=0)
bQ=bQ<<8|c;V((c=get())==0);u c;}Y{O(bQ==0)
Q(I i=0;i<4&&(c=get())>=0;++i)bQ=bQ<<8|c;V(bQ>0){V(bQ>0){--bQ;O(get()<0)u E("skipped to EOF"),-1;}Q(I i=0;i<4&&(c=get())>=0;++i)bQ=bQ<<8|c;}O(c>=0)c=get();u c;}}K cS::cE(I h,I m){be=bZ=0;ph=h;pm=m;z.clear();}I cS::bw(I c){switch (be){o 0:
O(c<0)E(0);be=c+1;O(be>2)E(0);O(be==1)z.clear();g;o 1:
z.outc(c);g;o 2:
O(c<0)E(0);bZ=c;be=3;g;o 3:
O(c<0)E(0);bZ+=c*256;O(bZ<1)E(0);z.D.bg(bZ+300);z.cd=8;z.bI=z.bk=z.cd+128;z.D[4]=ph;z.D[5]=pm;be=4;g;o 4:
O(c<0)E(0);z.D[z.bk++]=c;O(z.bk-z.bI==bZ){bZ=z.cd-2+z.bk-z.bI;z.D[0]=bZ&255;z.D[1]=bZ>>8;z.initp();be=5;}g;o 5:
z.run(c);O(c<0)z.flush();g;}u be;}cj D::findBlock(double* memptr){I c=dec.get();O(c!=1&&c!=2)E(0);z.read(&dec);O(c==1&&z.D.isize()>6&&z.D[6]==0)
E(0);O(memptr)*memptr=z.memory();be=FILENAME;cn=FIRSTSEG;u cQ;}cj D::findFilename(bn* cA){be=COMMENT;u cQ;}K D::readComment(bn* comment){be=DATA;}cj D::decompress(I n){O(cn==SKIP)E(0);O(cn==FIRSTSEG){dec.cE();pp.cE(z.D[4],z.D[5]);cn=SEG;}V((pp.getState()&3)!=1)
pp.bw(dec.decompress());V(n){I c=dec.decompress();pp.bw(c);O(c==-1){be=SEGEND;u false;}O(n>0)--n;}u cQ;}K D::readSegmentEnd(S* bX){I c=0;O(be==DATA){c=dec.skip();cn=SKIP;}Y O(be==SEGEND)
c=dec.get();be=FILENAME;O(c==254){O(bX)bX[0]=0;}Y O(c==253){O(bX)bX[0]=1;Q(I i=1;i<=20;++i){c=dec.get();O(bX)bX[i]=c;}}Y
E(0);}K decompress(cc* in,bn* out){D d;d.setInput(in);d.setOutput(out);if(d.findBlock()){d.decompress();}}K Encoder::cE(){low=1;cG=0xFFFFFFFF;pr.cE();O(!pr.dd())low=0,buf.bg(1<<16);}K Encoder::encode(I y,I p){U32 mid=low+U32(((cG-low)*U64(U32(p)))>>16);O(y)cG=mid;Y low=mid+1;V((cG^low)<0x1000000){out->put(cG>>24);cG=cG<<8|255;low=low<<8;low+=(low==0);}}K Encoder::compress(I c){O(pr.dd()){O(c==-1)
encode(1,0);Y{encode(0,0);Q(I i=7;i>=0;--i){I p=pr.predict()*2+1;I y=c>>i&1;encode(y,p);pr.update(y);}}}Y{E(0);}}A S* compname[256]={"","A","cm","icm","match","avg","mix2","mix","isse","sse",0};A S* opcodelist[272]={"E","a++","a--","a!","a=0","","","a=r","b<>a","b++","b--","b!","b=0","","","b=r","c<>a","c++","c--","c!","c=0","","","c=r","d<>a","d++","d--","d!","d=0","","","d=r","*b<>a","*b++","*b--","*b!","*b=0","","","jt","*c<>a","*c++","*c--","*c!","*c=0","","","jf","*d<>a","*d++","*d--","*d!","*d=0","","","r=a","halt","out","","hash","hashd","","","jmp","a=a","a=b","a=c","a=d","a=*b","a=*c","a=*d","a=","b=a","b=b","b=c","b=d","b=*b","b=*c","b=*d","b=","c=a","c=b","c=c","c=d","c=*b","c=*c","c=*d","c=","d=a","d=b","d=c","d=d","d=*b","d=*c","d=*d","d=","*b=a","*b=b","*b=c","*b=d","*b=*b","*b=*c","*b=*d","*b=","*c=a","*c=b","*c=c","*c=d","*c=*b","*c=*c","*c=*d","*c=","*d=a","*d=b","*d=c","*d=d","*d=*b","*d=*c","*d=*d","*d=","","","","","","","","","a+=a","a+=b","a+=c","a+=d","a+=*b","a+=*c","a+=*d","a+=","a-=a","a-=b","a-=c","a-=d","a-=*b","a-=*c","a-=*d","a-=","a*=a","a*=b","a*=c","a*=d","a*=*b","a*=*c","a*=*d","a*=","a/=a","a/=b","a/=c","a/=d","a/=*b","a/=*c","a/=*d","a/=","a%=a","a%=b","a%=c","a%=d","a%=*b","a%=*c","a%=*d","a%=","a&=a","a&=b","a&=c","a&=d","a&=*b","a&=*c","a&=*d","a&=","a&~a","a&~b","a&~c","a&~d","a&~*b","a&~*c","a&~*d","a&~","a|=a","a|=b","a|=c","a|=d","a|=*b","a|=*c","a|=*d","a|=","a^=a","a^=b","a^=c","a^=d","a^=*b","a^=*c","a^=*d","a^=","a<<=a","a<<=b","a<<=c","a<<=d","a<<=*b","a<<=*c","a<<=*d","a<<=","a>>=a","a>>=b","a>>=c","a>>=d","a>>=*b","a>>=*c","a>>=*d","a>>=","a==a","a==b","a==c","a==d","a==*b","a==*c","a==*d","a==","a<a","a<b","a<c","a<d","a<*b","a<*c","a<*d","a<","a>a","a>b","a>c","a>d","a>*b","a>*c","a>*d","a>","","","","","","","","","","","","","","","","lj","post","cJ","end","O","ifnot","Y","endif","do","V","until","Qever","ifl","ifnotl","Yl",";",0};K ce::bv(){Q(;*in;++in){O(*in=='\n')++line;O(*in=='(')be+=1+(be<0);Y O(be>0&&*in==')')--be;Y O(be<0&&*in<=' ')be=0;Y O(be==0&&*in>' '){be=-1;g;}}O(!*in)E(0);}I tolower(I c){u(c>='A'&&c<='Z')? c+'a'-'A':c;}
cj ce::matchToken(A S* word){A S* a=in;Q(;(*a>' '&&*a!='('&&*word);++a,++word)
O(tolower(*a)!=tolower(*word))u false;u !*word&&(*a<=' '||*a=='(');}K ce::SE(A S* msg,A S* expected){E(0);}I ce::rtoken(A S* list[]){bv();Q(I i=0;list[i];++i)
O(matchToken(list[i]))
u i;SE(0);u -1;}K ce::rtoken(A S* s){bv();O(!matchToken(s))SE(0,s);}I ce::rtoken(I low,I cG){bv();I r=0;O(in[0]=='$'&&in[1]>='1'&&in[1]<='9'){O(in[2]=='+')r=atoi(in+3);O(bh)r+=bh[in[1]-'1'];}Y O(in[0]=='-'||(in[0]>='0'&&in[0]<='9'))r=atoi(in);Y SE(0);O(r<low)SE(0);O(r>cG)SE(0);u r;}I ce::compile_comp(ZP&z){I op=0;A I comp_begin=z.bk;V(cQ){op=rtoken(opcodelist);O(op==POST||op==PCOMP||op==END)g;I bN=-1;I operand2=-1;O(op==IF){op=JF;bN=0;if_stack.push(z.bk+1);}Y O(op==IFNOT){op=JT;bN=0;if_stack.push(z.bk+1);}Y O(op==IFL||op==IFNOTL){O(op==IFL)z.D[z.bk++]=(JT);O(op==IFNOTL)z.D[z.bk++]=(JF);z.D[z.bk++]=(3);op=LJ;bN=operand2=0;if_stack.push(z.bk+1);}Y O(op==ELSE||op==ELSEL){O(op==ELSE)op=JMP,bN=0;O(op==ELSEL)op=LJ,bN=operand2=0;I a=if_stack.pop();O(z.D[a-1]!=LJ){I j=z.bk-a+1+(op==LJ);O(j>127)SE("O too big,try IFL,IFNOTL");z.D[a]=j;}Y{I j=z.bk-comp_begin+2+(op==LJ);z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}if_stack.push(z.bk+1);}Y O(op==ENDIF){I a=if_stack.pop();I j=z.bk-a-1;O(z.D[a-1]!=LJ){O(j>127)SE("O too big,try IFL,IFNOTL,ELSEL\n");z.D[a]=j;}Y{j=z.bk-comp_begin;z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}}Y O(op==DO){do_stack.push(z.bk);}Y O(op==WHILE||op==UNTIL||op==QEVER){I a=do_stack.pop();I j=a-z.bk-2;O(j>=-127){O(op==WHILE)op=JT;O(op==UNTIL)op=JF;O(op==QEVER)op=JMP;bN=j&255;}Y{j=a-comp_begin;O(op==WHILE){z.D[z.bk++]=(JF);z.D[z.bk++]=(3);}O(op==UNTIL){z.D[z.bk++]=(JT);z.D[z.bk++]=(3);}op=LJ;bN=j&255;operand2=j>>8;}}Y O((op&7)==7){O(op==LJ){bN=rtoken(0,65535);operand2=bN>>8;bN&=255;}Y O(op==JT||op==JF||op==JMP){bN=rtoken(-128,127);bN&=255;}Y
bN=rtoken(0,255);}O(op>=0&&op<=255)
z.D[z.bk++]=(op);O(bN>=0)
z.D[z.bk++]=(bN);O(operand2>=0)
z.D[z.bk++]=(operand2);O(z.bk>=z.D.isize()-130||z.bk-z.bI+z.cd-2>65535)
SE("program too big");}z.D[z.bk++]=(0);u op;}ce::ce(A S* in_,I* args_,ZP&hz_,ZP&pz_,bn* out2_):in(in_),bh(args_),hz(hz_),pz(pz_),out2(out2_),if_stack(1000),do_stack(1000){line=1;be=0;hz.clear();pz.clear();hz.D.bg(68000);rtoken("comp");hz.D[2]=rtoken(0,255);hz.D[3]=rtoken(0,255);hz.D[4]=rtoken(0,255);hz.D[5]=rtoken(0,255);A I n=hz.D[6]=rtoken(0,255);hz.cd=7;Q(I i=0;i<n;++i){rtoken(i,i);CompType bf=CompType(rtoken(compname));hz.D[hz.cd++]=bf;I clen=LQ::compsize[bf&255];O(clen<1||clen>10)SE("invalid component");Q(I j=1;j<clen;++j)
hz.D[hz.cd++]=rtoken(0,255);}hz.D[hz.cd++];hz.bI=hz.bk=hz.cd+128;rtoken("bJ");I op=compile_comp(hz);I bZ=hz.cd-2+hz.bk-hz.bI;hz.D[0]=bZ&255;hz.D[1]=bZ>>8;O(op==POST){rtoken(0,0);rtoken("end");}Y O(op==PCOMP){pz.D.bg(68000);pz.D[4]=hz.D[4];pz.D[5]=hz.D[5];pz.cd=8;pz.bI=pz.bk=pz.cd+128;bv();V(*in&&*in!=';'){O(out2)
out2->put(*in);++in;}O(*in)++in;op=compile_comp(pz);I len=pz.cd-2+pz.bk-pz.bI;pz.D[0]=len&255;pz.D[1]=len>>8;O(op!=END)
SE("expected END");}Y O(op!=END)
SE("expected END or POST 0 END or PCOMP cmd ;... END");}bq MemoryReader:bu cc{A S* p;bu:MemoryReader(A S* p_):p(p_){}I get(){u *p++&255;}};K C::startBlock(A S* config,I* bh,bn* pcomp_cmd){ce(config,bh,z,pz,pcomp_cmd);enc.out->put(1+(z.D[6]==0));z.bw(enc.out,false);be=BLOCK1;}K C::startSegment(A S* cA,A S* comment){O(be==BLOCK1)be=SEG1;O(be==BLOCK2)be=SEG2;}K C::postProcess(A S* cJ,I len){O(be==SEG2)u;enc.cE();O(!cJ){len=pz.bk-pz.bI;O(len>0){cJ=(A S*)&pz.D[pz.bI];}}Y O(len==0){len=toU16(cJ);cJ+=2;}O(len>0){enc.compress(1);enc.compress(len&255);enc.compress((len>>8)&255);Q(I i=0;i<len;++i)
enc.compress(cJ[i]&255);}Y
enc.compress(0);be=SEG2;}cj C::compress(I n){O(be==SEG1)
postProcess();A I cl=1<<14;S buf[cl];V(n){I nbuf=cl;O(n>=0&&n<nbuf)nbuf=n;I nr=in->read(buf,nbuf);O(nr<0||nr>cl||nr>nbuf)E(0);O(nr<=0)u false;O(n>=0)n-=nr;Q(I i=0;i<nr;++i){I ch=U8(buf[i]);enc.compress(ch);}}u cQ;}K C::endSegment(A S* bX){O(be==SEG1)
postProcess();enc.compress(-1);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(254);be=BLOCK2;}K compress(cc* in,bn* out,A S* ME,A S* cA,A S* comment,cj dosha1){I bs=4;O(ME&&ME[0]&&ME[1]>='0'&&ME[1]<='9'){bs=ME[1]-'0';O(ME[2]>='0'&&ME[2]<='9')bs=bs*10+ME[2]-'0';O(bs>11)bs=11;}bs=(0x100000<<bs)-4096;SB sb(bs);sb.bw(0,bs);I n=0;V(in&&(n=in->read((S*)sb.bP(),bs))>0){sb.bg(n);compressBlock(&sb,out,ME,cA,comment,dosha1);cA=0;comment=0;sb.bg(0);}}I P::predict(){u predict0();}K P::update(I y){update0(y);}K ZP::run(U32 input){pc=bI;a=input;V(execute());}
#define AS 256
#define BUCKET_A_SIZE AS
#define BUCKET_B_SIZE (AS*AS)
#define SSIT 8
#define SSB 1024
#define SS_MISORT_STACKSIZE 16
#define SS_SMERGE_STACKSIZE 32
#define TR_INSERTIONSORT_THRESHOLD 8
#define TR_STACKSIZE 64
#define bA(_a,_b)do{t=(_a);(_a)= (_b);(_b)= t;}V(0)
#define MIN(_a,_b)(((_a)< (_b))? (_a):(_b))
#define MAX(_a,_b)(((_a)> (_b))? (_a):(_b))
#define SP(_a,_b,_c,_d)\
do{\
assert(bl<bL);\
bz[bl].a=(_a),bz[bl].b=(_b),\
bz[bl].c=(_c),bz[bl++].d=(_d);\
}V(0)
#define S5(_a,_b,_c,_d,_e)\
do{\
assert(bl<bL);\
bz[bl].a=(_a),bz[bl].b=(_b),\
bz[bl].c=(_c),bz[bl].d=(_d),bz[bl++].e=(_e);\
}V(0)
#define STACK_POP(_a,_b,_c,_d)\
do{\
assert(0<=bl);\
O(bl==0){u;}\
(_a)= bz[--bl].a,(_b)= bz[bl].b,\
(_c)= bz[bl].c,(_d)= bz[bl].d;\
}V(0)
#define ci(_a,_b,_c,_d,_e)\
do{\
assert(0<=bl);\
O(bl==0){u;}\
(_a)= bz[--bl].a,(_b)= bz[bl].b,\
(_c)= bz[bl].c,(_d)= bz[bl].d,(_e)= bz[bl].e;\
}V(0)
#define cN(_c0)cB[(_c0)]
#define cq(_c0,_c1)(ct[((_c1)<< 8)| (_c0)])
#define BB(_c0,_c1)(ct[((_c0)<< 8)| (_c1)])
#define R(v)v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
bE A I cu[256]={-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,R(4),R(5),R(5),R(6),R(6),R(6),R(6),R(7),R(7),R(7),R(7),R(7),R(7),R(7),R(7),};
#undef R
I
cT(I n){u(n&0xff00)?
8+cu[(n >> 8)&0xff]:0+cu[(n >> 0)&0xff];}bE I dqq_table[256] ={0};I
ss_isqrt(I x){I y,e;O(x >= (SSB * SSB)){u SSB;}
e=(x&0xffff0000)?
((x&0xff000000)?
24+cu[(x >> 24)&0xff]:16+cu[(x >> 16)&0xff]):
((x&0x0000ff00)?
8+cu[(x >> 8)&0xff]:0+cu[(x >> 0)&0xff]);O(dqq_table[255] != 255){Q(I i=0;i<256;++i){dqq_table[i]=static_cast<I>(16 * sqrt(i));}}O(e >= 16){y=dqq_table[x >> ((e - 6)- (e&1))] << ((e >> 1)- 7);O(e >= 24){y=(y+1+x / y)>> 1;}
y=(y+1+x / y)>> 1;}Y O(e >= 8){y=(dqq_table[x >> ((e - 6)- (e&1))] >> (7 - (e >> 1)))+ 1;}Y{u dqq_table[x] >> 4;}u(x<(y * y))? y - 1:y;}I
bY(A B S *T,A I *p1,A I *p2,I U){A B S *U1,*U2,*U1n,*U2n;Q(U1=T+U+*p1,U2=T+U+*p2,U1n=T+*(p1+1)+ 2,U2n=T+*(p2+1)+ 2;(U1<U1n)&&(U2<U2n)&&(*U1==*U2);++U1,++U2){}u U1<U1n ?
(U2<U2n ? *U1 - *U2:1):
(U2<U2n ? -1:0);}bE
K
ss_insertionsort(A B S *T,A I *PA,I *F,I *L,I U){I *i,*j;I t;I r;Q(i=L - 2;F<=i;--i){Q(t=*i,j=i+1;0<(r=bY(T,PA+t,PA+*j,U));){do{*(j - 1)= *j;}V((++j<L)&&(*j<0));O(L<=j){g;}}O(r==0){*j=~*j;}
*(j - 1)= t;}}K
ss_fixdown(A B S *Td,A I *PA,I *SA,I i,I bc){I j,k;I v;I c,d,e;Q(v=SA[i],c=Td[PA[v]];(j=2 * i+1)< bc;SA[i]=SA[k],i=k){d=Td[PA[SA[k=j++]]];O(d<(e=Td[PA[SA[j]]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bE
K
ss_heapsort(A B S *Td,A I *PA,I *SA,I bc){I i,m;I t;m=bc;O((bc % 2)== 0){m--;O(Td[PA[SA[m / 2]]]<Td[PA[SA[m]]]){bA(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){ss_fixdown(Td,PA,SA,i,m);}
O((bc % 2)== 0){bA(SA[0],SA[m]);ss_fixdown(Td,PA,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];ss_fixdown(Td,PA,SA,0,i);SA[i]=t;}}I *
ss_median3(A B S *Td,A I *PA,I *v1,I *v2,I *v3){I *t;O(Td[PA[*v1]]>Td[PA[*v2]]){bA(v1,v2);}
O(Td[PA[*v2]]>Td[PA[*v3]]){O(Td[PA[*v1]]>Td[PA[*v3]]){u v1;}
Y{u v3;}}u v2;}I *
ss_median5(A B S *Td,A I *PA,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(Td[PA[*v2]]>Td[PA[*v3]]){bA(v2,v3);}
O(Td[PA[*v4]]>Td[PA[*v5]]){bA(v4,v5);}
O(Td[PA[*v2]]>Td[PA[*v4]]){bA(v2,v4);bA(v3,v5);}
O(Td[PA[*v1]]>Td[PA[*v3]]){bA(v1,v3);}
O(Td[PA[*v1]]>Td[PA[*v4]]){bA(v1,v4);bA(v3,v5);}
O(Td[PA[*v3]]>Td[PA[*v4]]){u v4;}
u v3;}I *
ss_pivot(A B S *Td,A I *PA,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){u ss_median3(Td,PA,F,M,L - 1);}Y{t >>= 2;u ss_median5(Td,PA,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=ss_median3(Td,PA,F,F+t,F+(t << 1));M=ss_median3(Td,PA,M - t,M,M+t);L=ss_median3(Td,PA,L - 1 - (t << 1),L - 1 - t,L - 1);u ss_median3(Td,PA,F,M,L);}I *
ss_partition(A I *PA,I *F,I *L,I U){I *a,*b;I t;Q(a=F - 1,b=L;;){Q(;(++a<b)&&((PA[*a]+U)>= (PA[*a+1]+1));){*a=~*a;}
Q(;(a<--b)&&((PA[*b]+U)< (PA[*b+1]+1));){}O(b<=a){g;}
t=~*b;*b=*a;*a=t;}O(F<a){*F=~*F;}
u a;}bE
K
ss_mIrosort(A B S *T,A I *PA,I *F,I *L,I U){
#define bL SS_MISORT_STACKSIZE
bj{I *a,*b,c;I d;}bz[bL];A B S *Td;I *a,*b,*c,*d,*e,*f;I s,t;I bl;I bF;I v,x=0;Q(bl=0,bF=cT(L - F);;){O((L - F)<= SSIT){O(1<(L - F)){ss_insertionsort(T,PA,F,L,U);}
STACK_POP(F,L,U,bF);continue;}Td=T+U;O(bF--==0){ss_heapsort(Td,PA,F,L - F);}
O(bF<0){Q(a=F+1,v=Td[PA[*F]];a<L;++a){O((x=Td[PA[*a]])!= v){O(1<(a - F)){g;}
v=x;F=a;}}O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,a,U);}O((a - F)<= (L - a)){O(1<(a - F)){SP(a,L,U,-1);L=a,U+=1,bF=cT(a - F);}Y{F=a,bF=-1;}}Y{O(1<(L - a)){SP(F,a,U+1,cT(a - F));F=a,bF=-1;}Y{L=a,U+=1,bF=cT(a - F);}}continue;}a=ss_pivot(Td,PA,F,L);v=Td[PA[*a]];bA(*F,*a);Q(b=F;(++b<L)&&((x=Td[PA[*b]])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=Td[PA[*b]])<= v);){O(x==v){bA(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=Td[PA[*c]])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bA(*c,*d);--d;}}}Q(;b<c;){bA(*b,*c);Q(;(++b<c)&&((x=Td[PA[*b]])<= v);){O(x==v){bA(*b,*a);++a;}}Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bA(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bA(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bA(*e,*f);}
a=F+(b - a),c=L - (d - c);b=(v<=Td[PA[*a] - 1])? a:ss_partition(PA,a,c,U);O((a - F)<= (L - c)){O((L - c)<= (c - b)){SP(b,c,U+1,cT(c - b));SP(c,L,U,bF);L=a;}Y O((a - F)<= (c - b)){SP(c,L,U,bF);SP(b,c,U+1,cT(c - b));L=a;}Y{SP(c,L,U,bF);SP(F,a,U,bF);F=b,L=c,U+=1,bF=cT(c - b);}}Y{O((a - F)<= (c - b)){SP(b,c,U+1,cT(c - b));SP(F,a,U,bF);F=c;}Y O((L - c)<= (c - b)){SP(F,a,U,bF);SP(b,c,U+1,cT(c - b));F=c;}Y{SP(F,a,U,bF);SP(c,L,U,bF);F=b,L=c,U+=1,bF=cT(c - b);}}}Y{bF+=1;O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,L,U);bF=cT(L - F);}U+=1;}}
#undef bL
}K
ss_blockswap(I *a,I *b,I n){I t;Q(;0<n;--n,++a,++b){t=*a,*a=*b,*b=t;}}K
ss_rotate(I *F,I *M,I *L){I *a,*b,t;I l,r;l=M - F,r=L - M;Q(;(0<l)&&(0<r);){O(l==r){ss_blockswap(F,M,l);g;}
O(l<r){a=L - 1,b=M - 1;t=*a;do{*a--=*b,*b--=*a;O(b<F){*a=t;L=a;O((r-=l+1)<= l){g;}
a-=1,b=M - 1;t=*a;}}V(1);}Y{a=F,b=M;t=*a;do{*a++=*b,*b++=*a;O(L<=b){*a=t;F=a+1;O((l-=r+1)<= r){g;}
a+=1,b=M;t=*a;}}V(1);}}}bE
K
ss_inplacemerge(A B S *T,A I *PA,I *F,I *M,I *L,I U){A I *p;I *a,*b;I len,half;I q,r;I x;Q(;;){O(*(L - 1)< 0){x=1;p=PA+~*(L - 1);}
Y{x=0;p=PA+*(L - 1);}
Q(a=F,len=M - F,half=len >> 1,r=-1;0<len;len=half,half >>= 1){b=a+half;q=bY(T,PA+((0<=*b)? *b:~*b),p,U);O(q<0){a=b+1;half-=(len&1)^ 1;}Y{r=q;}}O(a<M){O(r==0){*a=~*a;}
ss_rotate(a,M,L);L-=M - a;M=a;O(F==M){g;}}--L;O(x != 0){V(*--L<0){}}
O(M==L){g;}}}bE
K
ss_mergeQward(A B S *T,A I *PA,I *F,I *M,I *L,I *buf,I U){I *a,*b,*c,*cC;I t;I r;cC=buf+(M - F)- 1;ss_blockswap(buf,F,M - F);Q(t=*(a=F),b=buf,c=M;;){r=bY(T,PA+*b,PA+*c,U);O(r<0){do{*a++=*b;O(cC<=b){*cC=t;u;}
*b++=*a;}V(*b<0);}Y O(r > 0){do{*a++=*c,*c++=*a;O(L<=c){V(b<cC){*a++=*b,*b++=*a;}
*a=*b,*b=t;u;}}V(*c<0);}Y{*c=~*c;do{*a++=*b;O(cC<=b){*cC=t;u;}
*b++=*a;}V(*b<0);do{*a++=*c,*c++=*a;O(L<=c){V(b<cC){*a++=*b,*b++=*a;}
*a=*b,*b=t;u;}}V(*c<0);}}}bE
K
ss_mergebackward(A B S *T,A I *PA,I *F,I *M,I *L,I *buf,I U){A I *p1,*p2;I *a,*b,*c,*cC;I t;I r;I x;cC=buf+(L - M)- 1;ss_blockswap(buf,M,L - M);x=0;O(*cC<0){p1=PA+~*cC;x |= 1;}
Y{p1=PA+*cC;}
O(*(M - 1)< 0){p2=PA+~*(M - 1);x |= 2;}
Y{p2=PA+*(M - 1);}
Q(t=*(a=L - 1),b=cC,c=M - 1;;){r=bY(T,p1,p2,U);O(0<r){O(x&1){do{*a--=*b,*b--=*a;}V(*b<0);x^=1;}
*a--=*b;O(b<=buf){*buf=t;g;}
*b--=*a;O(*b<0){p1=PA+~*b;x |= 1;}
Y{p1=PA+*b;}}Y O(r<0){O(x&2){do{*a--=*c,*c--=*a;}V(*c<0);x^=2;}
*a--=*c,*c--=*a;O(c<F){V(buf<b){*a--=*b,*b--=*a;}
*a=*b,*b=t;g;}O(*c<0){p2=PA+~*c;x |= 2;}
Y{p2=PA+*c;}}Y{O(x&1){do{*a--=*b,*b--=*a;}V(*b<0);x^=1;}
*a--=~*b;O(b<=buf){*buf=t;g;}
*b--=*a;O(x&2){do{*a--=*c,*c--=*a;}V(*c<0);x^=2;}
*a--=*c,*c--=*a;O(c<F){V(buf<b){*a--=*b,*b--=*a;}
*a=*b,*b=t;g;}O(*b<0){p1=PA+~*b;x |= 1;}
Y{p1=PA+*b;}
O(*c<0){p2=PA+~*c;x |= 2;}
Y{p2=PA+*c;}}}}bE
K
ss_swapmerge(A B S *T,A I *PA,I *F,I *M,I *L,I *buf,I cf,I U){
#define bL SS_SMERGE_STACKSIZE
#define GETIDX(a)((0<=(a))? (a):(~(a)))
#define MERGE_(a,b,c)\
do{\
O(((c)&1)||\
(((c)&2)&&(bY(T,PA+GETIDX(*((a)- 1)),PA+*(a),U)== 0))){\
*(a)= ~*(a);\
}\
O(((c)&4)&&((bY(T,PA+GETIDX(*((b)- 1)),PA+*(b),U)== 0))){\
*(b)= ~*(b);\
}\
}V(0)
bj{I *a,*b,*c;I d;}bz[bL];I *l,*r,*lm,*rm;I m,len,half;I bl;I check,bv;Q(check=0,bl=0;;){O((L - M)<= cf){O((F<M)&&(M<L)){ss_mergebackward(T,PA,F,M,L,buf,U);}MERGE_(F,L,check);STACK_POP(F,M,L,check);continue;}O((M - F)<= cf){O(F<M){ss_mergeQward(T,PA,F,M,L,buf,U);}MERGE_(F,L,check);STACK_POP(F,M,L,check);continue;}Q(m=0,len=MIN(M - F,L - M),half=len >> 1;0<len;len=half,half >>= 1){O(bY(T,PA+GETIDX(*(M+m+half)),PA+GETIDX(*(M - m - half - 1)),U)< 0){m+=half+1;half-=(len&1)^ 1;}}O(0<m){lm=M - m,rm=M+m;ss_blockswap(lm,M,m);l=r=M,bv=0;O(rm<L){O(*rm<0){*rm=~*rm;O(F<lm){Q(;*--l<0;){}bv |= 4;}
bv |= 1;}Y O(F<lm){Q(;*r<0;++r){}bv |= 2;}}O((l - F)<= (L - r)){SP(r,rm,L,(bv&3)| (check&4));M=lm,L=l,check=(check&3)| (bv&4);}Y{O((bv&2)&&(r==M)){bv^=6;}
SP(F,lm,l,(check&3)| (bv&4));F=r,M=rm,check=(bv&3)| (check&4);}}Y{O(bY(T,PA+GETIDX(*(M - 1)),PA+*M,U)== 0){*M=~*M;}MERGE_(F,L,check);STACK_POP(F,M,L,check);}}
#undef bL
}bE
K
sssort(A B S *T,A I *PA,I *F,I *L,I *buf,I cf,I U,I n,I Lsuffix){I *a;I *b,*M,*curbuf;I j,k,curbufsize,Z;I i;O(Lsuffix != 0){++F;}
O((cf<SSB)&&
(cf<(L - F))&&
(cf<(Z=ss_isqrt(L - F)))){O(SSB<Z){Z=SSB;}
buf=M=L - Z,cf=Z;}Y{M=L,Z=0;}Q(a=F,i=0;SSB<(M - a);a+=SSB,++i){ss_mIrosort(T,PA,a,a+SSB,U);curbufsize=L - (a+SSB);curbuf=a+SSB;O(curbufsize<=cf){curbufsize=cf,curbuf=buf;}
Q(b=a,k=SSB,j=i;j&1;b-=k,k <<= 1,j >>= 1){ss_swapmerge(T,PA,b - k,b,b+k,curbuf,curbufsize,U);}}ss_mIrosort(T,PA,a,M,U);Q(k=SSB;i != 0;k <<= 1,i >>= 1){O(i&1){ss_swapmerge(T,PA,a - k,a,M,buf,cf,U);a-=k;}}O(Z != 0){ss_mIrosort(T,PA,M,L,U);ss_inplacemerge(T,PA,F,M,L,U);}O(Lsuffix != 0){I PAi[2];PAi[0]=PA[*(F - 1)],PAi[1]=n - 2;Q(a=F,i=*(F - 1);(a<L)&&((*a<0)|| (0<bY(T,&(PAi[0]),PA+*a,U)));++a){*(a - 1)= *a;}*(a - 1)= i;}}I
cU(I n){u(n&0xffff0000)?
((n&0xff000000)?
24+cu[(n >> 24)&0xff]:16+cu[(n >> 16)&0xff]):
((n&0x0000ff00)?
8+cu[(n >> 8)&0xff]:0+cu[(n >> 0)&0xff]);}bE
K
tr_insertionsort(A I *X,I *F,I *L){I *a,*b;I t,r;Q(a=F+1;a<L;++a){Q(t=*a,b=a - 1;0 > (r=X[t] - X[*b]);){do{*(b+1)= *b;}V((F<=--b)&&(*b<0));O(b<F){g;}}O(r==0){*b=~*b;}
*(b+1)= t;}}K
tr_fixdown(A I *X,I *SA,I i,I bc){I j,k;I v;I c,d,e;Q(v=SA[i],c=X[v];(j=2 * i+1)< bc;SA[i]=SA[k],i=k){d=X[SA[k=j++]];O(d<(e=X[SA[j]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bE
K
tr_heapsort(A I *X,I *SA,I bc){I i,m;I t;m=bc;O((bc % 2)== 0){m--;O(X[SA[m / 2]]<X[SA[m]]){bA(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){tr_fixdown(X,SA,i,m);}
O((bc % 2)== 0){bA(SA[0],SA[m]);tr_fixdown(X,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];tr_fixdown(X,SA,0,i);SA[i]=t;}}I *
tr_median3(A I *X,I *v1,I *v2,I *v3){I *t;O(X[*v1]>X[*v2]){bA(v1,v2);}
O(X[*v2]>X[*v3]){O(X[*v1]>X[*v3]){u v1;}
Y{u v3;}}u v2;}I *
tr_median5(A I *X,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(X[*v2]>X[*v3]){bA(v2,v3);}
O(X[*v4]>X[*v5]){bA(v4,v5);}
O(X[*v2]>X[*v4]){bA(v2,v4);bA(v3,v5);}
O(X[*v1]>X[*v3]){bA(v1,v3);}
O(X[*v1]>X[*v4]){bA(v1,v4);bA(v3,v5);}
O(X[*v3]>X[*v4]){u v4;}
u v3;}I *
tr_pivot(A I *X,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){u tr_median3(X,F,M,L - 1);}Y{t >>= 2;u tr_median5(X,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=tr_median3(X,F,F+t,F+(t << 1));M=tr_median3(X,M - t,M,M+t);L=tr_median3(X,L - 1 - (t << 1),L - 1 - t,L - 1);u tr_median3(X,F,M,L);}typedef bj _tr tr;bj _tr{I c;I r;I i;I n;};K
trbudget_init(tr *b,I c,I i){b->c=c;b->r=b->i=i;}I
trbudget_check(tr *b,I bc){O(bc<=b->r){b->r-=bc;u 1;}
O(b->c==0){b->n+=bc;u 0;}
b->r+=b->i - bc;b->c-=1;u 1;}K
tr_partition(A I *X,I *F,I *M,I *L,I **pa,I **pb,I v){I *a,*b,*c,*d,*e,*f;I t,s;I x=0;Q(b=M - 1;(++b<L)&&((x=X[*b])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=X[*b])<= v);){O(x==v){bA(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=X[*c])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bA(*c,*d);--d;}}}Q(;b<c;){bA(*b,*c);Q(;(++b<c)&&((x=X[*b])<= v);){O(x==v){bA(*b,*a);++a;}}Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bA(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bA(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bA(*e,*f);}
F+=(b - a),L-=(d - c);}*pa=F,*pb=L;}bE
K
tr_copy(I *ISA,A I *SA,I *F,I *a,I *b,I *L,I U){I *c,*d,*e;I s,v;v=b - SA - 1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - U))&&(ISA[s]==v)){*++d=s;ISA[s]=d - SA;}}Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - U))&&(ISA[s]==v)){*--d=s;ISA[s]=d - SA;}}}bE
K
tr_partialcopy(I *ISA,A I *SA,I *F,I *a,I *b,I *L,I U){I *c,*d,*e;I s,v;I rank,Lrank,newrank=-1;v=b - SA - 1;Lrank=-1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - U))&&(ISA[s]==v)){*++d=s;rank=ISA[s+U];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}Lrank=-1;Q(e=d;F<=e;--e){rank=ISA[*e];O(Lrank != rank){Lrank=rank;newrank=e - SA;}
O(newrank != rank){ISA[*e]=newrank;}}Lrank=-1;Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - U))&&(ISA[s]==v)){*--d=s;rank=ISA[s+U];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}}bE
K
tr_Irosort(I *ISA,A I *X,I *SA,I *F,I *L,tr *T){
#define bL TR_STACKSIZE
bj{A I *a;I *b,*c;I d,e;}bz[bL];I *a,*b,*c;I t;I v,x=0;I cX=X - ISA;I Z,bv;I bl,TR=-1;Q(bl=0,Z=cU(L - F);;){O(Z<0){O(Z==-1){tr_partition(X - cX,F,F,L,&a,&b,L - SA - 1);O(a<L){Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}}O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}O(1<(b - a)){S5(NULL,a,b,0,0);S5(X - cX,F,L,-2,TR);TR=bl - 2;}O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,cU(L - b),TR);L=a,Z=cU(a - F);}Y O(1<(L - b)){F=b,Z=cU(L - b);}Y{ci(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,cU(a - F),TR);F=b,Z=cU(L - b);}Y O(1<(a - F)){L=a,Z=cU(a - F);}Y{ci(X,F,L,Z,TR);}}}Y O(Z==-2){a=bz[--bl].b,b=bz[bl].c;O(bz[bl].d==0){tr_copy(ISA,SA,F,a,b,L,X - ISA);}Y{O(0<=TR){bz[TR].d=-1;}
tr_partialcopy(ISA,SA,F,a,b,L,X - ISA);}ci(X,F,L,Z,TR);}Y{O(0<=*F){a=F;do{ISA[*a]=a - SA;}V((++a<L)&&(0<=*a));F=a;}O(F<L){a=F;do{*a=~*a;}V(*++a<0);bv=(ISA[*a] != X[*a])? cU(a - F+1):-1;O(++a<L){Q(b=F,v=a - SA - 1;b<a;++b){ISA[*b]=v;}}
O(trbudget_check(T,a - F)){O((a - F)<= (L - a)){S5(X,a,L,-3,TR);X+=cX,L=a,Z=bv;}Y{O(1<(L - a)){S5(X+cX,F,a,bv,TR);F=a,Z=-3;}Y{X+=cX,L=a,Z=bv;}}}Y{O(0<=TR){bz[TR].d=-1;}
O(1<(L - a)){F=a,Z=-3;}Y{ci(X,F,L,Z,TR);}}}Y{ci(X,F,L,Z,TR);}}continue;}O((L - F)<= TR_INSERTIONSORT_THRESHOLD){tr_insertionsort(X,F,L);Z=-3;continue;}O(Z--==0){tr_heapsort(X,F,L - F);Q(a=L - 1;F<a;a=b){Q(x=X[*a],b=a - 1;(F<=b)&&(X[*b]==x);--b){*b=~*b;}}Z=-3;continue;}a=tr_pivot(X,F,L);bA(*F,*a);v=X[*F];tr_partition(X,F,F+1,L,&a,&b,v);O((L - F)!= (b - a)){bv=(ISA[*a] != v)? cU(b - a):-1;Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}
O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}
O((1<(b - a))&&(trbudget_check(T,b - a))){O((a - F)<= (L - b)){O((L - b)<= (b - a)){O(1<(a - F)){S5(X+cX,a,b,bv,TR);S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){S5(X+cX,a,b,bv,TR);F=b;}Y{X+=cX,F=a,L=b,Z=bv;}}Y O((a - F)<= (b - a)){O(1<(a - F)){S5(X,b,L,Z,TR);S5(X+cX,a,b,bv,TR);L=a;}Y{S5(X,b,L,Z,TR);X+=cX,F=a,L=b,Z=bv;}}Y{S5(X,b,L,Z,TR);S5(X,F,a,Z,TR);X+=cX,F=a,L=b,Z=bv;}}Y{O((a - F)<= (b - a)){O(1<(L - b)){S5(X+cX,a,b,bv,TR);S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){S5(X+cX,a,b,bv,TR);L=a;}Y{X+=cX,F=a,L=b,Z=bv;}}Y O((L - b)<= (b - a)){O(1<(L - b)){S5(X,F,a,Z,TR);S5(X+cX,a,b,bv,TR);F=b;}Y{S5(X,F,a,Z,TR);X+=cX,F=a,L=b,Z=bv;}}Y{S5(X,F,a,Z,TR);S5(X,b,L,Z,TR);X+=cX,F=a,L=b,Z=bv;}}}Y{O((1<(b - a))&&(0<=TR)){bz[TR].d=-1;}
O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){F=b;}Y{ci(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){L=a;}Y{ci(X,F,L,Z,TR);}}}}Y{O(trbudget_check(T,L - F)){Z=cU(L - F),X+=cX;}Y{O(0<=TR){bz[TR].d=-1;}
ci(X,F,L,Z,TR);}}}
#undef bL
}bE
K
trsort(I *ISA,I *SA,I n,I U){I *X;I *F,*L;tr b;I t,skip,unsorted;trbudget_init(&b,cU(n)* 2 / 3,n);Q(X=ISA+U;-n<*SA;X+=X - ISA){F=SA;skip=0;unsorted=0;do{O((t=*F)< 0){F-=t;skip+=t;}
Y{O(skip != 0){*(F+skip)= skip;skip=0;}
L=SA+ISA[t]+1;O(1<(L - F)){b.n=0;tr_Irosort(ISA,X,SA,F,L,&b);O(b.n != 0){unsorted+=b.n;}
Y{skip=F - L;}}Y O((L - F)== 1){skip=-1;}F=L;}}V(F<(SA+n));O(skip != 0){*(F+skip)= skip;}
O(unsorted==0){g;}}}bE
I
sort_typeBstar(A B S *T,I *SA,I *cB,I *ct,I n){I *PAb,*ISAb,*buf;I i,j,k,t,m,cf;I c0,c1;Q(i=0;i<BUCKET_A_SIZE;++i){cB[i]=0;}
Q(i=0;i<BUCKET_B_SIZE;++i){ct[i]=0;}
Q(i=n - 1,m=n,c0=T[n - 1];0<=i;){do{++cN(c1=c0);}V((0<=--i)&&((c0=T[i])>= c1));O(0<=i){++BB(c0,c1);SA[--m]=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){++cq(c0,c1);}}}m=n - m;Q(c0=0,i=0,j=0;c0<AS;++c0){t=i+cN(c0);cN(c0)= i+j;i=t+cq(c0,c0);Q(c1=c0+1;c1<AS;++c1){j+=BB(c0,c1);BB(c0,c1)= j;i+=cq(c0,c1);}}O(0<m){PAb=SA+n - m;ISAb=SA+m;Q(i=m - 2;0<=i;--i){t=PAb[i],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=i;}t=PAb[m - 1],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=m - 1;buf=SA+m,cf=n - (2 * m);Q(c0=AS - 2,j=m;0<j;--c0){Q(c1=AS - 1;c0<c1;j=i,--c1){i=BB(c0,c1);O(1<(j - i)){sssort(T,PAb,SA+i,SA+j,buf,cf,2,n,*(SA+i)== (m - 1));}}}Q(i=m - 1;0<=i;--i){O(0<=SA[i]){j=i;do{ISAb[SA[i]]=i;}V((0<=--i)&&(0<=SA[i]));SA[i+1]=i - j;O(i<=0){g;}}j=i;do{ISAb[SA[i]=~SA[i]]=j;}V(SA[--i]<0);ISAb[SA[i]]=j;}trsort(ISAb,SA,m,1);Q(i=n - 1,j=m,c0=T[n - 1];0<=i;){Q(--i,c1=c0;(0<=i)&&((c0=T[i])>= c1);--i,c1=c0){}O(0<=i){t=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){}SA[ISAb[--j]]=((t==0)|| (1<(t - i)))? t:~t;}}cq(AS - 1,AS - 1)= n;Q(c0=AS - 2,k=m - 1;0<=c0;--c0){i=cN(c0+1)- 1;Q(c1=AS - 1;c0<c1;--c1){t=i - cq(c0,c1);cq(c0,c1)= i;Q(i=t,j=BB(c0,c1);j<=k;--i,--k){SA[i]=SA[k];}}BB(c0,c0+1)= i - cq(c0,c0)+ 1;cq(c0,c0)= i;}}u m;}bE
K
construct_SA(A B S *T,I *SA,I *cB,I *ct,I n,I m){I *i,*j,*k;I s;I c0,c1,c2;O(0<m){Q(c1=AS - 2;0<=c1;--c1){Q(i=SA+BB(c1,c1+1),j=SA+cN(c1+1)- 1,k=NULL,c2=-1;i<=j;--j){O(0<(s=*j)){*j=~s;c0=T[--s];O((0<s)&&(T[s - 1]>c0)){s=~s;}
O(c0 != c2){O(0<=c2){cq(c2,c1)= k - SA;}
k=SA+cq(c2=c0,c1);}*k--=s;}Y{*j=~s;}}}}k=SA+cN(c2=T[n - 1]);*k++=(T[n - 2]<c2)? ~(n - 1):(n - 1);Q(i=SA,j=SA+n;i<j;++i){O(0<(s=*i)){c0=T[--s];O((s==0)|| (T[s - 1]<c0)){s=~s;}
O(c0 != c2){cN(c2)= k - SA;k=SA+cN(c2=c0);}*k++=s;}Y{*i=~s;}}}I
divsufsort(A B S *T,I *SA,I n){I *cB,*ct;I m;I err=0;O((T==NULL)|| (SA==NULL)|| (n<0)){u -1;}
Y O(n==0){u 0;}
Y O(n==1){SA[0]=0;u 0;}
Y O(n==2){m=(T[0]<T[1]);SA[m ^ 1]=0,SA[m]=1;u 0;}
cB=(I *)malloc(BUCKET_A_SIZE * sizeof(I));ct=(I *)malloc(BUCKET_B_SIZE * sizeof(I));O((cB != NULL)&&(ct != NULL)){m=sort_typeBstar(T,SA,cB,ct,n);construct_SA(T,SA,cB,ct,n,m);}Y{err=-2;}free(ct);free(cB);u err;}std::bO ca(int64_t x,I n=1){std::bO r;Q(;x||n>0;x/=10,--n)r=std::bO(1,'0'+x%10)+r;u r;}bq cO:bu LQ::cc{LQ::cv<B> ht;A B S* in;A I checkbits;A I bR;A B htsize;A B n;B i;A B m;A B m2;A B maxMatch;A B maxLiteral;A B cK;B h1,h2;A B bucket;A B shift1,shift2;A I mBoth;A B rb;B bits;B nbits;B bK,bo;B idx;A B* sa;B* isa;enum{cl=1<<14};B S buf[cl];K cV(B i,B&lit);K write_match(B len,B off);K fill();K putb(B x,I k){x&=(1<<k)-1;bits|=x<<nbits;nbits+=k;V(nbits>7){buf[bo++]=bits,bits>>=8,nbits-=8;}}K flush(){O(nbits>0)buf[bo++]=bits;bits=nbits=0;}K put(I c){buf[bo++]=c;}bu:cO(SB&inbuf,I bh[],A B* sap=0);I get(){I c=-1;O(bK==bo)fill();O(bK<bo)c=buf[bK++];O(bK==bo)bK=bo=0;u c;}I read(S* p,I n);};I lg(B x){B r=0;O(x>=65536)r=16,x>>=16;O(x>=256)r+=8,x>>=8;O(x>=16)r+=4,x>>=4;u
"\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;}I nbits(B x){I r;Q(r=0;x;x>>=1)r+=x&1;u r;}I cO::read(S* p,I n){O(bK==bo)fill();I nr=n;O(nr>I(bo-bK))nr=bo-bK;O(nr)memcpy(p,buf+bK,nr);bK+=nr;O(bK==bo)bK=bo=0;u nr;}cO::cO(SB&inbuf,I bh[],A B* sap):
ht((bh[1]&3)==3 ? (inbuf.bc()+1)*!sap
:bh[5]-bh[0]<21 ? 1u<<bh[5]
:(inbuf.bc()*!sap)+(1u<<17<<bh[0])),in(inbuf.bP()),checkbits(bh[5]-bh[0]<21 ? 12-bh[0]:17+bh[0]),bR(bh[1]&3),htsize(ht.bc()),n(inbuf.bc()),i(0),m(bh[2]),m2(bh[3]),maxMatch(cl*3),maxLiteral(cl/4),cK(bh[6]),h1(0),h2(0),bucket((1<<bh[4])-1),shift1(m>0 ? (bh[5]-1)/m+1:1),shift2(m2>0 ? (bh[5]-1)/m2+1:0),mBoth(MAX(m,m2+cK)+4),rb(bh[0]>4 ? bh[0]-4:0),bits(0),nbits(0),bK(0),bo(0),idx(0),sa(0),isa(0){O((m<4&&bR==1)|| (m<1&&bR==2))
E(0);O(bh[5]-bh[0]>=21||bR==3){O(sap)
sa=sap;Y{sa=&ht[0];O(n>0)divsufsort((A B S*)in,(I*)sa,n);}O(bR<3){isa=&ht[n*(sap==0)];}}}K cO::fill(){O(bR==3){Q(;bo<cl&&i<n+5;++i){O(i==0)put(n>0 ? in[n-1]:255);Y O(i>n)put(idx&255),idx>>=8;Y O(sa[i-1]==0)idx=i,put(255);Y put(in[sa[i-1]-1]);}u;}B lit=0;A B mask=(1<<checkbits)-1;V(i<n&&bo*2<cl){B cW=m-1;B bp=0;B blit=0;I bscore=0;O(isa){O(sa[isa[i&mask]]!=i)
Q(B j=0;j<n;++j)
O((sa[j]&~mask)==(i&~mask))
isa[sa[j]&mask]=j;Q(B h=0;h<=cK;++h){B q=isa[(h+i)&mask];O(sa[q]!=h+i)continue;Q(I j=-1;j<=1;j+=2){Q(B k=1;k<=bucket;++k){B p;O(q+j*k<n&&(p=sa[q+j*k]-h)<i){B l,l1;Q(l=h;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);Q(l1=h;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-4*(lit==0&&l1>0)-11;Q(B a=0;a<h;++a)score=score*5/8;O(score>bscore)cW=l,bp=p,blit=l1,bscore=score;O(l<cW||l<m||l>255)g;}}}O(bscore<=0||cW<m)g;}}Y O(bR==1||m<=64){O(m2>0){Q(B k=0;k<=bucket;++k){B p=ht[h2^k];O(p&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+cW<=n&&in[p+cW-1]==in[i+cW-1]){B l;Q(l=cK;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);O(l>=m2+cK){I l1;Q(l1=cK;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-8*(lit==0&&l1>0)-11;O(score>bscore)cW=l,bp=p,blit=l1,bscore=score;}}}O(cW>=128)g;}}O(!m2||cW<m2){Q(B k=0;k<=bucket;++k){B p=ht[h1^k];O(p&&i+3<n&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+cW<=n&&in[p+cW-1]==in[i+cW-1]){B l;Q(l=0;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);I score=l*8-lg(i-p)-2*(lit>0)-11;O(score>bscore)cW=l,bp=p,blit=0,bscore=score;}}O(cW>=128)g;}}}A B off=i-bp;O(off>0&&bscore>0
&&cW-blit>=m+(bR==2)*((off>=(1<<16))+(off>=(1<<24)))){lit+=blit;cV(i+blit,lit);write_match(cW-blit,off);}Y{cW=1;++lit;}O(isa)
i+=cW;Y{V(cW--){O(i+mBoth<n){B ih=((i*1234547)>>19)&bucket;A B p=(i<<checkbits)|(in[i+3]&mask);O(m2){ht[h2^ih]=p;h2=(((h2*9)<<shift2)
+(in[i+m2+cK]+1)*23456789u)&(htsize-1);}ht[h1^ih]=p;h1=(((h1*5)<<shift1)+(in[i+m]+1)*123456791u)&(htsize-1);}++i;}}O(lit>=maxLiteral)
cV(i,lit);}O(i==n){cV(n,lit);flush();}}K cO::cV(B i,B&lit){O(bR==1){O(lit<1)u;I ll=lg(lit);putb(0,2);--ll;V(--ll>=0){putb(1,1);putb((lit>>ll)&1,1);}putb(0,1);V(lit)putb(in[i-lit--],8);}Y{V(lit>0){B lit1=lit;O(lit1>64)lit1=64;put(lit1-1);Q(B j=i-lit;j<i-lit+lit1;++j)put(in[j]);lit-=lit1;}}}K cO::write_match(B len,B off){O(bR==1){I ll=lg(len)-1;off+=(1<<rb)-1;I lo=lg(off)-1-rb;putb((lo+8)>>3,2);putb(lo&7,3);V(--ll>=2){putb(1,1);putb((len>>ll)&1,1);}putb(0,1);putb(len&3,2);putb(off,rb);putb(off>>rb,lo);}Y{--off;V(len>0){A B len1=len>m*2+63 ? m+63:len>m+63 ? len-m:len;O(off<(1<<16)){put(64+len1-m);put(off>>8);put(off);}Y O(off<(1<<24)){put(128+len1-m);put(off>>16);put(off>>8);put(off);}Y{put(192+len1-m);put(off>>24);put(off>>16);put(off>>8);put(off);}len-=len1;}}}std::bO makeConfig(A S* ME,I bh[]){A S bf=ME[0];bh[0]=0;bh[1]=0;bh[2]=0;bh[3]=0;bh[4]=0;bh[5]=0;bh[6]=0;bh[7]=0;bh[8]=0;O(isdigit(*++ME))bh[0]=0;Q(I i=0;i<9&&(isdigit(*ME)|| *ME==','||*ME=='.');){O(isdigit(*ME))
bh[i]=bh[i]*10+*ME-'0';Y O(++i<9)
bh[i]=0;++ME;}O(bf=='0')
u "comp 0 0 0 0 0 bJ end\n";std::bO hdr,cJ;A I bR=bh[1]&3;A cj doe8=bh[1]>=4&&bh[1]<=7;O(bR==1){E(0);}Y O(bR==2){hdr="comp 9 16 0 $1+20 ";cJ=
"cJ lzpre c ;\n"
" a> 255 O\n";O(doe8){E(0);}cJ+="b=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n";O(!doe8)cJ+=" out ";cJ+="a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++";O(!doe8)cJ+=" out d-- a=d a> 0 V endif endif endif halt end ";}Y O(bR==3){E(0);}Y O(bR==0){E(0);}Y
E(0);I ncomp=0;A I membits=bh[0]+20;I sb=5;std::bO comp;std::bO bJ="bJ\n"
"c-- *c=a a+= 255 d=a *d=c\n";O(bR==2){bJ+=
" a=r 1 a== 0 O\n"
" a= "+ca(111+57*doe8)+"\n"
" Y a== 1 O\n"
" a=*c r=a 2\n"
" a> 63 O a>>= 6 a++ a++\n"
" Y a++ a++ endif\n"
" Y\n"
" a--\n"
" endif endif\n"
" r=a 1\n";}V(*ME&&ncomp<254){std::vector<I> v;v.push_back(*ME++);O(isdigit(*ME)){v.push_back(*ME++-'0');V(isdigit(*ME)|| *ME==','||*ME=='.'){O(isdigit(*ME))
v.back()=v.back()*10+*ME++-'0';Y{v.push_back(0);++ME;}}}O(v[0]=='c'){V(v.bc()<3)v.push_back(0);comp+=ca(ncomp)+" ";sb=11;O(v[2]<256)sb+=lg(v[2]);Y sb+=6;Q(B i=3;i<v.bc();++i)
O(v[i]<512)sb+=nbits(v[i])*3/4;O(sb>membits)sb=membits;O(v[1]%1000==0)comp+="icm "+ca(sb-6-v[1]/1000)+"\n";Y comp+="cm "+ca(sb-2-v[1]/1000)+" "+ca(v[1]%1000-1)+"\n";bJ+="d= "+ca(ncomp)+" *d=0\n";O(v[2]>1&&v[2]<=255){O(lg(v[2])!=lg(v[2]-1))
bJ+="a=c a&= "+ca(v[2]-1)+" hashd\n";Y
bJ+="a=c a%= "+ca(v[2])+" hashd\n";}Y O(v[2]>=1000&&v[2]<=1255)
bJ+="a= 255 a+= "+ca(v[2]-1000)+
" d=a a=*d a-=c a> 255 O a= 255 endif d= "+
ca(ncomp)+" hashd\n";Q(B i=3;i<v.bc();++i){O(i==3)bJ+="b=c ";O(v[i]==255)
bJ+="a=*b hashd\n";Y O(v[i]>0&&v[i]<255)
bJ+="a=*b a&= "+ca(v[i])+" hashd\n";Y O(v[i]>=256&&v[i]<512){bJ+=
"a=r 1 a> 1 O\n"
" a=r 2 a< 64 O\n"
" a=*b ";O(v[i]<511)bJ+="a&= "+ca(v[i]-256);bJ+=" hashd\n"
" Y\n"
" a>>= 6 hashd a=r 1 hashd\n"
" endif\n"
"Y\n"
" a= 255 hashd a=r 2 hashd\n"
"endif\n";}Y O(v[i]>=1256)
bJ+="a= "+ca(((v[i]-1000)>>8)&255)+" a<<= 8 a+= "
+ca((v[i]-1000)&255)+
" a+=b b=a\n";Y O(v[i]>1000)
bJ+="a= "+ca(v[i]-1000)+" a+=b b=a\n";O(v[i]<512&&i<v.bc()-1)
bJ+="b++ ";}++ncomp;}O(strchr("mts",v[0])&&ncomp>I(v[0]=='t')){E(0);}O(v[0]=='i'&&ncomp>0){bJ+="d= "+ca(ncomp-1)+" b=c a=*d d++\n";Q(B i=1;i<v.bc()&&ncomp<254;++i){Q(I j=0;j<v[i]%10;++j){bJ+="hash ";O(i<v.bc()-1||j<v[i]%10-1)bJ+="b++ ";sb+=6;}bJ+="*d=a";O(i<v.bc()-1)bJ+=" d++";bJ+="\n";O(sb>membits)sb=membits;comp+=ca(ncomp)+" isse "+ca(sb-6-v[i]/10)+" "+ca(ncomp-1)+"\n";++ncomp;}}O(v[0]=='a'){E(0);}O(v[0]=='w'){E(0);}}u hdr+ca(ncomp)+"\n"+comp+bJ+"halt\n"+cJ;}K compressBlock(SB* in,bn* out,A S* method_,A S* cA,A S* comment,cj dosha1){std::bO ME=method_;A B n=in->bc();A I arg0=MAX(lg(n+4095)-20,0);B bf=0;O(isdigit(ME[0])){I commas=0,arg[4]={0};Q(I i=1;i<I(ME.bc())&&commas<4;++i){O(ME[i]==','||ME[i]=='.')++commas;Y O(isdigit(ME[i]))arg[commas]=arg[commas]*10+ME[i]-'0';}O(commas==0)bf=512;Y bf=arg[1]*4+arg[2];}O(isdigit(ME[0])){A I bR=ME[0]-'0';A I doe8=(bf&2)*2;ME="x"+ca(arg0);std::bO htsz=","+ca(19+arg0+(arg0<=6));std::bO sasz=","+ca(21+arg0);O(bR==0)
ME="0"+ca(arg0)+",0";Y O(bR==1){E(0);}Y O(bR==2){E(0);}Y O(bR==3){O(bf<20)
ME+=",0";Y O(bf<48)
ME+=","+ca(1+doe8)+",4,0,3"+htsz;Y O(bf>=640||(bf&1))
ME+=","+ca(3+doe8)+"ci1";Y
ME+=","+ca(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";}Y O(bR==4){E(0);}Y{E(0);}}std::bO config;I bh[9]={0};config=makeConfig(ME.c_str(),bh);LQ::C co;co.setOutput(out);SB pcomp_cmd;co.startBlock(config.c_str(),bh,&pcomp_cmd);std::bO cs=ca(n);co.startSegment(cA,cs.c_str());O(bh[1]>=1&&bh[1]<=7&&bh[1]!=4){cO lz(*in,bh);co.setInput(&lz);co.compress();}Y{E(0);}co.endSegment(0);}}namespace zpaq{td::cD compress(td::Slice bP){I bc=td::narrow_cast<int>(bP.bc());LQ::SB in,out;in.bw(bP.bP(),bc);LQ::compressBlock(&in,&out,"3",0,0,false);u td::cD(out.c_str(),out.bc());}td::cD decompress(td::Slice bP){LQ::SB in,out;in.bw(bP.bP(),bP.bc());LQ::decompress(&in,&out);u td::cD(out.c_str(),out.bc());}}
#undef Y
#undef O
#undef Q
td::cD compress(td::Slice bP){NullStream ofs;Ref<Cell> block_root=std_boc_deserialize(bP).move_as_ok();FullBlock block;J load_std_ctx{ofs};block.bx(load_std_ctx,block_root,0,cQ);J pack_opt_ctx{ofs};cL opt_block_cell=block.bM(pack_opt_ctx);BagOfCells opt_boc;opt_boc.add_root(opt_block_cell);(opt_boc.import_cells().is_ok());cL opt_ser=std_boc_serialize(opt_block_cell).move_as_ok();cL compressed=zpaq::compress(opt_ser);u compressed;}td::cD decompress(td::Slice bP){NullStream ofs;cL decompressed=zpaq::decompress(bP);cL opt_deser=std_boc_deserialize(decompressed).move_as_ok();FullBlock opt_block;J parse_opt_ctx{ofs};opt_block.by(parse_opt_ctx,opt_deser,0,cQ);J pack_std_ctx{ofs};cL un_opt_block_cell=opt_block.bt(pack_std_ctx);BagOfCells un_opt_boc;un_opt_boc.add_root(un_opt_block_cell);(un_opt_boc.import_cells().is_ok());cL boc=std_boc_serialize(un_opt_block_cell,31).move_as_ok();u boc;}int main(){bO mode;cin >> mode;bO base64_data;cin >> base64_data;td::cD bP(td::base64_decode(base64_data));bP=(mode=="compress")? compress(bP):decompress(bP);cout << td::str_base64_encode(bP)<< endl;}