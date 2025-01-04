
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
#define dw continue
#define dq ShardFeeCreated
#define dp false
#define dm else
#define dl blen
#define df out_msg_descr
#define de skip
#define cQ CurrencyCollection
#define cO auto
#define cN BufferSlice
#define cJ fetch_ulong
#define cH template
#define cD true
#define cx state_update
#define cw special_type
#define ca HashmapAugE
#define bX data
#define bV bool
#define bS store_long
#define bP string
#define bO virtual
#define bM extra
#define bL static
#define bJ append_cellslice
#define bE public
#define bB class
#define bq write
#define bo fetch_ref
#define bh size_t
#define bf struct
#define be size
#define bd store_ref
#define ba while
#define V char
#define U CellBuilder
#define S CellSlice
#define K void
#define J unsigned
#define G const
#define B return
#define u case
#define g break
using namespace vm;using namespace std;set<bP> enabled_optimizations{"Block","BlockExtra","ca","HashmapAug","HashmapAugNode","InMsgDescr","OutMsgDescr","McBlockExtra","ShardFees","ShardAccountBlocks","MERKLE_UPDATE","Maybe",};S to_cs(Ref<Cell> cell){bV can_be_special=dp;B load_cell_slice_special(std::move(cell),can_be_special);}bf A{ostream&out;};cH<bB T_TLB>bf N{S incoming_cs;S bI;S ccs;bP name;T_TLB bj;int cw=0;N(bP name):N(name,T_TLB()){}N(bP name,T_TLB bj):name(name),bj(bj){}bO ~N(){}bO K bg(A&ctx,S&cs,int o=0){}bO K br(A&ctx,U&cb,int o=0){bt(ctx,cb,o);}bO K bk(A&ctx,S&cs,int o=0){bg(ctx,cs,o);}bO K bt(A&ctx,U&cb,int o=0){}bV is_enabled(){B enabled_optimizations.count(name)> 0;}bV db(){B cw==1;}K bw(A&ctx,S&cs,int o=0,bV dc=dp){cO e=is_enabled();cw=(int)cs.cw();incoming_cs=cs;if(db()){cs.advance(288);B;}if(e){bg(ctx,cs,o);}dm{bI=bj.fetch(cs).bq();}}K ci(A&ctx,U&cb,int o=0){cO e=is_enabled();if(db()){cb.bJ(incoming_cs);}dm if(e){br(ctx,cb,o);}dm{cb.bJ(bI);}}K bA(A&ctx,S&cs,int o=0,bV dc=dp){cO e=is_enabled();incoming_cs=cs;if(e){bk(ctx,cs,o);}dm{bI=bj.fetch(cs).bq();}}K bK(A&ctx,U&cb,int o=0){cO e=is_enabled();if(db()){cb.bJ(incoming_cs);}dm if(e){bt(ctx,cb,o);}dm{if(!bI.is_valid()){throw runtime_error(name+":optimization is disabled,but bI is empty,meaning it was never set");}cb.bJ(bI);}}Ref<Cell> bb(A&ctx,int o=0){U cb;bK(ctx,cb,o);B cb.finalize(cw!=0);}Ref<Cell> bx(A&ctx,int o=0){U cb;ci(ctx,cb,o);B cb.finalize(cw!=0);}K bc(A&ctx,Ref<Cell> cell_ref,int o=0,bV dc=dp){cO cs=to_cs(std::move(cell_ref));bw(ctx,cs,o,dc);}K bn(A&ctx,Ref<Cell> cell_ref,int o=0,bV dc=dp){cO cs=to_cs(std::move(cell_ref));bA(ctx,cs,o,dc);}K fetch_remaining(S&cs){ccs=cs;cs.advance(cs.be());cs.advance_refs(cs.size_refs());}K append_remaining(U&cb){cb.bJ(ccs);}};bf cj{bO ~cj(){}bO S cE(){throw runtime_error("aug bX requested but not implemented");}};cH<bB T_TLB>bf cF{T_TLB add_type;cF(T_TLB add_type):add_type(add_type){}cF():cF(T_TLB{}){}bO ~cF(){}bO S add_values(S&cs1,S&cs2){U cb;(add_type.add_values(cb,cs1,cs2));B cb.as_cellslice();}};using namespace block::tlb;G cQ tCC;G OutMsg tOM;G AccountBlock tAB;G ImportFees tIF;G InMsg tIM;G EnqueuedMsg tEM;G UInt tU64{64};G dq tSFC;cH<bB cS,bB cI>bf by;cH<bB cS,bB cI>bf FullHashmapAugNode:N<block::gen::HashmapAugNode>{int tag=-1;int n=-1;Ref<by<cS,cI>> left;Ref<by<cS,cI>> right;cS dd;cI bM;FullHashmapAugNode(int m,G TLB&X,G TLB&Y):N("HashmapAugNode",block::gen::HashmapAugNode(m,X,Y)){}K bg(A&ctx,S&cs,int o=0){tag=bj.check_tag(cs);if(tag==0){bM.bw(ctx,cs,o+1);dd.bw(ctx,cs,o+1);}dm{int n;add_r1(n,1,bj.m_);left=Ref<by<cS,cI>>(cD,n,bj.X_,bj.Y_);left.bq().bc(ctx,cs.bo(),o+1);right=Ref<by<cS,cI>>(cD,n,bj.X_,bj.Y_);right.bq().bc(ctx,cs.bo(),o+1);bM.bw(ctx,cs,o+1);}}K br(A&ctx,U&cb,int o=0){if(tag==0){dd.ci(ctx,cb,o+1);}dm{int n;(add_r1(n,1,bj.m_));cb.bd(left.bq().bx(ctx,o+1));cb.bd(right.bq().bx(ctx,o+1));}}K bk(A&ctx,S&cs,int o=0){tag=bj.check_tag(cs);if(tag==0){dd.bA(ctx,cs,o+1);cO extra_cs=dd.cE();bM.bA(ctx,extra_cs,o+1,cD);}dm{int n;add_r1(n,1,bj.m_);left=Ref<by<cS,cI>>(cD,n,bj.X_,bj.Y_);left.bq().bn(ctx,cs.bo(),o+1);right=Ref<by<cS,cI>>(cD,n,bj.X_,bj.Y_);right.bq().bn(ctx,cs.bo(),o+1);cO left_extra_cs=to_cs(left.bq().node.bM.bb(ctx));cO right_extra_cs=to_cs(right.bq().node.bM.bb(ctx));cO extra_cs=bM.add_values(left_extra_cs,right_extra_cs);bM.bA(ctx,extra_cs,o+1,cD);}}K bt(A&ctx,U&cb,int o=0){if(tag==0){bM.bK(ctx,cb,o+1);dd.bK(ctx,cb,o+1);}dm{int n;(add_r1(n,1,bj.m_));cb.bd(left.bq().bb(ctx,o+1));cb.bd(right.bq().bb(ctx,o+1));bM.bK(ctx,cb,o+1);}}};cH<bB cS,bB cI>bf by:N<block::gen::HashmapAug>,td::CntObject{Ref<S> label;int n,m,l;FullHashmapAugNode<cS,cI> node;by(int n,G TLB&X,G TLB&Y):N("HashmapAug",block::gen::HashmapAug(n,X,Y)),node(n,X,Y){}K bg(A&ctx,S&cs,int o=0){n=bj.m_;(block::gen::HmLabel{n}.fetch_to(cs,label,l));m=n - l;node.bj.m_=m;node.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){int l,m;(tlb::store_from(cb,HmLabel{bj.m_},label,l));(add_r1(m,l,bj.m_));node.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){(
(n=bj.m_)>= 0
&&block::gen::HmLabel{bj.m_}.fetch_to(cs,label,l)&&add_r1(m,l,bj.m_));node.bj.m_=m;node.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){int l,m;(tlb::store_from(cb,block::gen::HmLabel{bj.m_},label,l)&&add_r1(m,l,bj.m_));node.bK(ctx,cb,o+1);}};cH<bB cS,bB cI>bf ck:N<block::gen::ca>{block::gen::ca::Record_ahme_root r;int tag=-1;by<cS,cI> root;cI bM;ck(int n,G TLB&X,G TLB&Y):N("ca",block::gen::ca(n,X,Y)),root(n,X,Y){}K bg(A&ctx,S&cs,int o=0){tag=bj.check_tag(cs);if(tag==block::gen::ca::ahme_empty){(cs.cJ(1)== 0);bM.bw(ctx,cs,o+1);}dm{(bj.unpack(cs,r));root.bc(ctx,r.root,o+1);bM.bw(ctx,r.bM.bq(),o+1);}}K br(A&ctx,U&cb,int o=0){if(tag==ca::ahme_empty){cb.bS(0,1);bM.ci(ctx,cb,o+1);}dm{cb.bS(1,1).bd(root.bx(ctx,o+1));}}K bk(A&ctx,S&cs,int o=0){tag=bj.check_tag(cs);if(tag==ca::ahme_empty){(cs.cJ(1)== 0);bM.bA(ctx,cs,o+1);}dm{(cs.cJ(1)== 1&&(r.n=bj.m_)>= 0);cO root_ref=cs.bo();root.bn(ctx,root_ref,o+1);bM=root.node.bM;}}K bt(A&ctx,U&cb,int o=0){if(tag==ca::ahme_empty){cb.bS(0,1);bM.bK(ctx,cb,o+1);}dm{cb.bS(1,1).bd(root.bb(ctx,o+1));bM.bK(ctx,cb,o+1);}}};cH<bB T>bf FullMaybe:N<TLB>{T dd;int tag=-1;bV is_ref;FullMaybe(bV is_ref=dp):N("Maybe"),is_ref(is_ref){}K bg(A&ctx,S&cs,int o=0){tag=cs.cJ(1);if(tag){if(is_ref){dd.bc(ctx,cs.bo(),o+1);}dm{dd.bw(ctx,cs,o+1);}}}bO K br(A&ctx,U&cb,int o=0){(cb.store_long_bool(tag,1));if(tag){if(is_ref){cb.bd(dd.bx(ctx,o+1));}dm{dd.ci(ctx,cb,o+1);}}}bO K bk(A&ctx,S&cs,int o=0){tag=cs.cJ(1);if(tag){if(is_ref){dd.bn(ctx,cs.bo(),o+1);}dm{dd.bA(ctx,cs,o+1);}}}K bt(A&ctx,U&cb,int o=0){(cb.store_long_bool(tag,1));if(tag){if(is_ref){cb.bd(dd.bb(ctx,o+1));}dm{dd.bK(ctx,cb,o+1);}}}};bf cT;bf FullInMsg:N<InMsg>,cj{FullInMsg():N("InMsg",InMsg()){}S cE(){U cb;cO cs_copy=bI;(bj.get_import_fees(cb,cs_copy));B cb.as_cellslice();}};bf cl:N<cQ>,cF<cQ>{cl():N("cQ"){}};bf FullOutMsg:N<OutMsg>,cj{FullOutMsg():N("OutMsg",OutMsg()){}S cE(){U cb;cO cs_copy=bI;(bj.get_export_value(cb,cs_copy));B cb.as_cellslice();}};cH<bB T>bf FullMERKLE_UPDATE:N<TLB>{Ref<T> cX,to_proof;FullMERKLE_UPDATE(G T&bj):N("MERKLE_UPDATE"){}K bg(A&ctx,S&cs,int o=0){(cs.advance(520));cX=Ref<T>(cD);cX.bq().bc(ctx,cs.bo(),o+1);to_proof=Ref<T>(cD);to_proof.bq().bc(ctx,cs.bo(),o+1);}K br(A&ctx,U&cb,int o=0){cb.bd(cX.bq().bb(ctx,o+1));cb.bd(to_proof.bq().bb(ctx,o+1));cw=0;}K bk(A&ctx,S&cs,int o=0){cX=Ref<T>(cD);cX.bq().bc(ctx,cs.bo(),o+1);to_proof=Ref<T>(cD);to_proof.bq().bc(ctx,cs.bo(),o+1);}Ref<Cell> bb(A&ctx,int o=0){B U::create_merkle_update(
cX.bq().bb(ctx,o+1),to_proof.bq().bb(ctx,o+1));}};bf FullAccountBlock:N<AccountBlock>,cj{FullAccountBlock():N("AccountBlock"){}S cE(){U cb;cO cs_copy=bI;(Aug_ShardAccountBlocks().eval_leaf(cb,cs_copy));B cb.as_cellslice();}};bf cY:N<block::gen::ShardAccountBlocks>{ck<FullAccountBlock,cl> x{256,tAB,tCC};cY():N("ShardAccountBlocks"){}K bg(A&ctx,S&cs,int o=0){x.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){x.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){x.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){x.bK(ctx,cb,o+1);}};bf MyMcStateExtra:block::gen::McStateExtra{bV de(vm::S&cs)G {B cs.advance(16)&&block::gen::ShardHashes().de(cs)&&cs.advance_ext(0x100,2)&&tCC.de(cs);}};bf FullMcStateExtra:N<MyMcStateExtra>{FullMcStateExtra():N("McStateExtra",MyMcStateExtra()){}};G block::gen::ShardStateUnsplit_aux tSSUa;G block::gen::RefT tRMSE{MyMcStateExtra()};G block::gen::Maybe tMRMSE{tRMSE};bf MyShardStateUnsplit:block::gen::ShardStateUnsplit{bV de(vm::S&cs)G{B cs.advance_ext(0x169,3)&&tMRMSE.de(cs);}};bf FullShardState:N<ShardState>,td::CntObject{FullShardState():N("ShardState"){}};bf cG:N<dq>,cj,cF<dq>{cG():N("dq"){}S cE(){U cb;cO cs_copy=bI;(Aug_ShardFees().eval_leaf(cb,cs_copy));B cb.as_cellslice();}};bf FullShardFees:N<block::gen::ShardFees>{ck<cG,cG> x{96,tSFC,tSFC};FullShardFees():N("ShardFees"){}K bg(A&ctx,S&cs,int o=0){x.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){x.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){x.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){x.bK(ctx,cb,o+1);}};bf FullMcBlockExtra:N<block::gen::McBlockExtra>{block::gen::McBlockExtra::Record bD;FullShardFees shard_fees;FullMcBlockExtra():N("McBlockExtra"){}K bg(A&ctx,S&cs,int o=0){(cs.cJ(16)== 0xcca5);(cs.fetch_bool_to(bD.key_block));(block::gen::t_ShardHashes.fetch_to(cs,bD.shard_hashes));shard_fees.bw(ctx,cs,o+1);fetch_remaining(cs);}K br(A&ctx,U&cb,int o=0){cb.bS(bD.key_block,1);block::gen::t_ShardHashes.store_from(cb,bD.shard_hashes);shard_fees.ci(ctx,cb,o+1);append_remaining(cb);}K bk(A&ctx,S&cs,int o=0){(cs.fetch_bool_to(bD.key_block));(block::gen::t_ShardHashes.fetch_to(cs,bD.shard_hashes));shard_fees.bA(ctx,cs,o+1);fetch_remaining(cs);}K bt(A&ctx,U&cb,int o=0){cb.bS(0xcca5,16).bS(bD.key_block,1);block::gen::t_ShardHashes.store_from(cb,bD.shard_hashes);shard_fees.bK(ctx,cb,o+1);append_remaining(cb);}};bf cT;bf FullOutMsgDescr:N<OutMsgDescr>{ck<FullOutMsg,cl> x{256,tOM,tCC};FullOutMsgDescr():N("OutMsgDescr"){}K bg(A&ctx,S&cs,int o=0){x.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){x.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){x.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){x.bK(ctx,cb,o+1);}};bf cT:N<ImportFees>,cF<ImportFees>{cT():N("ImportFees",tIF),cF(tIF){}};bf FullInMsgDescr:N<InMsgDescr>{ck<FullInMsg,cT> x{256,tIM,tIF};FullInMsgDescr():N("InMsgDescr",InMsgDescr()){}K bg(A&ctx,S&cs,int o=0){x.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){x.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){x.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){x.bK(ctx,cb,o+1);}};G block::gen::McBlockExtra tMBE{};G block::gen::RefT tRMBE{tMBE};G block::gen::Maybe tMRMBE(tRMBE);bf FullBlockExtra:N<block::gen::BlockExtra>{block::gen::BlockExtra::Record bD;FullInMsgDescr in_msg_descr;FullOutMsgDescr df;cY cU;FullMaybe<FullMcBlockExtra> custom;FullBlockExtra():N("BlockExtra"),custom(cD){}K bg(A&ctx,S&cs,int o=0){((cs.cJ(32)== 0x4a33f6fd));in_msg_descr.bc(ctx,cs.bo(),o+1);df.bc(ctx,cs.bo(),o+1);cU.bc(ctx,cs.bo(),o+1);ccs=cs.fetch_subslice(512).bq();custom.bw(ctx,cs,o+1);}K br(A&ctx,U&cb,int o=0){cb.bd(in_msg_descr.bx(ctx,o+1)).bd(df.bx(ctx,o+1)).bd(cU.bx(ctx,o+1)).bJ(ccs);custom.ci(ctx,cb,o+1);}K bk(A&ctx,S&cs,int o=0){in_msg_descr.bn(ctx,cs.bo(),o+1);df.bn(ctx,cs.bo(),o+1);cU.bn(ctx,cs.bo(),o+1);ccs=cs.fetch_subslice(512).bq();custom.bA(ctx,cs,o+1);}K bt(A&ctx,U&cb,int o=0){cb.bS(0x4a33f6fd,32).bd(in_msg_descr.bb(ctx,o+1)).bd(df.bb(ctx,o+1)).bd(cU.bb(ctx,o+1)).bJ(ccs);custom.bK(ctx,cb,o+1);}};bf FullBlock:N<block::gen::Block>{block::gen::Block::Record bD;FullMERKLE_UPDATE<FullShardState> cx;FullBlockExtra bM;FullBlock():N("Block"),cx(FullShardState()){}K bg(A&ctx,S&cs,int o=0){(bj.unpack(cs,bD));cx.bc(ctx,bD.cx,o+1);bM.bc(ctx,bD.bM,o+1,cD);}K br(A&ctx,U&cb,int o=0){cb.bS(bD.global_id,32).bd(bD.info).bd(bD.value_flow).bd(cx.bx(ctx,o+1)).bd(bM.bx(ctx,o+1));}K bk(A&ctx,S&cs,int o=0){(
cs.fetch_int_to(32,bD.global_id)&&cs.fetch_ref_to(bD.info)&&cs.fetch_ref_to(bD.value_flow));cx.bn(ctx,cs.bo(),o+1);bM.bn(ctx,cs.bo(),o+1,cD);}K bt(A&ctx,U&cb,int o=0){cb.bS(0x11ef55aa,32).bS(bD.global_id,32).bd(bD.info).bd(bD.value_flow).bd(cx.bb(ctx,o+1)).bd(bM.bb(ctx,o+1));}};bB NullStream:bE ostream{bB NullBuffer:bE streambuf{bE:int overflow(int c){B c;}}m_nb;bE:NullStream():ostream(&m_nb){}};
#define Y dm
#define O if
#define Q for
using I=int;using namespace std;namespace LQ{typedef uint8_t U8;typedef uint16_t U16;typedef uint32_t U32;typedef uint64_t U64;K E(G V* msg=0){throw runtime_error(msg);}bf cg{bO I get()= 0;bO I read(V* buf,I n);bO ~cg(){}};bf bz{bO K put(I c)= 0;bO K bq(G V* buf,I n);bO ~bz(){}};I toU16(G V* p);cH<bB T>
bB cC{T *bX;bh n;I offset;K operator=(G cC&);cC(G cC&);bE:cC(bh sz=0,I ex=0):bX(0),n(0),offset(0){bl(sz,ex);}
K bl(bh sz,I ex=0);~cC(){bl(0);}
bh be()G{B n;}
I isize()G{B I(n);}
T&operator[](bh i){B bX[i];}
T&operator()(bh i){B bX[i&(n-1)];}};cH<bB T>
K cC<T>::bl(bh sz,I ex){ba(ex>0){O(sz>sz*2)E(0);sz*=2,--ex;}O(n>0){::free((V*)bX-offset);}n=0;offset=0;O(sz==0)B;n=sz;G bh nb=128+n*sizeof(T);O(nb<=128||(nb-128)/sizeof(T)!=n)n=0,E(0);bX=(T*)::calloc(nb,1);O(!bX)n=0,E(0);offset=64-(((V*)bX-(V*)0)&63);bX=(T*)((V*)bX+offset);}typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE}CompType;bB dg;bB ZP{bE:ZP();~ZP();K clear();K inith();K initp();double memory();K run(U32 input);I read(cg* in2);bV bq(bz* out2,bV pp);I step(U32 input,I mode);bz* output;U32 H(I i){B h(i);}
K flush();K outc(I ch){O(ch<0||(outbuf[bufptr]=ch,++bufptr==outbuf.isize()))flush();}cC<U8> D;I cn;I bQ,bu;
cC<U8> m;cC<U32> h,r;cC<V> outbuf;I bufptr;U32 a,b,c,d;I f,pc,rcode_size;U8* rcode;K cP(I hbits,I mbits);I execute();K div(U32 x){O(x)a/=x;Y a=0;}
K mod(U32 x){O(x)a%=x;Y a=0;}
K swap(U32&x){a^=x;x^=a;a^=x;}
K swap(U8&x){a^=x;x^=a;a^=x;}
K err(){E(0);}};bf dr{bh z;bh cxt;bh a,b,c;cC<U32> cm;cC<U8> ht;cC<U16> a16;K cP(){z=cxt=a=b=c=0;cm.bl(0);ht.bl(0);a16.bl(0);}dr(){cP();}};bB StateTable{bE:U8 ns[1024];I bF(I s,I y){B ns[s*4+y];}I cminit(I s){B((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1);}StateTable();};bB P{bE:P(ZP&);~P();K cP();I predict();K update(I y);I stat(I);bV ds(){B z.D[6]!=0;}
I c8;I du;I p[256];U32 h[256];ZP&z;dr comp[256];bV initTables;I predict0();K update0(I y);I dt2k[256];I dt[1024];U16 squasht[4096];short stretcht[32768];StateTable st;U8* pcode;I pcode_size;K train(dr&cr,I y){U32&pn=cr.cm(cr.cxt);U32 count=pn&0x3ff;I E=y*32767-(cr.cm(cr.cxt)>>17);pn+=(E*dt[count]&-1024)+(count<cr.z);}I squash(I x){B squasht[x+2048];}I stretch(I x){B stretcht[x];}I clamp2k(I x){O(x<-2048)B -2048;Y O(x>2047)B 2047;Y B x;}I clamp512k(I x){O(x<-(1<<19))B -(1<<19);Y O(x>=(1<<19))B(1<<19)-1;Y B x;}bh find(cC<U8>&ht,I sizebits,U32 cxt){I chk=cxt>>sizebits&255;bh h0=(cxt*16)&(ht.be()-16);O(ht[h0]==chk)B h0;bh h1=h0^16;O(ht[h1]==chk)B h1;bh h2=h0^32;O(ht[h2]==chk)B h2;O(ht[h0+1]<=ht[h1+1]&&ht[h0+1]<=ht[h2+1])
B memset(&ht[h0],0,16),ht[h0]=chk,h0;Y O(ht[h1+1]<ht[h2+1])
B memset(&ht[h1],0,16),ht[h1]=chk,h1;Y
B memset(&ht[h2],0,16),ht[h2]=chk,h2;}I assemble_p();};bB dg:bE cg{bE:cg* in;dg(ZP&z);I decompress();I de();K cP();I stat(I x){B pr.stat(x);}
I get(){O(bT==bC){bT=0;bC=in ? in->read(&buf[0],cv):0;}B bT<bC ? U8(buf[bT++]):-1;}I buffered(){B bC-bT;}
U32 low,cR;U32 bY;U32 bT,bC;P pr;enum{cv=1<<16};cC<V> buf;I decode(I p);};bB dh{I bi;I ce;I ph,pm;bE:ZP z;dh():bi(0),ce(0),ph(0),pm(0){}K cP(I h,I m);I bq(I c);I getState()G{B bi;}
K setOutput(bz* out){z.output=out;}};bB D{bE:D():z(),dec(z),pp(),bi(BLOCK),cy(FIRSTSEG){}K setInput(cg* in){dec.in=in;}
bV findBlock(double* memptr=0);bV findFilename(bz* = 0);K readComment(bz* = 0);K setOutput(bz* out){pp.setOutput(out);}
bV decompress(I n=-1);K readSegmentEnd(V* cc=0);I stat(I x){B dec.stat(x);}
I buffered(){B dec.buffered();}
ZP z;dg dec;dh pp;enum{BLOCK,FILENAME,COMMENT,DATA,SEGEND}bi;enum{FIRSTSEG,SEG,SKIP}cy;};K decompress(cg* in,bz* out);bB Encoder{bE:Encoder(ZP&z,I be=0):
out(0),low(1),cR(0xFFFFFFFF),pr(z){}K cP();K compress(I c);I stat(I x){B pr.stat(x);}
bz* out;
U32 low,cR;P pr;cC<V> buf;K encode(I y,I p);};bB cq{bE:cq(G V* in,I* bm,ZP&hz,ZP&pz,bz* out2);
G V* in;I* bm;ZP&hz;ZP&pz;bz* out2;I line;I bi;typedef enum{NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,JT=39,JF=47,JMP=63,LJ=255,POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON}CompType;K SE(G V* msg,G V* expected=0);K bF();bV matchToken(G V* tok);I rtoken(I low,I cR);I rtoken(G V* list[]);K rtoken(G V* s);I compile_comp(ZP&z);bB Stack{LQ::cC<U16> s;bh top;bE:Stack(I n):s(n),top(0){}K push(G U16&x){O(top>=s.be())E(0);s[top++]=x;}U16 pop(){O(top<=0)E(0);B s[--top];}};Stack dv,do_stack;};bB C{bE:C():enc(z),in(0),bi(INIT){}K setOutput(bz* out){enc.out=out;}
K startBlock(G V* config,I* bm,bz* pcomp_cmd=0);K startSegment(G V* cK=0,G V* comment=0);K setInput(cg* i){in=i;}
K postProcess(G V* cV=0,I len=0);bV compress(I n=-1);K endSegment(G V* cc=0);I stat(I x){B enc.stat(x);}
ZP z,pz;Encoder enc;cg* in;enum{INIT,BLOCK1,SEG1,BLOCK2,SEG2}bi;};bB SB:bE LQ::cg,bE LQ::bz{J V* p;bh al;bh bC;bh bT;bh bN;G bh cP;K reserve(bh a){O(a<=al)B;J V* q=0;O(a>0)q=(J V*)(p ? realloc(p,a):malloc(a));O(a>0&&!q)E(0);p=q;al=a;}K lengthen(bh n){O(bC+n>bN||bC+n<bC)E(0);O(bC+n<=al)B;bh a=al;ba(bC+n>=a)a=a*2+cP;reserve(a);}K operator=(G SB&);SB(G SB&);bE:J V* bX(){B p;}
SB(bh n=0):
p(0),al(0),bC(0),bT(0),bN(bh(-1)),cP(n>128?n:128){}~SB(){O(p)free(p);}
bh be()G{B bC;}
K put(I c){lengthen(1);p[bC++]=c;}K bq(G V* buf,I n){O(n<1)B;lengthen(n);O(buf)memcpy(p+bC,buf,n);bC+=n;}I get(){B bT<bC ? p[bT++]:-1;}I read(V* buf,I n){O(bT+n>bC)n=bC-bT;O(n>0&&buf)memcpy(buf,p+bT,n);bT+=n;B n;}G V* c_str()G{B(G V*)p;}
K bl(bh i){bC=i;O(bT>bC)bT=bC;}K swap(SB&s){::swap(p,s.p);::swap(al,s.al);::swap(bC,s.bC);::swap(bT,s.bT);::swap(bN,s.bN);}};K compress(cg* in,bz* out,G V* ME,G V* cK=0,G V* comment=0,bV dosha1=cD);K compressBlock(SB* in,bz* out,G V* ME,G V* cK=0,G V* comment=0,bV dosha1=cD);I toU16(G V* p){B(p[0]&255)+256*(p[1]&255);}I cg::read(V* buf,I n){I i=0,c;ba(i<n&&(c=get())>=0)
buf[i++]=c;B i;}K bz::bq(G V* buf,I n){Q(I i=0;i<n;++i)
put(U8(buf[i]));}K allocx(U8*&p,I&n,I newsize){p=0;n=0;}G I compsize[256]={0,2,3,2,3,4,6,6,3,5};bL G U8 sns[1024]={1,2,0,0,3,5,1,0,4,6,0,1,7,9,2,0,8,11,1,1,8,11,1,1,10,12,0,2,13,15,3,0,14,17,2,1,14,17,2,1,16,19,1,2,16,19,1,2,18,20,0,3,21,23,4,0,22,25,3,1,22,25,3,1,24,27,2,2,24,27,2,2,26,29,1,3,26,29,1,3,28,30,0,4,31,33,5,0,32,35,4,1,32,35,4,1,34,37,3,2,34,37,3,2,36,39,2,3,36,39,2,3,38,41,1,4,38,41,1,4,40,42,0,5,43,33,6,0,44,47,5,1,44,47,5,1,46,49,4,2,46,49,4,2,48,51,3,3,48,51,3,3,50,53,2,4,50,53,2,4,52,55,1,5,52,55,1,5,40,56,0,6,57,45,7,0,58,47,6,1,58,47,6,1,60,63,5,2,60,63,5,2,62,65,4,3,62,65,4,3,64,67,3,4,64,67,3,4,66,69,2,5,66,69,2,5,52,71,1,6,52,71,1,6,54,72,0,7,73,59,8,0,74,61,7,1,74,61,7,1,76,63,6,2,76,63,6,2,78,81,5,3,78,81,5,3,80,83,4,4,80,83,4,4,82,85,3,5,82,85,3,5,66,87,2,6,66,87,2,6,68,89,1,7,68,89,1,7,70,90,0,8,91,59,9,0,92,77,8,1,92,77,8,1,94,79,7,2,94,79,7,2,96,81,6,3,96,81,6,3,98,101,5,4,98,101,5,4,100,103,4,5,100,103,4,5,82,105,3,6,82,105,3,6,84,107,2,7,84,107,2,7,86,109,1,8,86,109,1,8,70,110,0,9,111,59,10,0,112,77,9,1,112,77,9,1,114,97,8,2,114,97,8,2,116,99,7,3,116,99,7,3,62,101,6,4,62,101,6,4,80,83,5,5,80,83,5,5,100,67,4,6,100,67,4,6,102,119,3,7,102,119,3,7,104,121,2,8,104,121,2,8,86,123,1,9,86,123,1,9,70,124,0,10,125,59,11,0,126,77,10,1,126,77,10,1,128,97,9,2,128,97,9,2,60,63,8,3,60,63,8,3,66,69,3,8,66,69,3,8,104,131,2,9,104,131,2,9,86,133,1,10,86,133,1,10,70,134,0,11,135,59,12,0,136,77,11,1,136,77,11,1,138,97,10,2,138,97,10,2,104,141,2,10,104,141,2,10,86,143,1,11,86,143,1,11,70,144,0,12,145,59,13,0,146,77,12,1,146,77,12,1,148,97,11,2,148,97,11,2,104,151,2,11,104,151,2,11,86,153,1,12,86,153,1,12,70,154,0,13,155,59,14,0,156,77,13,1,156,77,13,1,158,97,12,2,158,97,12,2,104,161,2,12,104,161,2,12,86,163,1,13,86,163,1,13,70,164,0,14,165,59,15,0,166,77,14,1,166,77,14,1,168,97,13,2,168,97,13,2,104,171,2,13,104,171,2,13,86,173,1,14,86,173,1,14,70,174,0,15,175,59,16,0,176,77,15,1,176,77,15,1,178,97,14,2,178,97,14,2,104,181,2,14,104,181,2,14,86,183,1,15,86,183,1,15,70,184,0,16,185,59,17,0,186,77,16,1,186,77,16,1,74,97,15,2,74,97,15,2,104,89,2,15,104,89,2,15,86,187,1,16,86,187,1,16,70,188,0,17,189,59,18,0,190,77,17,1,86,191,1,17,70,192,0,18,193,59,19,0,194,77,18,1,86,195,1,18,70,196,0,19,193,59,20,0,197,77,19,1,86,198,1,19,70,196,0,20,199,77,20,1,86,200,1,20,201,77,21,1,86,202,1,21,203,77,22,1,86,204,1,22,205,77,23,1,86,206,1,23,207,77,24,1,86,208,1,24,209,77,25,1,86,210,1,25,211,77,26,1,86,212,1,26,213,77,27,1,86,214,1,27,215,77,28,1,86,216,1,28,217,77,29,1,86,218,1,29,219,77,30,1,86,220,1,30,221,77,31,1,86,222,1,31,223,77,32,1,86,224,1,32,225,77,33,1,86,226,1,33,227,77,34,1,86,228,1,34,229,77,35,1,86,230,1,35,231,77,36,1,86,232,1,36,233,77,37,1,86,234,1,37,235,77,38,1,86,236,1,38,237,77,39,1,86,238,1,39,239,77,40,1,86,240,1,40,241,77,41,1,86,242,1,41,243,77,42,1,86,244,1,42,245,77,43,1,86,246,1,43,247,77,44,1,86,248,1,44,249,77,45,1,86,250,1,45,251,77,46,1,86,252,1,46,253,77,47,1,86,254,1,47,253,77,48,1,86,254,1,48,0,0,0,0
};StateTable::StateTable(){memcpy(ns,sns,sizeof(ns));}bV ZP::bq(bz* out2,bV pp){O(D.be()<=6)B dp;O(!pp){Q(I i=0;i<cn;++i)
out2->put(D[i]);}Y{out2->put((bu-bQ)&255);out2->put((bu-bQ)>>8);}Q(I i=bQ;i<bu;++i)
out2->put(D[i]);B cD;}I ZP::read(cg* in2){I ce=in2->get();ce+=in2->get()*256;D.bl(ce+300);cn=bQ=bu=0;D[cn++]=ce&255;D[cn++]=ce>>8;ba(cn<7)D[cn++]=in2->get();I n=D[cn-1];Q(I i=0;i<n;++i){I bj=in2->get();O(bj<0||bj>255)E(0);D[cn++]=bj;I be=compsize[bj];O(be<1)E(0);O(cn+be>ce)E(0);Q(I j=1;j<be;++j)
D[cn++]=in2->get();}O((D[cn++]=in2->get())!=0)E(0);bQ=bu=cn+128;O(bu>ce+129)E(0);ba(bu<ce+129){I op=in2->get();O(op==-1)E(0);D[bu++]=op;}O((D[bu++]=in2->get())!=0)E(0);allocx(rcode,rcode_size,0);B cn+bu-bQ;}K ZP::clear(){cn=bQ=bu=0;a=b=c=d=f=pc=0;D.bl(0);h.bl(0);m.bl(0);r.bl(0);allocx(rcode,rcode_size,0);}ZP::ZP(){output=0;rcode=0;rcode_size=0;clear();outbuf.bl(1<<14);bufptr=0;}ZP::~ZP(){allocx(rcode,rcode_size,0);}K ZP::inith(){cP(D[2],D[3]);}K ZP::initp(){cP(D[4],D[5]);}K ZP::flush(){O(output)output->bq(&outbuf[0],bufptr);bufptr=0;}bL double pow2(I x){double r=1;Q(;x>0;x--)r+=r;B r;}double ZP::memory(){double mem=pow2(D[2]+2)+pow2(D[3])
+pow2(D[4]+2)+pow2(D[5])
+D.be();I cp=7;Q(I i=0;i<D[6];++i){double be=pow2(D[cp+1]);switch(D[cp]){u CM:mem+=4*be;g;u ICM:mem+=64*be+1024;g;u MATCH:mem+=4*be+pow2(D[cp+2]);g;u MIX2:mem+=2*be;g;u MIX:mem+=4*be*D[cp+3];g;u ISSE:mem+=64*be+2048;g;u SSE:mem+=128*be;g;}cp+=compsize[D[cp]];}B mem;}K ZP::cP(I hbits,I mbits){O(hbits>32)E(0);O(mbits>32)E(0);h.bl(1,hbits);m.bl(1,mbits);r.bl(256);a=b=c=d=pc=f=0;}I ZP::execute(){switch(D[pc++]){u 0:err();g;u 1:++a;g;u 2:--a;g;u 3:a=~a;g;u 4:a=0;g;u 7:a=r[D[pc++]];g;u 8:swap(b);g;u 9:++b;g;u 10:--b;g;u 11:b=~b;g;u 12:b=0;g;u 15:b=r[D[pc++]];g;u 16:swap(c);g;u 17:++c;g;u 18:--c;g;u 19:c=~c;g;u 20:c=0;g;u 23:c=r[D[pc++]];g;u 24:swap(d);g;u 25:++d;g;u 26:--d;g;u 27:d=~d;g;u 28:d=0;g;u 31:d=r[D[pc++]];g;u 32:swap(m(b));g;u 33:++m(b);g;u 34:--m(b);g;u 35:m(b)= ~m(b);g;u 36:m(b)= 0;g;u 39:O(f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;u 40:swap(m(c));g;u 41:++m(c);g;u 42:--m(c);g;u 43:m(c)= ~m(c);g;u 44:m(c)= 0;g;u 47:O(!f)pc+=((D[pc]+128)&255)-127;Y ++pc;g;u 48:swap(h(d));g;u 49:++h(d);g;u 50:--h(d);g;u 51:h(d)= ~h(d);g;u 52:h(d)= 0;g;u 55:r[D[pc++]]=a;g;u 56:B 0 ;u 57:outc(a&255);g;u 59:a=(a+m(b)+512)*773;g;u 60:h(d)= (h(d)+a+512)*773;g;u 63:pc+=((D[pc]+128)&255)-127;g;u 64:g;u 65:a=b;g;u 66:a=c;g;u 67:a=d;g;u 68:a=m(b);g;u 69:a=m(c);g;u 70:a=h(d);g;u 71:a=D[pc++];g;u 72:b=a;g;u 73:g;u 74:b=c;g;u 75:b=d;g;u 76:b=m(b);g;u 77:b=m(c);g;u 78:b=h(d);g;u 79:b=D[pc++];g;u 80:c=a;g;u 81:c=b;g;u 82:g;u 83:c=d;g;u 84:c=m(b);g;u 85:c=m(c);g;u 86:c=h(d);g;u 87:c=D[pc++];g;u 88:d=a;g;u 89:d=b;g;u 90:d=c;g;u 91:g;u 92:d=m(b);g;u 93:d=m(c);g;u 94:d=h(d);g;u 95:d=D[pc++];g;u 96:m(b)=a;g;u 97:m(b)=b;g;u 98:m(b)=c;g;u 99:m(b)=d;g;u 100:g;u 101:m(b)=m(c);g;u 102:m(b)=h(d);g;u 103:m(b)=D[pc++];g;u 104:m(c)=a;g;u 105:m(c)=b;g;u 106:m(c)=c;g;u 107:m(c)=d;g;u 108:m(c)=m(b);g;u 109:g;u 110:m(c)=h(d);g;u 111:m(c)=D[pc++];g;u 112:h(d)=a;g;u 113:h(d)=b;g;u 114:h(d)=c;g;u 115:h(d)=d;g;u 116:h(d)=m(b);g;u 117:h(d)=m(c);g;u 118:g;u 119:h(d)=D[pc++];g;u 128:a+=a;g;u 129:a+=b;g;u 130:a+=c;g;u 131:a+=d;g;u 132:a+=m(b);g;u 133:a+=m(c);g;u 134:a+=h(d);g;u 135:a+=D[pc++];g;u 136:a-=a;g;u 137:a-=b;g;u 138:a-=c;g;u 139:a-=d;g;u 140:a-=m(b);g;u 141:a-=m(c);g;u 142:a-=h(d);g;u 143:a-=D[pc++];g;u 144:a*=a;g;u 145:a*=b;g;u 146:a*=c;g;u 147:a*=d;g;u 148:a*=m(b);g;u 149:a*=m(c);g;u 150:a*=h(d);g;u 151:a*=D[pc++];g;u 152:div(a);g;u 153:div(b);g;u 154:div(c);g;u 155:div(d);g;u 156:div(m(b));g;u 157:div(m(c));g;u 158:div(h(d));g;u 159:div(D[pc++]);g;u 160:mod(a);g;u 161:mod(b);g;u 162:mod(c);g;u 163:mod(d);g;u 164:mod(m(b));g;u 165:mod(m(c));g;u 166:mod(h(d));g;u 167:mod(D[pc++]);g;u 168:a&=a;g;u 169:a&=b;g;u 170:a&=c;g;u 171:a&=d;g;u 172:a&=m(b);g;u 173:a&=m(c);g;u 174:a&=h(d);g;u 175:a&=D[pc++];g;u 176:a&=~a;g;u 177:a&=~b;g;u 178:a&=~c;g;u 179:a&=~d;g;u 180:a&=~m(b);g;u 181:a&=~m(c);g;u 182:a&=~h(d);g;u 183:a&=~D[pc++];g;u 184:a|=a;g;u 185:a|=b;g;u 186:a|=c;g;u 187:a|=d;g;u 188:a|=m(b);g;u 189:a|=m(c);g;u 190:a|=h(d);g;u 191:a|=D[pc++];g;u 192:a^=a;g;u 193:a^=b;g;u 194:a^=c;g;u 195:a^=d;g;u 196:a^=m(b);g;u 197:a^=m(c);g;u 198:a^=h(d);g;u 199:a^=D[pc++];g;u 200:a<<=(a&31);g;u 201:a<<=(b&31);g;u 202:a<<=(c&31);g;u 203:a<<=(d&31);g;u 204:a<<=(m(b)&31);g;u 205:a<<=(m(c)&31);g;u 206:a<<=(h(d)&31);g;u 207:a<<=(D[pc++]&31);g;u 208:a>>=(a&31);g;u 209:a>>=(b&31);g;u 210:a>>=(c&31);g;u 211:a>>=(d&31);g;u 212:a>>=(m(b)&31);g;u 213:a>>=(m(c)&31);g;u 214:a>>=(h(d)&31);g;u 215:a>>=(D[pc++]&31);g;u 216:f=1;g;u 217:f=(a==b);g;u 218:f=(a==c);g;u 219:f=(a==d);g;u 220:f=(a==U32(m(b)));g;u 221:f=(a==U32(m(c)));g;u 222:f=(a==h(d));g;u 223:f=(a==U32(D[pc++]));g;u 224:f=0;g;u 225:f=(a<b);g;u 226:f=(a<c);g;u 227:f=(a<d);g;u 228:f=(a<U32(m(b)));g;u 229:f=(a<U32(m(c)));g;u 230:f=(a<h(d));g;u 231:f=(a<U32(D[pc++]));g;u 232:f=0;g;u 233:f=(a>b);g;u 234:f=(a>c);g;u 235:f=(a>d);g;u 236:f=(a>U32(m(b)));g;u 237:f=(a>U32(m(c)));g;u 238:f=(a>h(d));g;u 239:f=(a>U32(D[pc++]));g;u 255:O((pc=bQ+D[pc]+256*D[pc+1])>=bu)err();g;default:err();}B 1;}bL G U8 stdt[712]={64,128,128,128,128,128,127,128,127,128,127,127,127,127,126,126,126,126,126,125,125,124,125,124,123,123,123,123,122,122,121,121,120,120,119,119,118,118,118,116,117,115,116,114,114,113,113,112,112,111,110,110,109,108,108,107,106,106,105,104,104,102,103,101,101,100,99,98,98,97,96,96,94,94,94,92,92,91,90,89,89,88,87,86,86,84,84,84,82,82,81,80,79,79,78,77,76,76,75,74,73,73,72,71,70,70,69,68,67,67,66,65,65,64,63,62,62,61,61,59,59,59,57,58,56,56,55,54,54,53,52,52,51,51,50,49,49,48,48,47,47,45,46,44,45,43,43,43,42,41,41,40,40,40,39,38,38,37,37,36,36,36,35,34,34,34,33,32,33,32,31,31,30,31,29,30,28,29,28,28,27,27,27,26,26,25,26,24,25,24,24,23,23,23,23,22,22,21,22,21,20,21,20,19,20,19,19,19,18,18,18,18,17,17,17,17,16,16,16,16,15,15,15,15,15,14,14,14,14,13,14,13,13,13,12,13,12,12,12,11,12,11,11,11,11,11,10,11,10,10,10,10,9,10,9,9,9,9,9,8,9,8,9,8,8,8,7,8,8,7,7,8,7,7,7,6,7,7,6,6,7,6,6,6,6,6,6,5,6,5,6,5,5,5,5,5,5,5,5,5,4,5,4,5,4,4,5,4,4,4,4,4,4,3,4,4,3,4,4,3,3,4,3,3,3,4,3,3,3,3,3,3,2,3,3,3,2,3,2,3,3,2,2,3,2,2,3,2,2,2,2,3,2,2,2,2,2,2,1,2,2,2,2,1,2,2,2,1,2,1,2,2,1,2,1,2,1,1,2,1,1,2,1,1,2,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,1,1,1,0,1,1,1,0,1,1,0,1,1,0,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0
};P::P(ZP&zr):
c8(1),du(1),z(zr){pcode=0;pcode_size=0;initTables=dp;}P::~P(){allocx(pcode,pcode_size,0);}K P::cP(){allocx(pcode,pcode_size,0);z.inith();O(!initTables&&ds()){initTables=cD;dt2k[0]=0;Q(I i=1;i<256;++i){dt2k[i]=2048/i;}Q(I i=0;i<1024;++i){dt[i]=(1<<17)/(i*2+3)*2;}memset(squasht,0,(1376+7)*2);Q(I i=1376+7;i<1376+1344;++i){squasht[i]=static_cast<U16>(32768.0 / (1+std::exp((i - 2048)* (-1.0 / 64))));}Q(I i=2720;i<4096;++i)squasht[i]=32767;I k=16384;Q(I i=0;i<712;++i)
Q(I j=stdt[i];j>0;--j)
stretcht[k++]=i;Q(I i=0;i<16384;++i)
stretcht[i]=-stretcht[32767-i];}Q(I i=0;i<256;++i)h[i]=p[i]=0;Q(I i=0;i<256;++i)
comp[i].cP();I n=z.D[6];G U8* cp=&z.D[7];Q(I i=0;i<n;++i){dr&cr=comp[i];switch(cp[0]){u CONS:
p[i]=(cp[1]-128)*4;g;u CM:
O(cp[1]>32)E(0);cr.cm.bl(1,cp[1]);cr.z=cp[2]*4;Q(bh j=0;j<cr.cm.be();++j)
cr.cm[j]=0x80000000;g;u ICM:
O(cp[1]>26)E(0);cr.z=1023;cr.cm.bl(256);cr.ht.bl(64,cp[1]);Q(bh j=0;j<cr.cm.be();++j)
cr.cm[j]=st.cminit(j);g;u MATCH:
O(cp[1]>32||cp[2]>32)E(0);cr.cm.bl(1,cp[1]);cr.ht.bl(1,cp[2]);cr.ht(0)=1;g;u AVG:
O(cp[1]>=i)E(0);O(cp[2]>=i)E(0);g;u MIX2:
O(cp[1]>32)E(0);O(cp[3]>=i)E(0);O(cp[2]>=i)E(0);cr.c=(bh(1)<<cp[1]);cr.a16.bl(1,cp[1]);Q(bh j=0;j<cr.a16.be();++j)
cr.a16[j]=32768;g;u MIX:{O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]<1||cp[3]>i-cp[2])E(0);I m=cp[3];cr.c=(bh(1)<<cp[1]);cr.cm.bl(m,cp[1]);Q(bh j=0;j<cr.cm.be();++j)
cr.cm[j]=65536/m;g;}u ISSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);cr.ht.bl(64,cp[1]);cr.cm.bl(512);Q(I j=0;j<256;++j){cr.cm[j*2]=1<<15;cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);}g;u SSE:
O(cp[1]>32)E(0);O(cp[2]>=i)E(0);O(cp[3]>cp[4]*4)E(0);cr.cm.bl(32,cp[1]);cr.z=cp[4]*4;Q(bh j=0;j<cr.cm.be();++j)
cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];g;default:E(0);}cp+=compsize[*cp];}}I P::predict0(){I n=z.D[6];G U8* cp=&z.D[7];Q(I i=0;i<n;++i){dr&cr=comp[i];switch(cp[0]){u CONS:
g;u CM:
cr.cxt=h[i]^du;p[i]=stretch(cr.cm(cr.cxt)>>17);g;u ICM:
O(c8==1||(c8&0xf0)==16)cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(du&15)];p[i]=stretch(cr.cm(cr.cxt)>>8);g;u MATCH:
O(cr.a==0)p[i]=0;Y{cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1;p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);}g;u AVG:
p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;g;u MIX2:{cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));I w=cr.a16[cr.cxt];p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;}g;u MIX:{I m=cp[3];cr.cxt=h[i]+(c8&cp[5]);cr.cxt=(cr.cxt&(cr.c-1))*m;I* wt=(I*)&cr.cm[cr.cxt];p[i]=0;Q(I j=0;j<m;++j)
p[i]+=(wt[j]>>8)*p[cp[2]+j];p[i]=clamp2k(p[i]>>8);}g;u ISSE:{O(c8==1||(c8&0xf0)==16)
cr.c=find(cr.ht,cp[1]+2,h[i]+16*c8);cr.cxt=cr.ht[cr.c+(du&15)];I *wt=(I*)&cr.cm[cr.cxt*2];p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);}g;u SSE:{cr.cxt=(h[i]+c8)*32;I pq=p[cp[2]]+992;O(pq<0)pq=0;O(pq>1983)pq=1983;I wt=pq&63;pq>>=6;cr.cxt+=pq;p[i]=stretch(((cr.cm(cr.cxt)>>10)*(64-wt)+(cr.cm(cr.cxt+1)>>10)*wt)>>13);cr.cxt+=wt>>5;}g;default:
E(0);}cp+=compsize[cp[0]];}B squash(p[n-1]);}K P::update0(I y){G U8* cp=&z.D[7];I n=z.D[6];Q(I i=0;i<n;++i){dr&cr=comp[i];switch(cp[0]){u CONS:
g;u CM:
train(cr,y);g;u ICM:{cr.ht[cr.c+(du&15)]=st.bF(cr.ht[cr.c+(du&15)],y);U32&pn=cr.cm(cr.cxt);pn+=I(y*32767-(pn>>8))>>2;}g;u MATCH:{O(I(cr.c)!=y)cr.a=0;cr.ht(cr.z)+=cr.ht(cr.z)+y;O(++cr.cxt==8){cr.cxt=0;++cr.z;cr.z&=(1<<cp[2])-1;O(cr.a==0){cr.b=cr.z-cr.cm(h[i]);O(cr.b&(cr.ht.be()-1))
ba(cr.a<255
&&cr.ht(cr.z-cr.a-1)==cr.ht(cr.z-cr.a-cr.b-1))
++cr.a;}Y cr.a+=cr.a<255;cr.cm(h[i])=cr.z;}}g;u AVG:
g;u MIX2:{I err=(y*32767-squash(p[i]))*cp[4]>>5;I w=cr.a16[cr.cxt];w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;O(w<0)w=0;O(w>65535)w=65535;cr.a16[cr.cxt]=w;}g;u MIX:{I m=cp[3];I err=(y*32767-squash(p[i]))*cp[4]>>4;I* wt=(I*)&cr.cm[cr.cxt];Q(I j=0;j<m;++j)
wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));}g;u ISSE:{I err=y*32767-squash(p[i]);I *wt=(I*)&cr.cm[cr.cxt*2];wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));wt[1]=clamp512k(wt[1]+((err+16)>>5));cr.ht[cr.c+(du&15)]=st.bF(cr.cxt,y);}g;u SSE:
train(cr,y);g;default:
assert(0);}cp+=compsize[cp[0]];}c8+=c8+y;O(c8>=256){z.run(c8-256);du=1;c8=1;Q(I i=0;i<n;++i)h[i]=z.H(i);}Y O(c8>=16&&c8<32)
du=(du&0xf)<<5|y<<4|1;Y
du=(du&0x1f0)|(((du&0xf)*2+y)&0xf);}dg::dg(ZP&z):
in(0),low(1),cR(0xFFFFFFFF),bY(0),bT(0),bC(0),pr(z),buf(cv){}K dg::cP(){pr.cP();O(pr.ds())low=1,cR=0xFFFFFFFF,bY=0;Y low=cR=bY=0;}I dg::decode(I p){O(bY<low||bY>cR)E(0);U32 mid=low+U32(((cR-low)*U64(U32(p)))>>16);I y;O(bY<=mid)y=1,cR=mid;Y y=0,low=mid+1;ba((cR^low)<0x1000000){cR=cR<<8|255;low=low<<8;low+=(low==0);I c=get();O(c<0)E(0);bY=bY<<8|c;}B y;}I dg::decompress(){O(pr.ds()){O(bY==0){Q(I i=0;i<4;++i)
bY=bY<<8|get();}O(decode(0)){O(bY!=0)E(0);B -1;}Y{I c=1;ba(c<256){I p=pr.predict()*2+1;c+=c+decode(p);pr.update(c&1);}B c-256;}}Y{O(bY==0){Q(I i=0;i<4;++i)bY=bY<<8|get();O(bY==0)B -1;}--bY;B get();}}I dg::de(){I c=-1;O(pr.ds()){ba(bY==0)
bY=get();ba(bY&&(c=get())>=0)
bY=bY<<8|c;ba((c=get())==0);B c;}Y{O(bY==0)
Q(I i=0;i<4&&(c=get())>=0;++i)bY=bY<<8|c;ba(bY>0){ba(bY>0){--bY;O(get()<0)B E("skipped to EOF"),-1;}Q(I i=0;i<4&&(c=get())>=0;++i)bY=bY<<8|c;}O(c>=0)c=get();B c;}}K dh::cP(I h,I m){bi=ce=0;ph=h;pm=m;z.clear();}I dh::bq(I c){switch (bi){u 0:
O(c<0)E(0);bi=c+1;O(bi>2)E(0);O(bi==1)z.clear();g;u 1:
z.outc(c);g;u 2:
O(c<0)E(0);ce=c;bi=3;g;u 3:
O(c<0)E(0);ce+=c*256;O(ce<1)E(0);z.D.bl(ce+300);z.cn=8;z.bQ=z.bu=z.cn+128;z.D[4]=ph;z.D[5]=pm;bi=4;g;u 4:
O(c<0)E(0);z.D[z.bu++]=c;O(z.bu-z.bQ==ce){ce=z.cn-2+z.bu-z.bQ;z.D[0]=ce&255;z.D[1]=ce>>8;z.initp();bi=5;}g;u 5:
z.run(c);O(c<0)z.flush();g;}B bi;}bV D::findBlock(double* memptr){I c=dec.get();O(c!=1&&c!=2)E(0);z.read(&dec);O(c==1&&z.D.isize()>6&&z.D[6]==0)
E(0);O(memptr)*memptr=z.memory();bi=FILENAME;cy=FIRSTSEG;B cD;}bV D::findFilename(bz* cK){bi=COMMENT;B cD;}K D::readComment(bz* comment){bi=DATA;}bV D::decompress(I n){O(cy==SKIP)E(0);O(cy==FIRSTSEG){dec.cP();pp.cP(z.D[4],z.D[5]);cy=SEG;}ba((pp.getState()&3)!=1)
pp.bq(dec.decompress());ba(n){I c=dec.decompress();pp.bq(c);O(c==-1){bi=SEGEND;B dp;}O(n>0)--n;}B cD;}K D::readSegmentEnd(V* cc){I c=0;O(bi==DATA){c=dec.de();cy=SKIP;}Y O(bi==SEGEND)
c=dec.get();bi=FILENAME;O(c==254){O(cc)cc[0]=0;}Y O(c==253){O(cc)cc[0]=1;Q(I i=1;i<=20;++i){c=dec.get();O(cc)cc[i]=c;}}Y
E(0);}K decompress(cg* in,bz* out){D d;d.setInput(in);d.setOutput(out);if(d.findBlock()){d.decompress();}}K Encoder::cP(){low=1;cR=0xFFFFFFFF;pr.cP();O(!pr.ds())low=0,buf.bl(1<<16);}K Encoder::encode(I y,I p){U32 mid=low+U32(((cR-low)*U64(U32(p)))>>16);O(y)cR=mid;Y low=mid+1;ba((cR^low)<0x1000000){out->put(cR>>24);cR=cR<<8|255;low=low<<8;low+=(low==0);}}K Encoder::compress(I c){O(pr.ds()){O(c==-1)
encode(1,0);Y{encode(0,0);Q(I i=7;i>=0;--i){I p=pr.predict()*2+1;I y=c>>i&1;encode(y,p);pr.update(y);}}}Y{E(0);}}G V* compname[256]={"","G","cm","icm","match","avg","mix2","mix","isse","sse",0};G V* opcodelist[272]={"E","a++","a--","a!","a=0","","","a=r","b<>a","b++","b--","b!","b=0","","","b=r","c<>a","c++","c--","c!","c=0","","","c=r","d<>a","d++","d--","d!","d=0","","","d=r","*b<>a","*b++","*b--","*b!","*b=0","","","jt","*c<>a","*c++","*c--","*c!","*c=0","","","jf","*d<>a","*d++","*d--","*d!","*d=0","","","r=a","halt","out","","hash","hashd","","","jmp","a=a","a=b","a=c","a=d","a=*b","a=*c","a=*d","a=","b=a","b=b","b=c","b=d","b=*b","b=*c","b=*d","b=","c=a","c=b","c=c","c=d","c=*b","c=*c","c=*d","c=","d=a","d=b","d=c","d=d","d=*b","d=*c","d=*d","d=","*b=a","*b=b","*b=c","*b=d","*b=*b","*b=*c","*b=*d","*b=","*c=a","*c=b","*c=c","*c=d","*c=*b","*c=*c","*c=*d","*c=","*d=a","*d=b","*d=c","*d=d","*d=*b","*d=*c","*d=*d","*d=","","","","","","","","","a+=a","a+=b","a+=c","a+=d","a+=*b","a+=*c","a+=*d","a+=","a-=a","a-=b","a-=c","a-=d","a-=*b","a-=*c","a-=*d","a-=","a*=a","a*=b","a*=c","a*=d","a*=*b","a*=*c","a*=*d","a*=","a/=a","a/=b","a/=c","a/=d","a/=*b","a/=*c","a/=*d","a/=","a%=a","a%=b","a%=c","a%=d","a%=*b","a%=*c","a%=*d","a%=","a&=a","a&=b","a&=c","a&=d","a&=*b","a&=*c","a&=*d","a&=","a&~a","a&~b","a&~c","a&~d","a&~*b","a&~*c","a&~*d","a&~","a|=a","a|=b","a|=c","a|=d","a|=*b","a|=*c","a|=*d","a|=","a^=a","a^=b","a^=c","a^=d","a^=*b","a^=*c","a^=*d","a^=","a<<=a","a<<=b","a<<=c","a<<=d","a<<=*b","a<<=*c","a<<=*d","a<<=","a>>=a","a>>=b","a>>=c","a>>=d","a>>=*b","a>>=*c","a>>=*d","a>>=","a==a","a==b","a==c","a==d","a==*b","a==*c","a==*d","a==","a<a","a<b","a<c","a<d","a<*b","a<*c","a<*d","a<","a>a","a>b","a>c","a>d","a>*b","a>*c","a>*d","a>","","","","","","","","","","","","","","","","lj","post","cV","end","O","ifnot","Y","endif","do","ba","until","Qever","ifl","ifnotl","Yl",";",0};K cq::bF(){Q(;*in;++in){O(*in=='\n')++line;O(*in=='(')bi+=1+(bi<0);Y O(bi>0&&*in==')')--bi;Y O(bi<0&&*in<=' ')bi=0;Y O(bi==0&&*in>' '){bi=-1;g;}}O(!*in)E(0);}I tolower(I c){B(c>='A'&&c<='Z')? c+'a'-'A':c;}
bV cq::matchToken(G V* word){G V* a=in;Q(;(*a>' '&&*a!='('&&*word);++a,++word)
O(tolower(*a)!=tolower(*word))B dp;B !*word&&(*a<=' '||*a=='(');}K cq::SE(G V* msg,G V* expected){E(0);}I cq::rtoken(G V* list[]){bF();Q(I i=0;list[i];++i)
O(matchToken(list[i]))
B i;SE(0);B -1;}K cq::rtoken(G V* s){bF();O(!matchToken(s))SE(0,s);}I cq::rtoken(I low,I cR){bF();I r=0;O(in[0]=='$'&&in[1]>='1'&&in[1]<='9'){O(in[2]=='+')r=atoi(in+3);O(bm)r+=bm[in[1]-'1'];}Y O(in[0]=='-'||(in[0]>='0'&&in[0]<='9'))r=atoi(in);Y SE(0);O(r<low)SE(0);O(r>cR)SE(0);B r;}I cq::compile_comp(ZP&z){I op=0;G I comp_begin=z.bu;ba(cD){op=rtoken(opcodelist);O(op==POST||op==PCOMP||op==END)g;I bW=-1;I operand2=-1;O(op==IF){op=JF;bW=0;dv.push(z.bu+1);}Y O(op==IFNOT){op=JT;bW=0;dv.push(z.bu+1);}Y O(op==IFL||op==IFNOTL){O(op==IFL)z.D[z.bu++]=(JT);O(op==IFNOTL)z.D[z.bu++]=(JF);z.D[z.bu++]=(3);op=LJ;bW=operand2=0;dv.push(z.bu+1);}Y O(op==ELSE||op==ELSEL){O(op==ELSE)op=JMP,bW=0;O(op==ELSEL)op=LJ,bW=operand2=0;I a=dv.pop();O(z.D[a-1]!=LJ){I j=z.bu-a+1+(op==LJ);O(j>127)SE("O too big,try IFL,IFNOTL");z.D[a]=j;}Y{I j=z.bu-comp_begin+2+(op==LJ);z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}dv.push(z.bu+1);}Y O(op==ENDIF){I a=dv.pop();I j=z.bu-a-1;O(z.D[a-1]!=LJ){O(j>127)SE("O too big,try IFL,IFNOTL,ELSEL\n");z.D[a]=j;}Y{j=z.bu-comp_begin;z.D[a]=j&255;z.D[a+1]=(j>>8)&255;}}Y O(op==DO){do_stack.push(z.bu);}Y O(op==WHILE||op==UNTIL||op==QEVER){I a=do_stack.pop();I j=a-z.bu-2;O(j>=-127){O(op==WHILE)op=JT;O(op==UNTIL)op=JF;O(op==QEVER)op=JMP;bW=j&255;}Y{j=a-comp_begin;O(op==WHILE){z.D[z.bu++]=(JF);z.D[z.bu++]=(3);}O(op==UNTIL){z.D[z.bu++]=(JT);z.D[z.bu++]=(3);}op=LJ;bW=j&255;operand2=j>>8;}}Y O((op&7)==7){O(op==LJ){bW=rtoken(0,65535);operand2=bW>>8;bW&=255;}Y O(op==JT||op==JF||op==JMP){bW=rtoken(-128,127);bW&=255;}Y
bW=rtoken(0,255);}O(op>=0&&op<=255)
z.D[z.bu++]=(op);O(bW>=0)
z.D[z.bu++]=(bW);O(operand2>=0)
z.D[z.bu++]=(operand2);O(z.bu>=z.D.isize()-130||z.bu-z.bQ+z.cn-2>65535)
SE("program too big");}z.D[z.bu++]=(0);B op;}cq::cq(G V* in_,I* args_,ZP&hz_,ZP&pz_,bz* out2_):in(in_),bm(args_),hz(hz_),pz(pz_),out2(out2_),dv(1000),do_stack(1000){line=1;bi=0;hz.clear();pz.clear();hz.D.bl(68000);rtoken("comp");hz.D[2]=rtoken(0,255);hz.D[3]=rtoken(0,255);hz.D[4]=rtoken(0,255);hz.D[5]=rtoken(0,255);G I n=hz.D[6]=rtoken(0,255);hz.cn=7;Q(I i=0;i<n;++i){rtoken(i,i);CompType bj=CompType(rtoken(compname));hz.D[hz.cn++]=bj;I clen=LQ::compsize[bj&255];O(clen<1||clen>10)SE("invalid component");Q(I j=1;j<clen;++j)
hz.D[hz.cn++]=rtoken(0,255);}hz.D[hz.cn++];hz.bQ=hz.bu=hz.cn+128;rtoken("bR");I op=compile_comp(hz);I ce=hz.cn-2+hz.bu-hz.bQ;hz.D[0]=ce&255;hz.D[1]=ce>>8;O(op==POST){rtoken(0,0);rtoken("end");}Y O(op==PCOMP){pz.D.bl(68000);pz.D[4]=hz.D[4];pz.D[5]=hz.D[5];pz.cn=8;pz.bQ=pz.bu=pz.cn+128;bF();ba(*in&&*in!=';'){O(out2)
out2->put(*in);++in;}O(*in)++in;op=compile_comp(pz);I len=pz.cn-2+pz.bu-pz.bQ;pz.D[0]=len&255;pz.D[1]=len>>8;O(op!=END)
SE("expected END");}Y O(op!=END)
SE("expected END or POST 0 END or PCOMP cmd ;... END");}bB MemoryReader:bE cg{G V* p;bE:MemoryReader(G V* p_):p(p_){}I get(){B *p++&255;}};K C::startBlock(G V* config,I* bm,bz* pcomp_cmd){cq(config,bm,z,pz,pcomp_cmd);enc.out->put(1+(z.D[6]==0));z.bq(enc.out,dp);bi=BLOCK1;}K C::startSegment(G V* cK,G V* comment){O(bi==BLOCK1)bi=SEG1;O(bi==BLOCK2)bi=SEG2;}K C::postProcess(G V* cV,I len){O(bi==SEG2)B;enc.cP();O(!cV){len=pz.bu-pz.bQ;O(len>0){cV=(G V*)&pz.D[pz.bQ];}}Y O(len==0){len=toU16(cV);cV+=2;}O(len>0){enc.compress(1);enc.compress(len&255);enc.compress((len>>8)&255);Q(I i=0;i<len;++i)
enc.compress(cV[i]&255);}Y
enc.compress(0);bi=SEG2;}bV C::compress(I n){O(bi==SEG1)
postProcess();G I cv=1<<14;V buf[cv];ba(n){I nbuf=cv;O(n>=0&&n<nbuf)nbuf=n;I nr=in->read(buf,nbuf);O(nr<0||nr>cv||nr>nbuf)E(0);O(nr<=0)B dp;O(n>=0)n-=nr;Q(I i=0;i<nr;++i){I ch=U8(buf[i]);enc.compress(ch);}}B cD;}K C::endSegment(G V* cc){O(bi==SEG1)
postProcess();enc.compress(-1);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(0);enc.out->put(254);bi=BLOCK2;}K compress(cg* in,bz* out,G V* ME,G V* cK,G V* comment,bV dosha1){I bs=4;O(ME&&ME[0]&&ME[1]>='0'&&ME[1]<='9'){bs=ME[1]-'0';O(ME[2]>='0'&&ME[2]<='9')bs=bs*10+ME[2]-'0';O(bs>11)bs=11;}bs=(0x100000<<bs)-4096;SB sb(bs);sb.bq(0,bs);I n=0;ba(in&&(n=in->read((V*)sb.bX(),bs))>0){sb.bl(n);compressBlock(&sb,out,ME,cK,comment,dosha1);cK=0;comment=0;sb.bl(0);}}I P::predict(){B predict0();}K P::update(I y){update0(y);}K ZP::run(U32 input){pc=bQ;a=input;ba(execute());}
#define AS 256
#define BUCKET_A_SIZE AS
#define BUCKET_B_SIZE (AS*AS)
#define SSIT 8
#define SSB 1024
#define SS_MISORT_STACKSIZE 16
#define SS_SMERGE_STACKSIZE 32
#define TR_INSERTIONSORT_THRESHOLD 8
#define TR_STACKSIZE 64
#define bH(_a,_b)do{t=(_a);(_a)= (_b);(_b)= t;}ba(0)
#define MIN(_a,_b)(((_a)< (_b))? (_a):(_b))
#define MAX(_a,_b)(((_a)> (_b))? (_a):(_b))
#define SP(_a,_b,_c,_d)\
do{\
assert(bv<bU);\
bG[bv].a=(_a),bG[bv].b=(_b),\
bG[bv].c=(_c),bG[bv++].d=(_d);\
}ba(0)
#define S5(_a,_b,_c,_d,_e)\
do{\
assert(bv<bU);\
bG[bv].a=(_a),bG[bv].b=(_b),\
bG[bv].c=(_c),bG[bv].d=(_d),bG[bv++].e=(_e);\
}ba(0)
#define STACK_POP(_a,_b,_c,_d)\
do{\
assert(0<=bv);\
O(bv==0){B;}\
(_a)= bG[--bv].a,(_b)= bG[bv].b,\
(_c)= bG[bv].c,(_d)= bG[bv].d;\
}ba(0)
#define cu(_a,_b,_c,_d,_e)\
do{\
assert(0<=bv);\
O(bv==0){B;}\
(_a)= bG[--bv].a,(_b)= bG[bv].b,\
(_c)= bG[bv].c,(_d)= bG[bv].d,(_e)= bG[bv].e;\
}ba(0)
#define cZ(_c0)cL[(_c0)]
#define cz(_c0,_c1)(cA[((_c1)<< 8)| (_c0)])
#define BB(_c0,_c1)(cA[((_c0)<< 8)| (_c1)])
#define R(v)v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
bL G I cB[256]={-1,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,R(4),R(5),R(5),R(6),R(6),R(6),R(6),R(7),R(7),R(7),R(7),R(7),R(7),R(7),R(7),};
#undef R
I
di(I n){B(n&0xff00)?
8+cB[(n >> 8)&0xff]:0+cB[(n >> 0)&0xff];}bL I dqq_table[256] ={0};I
ss_isqrt(I x){I y,e;O(x >= (SSB * SSB)){B SSB;}
e=(x&0xffff0000)?
((x&0xff000000)?
24+cB[(x >> 24)&0xff]:16+cB[(x >> 16)&0xff]):
((x&0x0000ff00)?
8+cB[(x >> 8)&0xff]:0+cB[(x >> 0)&0xff]);O(dqq_table[255] != 255){Q(I i=0;i<256;++i){dqq_table[i]=static_cast<I>(16 * sqrt(i));}}O(e >= 16){y=dqq_table[x >> ((e - 6)- (e&1))] << ((e >> 1)- 7);O(e >= 24){y=(y+1+x / y)>> 1;}
y=(y+1+x / y)>> 1;}Y O(e >= 8){y=(dqq_table[x >> ((e - 6)- (e&1))] >> (7 - (e >> 1)))+ 1;}Y{B dqq_table[x] >> 4;}B(x<(y * y))? y - 1:y;}I
cd(G J V *T,G I *p1,G I *p2,I W){G J V *U1,*U2,*U1n,*U2n;Q(U1=T+W+*p1,U2=T+W+*p2,U1n=T+*(p1+1)+ 2,U2n=T+*(p2+1)+ 2;(U1<U1n)&&(U2<U2n)&&(*U1==*U2);++U1,++U2){}B U1<U1n ?
(U2<U2n ? *U1 - *U2:1):
(U2<U2n ? -1:0);}bL
K
ss_insertionsort(G J V *T,G I *PA,I *F,I *L,I W){I *i,*j;I t;I r;Q(i=L - 2;F<=i;--i){Q(t=*i,j=i+1;0<(r=cd(T,PA+t,PA+*j,W));){do{*(j - 1)= *j;}ba((++j<L)&&(*j<0));O(L<=j){g;}}O(r==0){*j=~*j;}
*(j - 1)= t;}}K
ss_fixdown(G J V *Td,G I *PA,I *SA,I i,I be){I j,k;I v;I c,d,e;Q(v=SA[i],c=Td[PA[v]];(j=2 * i+1)< be;SA[i]=SA[k],i=k){d=Td[PA[SA[k=j++]]];O(d<(e=Td[PA[SA[j]]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bL
K
ss_heapsort(G J V *Td,G I *PA,I *SA,I be){I i,m;I t;m=be;O((be % 2)== 0){m--;O(Td[PA[SA[m / 2]]]<Td[PA[SA[m]]]){bH(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){ss_fixdown(Td,PA,SA,i,m);}
O((be % 2)== 0){bH(SA[0],SA[m]);ss_fixdown(Td,PA,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];ss_fixdown(Td,PA,SA,0,i);SA[i]=t;}}I *
ss_median3(G J V *Td,G I *PA,I *v1,I *v2,I *v3){I *t;O(Td[PA[*v1]]>Td[PA[*v2]]){bH(v1,v2);}
O(Td[PA[*v2]]>Td[PA[*v3]]){O(Td[PA[*v1]]>Td[PA[*v3]]){B v1;}
Y{B v3;}}B v2;}I *
ss_median5(G J V *Td,G I *PA,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(Td[PA[*v2]]>Td[PA[*v3]]){bH(v2,v3);}
O(Td[PA[*v4]]>Td[PA[*v5]]){bH(v4,v5);}
O(Td[PA[*v2]]>Td[PA[*v4]]){bH(v2,v4);bH(v3,v5);}
O(Td[PA[*v1]]>Td[PA[*v3]]){bH(v1,v3);}
O(Td[PA[*v1]]>Td[PA[*v4]]){bH(v1,v4);bH(v3,v5);}
O(Td[PA[*v3]]>Td[PA[*v4]]){B v4;}
B v3;}I *
ss_pivot(G J V *Td,G I *PA,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){B ss_median3(Td,PA,F,M,L - 1);}Y{t >>= 2;B ss_median5(Td,PA,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=ss_median3(Td,PA,F,F+t,F+(t << 1));M=ss_median3(Td,PA,M - t,M,M+t);L=ss_median3(Td,PA,L - 1 - (t << 1),L - 1 - t,L - 1);B ss_median3(Td,PA,F,M,L);}I *
ss_partition(G I *PA,I *F,I *L,I W){I *a,*b;I t;Q(a=F - 1,b=L;;){Q(;(++a<b)&&((PA[*a]+W)>= (PA[*a+1]+1));){*a=~*a;}
Q(;(a<--b)&&((PA[*b]+W)< (PA[*b+1]+1));){}O(b<=a){g;}
t=~*b;*b=*a;*a=t;}O(F<a){*F=~*F;}
B a;}bL
K
ss_mIrosort(G J V *T,G I *PA,I *F,I *L,I W){
#define bU SS_MISORT_STACKSIZE
bf{I *a,*b,c;I d;}bG[bU];G J V *Td;I *a,*b,*c,*d,*e,*f;I s,t;I bv;I bN;I v,x=0;Q(bv=0,bN=di(L - F);;){O((L - F)<= SSIT){O(1<(L - F)){ss_insertionsort(T,PA,F,L,W);}
STACK_POP(F,L,W,bN);dw;}Td=T+W;O(bN--==0){ss_heapsort(Td,PA,F,L - F);}
O(bN<0){Q(a=F+1,v=Td[PA[*F]];a<L;++a){O((x=Td[PA[*a]])!= v){O(1<(a - F)){g;}
v=x;F=a;}}O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,a,W);}O((a - F)<= (L - a)){O(1<(a - F)){SP(a,L,W,-1);L=a,W+=1,bN=di(a - F);}Y{F=a,bN=-1;}}Y{O(1<(L - a)){SP(F,a,W+1,di(a - F));F=a,bN=-1;}Y{L=a,W+=1,bN=di(a - F);}}dw;}a=ss_pivot(Td,PA,F,L);v=Td[PA[*a]];bH(*F,*a);Q(b=F;(++b<L)&&((x=Td[PA[*b]])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=Td[PA[*b]])<= v);){O(x==v){bH(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=Td[PA[*c]])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bH(*c,*d);--d;}}}Q(;b<c;){bH(*b,*c);Q(;(++b<c)&&((x=Td[PA[*b]])<= v);){O(x==v){bH(*b,*a);++a;}}Q(;(b<--c)&&((x=Td[PA[*c]])>= v);){O(x==v){bH(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bH(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bH(*e,*f);}
a=F+(b - a),c=L - (d - c);b=(v<=Td[PA[*a] - 1])? a:ss_partition(PA,a,c,W);O((a - F)<= (L - c)){O((L - c)<= (c - b)){SP(b,c,W+1,di(c - b));SP(c,L,W,bN);L=a;}Y O((a - F)<= (c - b)){SP(c,L,W,bN);SP(b,c,W+1,di(c - b));L=a;}Y{SP(c,L,W,bN);SP(F,a,W,bN);F=b,L=c,W+=1,bN=di(c - b);}}Y{O((a - F)<= (c - b)){SP(b,c,W+1,di(c - b));SP(F,a,W,bN);F=c;}Y O((L - c)<= (c - b)){SP(F,a,W,bN);SP(b,c,W+1,di(c - b));F=c;}Y{SP(F,a,W,bN);SP(c,L,W,bN);F=b,L=c,W+=1,bN=di(c - b);}}}Y{bN+=1;O(Td[PA[*F] - 1]<v){F=ss_partition(PA,F,L,W);bN=di(L - F);}W+=1;}}
#undef bU
}K
ss_blockswap(I *a,I *b,I n){I t;Q(;0<n;--n,++a,++b){t=*a,*a=*b,*b=t;}}K
ss_rotate(I *F,I *M,I *L){I *a,*b,t;I l,r;l=M - F,r=L - M;Q(;(0<l)&&(0<r);){O(l==r){ss_blockswap(F,M,l);g;}
O(l<r){a=L - 1,b=M - 1;t=*a;do{*a--=*b,*b--=*a;O(b<F){*a=t;L=a;O((r-=l+1)<= l){g;}
a-=1,b=M - 1;t=*a;}}ba(1);}Y{a=F,b=M;t=*a;do{*a++=*b,*b++=*a;O(L<=b){*a=t;F=a+1;O((l-=r+1)<= r){g;}
a+=1,b=M;t=*a;}}ba(1);}}}bL
K
ss_inplacemerge(G J V *T,G I *PA,I *F,I *M,I *L,I W){G I *p;I *a,*b;I len,half;I q,r;I x;Q(;;){O(*(L - 1)< 0){x=1;p=PA+~*(L - 1);}
Y{x=0;p=PA+*(L - 1);}
Q(a=F,len=M - F,half=len >> 1,r=-1;0<len;len=half,half >>= 1){b=a+half;q=cd(T,PA+((0<=*b)? *b:~*b),p,W);O(q<0){a=b+1;half-=(len&1)^ 1;}Y{r=q;}}O(a<M){O(r==0){*a=~*a;}
ss_rotate(a,M,L);L-=M - a;M=a;O(F==M){g;}}--L;O(x != 0){ba(*--L<0){}}
O(M==L){g;}}}bL
K
ss_mergeQward(G J V *T,G I *PA,I *F,I *M,I *L,I *buf,I W){I *a,*b,*c,*cM;I t;I r;cM=buf+(M - F)- 1;ss_blockswap(buf,F,M - F);Q(t=*(a=F),b=buf,c=M;;){r=cd(T,PA+*b,PA+*c,W);O(r<0){do{*a++=*b;O(cM<=b){*cM=t;B;}
*b++=*a;}ba(*b<0);}Y O(r > 0){do{*a++=*c,*c++=*a;O(L<=c){ba(b<cM){*a++=*b,*b++=*a;}
*a=*b,*b=t;B;}}ba(*c<0);}Y{*c=~*c;do{*a++=*b;O(cM<=b){*cM=t;B;}
*b++=*a;}ba(*b<0);do{*a++=*c,*c++=*a;O(L<=c){ba(b<cM){*a++=*b,*b++=*a;}
*a=*b,*b=t;B;}}ba(*c<0);}}}bL
K
ss_mergebackward(G J V *T,G I *PA,I *F,I *M,I *L,I *buf,I W){G I *p1,*p2;I *a,*b,*c,*cM;I t;I r;I x;cM=buf+(L - M)- 1;ss_blockswap(buf,M,L - M);x=0;O(*cM<0){p1=PA+~*cM;x |= 1;}
Y{p1=PA+*cM;}
O(*(M - 1)< 0){p2=PA+~*(M - 1);x |= 2;}
Y{p2=PA+*(M - 1);}
Q(t=*(a=L - 1),b=cM,c=M - 1;;){r=cd(T,p1,p2,W);O(0<r){O(x&1){do{*a--=*b,*b--=*a;}ba(*b<0);x^=1;}
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
Y{p2=PA+*c;}}}}bL
K
ss_swapmerge(G J V *T,G I *PA,I *F,I *M,I *L,I *buf,I ct,I W){
#define bU SS_SMERGE_STACKSIZE
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
bf{I *a,*b,*c;I d;}bG[bU];I *l,*r,*lm,*rm;I m,len,half;I bv;I dx,bF;Q(dx=0,bv=0;;){O((L - M)<= ct){O((F<M)&&(M<L)){ss_mergebackward(T,PA,F,M,L,buf,W);}MERGE_(F,L,dx);STACK_POP(F,M,L,dx);dw;}O((M - F)<= ct){O(F<M){ss_mergeQward(T,PA,F,M,L,buf,W);}MERGE_(F,L,dx);STACK_POP(F,M,L,dx);dw;}Q(m=0,len=MIN(M - F,L - M),half=len >> 1;0<len;len=half,half >>= 1){O(cd(T,PA+GETIDX(*(M+m+half)),PA+GETIDX(*(M - m - half - 1)),W)< 0){m+=half+1;half-=(len&1)^ 1;}}O(0<m){lm=M - m,rm=M+m;ss_blockswap(lm,M,m);l=r=M,bF=0;O(rm<L){O(*rm<0){*rm=~*rm;O(F<lm){Q(;*--l<0;){}bF |= 4;}
bF |= 1;}Y O(F<lm){Q(;*r<0;++r){}bF |= 2;}}O((l - F)<= (L - r)){SP(r,rm,L,(bF&3)| (dx&4));M=lm,L=l,dx=(dx&3)| (bF&4);}Y{O((bF&2)&&(r==M)){bF^=6;}
SP(F,lm,l,(dx&3)| (bF&4));F=r,M=rm,dx=(bF&3)| (dx&4);}}Y{O(cd(T,PA+GETIDX(*(M - 1)),PA+*M,W)== 0){*M=~*M;}MERGE_(F,L,dx);STACK_POP(F,M,L,dx);}}
#undef bU
}bL
K
sssort(G J V *T,G I *PA,I *F,I *L,I *buf,I ct,I W,I n,I Lsuffix){I *a;I *b,*M,*curbuf;I j,k,curbufsize,Z;I i;O(Lsuffix != 0){++F;}
O((ct<SSB)&&
(ct<(L - F))&&
(ct<(Z=ss_isqrt(L - F)))){O(SSB<Z){Z=SSB;}
buf=M=L - Z,ct=Z;}Y{M=L,Z=0;}Q(a=F,i=0;SSB<(M - a);a+=SSB,++i){ss_mIrosort(T,PA,a,a+SSB,W);curbufsize=L - (a+SSB);curbuf=a+SSB;O(curbufsize<=ct){curbufsize=ct,curbuf=buf;}
Q(b=a,k=SSB,j=i;j&1;b-=k,k <<= 1,j >>= 1){ss_swapmerge(T,PA,b - k,b,b+k,curbuf,curbufsize,W);}}ss_mIrosort(T,PA,a,M,W);Q(k=SSB;i != 0;k <<= 1,i >>= 1){O(i&1){ss_swapmerge(T,PA,a - k,a,M,buf,ct,W);a-=k;}}O(Z != 0){ss_mIrosort(T,PA,M,L,W);ss_inplacemerge(T,PA,F,M,L,W);}O(Lsuffix != 0){I PAi[2];PAi[0]=PA[*(F - 1)],PAi[1]=n - 2;Q(a=F,i=*(F - 1);(a<L)&&((*a<0)|| (0<cd(T,&(PAi[0]),PA+*a,W)));++a){*(a - 1)= *a;}*(a - 1)= i;}}I
dj(I n){B(n&0xffff0000)?
((n&0xff000000)?
24+cB[(n >> 24)&0xff]:16+cB[(n >> 16)&0xff]):
((n&0x0000ff00)?
8+cB[(n >> 8)&0xff]:0+cB[(n >> 0)&0xff]);}bL
K
tr_insertionsort(G I *X,I *F,I *L){I *a,*b;I t,r;Q(a=F+1;a<L;++a){Q(t=*a,b=a - 1;0 > (r=X[t] - X[*b]);){do{*(b+1)= *b;}ba((F<=--b)&&(*b<0));O(b<F){g;}}O(r==0){*b=~*b;}
*(b+1)= t;}}K
tr_fixdown(G I *X,I *SA,I i,I be){I j,k;I v;I c,d,e;Q(v=SA[i],c=X[v];(j=2 * i+1)< be;SA[i]=SA[k],i=k){d=X[SA[k=j++]];O(d<(e=X[SA[j]])){k=j;d=e;}
O(d<=c){g;}}SA[i]=v;}bL
K
tr_heapsort(G I *X,I *SA,I be){I i,m;I t;m=be;O((be % 2)== 0){m--;O(X[SA[m / 2]]<X[SA[m]]){bH(SA[m],SA[m / 2]);}}Q(i=m / 2 - 1;0<=i;--i){tr_fixdown(X,SA,i,m);}
O((be % 2)== 0){bH(SA[0],SA[m]);tr_fixdown(X,SA,0,m);}
Q(i=m - 1;0<i;--i){t=SA[0],SA[0]=SA[i];tr_fixdown(X,SA,0,i);SA[i]=t;}}I *
tr_median3(G I *X,I *v1,I *v2,I *v3){I *t;O(X[*v1]>X[*v2]){bH(v1,v2);}
O(X[*v2]>X[*v3]){O(X[*v1]>X[*v3]){B v1;}
Y{B v3;}}B v2;}I *
tr_median5(G I *X,I *v1,I *v2,I *v3,I *v4,I *v5){I *t;O(X[*v2]>X[*v3]){bH(v2,v3);}
O(X[*v4]>X[*v5]){bH(v4,v5);}
O(X[*v2]>X[*v4]){bH(v2,v4);bH(v3,v5);}
O(X[*v1]>X[*v3]){bH(v1,v3);}
O(X[*v1]>X[*v4]){bH(v1,v4);bH(v3,v5);}
O(X[*v3]>X[*v4]){B v4;}
B v3;}I *
tr_pivot(G I *X,I *F,I *L){I *M;I t;t=L - F;M=F+t / 2;O(t<=512){O(t<=32){B tr_median3(X,F,M,L - 1);}Y{t >>= 2;B tr_median5(X,F,F+t,M,L - 1 - t,L - 1);}}t >>= 3;F=tr_median3(X,F,F+t,F+(t << 1));M=tr_median3(X,M - t,M,M+t);L=tr_median3(X,L - 1 - (t << 1),L - 1 - t,L - 1);B tr_median3(X,F,M,L);}typedef bf _tr tr;bf _tr{I c;I r;I i;I n;};K
trbudget_init(tr *b,I c,I i){b->c=c;b->r=b->i=i;}I
trbudget_check(tr *b,I be){O(be<=b->r){b->r-=be;B 1;}
O(b->c==0){b->n+=be;B 0;}
b->r+=b->i - be;b->c-=1;B 1;}K
tr_partition(G I *X,I *F,I *M,I *L,I **pa,I **pb,I v){I *a,*b,*c,*d,*e,*f;I t,s;I x=0;Q(b=M - 1;(++b<L)&&((x=X[*b])== v);){}O(((a=b)< L)&&(x<v)){Q(;(++b<L)&&((x=X[*b])<= v);){O(x==v){bH(*b,*a);++a;}}}Q(c=L;(b<--c)&&((x=X[*c])== v);){}O((b<(d=c))&&(x > v)){Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bH(*c,*d);--d;}}}Q(;b<c;){bH(*b,*c);Q(;(++b<c)&&((x=X[*b])<= v);){O(x==v){bH(*b,*a);++a;}}Q(;(b<--c)&&((x=X[*c])>= v);){O(x==v){bH(*c,*d);--d;}}}O(a<=d){c=b - 1;O((s=a - F)> (t=b - a)){s=t;}
Q(e=F,f=b - s;0<s;--s,++e,++f){bH(*e,*f);}
O((s=d - c)> (t=L - d - 1)){s=t;}
Q(e=b,f=L - s;0<s;--s,++e,++f){bH(*e,*f);}
F+=(b - a),L-=(d - c);}*pa=F,*pb=L;}bL
K
tr_copy(I *ISA,G I *SA,I *F,I *a,I *b,I *L,I W){I *c,*d,*e;I s,v;v=b - SA - 1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - W))&&(ISA[s]==v)){*++d=s;ISA[s]=d - SA;}}Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - W))&&(ISA[s]==v)){*--d=s;ISA[s]=d - SA;}}}bL
K
tr_partialcopy(I *ISA,G I *SA,I *F,I *a,I *b,I *L,I W){I *c,*d,*e;I s,v;I rank,Lrank,newrank=-1;v=b - SA - 1;Lrank=-1;Q(c=F,d=a - 1;c<=d;++c){O((0<=(s=*c - W))&&(ISA[s]==v)){*++d=s;rank=ISA[s+W];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}Lrank=-1;Q(e=d;F<=e;--e){rank=ISA[*e];O(Lrank != rank){Lrank=rank;newrank=e - SA;}
O(newrank != rank){ISA[*e]=newrank;}}Lrank=-1;Q(c=L - 1,e=d+1,d=b;e<d;--c){O((0<=(s=*c - W))&&(ISA[s]==v)){*--d=s;rank=ISA[s+W];O(Lrank != rank){Lrank=rank;newrank=d - SA;}
ISA[s]=newrank;}}}bL
K
tr_Irosort(I *ISA,G I *X,I *SA,I *F,I *L,tr *T){
#define bU TR_STACKSIZE
bf{G I *a;I *b,*c;I d,e;}bG[bU];I *a,*b,*c;I t;I v,x=0;I dn=X - ISA;I Z,bF;I bv,TR=-1;Q(bv=0,Z=dj(L - F);;){O(Z<0){O(Z==-1){tr_partition(X - dn,F,F,L,&a,&b,L - SA - 1);O(a<L){Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}}O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}O(1<(b - a)){S5(NULL,a,b,0,0);S5(X - dn,F,L,-2,TR);TR=bv - 2;}O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,dj(L - b),TR);L=a,Z=dj(a - F);}Y O(1<(L - b)){F=b,Z=dj(L - b);}Y{cu(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,dj(a - F),TR);F=b,Z=dj(L - b);}Y O(1<(a - F)){L=a,Z=dj(a - F);}Y{cu(X,F,L,Z,TR);}}}Y O(Z==-2){a=bG[--bv].b,b=bG[bv].c;O(bG[bv].d==0){tr_copy(ISA,SA,F,a,b,L,X - ISA);}Y{O(0<=TR){bG[TR].d=-1;}
tr_partialcopy(ISA,SA,F,a,b,L,X - ISA);}cu(X,F,L,Z,TR);}Y{O(0<=*F){a=F;do{ISA[*a]=a - SA;}ba((++a<L)&&(0<=*a));F=a;}O(F<L){a=F;do{*a=~*a;}ba(*++a<0);bF=(ISA[*a] != X[*a])? dj(a - F+1):-1;O(++a<L){Q(b=F,v=a - SA - 1;b<a;++b){ISA[*b]=v;}}
O(trbudget_check(T,a - F)){O((a - F)<= (L - a)){S5(X,a,L,-3,TR);X+=dn,L=a,Z=bF;}Y{O(1<(L - a)){S5(X+dn,F,a,bF,TR);F=a,Z=-3;}Y{X+=dn,L=a,Z=bF;}}}Y{O(0<=TR){bG[TR].d=-1;}
O(1<(L - a)){F=a,Z=-3;}Y{cu(X,F,L,Z,TR);}}}Y{cu(X,F,L,Z,TR);}}dw;}O((L - F)<= TR_INSERTIONSORT_THRESHOLD){tr_insertionsort(X,F,L);Z=-3;dw;}O(Z--==0){tr_heapsort(X,F,L - F);Q(a=L - 1;F<a;a=b){Q(x=X[*a],b=a - 1;(F<=b)&&(X[*b]==x);--b){*b=~*b;}}Z=-3;dw;}a=tr_pivot(X,F,L);bH(*F,*a);v=X[*F];tr_partition(X,F,F+1,L,&a,&b,v);O((L - F)!= (b - a)){bF=(ISA[*a] != v)? dj(b - a):-1;Q(c=F,v=a - SA - 1;c<a;++c){ISA[*c]=v;}
O(b<L){Q(c=a,v=b - SA - 1;c<b;++c){ISA[*c]=v;}}
O((1<(b - a))&&(trbudget_check(T,b - a))){O((a - F)<= (L - b)){O((L - b)<= (b - a)){O(1<(a - F)){S5(X+dn,a,b,bF,TR);S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){S5(X+dn,a,b,bF,TR);F=b;}Y{X+=dn,F=a,L=b,Z=bF;}}Y O((a - F)<= (b - a)){O(1<(a - F)){S5(X,b,L,Z,TR);S5(X+dn,a,b,bF,TR);L=a;}Y{S5(X,b,L,Z,TR);X+=dn,F=a,L=b,Z=bF;}}Y{S5(X,b,L,Z,TR);S5(X,F,a,Z,TR);X+=dn,F=a,L=b,Z=bF;}}Y{O((a - F)<= (b - a)){O(1<(L - b)){S5(X+dn,a,b,bF,TR);S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){S5(X+dn,a,b,bF,TR);L=a;}Y{X+=dn,F=a,L=b,Z=bF;}}Y O((L - b)<= (b - a)){O(1<(L - b)){S5(X,F,a,Z,TR);S5(X+dn,a,b,bF,TR);F=b;}Y{S5(X,F,a,Z,TR);X+=dn,F=a,L=b,Z=bF;}}Y{S5(X,F,a,Z,TR);S5(X,b,L,Z,TR);X+=dn,F=a,L=b,Z=bF;}}}Y{O((1<(b - a))&&(0<=TR)){bG[TR].d=-1;}
O((a - F)<= (L - b)){O(1<(a - F)){S5(X,b,L,Z,TR);L=a;}Y O(1<(L - b)){F=b;}Y{cu(X,F,L,Z,TR);}}Y{O(1<(L - b)){S5(X,F,a,Z,TR);F=b;}Y O(1<(a - F)){L=a;}Y{cu(X,F,L,Z,TR);}}}}Y{O(trbudget_check(T,L - F)){Z=dj(L - F),X+=dn;}Y{O(0<=TR){bG[TR].d=-1;}
cu(X,F,L,Z,TR);}}}
#undef bU
}bL
K
trsort(I *ISA,I *SA,I n,I W){I *X;I *F,*L;tr b;I t,de,unsorted;trbudget_init(&b,dj(n)* 2 / 3,n);Q(X=ISA+W;-n<*SA;X+=X - ISA){F=SA;de=0;unsorted=0;do{O((t=*F)< 0){F-=t;de+=t;}
Y{O(de != 0){*(F+de)= de;de=0;}
L=SA+ISA[t]+1;O(1<(L - F)){b.n=0;tr_Irosort(ISA,X,SA,F,L,&b);O(b.n != 0){unsorted+=b.n;}
Y{de=F - L;}}Y O((L - F)== 1){de=-1;}F=L;}}ba(F<(SA+n));O(de != 0){*(F+de)= de;}
O(unsorted==0){g;}}}bL
I
sort_typeBstar(G J V *T,I *SA,I *cL,I *cA,I n){I *PAb,*ISAb,*buf;I i,j,k,t,m,ct;I c0,c1;Q(i=0;i<BUCKET_A_SIZE;++i){cL[i]=0;}
Q(i=0;i<BUCKET_B_SIZE;++i){cA[i]=0;}
Q(i=n - 1,m=n,c0=T[n - 1];0<=i;){do{++cZ(c1=c0);}ba((0<=--i)&&((c0=T[i])>= c1));O(0<=i){++BB(c0,c1);SA[--m]=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){++cz(c0,c1);}}}m=n - m;Q(c0=0,i=0,j=0;c0<AS;++c0){t=i+cZ(c0);cZ(c0)= i+j;i=t+cz(c0,c0);Q(c1=c0+1;c1<AS;++c1){j+=BB(c0,c1);BB(c0,c1)= j;i+=cz(c0,c1);}}O(0<m){PAb=SA+n - m;ISAb=SA+m;Q(i=m - 2;0<=i;--i){t=PAb[i],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=i;}t=PAb[m - 1],c0=T[t],c1=T[t+1];SA[--BB(c0,c1)]=m - 1;buf=SA+m,ct=n - (2 * m);Q(c0=AS - 2,j=m;0<j;--c0){Q(c1=AS - 1;c0<c1;j=i,--c1){i=BB(c0,c1);O(1<(j - i)){sssort(T,PAb,SA+i,SA+j,buf,ct,2,n,*(SA+i)== (m - 1));}}}Q(i=m - 1;0<=i;--i){O(0<=SA[i]){j=i;do{ISAb[SA[i]]=i;}ba((0<=--i)&&(0<=SA[i]));SA[i+1]=i - j;O(i<=0){g;}}j=i;do{ISAb[SA[i]=~SA[i]]=j;}ba(SA[--i]<0);ISAb[SA[i]]=j;}trsort(ISAb,SA,m,1);Q(i=n - 1,j=m,c0=T[n - 1];0<=i;){Q(--i,c1=c0;(0<=i)&&((c0=T[i])>= c1);--i,c1=c0){}O(0<=i){t=i;Q(--i,c1=c0;(0<=i)&&((c0=T[i])<= c1);--i,c1=c0){}SA[ISAb[--j]]=((t==0)|| (1<(t - i)))? t:~t;}}cz(AS - 1,AS - 1)= n;Q(c0=AS - 2,k=m - 1;0<=c0;--c0){i=cZ(c0+1)- 1;Q(c1=AS - 1;c0<c1;--c1){t=i - cz(c0,c1);cz(c0,c1)= i;Q(i=t,j=BB(c0,c1);j<=k;--i,--k){SA[i]=SA[k];}}BB(c0,c0+1)= i - cz(c0,c0)+ 1;cz(c0,c0)= i;}}B m;}bL
K
construct_SA(G J V *T,I *SA,I *cL,I *cA,I n,I m){I *i,*j,*k;I s;I c0,c1,c2;O(0<m){Q(c1=AS - 2;0<=c1;--c1){Q(i=SA+BB(c1,c1+1),j=SA+cZ(c1+1)- 1,k=NULL,c2=-1;i<=j;--j){O(0<(s=*j)){*j=~s;c0=T[--s];O((0<s)&&(T[s - 1]>c0)){s=~s;}
O(c0 != c2){O(0<=c2){cz(c2,c1)= k - SA;}
k=SA+cz(c2=c0,c1);}*k--=s;}Y{*j=~s;}}}}k=SA+cZ(c2=T[n - 1]);*k++=(T[n - 2]<c2)? ~(n - 1):(n - 1);Q(i=SA,j=SA+n;i<j;++i){O(0<(s=*i)){c0=T[--s];O((s==0)|| (T[s - 1]<c0)){s=~s;}
O(c0 != c2){cZ(c2)= k - SA;k=SA+cZ(c2=c0);}*k++=s;}Y{*i=~s;}}}I
divsufsort(G J V *T,I *SA,I n){I *cL,*cA;I m;I err=0;O((T==NULL)|| (SA==NULL)|| (n<0)){B -1;}
Y O(n==0){B 0;}
Y O(n==1){SA[0]=0;B 0;}
Y O(n==2){m=(T[0]<T[1]);SA[m ^ 1]=0,SA[m]=1;B 0;}
cL=(I *)malloc(BUCKET_A_SIZE * sizeof(I));cA=(I *)malloc(BUCKET_B_SIZE * sizeof(I));O((cL != NULL)&&(cA != NULL)){m=sort_typeBstar(T,SA,cL,cA,n);construct_SA(T,SA,cL,cA,n,m);}Y{err=-2;}free(cA);free(cL);B err;}std::bP cf(int64_t x,I n=1){std::bP r;Q(;x||n>0;x/=10,--n)r=std::bP(1,'0'+x%10)+r;B r;}bB da:bE LQ::cg{LQ::cC<J> ht;G J V* in;G I checkbits;G I bZ;G J htsize;G J n;J i;G J m;G J m2;G J maxMatch;G J maxLiteral;G J cW;J h1,h2;G J bucket;G J shift1,shift2;G I mBoth;G J rb;J bits;J nbits;J bT,bC;J idx;G J* sa;J* isa;enum{cv=1<<14};J V buf[cv];K dk(J i,J&lit);K write_match(J len,J off);K fill();K putb(J x,I k){x&=(1<<k)-1;bits|=x<<nbits;nbits+=k;ba(nbits>7){buf[bC++]=bits,bits>>=8,nbits-=8;}}K flush(){O(nbits>0)buf[bC++]=bits;bits=nbits=0;}K put(I c){buf[bC++]=c;}bE:da(SB&inbuf,I bm[],G J* sap=0);I get(){I c=-1;O(bT==bC)fill();O(bT<bC)c=buf[bT++];O(bT==bC)bT=bC=0;B c;}I read(V* p,I n);};I lg(J x){J r=0;O(x>=65536)r=16,x>>=16;O(x>=256)r+=8,x>>=8;O(x>=16)r+=4,x>>=4;B
"\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;}I nbits(J x){I r;Q(r=0;x;x>>=1)r+=x&1;B r;}I da::read(V* p,I n){O(bT==bC)fill();I nr=n;O(nr>I(bC-bT))nr=bC-bT;O(nr)memcpy(p,buf+bT,nr);bT+=nr;O(bT==bC)bT=bC=0;B nr;}da::da(SB&inbuf,I bm[],G J* sap):
ht((bm[1]&3)==3 ? (inbuf.be()+1)*!sap
:bm[5]-bm[0]<21 ? 1u<<bm[5]
:(inbuf.be()*!sap)+(1u<<17<<bm[0])),in(inbuf.bX()),checkbits(bm[5]-bm[0]<21 ? 12-bm[0]:17+bm[0]),bZ(bm[1]&3),htsize(ht.be()),n(inbuf.be()),i(0),m(bm[2]),m2(bm[3]),maxMatch(cv*3),maxLiteral(cv/4),cW(bm[6]),h1(0),h2(0),bucket((1<<bm[4])-1),shift1(m>0 ? (bm[5]-1)/m+1:1),shift2(m2>0 ? (bm[5]-1)/m2+1:0),mBoth(MAX(m,m2+cW)+4),rb(bm[0]>4 ? bm[0]-4:0),bits(0),nbits(0),bT(0),bC(0),idx(0),sa(0),isa(0){O((m<4&&bZ==1)|| (m<1&&bZ==2))
E(0);O(bm[5]-bm[0]>=21||bZ==3){O(sap)
sa=sap;Y{sa=&ht[0];O(n>0)divsufsort((G J V*)in,(I*)sa,n);}O(bZ<3){isa=&ht[n*(sap==0)];}}}K da::fill(){O(bZ==3){Q(;bC<cv&&i<n+5;++i){O(i==0)put(n>0 ? in[n-1]:255);Y O(i>n)put(idx&255),idx>>=8;Y O(sa[i-1]==0)idx=i,put(255);Y put(in[sa[i-1]-1]);}B;}J lit=0;G J mask=(1<<checkbits)-1;ba(i<n&&bC*2<cv){J dl=m-1;J bp=0;J blit=0;I bscore=0;O(isa){O(sa[isa[i&mask]]!=i)
Q(J j=0;j<n;++j)
O((sa[j]&~mask)==(i&~mask))
isa[sa[j]&mask]=j;Q(J h=0;h<=cW;++h){J q=isa[(h+i)&mask];O(sa[q]!=h+i)dw;Q(I j=-1;j<=1;j+=2){Q(J k=1;k<=bucket;++k){J p;O(q+j*k<n&&(p=sa[q+j*k]-h)<i){J l,l1;Q(l=h;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);Q(l1=h;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-4*(lit==0&&l1>0)-11;Q(J a=0;a<h;++a)score=score*5/8;O(score>bscore)dl=l,bp=p,blit=l1,bscore=score;O(l<dl||l<m||l>255)g;}}}O(bscore<=0||dl<m)g;}}Y O(bZ==1||m<=64){O(m2>0){Q(J k=0;k<=bucket;++k){J p=ht[h2^k];O(p&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+dl<=n&&in[p+dl-1]==in[i+dl-1]){J l;Q(l=cW;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);O(l>=m2+cW){I l1;Q(l1=cW;l1>0&&in[p+l1-1]==in[i+l1-1];--l1);I score=I(l-l1)*8-lg(i-p)-8*(lit==0&&l1>0)-11;O(score>bscore)dl=l,bp=p,blit=l1,bscore=score;}}}O(dl>=128)g;}}O(!m2||dl<m2){Q(J k=0;k<=bucket;++k){J p=ht[h1^k];O(p&&i+3<n&&(p&mask)==(in[i+3]&mask)){p>>=checkbits;O(p<i&&i+dl<=n&&in[p+dl-1]==in[i+dl-1]){J l;Q(l=0;i+l<n&&l<maxMatch&&in[p+l]==in[i+l];++l);I score=l*8-lg(i-p)-2*(lit>0)-11;O(score>bscore)dl=l,bp=p,blit=0,bscore=score;}}O(dl>=128)g;}}}G J off=i-bp;O(off>0&&bscore>0
&&dl-blit>=m+(bZ==2)*((off>=(1<<16))+(off>=(1<<24)))){lit+=blit;dk(i+blit,lit);write_match(dl-blit,off);}Y{dl=1;++lit;}O(isa)
i+=dl;Y{ba(dl--){O(i+mBoth<n){J ih=((i*1234547)>>19)&bucket;G J p=(i<<checkbits)|(in[i+3]&mask);O(m2){ht[h2^ih]=p;h2=(((h2*9)<<shift2)
+(in[i+m2+cW]+1)*23456789u)&(htsize-1);}ht[h1^ih]=p;h1=(((h1*5)<<shift1)+(in[i+m]+1)*123456791u)&(htsize-1);}++i;}}O(lit>=maxLiteral)
dk(i,lit);}O(i==n){dk(n,lit);flush();}}K da::dk(J i,J&lit){O(bZ==1){O(lit<1)B;I ll=lg(lit);putb(0,2);--ll;ba(--ll>=0){putb(1,1);putb((lit>>ll)&1,1);}putb(0,1);ba(lit)putb(in[i-lit--],8);}Y{ba(lit>0){J lit1=lit;O(lit1>64)lit1=64;put(lit1-1);Q(J j=i-lit;j<i-lit+lit1;++j)put(in[j]);lit-=lit1;}}}K da::write_match(J len,J off){O(bZ==1){I ll=lg(len)-1;off+=(1<<rb)-1;I lo=lg(off)-1-rb;putb((lo+8)>>3,2);putb(lo&7,3);ba(--ll>=2){putb(1,1);putb((len>>ll)&1,1);}putb(0,1);putb(len&3,2);putb(off,rb);putb(off>>rb,lo);}Y{--off;ba(len>0){G J len1=len>m*2+63 ? m+63:len>m+63 ? len-m:len;O(off<(1<<16)){put(64+len1-m);put(off>>8);put(off);}Y O(off<(1<<24)){put(128+len1-m);put(off>>16);put(off>>8);put(off);}Y{put(192+len1-m);put(off>>24);put(off>>16);put(off>>8);put(off);}len-=len1;}}}std::bP makeConfig(G V* ME,I bm[]){G V bj=ME[0];bm[0]=0;bm[1]=0;bm[2]=0;bm[3]=0;bm[4]=0;bm[5]=0;bm[6]=0;bm[7]=0;bm[8]=0;O(isdigit(*++ME))bm[0]=0;Q(I i=0;i<9&&(isdigit(*ME)|| *ME==','||*ME=='.');){O(isdigit(*ME))
bm[i]=bm[i]*10+*ME-'0';Y O(++i<9)
bm[i]=0;++ME;}O(bj=='0')
B "comp 0 0 0 0 0 bR end\n";std::bP hdr,cV;G I bZ=bm[1]&3;G bV doe8=bm[1]>=4&&bm[1]<=7;O(bZ==1){E(0);}Y O(bZ==2){hdr="comp 9 16 0 $1+20 ";cV=
"cV lzpre c ;\n"
" a> 255 O\n";O(doe8){E(0);}cV+="b=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n";O(!doe8)cV+=" out ";cV+="a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++";O(!doe8)cV+=" out d-- a=d a> 0 ba endif endif endif halt end ";}Y O(bZ==3){E(0);}Y O(bZ==0){E(0);}Y
E(0);I ncomp=0;G I membits=bm[0]+20;I sb=5;std::bP comp;std::bP bR="bR\n"
"c-- *c=a a+= 255 d=a *d=c\n";O(bZ==2){bR+=
" a=r 1 a== 0 O\n"
" a= "+cf(111+57*doe8)+"\n"
" Y a== 1 O\n"
" a=*c r=a 2\n"
" a> 63 O a>>= 6 a++ a++\n"
" Y a++ a++ endif\n"
" Y\n"
" a--\n"
" endif endif\n"
" r=a 1\n";}ba(*ME&&ncomp<254){std::vector<I> v;v.push_back(*ME++);O(isdigit(*ME)){v.push_back(*ME++-'0');ba(isdigit(*ME)|| *ME==','||*ME=='.'){O(isdigit(*ME))
v.back()=v.back()*10+*ME++-'0';Y{v.push_back(0);++ME;}}}O(v[0]=='c'){ba(v.be()<3)v.push_back(0);comp+=cf(ncomp)+" ";sb=11;O(v[2]<256)sb+=lg(v[2]);Y sb+=6;Q(J i=3;i<v.be();++i)
O(v[i]<512)sb+=nbits(v[i])*3/4;O(sb>membits)sb=membits;O(v[1]%1000==0)comp+="icm "+cf(sb-6-v[1]/1000)+"\n";Y comp+="cm "+cf(sb-2-v[1]/1000)+" "+cf(v[1]%1000-1)+"\n";bR+="d= "+cf(ncomp)+" *d=0\n";O(v[2]>1&&v[2]<=255){O(lg(v[2])!=lg(v[2]-1))
bR+="a=c a&= "+cf(v[2]-1)+" hashd\n";Y
bR+="a=c a%= "+cf(v[2])+" hashd\n";}Y O(v[2]>=1000&&v[2]<=1255)
bR+="a= 255 a+= "+cf(v[2]-1000)+
" d=a a=*d a-=c a> 255 O a= 255 endif d= "+
cf(ncomp)+" hashd\n";Q(J i=3;i<v.be();++i){O(i==3)bR+="b=c ";O(v[i]==255)
bR+="a=*b hashd\n";Y O(v[i]>0&&v[i]<255)
bR+="a=*b a&= "+cf(v[i])+" hashd\n";Y O(v[i]>=256&&v[i]<512){bR+=
"a=r 1 a> 1 O\n"
" a=r 2 a< 64 O\n"
" a=*b ";O(v[i]<511)bR+="a&= "+cf(v[i]-256);bR+=" hashd\n"
" Y\n"
" a>>= 6 hashd a=r 1 hashd\n"
" endif\n"
"Y\n"
" a= 255 hashd a=r 2 hashd\n"
"endif\n";}Y O(v[i]>=1256)
bR+="a= "+cf(((v[i]-1000)>>8)&255)+" a<<= 8 a+= "
+cf((v[i]-1000)&255)+
" a+=b b=a\n";Y O(v[i]>1000)
bR+="a= "+cf(v[i]-1000)+" a+=b b=a\n";O(v[i]<512&&i<v.be()-1)
bR+="b++ ";}++ncomp;}O(strchr("mts",v[0])&&ncomp>I(v[0]=='t')){E(0);}O(v[0]=='i'&&ncomp>0){bR+="d= "+cf(ncomp-1)+" b=c a=*d d++\n";Q(J i=1;i<v.be()&&ncomp<254;++i){Q(I j=0;j<v[i]%10;++j){bR+="hash ";O(i<v.be()-1||j<v[i]%10-1)bR+="b++ ";sb+=6;}bR+="*d=a";O(i<v.be()-1)bR+=" d++";bR+="\n";O(sb>membits)sb=membits;comp+=cf(ncomp)+" isse "+cf(sb-6-v[i]/10)+" "+cf(ncomp-1)+"\n";++ncomp;}}O(v[0]=='a'){E(0);}O(v[0]=='w'){E(0);}}B hdr+cf(ncomp)+"\n"+comp+bR+"halt\n"+cV;}K compressBlock(SB* in,bz* out,G V* method_,G V* cK,G V* comment,bV dosha1){std::bP ME=method_;G J n=in->be();G I arg0=MAX(lg(n+4095)-20,0);J bj=0;O(isdigit(ME[0])){I commas=0,arg[4]={0};Q(I i=1;i<I(ME.be())&&commas<4;++i){O(ME[i]==','||ME[i]=='.')++commas;Y O(isdigit(ME[i]))arg[commas]=arg[commas]*10+ME[i]-'0';}O(commas==0)bj=512;Y bj=arg[1]*4+arg[2];}O(isdigit(ME[0])){G I bZ=ME[0]-'0';G I doe8=(bj&2)*2;ME="x"+cf(arg0);std::bP htsz=","+cf(19+arg0+(arg0<=6));std::bP sasz=","+cf(21+arg0);O(bZ==0)
ME="0"+cf(arg0)+",0";Y O(bZ==1){E(0);}Y O(bZ==2){E(0);}Y O(bZ==3){O(bj<20)
ME+=",0";Y O(bj<48)
ME+=","+cf(1+doe8)+",4,0,3"+htsz;Y O(bj>=640||(bj&1))
ME+=","+cf(3+doe8)+"ci1";Y
ME+=","+cf(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";}Y O(bZ==4){E(0);}Y{E(0);}}std::bP config;I bm[9]={0};config=makeConfig(ME.c_str(),bm);LQ::C co;co.setOutput(out);SB pcomp_cmd;co.startBlock(config.c_str(),bm,&pcomp_cmd);std::bP cs=cf(n);co.startSegment(cK,cs.c_str());O(bm[1]>=1&&bm[1]<=7&&bm[1]!=4){da lz(*in,bm);co.setInput(&lz);co.compress();}Y{E(0);}co.endSegment(0);}}namespace zpaq{td::cN compress(td::Slice bX){I be=td::narrow_cast<int>(bX.be());LQ::SB in,out;in.bq(bX.bX(),be);LQ::compressBlock(&in,&out,"3",0,0,dp);B td::cN(out.c_str(),out.be());}td::cN decompress(td::Slice bX){LQ::SB in,out;in.bq(bX.bX(),bX.be());LQ::decompress(&in,&out);B td::cN(out.c_str(),out.be());}}
#undef Y
#undef O
#undef Q
td::cN compress(td::Slice bX){NullStream ofs;Ref<Cell> block_root=std_boc_deserialize(bX).move_as_ok();FullBlock block;A load_std_ctx{ofs};block.bc(load_std_ctx,block_root,0,cD);A pack_opt_ctx{ofs};cO opt_block_cell=block.bx(pack_opt_ctx);BagOfCells opt_boc;opt_boc.add_root(opt_block_cell);(opt_boc.import_cells().is_ok());cO opt_ser=std_boc_serialize(opt_block_cell).move_as_ok();cO compressed=zpaq::compress(opt_ser);B compressed;}td::cN decompress(td::Slice bX){NullStream ofs;cO decompressed=zpaq::decompress(bX);cO opt_deser=std_boc_deserialize(decompressed,dp,cD).move_as_ok();FullBlock opt_block;A parse_opt_ctx{ofs};opt_block.bn(parse_opt_ctx,opt_deser,0,cD);A pack_std_ctx{ofs};cO un_opt_block_cell=opt_block.bb(pack_std_ctx);BagOfCells un_opt_boc;un_opt_boc.add_root(un_opt_block_cell);(un_opt_boc.import_cells().is_ok());cO boc=std_boc_serialize(un_opt_block_cell,31).move_as_ok();B boc;}int main(
){bP mode;cin >> mode;bP base64_data;cin >> base64_data;td::cN bX(td::base64_decode(base64_data));bX=(mode=="compress")? compress(bX):decompress(bX);cout << td::str_base64_encode(bX)<< endl;}