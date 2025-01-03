
#define final
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
#define dx as_cellslice
#define du continue
#define dl blen
#define cZ skip
#define cV ShardStateUnsplit
#define cS in_msg_descr
#define cR auto
#define cQ CurrencyCollection
#define cO BufferSlice
#define cK shard_state
#define cH template
#define cG out_msg_descr
#define cE true
#define cx state_update
#define cw special_type
#define cn store_long
#define bZ HashmapAugE
#define bV data
#define bU bool
#define bM virtual
#define bK static
#define bJ append_cellslice
#define bI string
#define bH extra
#define bB public
#define by class
#define br fetch_ref
#define bo write
#define bi size_t
#define bh size
#define bf struct
#define be store_ref
#define ba while
#define V char
#define U CellBuilder
#define S CellSlice
#define N void
#define J unsigned
#define B const
#define A return
#define o case
#define g break
using namespace vm;using namespace std;set<bI> enabled_optimizations{"Block","BlockExtra","bZ","HashmapAug","HashmapAugNode","InMsgDescr","OutMsgDescr","MERKLE_UPDATE","ShardState","cV","OutMsgQueueInfo",};S to_cs(Ref<Cell> cell){bU can_be_special=false;A load_cell_slice_special(std::move(cell),can_be_special);}bf G{ostream&out;};cH<by T_TLB>bf K{S incoming_cs;S bN;bI name;T_TLB bd;int cw=0;K(bI name):K(name,T_TLB()){}K(bI name,T_TLB bd):name(name),bd(bd){}bM ~K(){}bM N bg(G&ctx,S&cs,int u=0){}bM N bF(G&ctx,U&cb,int u=0){bm(ctx,cb,u);}bM N bu(G&ctx,S&cs,int u=0){bg(ctx,cs,u);}bM N bm(G&ctx,U&cb,int u=0){}bU is_enabled(){A enabled_optimizations.count(name)> 0;}bU dd(){A cw==1;}N bA(G&ctx,S&cs,int u=0,bU de=false){cR e=is_enabled();cw=(int)cs.cw();incoming_cs=cs;if(dd()){cs.advance(288);A;}if(e){bg(ctx,cs,u);}else{bN=bd.fetch(cs).bo();}}N cY(G&ctx,U&cb,int u=0){cR e=is_enabled();if(dd()){cb.bJ(incoming_cs);}else if(e){bF(ctx,cb,u);}else{cb.bJ(bN);}}N ca(G&ctx,S&cs,int u=0,bU de=false){cR e=is_enabled();incoming_cs=cs;if(e){bu(ctx,cs,u);}else{bN=bd.fetch(cs).bo();}}N bY(G&ctx,U&cb,int u=0){cR e=is_enabled();if(dd()){cb.bJ(incoming_cs);}else if(e){bm(ctx,cb,u);}else{if(!bN.is_valid()){throw runtime_error(name+":optimization is disabled,but bN is empty,meaning it was never set");}cb.bJ(bN);}}Ref<Cell> bb(G&ctx,int u=0){U cb;bY(ctx,cb,u);A cb.finalize(cw!=0);}Ref<Cell> bG(G&ctx,int u=0){U cb;cY(ctx,cb,u);A cb.finalize(cw!=0);}N bc(G&ctx,Ref<Cell> cell_ref,int u=0,bU de=false){cR cs=to_cs(std::move(cell_ref));bA(ctx,cs,u,de);}N bx(G&ctx,Ref<Cell> cell_ref,int u=0,bU de=false){cR cs=to_cs(std::move(cell_ref));ca(ctx,cs,u,de);}};cH<by cD>bf cI{bM ~cI(){}bM S df(){throw runtime_error("aug bV requested but not implemented");}};cH<by T_TLB>bf ct{T_TLB add_type;ct(T_TLB add_type):add_type(add_type){}ct():ct(T_TLB{}){}bM ~ct(){}bM S add_values(S&cs1,S&cs2){U cb;(add_type.add_values(cb,cs1,cs2));A cb.dx();}};using namespace block::tlb;B cQ tCC;B OutMsg tOM;B ImportFees tIF;B InMsg tIM;B EnqueuedMsg tEM;B UInt tU64{64};cH<by cU,by cD>bf bv;cH<by cU,by cD>bf FullHashmapAugNode:K<block::gen::HashmapAugNode>{int tag=-1;int n=-1;Ref<bv<cU,cD>> left;Ref<bv<cU,cD>> right;cU value;cD bH;FullHashmapAugNode(int m,B TLB&X,B TLB&Y):K("HashmapAugNode",block::gen::HashmapAugNode(m,X,Y)){}N bg(G&ctx,S&cs,int u=0){tag=bd.check_tag(cs);if(tag==0){bH.bA(ctx,cs,u+1);value.bA(ctx,cs,u+1);}else{int n;add_r1(n,1,bd.m_);left=Ref<bv<cU,cD>>(cE,n,bd.X_,bd.Y_);left.bo().bc(ctx,cs.br(),u+1);right=Ref<bv<cU,cD>>(cE,n,bd.X_,bd.Y_);right.bo().bc(ctx,cs.br(),u+1);bH.bA(ctx,cs,u+1);}}N bF(G&ctx,U&cb,int u=0){if(tag==0){value.cY(ctx,cb,u+1);}else{int n;(add_r1(n,1,bd.m_));cb.be(left.bo().bG(ctx,u+1));cb.be(right.bo().bG(ctx,u+1));}}N bu(G&ctx,S&cs,int u=0){tag=bd.check_tag(cs);if(tag==0){value.ca(ctx,cs,u+1);cR extra_cs=value.df();bH.ca(ctx,extra_cs,u+1,cE);}else{int n;add_r1(n,1,bd.m_);left=Ref<bv<cU,cD>>(cE,n,bd.X_,bd.Y_);left.bo().bx(ctx,cs.br(),u+1);right=Ref<bv<cU,cD>>(cE,n,bd.X_,bd.Y_);right.bo().bx(ctx,cs.br(),u+1);cR left_extra_cs=to_cs(left.bo().node.bH.bb(ctx));cR right_extra_cs=to_cs(right.bo().node.bH.bb(ctx));cR extra_cs=bH.add_values(left_extra_cs,right_extra_cs);bH.ca(ctx,extra_cs,u+1,cE);}}N bm(G&ctx,U&cb,int u=0){if(tag==0){bH.bY(ctx,cb,u+1);value.bY(ctx,cb,u+1);}else{int n;(add_r1(n,1,bd.m_));cb.be(left.bo().bb(ctx,u+1));cb.be(right.bo().bb(ctx,u+1));bH.bY(ctx,cb,u+1);}}};cH<by cU,by cD>bf bv:K<block::gen::HashmapAug>,td::CntObject{Ref<S> label;int n,m,l;FullHashmapAugNode<cU,cD> node;bv(int n,B TLB&X,B TLB&Y):K("HashmapAug",block::gen::HashmapAug(n,X,Y)),node(n,X,Y){}N bg(G&ctx,S&cs,int u=0){n=bd.m_;(block::gen::HmLabel{n}.fetch_to(cs,label,l));m=n - l;node.bd.m_=m;node.bA(ctx,cs,u+1);}N bF(G&ctx,U&cb,int u=0){int l,m;(tlb::store_from(cb,HmLabel{bd.m_},label,l));(add_r1(m,l,bd.m_));node.cY(ctx,cb,u+1);}N bu(G&ctx,S&cs,int u=0){(
(n=bd.m_)>= 0
&&block::gen::HmLabel{bd.m_}.fetch_to(cs,label,l)&&add_r1(m,l,bd.m_));node.bd.m_=m;node.ca(ctx,cs,u+1);}N bm(G&ctx,U&cb,int u=0){int l,m;(tlb::store_from(cb,block::gen::HmLabel{bd.m_},label,l)&&add_r1(m,l,bd.m_));node.bY(ctx,cb,u+1);}};cH<by cU,by cD>bf cJ:K<block::gen::bZ>{block::gen::bZ::Record_ahme_root r;int tag=-1;bv<cU,cD> root;cD bH;cJ(int n,B TLB&X,B TLB&Y):K("bZ",block::gen::bZ(n,X,Y)),root(n,X,Y){}N bg(G&ctx,S&cs,int u=0){tag=bd.check_tag(cs);if(tag==block::gen::bZ::ahme_empty){(cs.fetch_ulong(1)== 0);bH.bA(ctx,cs,u+1);}else{(bd.unpack(cs,r));root.bc(ctx,r.root,u+1);bH.bA(ctx,r.bH.bo(),u+1);}}N bF(G&ctx,U&cb,int u=0){if(tag==bZ::ahme_empty){cb.cn(0,1);bH.cY(ctx,cb,u+1);}else{cb.cn(1,1).be(root.bG(ctx,u+1));bH.cY(ctx,cb,u+1);}}N bu(G&ctx,S&cs,int u=0){tag=bd.check_tag(cs);if(tag==bZ::ahme_empty){(cs.fetch_ulong(1)== 0);bH.ca(ctx,cs,u+1);}else{(cs.fetch_ulong(1)== 1&&(r.n=bd.m_)>= 0);cR root_ref=cs.br();root.bx(ctx,root_ref,u+1);bH=root.node.bH;}}N bm(G&ctx,U&cb,int u=0){if(tag==bZ::ahme_empty){cb.cn(0,1);bH.bY(ctx,cb,u+1);}else{cb.cn(1,1).be(root.bb(ctx,u+1));bH.bY(ctx,cb,u+1);}}};bf FullUInt:K<UInt>,ct<UInt>{FullUInt(bI name,UInt bd):K(name,bd),ct(bd){}};bf FullUInt64:FullUInt{FullUInt64():FullUInt("Unit64",tU64){}};bf cu;bf FullInMsg:K<InMsg>,cI<cu>{FullInMsg():K("InMsg",InMsg()){}S df(){U cb;cR cs_copy=bN;(bd.get_import_fees(cb,cs_copy));A cb.dx();}};bf ci:K<cQ>,ct<cQ>{ci():K("cQ"){}};bf FullOutMsg:K<OutMsg>,cI<ci>{FullOutMsg():K("OutMsg",OutMsg()){}S df(){U cb;cR cs_copy=bN;(bd.get_export_value(cb,cs_copy));A cb.dx();}};bf EnqueuedMsgAug:FullUInt64{S add_values(S&cs1,S&cs2){U cb;J long long x,y;(cs1.fetch_ulong_bool(64,x)&&
cs2.fetch_ulong_bool(64,y)&&
cb.store_ulong_rchk_bool(std::min(x,y),64));A cb.dx();}};bf FullEnqueuedMsg:K<EnqueuedMsg>,cI<EnqueuedMsgAug>{FullEnqueuedMsg():K("EnqueuedMsg"){}S df(){U cb;cR cs_copy=bN;(Aug_OutMsgQueue().eval_leaf(cb,cs_copy));A cb.dx();}};bf FullOutMsgQueue:K<OutMsgQueue>{cJ<FullEnqueuedMsg,EnqueuedMsgAug> x{352,tEM,tU64};FullOutMsgQueue():K("OutMsgQueue",OutMsgQueue()){}N bg(G&ctx,S&cs,int u=0){x.bA(ctx,cs,u+1);}N bF(G&ctx,U&cb,int u=0){x.cY(ctx,cb,u+1);}N bu(G&ctx,S&cs,int u=0){x.ca(ctx,cs,u+1);}N bm(G&ctx,U&cb,int u=0){x.bY(ctx,cb,u+1);}};bf FullOutMsgQueueInfo:K<OutMsgQueueInfo>{S ics;FullOutMsgQueue out_queue;FullOutMsgQueueInfo():K("OutMsgQueueInfo"){}bM N bg(G&ctx,S&cs,int u=0){out_queue.bA(ctx,cs,u+1);ics=cs;cs.advance(cs.bh());cs.advance_refs(cs.size_refs());}bM N bm(G&ctx,U&cb,int u=0){out_queue.bY(ctx,cb,u+1);cb.bJ(ics);}};bf dn:block::gen::MERKLE_UPDATE{dn(B TLB&X):MERKLE_UPDATE(X){}bU cZ(vm::S&cs)B {A cs.advance_ext(0x208,2);}};cH<by T>bf FullMERKLE_UPDATE:K<dn>{Ref<T> da;Ref<T> to_proof;FullMERKLE_UPDATE(B T&bd):K("MERKLE_UPDATE",dn(bd.bd)){}N bg(G&ctx,S&cs,int u=0){(cs.advance(520));da=Ref<T>(cE);da.bo().bc(ctx,cs.br(),u+1);to_proof=Ref<T>(cE);to_proof.bo().bc(ctx,cs.br(),u+1);}N bF(G&ctx,U&cb,int u=0){cb.be(da.bo().bb(ctx,u+1));cb.be(to_proof.bo().bb(ctx,u+1));cw=0;}N bu(G&ctx,S&cs,int u=0){da=Ref<T>(cE);da.bo().bc(ctx,cs.br(),u+1);to_proof=Ref<T>(cE);to_proof.bo().bc(ctx,cs.br(),u+1);}Ref<Cell> bb(G&ctx,int u=0){A U::create_merkle_update(
da.bo().bb(ctx,u+1),to_proof.bo().bb(ctx,u+1));}};bf cF:K<ShardState_aux>{cF():K("ShardStateUnsplit_aux"){}S ics;Ref<Cell> r1,r2;N bg(G&ctx,S&cs,int u=0){r1=cs.br();ics=cs;}N bm(G&ctx,U&cb,int u=0){cb.bJ(ics);if(!r1.is_null()){cb.be(r1);}}};bf MyMcStateExtra:block::gen::McStateExtra{bU cZ(vm::S&cs)B {A cs.advance(16)&&block::gen::ShardHashes().cZ(cs)&&cs.advance_ext(0x100,2)&&tCC.cZ(cs);}};bf FullMcStateExtra:K<MyMcStateExtra>{FullMcStateExtra():K("McStateExtra",MyMcStateExtra()){}};B block::gen::ShardStateUnsplit_aux tSSUa;B block::gen::RefT tRMSE{MyMcStateExtra()};B block::gen::Maybe tMRMSE{tRMSE};bf MyShardStateUnsplit:block::gen::cV{bU cZ(vm::S&cs)B{A cs.advance_ext(0x169,3)&&tMRMSE.cZ(cs);}};bf bO:K<MyShardStateUnsplit>{block::gen::cV::Record bn;cF aux;FullOutMsgQueueInfo omqi;S ics;bO():K("cV"){}N bg(G&ctx,S&cs,int u=0){omqi.bc(ctx,cs.br(),u+1);ics=cs;cs.advance(cs.bh());cs.advance_refs(cs.size_refs());}N bm(G&ctx,U&cb,int u=0){cb.be(omqi.bb(ctx,u+1));cb.bJ(ics);}};bf FullShardState:K<ShardState>,td::CntObject{int tag=-1;bO cK;bO left;bO right;FullShardState():K("ShardState",ShardState()){}N bg(G&ctx,S&cs,int u=0){tag=bd.get_tag(cs);(tag==bd.cK||tag==bd.split_state);if(tag==bd.cK){cK.bA(ctx,cs,u+1);}else{cs.advance(32);left.bc(ctx,cs.br(),u+1);right.bc(ctx,cs.br(),u+1);}}N bm(G&ctx,U&cb,int u=0){(tag==bd.cK||tag==bd.split_state);if(tag==bd.cK){cK.bY(ctx,cb,u+1);}else{cb.cn(bd.split_state,32).be(left.bb(ctx,u+1)).be(right.bb(ctx,u+1));}}};bf cu;bf FullOutMsgDescr:K<OutMsgDescr>{cJ<FullOutMsg,ci> x{256,tOM,tCC};FullOutMsgDescr():K("OutMsgDescr",OutMsgDescr()){}N bg(G&ctx,S&cs,int u=0){x.bA(ctx,cs,u+1);}N bF(G&ctx,U&cb,int u=0){x.cY(ctx,cb,u+1);}N bu(G&ctx,S&cs,int u=0){x.ca(ctx,cs,u+1);}N bm(G&ctx,U&cb,int u=0){x.bY(ctx,cb,u+1);}};bf cu:K<ImportFees>,ct<ImportFees>{cu():K("ImportFees",tIF),ct(tIF){}};bf FullInMsgDescr:K<InMsgDescr>{cJ<FullInMsg,cu> x{256,tIM,tIF};FullInMsgDescr():K("InMsgDescr",InMsgDescr()){}N bg(G&ctx,S&cs,int u=0){x.bA(ctx,cs,u+1);}N bF(G&ctx,U&cb,int u=0){x.cY(ctx,cb,u+1);}N bu(G&ctx,S&cs,int u=0){x.ca(ctx,cs,u+1);}N bm(G&ctx,U&cb,int u=0){x.bY(ctx,cb,u+1);}};B block::gen::McBlockExtra tMBE{};B block::gen::RefT tRMBE{tMBE};B block::gen::Maybe tMRMBE(tRMBE);bf FullBlockExtra:K<block::gen::BlockExtra>{block::gen::BlockExtra::Record bn;FullInMsgDescr cS;FullOutMsgDescr cG;FullBlockExtra():K("BlockExtra",block::gen::t_BlockExtra){}N bg(G&ctx,S&cs,int u=0){(bd.unpack(cs,bn));cS.bc(ctx,bn.cS,u+1);cG.bc(ctx,bn.cG,u+1);}N bF(G&ctx,U&cb,int u=0){cb.be(cS.bG(ctx,u+1)).be(cG.bG(ctx,u+1)).be(bn.account_blocks).store_bits(bn.rand_seed.cbits(),256).store_bits(bn.created_by.cbits(),256);(tMRMBE.store_from(cb,bn.custom));}N bu(G&ctx,S&cs,int u=0){cS.bx(ctx,cs.br(),u+1);cG.bx(ctx,cs.br(),u+1);(cs.fetch_ref_to(bn.account_blocks)&&cs.fetch_bits_to(bn.rand_seed.bits(),256)&&cs.fetch_bits_to(bn.created_by.bits(),256)&&tMRMBE.fetch_to(cs,bn.custom));}N bm(G&ctx,U&cb,int u=0){cb.cn(0x4a33f6fd,32).be(cS.bb(ctx,u+1)).be(cG.bb(ctx,u+1)).be(bn.account_blocks).store_bits(bn.rand_seed.cbits(),256).store_bits(bn.created_by.cbits(),256);(tMRMBE.store_from(cb,bn.custom));}};bf FullBlock:K<block::gen::Block>{block::gen::Block::Record bn;FullMERKLE_UPDATE<FullShardState> cx;FullBlockExtra bH;FullBlock():K("Block",block::gen::t_Block),cx(FullShardState()){}N bg(G&ctx,S&cs,int u=0){(bd.unpack(cs,bn));cx.bc(ctx,bn.cx,u+1);bH.bc(ctx,bn.bH,u+1,cE);}N bF(G&ctx,U&cb,int u=0){cb.cn(bn.global_id,32).be(bn.info).be(bn.value_flow).be(cx.bG(ctx,u+1)).be(bH.bG(ctx,u+1));}N bu(G&ctx,S&cs,int u=0){(
cs.fetch_int_to(32,bn.global_id)&&cs.fetch_ref_to(bn.info)&&cs.fetch_ref_to(bn.value_flow));cx.bx(ctx,cs.br(),u+1);bH.bx(ctx,cs.br(),u+1,cE);}N bm(G&ctx,U&cb,int u=0){cb.cn(0x11ef55aa,32).cn(bn.global_id,32).be(bn.info).be(bn.value_flow).be(cx.bb(ctx,u+1)).be(bH.bb(ctx,u+1));}};by NullStream:bB ostream{by NullBuffer:bB streambuf{bB:int overflow(int c){A c;}}m_nb;bB:NullStream():ostream(&m_nb){}};
#define Y else
#define O if
#define Q for
using I=int;using namespace std;namespace LQ{typedef uint8_t U8;typedef uint16_t U16;typedef uint32_t U32;typedef uint64_t U64;N E(B V* msg=0){throw runtime_error(msg);}bf cg{bM I get()= 0;bM I read(V* buf,I n);bM ~cg(){}};bf bw{bM N put(I c)= 0;bM N bo(B V* buf,I n);bM ~bw(){}};I toU16(B V* p);cH<by T>
by cC{T *bV;bi n;I offset;N operator=(B cC&);cC(B cC&);bB:cC(bi sz=0,I ex=0):bV(0),n(0),offset(0){bk(sz,ex);}
N bk(bi sz,I ex=0);~cC(){bk(0);}
bi bh()B{A n;}
I isize()B{A I(n);}
T&operator[](bi i){A bV[i];}
T&operator()(bi i){A bV[i&(n-1)];}};cH<by T>
N cC<T>::bk(bi sz,I ex){ba(ex>0){O(sz>sz*2)E(0);sz*=2,--ex;}O(n>0){::free((V*)bV-offset);}n=0;offset=0;O(sz==0)A;n=sz;B bi nb=128+n*sizeof(T);O(nb<=128||(nb-128)/sizeof(T)!=n)n=0,E(0);bV=(T*)::calloc(nb,1);O(!bV)n=0,E(0);offset=64-(((V*)bV-(V*)0)&63);bV=(T*)((V*)bV+offset);}typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE}CompType;by dg;by ZP{bB:ZP();~ZP();N clear();N inith();N initp();double memory();N run(U32 input);I read(cg* in2);bU bo(bw* out2,bU pp);I step(U32 input,I mode);bw* output;U32 H(I i){A h(i);}
N flush();N outc(I ch){O(ch<0||(outbuf[bufptr]=ch,++bufptr==outbuf.isize()))flush();}cC<U8> D;I cj;I bP,bq;
cC<U8> m;cC<U32> h,r;cC<V> outbuf;I bufptr;U32 a,b,c,d;I f,pc,rcode_size;U8* rcode;N cP(I hbits,I mbits);I execute();N div(U32 x){O(x)a/=x;Y a=0;}
N mod(U32 x){O(x)a%=x;Y a=0;}
N swap(U32&x){a^=x;x^=a;a^=x;}
N swap(U8&x){a^=x;x^=a;a^=x;}
N err(){E(0);}};bf dp{bi z;bi cxt;bi a,b,c;cC<U32> cm;cC<U8> ht;cC<U16> a16;N cP(){z=cxt=a=b=c=0;cm.bk(0);ht.bk(0);a16.bk(0);}dp(){cP();}};by StateTable{bB:U8 ns[1024];I bC(I s,I y){A ns[s*4+y];}I cminit(I s){A((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1);}StateTable();};by P{bB:P(ZP&);~P();N cP();I predict();N update(I y);I stat(I);bU dq(){A z.D[6]!=0;}
I c8;I dr;I p[256];U32 h[256];ZP&z;dp comp[256];bU initTables;I predict0();N update0(I y);I dt2k[256];I dt[1024];U16 squasht[4096];short stretcht[32768];StateTable st;U8* pcode;I pcode_size;N train(dp&cr,I y){U32&pn=cr.cm(cr.cxt);U32 count=pn&0x3ff;I E=y*32767-(cr.cm(cr.cxt)>>17);pn+=(E*dt[count]&-1024)+(count<cr.z);}I squash(I x){A squasht[x+2048];}I stretch(I x){A stretcht[x];}I clamp2k(I x){O(x<-2048)A -2048;Y O(x>2047)A 2047;Y A x;}I clamp512k(I x){O(x<-(1<<19))A -(1<<19);Y O(x>=(1<<19))A(1<<19)-1;Y A x;}bi find(cC<U8>&ht,I sizebits,U32 cxt){I chk=cxt>>sizebits&255;bi h0=(cxt*16)&(ht.bh()-16);O(ht[h0]==chk)A h0;bi h1=h0^16;O(ht[h1]==chk)A h1;bi h2=h0^32;O(ht[h2]==chk)A h2;O(ht[h0+1]<=ht[h1+1]&&ht[h0+1]<=ht[h2+1])
A memset(&ht[h0],0,16),ht[h0]=chk,h0;Y O(ht[h1+1]<ht[h2+1])
A memset(&ht[h1],0,16),ht[h1]=chk,h1;Y
A memset(&ht[h2],0,16),ht[h2]=chk,h2;}I assemble_p();};by dg:bB cg{bB:cg* in;dg(ZP&z);I decompress();I cZ();N cP();I stat(I x){A pr.stat(x);}
I get(){O(bR==bz){bR=0;bz=in ? in->read(&buf[0],cv):0;}A bR<bz ? U8(buf[bR++]):-1;}I buffered(){A bz-bR;}
U32 low,cT;U32 bW;U32 bR,bz;P pr;enum{cv=1<<16};cC<V> buf;I decode(I p);};by dh{I bj;I ce;I ph,pm;bB:ZP z;dh():bj(0),ce(0),ph(0),pm(0){}N cP(I h,I m);I bo(I c);I getState()B{A bj;}
N setOutput(bw* out){z.output=out;}};by D{bB:D():z(),dec(z),pp(),bj(BLOCK),cy(FIRSTSEG){}N setInput(cg* in){dec.in=in;}
bU findBlock(double* memptr=0);bU findFilename(bw* = 0);N readComment(bw* = 0);N setOutput(bw* out){pp.setOutput(out);}
bU decompress(I n=-1);N readSegmentEnd(V* cc=0);I stat(I x){A dec.stat(x);}
I buffered(){A dec.buffered();}
ZP z;dg dec;dh pp;enum{BLOCK,FILENAME,COMMENT,DATA,SEGEND}bj;enum{FIRSTSEG,SEG,SKIP}cy;};N decompress(cg* in,bw* out);by Encoder{bB:Encoder(ZP&z,I bh=0):
out(0),low(1),cT(0xFFFFFFFF),pr(z){}N cP();N compress(I c);I stat(I x){A pr.stat(x);}
bw* out;
U32 low,cT;P pr;cC<V> buf;N encode(I y,I p);};by ck{bB:ck(B V* in,I* bl,ZP&hz,ZP&pz,bw* out2);
B V* in;I* bl;ZP&hz;ZP&pz;bw* out2;I line;I bj;typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,JT=39,JF=47,JMP=63,LJ=255,POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON}CompType;N SE(B V* msg,B V* expected=0);N bC();bU matchToken(B V* tok);I rtoken(I low,I cT);I rtoken(B V* list[]);N rtoken(B V* s);I compile_comp(ZP&z);by Stack{LQ::cC<U16> s;bi top;bB:Stack(I n):s(n),top(0){}N push(B U16&x){O(top>=s.bh())E(0);s[top++]=x;}U16 pop(){O(top<=0)E(0);A s[--top];}};Stack ds,do_stack;};by C{bB:C():enc(z),in(0),bj(INIT){}N setOutput(bw* out){enc.out=out;}
N startBlock(B V* config,I* bl,bw* pcomp_cmd=0);N startSegment(B V* cL=0,B V* comment=0);N setInput(cg* i){in=i;}
N postProcess(B V* cW=0,I len=0);bU compress(I n=-1);N endSegment(B V* cc=0);I stat(I x){A enc.stat(x);}
ZP z,pz;Encoder enc;cg* in;enum{INIT,BLOCK1,SEG1,BLOCK2,SEG2}bj;};by SB:bB LQ::cg,bB LQ::bw{J V* p;bi al;bi bz;bi bR;bi bL;B bi cP;N reserve(bi a){O(a<=al)A;J V* q=0;O(a>0)q=(J V*)(p ? realloc(p,a):malloc(a));O(a>0&&!q)E(0);p=q;al=a;}N lengthen(bi n){O(bz+n>bL||bz+n<bz)E(0);O(bz+n<=al)A;bi a=al;ba(bz+n>=a)a=a*2+cP;reserve(a);}N operator=(B SB&);SB(B SB&);bB:J V* bV(){A p;}
SB(bi n=0):
p(0),al(0),bz(0),bR(0),bL(bi(-1)),cP(n>128?n:128){}~SB(){O(p)free(p);}
bi bh()B{A bz;}
N put(I c){lengthen(1);p[bz++]=c;}N bo(B V* buf,I n){O(n<1)A;lengthen(n);O(buf)memcpy(p+bz,buf,n);bz+=n;}I get(){A bR<bz ? p[bR++]:-1;}I read(V* buf,I n){O(bR+n>bz)n=bz-bR;O(n>0&&buf)memcpy(buf,p+bR,n);bR+=n;A n;}B V* c_str()B{A(B V*)p;}
N bk(bi i){bz=i;O(bR>bz)bR=bz;}N swap(SB&s){::swap(p,s.p);::swap(al,s.al);::swap(bz,s.bz);::swap(bR,s.bR);::swap(bL,s.bL);}};N compress(cg* in,bw* out,B V* ME,B V* cL=0,B V* comment=0,bU dosha1=cE);N compressBlock(SB* in,bw* out,B V* ME,B V* cL=0,B V* comment=0,bU dosha1=cE);I toU16(B V* p){A(p[0]&255)+256*(p[1]&255);}I cg::read(V* buf,I n){I i=0,c;ba(i<n&&(c=get())>=0)
buf[i++]=c;A i;}N bw::bo(B V* buf,I n){Q(I i=0;i<n;++i)
put(U8(buf[i]));}N allocx(U8*&p,I&n,I newsize){p=0;n=0;}B I compsize[256]={0,2,3,2,3,4,6,6,3,5};bK B U8 sns[1024]={1,2,0,0,3,5,1,0,4,6,0,1,7,9,2,0,8,11,1,1,8,11,1,1,10,12,0,2,13,15,3,0,14,17,2,1,14,17,2,1,16,19,1,2,16,19,1,2,18,20,0,3,21,23,4,0,22,25,3,1,22,25,3,1,24,27,2,2,24,27,2,2,26,29,1,3,26,29,1,3,28,30,0,4,31,33,5,0,32,35,4,1,32,35,4,1,34,37,3,2,34,37,3,2,36,39,2,3,36,39,2,3,38,41,1,4,38,41,1,4,40,42,0,5,43,33,6,0,44,47,5,1,44,47,5,1,46,49,4,2,46,49,4,2,48,51,3,3,48,51,3,3,50,53,2,4,50,53,2,4,52,55,1,5,52,55,1,5,40,56,0,6,57,45,7,0,58,47,6,1,58,47,6,1,60,63,5,2,60,63,5,2,62,65,4,3,62,65,4,3,64,67,3,4,64,67,3,4,66,69,2,5,66,69,2,5,52,71,1,6,52,71,1,6,54,72,0,7,73,59,8,0,74,61,7,1,74,61,7,1,76,63,6,2,76,63,6,2,78,81,5,3,78,81,5,3,80,83,4,4,80,83,4,4,82,85,3,5,82,85,3,5,66,87,2,6,66,87,2,6,68,89,1,7,68,89,1,7,70,90,0,8,91,59,9,0,92,77,8,1,92,77,8,1,94,79,7,2,94,79,7,2,96,81,6,3,96,81,6,3,98,101,5,4,98,101,5,4,100,103,4,5,100,103,4,5,82,105,3,6,82,105,3,6,84,107,2,7,84,107,2,7,86,109,1,8,86,109,1,8,70,110,0,9,111,59,10,0,112,77,9,1,112,77,9,1,114,97,8,2,114,97,8,2,116,99,7,3,116,99,7,3,62,101,6,4,62,101,6,4,80,83,5,5,80,83,5,5,100,67,4,6,100,67,4,6,102,119,3,7,102,119,3,7,104,121,2,8,104,121,2,8,86,123,1,9,86,123,1,9,70,124,0,10,125,59,11,0,126,77,10,1,126,77,10,1,128,97,9,2,128,97,9,2,60,63,8,3,60,63,8,3,66,69,3,8,66,69,3,8,104,131,2,9,104,131,2,9,86,133,1,10,86,133,1,10,70,134,0,11,135,59,12,0,136,77,11,1,136,77,11,1,138,97,10,2,138,97,10,2,104,141,2,10,104,141,2,10,86,143,1,11,86,143,1,11,70,144,0,12,145,59,13,0,146,77,12,1,146,77,12,1,148,97,11,2,148,97,11,2,104,151,2,11,104,151,2,11,86,153,1,12,86,153,1,12,70,154,0,13,155,59,14,0,156,77,13,1,156,77,13,1,158,97,12,2,158,97,12,2,104,161,2,12,104,161,2,12,86,163,1,13,86,163,1,13,70,164,0,14,165,59,15,0,166,77,14,1,166,77,14,1,168,97,13,2,168,97,13,2,104,171,2,13,104,171,2,13,86,173,1,14,86,173,1,14,70,174,0,15,175,59,16,0,176,77,15,1,176,77,15,1,178,97,14,2,178,97,14,2,104,181,2,14,104,181,2,14,86,183,1,15,86,183,1,15,70,184,0,16,185,59,17,0,186,77,16,1,186,77,16,1,74,97,15,2,74,97,15,2,104,89,2,15,104,89,2,15,86,187,1,16,86,187,1,16,70,188,0,17,189,59,18,0,190,77,17,1,86,191,1,17,70,192,0,18,193,59,19,0,194,77,18,1,86,195,1,18,70,196,0,19,193,59,20,0,197,77,19,1,86,198,1,19,70,196,0,20,199,77,20,1,86,200,1,20,201,77,21,1,86,202,1,21,203,77,22,1,86,204,1,22,205,77,23,1,86,206,1,23,207,77,24,1,86,208,1,24,209,77,25,1,86,210,1,25,211,77,26,1,86,212,1,26,213,77,27,1,86,214,1,27,215,77,28,1,86,216,1,28,217,77,29,1,86,218,1,29,219,77,30,1,86,220,1,30,221,77,31,1,86,222,1,31,223,77,32,1,86,224,1,32,225,77,33,1,86,226,1,33,227,77,34,1,86,228,1,34,229,77,35,1,86,230,1,35,231,77,36,1,86,232,1,36,233,77,37,1,86,234,1,37,235,77,38,1,86,236,1,38,237,77,39,1,86,238,1,39,239,77,40,1,86,240,1,40,241,77,41,1,86,242,1,41,243,77,42,1,86,244,1,42,245,77,43,1,86,246,1,43,247,77,44,1,86,248,1,44,249,77,45,1,86,250,1,45,251,77,46,1,86,252,1,46,253,77,47,1,86,254,1,47,253,77,48,1,86,254,1,48,0,0,0,0
};StateTable::StateTable(){memcpy(ns,sns,sizeof(ns));}bU ZP::bo(bw* out2,bU pp){O(D.bh()<=6)A false;O(!pp){Q(I i=0;i<cj;++i)
out2->put(D[i]);}Y{out2->put((bq-bP)&255);out2->put((bq-bP)>>8);}Q(I i=bP;i<bq;++i)
out2->put(D[i]);A cE;}I ZP::read(cg* in2){I ce=in2->get();ce+=in2->get()*256;D.bk(ce+300);cj=bP=bq=0;D[cj++]=ce&255;D[cj++]=ce>>8;ba(cj<7)D[cj++]=in2->get();I n=D[cj-1];Q(I i=0;i<n;++i){I bd=in2->get();O(bd<0||bd>255)E(0);D[cj++]=bd;I bh=compsize[bd];O(bh<1)E(0);O(cj+bh>ce)E(0);Q(I j=1;j<bh;++j)
D[cj++]=in2->get();}O((D[cj++]=in2->get())!=0)E(0);bP=bq=cj+128;O(bq>ce+129)E(0);ba(bq<ce+129){I op=in2->get();O(op==-1)E(0);D[bq++]=op;}O((D[bq++]=in2->get())!=0)E(0);allocx(rcode,rcode_size,0);A cj+bq-bP;}N ZP::clear(){cj=bP=bq=0;a=b=c=d=f=pc=0;D.bk(0);h.bk(0);m.bk(0);r.bk(0);allocx(rcode,rcode_size,0);}ZP::ZP(){output=0;rcode=0;rcode_size=0;clear();outbuf.bk(1<<14);bufptr=0;}ZP::~ZP(){allocx(rcode,rcode_size,0);}N ZP::inith(){cP(D[2],D[3]);}N ZP::initp(){cP(D[4],D[5]);}N ZP::flush(){O(output)output->bo(&outbuf[0],bufptr);bufptr=0;}bK double pow2(I x){double r=1;Q(;x>0;x--)r+=r;A r;}double ZP::memory(){double mem=pow2(D[2]+2)+pow2(D[3])
+pow2(D[4]+2)+pow2(D[5])
+D.bh();I cp=7;Q(I i=0;i<D[6];++i){double bh=pow2(D[cp+1]);switch(D[cp]){o CM:mem+=4*bh;g;o ICM:mem+=64*bh+1024;g;o MATCH:mem+=4*bh+pow2(D[cp+2]);g;o MIX2:mem+=2*bh;g;o MIX:mem+=4*bh*D[cp+3];g;o ISSE:mem+=64*bh+2048;g;o SSE:mem+=128*bh;g;}cp+=compsize[D[cp]];}A mem;}N ZP::cP(I hbits,I mbits){O(hbits>32)E(0);O(mbits>32)E(0);h.bk(1,hbits);m.bk(1,mbits);r.bk(256);a=b=c=d=pc=f=0;}I ZP::execute(){switch(D[pc++]){o 0:err();g;o 1:++a;g;o 2:--a;g;o 3:a=~a;g;o 4:a=0;g;o 7:a=r[D[pc++]];g;o 8:swap(b);g;o 9:++b;g;o 10:--b;g;o 11:b=~b;g;o 12:b=0;g;o 15:b=r[D[pc++]];g;o 16:swap(c);g;o 17:++c;g;o 18:--c;g;o 19:c=~c;g;o 20:c=0;g;o 23:c=r[D[pc++]];g;o 24:swap(d);g;o 25:++d;g;o 26:--d;g;o 27:d=~d;g;o 28:d=0;g;o 31:d=r[D[pc++]];g;o 32:swap(m(b));g;o 33:++m(b);g;o 34:--m(b);g;o 35:m(b)= ~m(b);g;o 36:m(b)= 0;g;o 39:O(f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;o 40:swap(m(c));g;o 41:++m(c);g;o 42:--m(c);g;o 43:m(c)= ~m(c);g;o 44:m(c)= 0;g;o 47:O(!f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;o 48:swap(h(d));g;o 49:++h(d);g;o 50:--h(d);g;o 51:h(d)= ~h(d);g;o 52:h(d)= 0;g;o 55:r[D[pc++]]=a;g;o 56:A 0 ;o 57:outc(a&255);g;o 59:a=(a+m(b)+512)*773;g;o 60:h(d)= (h(d)+a+512)*773;g;o 63:pc+=((D[pc]+128)&255)-127;g;o 64:g;o 65:a=b;g;o 66:a=c;g;o 67:a=d;g;o 68:a=m(b);g;o 69:a=m(c);g;o 70:a=h(d);g;o 71:a=D[pc++];g;o 72:b=a;g;o 73:g;o 74:b=c;g;o 75:b=d;g;o 76:b=m(b);g;o 77:b=m(c);g;o 78:b=h(d);g;o 79:b=D[pc++];g;o 80:c=a;g;o 81:c=b;g;o 82:g;o 83:c=d;g;o 84:c=m(b);g;o 85:c=m(c);g;o 86:c=h(d);g;o 87:c=D[pc++];g;o 88:d=a;g;o 89:d=b;g;o 90:d=c;g;o 91:g;o 92:d=m(b);g;o 93:d=m(c);g;o 94:d=h(d);g;o 95:d=D[pc++];g;o 96:m(b)=a;g;o 97:m(b)=b;g;o 98:m(b)=c;g;o 99:m(b)=d;g;o 100:g;o 101:m(b)=m(c);g;o 102:m(b)=h(d);g;o 103:m(b)=D[pc++];g;o 104:m(c)=a;g;o 105:m(c)=b;g;o 106:m(c)=c;g;o 107:m(c)=d;g;o 108:m(c)=m(b);g;o 109:g;o 110:m(c)=h(d);g;o 111:m(c)=D[pc++];g;o 112:h(d)=a;g;o 113:h(d)=b;g;o 114:h(d)=c;g;o 115:h(d)=d;g;o 116:h(d)=m(b);g;o 117:h(d)=m(c);g;o 118:g;o 119:h(d)=D[pc++];g;o 128:a+=a;g;o 129:a+=b;g;o 130:a+=c;g;o 131:a+=d;g;o 132:a+=m(b);g;o 133:a+=m(c);g;o 134:a+=h(d);g;o 135:a+=D[pc++];g;o 136:a-=a;g;o 137:a-=b;g;o 138:a-=c;g;o 139:a-=d;g;o 140:a-=m(b);g;o 141:a-=m(c);g;o 142:a-=h(d);g;o 143:a-=D[pc++];g;o 144:a*=a;g;o 145:a*=b;g;o 146:a*=c;g;o 147:a*=d;g;o 148:a*=m(b);g;o 149:a*=m(c);g;o 150:a*=h(d);g;o 151:a*=D[pc++];g;o 152:div(a);g;o 153:div(b);g;o 154:div(c);g;o 155:div(d);g;o 156:div(m(b));g;o 157:div(m(c));g;o 158:div(h(d));g;o 159:div(D[pc++]);g;o 160:mod(a);g;o 161:mod(b);g;o 162:mod(c);g;o 163:mod(d);g;o 164:mod(m(b));g;o 165:mod(m(c));g;o 166:mod(h(d));g;o 167:mod(D[pc++]);g;o 168:a&=a;g;o 169:a&=b;g;o 170:a&=c;g;o 171:a&=d;g;o 172:a&=m(b);g;o 173:a&=m(c);g;o 174:a&=h(d);g;o 175:a&=D[pc++];g;o 176:a&=~a;g;o 177:a&=~b;g;o 178:a&=~c;g;o 179:a&=~d;g;o 180:a&=~m(b);g;o 181:a&=~m(c);g;o 182:a&=~h(d);g;o 183:a&=~D[pc++];g;o 184:a|=a;g;o 185:a|=b;g;o 186:a|=c;g;o 187:a|=d;g;o 188:a|=m(b);g;o 189:a|=m(c);g;o 190:a|=h(d);g;o 191:a|=D[pc++];g;o 192:a^=a;g;o 193:a^=b;g;o 194:a^=c;g;o 195:a^=d;g;o 196:a^=m(b);g;o 197:a^=m(c);g;o 198:a^=h(d);g;o 199:a^=D[pc++];g;o 200:a<<=(a&31);g;o 201:a<<=(b&31);g;o 202:a<<=(c&31);g;o 203:a<<=(d&31);g;o 204:a<<=(m(b)&31);g;o 205:a<<=(m(c)&31);g;o 206:a<<=(h(d)&31);g;o 207:a<<=(D[pc++]&31);g;o 208:a>>=(a&31);g;o 209:a>>=(b&31);g;o 210:a>>=(c&31);g;o 211:a>>=(d&31);g;o 212:a>>=(m(b)&31);g;o 213:a>>=(m(c)&31);g;o 214:a>>=(h(d)&31);g;o 215:a>>=(D[pc++]&31);g;o 216:f=1;g;o 217:f=(a==b);g;o 218:f=(a==c);g;o 219:f=(a==d);g;o 220:f=(a==U32(m(b)));g;o 221:f=(a==U32(m(c)));g;o 222:f=(a==h(d));g;o 223:f=(a==U32(D[pc++]));g;o 224:f=0;g;o 225:f=(a<b);g;o 226:f=(a<c);g;o 227:f=(a<d);g;o 228:f=(a<U32(m(b)));g;o 229:f=(a<U32(m(c)));g;o 230:f=(a<h(d));g;o 231:f=(a<U32(D[pc++]));g;o 232:f=0;g;o 233:f=(a>b);g;o 234:f=(a>c);g;o 235:f=(a>d);g;o 236:f=(a>U32(m(b)));g;o 237:f=(a>U32(m(c)));g;o 238:f=(a>h(d));g;o 239:f=(a>U32(D[pc++]));g;o 255:O((pc=bP+D[pc]+256*D[pc+1])>=bq)err();g;default:err();}A 1;}bK B U8 stdt[712]={64,128,128,128,128,128,127,128,127,128,127,127,127,127,126,126,126,126,126,125,125,124,125,124,123,123,123,123,122,122,121,121,120,120,119,119,118,118,118,116,117,115,116,114,114,113,113,112,112,111,110,110,109,108,108,107,106,106,105,104,104,102,103,101,101,100,99,98,98,97,96,96,94,94,94,92,92,91,90,89,89,88,87,86,86,84,84,84,82,82,81,80,79,79,78,77,76,76,75,74,73,73,72,71,70,70,69,68,67,67,66,65,65,64,63,62,62,61,61,59,59,59,57,58,56,56,55,54,54,53,52,52,51,51,50,49,49,48,48,47,47,45,46,44,45,43,43,43,42,41,41,40,40,40,39,38,38,37,37,36,36,36,35,34,34,34,33,32,33,32,31,31,30,31,29,30,28,29,28,28,27,27,27,26,26,25,26,24,25,24,24,23,23,23,23,22,22,21,22,21,20,21,20,19,20,19,19,19,18,18,18,18,17,17,17,17,16,16,16,16,15,15,15,15,15,14,14,14,14,13,14,13,13,13,12,13,12,12,12,11,12,11,11,11,11,11,10,11,10,10,10,10,9,10,9,9,9,9,9,8,9,8,9,8,8,8,7,8,8,7,7,8,7,7,7,6,7,7,6,6,7,6,6,6,6,6,6,5,6,5,6,5,5,5,5,5,5,5,5,5,4,5,4,5,4,4,5,4,4,4,4,4,4,3,4,4,3,4,4,3,3,4,3,3,3,4,3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,2,2,3,2,2,3,2,2,2,2,3,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,1,2,1,2,2,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0
};P::P(ZP&zr):
c8(1),dr(1),z(zr){pcode=0;pcode_size=0;initTables=false;}P::~P(){allocx(pcode,pcode_size,0);}N P::cP(){allocx(pcode,pcode_size,0);z.inith();O(!initTables&&dq()){initTables=cE;dt2k[0]=0;Q(I i=1;i<256;++i){dt2k[i]=2048/i;}Q(I i=0;i<1024;++i){dt[i]=(1<<17)/(i*2+3)*2;}memset(squasht,0,(1376+7)*2);Q(I i=1376+7;i<1376+1344;++i){squasht[i]=static_cast<U16>(32768.0 / (1+std::exp((i - 2048)* (-1.0 / 64))));}Q(I i=2720;i<4096;++i)squasht[i]=32767;I k=16384;Q(I i=0;i<712;++i)
Q(I j=stdt[i];j>0;--j)
stretcht[k++]=i;Q(I i=0;i<16384;++i)
stretcht[i]=-stretcht[32767-i];}Q(I i=0;i<256;++i)h[i]=p[i]=0;Q(I i=0;i<256;++i)
comp[i].cP();I n=z.D[6];B U8* cp=&z.D[7];Q(I i=0;i<n;++i){dp&cr=comp[i];switch(cp[0]){o CONS:
p[i]=(cp[1]-128)*4;g;o CM:
O(cp[1]>32)E(0);cr.cm.bk(1,cp[1]);cr.z=cp[2]*4;Q(bi j=0;j<cr.cm.bh();++j)
cr.cm[j]=0x80000000;g;o ICM:
O(cp[1]>26)E(0);cr.z=1023;cr.cm.bk(256);cr.ht.bk(64,cp[1]);Q(bi j=0;j<cr.cm.bh();++j)
cr.cm[j]=st.cminit(j);g;o MATCH:
O(cp[1]>32||cp[2]>32)E(0);cr.cm.bk(1,cp[1]);cr.ht.bk(1,cp[2]);cr.ht(0)=1;g;o AVG:
O(cp[1]>=i)E(0);O(cp[2]>=i)E(0);g;o MIX2:
O(cp[1]>32)E(0);O(cp[3]>=i)E(0);O(cp[2]>=i)E(0);cr.c=(bi(1)<<cp[1]);cr.a16.bk(1,cp[1]);Q(bi j=0;j<cr.a16.bh();++j)
cr.a16[j]=32768;g;o MIX:{O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]<1||cp[3]>i-cp[2])E(0);I m=cp[3];cr.c=(bi(1)<<cp[1]);cr.cm.bk(m,cp[1]);Q(bi j=0;j<cr.cm.bh();++j)
cr.cm[j]=65536/m;g;}o ISSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);cr.ht.bk(64,cp[1]);cr.cm.bk(512);Q(I j=0;j<256;++j){cr.cm[j*2]=1<<15;cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);}g;o SSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]>cp[4]*4)E(0);cr.cm.bk(32,cp[1]);cr.z=cp[4]*4;Q(bi j=0;j<cr.cm.bh();++j)
cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];g;default:E(0);}cp+=compsize[*cp];}}I P::predict0(){I n=z.D[6];B U8* cp=&z.D[7];Q(I i=0;i<n;++i){dp&cr=comp[i];switch(cp[0]){o CONS:
g;o CM:
cr.cxt=h[i]^dr;p[i]=stretch(cr.cm(cr.cxt)>>17);g;o ICM:
O(c8==1||(c8&0xf0)==16)cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(dr&15)];p[i]=stretch(cr.cm(cr.cxt)>>8);g;o MATCH:
O(cr.a==0)p[i]=0;Y{cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1;p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);}g;o AVG:
p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;g;o MIX2:{cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));I w=cr.a16[cr.cxt];p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;}g;o MIX:{I m=cp[3];cr.cxt=h[i]+(c8&cp[5]);cr.cxt=(cr.cxt&(cr.c-1))*m;I* wt=(I*)&cr.cm[cr.cxt];p[i]=0;Q(I j=0;j<m;++j)
p[i]+=(wt[j]>>8)*p[cp[2]+j];p[i]=clamp2k(p[i]>>8);}g;o ISSE:{O(c8==1||(c8&0xf0)==16)
cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(dr&15)];I *wt=(I*)&cr.cm[cr.cxt*2];p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);}g;o SSE:{cr.cxt=(h[i]+c8)*32;I pq=p[cp[2]]+992;O(pq<0)pq=0;O(pq>1983)pq=1983;I wt=pq&63;pq>>=6;cr.cxt+=pq;p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)+(cr.cm(cr.cxt+1)>>10)*wt)>>13);cr.cxt+=wt>>5;}g;default:
E(0);}cp+=compsize[cp[0]];}A squash(p[n-1]);}N P::update0(I y){B U8* cp=&z.D[7];I n=z.D[6];Q(I i=0;i<n;++i){dp&cr=comp[i];switch(cp[0]){o CONS:
g;o CM:
train(cr,y);g;o ICM:{cr.ht[cr.c+(dr&15)]=st.bC(cr.ht[cr.c+(dr&15)],y);U32&pn=cr.cm(cr.cxt);pn+=I(y*32767-(pn>>8))>>2;}g;o MATCH:{O(I(cr.c)!=y)cr.a=0;cr.ht(cr.z)+=cr.ht(cr.z)+y;O(++cr.cxt==8){cr.cxt=0;++cr.z;cr.z&=(1<<cp[2])-1;O(cr.a==0){cr.b=cr.z-cr.cm(h[i]);O(cr.b&(cr.ht.bh()-1))
ba(cr.a<255
&&cr.ht(cr.z-cr.a-1)==cr.ht(cr.z-cr.a-cr.b-1))
++cr.a;}Y cr.a+=cr.a<255;cr.cm(h[i])=cr.z;}}g;o AVG:
g;o MIX2:{I err=(y*32767-squash(p[i]))*cp[4]>>5;I w=cr.a16[cr.cxt];w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;O(w<0)w=0;O(w>65535)w=65535;cr.a16[cr.cxt]=w;}g;o MIX:{I m=cp[3];I err=(y*32767-squash(p[i]))*cp[4]>>4;I* wt=(I*)&cr.cm[cr.cxt];Q(I j=0;j<m;++j)
wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));}g;o ISSE:{I err=y*32767-squash(p[i]);I *wt=(I*)&cr.cm[cr.cxt*2];wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));wt[1]=clamp512k(wt[1]+((err+16)>>5));cr.ht[cr.c+(dr&15)]=st.bC(cr.cxt,y);}g;o SSE:
train(cr,y);g;default:
assert(0);}cp+=compsize[cp[0]];}c8+=c8+y;O(c8>=256){z.run(c8-256);dr=1;c8=1;Q(I i=0;i<n;++i)h[i]=z.H(i);}Y O(c8>=16&&c8<32)
dr=(dr&0xf)<<5|y<<4|1;Y
dr=(dr&0x1f0)|(((dr&0xf)*2+y)&0xf);}dg::dg(ZP&z):
in(0),low(1),cT(0xFFFFFFFF),bW(0),bR(0),bz(0),pr(z),buf(cv){}N dg::cP(){pr.cP();O(pr.dq())low=1,cT=0xFFFFFFFF,bW=0;Y low=cT=bW=0;}I dg::decode(I p){O(bW<low||bW>cT)E(0);U32 mid=low+U32(((cT-low)*U64(U32(p)))>>16);I y;O(bW<=mid)y=1,cT=mid;Y y=0,low=mid+1;ba((cT^low)<0x1000000){cT=cT<<8|255;low=low<<8;low+=(low==0);I c=get();O(c<0)E(0);bW=bW<<8|c;}A y;}I dg::decompress(){O(pr.dq()){O(bW==0){Q(I i=0;i<4;++i)
bW=bW<<8|get();}O(decode(0)){O(bW!=0)E(0);A -1;}Y{I c=1;ba(c<256){I p=pr.predict()*2+1;c+=c+decode(p);pr.update(c&1);}A c-256;}}Y{O(bW==0){Q(I i=0;i<4;++i)bW=bW<<8|get();O(bW==0)A -1;}--bW;A get();}}I dg::cZ(){I c=-1;O(pr.dq()){ba(bW==0)
bW=get();ba(bW&&(c=get())>=0)
bW=bW<<8|c;ba((c=get())==0);A c;}Y{O(bW==0)
Q(I i=0;i<4&&(c=get())>=0;++i)bW=bW<<8|c;ba(bW>0){ba(bW>0){--bW;O(get()<0)A E("skipped to EOF"),-1;}Q(I i=0;i<4&&(c=get())>=0;++i)bW=bW<<8|c;}O(c>=0)c=get();A c;}}N dh::cP(I h,I m){bj=ce=0;ph=h;pm=m;z.clear();}I dh::bo(I c){switch (bj){o 0:
O(c<0)E(0);bj=c+1;O(bj>2)E(0);O(bj==1)z.clear();g;o 1:
z.outc(c);g;o 2:
O(c<0)E(0);ce=c;bj=3;g;o 3:
O(c<0)E(0);ce+=c*256;O(ce<1)E(0);z.D.bk(ce+300);z.cj=8;z.bP=z.bq=z.cj+128;z.D[4]=ph;z.D[5]=pm;bj=4;g;o 4:
O(c<0)E(0);z.D[z.bq++]=c;O(z.bq-z.bP==ce){ce=z.cj-2+z.bq-z.bP;z.D[0]=ce&255;z.D[1]=ce>>8;z.initp();bj=5;}g;o 5:
z.run(c);O(c<0)z.flush();g;}A bj;}bU D::findBlock(double* memptr){I c=dec.get();O(c!=1&&c!=2)E(0);z.read(&dec);O(c==1&&z.D.isize()>6&&z.D[6]==0)
E(0);O(memptr)*memptr=z.memory();bj=FILENAME;cy=FIRSTSEG;A cE;}bU D::findFilename(bw* cL){bj=COMMENT;A cE;}N D::readComment(bw* comment){bj=DATA;}bU D::decompress(I n){O(cy==SKIP)E(0);O(cy==FIRSTSEG){dec.cP();pp.cP(z.D[4],z.D[5]);cy=SEG;}ba((pp.getState()&3)!=1)
pp.bo(dec.decompress());ba(n){I c=dec.decompress();pp.bo(c);O(c==-1){bj=SEGEND;A false;}O(n>0)--n;}A cE;}N D::readSegmentEnd(V* cc){I c=0;O(bj==DATA){c=dec.cZ();cy=SKIP;}Y O(bj==SEGEND)
c=dec.get();bj=FILENAME;O(c==254){O(cc)cc[0]=0;}Y O(c==253){O(cc)cc[0]=1;Q(I i=1;i<=20;++i){c=dec.get();O(cc)cc[i]=c;}}Y
E(0);}N decompress(cg* in,bw* out){D d;d.setInput(in);d.setOutput(out);if(d.findBlock()){d.decompress();}}N Encoder::cP(){low=1;cT=0xFFFFFFFF;pr.cP();O(!pr.dq())low=0,buf.bk(1<<16);}N Encoder::encode(I y,I p){U32 mid=low+U32(((cT-low)*U64(U32(p)))>>16);O(y)cT=mid;Y low=mid+1;ba((cT^low)<0x1000000){out->put(cT>>24);cT=cT<<8|255;low=low<<8;low+=(low==0);}}N Encoder::compress(I c){O(pr.dq()){O(c==-1)
encode(1,0);Y{encode(0,0);Q(I i=7;i>=0;--i){I p=pr.predict()*2+1;I y=c>>i&1;encode(y,p);pr.update(y);}}}Y{E(0);}}B V* compname[256]={"","B","cm","icm","match","avg","mix2","mix","isse","sse",0};B V* opcodelist[272]={"E","a++","a--","a!","a=0","","","a=r","b<>a","b++","b--","b!","b=0","","","b=r","c<>a","c++","c--","c!","c=0","","","c=r","d<>a","d++","d--","d!","d=0","","","d=r","*b<>a","*b++","*b--","*b!","*b=0","","","jt","*c<>a","*c++","*c--","*c!","*c=0","","","jf","*d<>a","*d++","*d--","*d!","*d=0","","","r=a","halt","out","","hash","hashd","","","jmp","a=a","a=b","a=c","a=d","a=*b","a=*c","a=*d","a=","b=a","b=b","b=c","b=d","b=*b","b=*c","b=*d","b=","c=a","c=b","c=c","c=d","c=*b","c=*c","c=*d","c=","d=a","d=b","d=c","d=d","d=*b","d=*c","d=*d","d=","*b=a","*b=b","*b=c","*b=d","*b=*b","*b=*c","*b=*d","*b=","*c=a","*c=b","*c=c","*c=d","*c=*b","*c=*c","*c=*d","*c=","*d=a","*d=b","*d=c","*d=d","*d=*b","*d=*c","*d=*d","*d=","","","","","","","","","a+=a","a+=b","a+=c","a+=d","a+=*b","a+=*c","a+=*d","a+=","a-=a","a-=b","a-=c","a-=d","a-=*b","a-=*c","a-=*d","a-=","a*=a","a*=b","a*=c","a*=d","a*=*b","a*=*c","a*=*d","a*=","a/=a","a/=b","a/=c","a/=d","a/=*b","a/=*c","a/=*d","a/=","a%=a","a%=b","a%=c","a%=d","a%=*b","a%=*c","a%=*d","a%=","a&=a","a&=b","a&=c","a&=d","a&=*b","a&=*c","a&=*d","a&=","a&~a","a&~b","a&~c","a&~d","a&~*b","a&~*c","a&~*d","a&~","a|=a","a|=b","a|=c","a|=d","a|=*b","a|=*c","a|=*d","a|=","a^=a","a^=b","a^=c","a^=d","a^=*b","a^=*c","a^=*d","a^=","a<<=a","a<<=b","a<<=c","a<<=d","a<<=*b","a<<=*c","a<<=*d","a<<=","a>>=a","a>>=b","a>>=c","a>>=d","a>>=*b","a>>=*c","a>>=*d","a>>=","a==a","a==b","a==c","a==d","a==*b","a==*c","a==*d","a==","a<a","a<b","a<c","a<d","a<*b","a<*c","a<*d","a<","a>a","a>b","a>c","a>d","a>*b","a>*c","a>*d","a>","","","","","","","","","","","","","","","","lj","post","cW","end","O","ifnot","Y","endif","do","ba","until","Qever","ifl","ifnotl","Yl",";",0};N ck::bC(){Q(;*in;++in){O(*in=='\n')++line;O(*in=='(')bj+=1+(bj<0);Y O(bj>0&&*in==')')--bj;Y O(bj<0&&*in<=' ')bj=0;Y O(bj==0&&*in>' '){bj=-1;g;}}O(!*in)E(0);}I tolower(I c){A(c>='A'&&c<='Z')? c+'a'-'A':c;}
bU ck::matchToken(B V* word){B V* a=in;Q(;(*a>' '&&*a!='('&&*word);++a,++word)
O(tolower(*a)!=tolower(*word))A false;A !*word&&(*a<=' '||*a=='(');}N ck::SE(B V* msg,B V* expected){E(0);}I ck::rtoken(B V* list[]){bC();Q(I i=0;list[i];++i)
O(matchToken(list[i]))
A i;SE(0);A -1;}N ck::rtoken(B V* s){bC();O(!matchToken(s))SE(0,s);}I ck::rtoken(I low,I cT){bC();I r=0;O(in[0]=='$'&&in[1]>='1'&&in[1]<='9'){O(in[2]=='+')r=atoi(in+3);O(bl)r+=bl[in[1]-'1'];}Y O(in[0]=='-'||(in[0]>='0'&&in[0]<='9'))r=atoi(in);Y SE(0);O(r<low)SE(0);O(r>cT)SE(0);A r;}I ck::compile_comp(ZP&z){I op=0;B I comp_begin=z.bq;ba(cE){op=rtoken(opcodelist);O(op==POST||op==PCOMP||op==END)g;I bT=-1;I operand2=-1;O(op==IF){op=JF;bT=0;ds.push(z.bq+1);}Y O(op==IFNOT){op=JT;bT=0;ds.push(z.bq+1);}Y O(op==IFL||op==IFNOTL){O(op==IFL)z.D[z.bq++]=(JT);O(op==IFNOTL)z.D[z.bq++]=(JF);z.D[z.bq++]=(3);op=LJ;bT=operand2=0;ds.push(z.bq+1);}Y O(op==ELSE||op==ELSEL){O(op==ELSE)op=JMP,bT=0;O(op==ELSEL)op=LJ,bT=operand2=0;I a=ds.pop();O(z.D[a-1]!=LJ){I j=z.bq-a+1+(op==LJ);O(j>127)SE("O too big,try IFL,IFNOTL");z.D[a]=j;}Y{I j=z.bq-comp_begin+2+(op==LJ);z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}ds.push(z.bq+1);}Y O(op==ENDIF){I a=ds.pop();I j=z.bq-a-1;O(z.D[a-1]!=LJ){O(j>127)SE("O too big,try IFL,IFNOTL,ELSEL\n");z.D[a]=j;}Y{j=z.bq-comp_begin;z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}}Y O(op==DO){do_stack.push(z.bq);}Y O(op==WHILE||op==UNTIL||op==QEVER){I a=do_stack.pop();I j=a-z.bq-2;O(j>=-127){O(op==WHILE)op=JT;O(op==UNTIL)op=JF;O(op==QEVER)op=JMP;bT=j&255;}Y{j=a-comp_begin;O(op==WHILE){z.D[z.bq++]=(JF);z.D[z.bq++]=(3);}O(op==UNTIL){z.D[z.bq++]=(JT);z.D[z.bq++]=(3);}op=LJ;bT=j&255;operand2=j>>8;}}Y O((op&7)==7){O(op==LJ){bT=rtoken(0,65535);operand2=bT>>8;bT&=255;}Y O(op==JT||op==JF||op==JMP){bT=rtoken(-128,127);bT&=255;}Y
bT=rtoken(0,255);}O(op>=0&&op<=255)
z.D[z.bq++]=(op);O(bT>=0)
z.D[z.bq++]=(bT);O(operand2>=0)
z.D[z.bq++]=(operand2);O(z.bq>=z.D.isize()-130||z.bq-z.bP+z.cj-2>65535)
SE("program too big");}z.D[z.bq++]=(0);A op;}ck::ck(B V* in_,I* args_,ZP&hz_,ZP&pz_,bw* out2_):in(in_),bl(args_),hz(hz_),pz(pz_),out2(out2_),ds(1000),do_stack(1000){line=1;bj=0;hz.clear();pz.clear();hz.D.bk(68000);rtoken("comp");hz.D[2]=rtoken(0,255);hz.D[3]=rtoken(0,255);hz.D[4]=rtoken(0,255);hz.D[5]=rtoken(0,255);B I n=hz.D[6]=rtoken(0,255);hz.cj=7;Q(I i=0;i<n;++i){rtoken(i,i);CompType bd=CompType(rtoken(compname));hz.D[hz.cj++]=bd;I clen=LQ::compsize[bd&255];O(clen<1||clen>10)SE("invalid component");Q(I j=1;j<clen;++j)
hz.D[hz.cj++]=rtoken(0,255);}hz.D[hz.cj++];hz.bP=hz.bq=hz.cj+128;rtoken("bQ");I op=compile_comp(hz);I ce=hz.cj-2+hz.bq-hz.bP;hz.D[0]=ce&255;hz.D[1]=ce>>8;O(op==POST){rtoken(0,0);rtoken("end");}Y O(op==PCOMP){pz.D.bk(68000);pz.D[4]=hz.D[4];pz.D[5]=hz.D[5];pz.cj=8;pz.bP=pz.bq=pz.cj+128;bC();ba(*in&&*in!=';'){O(out2)
out2->put(*in);++in;}O(*in)++in;op=compile_comp(pz);I len=pz.cj-2+pz.bq-pz.bP;pz.D[0]=len&255;pz.D[1]=len>>8;O(op!=END)
SE("expected END");}Y O(op!=END)
SE("expected END or POST 0 END or PCOMP cmd ;... END");}by MemoryReader:bB cg{B V* p;bB:MemoryReader(B V* p_):p(p_){}I get(){A *p++&255;}};N C::startBlock(B V* config,I* bl,bw* pcomp_cmd){ck(config,bl,z,pz,pcomp_cmd);enc.out->put(1+(z.D[6]==0));z.bo(enc.out,false);bj=BLOCK1;}N C::startSegment(B V* cL,B V* comment){O(bj==BLOCK1)bj=SEG1;O(bj==BLOCK2)bj=SEG2;}N C::postProcess(B V* cW,I len){O(bj==SEG2)A;enc.cP();O(!cW){len=pz.bq-pz.bP;O(len>0){cW=(B V*)&pz.D[pz.bP];}}Y O(len==0){len=toU16(cW);cW+=2;}O(len>0){enc.compress(1);enc.compress(len&255);enc.compress((len>>8)&255);Q(I i=0;i<len;++i)
enc.compress(cW[i]&255);}Y
enc.compress(0);bj=SEG2;}bU C::compress(I n){O(bj==SEG1)
postProcess();B I cv=1<<14;V buf[cv];ba(n){I nbuf=cv;O(n>=0&&n<nbuf)nbuf=n;I nr=in->read(buf,nbuf);O(nr<0||nr>cv||nr>nbuf)E(0);O(nr<=0)A false;O(n>=0)n-=nr;Q(I i=0;i<nr;++i){I ch=U8(buf[i]);enc.compress(ch);}}A cE;}N C::endSegment(B V* cc){O(bj==SEG1)
postProcess();enc.compress(-1);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(254);bj=BLOCK2;}N compress(cg* in,bw* out,B V* ME,B V* cL,B V* comment,bU dosha1){I bs=4;O(ME&&ME[0]&&ME[1]>='0'&&ME[1]<='9'){bs=ME[1]-'0';O(ME[2]>='0'&&ME[2]<='9')bs=bs*10+ME[2]-'0';O(bs>11)bs=11;}bs=(0x100000<<bs)-4096;SB sb(bs);sb.bo(0,bs);I n=0;ba(in&&(n=in->read((V*)sb.bV(),bs))>0){sb.bk(n);compressBlock(&sb,out,ME,cL,comment,dosha1);cL=0;comment=0;sb.bk(0);}}I P::predict(){A predict0();}N P::update(I y){update0(y);}N ZP::run(U32 input){pc=bP;a=input;ba(execute());}
#define AS 256
#define BUCKET_A_SIZE AS
#define BUCKET_B_SIZE (AS*AS)
#define SSIT 8
#define SSB 1024
#define SS_MISORT_STACKSIZE 16
#define SS_SMERGE_STACKSIZE 32
#define TR_INSERTIONSORT_THRESHOLD 8
#define TR_STACKSIZE 64
#define bE(_a,_b)do{t=(_a);(_a)= (_b);(_b)= t;}ba(0)
#define MIN(_a,_b)(((_a)< (_b))? (_a):(_b))
#define MAX(_a,_b)(((_a)> (_b))? (_a):(_b))
#define SP(_a,_b,_c,_d)\
do{\
assert(bt<bS);\
bD[bt].a=(_a),bD[bt].b=(_b),\
bD[bt].c=(_c),bD[bt++].d=(_d);\
}ba(0)
#define S5(_a,_b,_c,_d,_e)\
do{\
assert(bt<bS);\
bD[bt].a=(_a),bD[bt].b=(_b),\
bD[bt].c=(_c),bD[bt].d=(_d),bD[bt++].e=(_e);\
}ba(0)
#define STACK_POP(_a,_b,_c,_d)\
do{\
assert(0<=bt);\
O(bt==0){A;}\
(_a)= bD[--bt].a,(_b)= bD[bt].b,\
(_c)= bD[bt].c,(_d)= bD[bt].d;\
}ba(0)
#define cq(_a,_b,_c,_d,_e)\
do{\
assert(0<=bt);\
O(bt==0){A;}\
(_a)= bD[--bt].a,(_b)= bD[bt].b,\
(_c)= bD[bt].c,(_d)= bD[bt].d,(_e)= bD[bt].e;\
}ba(0)
#define db(_c0)cM[(_c0)]
#define cz(_c0,_c1)(cA[((_c1)<< 8)| (_c0)])
#define BB(_c0,_c1)(cA[((_c0)<< 8)| (_c1)])
#define R(v)v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
bK B I cB[256]={-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,R(4),R(5),R(5),R(6),R(6),R(6),R(6),R(7),R(7),R(7),R(7),R(7),R(7),R(7),R(7),};
#undef R
I
di(I n){A(n&0xff00)?
8+cB[(n >> 8)&0xff]:0+cB[(n >> 0)&0xff];}bK I dqq_table[256] ={0};I
ss_isqrt(I x){I y,e;O(x >= (SSB * SSB)){A SSB;}
e=(x&0xffff0000)?
((x&0xff000000)?
24+cB[(x >> 24)&0xff]:16+cB[(x >> 16)&0xff]):
((x&0x0000ff00)?
8+cB[(x >> 8)&0xff]:0+cB[(x >> 0)&0xff]);O(dqq_table[255] != 255){Q(I i=0;i<256;++i){dqq_table[i]=static_cast<I>(16 * sqrt(i));}}O(e >= 16){y=dqq_table[x >> ((e - 6)- (e&1))] << ((e >> 1)- 7);O(e >= 24){y=(y+1+x / y)>> 1;}
y=(y+1+x / y)>> 1;}Y O(e >= 8){y=(dqq_table[x >> ((e - 6)- (e&1))] >> (7 - (e >> 1)))+ 1;}Y{A dqq_table[x] >> 4;}A(x<(y * y))? y - 1:y;}I
cd(B J V *T,B I *p1,B I *p2,I W){B J V *U1,*U2,*U1n,*U2n;Q(U1=T+W+*p1,U2=T+W+*p2,U1n=T+*(p1+1)+ 2,U2n=T+*(p2+1)+ 2;(U1<U1n)&&(U2<U2n)&&(*U1==*U2);++U1,++U2){}A U1<U1n ?
(U2<U2n ? *U1 - *U2:1):
(U2<U2n ? -1:0);}bK
N
ss_insertionsort(B J V *T,B I *PA,I *F,I *L,I W){I *i,*j;I t;I r;Q(i=L - 2;F<=i;--i){Q(t=*i,j=i+1;0<(r=cd(T,PA+t,PA+*j,W));){do{*(j - 1)= *j;}ba((++j<L)&&(*j<0));O(L<=j){g;}}O(r==0){*j=~*j;}
*(j - 1)= t;}}N
ss_fixdown(B J V *Td,B I *PA,I *SA,I i,I bh){I j,k;I v;I c,d,e;Q(v=SA[i],c=Td[PA[v]];(j=2 * i+1)< bh;SA[i]=SA[k],i=k){d=Td[PA[SA[k=j++]]];O(d<(e=Td[PA[SA[j]]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bK
N
ss_heapsort(B J V *Td,B I *PA,I *SA,I bh){I i,m;I t;m=bh;O((bh % 2)== 0){m--;O(Td[PA[SA[m / 2]]]<Td[PA[SA[m]]]){bE(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){ss_fixdown(Td,PA,SA,i,m);}
O((bh % 2)== 0){bE(SA[0],SA[m]);ss_fixdown(Td,PA,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];ss_fixdown(Td,PA,SA,0,i);SA[i]=t;}}I *
ss_median3(B J V *Td,B I *PA,I *v1,I *v2,I *v3){I *t;O(Td[PA[*v1]]>Td[PA[*v2]]){bE(v1,v2);}
O(Td[PA[*v2]]>Td[PA[*v3]]){O(Td[PA[*v1]]>Td[PA[*v3]]){A v1;}
Y{A v3;}}A v2;}I *
ss_median5(B J V *Td,B I *PA,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(Td[PA[*v2]]>Td[PA[*v3]]){bE(v2,v3);}
O(Td[PA[*v4]]>Td[PA[*v5]]){bE(v4,v5);}
O(Td[PA[*v2]]>Td[PA[*v4]]){bE(v2,v4);bE(v3,v5);}
O(Td[PA[*v1]]>Td[PA[*v3]]){bE(v1,v3);}
O(Td[PA[*v1]]>Td[PA[*v4]]){bE(v1,v4);bE(v3,v5);}
O(Td[PA[*v3]]>Td[PA[*v4]]){A v4;}
A v3;}I *
ss_pivot(B J V *Td,B I *PA,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){A ss_median3(Td,PA,F,M,L - 1);}Y{t >>= 2;A ss_median5(Td,PA,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=ss_median3(Td,PA,F,F+t,F+(t << 1));M=ss_median3(Td,PA,M - t,M,M+t);L=ss_median3(Td,PA,L - 1 - (t << 1),L - 1 - t,L - 1);A ss_median3(Td,PA,F,M,L);}I *
ss_partition(B I *PA,I *F,I *L,I W){I *a,*b;I t;Q(a=F - 1,b=L;;){Q(;(++a<b)&&((PA[*a]+W)>= (PA[*a+1]+1));){*a=~*a;}
Q(;(a<--b)&&((PA[*b]+W)< (PA[*b+1]+1));){}O(b<=a){g;}
t=~*b;*b=*a;*a=t;}O(F<a){*F=~*F;}
A a;}bK
N
ss_mIrosort(B J V *T,B I *PA,I *F,I *L,I W){
#define bS SS_MISORT_STACKSIZE
bf{I *a,*b,c;I d;}bD[bS];B J V *Td;I *a,*b,*c,*d,*e,*f;I s,t;I bt;I bL;I v,x=0;Q(bt=0,bL=di(L - F);;){O((L - F)<= SSIT){O(1<(L - F)){ss_insertionsort(T,PA,F,L,W);}
STACK_POP(F,L,W,bL);du;}Td=T+W;O(bL--==0){ss_heapsort(Td,PA,F,L - F);}
O(bL<0){Q(a=F+1,v=Td[PA[*F]];a<L;++a){O((x=Td[PA[*a]])!= v){O(1<(a - F)){g;}
v=x;F=a;}}O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,a,W);}O((a - F)<= (L - a)){O(1<(a - F)){SP(a,L,W,-1);L=a,W+=1,bL=di(a - F);}Y{F=a,bL=-1;}}Y{O(1<(L - a)){SP(F,a,W+1,di(a - F));F=a,bL=-1;}Y{L=a,W+=1,bL=di(a - F);}}du;}a=ss_pivot(Td,PA,F,L);v=Td[PA[*a]];bE(*F,*a);Q(b=F;(++b<L)&&((x=Td[PA[*b]])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=Td[PA[*b]])<= v);){O(x==v){bE(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=Td[PA[*c]])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bE(*c,*d);--d;}}}Q(;b<c;){bE(*b,*c);Q(;(++b<c)&&((x=Td[PA[*b]])<= v);){O(x==v){bE(*b,*a);++a;}}Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bE(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bE(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bE(*e,*f);}
a=F+(b - a),c=L - (d - c);b=(v<=Td[PA[*a] - 1])? a:ss_partition(PA,a,c,W);O((a - F)<= (L - c)){O((L - c)<= (c - b)){SP(b,c,W+1,di(c - b));SP(c,L,W,bL);L=a;}Y O((a - F)<= (c - b)){SP(c,L,W,bL);SP(b,c,W+1,di(c - b));L=a;}Y{SP(c,L,W,bL);SP(F,a,W,bL);F=b,L=c,W+=1,bL=di(c - b);}}Y{O((a - F)<= (c - b)){SP(b,c,W+1,di(c - b));SP(F,a,W,bL);F=c;}Y O((L - c)<= (c - b)){SP(F,a,W,bL);SP(b,c,W+1,di(c - b));F=c;}Y{SP(F,a,W,bL);SP(c,L,W,bL);F=b,L=c,W+=1,bL=di(c - b);}}}Y{bL+=1;O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,L,W);bL=di(L - F);}W+=1;}}
#undef bS
}N
ss_blockswap(I *a,I *b,I n){I t;Q(;0<n;--n,++a,++b){t=*a,*a=*b,*b=t;}}N
ss_rotate(I *F,I *M,I *L){I *a,*b,t;I l,r;l=M - F,r=L - M;Q(;(0<l)&&(0<r);){O(l==r){ss_blockswap(F,M,l);g;}
O(l<r){a=L - 1,b=M - 1;t=*a;do{*a--=*b,*b--=*a;O(b<F){*a=t;L=a;O((r-=l+1)<= l){g;}
a-=1,b=M - 1;t=*a;}}ba(1);}Y{a=F,b=M;t=*a;do{*a++=*b,*b++=*a;O(L<=b){*a=t;F=a+1;O((l-=r+1)<= r){g;}
a+=1,b=M;t=*a;}}ba(1);}}}bK
N
ss_inplacemerge(B J V *T,B I *PA,I *F,I *M,I *L,I W){B I *p;I *a,*b;I len,half;I q,r;I x;Q(;;){O(*(L - 1)< 0){x=1;p=PA+~*(L - 1);}
Y{x=0;p=PA+*(L - 1);}
Q(a=F,len=M - F,half=len >> 1,r=-1;0<len;len=half,half >>= 1){b=a+half;q=cd(T,PA+((0<=*b)? *b:~*b),p,W);O(q<0){a=b+1;half-=(len&1)^ 1;}Y{r=q;}}O(a<M){O(r==0){*a=~*a;}
ss_rotate(a,M,L);L-=M - a;M=a;O(F==M){g;}}--L;O(x != 0){ba(*--L<0){}}
O(M==L){g;}}}bK
N
ss_mergeQward(B J V *T,B I *PA,I *F,I *M,I *L,I *buf,I W){I *a,*b,*c,*cN;I t;I r;cN=buf+(M - F)- 1;ss_blockswap(buf,F,M - F);Q(t=*(a=F),b=buf,c=M;;){r=cd(T,PA+*b,PA+*c,W);O(r<0){do{*a++=*b;O(cN<=b){*cN=t;A;}
*b++=*a;}ba(*b<0);}Y O(r > 0){do{*a++=*c,*c++=*a;O(L<=c){ba(b<cN){*a++=*b,*b++=*a;}
*a=*b,*b=t;A;}}ba(*c<0);}Y{*c=~*c;do{*a++=*b;O(cN<=b){*cN=t;A;}
*b++=*a;}ba(*b<0);do{*a++=*c,*c++=*a;O(L<=c){ba(b<cN){*a++=*b,*b++=*a;}
*a=*b,*b=t;A;}}ba(*c<0);}}}bK
N
ss_mergebackward(B J V *T,B I *PA,I *F,I *M,I *L,I *buf,I W){B I *p1,*p2;I *a,*b,*c,*cN;I t;I r;I x;cN=buf+(L - M)- 1;ss_blockswap(buf,M,L - M);x=0;O(*cN<0){p1=PA+~*cN;x |= 1;}
Y{p1=PA+*cN;}
O(*(M - 1)< 0){p2=PA+~*(M - 1);x |= 2;}
Y{p2=PA+*(M - 1);}
Q(t=*(a=L - 1),b=cN,c=M - 1;;){r=cd(T,p1,p2,W);O(0<r){O(x&1){do{*a--=*b,*b--=*a;}ba(*b<0);x^=1;}
*a--=*b;O(b<=buf){*buf=t;g;}
*b--=*a;O(*b<0){p1=PA+~*b;x |= 1;}
Y{p1=PA+*b;}}Y O(r<0){O(x&2){do{*a--=*c,*c--=*a;}ba(*c<0);x^=2;}
*a--=*c,*c--=*a;O(c<F){ba(buf<b){*a--=*b,*b--=*a;}
*a=*b,*b=t;g;}O(*c<0){p2=PA+~*c;x |= 2;}
Y{p2=PA+*c;}}Y{O(x&1){do{*a--=*b,*b--=*a;}ba(*b<0);x^=1;}
*a--=~*b;O(b<=buf){*buf=t;g;}
*b--=*a;O(x&2){do{*a--=*c,*c--=*a;}ba(*c<0);x^=2;}
*a--=*c,*c--=*a;O(c<F){ba(buf<b){*a--=*b,*b--=*a;}
*a=*b,*b=t;g;}O(*b<0){p1=PA+~*b;x |= 1;}
Y{p1=PA+*b;}
O(*c<0){p2=PA+~*c;x |= 2;}
Y{p2=PA+*c;}}}}bK
N
ss_swapmerge(B J V *T,B I *PA,I *F,I *M,I *L,I *buf,I cl,I W){
#define bS SS_SMERGE_STACKSIZE
#define GETIDX(a)((0<=(a))? (a):(~(a)))
#define MERGE_(a,b,c)\
do{\
O(((c)&1)||\
(((c)&2)&&(cd(T,PA+GETIDX(*((a)- 1)),PA+*(a),W)== 0))){\
*(a)= ~*(a);\
}\
O(((c)&4)&&((cd(T,PA+GETIDX(*((b)- 1)),PA+*(b),W)== 0))){\
*(b)= ~*(b);\
}\
}ba(0)
bf{I *a,*b,*c;I d;}bD[bS];I *l,*r,*lm,*rm;I m,len,half;I bt;I dv,bC;Q(dv=0,bt=0;;){O((L - M)<= cl){O((F<M)&&(M<L)){ss_mergebackward(T,PA,F,M,L,buf,W);}MERGE_(F,L,dv);STACK_POP(F,M,L,dv);du;}O((M - F)<= cl){O(F<M){ss_mergeQward(T,PA,F,M,L,buf,W);}MERGE_(F,L,dv);STACK_POP(F,M,L,dv);du;}Q(m=0,len=MIN(M - F,L - M),half=len >> 1;0<len;len=half,half >>= 1){O(cd(T,PA+GETIDX(*(M+m+half)),PA+GETIDX(*(M - m - half - 1)),W)< 0){m+=half+1;half-=(len&1)^ 1;}}O(0<m){lm=M - m,rm=M+m;ss_blockswap(lm,M,m);l=r=M,bC=0;O(rm<L){O(*rm<0){*rm=~*rm;O(F<lm){Q(;*--l<0;){}bC |= 4;}
bC |= 1;}Y O(F<lm){Q(;*r<0;++r){}bC |= 2;}}O((l - F)<= (L - r)){SP(r,rm,L,(bC&3)| (dv&4));M=lm,L=l,dv=(dv&3)| (bC&4);}Y{O((bC&2)&&(r==M)){bC^=6;}
SP(F,lm,l,(dv&3)| (bC&4));F=r,M=rm,dv=(bC&3)| (dv&4);}}Y{O(cd(T,PA+GETIDX(*(M - 1)),PA+*M,W)== 0){*M=~*M;}MERGE_(F,L,dv);STACK_POP(F,M,L,dv);}}
#undef bS
}bK
N
sssort(B J V *T,B I *PA,I *F,I *L,I *buf,I cl,I W,I n,I Lsuffix){I *a;I *b,*M,*curbuf;I j,k,curbufsize,Z;I i;O(Lsuffix != 0){++F;}
O((cl<SSB)&&
(cl<(L - F))&&
(cl<(Z=ss_isqrt(L - F)))){O(SSB<Z){Z=SSB;}
buf=M=L - Z,cl=Z;}Y{M=L,Z=0;}Q(a=F,i=0;SSB<(M - a);a+=SSB,++i){ss_mIrosort(T,PA,a,a+SSB,W);curbufsize=L - (a+SSB);curbuf=a+SSB;O(curbufsize<=cl){curbufsize=cl,curbuf=buf;}
Q(b=a,k=SSB,j=i;j&1;b-=k,k <<= 1,j >>= 1){ss_swapmerge(T,PA,b - k,b,b+k,curbuf,curbufsize,W);}}ss_mIrosort(T,PA,a,M,W);Q(k=SSB;i != 0;k <<= 1,i >>= 1){O(i&1){ss_swapmerge(T,PA,a - k,a,M,buf,cl,W);a-=k;}}O(Z != 0){ss_mIrosort(T,PA,M,L,W);ss_inplacemerge(T,PA,F,M,L,W);}O(Lsuffix != 0){I PAi[2];PAi[0]=PA[*(F - 1)],PAi[1]=n - 2;Q(a=F,i=*(F - 1);(a<L)&&((*a<0)|| (0<cd(T,&(PAi[0]),PA+*a,W)));++a){*(a - 1)= *a;}*(a - 1)= i;}}I
dj(I n){A(n&0xffff0000)?
((n&0xff000000)?
24+cB[(n >> 24)&0xff]:16+cB[(n >> 16)&0xff]):
((n&0x0000ff00)?
8+cB[(n >> 8)&0xff]:0+cB[(n >> 0)&0xff]);}bK
N
tr_insertionsort(B I *X,I *F,I *L){I *a,*b;I t,r;Q(a=F+1;a<L;++a){Q(t=*a,b=a - 1;0 > (r=X[t] - X[*b]);){do{*(b+1)= *b;}ba((F<=--b)&&(*b<0));O(b<F){g;}}O(r==0){*b=~*b;}
*(b+1)= t;}}N
tr_fixdown(B I *X,I *SA,I i,I bh){I j,k;I v;I c,d,e;Q(v=SA[i],c=X[v];(j=2 * i+1)< bh;SA[i]=SA[k],i=k){d=X[SA[k=j++]];O(d<(e=X[SA[j]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bK
N
tr_heapsort(B I *X,I *SA,I bh){I i,m;I t;m=bh;O((bh % 2)== 0){m--;O(X[SA[m / 2]]<X[SA[m]]){bE(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){tr_fixdown(X,SA,i,m);}
O((bh % 2)== 0){bE(SA[0],SA[m]);tr_fixdown(X,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];tr_fixdown(X,SA,0,i);SA[i]=t;}}I *
tr_median3(B I *X,I *v1,I *v2,I *v3){I *t;O(X[*v1]>X[*v2]){bE(v1,v2);}
O(X[*v2]>X[*v3]){O(X[*v1]>X[*v3]){A v1;}
Y{A v3;}}A v2;}I *
tr_median5(B I *X,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(X[*v2]>X[*v3]){bE(v2,v3);}
O(X[*v4]>X[*v5]){bE(v4,v5);}
O(X[*v2]>X[*v4]){bE(v2,v4);bE(v3,v5);}
O(X[*v1]>X[*v3]){bE(v1,v3);}
O(X[*v1]>X[*v4]){bE(v1,v4);bE(v3,v5);}
O(X[*v3]>X[*v4]){A v4;}
A v3;}I *
tr_pivot(B I *X,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){A tr_median3(X,F,M,L - 1);}Y{t >>= 2;A tr_median5(X,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=tr_median3(X,F,F+t,F+(t << 1));M=tr_median3(X,M - t,M,M+t);L=tr_median3(X,L - 1 - (t << 1),L - 1 - t,L - 1);A tr_median3(X,F,M,L);}typedef bf _tr tr;bf _tr{I c;I r;I i;I n;};N
trbudget_init(tr *b,I c,I i){b->c=c;b->r=b->i=i;}I
trbudget_check(tr *b,I bh){O(bh<=b->r){b->r-=bh;A 1;}
O(b->c==0){b->n+=bh;A 0;}
b->r+=b->i - bh;b->c-=1;A 1;}N
tr_partition(B I *X,I *F,I *M,I *L,I **pa,I **pb,I v){I *a,*b,*c,*d,*e,*f;I t,s;I x=0;Q(b=M - 1;(++b<L)&&((x=X[*b])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=X[*b])<= v);){O(x==v){bE(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=X[*c])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bE(*c,*d);--d;}}}Q(;b<c;){bE(*b,*c);Q(;(++b<c)&&((x=X[*b])<= v);){O(x==v){bE(*b,*a);++a;}}Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bE(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bE(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bE(*e,*f);}
F+=(b - a),L-=(d - c);}*pa=F,*pb=L;}bK
N
tr_copy(I *ISA,B I *SA,I *F,I *a,I *b,I *L,I W){I *c,*d,*e;I s,v;v=b - SA - 1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - W))&&(ISA[s]==v)){*++d=s;ISA[s]=d - SA;}}Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - W))&&(ISA[s]==v)){*--d=s;ISA[s]=d - SA;}}}bK
N
tr_partialcopy(I *ISA,B I *SA,I *F,I *a,I *b,I *L,I W){I *c,*d,*e;I s,v;I rank,Lrank,newrank=-1;v=b - SA - 1;Lrank=-1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - W))&&(ISA[s]==v)){*++d=s;rank=ISA[s+W];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}Lrank=-1;Q(e=d;F<=e;--e){rank=ISA[*e];O(Lrank != rank){Lrank=rank;newrank=e - SA;}
O(newrank != rank){ISA[*e]=newrank;}}Lrank=-1;Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - W))&&(ISA[s]==v)){*--d=s;rank=ISA[s+W];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}}bK
N
tr_Irosort(I *ISA,B I *X,I *SA,I *F,I *L,tr *T){
#define bS TR_STACKSIZE
bf{B I *a;I *b,*c;I d,e;}bD[bS];I *a,*b,*c;I t;I v,x=0;I dm=X - ISA;I Z,bC;I bt,TR=-1;Q(bt=0,Z=dj(L - F);;){O(Z<0){O(Z==-1){tr_partition(X - dm,F,F,L,&a,&b,L - SA - 1);O(a<L){Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}}O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}O(1<(b - a)){S5(NULL,a,b,0,0);S5(X - dm,F,L,-2,TR);TR=bt - 2;}O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,dj(L - b),TR);L=a,Z=dj(a - F);}Y O(1<(L - b)){F=b,Z=dj(L - b);}Y{cq(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,dj(a - F),TR);F=b,Z=dj(L - b);}Y O(1<(a - F)){L=a,Z=dj(a - F);}Y{cq(X,F,L,Z,TR);}}}Y O(Z==-2){a=bD[--bt].b,b=bD[bt].c;O(bD[bt].d==0){tr_copy(ISA,SA,F,a,b,L,X - ISA);}Y{O(0<=TR){bD[TR].d=-1;}
tr_partialcopy(ISA,SA,F,a,b,L,X - ISA);}cq(X,F,L,Z,TR);}Y{O(0<=*F){a=F;do{ISA[*a]=a - SA;}ba((++a<L)&&(0<=*a));F=a;}O(F<L){a=F;do{*a=~*a;}ba(*++a<0);bC=(ISA[*a] != X[*a])? dj(a - F+1):-1;O(++a<L){Q(b=F,v=a - SA - 1;b<a;++b){ISA[*b]=v;}}
O(trbudget_check(T,a - F)){O((a - F)<= (L - a)){S5(X,a,L,-3,TR);X+=dm,L=a,Z=bC;}Y{O(1<(L - a)){S5(X+dm,F,a,bC,TR);F=a,Z=-3;}Y{X+=dm,L=a,Z=bC;}}}Y{O(0<=TR){bD[TR].d=-1;}
O(1<(L - a)){F=a,Z=-3;}Y{cq(X,F,L,Z,TR);}}}Y{cq(X,F,L,Z,TR);}}du;}O((L - F)<= TR_INSERTIONSORT_THRESHOLD){tr_insertionsort(X,F,L);Z=-3;du;}O(Z--==0){tr_heapsort(X,F,L - F);Q(a=L - 1;F<a;a=b){Q(x=X[*a],b=a - 1;(F<=b)&&(X[*b]==x);--b){*b=~*b;}}Z=-3;du;}a=tr_pivot(X,F,L);bE(*F,*a);v=X[*F];tr_partition(X,F,F+1,L,&a,&b,v);O((L - F)!= (b - a)){bC=(ISA[*a] != v)? dj(b - a):-1;Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}
O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}
O((1<(b - a))&&(trbudget_check(T,b - a))){O((a - F)<= (L - b)){O((L - b)<= (b - a)){O(1<(a - F)){S5(X+dm,a,b,bC,TR);S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){S5(X+dm,a,b,bC,TR);F=b;}Y{X+=dm,F=a,L=b,Z=bC;}}Y O((a - F)<= (b - a)){O(1<(a - F)){S5(X,b,L,Z,TR);S5(X+dm,a,b,bC,TR);L=a;}Y{S5(X,b,L,Z,TR);X+=dm,F=a,L=b,Z=bC;}}Y{S5(X,b,L,Z,TR);S5(X,F,a,Z,TR);X+=dm,F=a,L=b,Z=bC;}}Y{O((a - F)<= (b - a)){O(1<(L - b)){S5(X+dm,a,b,bC,TR);S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){S5(X+dm,a,b,bC,TR);L=a;}Y{X+=dm,F=a,L=b,Z=bC;}}Y O((L - b)<= (b - a)){O(1<(L - b)){S5(X,F,a,Z,TR);S5(X+dm,a,b,bC,TR);F=b;}Y{S5(X,F,a,Z,TR);X+=dm,F=a,L=b,Z=bC;}}Y{S5(X,F,a,Z,TR);S5(X,b,L,Z,TR);X+=dm,F=a,L=b,Z=bC;}}}Y{O((1<(b - a))&&(0<=TR)){bD[TR].d=-1;}
O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){F=b;}Y{cq(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){L=a;}Y{cq(X,F,L,Z,TR);}}}}Y{O(trbudget_check(T,L - F)){Z=dj(L - F),X+=dm;}Y{O(0<=TR){bD[TR].d=-1;}
cq(X,F,L,Z,TR);}}}
#undef bS
}bK
N
trsort(I *ISA,I *SA,I n,I W){I *X;I *F,*L;tr b;I t,cZ,unsorted;trbudget_init(&b,dj(n)* 2 / 3,n);Q(X=ISA+W;-n<*SA;X+=X - ISA){F=SA;cZ=0;unsorted=0;do{O((t=*F)< 0){F-=t;cZ+=t;}
Y{O(cZ != 0){*(F+cZ)= cZ;cZ=0;}
L=SA+ISA[t]+1;O(1<(L - F)){b.n=0;tr_Irosort(ISA,X,SA,F,L,&b);O(b.n != 0){unsorted+=b.n;}
Y{cZ=F - L;}}Y O((L - F)== 1){cZ=-1;}F=L;}}ba(F<(SA+n));O(cZ != 0){*(F+cZ)= cZ;}
O(unsorted==0){g;}}}bK
I
sort_typeBstar(B J V *T,I *SA,I *cM,I *cA,I n){I *PAb,*ISAb,*buf;I i,j,k,t,m,cl;I c0,c1;Q(i=0;i<BUCKET_A_SIZE;++i){cM[i]=0;}
Q(i=0;i<BUCKET_B_SIZE;++i){cA[i]=0;}
Q(i=n - 1,m=n,c0=T[n - 1];0<=i;){do{++db(c1=c0);}ba((0<=--i)&&((c0=T[i])>= c1));O(0<=i){++BB(c0,c1);SA[--m]=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){++cz(c0,c1);}}}m=n - m;Q(c0=0,i=0,j=0;c0<AS;++c0){t=i+db(c0);db(c0)= i+j;i=t+cz(c0,c0);Q(c1=c0+1;c1<AS;++c1){j+=BB(c0,c1);BB(c0,c1)= j;i+=cz(c0,c1);}}O(0<m){PAb=SA+n - m;ISAb=SA+m;Q(i=m - 2;0<=i;--i){t=PAb[i],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=i;}t=PAb[m - 1],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=m - 1;buf=SA+m,cl=n - (2 * m);Q(c0=AS - 2,j=m;0<j;--c0){Q(c1=AS - 1;c0<c1;j=i,--c1){i=BB(c0,c1);O(1<(j - i)){sssort(T,PAb,SA+i,SA+j,buf,cl,2,n,*(SA+i)== (m - 1));}}}Q(i=m - 1;0<=i;--i){O(0<=SA[i]){j=i;do{ISAb[SA[i]]=i;}ba((0<=--i)&&(0<=SA[i]));SA[i+1]=i - j;O(i<=0){g;}}j=i;do{ISAb[SA[i]=~SA[i]]=j;}ba(SA[--i]<0);ISAb[SA[i]]=j;}trsort(ISAb,SA,m,1);Q(i=n - 1,j=m,c0=T[n - 1];0<=i;){Q(--i,c1=c0;(0<=i)&&((c0=T[i])>= c1);--i,c1=c0){}O(0<=i){t=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){}SA[ISAb[--j]]=((t==0)|| (1<(t - i)))? t:~t;}}cz(AS - 1,AS - 1)= n;Q(c0=AS - 2,k=m - 1;0<=c0;--c0){i=db(c0+1)- 1;Q(c1=AS - 1;c0<c1;--c1){t=i - cz(c0,c1);cz(c0,c1)= i;Q(i=t,j=BB(c0,c1);j<=k;--i,--k){SA[i]=SA[k];}}BB(c0,c0+1)= i - cz(c0,c0)+ 1;cz(c0,c0)= i;}}A m;}bK
N
construct_SA(B J V *T,I *SA,I *cM,I *cA,I n,I m){I *i,*j,*k;I s;I c0,c1,c2;O(0<m){Q(c1=AS - 2;0<=c1;--c1){Q(i=SA+BB(c1,c1+1),j=SA+db(c1+1)- 1,k=NULL,c2=-1;i<=j;--j){O(0<(s=*j)){*j=~s;c0=T[--s];O((0<s)&&(T[s - 1]>c0)){s=~s;}
O(c0 != c2){O(0<=c2){cz(c2,c1)= k - SA;}
k=SA+cz(c2=c0,c1);}*k--=s;}Y{*j=~s;}}}}k=SA+db(c2=T[n - 1]);*k++=(T[n - 2]<c2)? ~(n - 1):(n - 1);Q(i=SA,j=SA+n;i<j;++i){O(0<(s=*i)){c0=T[--s];O((s==0)|| (T[s - 1]<c0)){s=~s;}
O(c0 != c2){db(c2)= k - SA;k=SA+db(c2=c0);}*k++=s;}Y{*i=~s;}}}I
divsufsort(B J V *T,I *SA,I n){I *cM,*cA;I m;I err=0;O((T==NULL)|| (SA==NULL)|| (n<0)){A -1;}
Y O(n==0){A 0;}
Y O(n==1){SA[0]=0;A 0;}
Y O(n==2){m=(T[0]<T[1]);SA[m ^ 1]=0,SA[m]=1;A 0;}
cM=(I *)malloc(BUCKET_A_SIZE * sizeof(I));cA=(I *)malloc(BUCKET_B_SIZE * sizeof(I));O((cM != NULL)&&(cA != NULL)){m=sort_typeBstar(T,SA,cM,cA,n);construct_SA(T,SA,cM,cA,n,m);}Y{err=-2;}free(cA);free(cM);A err;}std::bI cf(int64_t x,I n=1){std::bI r;Q(;x||n>0;x/=10,--n)r=std::bI(1,'0'+x%10)+r;A r;}by dc:bB LQ::cg{LQ::cC<J> ht;B J V* in;B I checkbits;B I bX;B J htsize;B J n;J i;B J m;B J m2;B J maxMatch;B J maxLiteral;B J cX;J h1,h2;B J bucket;B J shift1,shift2;B I mBoth;B J rb;J bits;J nbits;J bR,bz;J idx;B J* sa;J* isa;enum{cv=1<<14};J V buf[cv];N dk(J i,J&lit);N write_match(J len,J off);N fill();N putb(J x,I k){x&=(1<<k)-1;bits|=x<<nbits;nbits+=k;ba(nbits>7){buf[bz++]=bits,bits>>=8,nbits-=8;}}N flush(){O(nbits>0)buf[bz++]=bits;bits=nbits=0;}N put(I c){buf[bz++]=c;}bB:dc(SB&inbuf,I bl[],B J* sap=0);I get(){I c=-1;O(bR==bz)fill();O(bR<bz)c=buf[bR++];O(bR==bz)bR=bz=0;A c;}I read(V* p,I n);};I lg(J x){J r=0;O(x>=65536)r=16,x>>=16;O(x>=256)r+=8,x>>=8;O(x>=16)r+=4,x>>=4;A
"\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;}I nbits(J x){I r;Q(r=0;x;x>>=1)r+=x&1;A r;}I dc::read(V* p,I n){O(bR==bz)fill();I nr=n;O(nr>I(bz-bR))nr=bz-bR;O(nr)memcpy(p,buf+bR,nr);bR+=nr;O(bR==bz)bR=bz=0;A nr;}dc::dc(SB&inbuf,I bl[],B J* sap):
ht((bl[1]&3)==3 ? (inbuf.bh()+1)*!sap
:bl[5]-bl[0]<21 ? 1u<<bl[5]
:(inbuf.bh()*!sap)+(1u<<17<<bl[0])),in(inbuf.bV()),checkbits(bl[5]-bl[0]<21 ? 12-bl[0]:17+bl[0]),bX(bl[1]&3),htsize(ht.bh()),n(inbuf.bh()),i(0),m(bl[2]),m2(bl[3]),maxMatch(cv*3),maxLiteral(cv/4),cX(bl[6]),h1(0),h2(0),bucket((1<<bl[4])-1),shift1(m>0 ? (bl[5]-1)/m+1:1),shift2(m2>0 ? (bl[5]-1)/m2+1:0),mBoth(MAX(m,m2+cX)+4),rb(bl[0]>4 ? bl[0]-4:0),bits(0),nbits(0),bR(0),bz(0),idx(0),sa(0),isa(0){O((m<4&&bX==1)|| (m<1&&bX==2))
E(0);O(bl[5]-bl[0]>=21||bX==3){O(sap)
sa=sap;Y{sa=&ht[0];O(n>0)divsufsort((B J V*)in,(I*)sa,n);}O(bX<3){isa=&ht[n*(sap==0)];}}}N dc::fill(){O(bX==3){Q(;bz<cv&&i<n+5;++i){O(i==0)put(n>0 ? in[n-1]:255);Y O(i>n)put(idx&255),idx>>=8;Y O(sa[i-1]==0)idx=i,put(255);Y put(in[sa[i-1]-1]);}A;}J lit=0;B J mask=(1<<checkbits)-1;ba(i<n&&bz*2<cv){J dl=m-1;J bp=0;J blit=0;I bscore=0;O(isa){O(sa[isa[i&mask]]!=i)
Q(J j=0;j<n;++j)
O((sa[j]&~mask)==(i&~mask))
isa[sa[j]&mask]=j;Q(J h=0;h<=cX;++h){J q=isa[(h+i)&mask];O(sa[q]!=h+i)du;Q(I j=-1;j<=1;j+=2){Q(J k=1;k<=bucket;++k){J p;O(q+j*k<n&&(p=sa[q+j*k]-h)<i){J l,l1;Q(l=h;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);Q(l1=h;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-4*(lit==0&&l1>0)-11;Q(J a=0;a<h;++a)score=score*5/8;O(score>bscore)dl=l,bp=p,blit=l1,bscore=score;O(l<dl||l<m||l>255)g;}}}O(bscore<=0||dl<m)g;}}Y O(bX==1||m<=64){O(m2>0){Q(J k=0;k<=bucket;++k){J p=ht[h2^k];O(p&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+dl<=n&&in[p+dl-1]==in[i+dl-1]){J l;Q(l=cX;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);O(l>=m2+cX){I l1;Q(l1=cX;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-8*(lit==0&&l1>0)-11;O(score>bscore)dl=l,bp=p,blit=l1,bscore=score;}}}O(dl>=128)g;}}O(!m2||dl<m2){Q(J k=0;k<=bucket;++k){J p=ht[h1^k];O(p&&i+3<n&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+dl<=n&&in[p+dl-1]==in[i+dl-1]){J l;Q(l=0;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);I score=l*8-lg(i-p)-2*(lit>0)-11;O(score>bscore)dl=l,bp=p,blit=0,bscore=score;}}O(dl>=128)g;}}}B J off=i-bp;O(off>0&&bscore>0
&&dl-blit>=m+(bX==2)*((off>=(1<<16))+(off>=(1<<24)))){lit+=blit;dk(i+blit,lit);write_match(dl-blit,off);}Y{dl=1;++lit;}O(isa)
i+=dl;Y{ba(dl--){O(i+mBoth<n){J ih=((i*1234547)>>19)&bucket;B J p=(i<<checkbits)|(in[i+3]&mask);O(m2){ht[h2^ih]=p;h2=(((h2*9)<<shift2)
+(in[i+m2+cX]+1)*23456789u)&(htsize-1);}ht[h1^ih]=p;h1=(((h1*5)<<shift1)+(in[i+m]+1)*123456791u)&(htsize-1);}++i;}}O(lit>=maxLiteral)
dk(i,lit);}O(i==n){dk(n,lit);flush();}}N dc::dk(J i,J&lit){O(bX==1){O(lit<1)A;I ll=lg(lit);putb(0,2);--ll;ba(--ll>=0){putb(1,1);putb((lit>>ll)&1,1);}putb(0,1);ba(lit)putb(in[i-lit--],8);}Y{ba(lit>0){J lit1=lit;O(lit1>64)lit1=64;put(lit1-1);Q(J j=i-lit;j<i-lit+lit1;++j)put(in[j]);lit-=lit1;}}}N dc::write_match(J len,J off){O(bX==1){I ll=lg(len)-1;off+=(1<<rb)-1;I lo=lg(off)-1-rb;putb((lo+8)>>3,2);putb(lo&7,3);ba(--ll>=2){putb(1,1);putb((len>>ll)&1,1);}putb(0,1);putb(len&3,2);putb(off,rb);putb(off>>rb,lo);}Y{--off;ba(len>0){B J len1=len>m*2+63 ? m+63:len>m+63 ? len-m:len;O(off<(1<<16)){put(64+len1-m);put(off>>8);put(off);}Y O(off<(1<<24)){put(128+len1-m);put(off>>16);put(off>>8);put(off);}Y{put(192+len1-m);put(off>>24);put(off>>16);put(off>>8);put(off);}len-=len1;}}}std::bI makeConfig(B V* ME,I bl[]){B V bd=ME[0];bl[0]=0;bl[1]=0;bl[2]=0;bl[3]=0;bl[4]=0;bl[5]=0;bl[6]=0;bl[7]=0;bl[8]=0;O(isdigit(*++ME))bl[0]=0;Q(I i=0;i<9&&(isdigit(*ME)|| *ME==','||*ME=='.');){O(isdigit(*ME))
bl[i]=bl[i]*10+*ME-'0';Y O(++i<9)
bl[i]=0;++ME;}O(bd=='0')
A "comp 0 0 0 0 0 bQ end\n";std::bI hdr,cW;B I bX=bl[1]&3;B bU doe8=bl[1]>=4&&bl[1]<=7;O(bX==1){E(0);}Y O(bX==2){hdr="comp 9 16 0 $1+20 ";cW=
"cW lzpre c ;\n"
" a> 255 O\n";O(doe8){E(0);}cW+="b=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n";O(!doe8)cW+=" out ";cW+="a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++";O(!doe8)cW+=" out d-- a=d a> 0 ba endif endif endif halt end ";}Y O(bX==3){E(0);}Y O(bX==0){E(0);}Y
E(0);I dw=0;B I membits=bl[0]+20;I sb=5;std::bI comp;std::bI bQ="bQ\n"
"c-- *c=a a+= 255 d=a *d=c\n";O(bX==2){bQ+=
" a=r 1 a== 0 O\n"
" a= "+cf(111+57*doe8)+"\n"
" Y a== 1 O\n"
" a=*c r=a 2\n"
" a> 63 O a>>= 6 a++ a++\n"
" Y a++ a++ endif\n"
" Y\n"
" a--\n"
" endif endif\n"
" r=a 1\n";}ba(*ME&&dw<254){std::vector<I> v;v.push_back(*ME++);O(isdigit(*ME)){v.push_back(*ME++-'0');ba(isdigit(*ME)|| *ME==','||*ME=='.'){O(isdigit(*ME))
v.back()=v.back()*10+*ME++-'0';Y{v.push_back(0);++ME;}}}O(v[0]=='c'){ba(v.bh()<3)v.push_back(0);comp+=cf(dw)+" ";sb=11;O(v[2]<256)sb+=lg(v[2]);Y sb+=6;Q(J i=3;i<v.bh();++i)
O(v[i]<512)sb+=nbits(v[i])*3/4;O(sb>membits)sb=membits;O(v[1]%1000==0)comp+="icm "+cf(sb-6-v[1]/1000)+"\n";Y comp+="cm "+cf(sb-2-v[1]/1000)+" "+cf(v[1]%1000-1)+"\n";bQ+="d= "+cf(dw)+" *d=0\n";O(v[2]>1&&v[2]<=255){O(lg(v[2])!=lg(v[2]-1))
bQ+="a=c a&= "+cf(v[2]-1)+" hashd\n";Y
bQ+="a=c a%= "+cf(v[2])+" hashd\n";}Y O(v[2]>=1000&&v[2]<=1255)
bQ+="a= 255 a+= "+cf(v[2]-1000)+
" d=a a=*d a-=c a> 255 O a= 255 endif d= "+
cf(dw)+" hashd\n";Q(J i=3;i<v.bh();++i){O(i==3)bQ+="b=c ";O(v[i]==255)
bQ+="a=*b hashd\n";Y O(v[i]>0&&v[i]<255)
bQ+="a=*b a&= "+cf(v[i])+" hashd\n";Y O(v[i]>=256&&v[i]<512){bQ+=
"a=r 1 a> 1 O\n"
" a=r 2 a< 64 O\n"
" a=*b ";O(v[i]<511)bQ+="a&= "+cf(v[i]-256);bQ+=" hashd\n"
" Y\n"
" a>>= 6 hashd a=r 1 hashd\n"
" endif\n"
"Y\n"
" a= 255 hashd a=r 2 hashd\n"
"endif\n";}Y O(v[i]>=1256)
bQ+="a= "+cf(((v[i]-1000)>>8)&255)+" a<<= 8 a+= "
+cf((v[i]-1000)&255)+
" a+=b b=a\n";Y O(v[i]>1000)
bQ+="a= "+cf(v[i]-1000)+" a+=b b=a\n";O(v[i]<512&&i<v.bh()-1)
bQ+="b++ ";}++dw;}O(strchr("mts",v[0])&&dw>I(v[0]=='t')){E(0);}O(v[0]=='i'&&dw>0){bQ+="d= "+cf(dw-1)+" b=c a=*d d++\n";Q(J i=1;i<v.bh()&&dw<254;++i){Q(I j=0;j<v[i]%10;++j){bQ+="hash ";O(i<v.bh()-1||j<v[i]%10-1)bQ+="b++ ";sb+=6;}bQ+="*d=a";O(i<v.bh()-1)bQ+=" d++";bQ+="\n";O(sb>membits)sb=membits;comp+=cf(dw)+" isse "+cf(sb-6-v[i]/10)+" "+cf(dw-1)+"\n";++dw;}}O(v[0]=='a'){E(0);}O(v[0]=='w'){E(0);}}A hdr+cf(dw)+"\n"+comp+bQ+"halt\n"+cW;}N compressBlock(SB* in,bw* out,B V* method_,B V* cL,B V* comment,bU dosha1){std::bI ME=method_;B J n=in->bh();B I arg0=MAX(lg(n+4095)-20,0);J bd=0;O(isdigit(ME[0])){I commas=0,arg[4]={0};Q(I i=1;i<I(ME.bh())&&commas<4;++i){O(ME[i]==','||ME[i]=='.')++commas;Y O(isdigit(ME[i]))arg[commas]=arg[commas]*10+ME[i]-'0';}O(commas==0)bd=512;Y bd=arg[1]*4+arg[2];}O(isdigit(ME[0])){B I bX=ME[0]-'0';B I doe8=(bd&2)*2;ME="x"+cf(arg0);std::bI htsz=","+cf(19+arg0+(arg0<=6));std::bI sasz=","+cf(21+arg0);O(bX==0)
ME="0"+cf(arg0)+",0";Y O(bX==1){E(0);}Y O(bX==2){E(0);}Y O(bX==3){O(bd<20)
ME+=",0";Y O(bd<48)
ME+=","+cf(1+doe8)+",4,0,3"+htsz;Y O(bd>=640||(bd&1))
ME+=","+cf(3+doe8)+"ci1";Y
ME+=","+cf(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";}Y O(bX==4){E(0);}Y{E(0);}}std::bI config;I bl[9]={0};config=makeConfig(ME.c_str(),bl);LQ::C co;co.setOutput(out);SB pcomp_cmd;co.startBlock(config.c_str(),bl,&pcomp_cmd);std::bI cs=cf(n);co.startSegment(cL,cs.c_str());O(bl[1]>=1&&bl[1]<=7&&bl[1]!=4){dc lz(*in,bl);co.setInput(&lz);co.compress();}Y{E(0);}co.endSegment(0);}}namespace zpaq{td::cO compress(td::Slice bV){I bh=td::narrow_cast<int>(bV.bh());LQ::SB in,out;in.bo(bV.bV(),bh);LQ::compressBlock(&in,&out,"3",0,0,false);A td::cO(out.c_str(),out.bh());}td::cO decompress(td::Slice bV){LQ::SB in,out;in.bo(bV.bV(),bV.bh());LQ::decompress(&in,&out);A td::cO(out.c_str(),out.bh());}}
#undef Y
#undef O
#undef Q
td::cO compress(td::Slice bV){NullStream ofs;Ref<Cell> block_root=std_boc_deserialize(bV).move_as_ok();FullBlock block;G load_std_ctx{ofs};block.bc(load_std_ctx,block_root,0,cE);G pack_opt_ctx{ofs};cR opt_block_cell=block.bG(pack_opt_ctx);BagOfCells opt_boc;opt_boc.add_root(opt_block_cell);(opt_boc.import_cells().is_ok());cR opt_ser=std_boc_serialize(opt_block_cell).move_as_ok();cR compressed=zpaq::compress(opt_ser);A compressed;}td::cO decompress(td::Slice bV){NullStream ofs;cR decompressed=zpaq::decompress(bV);cR opt_deser=std_boc_deserialize(decompressed,false,cE).move_as_ok();FullBlock opt_block;G parse_opt_ctx{ofs};opt_block.bx(parse_opt_ctx,opt_deser,0,cE);G pack_std_ctx{ofs};cR un_opt_block_cell=opt_block.bb(pack_std_ctx);BagOfCells un_opt_boc;un_opt_boc.add_root(un_opt_block_cell);(un_opt_boc.import_cells().is_ok());cR boc=std_boc_serialize(un_opt_block_cell,31).move_as_ok();A boc;}int main(){bI mode;cin >> mode;bI base64_data;cin >> base64_data;td::cO bV(td::base64_decode(base64_data));bV=(mode=="compress")? compress(bV):decompress(bV);cout << td::str_base64_encode(bV)<< endl;}