
#include<iostream>
#include<sstream>
#include<fstream>
#include<set>
#include<stdio.h>
#include<string>
#include<vector>
#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<algorithm>
#include"common/util.h"
#include"vm/cells/CellSlice.h"
#include"td/utils/lz4.h"
#include"td/utils/misc.h"
#include"td/utils/buffer.h"
#include<errno.h>
#include<stddef.h>
#include<string.h>
#include<stdint.h>
#define private public
#define final
#include"block/block-auto.h"
#include"block/block-parse.h"
#include"vm/boc.h"
#include"crypto/vm/boc-writers.h"
#define ia sizeof
#define hW pair
#define hS ShardAccountBlocks
#define hJ HashmapAugNode
#define ht in_msg_descr
#define hr as_cellslice
#define hq McBlockExtra
#define hp store_ptr
#define hb ShardFeeCreated
#define ha class
#define gZ template
#define gU gen
#define gT ref_idx
#define gB out_msg_descr
#define gq char
#define gi string
#define fZ virtual
#define fW CurrencyCollection
#define fI uint64_t
#define fD fetch_ulong
#define fa continue
#define eX state_update
#define eW special_type
#define ee HashmapAugE
#define dZ write
#define dS store_long
#define dN typedef
#define dJ position
#define dD append_cellslice
#define dx BufferSlice
#define dm ptrdiff_t
#define cV size
#define cr for
#define cq fetch_ref
#define bT auto
#define bO extra
#define bL break
#define bK while
#define bp struct
#define bm store_ref
#define V data
#define U else
#define O CellSlice
#define N CellBuilder
#define L int
#define J static
#define D void
#define z const
#define y size_t
#define u return
#define f unsigned
using namespace vm;using namespace std;set<gi> enabled_optimizations{"Block","BlockExtra","ee","HashmapAug","hJ","InMsgDescr","OutMsgDescr","hq","ShardFees","hS","MERKLE_UPDATE","Maybe",};O to_cs(Ref<Cell> cell){bool can_be_special=false;u load_cell_slice_special(std::move(cell),can_be_special);}bp q{ostream&out;};gZ<ha T_TLB>bp C{O hY;O dt;O ccs;gi name;T_TLB cv;L eW=0;C(gi name):C(name,T_TLB()){}C(gi name,T_TLB cv):name(name),cv(cv){}fZ ~C(){}fZ D bN(q&S,O&cs,L o=0){}fZ D cF(q&S,N&cb,L o=0){cG(S,cb,o);}fZ D bX(q&S,O&cs,L o=0){bN(S,cs,o);}fZ D cG(q&S,N&cb,L o=0){}bool is_enabled(){u enabled_optimizations.count(name)> 0;}bool gz(){u eW==1;}D cJ(q&S,O&cs,L o=0,bool gA=false){bT e=is_enabled();eW=(L)cs.eW();hY=cs;if(gz()){cs.advance(288);u;}if(e){bN(S,cs,o);}U{dt=cv.fetch(cs).dZ();}}D eu(q&S,N&cb,L o=0){bT e=is_enabled();if(gz()){cb.dD(hY);}U if(e){cF(S,cb,o);}U{cb.dD(dt);}}D cS(q&S,O&cs,L o=0,bool gA=false){bT e=is_enabled();hY=cs;if(e){bX(S,cs,o);}U{dt=cv.fetch(cs).dZ();}}D dE(q&S,N&cb,L o=0){bT e=is_enabled();if(gz()){cb.dD(hY);}U if(e){cG(S,cb,o);}U{if(!dt.is_valid()){throw runtime_error(name+":optimization is disabled,but dt is empty,meaning it was never set");}cb.dD(dt);}}Ref<Cell> bt(q&S,L o=0){N cb;dE(S,cb,o);u cb.finalize(eW!=0);}Ref<Cell> cP(q&S,L o=0){N cb;eu(S,cb,o);u cb.finalize(eW!=0);}D bw(q&S,Ref<Cell> cell_ref,L o=0,bool gA=false){bT cs=to_cs(std::move(cell_ref));cJ(S,cs,o,gA);}D ck(q&S,Ref<Cell> cell_ref,L o=0,bool gA=false){bT cs=to_cs(std::move(cell_ref));cS(S,cs,o,gA);}D fetch_remaining(O&cs){ccs=cs;cs.advance(cs.cV());cs.advance_refs(cs.size_refs());}D append_remaining(N&cb){cb.dD(ccs);}};bp ev{fZ ~ev(){}fZ O fp(){throw runtime_error("aug V requested but not implemented");}};gZ<ha T_TLB>bp fq{T_TLB add_type;fq(T_TLB add_type):add_type(add_type){}fq():fq(T_TLB{}){}fZ ~fq(){}fZ O add_values(O&cs1,O&cs2){N cb;(add_type.add_values(cb,cs1,cs2));u cb.hr();}};using namespace block::tlb;z fW tCC;z OutMsg tOM;z AccountBlock tAB;z ImportFees tIF;z InMsg tIM;z EnqueuedMsg tEM;z UInt tU64{64};z hb tSFC;gZ<ha gj,ha fC>bp cR;gZ<ha gj,ha fC>bp hT:C<block::gU::hJ>{L tag=-1;L n=-1;Ref<cR<gj,fC>> left;Ref<cR<gj,fC>> right;gj ew;fC bO;hT(L m,z TLB&X,z TLB&Y):C("hJ",block::gU::hJ(m,X,Y)){}D bN(q&S,O&cs,L o=0){tag=cv.check_tag(cs);if(tag==0){bO.cJ(S,cs,o+1);ew.cJ(S,cs,o+1);}U{L n;add_r1(n,1,cv.m_);left=Ref<cR<gj,fC>>(true,n,cv.X_,cv.Y_);left.dZ().bw(S,cs.cq(),o+1);right=Ref<cR<gj,fC>>(true,n,cv.X_,cv.Y_);right.dZ().bw(S,cs.cq(),o+1);bO.cJ(S,cs,o+1);}}D cF(q&S,N&cb,L o=0){if(tag==0){ew.eu(S,cb,o+1);}U{L n;(add_r1(n,1,cv.m_));cb.bm(left.dZ().cP(S,o+1));cb.bm(right.dZ().cP(S,o+1));}}D bX(q&S,O&cs,L o=0){tag=cv.check_tag(cs);if(tag==0){ew.cS(S,cs,o+1);bT extra_cs=ew.fp();bO.cS(S,extra_cs,o+1,true);}U{L n;add_r1(n,1,cv.m_);left=Ref<cR<gj,fC>>(true,n,cv.X_,cv.Y_);left.dZ().ck(S,cs.cq(),o+1);right=Ref<cR<gj,fC>>(true,n,cv.X_,cv.Y_);right.dZ().ck(S,cs.cq(),o+1);bT left_extra_cs=to_cs(left.dZ().node.bO.bt(S));bT right_extra_cs=to_cs(right.dZ().node.bO.bt(S));bT extra_cs=bO.add_values(left_extra_cs,right_extra_cs);bO.cS(S,extra_cs,o+1,true);}}D cG(q&S,N&cb,L o=0){if(tag==0){bO.dE(S,cb,o+1);ew.dE(S,cb,o+1);}U{L n;(add_r1(n,1,cv.m_));cb.bm(left.dZ().bt(S,o+1));cb.bm(right.dZ().bt(S,o+1));bO.dE(S,cb,o+1);}}};gZ<ha gj,ha fC>bp cR:C<block::gU::HashmapAug>,td::CntObject{Ref<O> label;L n,m,l;hT<gj,fC> node;cR(L n,z TLB&X,z TLB&Y):C("HashmapAug",block::gU::HashmapAug(n,X,Y)),node(n,X,Y){}D bN(q&S,O&cs,L o=0){n=cv.m_;(block::gU::HmLabel{n}.fetch_to(cs,label,l));m=n-l;node.cv.m_=m;node.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){L l,m;(tlb::store_from(cb,HmLabel{cv.m_},label,l));(add_r1(m,l,cv.m_));node.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){(
(n=cv.m_)>= 0
&&block::gU::HmLabel{cv.m_}.fetch_to(cs,label,l)&&add_r1(m,l,cv.m_));node.cv.m_=m;node.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){L l,m;(tlb::store_from(cb,block::gU::HmLabel{cv.m_},label,l)&&add_r1(m,l,cv.m_));node.dE(S,cb,o+1);}};gZ<ha gj,ha fC>bp ex:C<block::gU::ee>{block::gU::ee::Record_ahme_root r;L tag=-1;cR<gj,fC> root;fC bO;ex(L n,z TLB&X,z TLB&Y):C("ee",block::gU::ee(n,X,Y)),root(n,X,Y){}D bN(q&S,O&cs,L o=0){tag=cv.check_tag(cs);if(tag==block::gU::ee::ahme_empty){(cs.fD(1)== 0);bO.cJ(S,cs,o+1);}U{(cv.unpack(cs,r));root.bw(S,r.root,o+1);bO.cJ(S,r.bO.dZ(),o+1);}}D cF(q&S,N&cb,L o=0){if(tag==ee::ahme_empty){cb.dS(0,1);bO.eu(S,cb,o+1);}U{cb.dS(1,1).bm(root.cP(S,o+1));}}D bX(q&S,O&cs,L o=0){tag=cv.check_tag(cs);if(tag==ee::ahme_empty){(cs.fD(1)== 0);bO.cS(S,cs,o+1);}U{(cs.fD(1)== 1&&(r.n=cv.m_)>= 0);bT root_ref=cs.cq();root.ck(S,root_ref,o+1);bO=root.node.bO;}}D cG(q&S,N&cb,L o=0){if(tag==ee::ahme_empty){cb.dS(0,1);bO.dE(S,cb,o+1);}U{cb.dS(1,1).bm(root.bt(S,o+1));bO.dE(S,cb,o+1);}}};gZ<ha T>bp FullMaybe:C<TLB>{T ew;L tag=-1;bool is_ref;FullMaybe(bool is_ref=false):C("Maybe"),is_ref(is_ref){}D bN(q&S,O&cs,L o=0){tag=cs.fD(1);if(tag){if(is_ref){ew.bw(S,cs.cq(),o+1);}U{ew.cJ(S,cs,o+1);}}}fZ D cF(q&S,N&cb,L o=0){(cb.store_long_bool(tag,1));if(tag){if(is_ref){cb.bm(ew.cP(S,o+1));}U{ew.eu(S,cb,o+1);}}}fZ D bX(q&S,O&cs,L o=0){tag=cs.fD(1);if(tag){if(is_ref){ew.ck(S,cs.cq(),o+1);}U{ew.cS(S,cs,o+1);}}}D cG(q&S,N&cb,L o=0){(cb.store_long_bool(tag,1));if(tag){if(is_ref){cb.bm(ew.bt(S,o+1));}U{ew.dE(S,cb,o+1);}}}};bp gk;bp FullInMsg:C<InMsg>,ev{FullInMsg():C("InMsg",InMsg()){}O fp(){N cb;bT cs_copy=dt;(cv.get_import_fees(cb,cs_copy));u cb.hr();}};bp ey:C<fW>,fq<fW>{ey():C("fW"){}};bp FullOutMsg:C<OutMsg>,ev{FullOutMsg():C("OutMsg",OutMsg()){}O fp(){N cb;bT cs_copy=dt;(cv.get_export_value(cb,cs_copy));u cb.hr();}};gZ<ha T>bp FullMERKLE_UPDATE:C<TLB>{Ref<T> go,to_proof;FullMERKLE_UPDATE(z T&cv):C("MERKLE_UPDATE"){}D bN(q&S,O&cs,L o=0){(cs.advance(520));go=Ref<T>(true);go.dZ().bw(S,cs.cq(),o+1);to_proof=Ref<T>(true);to_proof.dZ().bw(S,cs.cq(),o+1);}D cF(q&S,N&cb,L o=0){cb.bm(go.dZ().bt(S,o+1));cb.bm(to_proof.dZ().bt(S,o+1));eW=0;}D bX(q&S,O&cs,L o=0){go=Ref<T>(true);go.dZ().bw(S,cs.cq(),o+1);to_proof=Ref<T>(true);to_proof.dZ().bw(S,cs.cq(),o+1);}Ref<Cell> bt(q&S,L o=0){u N::create_merkle_update(
go.dZ().bt(S,o+1),to_proof.dZ().bt(S,o+1));}};bp FullAccountBlock:C<AccountBlock>,ev{FullAccountBlock():C("AccountBlock"){}O fp(){N cb;bT cs_copy=dt;(Aug_ShardAccountBlocks().eval_leaf(cb,cs_copy));u cb.hr();}};bp gp:C<block::gU::hS>{ex<FullAccountBlock,ey> x{256,tAB,tCC};gp():C("hS"){}D bN(q&S,O&cs,L o=0){x.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){x.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){x.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){x.dE(S,cb,o+1);}};bp hK:block::gU::McStateExtra{bool skip(vm::O&cs)z {u cs.advance(16)&&block::gU::ShardHashes().skip(cs)&&cs.advance_ext(0x100,2)&&tCC.skip(cs);}};bp FullMcStateExtra:C<hK>{FullMcStateExtra():C("McStateExtra",hK()){}};z block::gU::ShardStateUnsplit_aux tSSUa;z block::gU::RefT tRMSE{hK()};z block::gU::Maybe tMRMSE{tRMSE};bp MyShardStateUnsplit:block::gU::ShardStateUnsplit{bool skip(vm::O&cs)z{u cs.advance_ext(0x169,3)&&tMRMSE.skip(cs);}};bp hL:C<ShardState>,td::CntObject{hL():C("ShardState"){}};bp fr:C<hb>,ev,fq<hb>{fr():C("hb"){}O fp(){N cb;bT cs_copy=dt;(Aug_ShardFees().eval_leaf(cb,cs_copy));u cb.hr();}};bp FullShardFees:C<block::gU::ShardFees>{ex<fr,fr> x{96,tSFC,tSFC};FullShardFees():C("ShardFees"){}D bN(q&S,O&cs,L o=0){x.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){x.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){x.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){x.dE(S,cb,o+1);}};z block::gU::ShardHashes tSH;bp FullMcBlockExtra:C<block::gU::hq>{block::gU::hq::Record dj;FullShardFees shard_fees;FullMcBlockExtra():C("hq"){}D bN(q&S,O&cs,L o=0){(cs.fD(16)== 0xcca5);(cs.fetch_bool_to(dj.key_block));(tSH.fetch_to(cs,dj.shard_hashes));shard_fees.cJ(S,cs,o+1);fetch_remaining(cs);}D cF(q&S,N&cb,L o=0){cb.dS(dj.key_block,1);tSH.store_from(cb,dj.shard_hashes);shard_fees.eu(S,cb,o+1);append_remaining(cb);}D bX(q&S,O&cs,L o=0){(cs.fetch_bool_to(dj.key_block));(tSH.fetch_to(cs,dj.shard_hashes));shard_fees.cS(S,cs,o+1);fetch_remaining(cs);}D cG(q&S,N&cb,L o=0){cb.dS(0xcca5,16).dS(dj.key_block,1);tSH.store_from(cb,dj.shard_hashes);shard_fees.dE(S,cb,o+1);append_remaining(cb);}};bp gk;bp FullOutMsgDescr:C<OutMsgDescr>{ex<FullOutMsg,ey> x{256,tOM,tCC};FullOutMsgDescr():C("OutMsgDescr"){}D bN(q&S,O&cs,L o=0){x.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){x.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){x.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){x.dE(S,cb,o+1);}};bp gk:C<ImportFees>,fq<ImportFees>{gk():C("ImportFees",tIF),fq(tIF){}};bp FullInMsgDescr:C<InMsgDescr>{ex<FullInMsg,gk> x{256,tIM,tIF};FullInMsgDescr():C("InMsgDescr",InMsgDescr()){}D bN(q&S,O&cs,L o=0){x.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){x.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){x.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){x.dE(S,cb,o+1);}};z block::gU::hq tMBE{};z block::gU::RefT tRMBE{tMBE};z block::gU::Maybe tMRMBE(tRMBE);bp FullBlockExtra:C<block::gU::BlockExtra>{block::gU::BlockExtra::Record dj;FullInMsgDescr ht;FullOutMsgDescr gB;gp gl;FullMaybe<FullMcBlockExtra> custom;FullBlockExtra():C("BlockExtra"),custom(true){}D bN(q&S,O&cs,L o=0){((cs.fD(32)== 0x4a33f6fd));ht.bw(S,cs.cq(),o+1);gB.bw(S,cs.cq(),o+1);gl.bw(S,cs.cq(),o+1);ccs=cs.fetch_subslice(512).dZ();custom.cJ(S,cs,o+1);}D cF(q&S,N&cb,L o=0){cb.bm(ht.cP(S,o+1)).bm(gB.cP(S,o+1)).bm(gl.cP(S,o+1)).dD(ccs);custom.eu(S,cb,o+1);}D bX(q&S,O&cs,L o=0){ht.ck(S,cs.cq(),o+1);gB.ck(S,cs.cq(),o+1);gl.ck(S,cs.cq(),o+1);ccs=cs.fetch_subslice(512).dZ();custom.cS(S,cs,o+1);}D cG(q&S,N&cb,L o=0){cb.dS(0x4a33f6fd,32).bm(ht.bt(S,o+1)).bm(gB.bt(S,o+1)).bm(gl.bt(S,o+1)).dD(ccs);custom.dE(S,cb,o+1);}};bp FullBlock:C<block::gU::Block>{block::gU::Block::Record dj;FullMERKLE_UPDATE<hL> eX;FullBlockExtra bO;FullBlock():C("Block"),eX(hL()){}D bN(q&S,O&cs,L o=0){(cv.unpack(cs,dj));eX.bw(S,dj.eX,o+1);bO.bw(S,dj.bO,o+1,true);}D cF(q&S,N&cb,L o=0){cb.dS(dj.global_id,32).bm(dj.info).bm(dj.value_flow).bm(eX.cP(S,o+1)).bm(bO.cP(S,o+1));}D bX(q&S,O&cs,L o=0){(
cs.fetch_int_to(32,dj.global_id)&&cs.fetch_ref_to(dj.info)&&cs.fetch_ref_to(dj.value_flow));eX.ck(S,cs.cq(),o+1);bO.ck(S,cs.cq(),o+1,true);}D cG(q&S,N&cb,L o=0){cb.dS(0x11ef55aa,32).dS(dj.global_id,32).bm(dj.info).bm(dj.value_flow).bm(eX.bt(S,o+1)).bm(bO.bt(S,o+1));}};ha NullStream:public ostream{ha NullBuffer:public streambuf{public:L overflow(L c){u c;}}m_nb;public:NullStream():ostream(&m_nb){}};
#define cy 0
#define dy 1
#define ga 2
#define gY 4
#define hM 5
#define fX 6
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_FAIL 11
#define RINOK(x){z L _result_=(x);if(_result_!=0)u _result_;}
dN f gq Z;dN f short UInt16;dN f L g;bp gC{y (*Write)(gC* p,z D *eR,y cV);};
#define MY_offsetof(cv,m)offsetof(cv,m)
#define Z7_container_of(ptr,cv,m)((cv *)(D *)((gq*)(D *)(1 ? (ptr):&((cv *)0)->m)- MY_offsetof(cv,m)))
#define Z7_CONTAINER_FROM_VTBL(ptr,cv,m)Z7_container_of(ptr,cv,m)
#define gb(ptr,cv,m,p)cv *p=Z7_CONTAINER_FROM_VTBL(ptr,cv,m);
#define UNUSED_VAR(x)(D)x;
D *SzAlloc(y cV){u cV==0 ? 0:malloc(cV);}
#define GetUi16(p)(*(z UInt16 *)(z D *)(p))
#define GetUi32(p)(*(z g *)(z D *)(p))
#define SetUi32(p,v){*(g *)(D *)(p)= (v);}
#define UNUSED_VAR(x)(D)x;
dN g dF;dN bp{z Z *eq;g bG,posLimit,streamPos,cl,cm,dG;Z ef,fE,bigHash,fF;g gD;dF *bf,*son;g hashMask,fG;Z *ch;g cf,ea,fs;g M;y eS;g cH,cW;Z gE,dL,_pad2_[2];L fH;g crc[256];y numRefs;fI dH;}B;
#define ff(p)((g)((p)->streamPos-(p)->bG))
#define gx(p,a,b){(p)->fF=1;(p)->eq=(a);(p)->eS=(b);}
#define hZ(p,a)(p)->bG-=(a);(p)->streamPos-=(a);
dN D (*Mf_Init_Func)(D *object);dN g (*hx)(D *object);dN z Z * (*gV)(D *object);dN g * (*Mf_GetMatches_Func)(D *object,g *K);dN D (*Mf_Skip_Func)(D *object,g);bp IMatchFinder2{Mf_Init_Func Init;hx dl;gV bH;Mf_GetMatches_Func ft;Mf_Skip_Func Skip;};
#define gr (1 << 10)
#define kHash3Size (1 << 16)
#define bA (gr)
#define cX (gr+kHash3Size)
#define hU 5
#define er 10
#define hc (1 << 7)
#define hd (1 << 16)
#define kBlockSizeReserveMin (1 << 24)
#define fJ 0
#define hy ((g)0)
#define dU(p)ff(p)
#define dz cX
#define HASH2_CALC hv=GetUi16(bv);
#define HASH3_CALC{g hV=p->crc[bv[0]] ^ bv[1];h2=hV&(gr-1);hv=(hV ^ ((g)bv[2] << 8))&p->hashMask;}
#define HASH4_CALC{g hV=p->crc[bv[0]] ^ bv[1];h2=hV&(gr-1);hV^=((g)bv[2] << 8);h3=hV&(kHash3Size-1);hv=(hV ^ (p->crc[bv[3]] << hU))&p->hashMask;}
#define HASH5_CALC{g hV=p->crc[bv[0]] ^ bv[1];h2=hV&(gr-1);hV^=((g)bv[2] << 8);h3=hV&(kHash3Size-1);hV^=(p->crc[bv[3]] << hU);hv=(hV ^ (p->crc[bv[4]] << er))&p->hashMask;}
J D LzInWindow_Free(B *p){{free(p->ch);p->ch=0;}}J L LzInWindow_Create2(B *p,g cf){if(cf==0)
u 0;if(!p->ch||p->cf!=cf){LzInWindow_Free(p);p->cf=cf;p->ch=(Z *)SzAlloc(cf);}u(p->ch!=0);}J z Z *gc(D *p){u((B *)p)->eq;}J g gy(D *p){u dU((B *)p);}J D gF(B *p){if(p->ef||p->fH!=cy)
u;g curSize=0xFFFFFFFF-dU(p);if(curSize > p->eS)
curSize=(g)p->eS;p->streamPos+=curSize;p->eS-=curSize;if(p->eS==0)
p->ef=1;}D MatchFinder_MoveBlock(B *p){z y cx=(y)(p->eq-p->ch)- p->ea;z y keepBefore=(cx&(hc-1))+ p->ea;p->eq=p->ch+keepBefore;memmove(p->ch,p->ch+(cx&~((y)hc-1)),keepBefore+(y)dU(p));}L MatchFinder_NeedMove(B *p){if(p->fF)
u 0;if(p->ef||p->fH!=cy)
u 0;u((y)(p->ch+p->cf-p->eq)<= p->fs);}J D gW(B *p){p->fG=32;p->fE=1;p->M=4;p->gE=2;p->dL=0;p->bigHash=0;}
#define kCrcPoly 0xEDB88320
D MatchFinder_Construct(B *p){f i;p->eq=0;p->ch=0;p->fF=0;p->bf=0;p->dH=(fI)(int64_t)-1;gW(p);cr(i=0;i<256;i++){g r=(g)i;f j;cr(j=0;j<8;j++)
r=(r >> 1)^ (kCrcPoly&((g)0-(r&1)));p->crc[i]=r;}}
#undef kCrcPoly
J D eg(B *p){free(p->bf);p->bf=0;}D MatchFinder_Free(B *p){eg(p);LzInWindow_Free(p);}J dF* AllocRefs(y hz){z y sizeInBytes=(y)hz * ia(dF);if(sizeInBytes / ia(dF)!= hz)
u 0;u(dF *)SzAlloc(sizeInBytes);}J g GetBlockSize(B *p,g cH){g cf=(p->ea+p->fs);if(p->ea<cH||cf<p->ea)
u 0;{z g kBlockSizeMax=(g)0-(g)hd;z g gm=kBlockSizeMax-cf;z g reserve=(cf >> (cf<((g)1 << 30)? 1:2))
+ (1 << 12)+ hc+hd;if(cf >= kBlockSizeMax
|| gm<kBlockSizeReserveMin)
u 0;if(reserve >= gm)
cf=kBlockSizeMax;U{cf+=reserve;cf&= ~(g)(hd-1);}}u cf;}J g fS(B *p,g hs){if(p->M==2)
u(1 << 16)- 1;if(hs!=0)
hs--;hs |= (hs >> 1);hs |= (hs >> 2);hs |= (hs >> 4);hs |= (hs >> 8);if(hs >= (1 << 24)){if(p->M==3)
hs=(1 << 24)- 1;}hs |= (1 << 16)- 1;if(p->M >= 5)
hs |= (256 << er)- 1;u hs;}J g gd(B *p,g hs){if(p->M==2)
u(1 << 16)- 1;if(hs!=0)
hs--;hs |= (hs >> 1);hs |= (hs >> 2);hs |= (hs >> 4);hs |= (hs >> 8);hs >>= 1;if(hs >= (1 << 24)){if(p->M==3)
hs=(1 << 24)- 1;U
hs >>= 1;}hs |= (1 << 16)- 1;if(p->M >= 5)
hs |= (256 << er)- 1;u hs;}L MatchFinder_Create(B *p,g cH,g keepAddBufferBefore,g gD,g es){p->ea=cH+keepAddBufferBefore+1;es+=gD;if(es<p->M)
es=p->M;p->fs=es;if(p->fF)
p->cf=0;if(p->fF||LzInWindow_Create2(p,GetBlockSize(p,cH))){y fK;{g hs;g hsCur;if(p->dL!=0){f hA=p->dL;z f nbMax=(p->M==2 ? 16:(p->M==3 ? 24:32));if(hA > nbMax)
hA=nbMax;if(hA >= 32)
hs=(g)0-1;U
hs=((g)1 << hA)- 1;hs |= (1 << 16)- 1;if(p->M >= 5)
hs |= (256 << er)- 1;{z g hs2=fS(p,cH);if(hs > hs2)
hs=hs2;}hsCur=hs;if(p->dH<cH){z g hs2=fS(p,(g)p->dH);if(hsCur > hs2)
hsCur=hs2;}}U{hs=gd(p,cH);hsCur=hs;if(p->dH<cH){hsCur=gd(p,(g)p->dH);if(hsCur > hs)
hsCur=hs;}}p->hashMask=hsCur;fK=hs;fK++;if(fK<hs)
u 0;{g cW=0;if(p->M > 2&&p->gE<=2)cW+=gr;if(p->M > 3&&p->gE<=3)cW+=kHash3Size;fK+=cW;p->cW=cW;}}p->gD=gD;{y newSize;y numSons;z g fu=cH+1;p->cH=cH;p->dG=fu;numSons=fu;if(p->fE)
numSons <<= 1;newSize=fK+numSons;if(numSons<fu||newSize<numSons)
u 0;
#define hB 0xF
newSize=(newSize+hB)&~(y)hB;if(p->bf&&p->numRefs >= newSize)
u 1;eg(p);p->numRefs=newSize;p->bf=AllocRefs(newSize);if(p->bf){p->son=p->bf+fK;u 1;}}}MatchFinder_Free(p);u 0;}J D gG(B *p){g k;g n=hy-p->bG;if(n==0)
n=(g)(L)-1;k=p->dG-p->cm;if(k<n)
n=k;k=dU(p);{z g ksa=p->fs;g mm=p->gD;if(k > ksa)
k-=ksa;U if(k >= mm){k-=mm;k++;}U{mm=k;if(k!=0)
k=1;}p->cl=mm;}if(k<n)
n=k;p->posLimit=p->bG+n;}D MatchFinder_Init_LowHash(B *p){y i;dF *eJ=p->bf;z y eY=p->cW;cr(i=0;i<eY;i++)
eJ[i]=fJ;}D MatchFinder_Init_HighHash(B *p){y i;dF *eJ=p->bf+p->cW;z y eY=(y)p->hashMask+1;cr(i=0;i<eY;i++)
eJ[i]=fJ;}D MatchFinder_Init_4(B *p){if(!p->fF)
p->eq=p->ch;{p->bG=p->streamPos=1;}p->fH=cy;p->ef=0;}
#define CYC_TO_POS_OFFSET 0
D MatchFinder_Init(D *_p){B *p=(B *)_p;MatchFinder_Init_HighHash(p);MatchFinder_Init_LowHash(p);MatchFinder_Init_4(p);gF(p);p->cm=(p->bG-CYC_TO_POS_OFFSET);gG(p);}
#define eZ(i){g v=eJ[i];if(v<fL)v=fL;eJ[i]=v-fL;}
#define DEFAULT_SaturSub LzFind_SaturSub_32
J
D
LzFind_SaturSub_32(g fL,dF *eJ,z dF *lim){do{eZ(0)eZ(1)eJ+=2;eZ(0)eZ(1)eJ+=2;eZ(0)eZ(1)eJ+=2;eZ(0)eZ(1)eJ+=2;}bK(eJ!=lim);}D gs(g fL,dF *eJ,y eY){
#define eF (1 << 7)
cr(;eY!=0&&((f)(dm)eJ&(eF-1))!= 0;eY--){eZ(0)
eJ++;}{z y k_Align_Mask=(eF / 4-1);dF *lim=eJ+(eY&~(y)k_Align_Mask);eY&= k_Align_Mask;if(eJ!=lim){DEFAULT_SaturSub(fL,eJ,lim);}eJ=lim;}cr(;eY!=0;eY--){eZ(0)
eJ++;}}J D ge(B *p){if(
p->fs==dU(p)){if(MatchFinder_NeedMove(p))
MatchFinder_MoveBlock(p);gF(p);}if(p->bG==hy)
if(dU(p)>= p->M){z g fL=(p->bG-p->cH-1);hZ(p,fL)
gs(fL,p->bf,(y)p->hashMask+1+p->cW);{y numSonRefs=p->dG;if(p->fE)
numSonRefs <<= 1;gs(fL,p->son,numSonRefs);}}if(p->cm==p->dG)
p->cm=0;gG(p);}J g * Hc_GetMatchesSpec(y cl,g bc,g bG,z Z *bv,dF *son,y bx,g bQ,g fG,g *d,f bS){z Z *lim=bv+cl;son[bx]=bc;do{g dW;if(bc==0)
bL;dW=bG-bc;if(dW >= bQ)
bL;{dm diff;bc=son[bx-dW+((dW > bx)? bQ:0)];diff=(dm)0-(dm)dW;if(bv[bS]==bv[(dm)bS+diff]){z Z *c=bv;bK(*c==c[diff]){if(++c==lim){d[0]=(g)(lim-bv);d[1]=dW-1;u d+2;}}{z f H=(f)(c-bv);if(bS<H){bS=H;d[0]=(g)H;d[1]=dW-1;d+=2;}}}}}bK(--fG);u d;}g * GetMatchesSpec1(g cl,g bc,g bG,z Z *bv,dF *son,y bx,g bQ,g fG,g *d,g bS){dF *ptr0=son+((y)bx << 1)+ 1;dF *ptr1=son+((y)bx << 1);f len0=0,len1=0;g gH;gH=(g)(bG-bQ);if((g)bG<=bQ)
gH=0;if(gH<bc)
do{z g dW=bG-bc;{dF *hW=son+((y)(bx-dW+((dW > bx)? bQ:0))<< 1);z Z *pb=bv-dW;f H=(len0<len1 ? len0:len1);z g pair0=hW[0];if(pb[H]==bv[H]){if(++H!=cl&&pb[H]==bv[H])
bK(++H!=cl)
if(pb[H]!=bv[H])
bL;if(bS<H){bS=(g)H;*d++=(g)H;*d++=dW-1;if(H==cl){*ptr1=pair0;*ptr0=hW[1];u d;}}}if(pb[H]<bv[H]){*ptr1=bc;bc=hW[1];ptr1=hW+1;len1=H;}U{*ptr0=bc;bc=hW[0];ptr0=hW;len0=H;}}}bK(--fG&&gH<bc);*ptr0=*ptr1=fJ;u d;}J D fM(g cl,g bc,g bG,z Z *bv,dF *son,y bx,g bQ,g fG){dF *ptr0=son+((y)bx << 1)+ 1;dF *ptr1=son+((y)bx << 1);f len0=0,len1=0;g gH;gH=(g)(bG-bQ);if((g)bG<=bQ)
gH=0;if(
gH<bc)
do{z g dW=bG-bc;{dF *hW=son+((y)(bx-dW+((dW > bx)? bQ:0))<< 1);z Z *pb=bv-dW;f H=(len0<len1 ? len0:len1);if(pb[H]==bv[H]){bK(++H!=cl)
if(pb[H]!=bv[H])
bL;{if(H==cl){*ptr1=hW[0];*ptr0=hW[1];u;}}}if(pb[H]<bv[H]){*ptr1=bc;bc=hW[1];ptr1=hW+1;len1=H;}U{*ptr0=bc;bc=hW[0];ptr0=hW;len0=H;}}}bK(--fG&&gH<bc);*ptr0=*ptr1=fJ;u;}
#define eb p->cm++;p->eq++;{z g pos1=p->bG+1;p->bG=pos1;if(pos1==p->posLimit)ge(p);}
#define el eb u K;
J D hC(B *p){eb
}
#define hD(minLen,ret_op)g hv;z Z *bv;g bc;g cl=p->cl;if(cl<minLen){hC(p);ret_op;}bv=p->eq;
#define de(minLen)hD(minLen,u K)
#define ib(minLen)do{hD(minLen,fa)
#define MF_PARAMS(p)cl,bc,p->bG,p->eq,p->son,p->cm,p->dG,p->fG
#define ic fM(MF_PARAMS(p));eb }bK(--hz);
#define gf(_maxLen_,func)K=func(MF_PARAMS(p),K,(g)_maxLen_);el
#define dR(_maxLen_)gf(_maxLen_,GetMatchesSpec1)
#define gI(_maxLen_)gf(_maxLen_,Hc_GetMatchesSpec)
#define fv{z dm diff=(dm)0-(dm)d2;z Z *c=bv+bS;z Z *lim=bv+cl;cr(;c!=lim;c++)if(*(c+diff)!= *c)bL;bS=(f)(c-bv);}
J g* id(D *_p,g *K){B *p=(B *)_p;de(2)
HASH2_CALC
bc=p->bf[hv];p->bf[hv]=p->bG;dR(1)
}
#define SET_mmm mmm=p->dG;if(bG<mmm)mmm=bG;
J g* ie(D *_p,g *K){B *p=(B *)_p;g mmm;g h2,d2,bG;f bS;g *bf;de(3)
HASH3_CALC
bf=p->bf;bG=p->bG;d2=bG-bf[h2];bc=(bf+bA)[hv];bf[h2]=bG;(bf+bA)[hv]=bG;SET_mmm
bS=2;if(d2<mmm&&*(bv-d2)== *bv){fv
K[0]=(g)bS;K[1]=d2-1;K+=2;if(bS==cl){fM(MF_PARAMS(p));el
}}dR(bS)
}J g* ig(D *_p,g *K){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bG;f bS;g *bf;de(4)
HASH4_CALC
bf=p->bf;bG=p->bG;d2=bG-bf [h2];d3=bG-(bf+bA)[h3];bc=(bf+cX)[hv];bf [h2]=bG;(bf+bA)[h3]=bG;(bf+cX)[hv]=bG;SET_mmm
bS=3;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){K[0]=2;K[1]=d2-1;K+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;K[1]=d3-1;K+=2;}U
bL;}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;K[1]=d3-1;K+=2;}U
bL;fv
K[-2]=(g)bS;if(bS==cl){fM(MF_PARAMS(p));el
}bL;}dR(bS)
}J g* ih(D *_p,g *K){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bG;f bS;g *bf;de(5)
HASH5_CALC
bf=p->bf;bG=p->bG;d2=bG-bf [h2];d3=bG-(bf+bA)[h3];bc=(bf+dz)[hv];bf [h2]=bG;(bf+bA)[h3]=bG;(bf+dz)[hv]=bG;SET_mmm
bS=4;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){K[0]=2;K[1]=d2-1;K+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){K[1]=d3-1;K+=2;d2=d3;}U
bL;}U if(d3<mmm&&*(bv-d3)== *bv){K[1]=d3-1;K+=2;d2=d3;}U
bL;K[-2]=3;if(*(bv-d2+3)!= bv[3])
bL;fv
K[-2]=(g)bS;if(bS==cl){fM(MF_PARAMS(p));el
}bL;}dR(bS)
}J g* ii(D *_p,g *K){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bG;f bS;g *bf;de(4)
HASH4_CALC
bf=p->bf;bG=p->bG;d2=bG-bf [h2];d3=bG-(bf+bA)[h3];bc=(bf+cX)[hv];bf [h2]=bG;(bf+bA)[h3]=bG;(bf+cX)[hv]=bG;SET_mmm
bS=3;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){K[0]=2;K[1]=d2-1;K+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;K[1]=d3-1;K+=2;}U
bL;}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;K[1]=d3-1;K+=2;}U
bL;fv
K[-2]=(g)bS;if(bS==cl){p->son[p->cm]=bc;el
}bL;}gI(bS)
}J g * ij(D *_p,g *K){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bG;f bS;g *bf;de(5)
HASH5_CALC
bf=p->bf;bG=p->bG;d2=bG-bf [h2];d3=bG-(bf+bA)[h3];bc=(bf+dz)[hv];bf [h2]=bG;(bf+bA)[h3]=bG;(bf+dz)[hv]=bG;SET_mmm
bS=4;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){K[0]=2;K[1]=d2-1;K+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){K[1]=d3-1;K+=2;d2=d3;}U
bL;}U if(d3<mmm&&*(bv-d3)== *bv){K[1]=d3-1;K+=2;d2=d3;}U
bL;K[-2]=3;if(*(bv-d2+3)!= bv[3])
bL;fv
K[-2]=(g)bS;if(bS==cl){p->son[p->cm]=bc;el
}bL;}gI(bS)
}J D Bt2_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ib(2){HASH2_CALC
bc=p->bf[hv];p->bf[hv]=p->bG;}ic
}J D Bt3_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ib(3){g h2;g *bf;HASH3_CALC
bf=p->bf;bc=(bf+bA)[hv];bf[h2]=(bf+bA)[hv]=p->bG;}ic
}J D Bt4_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ib(4){g h2,h3;g *bf;HASH4_CALC
bf=p->bf;bc=(bf+cX)[hv];bf [h2]=(bf+bA)[h3]=(bf+cX)[hv]=p->bG;}ic
}J D Bt5_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ib(5){g h2,h3;g *bf;HASH5_CALC
bf=p->bf;bc=(bf+dz)[hv];bf [h2]=(bf+bA)[h3]=(bf+dz)[hv]=p->bG;}ic
}
#define HC_SKIP_HEADER(minLen)do{if(p->cl<minLen){hC(p);hz--;fa;}{z Z *bv;g *bf;g *son;g bG=p->bG;g num2=hz;{z g gm=p->posLimit-bG;if(num2 > gm)num2=gm;}hz-=num2;{z g cycPos=p->cm;son=p->son+cycPos;p->cm=cycPos+num2;}bv=p->eq;bf=p->bf;do{g bc;g hv;
#define HC_SKIP_FOOTER bv++;bG++;*son++=bc;}bK(--num2);p->eq=bv;p->bG=bG;if(bG==p->posLimit)ge(p);}}bK(hz);
J D Hc4_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;HC_SKIP_HEADER(4)
g h2,h3;HASH4_CALC
bc=(bf+cX)[hv];bf [h2]=(bf+bA)[h3]=(bf+cX)[hv]=bG;HC_SKIP_FOOTER
}J D Hc5_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;HC_SKIP_HEADER(5)
g h2,h3;HASH5_CALC
bc=(bf+dz)[hv];bf [h2]=(bf+bA)[h3]=(bf+dz)[hv]=bG;HC_SKIP_FOOTER
}D MatchFinder_CreateVTable(B *p,IMatchFinder2 *eK){eK->Init=MatchFinder_Init;eK->dl=gy;eK->bH=gc;if(!p->fE){if(p->M<=4){eK->ft=ii;eK->Skip=Hc4_MatchFinder_Skip;}U{eK->ft=ij;eK->Skip=Hc5_MatchFinder_Skip;}}U if(p->M==2){eK->ft=id;eK->Skip=Bt2_MatchFinder_Skip;}U if(p->M==3){eK->ft=ie;eK->Skip=Bt3_MatchFinder_Skip;}U if(p->M==4){eK->ft=ig;eK->Skip=Bt4_MatchFinder_Skip;}U{eK->ft=ih;eK->Skip=Bt5_MatchFinder_Skip;}}
#undef eb
#undef el
dN UInt16 G;
#define dn 5
bp CLzmaProps{Z lc;Z lp;Z pb;Z _pad_;g fw;};
#define bq 20
bp ez{CLzmaProps prop;G *A;G *probs_1664;Z *dic;y cz;y bC;z Z *eR;g Q;g fx;g bh;g cn;g ba[4];g E;g cA;g gt;f bU;Z eh[bq];};
#define LzmaDec_CONSTRUCT(p){(p)->dic=0;(p)->A=0;}
dN enum{LZMA_FINISH_ANY,}ELzmaFinishMode;dN enum{fy,gX,fT,cK,fg
}ELzmaStatus;
#define kTopValue ((g)1 << 24)
#define bl 11
#define bV (1 << bl)
#define RC_INIT_SIZE 5
#define dT 5
#define NORMALIZE if(Q<kTopValue){Q <<= 8;fx=(fx << 8)|(*eR++);}
#define fc(p)he=*(p);NORMALIZE;bound=(Q >> bl)* (g)he;if(fx<bound)
#define fd(p)Q=bound;*(p)= (G)(he+((bV-he)>> dT));
#define fe(p)Q-=bound;fx-=bound;*(p)= (G)(he-(he >> dT));
#define GET_BIT2(p,i,A0,A1)fc(p){fd(p)i=(i+i);A0;}U{fe(p)i=(i+i)+ 1;A1;}
#define bZ(A,i){GET_BIT2(A+i,i,;,;);}
#define REV_BIT(p,i,A0,A1)fc(p+i){fd(p+i)A0;}U{fe(p+i)A1;}
#define REV_BIT_VAR(p,i,m)REV_BIT(p,i,i+=m;m+=m,m+=m;i+=m;)
#define REV_BIT_CONST(p,i,m)REV_BIT(p,i,i+=m;,i+=m * 2;)
#define REV_BIT_LAST(p,i,m)REV_BIT(p,i,i-=m ,;)
#define TREE_DECODE(A,ct,i){i=1;do{bZ(A,i);}bK(i<ct);i-=ct;}
#define TREE_6_DECODE(A,i){i=1;bZ(A,i)bZ(A,i)bZ(A,i)bZ(A,i)bZ(A,i)bZ(A,i)i-=0x40;}
#define cL bZ(F,fj)
#define cB bu+=bu;cu=dO;dO&= bu;probLit=F+(dO+cu+fj);GET_BIT2(probLit,fj,dO^=cu;,;)
#define hf if(Q<kTopValue){if(eR >= fN)u hg;Q <<= 8;fx=(fx << 8)|(*eR++);}
#define em(p)he=*(p);hf bound=(Q >> bl)* (g)he;if(fx<bound)
#define cD Q=bound;
#define cE Q-=bound;fx-=bound;
#define GET_BIT2_(p,i,A0,A1)em(p){cD i=(i+i);A0;}U{cE i=(i+i)+ 1;A1;}
#define GET_BIT_(p,i)GET_BIT2_(p,i,;,;)
#define TREE_DECODE_(A,ct,i){i=1;do{GET_BIT_(A+i,i)}bK(i<ct);i-=ct;}
#define REV_BIT_(p,i,m)em(p+i){cD i+=m;m+=m;}U{cE m+=m;i+=m;}
#define hN 4
#define kNumPosStatesMax (1 << hN)
#define bM 3
#define R (1 << bM)
#define dq 8
#define df (1 << dq)
#define LenLow 0
#define LenHigh (LenLow+2 * (kNumPosStatesMax << bM))
#define kNumLenProbs (LenHigh+df)
#define LenChoice LenLow
#define LenChoice2 (LenLow+(1 << bM))
#define bi 12
#define kNumStates2 16
#define dX 7
#define dA 4
#define cC 14
#define bd (1 << (cC >> 1))
#define bo 6
#define W 4
#define ci 4
#define dV (1 << ci)
#define kMatchMinLen 2
#define cp (kMatchMinLen+R * 2+df)
#define cI (1 << 9)
#define fU (cI-1)
#define kStartOffset 1664
#define GET_PROBS p->probs_1664
#define SpecPos (-kStartOffset)
#define IsRep0Long (SpecPos+bd)
#define RepLenCoder (IsRep0Long+(kNumStates2 << hN))
#define LenCoder (RepLenCoder+kNumLenProbs)
#define IsMatch (LenCoder+kNumLenProbs)
#define Align (IsMatch+(kNumStates2 << hN))
#define IsRep (Align+dV)
#define IsRepG0 (IsRep+bi)
#define IsRepG1 (IsRepG0+bi)
#define IsRepG2 (IsRepG1+bi)
#define PosSlot (IsRepG2+bi)
#define Literal (PosSlot+(W << bo))
#define NUM_BASE_PROBS (Literal+kStartOffset)
#define LZMA_LIT_SIZE 0x300
#define gJ(p)(NUM_BASE_PROBS+((g)LZMA_LIT_SIZE << ((p)->lc+(p)->lp)))
#define CALC_POS_STATE(bh,fk)(((bh)&(fk))<< 4)
#define eL (P+E)
#define gK (P)
#define LZMA_DIC_MIN (1 << 12)
J
L LzmaDec_DecodeReal_3(ez *p,y ct,z Z *fN){G *A=GET_PROBS;f E=(f)p->E;g rep0=p->ba[0],rep1=p->ba[1],rep2=p->ba[2],rep3=p->ba[3];f fk=((f)1 << (p->prop.pb))- 1;f lc=p->prop.lc;f lpMask=((f)0x100 << p->prop.lp)- ((f)0x100 >> lc);Z *dic=p->dic;y cz=p->cz;y bC=p->bC;g bh=p->bh;g cn=p->cn;f H=0;z Z *eR=p->eR;g Q=p->Q;g fx=p->fx;do{G *F;g bound;f he;f P=CALC_POS_STATE(bh,fk);F=A+IsMatch+eL;fc(F){f fj;fd(F)
F=A+Literal;if(bh!=0||cn!=0)
F+=(g)3 * ((((bh << 8)+ dic[(bC==0 ? cz:bC)- 1])&lpMask)<< lc);bh++;if(E<dX){E-=(E<4)? E:3;fj=1;cL
cL
cL
cL
cL
cL
cL
cL
}U{f bu=dic[bC-rep0+(bC<rep0 ? cz:0)];f dO=0x100;E-=(E<10)? 3:6;fj=1;{f cu;G *probLit;cB
cB
cB
cB
cB
cB
cB
cB
}}dic[bC++]=(Z)fj;fa;}{fe(F)
F=A+IsRep+E;fc(F){fd(F)
E+=bi;F=A+LenCoder;}U{fe(F)
F=A+IsRepG0+E;fc(F){fd(F)
F=A+IsRep0Long+eL;fc(F){fd(F)
dic[bC]=dic[bC-rep0+(bC<rep0 ? cz:0)];bC++;bh++;E=E<dX ? 9:11;fa;}fe(F)
}U{g ce;fe(F)
F=A+IsRepG1+E;fc(F){fd(F)
ce=rep1;}U{fe(F)
F=A+IsRepG2+E;fc(F){fd(F)
ce=rep2;}U{fe(F)
ce=rep3;rep3=rep2;}rep2=rep1;}rep1=rep0;rep0=ce;}E=E<dX ? 8:11;F=A+RepLenCoder;}{G *cj=F+LenChoice;fc(cj){fd(cj)
cj=F+LenLow+gK;H=1;bZ(cj,H)
bZ(cj,H)
bZ(cj,H)
H-=8;}U{fe(cj)
cj=F+LenChoice2;fc(cj){fd(cj)
cj=F+LenLow+gK+(1 << bM);H=1;bZ(cj,H)
bZ(cj,H)
bZ(cj,H)
}U{fe(cj)
cj=F+LenHigh;TREE_DECODE(cj,(1 << dq),H)
H+=R * 2;}}}if(E >= bi){g ce;F=A+PosSlot +
((H<W ? H:W-1)<< bo);TREE_6_DECODE(F,ce)
if(ce >= dA){f cY=(f)ce;f cQ=(f)(((ce >> 1)- 1));ce=(2 | (ce&1));if(cY<cC){ce <<= cQ;F=A+SpecPos;{g m=1;ce++;do{REV_BIT_VAR(F,ce,m)
}bK(--cQ);ce-=m;}}U{cQ-=ci;do{NORMALIZE
Q >>= 1;{g t;fx-=Q;t=(0-((g)fx >> 31));ce=(ce << 1)+ (t+1);fx+=Q&t;}}bK(--cQ);F=A+Align;ce <<= ci;{f i=1;REV_BIT_CONST(F,i,1)
REV_BIT_CONST(F,i,2)
REV_BIT_CONST(F,i,4)
REV_BIT_LAST (F,i,8)
ce |= i;}if(ce==(g)0xFFFFFFFF){H=cp;E-=bi;bL;}}}rep3=rep2;rep2=rep1;rep1=rep0;rep0=ce+1;E=(E<bi+dX)? dX:dX+3;if(ce >= (cn==0 ? bh:cn)){H+=cI+kMatchMinLen;bL;}}H+=kMatchMinLen;{y gm;f curLen;y bG;if((gm=ct-bC)== 0){bL;}curLen=((gm<H)? (f)gm:H);bG=bC-rep0+(bC<rep0 ? cz:0);bh+=(g)curLen;H-=curLen;if(curLen<=cz-bG){Z *gu=dic+bC;dm hh=(dm)bG-(dm)bC;z Z *lim=gu+curLen;bC+=(y)curLen;do
*(gu)= (Z)*(gu+hh);bK(++gu!=lim);}U{do{dic[bC++]=dic[bG];if(++bG==cz)
bG=0;}bK(--curLen!=0);}}}}bK(bC<ct&&eR<fN);NORMALIZE
p->eR=eR;p->Q=Q;p->fx=fx;p->cA=(g)H;p->bC=bC;p->bh=bh;p->ba[0]=rep0;p->ba[1]=rep1;p->ba[2]=rep2;p->ba[3]=rep3;p->E=(g)E;if(H >= cI)
u dy;u cy;}J D LzmaDec_WriteRem(ez *p,y ct){f H=(f)p->cA;if(H==0)
u;{y bC=p->bC;Z *dic;y cz;y rep0;{y gm=ct-bC;if(gm<H){H=(f)(gm);if(H==0)
u;}}if(p->cn==0&&p->prop.fw-p->bh<=H)
p->cn=p->prop.fw;p->bh+=(g)H;p->cA-=(g)H;dic=p->dic;rep0=p->ba[0];cz=p->cz;do{dic[bC]=dic[bC-rep0+(bC<rep0 ? cz:0)];bC++;}bK(--H);p->bC=bC;}}
#define kRange0 0xFFFFFFFF
#define kBound0 ((kRange0 >> bl)<< (bl-1))
#define kBadRepCode (kBound0+(((kRange0-kBound0)>> bl)<< (bl-1)))
J L hE(ez *p,y ct,z Z *fN){if(p->cn==0){g gm=p->prop.fw-p->bh;if(ct-p->bC > gm)
ct=p->bC+gm;}{L du=LzmaDec_DecodeReal_3(p,ct,fN);if(p->cn==0&&p->bh >= p->prop.fw)
p->cn=p->prop.fw;u du;}}dN enum{hg,DUMMY_LIT,DUMMY_MATCH,DUMMY_REP
}ELzmaDummy;
#define eG(hi)((hi)== DUMMY_MATCH)
J ELzmaDummy LzmaDec_TryDummy(z ez *p,z Z *eR,z Z **bufOut){g Q=p->Q;g fx=p->fx;z Z *fN=*bufOut;z G *A=GET_PROBS;f E=(f)p->E;ELzmaDummy du;cr(;;){z G *F;g bound;f he;f P=CALC_POS_STATE(p->bh,((f)1 << p->prop.pb)- 1);F=A+IsMatch+eL;em(F){cD
F=A+Literal;if(p->cn!=0||p->bh!=0)
F+=((g)LZMA_LIT_SIZE *
((((p->bh)&(((f)1 << (p->prop.lp))- 1))<< p->prop.lc)+
((f)p->dic[(p->bC==0 ? p->cz:p->bC)- 1] >> (8-p->prop.lc))));if(E<dX){f fj=1;do{GET_BIT_(F+fj,fj)}bK(fj<0x100);}U{f bu=p->dic[p->bC-p->ba[0] +
(p->bC<p->ba[0] ? p->cz:0)];f dO=0x100;f fj=1;do{f cu;z G *probLit;bu+=bu;cu=dO;dO&= bu;probLit=F+(dO+cu+fj);GET_BIT2_(probLit,fj,dO^=cu;,;)
}bK(fj<0x100);}du=DUMMY_LIT;}U{f H;cE
F=A+IsRep+E;em(F){cD
E=0;F=A+LenCoder;du=DUMMY_MATCH;}U{cE
du=DUMMY_REP;F=A+IsRepG0+E;em(F){cD
F=A+IsRep0Long+eL;em(F){cD
bL;}U{cE
}}U{cE
F=A+IsRepG1+E;em(F){cD
}U{cE
F=A+IsRepG2+E;em(F){cD
}U{cE
}}}E=bi;F=A+RepLenCoder;}{f ct,cx;z G *cj=F+LenChoice;em(cj){cD
cj=F+LenLow+gK;cx=0;ct=1 << bM;}U{cE
cj=F+LenChoice2;em(cj){cD
cj=F+LenLow+gK+(1 << bM);cx=R;ct=1 << bM;}U{cE
cj=F+LenHigh;cx=R * 2;ct=1 << dq;}}TREE_DECODE_(cj,ct,H)
H+=cx;}if(E<4){f cY;F=A+PosSlot +
((H<W-1 ? H:W-1)<<
bo);TREE_DECODE_(F,1 << bo,cY)
if(cY >= dA){f cQ=((cY >> 1)- 1);if(cY<cC){F=A+SpecPos+((2 | (cY&1))<< cQ);}U{cQ-=ci;do{hf
Q >>= 1;fx-=Q&(((fx-Q)>> 31)- 1);}bK(--cQ);F=A+Align;cQ=ci;}{f i=1;f m=1;do{REV_BIT_(F,i,m)
}bK(--cQ);}}}}bL;}hf
*bufOut=eR;u du;}D LzmaDec_InitDicAndState(ez *p,L initDic,L initState){p->cA=cp+1;p->bU=0;if(initDic){p->bh=0;p->cn=0;p->cA=cp+2;}if(initState)
p->cA=cp+2;}D LzmaDec_Init(ez *p){p->bC=0;LzmaDec_InitDicAndState(p,1,1);}
#define di *fl=fT;u dy;
L LzmaDec_DecodeToDic(ez *p,y dicLimit,z Z *hh,y *cM,ELzmaFinishMode finishMode,ELzmaStatus *fl){y et=*cM;(*cM)= 0;*fl=fy;if(p->cA > cp){if(p->cA > cp+2)
u p->cA==fU ? SZ_ERROR_FAIL:dy;cr(;et > 0&&p->bU<RC_INIT_SIZE;(*cM)++,et--)
p->eh[p->bU++]=*hh++;if(p->bU!=0&&p->eh[0]!=0)
u dy;if(p->bU<RC_INIT_SIZE){*fl=cK;u cy;}p->fx=((g)p->eh[1] << 24)
| ((g)p->eh[2] << 16)
| ((g)p->eh[3] << 8)
| ((g)p->eh[4]);if(p->cn==0
&&p->bh==0
&&p->fx >= kBadRepCode)
u dy;p->Q=0xFFFFFFFF;p->bU=0;if(p->cA > cp+1){y gt=gJ(&p->prop);y i;G *A=p->A;cr(i=0;i<gt;i++)
A[i]=bV >> 1;p->ba[0]=p->ba[1]=p->ba[2]=p->ba[3]=1;p->E=0;}p->cA=0;}cr(;;){if(p->cA==cp){if(p->fx!=0)
u dy;*fl=gX;u cy;}LzmaDec_WriteRem(p,dicLimit);{L eA=0;if(p->bC >= dicLimit){if(p->cA==0&&p->fx==0){*fl=fg;u cy;}if(finishMode==LZMA_FINISH_ANY){*fl=fT;u cy;}if(p->cA!=0){di
}eA=1;}if(p->bU==0){z Z *fN;L bF=-1;if(et<bq||eA){z Z *bufOut=hh+et;ELzmaDummy hi=LzmaDec_TryDummy(p,hh,&bufOut);if(hi==hg){y i;if(et >= bq)
bL;(*cM)+= et;p->bU=(f)et;cr(i=0;i<et;i++)
p->eh[i]=hh[i];*fl=cK;u cy;}bF=(L)(bufOut-hh);if((f)bF > bq)
bL;if(eA&&!eG(hi)){f i;(*cM)+= (f)bF;p->bU=(f)bF;cr(i=0;i<(f)bF;i++)
p->eh[i]=hh[i];di
}fN=hh;}U
fN=hh+et-bq;p->eR=hh;{L du=hE(p,dicLimit,fN);y bR=(y)(p->eR-hh);if(bF<0){if(bR > et)
bL;}U if((f)bF!=bR)
bL;hh+=bR;et-=bR;(*cM)+= bR;if(du!=cy){p->cA=cI;u dy;}}fa;}{f gm=p->bU;f ahead=0;L bF=-1;bK(gm<bq&&ahead<et)
p->eh[gm++]=hh[ahead++];if(gm<bq||eA){z Z *bufOut=p->eh+gm;ELzmaDummy hi=LzmaDec_TryDummy(p,p->eh,&bufOut);if(hi==hg){if(gm >= bq)
bL;p->bU=gm;(*cM)+= (y)ahead;*fl=cK;u cy;}bF=(L)(bufOut-p->eh);if((f)bF<p->bU)
bL;if(eA&&!eG(hi)){(*cM)+= (f)bF-p->bU;p->bU=(f)bF;di
}}p->eR=p->eh;{L du=hE(p,dicLimit,p->eR);y bR=(y)(p->eR-p->eh);gm=p->bU;if(bF<0){if(bR > bq)
bL;if(bR<gm)
bL;}U if((f)bF!=bR)
bL;bR-=gm;hh+=bR;et-=bR;(*cM)+= bR;p->bU=0;if(du!=cy){p->cA=cI;u dy;}}}}}p->cA=fU;u SZ_ERROR_FAIL;}D LzmaDec_FreeProbs(ez *p){free(p->A);p->A=0;}L LzmaProps_Decode(CLzmaProps *p,z Z *V,f cV){g fw;Z d;if(cV<dn)
u gY;U
fw=V[1] | ((g)V[2] << 8)|((g)V[3] << 16)|((g)V[4] << 24);if(fw<LZMA_DIC_MIN)
fw=LZMA_DIC_MIN;p->fw=fw;d=V[0];if(d >= (9 * 5 * 5))
u gY;p->lc=(Z)(d % 9);d /= 9;p->pb=(Z)(d / 5);p->lp=(Z)(d % 5);u cy;}J L LzmaDec_AllocateProbs2(ez *p,z CLzmaProps *propNew){g gt=gJ(propNew);if(!p->A||gt!=p->gt){LzmaDec_FreeProbs(p);p->A=(G *)SzAlloc(gt * ia(G));if(!p->A)
u ga;p->probs_1664=p->A+1664;p->gt=gt;}u cy;}L LzmaDec_AllocateProbs(ez *p,z Z *bJ,f gn){CLzmaProps propNew;RINOK(LzmaProps_Decode(&propNew,bJ,gn))
RINOK(LzmaDec_AllocateProbs2(p,&propNew))
p->prop=propNew;u cy;}L LzmaDecode(Z *gu,y *eT,z Z *hh,y *cM,z Z *propData,f propSize,ELzmaFinishMode finishMode,ELzmaStatus *fl){ez p;L du;y outSize=*eT,et=*cM;*eT=*cM=0;*fl=fy;if(et<RC_INIT_SIZE)
u fX;LzmaDec_CONSTRUCT(&p)
RINOK(LzmaDec_AllocateProbs(&p,propData,propSize))
p.dic=gu;p.cz=outSize;LzmaDec_Init(&p);*cM=et;du=LzmaDec_DecodeToDic(&p,outSize,hh,cM,finishMode,fl);*eT=p.bC;if(du==cy&&*fl==cK)
du=fX;LzmaDec_FreeProbs(&p);u du;}bp dY{L fh;g bj;L lc,lp,pb,algo,fb,fE,M;f dL;g mc;f cU;L eH;fI reduceSize,affinity;};bp bk;
#define hF ((g)15 << 28)
#define kTopValue ((g)1 << 24)
#define bl 11
#define bV (1 << bl)
#define dT 5
#define bW (bV >> 1)
#define by 4
#define eM 4
#define REP_LEN_COUNT 64
D LzmaEncProps_Init(dY *p){p->fh=5;p->bj=p->mc=0;p->reduceSize=(fI)(int64_t)-1;p->lc=p->lp=p->pb=p->algo=p->fb=p->fE=p->M=p->eH=-1;p->dL=0;p->cU=0;p->affinity=0;}D LzmaEncProps_Normalize(dY *p){L fh=p->fh;if(fh<0)fh=5;p->fh=fh;if(p->bj==0)
p->bj=(f)fh<=4 ?
(g)1 << (fh * 2+16):(f)fh<=ia(y)/ 2+4 ?
(g)1 << (fh+20):(g)1 << (ia(y)/ 2+24);if(p->bj > p->reduceSize){g v=(g)p->reduceSize;z g kReduceMin=((g)1 << 12);if(v<kReduceMin)
v=kReduceMin;if(p->bj > v)
p->bj=v;}if(p->lc<0)p->lc=3;if(p->lp<0)p->lp=0;if(p->pb<0)p->pb=2;if(p->algo<0)p->algo=(f)fh<5 ? 0:1;if(p->fb<0)p->fb=(f)fh<7 ? 32:64;if(p->fE<0)p->fE=(p->algo==0 ? 0:1);if(p->M<0)p->M=(p->fE ? 4:5);if(p->mc==0)p->mc=(16+((f)p->fb >> 1))>> (p->fE ? 0:1);if(p->eH<0)
p->eH=1;}
#define gL (11+ia(y)/ 8 * 3)
#define kDicLogSizeMaxCompress ((gL-1)* 2+7)
J D LzmaEnc_FastPosInit(Z *en){f gg;en[0]=0;en[1]=1;en+=2;cr(gg=2;gg<gL * 2;gg++){y k=((y)1 << ((gg >> 1)- 1));y j;cr(j=0;j<k;j++)
en[j]=(Z)gg;en+=k;}}
#define BSR2_RET(bG,du){f zz=(bG<(1 << (gL+6)))? 6:6+gL-1;du=p->en[bG >> zz]+(zz * 2);}
#define GetPosSlot1(bG)p->en[bG]
#define GetPosSlot2(bG,du){BSR2_RET(bG,du);}
#define GetPosSlot(bG,du){if(bG<bd)du=p->en[bG&(bd-1)];U BSR2_RET(bG,du);}
#define bb 4
dN UInt16 CState;dN UInt16 CExtra;bp eB{g I;CState E,bO;g H,bn,ba[bb];};
#define hj (1 << 11)
#define kPackReserve (hj * 8)
#define W 4
#define bo 6
#define kDicLogSizeMax 32
#define kDistTableSizeMax (kDicLogSizeMax * 2)
#define ci 4
#define dV (1 << ci)
#define kAlignMask (dV-1)
#define dA 4
#define cC 14
#define bd (1 << (cC >> 1))
#define LZMA_PB_MAX 4
#define LZMA_LC_MAX 8
#define LZMA_LP_MAX 4
#define bI (1 << LZMA_PB_MAX)
#define bM 3
#define R (1 << bM)
#define dq 8
#define df (1 << dq)
#define hG (R * 2+df)
#define dP 2
#define bB (dP+hG-1)
#define bi 12
bp CLenEnc{G low[bI << (bM+1)],high[df];};bp CLenPriceEnc{f tableSize;g eN[bI][hG];};
#define dB(p,P,H)((p)->eN[P][(y)(H)- dP])
bp dr{g Q;f cache;fI low,cacheSize;Z *eR,*bufLim,*ch;gC* dM;fI bR;L du;};bp CSaveState{G *dg;f E;g ba[bb];G cT[1 << ci],isRep[bi],isRepG0[bi],isRepG1[bi],isRepG2[bi],fz[bi][bI],gv[bi][bI],eU[W][1 << bo],ik[bd];CLenEnc lenProbs,gM;};bp bk{D *bz;IMatchFinder2 cg;f optCur,optEnd,fO,bg;g bE;f E,ca,cc;g ba[bb];f lpMask,fk;G *dg;dr rc;g eC;f lc,lp,pb,lclp;L fastMode,cU,finished,needInit;fI nowPos64;f hk;L fm;f distTableSize;g bj;L fH;B matchFinderBase;g be[bV >> by],eD[bB * 2+2];g il[dV],bY[W][kDistTableSizeMax],hl[W][bd];G cT[1 << ci],isRep[bi],isRepG0[bi],isRepG1[bi],isRepG2[bi],fz[bi][bI],gv[bi][bI],eU[W][1 << bo],ik[bd];CLenEnc lenProbs,gM;Z en[1 << gL];CLenPriceEnc lenEnc,eO;eB bP[hj];CSaveState hm;};
#define MFB (p->matchFinderBase)
L LzmaEnc_SetProps(bk* p,z dY *props2){dY bJ=*props2;LzmaEncProps_Normalize(&bJ);if(bJ.lc > LZMA_LC_MAX
|| bJ.lp > LZMA_LP_MAX
|| bJ.pb > LZMA_PB_MAX)
u hM;if(bJ.bj > hF)
bJ.bj=hF;{z fI dict64=bJ.bj;if(dict64 > ((fI)1 << kDicLogSizeMaxCompress))
u hM;}p->bj=bJ.bj;{f fb=(f)bJ.fb;if(fb<5)
fb=5;if(fb > bB)
fb=bB;p->ca=fb;}p->lc=(f)bJ.lc;p->lp=(f)bJ.lp;p->pb=(f)bJ.pb;p->fastMode=(bJ.algo==0);MFB.fE=(Z)(bJ.fE ? 1:0);{f M=4;if(bJ.fE){if(bJ.M<2)M=2;U if(bJ.M<4)M=(f)bJ.M;}if(bJ.M >= 5)M=5;MFB.M=M;MFB.dL=(Z)bJ.dL;}MFB.fG=bJ.mc;p->cU=(L)bJ.cU;u cy;}D LzmaEnc_SetDataSize(bk* p,fI expectedDataSiize){MFB.dH=expectedDataSiize;}
#define kState_Start 0
#define kState_LitAfterMatch 4
#define kState_LitAfterRep 5
#define kState_MatchAfterLit 7
#define hX 8
J z Z fY[bi] ={0,0,0,0,1,2,3,4,5,6,4,5};J z Z fn[bi] ={7,7,7,7,7,7,7,10,10,10,10,10};J z Z hO[bi] ={8,8,8,8,8,8,8,11,11,11,11,11};J z Z hH[bi]={9,9,9,9,9,9,9,11,11,11,11,11};
#define IsLitState(s)((s)< 7)
#define GetLenToPosState2(H)(((H)< W-1)? (H):W-1)
#define GetLenToPosState(H)(((H)< W+1)? (H)- 2:W-1)
#define hP (1 << 30)
J D RangeEnc_Construct(dr *p){p->dM=0;p->ch=0;}
#define hQ(p)((y)(p)->bR+(y)((p)->eR-(p)->ch)+ (y)(p)->cacheSize)
#define RC_BUF_SIZE (1 << 16)
J L RangeEnc_Alloc(dr *p){if(!p->ch){p->ch=(Z *)SzAlloc(RC_BUF_SIZE);if(!p->ch)
u 0;p->bufLim=p->ch+RC_BUF_SIZE;}u 1;}J D RangeEnc_Free(dr *p){free(p->ch);p->ch=0;}J D RangeEnc_Init(dr *p){p->Q=0xFFFFFFFF;p->cache=0;p->low=0;p->cacheSize=0;p->eR=p->ch;p->bR=0;p->du=cy;}J D fi(dr *p){z y hz=(y)(p->eR-p->ch);if(p->du==cy){if(hz!=p->dM->Write(p->dM,p->ch,hz))
p->du=SZ_ERROR_WRITE;}p->bR+=hz;p->eR=p->ch;}J D RangeEnc_ShiftLow(dr *p){g low=(g)p->low;f high=(f)(p->low >> 32);p->low=(g)(low << 8);if(low<(g)0xFF000000||high!=0){{Z *eR=p->eR;*eR++=(Z)(p->cache+high);p->cache=(f)(low >> 24);p->eR=eR;if(eR==p->bufLim)
fi(p);if(p->cacheSize==0)
u;}high+=0xFF;cr(;;){Z *eR=p->eR;*eR++=(Z)(high);p->eR=eR;if(eR==p->bufLim)
fi(p);if(--p->cacheSize==0)
u;}}p->cacheSize++;}J D RangeEnc_FlushData(dr *p){L i;cr(i=0;i<5;i++)
RangeEnc_ShiftLow(p);}
#define RC_NORM(p)if(Q<kTopValue){Q <<= 8;RangeEnc_ShiftLow(p);}
#define cN(p,F)he=*(F);dI=(Q >> bl)* he;
#define gN(p,F,cu){g mask;cN(p,F)mask=0-(g)cu;Q&= mask;mask&= dI;Q-=mask;(p)->low+=mask;mask=(g)cu-1;Q+=dI&mask;mask&= (bV-((1 << dT)- 1));mask+=((1 << dT)- 1);he+=(g)((L)(mask-he)>> dT);*(F)= (G)he;RC_NORM(p)}
#define gO(p,F)Q=dI;*(F)= (G)(he+((bV-he)>> dT));
#define RC_BIT_1_BASE(p,F)Q-=dI;(p)->low+=dI;*(F)= (G)(he-(he >> dT));
#define RC_BIT_0(p,F)gO(p,F)RC_NORM(p)
#define fP(p,F)RC_BIT_1_BASE(p,F)RC_NORM(p)
J D RangeEnc_EncodeBit_0(dr *p,G *F){g Q,he,dI;Q=p->Q;cN(p,F)
RC_BIT_0(p,F)
p->Q=Q;}J D LitEnc_Encode(dr *p,G *A,g cw){g Q=p->Q;cw |= 0x100;do{g he,dI;G *F=A+(cw >> 8);g cu=(cw >> 7)&1;cw <<= 1;gN(p,F,cu)
}bK(cw<0x10000);p->Q=Q;}J D LitEnc_EncodeMatched(dr *p,G *A,g cw,g bu){g Q=p->Q;g dO=0x100;cw |= 0x100;do{g he,dI;G *F;g cu;bu <<= 1;F=A+(dO+(bu&dO)+ (cw >> 8));cu=(cw >> 7)&1;cw <<= 1;dO&= ~(bu ^ cw);gN(p,F,cu)
}bK(cw<0x10000);p->Q=Q;}J D LzmaEnc_InitPriceTables(g *be){g i;cr(i=0;i<(bV >> by);i++){z f kCyclesBits=eM;g w=(i << by)+ (1 << (by-1));f bitCount=0;f j;cr(j=0;j<kCyclesBits;j++){w=w * w;bitCount <<= 1;bK(w >= ((g)1 << 16)){w >>= 1;bitCount++;}}be[i]=(g)(((f)bl << kCyclesBits)- 15-bitCount);}}
#define GET_PRICE(F,cu)p->be[((F)^ (f)(((-(L)(cu)))&(bV-1)))>> by]
#define cO(F,cu)be[((F)^ (f)((-((L)(cu)))&(bV-1)))>> by]
#define cZ(F)p->be[(F)>> by]
#define dv(F)p->be[((F)^ (bV-1))>> by]
#define eo(F)be[(F)>> by]
#define ep(F)be[((F)^ (bV-1))>> by]
J g LitEnc_GetPrice(z G *A,g cw,z g *be){g I=0;cw |= 0x100;do{f cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);}bK(cw >= 2);u I;}J g dw(z G *A,g cw,g bu,z g *be){g I=0;g dO=0x100;cw |= 0x100;do{bu <<= 1;I+=cO(A[dO+(bu&dO)+ (cw >> 8)],(cw >> 7)&1);cw <<= 1;dO&= ~(bu ^ cw);}bK(cw<0x10000);u I;}J D RcTree_ReverseEncode(dr *rc,G *A,f hA,f cw){g Q=rc->Q;f m=1;do{g he,dI;f cu=cw&1;cw >>= 1;gN(rc,A+m,cu)
m=(m << 1)| cu;}bK(--hA);rc->Q=Q;}J D LenEnc_Init(CLenEnc *p){f i;cr(i=0;i<(bI << (bM+1));i++)
p->low[i]=bW;cr(i=0;i<df;i++)
p->high[i]=bW;}J D LenEnc_Encode(CLenEnc *p,dr *rc,f cw,f P){g Q,he,dI;G *A=p->low;Q=rc->Q;cN(rc,A)
if(cw >= R){fP(rc,A)
A+=R;cN(rc,A)
if(cw >= R * 2){fP(rc,A)
rc->Q=Q;LitEnc_Encode(rc,p->high,cw-R * 2);u;}cw-=R;}{f m;f cu;RC_BIT_0(rc,A)
A+=(P << (1+bM));cu=(cw >> 2);gN(rc,A+1,cu)m=(1 << 1)+ cu;cu=(cw >> 1)&1;gN(rc,A+m,cu)m=(m << 1)+ cu;cu=cw&1;gN(rc,A+m,cu)
rc->Q=Q;}}J D SetPrices_3(z G *A,g startPrice,g *eN,z g *be){f i;cr(i=0;i<8;i+=2){g I=startPrice;g F;I+=cO(A[1 ],(i >> 2));I+=cO(A[2+(i >> 2)],(i >> 1)&1);F=A[4+(i >> 1)];eN[i ]=I+eo(F);eN[i+1]=I+ep(F);}}J D dk(
CLenPriceEnc *p,f numPosStates,z CLenEnc *enc,z g *be){g b;{f F=enc->low[0];g a,c;f P;b=ep(F);a=eo(F);c=b+eo(enc->low[R]);cr(P=0;P<numPosStates;P++){g *eN=p->eN[P];z G *A=enc->low+(P << (1+bM));SetPrices_3(A,a,eN,be);SetPrices_3(A+R,c,eN+R,be);}}{f i=p->tableSize;if(i > R * 2){z G *A=enc->high;g *eN=p->eN[0]+R * 2;i-=R * 2-1;i >>= 1;b+=ep(enc->low[R]);do{f cw=--i+(1 << (dq-1));g I=b;do{z f cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);}bK(cw >= 2);{z f F=A[(y)i+(1 << (dq-1))];eN[(y)i * 2 ]=I+eo(F);eN[(y)i * 2+1]=I+ep(F);}}bK(i);{f P;z y hz=(p->tableSize-R * 2)* ia(p->eN[0][0]);cr(P=1;P<numPosStates;P++)
memcpy(p->eN[P]+R * 2,p->eN[0]+R * 2,hz);}}}}
#define eb(p,hz){p->cc+=(hz);p->cg.Skip(p->bz,(g)(hz));}
J f dQ(bk *p,f *numPairsRes){f bg;p->cc++;p->bE=p->cg.dl(p->bz);{z g *d=p->cg.ft(p->bz,p->eD);bg=(f)(d-p->eD);}*numPairsRes=bg;if(bg==0)
u 0;{z f H=p->eD[(y)bg-2];if(H!=p->ca)
u H;{g bE=p->bE;if(bE > bB)
bE=bB;{z Z *p1=p->cg.bH(p->bz)- 1;z Z *p2=p1+H;z dm dif=(dm)-1-(dm)p->eD[(y)bg-1];z Z *lim=p1+bE;cr(;p2!=lim&&*p2==p2[dif];p2++){}u(f)(p2-p1);}}}}
#define hn ((g)(L)-1)
#define MakeAs_Lit(p){(p)->bn=hn;(p)->bO=0;}
#define MakeAs_ShortRep(p){(p)->bn=0;(p)->bO=0;}
#define IsShortRep(p)((p)->bn==0)
#define GetPrice_ShortRep(p,E,P)(cZ(p->isRepG0[E])+ cZ(p->gv[E][P]))
#define hR(p,E,P)(dv(p->fz[E][P])+ dv(p->gv[E][P]))+ dv(p->isRep[E])+ cZ(p->isRepG0[E])
J g GetPrice_PureRep(z bk *p,f ds,y E,y P){g I;g F=p->isRepG0[E];if(ds==0){I=cZ(F);I+=dv(p->gv[E][P]);}U{I=dv(F);F=p->isRepG1[E];if(ds==1)
I+=cZ(F);U{I+=dv(F);I+=GET_PRICE(p->isRepG2[E],ds-2);}}u I;}J f Backward(bk *p,f bv){f wr=bv+1;p->optEnd=wr;cr(;;){g bn=p->bP[bv].bn;f H=(f)p->bP[bv].H;f bO=(f)p->bP[bv].bO;bv-=H;if(bO){wr--;p->bP[wr].H=(g)H;bv-=bO;H=bO;if(bO==1){p->bP[wr].bn=bn;bn=hn;}U{p->bP[wr].bn=0;H--;wr--;p->bP[wr].bn=hn;p->bP[wr].H=1;}}if(bv==0){p->eC=bn;p->optCur=wr;u H;}wr--;p->bP[wr].bn=bn;p->bP[wr].H=(g)H;}}
#define LIT_PROBS(bG,prevByte)(p->dg+(g)3 * (((((bG)<< 8)+ (prevByte))&p->lpMask)<< p->lc))
J f GetOptimum(bk *p,g dJ){f gP,bv;g ba[bb];f gQ[bb];g *eD;{g bE;f bg,bD,eP,i,P;g fA,dC;z Z *V;Z da,bu;p->optCur=p->optEnd=0;if(p->cc==0)
bD=dQ(p,&bg);U{bD=p->fO;bg=p->bg;}bE=p->bE;if(bE<2){p->eC=hn;u 1;}if(bE > bB)
bE=bB;V=p->cg.bH(p->bz)- 1;eP=0;cr(i=0;i<bb;i++){f H;z Z *db;ba[i]=p->ba[i];db=V-ba[i];if(V[0]!=db[0]||V[1]!=db[1]){gQ[i]=0;fa;}cr(H=2;H<bE&&V[H]==db[H];H++){}gQ[i]=H;if(H > gQ[eP])
eP=i;if(H==bB)
bL;}if(gQ[eP] >= p->ca){f H;p->eC=(g)eP;H=gQ[eP];eb(p,H-1)
u H;}eD=p->eD;
#define gh eD
if(bD >= p->ca){p->eC=gh[(y)bg-1]+bb;eb(p,bD-1)
u bD;}da=*V;bu=*(V-ba[0]);gP=gQ[eP];if(gP<=bD)
gP=bD;if(gP<2&&da!=bu){p->eC=hn;u 1;}p->bP[0].E=(CState)p->E;P=(dJ&p->fk);{z G *A=LIT_PROBS(dJ,*(V-1));p->bP[1].I=cZ(p->fz[p->E][P])+
(!IsLitState(p->E)?
dw(A,da,bu,p->be):LitEnc_GetPrice(A,da,p->be));}MakeAs_Lit(&p->bP[1])
fA=dv(p->fz[p->E][P]);dC=fA+dv(p->isRep[p->E]);if(bu==da&&gQ[0]==0){g fB=dC+GetPrice_ShortRep(p,p->E,P);if(fB<p->bP[1].I){p->bP[1].I=fB;MakeAs_ShortRep(&p->bP[1])
}if(gP<2){p->eC=p->bP[1].bn;u 1;}}p->bP[1].H=1;p->bP[0].ba[0]=ba[0];p->bP[0].ba[1]=ba[1];p->bP[0].ba[2]=ba[2];p->bP[0].ba[3]=ba[3];cr(i=0;i<bb;i++){f eQ=gQ[i];g I;if(eQ<2)
fa;I=dC+GetPrice_PureRep(p,i,p->E,P);do{g dK=I+dB(&p->eO,P,eQ);eB *bP =&p->bP[eQ];if(dK<bP->I){bP->I=dK;bP->H=(g)eQ;bP->bn=(g)i;bP->bO=0;}}bK(--eQ >= 2);}{f H=gQ[0]+1;if(H<=bD){f dO=0;g gR=fA+cZ(p->isRep[p->E]);if(H<2)
H=2;U
bK(H > gh[dO])
dO+=2;cr(;;H++){eB *bP;g bn=gh[(y)dO+1];g I=gR+dB(&p->lenEnc,P,H);f lenToPosState=GetLenToPosState(H);if(bn<bd)
I+=p->hl[lenToPosState][bn&(bd-1)];U{f gg;GetPosSlot2(bn,gg)
I+=p->il[bn&kAlignMask];I+=p->bY[lenToPosState][gg];}bP =&p->bP[H];if(I<bP->I){bP->I=I;bP->H=(g)H;bP->bn=bn+bb;bP->bO=0;}if(H==gh[dO]){dO+=2;if(dO==bg)
bL;}}}}bv=0;}cr(;;){f bE;g co;f im,bg,prev,E,P,startLen;g gw,fA,dC;L nextIsLit;Z da,bu;z Z *V;eB *fo,*ei;if(++bv==gP)
bL;if(bv >= hj-64){f j,best;g I=p->bP[bv].I;best=bv;cr(j=bv+1;j<=gP;j++){g dK=p->bP[j].I;if(I >= dK){I=dK;best=j;}}{f dW=best-bv;if(dW!=0){eb(p,dW)
}}bv=best;bL;}im=dQ(p,&bg);if(im >= p->ca){p->bg=bg;p->fO=im;bL;}fo =&p->bP[bv];dJ++;prev=bv-fo->H;if(fo->H==1){E=(f)p->bP[prev].E;if(IsShortRep(fo))
E=hH[E];U
E=fY[E];}U{z eB *eV;g b0;g bn=fo->bn;if(fo->bO){prev-=(f)fo->bO;E=hX;if(fo->bO==1)
E=(bn<bb ? hX:kState_MatchAfterLit);}U{E=(f)p->bP[prev].E;if(bn<bb)
E=hO[E];U
E=fn[E];}eV =&p->bP[prev];b0=eV->ba[0];if(bn<bb){if(bn==0){ba[0]=b0;ba[1]=eV->ba[1];ba[2]=eV->ba[2];ba[3]=eV->ba[3];}U{ba[1]=b0;b0=eV->ba[1];if(bn==1){ba[0]=b0;ba[2]=eV->ba[2];ba[3]=eV->ba[3];}U{ba[2]=b0;ba[0]=eV->ba[bn];ba[3]=eV->ba[bn ^ 1];}}}U{ba[0]=(bn-bb+1);ba[1]=b0;ba[2]=eV->ba[1];ba[3]=eV->ba[2];}}fo->E=(CState)E;fo->ba[0]=ba[0];fo->ba[1]=ba[1];fo->ba[2]=ba[2];fo->ba[3]=ba[3];V=p->cg.bH(p->bz)- 1;da=*V;bu=*(V-ba[0]);P=(dJ&p->fk);{g curPrice=fo->I;f F=p->fz[E][P];fA=curPrice+dv(F);gw=curPrice+cZ(F);}ei =&p->bP[(y)bv+1];nextIsLit=0;if((ei->I<hP
&&bu==da)
|| gw > ei->I
)
gw=0;U{z G *A=LIT_PROBS(dJ,*(V-1));gw+=(!IsLitState(E)?
dw(A,da,bu,p->be):LitEnc_GetPrice(A,da,p->be));if(gw<ei->I){ei->I=gw;ei->H=1;MakeAs_Lit(ei)
nextIsLit=1;}}dC=fA+dv(p->isRep[E]);co=p->bE;{f hV=hj-1-bv;if(co > hV)
co=(g)hV;}if(IsLitState(E))
if(bu==da)
if(dC<ei->I)
if(
ei->H<2
|| (ei->bn!=0
)){g fB=dC+GetPrice_ShortRep(p,E,P);if(fB<ei->I){ei->I=fB;ei->H=1;MakeAs_ShortRep(ei)
nextIsLit=0;}}if(co<2)
fa;bE=(co<=p->ca ? co:p->ca);if(!nextIsLit
&&gw!=0
&&bu!=da
&&co > 2){z Z *db=V-ba[0];if(V[1]==db[1]&&V[2]==db[2]){f H;f ct=p->ca+1;if(ct > co)
ct=co;cr(H=3;H<ct&&V[H]==db[H];H++){}{f in=fY[E];f dd=(dJ+1)&p->fk;g I=gw+hR(p,in,dd);{f cx=bv+H;if(gP<cx)
gP=cx;{g dK;eB *bP;H--;dK=I+dB(&p->eO,dd,H);bP =&p->bP[cx];if(dK<bP->I){bP->I=dK;bP->H=(g)H;bP->bn=0;bP->bO=1;}}}}}}startLen=2;{f ds=0;cr(;ds<bb;ds++){f H;g I;z Z *db=V-ba[ds];if(V[0]!=db[0]||V[1]!=db[1])
fa;cr(H=2;H<bE&&V[H]==db[H];H++){}{f cx=bv+H;if(gP<cx)
gP=cx;}{f dh=H;I=dC+GetPrice_PureRep(p,ds,E,P);do{g dK=I+dB(&p->eO,P,dh);eB *bP =&p->bP[bv+dh];if(dK<bP->I){bP->I=dK;bP->H=(g)dh;bP->bn=(g)ds;bP->bO=0;}}bK(--dh >= 2);}if(ds==0)startLen=H+1;{f dh=H+1;f ct=dh+p->ca;if(ct > co)
ct=co;dh+=2;if(dh<=ct)
if(V[dh-2]==db[dh-2])
if(V[dh-1]==db[dh-1]){f in=hO[E];f dd=(dJ+H)&p->fk;I+=dB(&p->eO,P,H)
+ cZ(p->fz[in][dd])
+ dw(LIT_PROBS(dJ+H,V[(y)H-1]),V[H],db[H],p->be);in=kState_LitAfterRep;dd=(dd+1)&p->fk;I+=hR(p,in,dd);cr(;dh<ct&&V[dh]==db[dh];dh++){}dh-=H;{{f cx=bv+H+dh;if(gP<cx)
gP=cx;{g dK;eB *bP;dh--;dK=I+dB(&p->eO,dd,dh);bP =&p->bP[cx];if(dK<bP->I){bP->I=dK;bP->H=(g)dh;bP->bO=(CExtra)(H+1);bP->bn=(g)ds;}}}}}}}}if(im > bE){im=bE;cr(bg=0;im > gh[bg];bg+=2);gh[bg]=(g)im;bg+=2;}if(im >= startLen){g gR=fA+cZ(p->isRep[E]);g bn;f dO,cY,H;{f cx=bv+im;if(gP<cx)
gP=cx;}dO=0;bK(startLen > gh[dO])
dO+=2;bn=gh[(y)dO+1];GetPosSlot2(bn,cY)
cr(H=startLen;;H++){g I=gR+dB(&p->lenEnc,P,H);{eB *bP;f lenNorm=H-2;lenNorm=GetLenToPosState2(lenNorm);if(bn<bd)
I+=p->hl[lenNorm][bn&(bd-1)];U
I+=p->bY[lenNorm][cY]+p->il[bn&kAlignMask];bP =&p->bP[bv+H];if(I<bP->I){bP->I=I;bP->H=(g)H;bP->bn=bn+bb;bP->bO=0;}}if(H==gh[dO]){z Z *db=V-bn-1;f dh=H+1;f ct=dh+p->ca;if(ct > co)
ct=co;dh+=2;if(dh<=ct)
if(V[dh-2]==db[dh-2])
if(V[dh-1]==db[dh-1]){cr(;dh<ct&&V[dh]==db[dh];dh++){}dh-=H;{f in=fn[E];f dd=(dJ+H)&p->fk;f cx;I+=cZ(p->fz[in][dd]);I+=dw(LIT_PROBS(dJ+H,V[(y)H-1]),V[H],db[H],p->be);in=kState_LitAfterMatch;dd=(dd+1)&p->fk;I+=hR(p,in,dd);cx=bv+H+dh;if(gP<cx)
gP=cx;{g dK;eB *bP;dh--;dK=I+dB(&p->eO,dd,dh);bP =&p->bP[cx];if(dK<bP->I){bP->I=dK;bP->H=(g)dh;bP->bO=(CExtra)(H+1);bP->bn=bn+bb;}}}}dO+=2;if(dO==bg)
bL;bn=gh[(y)dO+1];GetPosSlot2(bn,cY)
}}}}do
p->bP[gP].I=hP;bK(--gP);u Backward(p,bv);}
#define ChangePair(smallDist,bigDist)(((bigDist)>> 7)>(smallDist))
J f GetOptimumFast(bk *p){g bE,eE;f bD,bg,ds,eQ,i;z Z *V;if(p->cc==0)
bD=dQ(p,&bg);U{bD=p->fO;bg=p->bg;}bE=p->bE;p->eC=hn;if(bE<2)
u 1;if(bE > bB)
bE=bB;V=p->cg.bH(p->bz)- 1;eQ=ds=0;cr(i=0;i<bb;i++){f H;z Z *db=V-p->ba[i];if(V[0]!=db[0]||V[1]!=db[1])
fa;cr(H=2;H<bE&&V[H]==db[H];H++){}if(H >= p->ca){p->eC=(g)i;eb(p,H-1)
u H;}if(H > eQ){ds=i;eQ=H;}}if(bD >= p->ca){p->eC=p->eD[(y)bg-1]+bb;eb(p,bD-1)
u bD;}eE=0;if(bD >= 2){eE=p->eD[(y)bg-1];bK(bg > 2){g dist2;if(bD!=p->eD[(y)bg-4]+1)
bL;dist2=p->eD[(y)bg-3];if(!ChangePair(dist2,eE))
bL;bg-=2;bD--;eE=dist2;}if(bD==2&&eE >= 0x80)
bD=1;}if(eQ >= 2)
if(eQ+1 >= bD
|| (eQ+2 >= bD&&eE >= (1 << 9))
|| (eQ+3 >= bD&&eE >= (1 << 15))){p->eC=(g)ds;eb(p,eQ-1)
u eQ;}if(bD<2||bE<=2)
u 1;{f len1=dQ(p,&p->bg);p->fO=len1;if(len1 >= 2){g newDist=p->eD[(y)p->bg-1];if((len1 >= bD&&newDist<eE)
|| (len1==bD+1&&!ChangePair(eE,newDist))
|| (len1 > bD+1)
|| (len1+1 >= bD&&bD >= 3&&ChangePair(newDist,eE)))
u 1;}}V=p->cg.bH(p->bz)- 1;cr(i=0;i<bb;i++){f H,ct;z Z *db=V-p->ba[i];if(V[0]!=db[0]||V[1]!=db[1])
fa;ct=bD-1;cr(H=2;;H++){if(H >= ct)
u 1;if(V[H]!=db[H])
bL;}}p->eC=eE+bb;if(bD!=2){eb(p,bD-2)
}u bD;}J D WriteEndMarker(bk *p,f P){g Q;Q=p->rc.Q;{g he,dI;G *F =&p->fz[p->E][P];cN(&p->rc,F)
fP(&p->rc,F)
F =&p->isRep[p->E];cN(&p->rc,F)
RC_BIT_0(&p->rc,F)
}p->E=fn[p->E];p->rc.Q=Q;LenEnc_Encode(&p->lenProbs,&p->rc,0,P);Q=p->rc.Q;{G *A=p->eU[0];f m=1;do{g he,dI;cN(p,A+m)
fP(&p->rc,A+m)
m=(m << 1)+ 1;}bK(m<(1 << bo));}{f hA=30-ci;do{Q >>= 1;p->rc.low+=Q;RC_NORM(&p->rc)
}bK(--hA);}{G *A=p->cT;f m=1;do{g he,dI;cN(p,A+m)
fP(&p->rc,A+m)
m=(m << 1)+ 1;}bK(m<dV);}p->rc.Q=Q;}J L CheckErrors(bk *p){if(p->fH!=cy)
u p->fH;if(p->rc.du!=cy)
p->fH=SZ_ERROR_WRITE;if(MFB.fH!=cy)
p->fH=8;if(p->fH!=cy)
p->finished=1;u p->fH;}J L Flush(bk *p,g nowPos){p->finished=1;if(p->cU)
WriteEndMarker(p,nowPos&p->fk);RangeEnc_FlushData(&p->rc);fi(&p->rc);u CheckErrors(p);}J D FillAlignPrices(bk *p){f i;z g *be=p->be;z G *A=p->cT;cr(i=0;i<dV / 2;i++){g I=0;f cw=i;f m=1;f cu;g F;cu=cw&1;cw >>= 1;I+=cO(A[m],cu);m=(m << 1)+ cu;cu=cw&1;cw >>= 1;I+=cO(A[m],cu);m=(m << 1)+ cu;cu=cw&1;cw >>= 1;I+=cO(A[m],cu);m=(m << 1)+ cu;F=A[m];p->il[i ]=I+eo(F);p->il[i+8]=I+ep(F);}}J D hI(bk *p){g tempPrices[bd];f i,lps;z g *be=p->be;p->hk=0;cr(i=dA / 2;i<bd / 2;i++){f cY=GetPosSlot1(i);f eI=(cY >> 1)- 1;f base=((2 | (cY&1))<< eI);z G *A=p->ik+(y)base * 2;g I=0;f m=1;f cw=i;f cx=(f)1 << eI;base+=i;if(eI)
do{f cu=cw&1;cw >>= 1;I+=cO(A[m],cu);m=(m << 1)+ cu;}bK(--eI);{f F=A[m];tempPrices[base ]=I+eo(F);tempPrices[base+cx]=I+ep(F);}}cr(lps=0;lps<W;lps++){f gg;f distTableSize2=(p->distTableSize+1)>> 1;g *bY=p->bY[lps];z G *A=p->eU[lps];cr(gg=0;gg<distTableSize2;gg++){g I;f cu;f cw=gg+(1 << (bo-1));f F;cu=cw&1;cw >>= 1;I=cO(A[cw],cu);cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);cu=cw&1;cw >>= 1;I+=cO(A[cw],cu);F=A[(y)gg+(1 << (bo-1))];bY[(y)gg * 2 ]=I+eo(F);bY[(y)gg * 2+1]=I+ep(F);}{g dW=((g)((cC / 2-1)- ci)<< eM);cr(gg=cC / 2;gg<distTableSize2;gg++){bY[(y)gg * 2 ]+=dW;bY[(y)gg * 2+1]+=dW;dW+=((g)1 << eM);}}{g *dp=p->hl[lps];dp[0]=bY[0];dp[1]=bY[1];dp[2]=bY[2];dp[3]=bY[3];cr(i=4;i<bd;i+=2){g slotPrice=bY[GetPosSlot1(i)];dp[i ]=slotPrice+tempPrices[i];dp[i+1]=slotPrice+tempPrices[i+1];}}}}J D LzmaEnc_Construct(bk *p){RangeEnc_Construct(&p->rc);MatchFinder_Construct(&MFB);{dY bJ;LzmaEncProps_Init(&bJ);LzmaEnc_SetProps((bk*)(D *)p,&bJ);}LzmaEnc_FastPosInit(p->en);LzmaEnc_InitPriceTables(p->be);p->dg=0;p->hm.dg=0;}bk* LzmaEnc_Create(){D *p;p=SzAlloc(ia(bk));if(p)
LzmaEnc_Construct((bk *)p);u(bk*)p;}J D gS(bk *p){free(p->dg);free(p->hm.dg);p->dg=0;p->hm.dg=0;}J D LzmaEnc_Destruct(bk *p){MatchFinder_Free(&MFB);gS(p);RangeEnc_Free(&p->rc);}D LzmaEnc_Destroy(bk* p){LzmaEnc_Destruct(p);free(p);}J L LzmaEnc_CodeOneBlock(bk *p,g maxPackSize,g maxUnpackSize){g ec,startPos32;if(p->needInit){p->cg.Init(p->bz);p->needInit=0;}if(p->finished)
u p->fH;RINOK(CheckErrors(p))
ec=(g)p->nowPos64;startPos32=ec;if(p->nowPos64==0){f bg;Z da;if(p->cg.dl(p->bz)== 0)
u Flush(p,ec);dQ(p,&bg);RangeEnc_EncodeBit_0(&p->rc,&p->fz[kState_Start][0]);da=*(p->cg.bH(p->bz)- p->cc);LitEnc_Encode(&p->rc,p->dg,da);p->cc--;ec++;}if(p->cg.dl(p->bz)!= 0)
cr(;;){g bn;f H,P;g Q,he,dI;G *A;if(p->fastMode)
H=GetOptimumFast(p);U{f oci=p->optCur;if(p->optEnd==oci)
H=GetOptimum(p,ec);U{z eB *bP =&p->bP[oci];H=bP->H;p->eC=bP->bn;p->optCur=oci+1;}}P=(f)ec&p->fk;Q=p->rc.Q;A =&p->fz[p->E][P];cN(&p->rc,A)
bn=p->eC;if(bn==hn){Z da;z Z *V;f E;RC_BIT_0(&p->rc,A)
p->rc.Q=Q;V=p->cg.bH(p->bz)- p->cc;A=LIT_PROBS(ec,*(V-1));da=*V;E=p->E;p->E=fY[E];if(IsLitState(E))
LitEnc_Encode(&p->rc,A,da);U
LitEnc_EncodeMatched(&p->rc,A,da,*(V-p->ba[0]));}U{fP(&p->rc,A)
A =&p->isRep[p->E];cN(&p->rc,A)
if(bn<bb){fP(&p->rc,A)
A =&p->isRepG0[p->E];cN(&p->rc,A)
if(bn==0){RC_BIT_0(&p->rc,A)
A =&p->gv[p->E][P];cN(&p->rc,A)
if(H!=1){RC_BIT_1_BASE(&p->rc,A)
}U{gO(&p->rc,A)
p->E=hH[p->E];}}U{fP(&p->rc,A)
A =&p->isRepG1[p->E];cN(&p->rc,A)
if(bn==1){gO(&p->rc,A)
bn=p->ba[1];}U{fP(&p->rc,A)
A =&p->isRepG2[p->E];cN(&p->rc,A)
if(bn==2){gO(&p->rc,A)
bn=p->ba[2];}U{RC_BIT_1_BASE(&p->rc,A)
bn=p->ba[3];p->ba[3]=p->ba[2];}p->ba[2]=p->ba[1];}p->ba[1]=p->ba[0];p->ba[0]=bn;}RC_NORM(&p->rc)
p->rc.Q=Q;if(H!=1){LenEnc_Encode(&p->gM,&p->rc,H-dP,P);--p->fm;p->E=hO[p->E];}}U{f cY;RC_BIT_0(&p->rc,A)
p->rc.Q=Q;p->E=fn[p->E];LenEnc_Encode(&p->lenProbs,&p->rc,H-dP,P);bn-=bb;p->ba[3]=p->ba[2];p->ba[2]=p->ba[1];p->ba[1]=p->ba[0];p->ba[0]=bn+1;p->hk++;GetPosSlot(bn,cY){g cw=(g)cY+(1 << bo);Q=p->rc.Q;A=p->eU[GetLenToPosState(H)];do{G *F=A+(cw >> bo);g cu=(cw >> (bo-1))&1;cw <<= 1;gN(&p->rc,F,cu)
}bK(cw<(1 << bo * 2));p->rc.Q=Q;}if(bn >= dA){f eI=((cY >> 1)- 1);if(bn<bd){f base=((2 | (cY&1))<< eI);RcTree_ReverseEncode(&p->rc,p->ik+base,eI,(f)(bn));}U{g pos2=(bn | 0xF)<< (32-eI);Q=p->rc.Q;do{Q >>= 1;p->rc.low+=Q&(0-(pos2 >> 31));pos2+=pos2;RC_NORM(&p->rc)
}bK(pos2!=0xF0000000);{f m=1;f cu;cu=bn&1;bn >>= 1;gN(&p->rc,p->cT+m,cu)m=(m << 1)+ cu;cu=bn&1;bn >>= 1;gN(&p->rc,p->cT+m,cu)m=(m << 1)+ cu;cu=bn&1;bn >>= 1;gN(&p->rc,p->cT+m,cu)m=(m << 1)+ cu;cu=bn&1;gN(&p->rc,p->cT+m,cu)
p->rc.Q=Q;}}}}}ec+=(g)H;p->cc-=H;if(p->cc==0){g bR;if(!p->fastMode){if(p->hk >= 64){FillAlignPrices(p);hI(p);dk(&p->lenEnc,(f)1 << p->pb,&p->lenProbs,p->be);}if(p->fm<=0){p->fm=REP_LEN_COUNT;dk(&p->eO,(f)1 << p->pb,&p->gM,p->be);}}if(p->cg.dl(p->bz)== 0)
bL;bR=ec-startPos32;if(maxPackSize){if(bR+hj+300 >= maxUnpackSize
|| hQ(&p->rc)+ kPackReserve >= maxPackSize)
bL;}U if(bR >= (1 << 17)){p->nowPos64+=ec-startPos32;u CheckErrors(p);}}}p->nowPos64+=ec-startPos32;u Flush(p,ec);}
#define kBigHashDicLimit ((g)1 << 24)
J L LzmaEnc_Alloc(bk *p,g ed){g beforeSize=hj;g bj;if(!RangeEnc_Alloc(&p->rc))
u ga;{z f lclp=p->lc+p->lp;if(!p->dg||!p->hm.dg||p->lclp!=lclp){gS(p);p->dg=(G *)SzAlloc(((y)0x300 * ia(G))<< lclp);p->hm.dg=(G *)SzAlloc(((y)0x300 * ia(G))<< lclp);if(!p->dg||!p->hm.dg){gS(p);u ga;}p->lclp=lclp;}}MFB.bigHash=(Z)(p->bj > kBigHashDicLimit ? 1:0);bj=p->bj;if(bj==((g)2 << 30)||
bj==((g)3 << 30)){bj-=1;}if(beforeSize+bj<ed)
beforeSize=ed-bj;{if(!MatchFinder_Create(&MFB,bj,beforeSize,p->ca,bB+1))
u ga;p->bz =&MFB;MatchFinder_CreateVTable(&MFB,&p->cg);}u cy;}J D LzmaEnc_Init(bk *p){f i;p->E=0;p->ba[0]=p->ba[1]=p->ba[2]=p->ba[3]=1;RangeEnc_Init(&p->rc);cr(i=0;i<(1 << ci);i++)
p->cT[i]=bW;cr(i=0;i<bi;i++){f j;cr(j=0;j<bI;j++){p->fz[i][j]=bW;p->gv[i][j]=bW;}p->isRep[i]=bW;p->isRepG0[i]=bW;p->isRepG1[i]=bW;p->isRepG2[i]=bW;}{cr(i=0;i<W;i++){G *A=p->eU[i];f j;cr(j=0;j<(1 << bo);j++)
A[j]=bW;}}{cr(i=0;i<bd;i++)
p->ik[i]=bW;}{z y hz=(y)0x300 << (p->lp+p->lc);y k;G *A=p->dg;cr(k=0;k<hz;k++)
A[k]=bW;}LenEnc_Init(&p->lenProbs);LenEnc_Init(&p->gM);p->optEnd=0;p->optCur=0;{cr(i=0;i<hj;i++)
p->bP[i].I=hP;}p->cc=0;p->fk=((f)1 << p->pb)- 1;p->lpMask=((g)0x100 << p->lp)- ((f)0x100 >> p->lc);}J D LzmaEnc_InitPrices(bk *p){if(!p->fastMode){hI(p);FillAlignPrices(p);}p->lenEnc.tableSize=p->eO.tableSize=p->ca+1-dP;p->fm=REP_LEN_COUNT;dk(&p->lenEnc,(f)1 << p->pb,&p->lenProbs,p->be);dk(&p->eO,(f)1 << p->pb,&p->gM,p->be);}J L LzmaEnc_AllocAndInit(bk *p,g ed){f i;cr(i=cC / 2;i<kDicLogSizeMax;i++)
if(p->bj<=((g)1 << i))
bL;p->distTableSize=i * 2;p->finished=0;p->fH=cy;p->nowPos64=0;p->needInit=1;RINOK(LzmaEnc_Alloc(p,ed))
LzmaEnc_Init(p);LzmaEnc_InitPrices(p);u cy;}L LzmaEnc_MemPrepare(bk* p,z Z *hh,y cM,g ed){gx(&MFB,hh,cM)
LzmaEnc_SetDataSize(p,cM);u LzmaEnc_AllocAndInit(p,ed);}D LzmaEnc_Finish(bk* p){UNUSED_VAR(p)
}dN bp{gC vt;Z *V;y gm;L overflow;}fV;J y SeqOutStreamBuf_Write(gC* pp,z D *V,y cV){gb(pp,fV,vt,p)
if(p->gm<cV){cV=p->gm;p->overflow=1;}if(cV!=0){memcpy(p->V,V,cV);p->gm-=cV;p->V+=cV;}u cV;}J L LzmaEnc_Encode2(bk *p){L du=cy;cr(;;){du=LzmaEnc_CodeOneBlock(p,0,0);if(du!=cy||p->finished)
bL;}LzmaEnc_Finish((bk*)(D *)p);u du;}L LzmaEnc_WriteProperties(bk* p,Z *bJ,y *cV){if(*cV<dn)
u hM;*cV=dn;{z g bj=p->bj;g v;bJ[0]=(Z)((p->pb * 5+p->lp)* 9+p->lc);if(bj >= ((g)1 << 21)){z g kDictMask=((g)1 << 20)- 1;v=(bj+kDictMask)&~kDictMask;if(v<bj)
v=bj;}U{f i=11 * 2;do{v=(g)(2+(i&1))<< (i >> 1);i++;}bK(v<bj);}SetUi32(bJ+1,v)
u cy;}}L LzmaEnc_MemEncode(bk* p,Z *gu,y *eT,z Z *hh,y cM,L cU){L du;fV dM;dM.vt.Write=SeqOutStreamBuf_Write;dM.V=gu;dM.gm=*eT;dM.overflow=0;p->cU=cU;p->rc.dM =&dM.vt;du=LzmaEnc_MemPrepare(p,hh,cM,0);if(du==cy){du=LzmaEnc_Encode2(p);if(du==cy&&p->nowPos64!=cM)
du=SZ_ERROR_FAIL;}*eT-=(y)dM.gm;if(dM.overflow)
u 7;u du;}L LzmaEncode(Z *gu,y *eT,z Z *hh,y cM,z dY *bJ,Z *propsEncoded,y *gn,L cU){bk* p=LzmaEnc_Create();L du;if(!p)
u ga;du=LzmaEnc_SetProps(p,bJ);if(du==cy){du=LzmaEnc_WriteProperties(p,propsEncoded,gn);if(du==cy)
du=LzmaEnc_MemEncode(p,gu,eT,hh,cM,cU);}LzmaEnc_Destroy(p);u du;}L LzmaCompress(f gq*gu,y *eT,z f gq*hh,y cM,f gq*outProps,y *outPropsSize,L fh,f bj,L lc,L lp,L pb,L fb,L eH
){dY bJ;LzmaEncProps_Init(&bJ);bJ.fh=fh;bJ.bj=bj;bJ.lc=lc;bJ.lp=lp;bJ.pb=pb;bJ.fb=fb;bJ.eH=eH;u LzmaEncode(gu,eT,hh,cM,&bJ,outProps,outPropsSize,0);}L LzmaUncompress(f gq*gu,y *eT,z f gq*hh,y *cM,z f gq*bJ,y gn){ELzmaStatus fl;u LzmaDecode(gu,eT,hh,cM,bJ,(f)gn,LZMA_FINISH_ANY,&fl);}namespace sz{td::dx compress(td::Slice V){bT cV=V.cV();y ho=1 << 22;bT gu=new f gq[ho];f gq outProps[dn] ={};y outPropsSize=dn;L fh=9;f bj=1<<20;L lc=8;L lp=0;L pb=0;L fb=273;L eH=1;L du=LzmaCompress(gu,&ho,(f gq*)V.V(),cV,outProps,&outPropsSize,fh,bj,lc,lp,pb,fb,eH);(du==cy);u td::dx((gq*)gu,ho);}td::dx decompress(td::Slice V){bT cV=V.cV();y ho=1 << 22;bT gu=new f gq[ho];f gq bJ[dn] ={0x8,0,0,0,0x80
};y gn=dn;L du=LzmaUncompress(gu,&ho,(f gq*)V.V(),&cV,bJ,gn);(du==cy||du==fX);u td::dx((gq*)gu,ho);}}td::dx serialize_boc_opt(ostream&out,Ref<Cell> cell){(!cell.is_null());BagOfCells boc;boc.add_root(cell);(boc.import_cells().is_ok());bT cV=boc.estimate_serialized_size(0);td::dx bs{cV};bT eq=(f gq*)bs.V();boc_writers::BufferWriter fQ{eq,eq+cV};vector<L> backrefs(boc.cell_list_.cV(),0);cr(L i=0;i<boc.cell_count;++i){z bT&cell=boc.cell_list_[i];cr(L j=0;j<cell.ref_num;++j){backrefs[cell.gT[j]]+=1;}}bT ej=fQ.hp;bT hu=[&](gi label){if(ej<fQ.hp){out << label << ":" << td::ConstBitPtr{ej}.to_hex((fQ.hp-ej)* 8)<< endl;ej=fQ.hp;}};bT store_byte=[&](f long long ew){fQ.store_uint(ew,1);};bT bm=[&](f long long ew){fQ.store_uint(ew,boc.info.ref_byte_size);};bT overwrite_ref=[&](L dJ,f long long ew){bT ptr=fQ.hp;fQ.hp=fQ.store_start+dJ;bm(ew);fQ.hp=ptr;};store_byte(boc.info.ref_byte_size);hu("ref-cV");bm(0);hu("cell-hz");vector<L> idx_to_ref(boc.cell_list_.cV(),-1);vector<hW<L,y>> refs_to_set;function<D(L,z vm::BagOfCells::CellInfo&)> store_cell;store_cell=[&](L idx,z vm::BagOfCells::CellInfo&dc_info){f gq eR[256] ={};z Ref<DataCell>&dc=dc_info.dc_ref;L mask=0;cr(f j=0;j<dc_info.ref_num;++j){L gT=dc_info.gT[j];L br=backrefs.at(gT);if(br==1){mask |= (1 << j);}}L s=dc->serialize(eR,256,false);eR[0]=(eR[0]&15)+ mask * 16;fQ.store_bytes(eR,s);hu("V");cr(f j=0;j<dc_info.ref_num;++j){L gT=dc_info.gT[j];L br=backrefs.at(gT);(br > 0);if(br==1){store_cell(gT,boc.cell_list_[gT]);}U{refs_to_set.emplace_back(gT,fQ.dJ());bm(0);hu("ref");}}};L cells_cnt=0;cr(L i=0;i<boc.cell_count;++i){L k=boc.cell_count-1-i;z bT&dc_info=boc.cell_list_[k];L br=backrefs.at(k);if(br!=1){store_cell(k,dc_info);idx_to_ref[k]=cells_cnt++;}}overwrite_ref(1,cells_cnt);cr(z bT&p:refs_to_set){overwrite_ref(p.second,idx_to_ref[p.first]);}bs.truncate(fQ.dJ());u bs;}bp DeserializedCell{bool special;L bits;gi V;vector<hW<L,L>> refs{};};Ref<Cell> deserialize_boc_opt(ostream&out,td::Slice V){(!V.empty());L start=0;bT ej=0;bT hw=[&](gi label){if(ej<start){out << label << ":" << td::ConstBitPtr{(z f gq*)V.V()+ ej}.to_hex((start-ej)* 8)<< endl;ej=start;}};bT read_byte=[&](){(V.cV()> start);u(f gq)V[start++];};bT read_int=[&](L bytes){f long long du=0;bK(bytes > 0){du=(du << 8)+ read_byte();--bytes;}u du;};bT read_bytes=[&](L bytes){gi bs(bytes,0);cr(L i=0;i<bytes;++i){bs.at(i)= read_byte();}u bs;};bT ref_byte_size=(L)read_byte();hw("ref-cV");bT read_ref=[&](){u read_int(ref_byte_size);};bT cell_num=read_ref();hw("cell-hz");vector<DeserializedCell> ek;vector<array<hW<L,L>,4>> cells_refs;vector<L> ref_to_cd_idx(cell_num);function<D(L)> read_cell;read_cell=[&](L idx){bT d1=read_byte();bT d2=read_byte();bT ref_num=d1&7;bT special=(d1&8)> 0;bT mask=(d1 >> 4)&15;bT bytes=(d2+1)>> 1;bT V=read_bytes(bytes);hw("V");bT bits=bytes * 8;if(V.cV()> 0&&(d2&1)){f gq last_byte=V[V.cV()- 1];L fR=8;bK(fR&&(last_byte&1)== 0){fR--;last_byte >>= 1;}if(fR){last_byte=(last_byte&254)<< (8-fR);fR--;}V[V.cV()- 1]=last_byte;bits-=(8-fR);}if(idx!=-1){ref_to_cd_idx.at(idx)= ek.cV();}ek.push_back(DeserializedCell{special,bits,V,});L dc_idx=ek.cV()- 1;cr(L i=0;i<ref_num;++i){bool is_embedded=(mask >> i)&1;if(is_embedded){ek.at(dc_idx).refs.push_back({ek.cV(),-1});read_cell(-1);}U{ek.at(dc_idx).refs.push_back({-1,read_ref()});hw("ref");}}};cr(L i=0;i<cell_num;++i){read_cell(i);}vector<Ref<Cell>> cells(ek.cV());cr(L i=ek.cV()- 1;i >= 0;--i){bT&cd=ek[i];N cb;cb.store_bits(cd.V.V(),cd.bits);cr(z bT&ref:cd.refs){L idx=ref.first;if(idx==-1){(ref.second!=-1);idx=ref_to_cd_idx.at(ref.second);}bT&cell=cells.at(idx);(!cell.is_null());cb.bm(cell);}cells[i]=cb.finalize(cd.special);}u std::move(cells[0]);}td::dx do_compress(td::Slice V){u sz::compress(V);}td::dx do_decompress(td::Slice V){u sz::decompress(V);}td::dx compress(td::Slice V){NullStream ofs;Ref<Cell> block_root=std_boc_deserialize(V).move_as_ok();FullBlock block;q load_std_ctx{ofs};block.bw(load_std_ctx,block_root,0,true);q pack_opt_ctx{ofs};bT opt_block_cell=block.cP(pack_opt_ctx);bT opt_ser=serialize_boc_opt(ofs,opt_block_cell);bT compressed=do_compress(opt_ser);u compressed;}td::dx decompress(td::Slice V){NullStream ofs;bT decompressed=do_decompress(V);bT opt_deser=deserialize_boc_opt(ofs,decompressed);FullBlock opt_block;q parse_opt_ctx{ofs};opt_block.ck(parse_opt_ctx,opt_deser,0,true);q pack_std_ctx{ofs};bT un_opt_block_cell=opt_block.bt(pack_std_ctx);bT boc=std_boc_serialize(un_opt_block_cell,31).move_as_ok();u boc;}L main(
){gi mode;cin >> mode;gi base64_data;cin >> base64_data;td::dx V(td::base64_decode(base64_data));V=(mode=="compress")? compress(V):decompress(V);cout << td::str_base64_encode(V)<< endl;}