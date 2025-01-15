/* minified, contact for original */
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
#define id sizeof
#define hY pair
#define hU ShardAccountBlocks
#define hM HashmapAugNode
#define hJ cell_list_
#define hr in_msg_descr
#define hq as_cellslice
#define hp McBlockExtra
#define ha ShardFeeCreated
#define gZ class
#define gY template
#define gT ref_idx
#define gB out_msg_descr
#define gq char
#define gi string
#define fZ state_update
#define fY virtual
#define fU CurrencyCollection
#define fH uint64_t
#define eY continue
#define eV special_type
#define eH fetch_ulong
#define ed HashmapAugE
#define dY write
#define dN typedef
#define dJ position
#define dD append_cellslice
#define dx BufferSlice
#define dm ptrdiff_t
#define dh store_long
#define cV size
#define cr for
#define bV auto
#define bR extra
#define bN break
#define bM fetch_ref
#define bK while
#define bw struct
#define bm store_ref
#define V data
#define U else
#define O CellSlice
#define L static
#define K CellBuilder
#define I int
#define D void
#define z const
#define y size_t
#define u return
#define f unsigned
using namespace vm;using namespace std;set<gi> enabled_optimizations{"Block","BlockExtra","ed","HashmapAug","hM","InMsgDescr","OutMsgDescr","hp","ShardFees","hU","MERKLE_UPDATE","Maybe","ValueFlow",};O to_cs(Ref<Cell> cell){bool can_be_special=false;u load_cell_slice_special(std::move(cell),can_be_special);}bw q{ostream&out;};gY<gZ T_TLB>bw C{O ia;O dt;O ccs;gi name;T_TLB cz;I eV=0;C(gi name):C(name,T_TLB()){}C(gi name,T_TLB cz):name(name),cz(cz){}fY ~C(){}fY D bG(q&S,O&cs,I o=0){}fY D ct(q&S,K&cb,I o=0){cu(S,cb,o);}fY D bP(q&S,O&cs,I o=0){bG(S,cs,o);}fY D cu(q&S,K&cb,I o=0){}bool is_enabled(){u enabled_optimizations.count(name)> 0;}bool gz(){u eV==1;}D cK(q&S,O&cs,I o=0,bool gA=false){bV e=is_enabled();eV=(I)cs.eV();ia=cs;if(gz()){cs.advance(288);u;}if(e){bG(S,cs,o);}U{dt=cz.fetch(cs).dY();}}D es(q&S,K&cb,I o=0){bV e=is_enabled();if(gz()){cb.dD(ia);}U if(e){ct(S,cb,o);}U{cb.dD(dt);}}D cS(q&S,O&cs,I o=0,bool gA=false){bV e=is_enabled();ia=cs;if(e){bP(S,cs,o);}U{dt=cz.fetch(cs).dY();}}D dE(q&S,K&cb,I o=0){bV e=is_enabled();if(gz()){cb.dD(ia);}U if(e){cu(S,cb,o);}U{if(!dt.is_valid()){throw runtime_error(name+":optimization is disabled,but dt is empty,meaning it was never set");}cb.dD(dt);}}Ref<Cell> bn(q&S,I o=0){K cb;dE(S,cb,o);u cb.finalize(eV!=0);}Ref<Cell> cA(q&S,I o=0){K cb;es(S,cb,o);u cb.finalize(eV!=0);}D bp(q&S,Ref<Cell> cell_ref,I o=0,bool gA=false){bV cs=to_cs(std::move(cell_ref));cK(S,cs,o,gA);}D bZ(q&S,Ref<Cell> cell_ref,I o=0,bool gA=false){bV cs=to_cs(std::move(cell_ref));cS(S,cs,o,gA);}D fB(O&cs){ccs=cs;cs.advance(cs.cV());cs.advance_refs(cs.size_refs());}D fh(K&cb){cb.dD(ccs);}};bw et{fY ~et(){}fY O fo(){throw runtime_error("aug V requested but not implemented");}};gY<gZ T_TLB>bw fp{T_TLB add_type;fp(T_TLB add_type):add_type(add_type){}fp():fp(T_TLB{}){}fY ~fp(){}fY O add_values(O&cs1,O&cs2){K cb;(add_type.add_values(cb,cs1,cs2));u cb.hq();}};using namespace block::tlb;z fU tCC;z OutMsg tOM;z AccountBlock tAB;z ImportFees tIF;z InMsg tIM;z EnqueuedMsg tEM;z UInt tU64{64};z ha tSFC;gY<gZ gj,gZ fC>bw cR;gY<gZ gj,gZ fC>bw hV:C<block::gen::hM>{I hb=-1;I n=-1;Ref<cR<gj,fC>> left;Ref<cR<gj,fC>> right;gj eu;fC bR;hV(I m,z TLB&X,z TLB&Y):C("hM",block::gen::hM(m,X,Y)){}D bG(q&S,O&cs,I o=0){hb=cz.check_tag(cs);if(hb==0){bR.cK(S,cs,o+1);eu.cK(S,cs,o+1);}U{I n;add_r1(n,1,cz.m_);left=Ref<cR<gj,fC>>(true,n,cz.X_,cz.Y_);left.dY().bp(S,cs.bM(),o+1);right=Ref<cR<gj,fC>>(true,n,cz.X_,cz.Y_);right.dY().bp(S,cs.bM(),o+1);bR.cK(S,cs,o+1);}}D ct(q&S,K&cb,I o=0){if(hb==0){eu.es(S,cb,o+1);}U{I n;(add_r1(n,1,cz.m_));cb.bm(left.dY().cA(S,o+1));cb.bm(right.dY().cA(S,o+1));}}D bP(q&S,O&cs,I o=0){hb=cz.check_tag(cs);if(hb==0){eu.cS(S,cs,o+1);bV extra_cs=eu.fo();bR.cS(S,extra_cs,o+1,true);}U{I n;add_r1(n,1,cz.m_);left=Ref<cR<gj,fC>>(true,n,cz.X_,cz.Y_);left.dY().bZ(S,cs.bM(),o+1);right=Ref<cR<gj,fC>>(true,n,cz.X_,cz.Y_);right.dY().bZ(S,cs.bM(),o+1);bV left_extra_cs=to_cs(left.dY().node.bR.bn(S));bV right_extra_cs=to_cs(right.dY().node.bR.bn(S));bV extra_cs=bR.add_values(left_extra_cs,right_extra_cs);bR.cS(S,extra_cs,o+1,true);}}D cu(q&S,K&cb,I o=0){if(hb==0){bR.dE(S,cb,o+1);eu.dE(S,cb,o+1);}U{I n;(add_r1(n,1,cz.m_));cb.bm(left.dY().bn(S,o+1));cb.bm(right.dY().bn(S,o+1));bR.dE(S,cb,o+1);}}};gY<gZ gj,gZ fC>bw cR:C<block::gen::HashmapAug>,td::CntObject{Ref<O> label;I n,m,l;hV<gj,fC> node;cR(I n,z TLB&X,z TLB&Y):C("HashmapAug",block::gen::HashmapAug(n,X,Y)),node(n,X,Y){}D bG(q&S,O&cs,I o=0){n=cz.m_;(block::gen::HmLabel{n}.fetch_to(cs,label,l));m=n-l;node.cz.m_=m;node.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){I l,m;(tlb::store_from(cb,HmLabel{cz.m_},label,l));(add_r1(m,l,cz.m_));node.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){(
(n=cz.m_)>= 0
&&block::gen::HmLabel{cz.m_}.fetch_to(cs,label,l)&&add_r1(m,l,cz.m_));node.cz.m_=m;node.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){I l,m;(tlb::store_from(cb,block::gen::HmLabel{cz.m_},label,l)&&add_r1(m,l,cz.m_));node.dE(S,cb,o+1);}};gY<gZ gj,gZ fC>bw ev:C<block::gen::ed>{block::gen::ed::Record_ahme_root r;I hb=-1;cR<gj,fC> root;fC bR;ev(I n,z TLB&X,z TLB&Y):C("ed",block::gen::ed(n,X,Y)),root(n,X,Y){}D bG(q&S,O&cs,I o=0){hb=cz.check_tag(cs);if(hb==block::gen::ed::ahme_empty){(cs.eH(1)== 0);bR.cK(S,cs,o+1);}U{(cz.unpack(cs,r));root.bp(S,r.root,o+1);bR.cK(S,r.bR.dY(),o+1);}}D ct(q&S,K&cb,I o=0){if(hb==ed::ahme_empty){cb.dh(0,1);bR.es(S,cb,o+1);}U{cb.dh(1,1).bm(root.cA(S,o+1));}}D bP(q&S,O&cs,I o=0){hb=cz.check_tag(cs);if(hb==ed::ahme_empty){(cs.eH(1)== 0);bR.cS(S,cs,o+1);}U{(cs.eH(1)== 1&&(r.n=cz.m_)>= 0);bV root_ref=cs.bM();root.bZ(S,root_ref,o+1);bR=root.node.bR;}}D cu(q&S,K&cb,I o=0){if(hb==ed::ahme_empty){cb.dh(0,1);bR.dE(S,cb,o+1);}U{cb.dh(1,1).bm(root.bn(S,o+1));bR.dE(S,cb,o+1);}}};gY<gZ T>bw FullMaybe:C<TLB>{T eu;I hb=-1;bool is_ref;FullMaybe(bool is_ref=false):C("Maybe"),is_ref(is_ref){}D bG(q&S,O&cs,I o=0){hb=cs.eH(1);if(hb){if(is_ref){eu.bp(S,cs.bM(),o+1);}U{eu.cK(S,cs,o+1);}}}fY D ct(q&S,K&cb,I o=0){(cb.store_long_bool(hb,1));if(hb){if(is_ref){cb.bm(eu.cA(S,o+1));}U{eu.es(S,cb,o+1);}}}fY D bP(q&S,O&cs,I o=0){hb=cs.eH(1);if(hb){if(is_ref){eu.bZ(S,cs.bM(),o+1);}U{eu.cS(S,cs,o+1);}}}D cu(q&S,K&cb,I o=0){(cb.store_long_bool(hb,1));if(hb){if(is_ref){cb.bm(eu.bn(S,o+1));}U{eu.dE(S,cb,o+1);}}}};bw gk;bw FullInMsg:C<InMsg>,et{FullInMsg():C("InMsg",InMsg()){}O fo(){K cb;bV cs_copy=dt;(cz.get_import_fees(cb,cs_copy));u cb.hq();}};bw ew:C<fU>,fp<fU>{ew():C("fU"){}};bw FullOutMsg:C<OutMsg>,et{FullOutMsg():C("OutMsg",OutMsg()){}O fo(){K cb;bV cs_copy=dt;(cz.get_export_value(cb,cs_copy));u cb.hq();}};gY<gZ T>bw FullMERKLE_UPDATE:C<TLB>{Ref<T> go,to_proof;FullMERKLE_UPDATE(z T&cz):C("MERKLE_UPDATE"){}D bG(q&S,O&cs,I o=0){(cs.advance(520));go=Ref<T>(true);go.dY().bp(S,cs.bM(),o+1);to_proof=Ref<T>(true);to_proof.dY().bp(S,cs.bM(),o+1);}D ct(q&S,K&cb,I o=0){cb.bm(go.dY().bn(S,o+1));cb.bm(to_proof.dY().bn(S,o+1));eV=0;}D bP(q&S,O&cs,I o=0){go=Ref<T>(true);go.dY().bp(S,cs.bM(),o+1);to_proof=Ref<T>(true);to_proof.dY().bp(S,cs.bM(),o+1);}Ref<Cell> bn(q&S,I o=0){u K::create_merkle_update(
go.dY().bn(S,o+1),to_proof.dY().bn(S,o+1));}};bw FullAccountBlock:C<AccountBlock>,et{FullAccountBlock():C("AccountBlock"){}O fo(){K cb;bV cs_copy=dt;(Aug_ShardAccountBlocks().eval_leaf(cb,cs_copy));u cb.hq();}};bw gp:C<block::gen::hU>{ev<FullAccountBlock,ew> x{256,tAB,tCC};gp():C("hU"){}D bG(q&S,O&cs,I o=0){x.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){x.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){x.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){x.dE(S,cb,o+1);}};bw hN:C<ShardState>,td::CntObject{hN():C("ShardState"){}};bw FullValueFlow:C<block::gen::ValueFlow>{FullValueFlow():C("ValueFlow"){}f long long hb=-1;D bG(q&S,O&cs,I o=0){hb=cs.eH(32);fB(cs);}D ct(q&S,K&cb,I o=0){cb.dh((I)(hb==0xb8e48dfb),1);fh(cb);}D bP(q&S,O&cs,I o=0){hb=cs.fetch_long(1)? 0xb8e48dfb:0x3ebf98b7;fB(cs);}D cu(q&S,K&cb,I o=0){cb.dh(hb,32);fh(cb);}};bw fq:C<ha>,et,fp<ha>{fq():C("ha"){}O fo(){K cb;bV cs_copy=dt;(Aug_ShardFees().eval_leaf(cb,cs_copy));u cb.hq();}};bw FullShardFees:C<block::gen::ShardFees>{ev<fq,fq> x{96,tSFC,tSFC};FullShardFees():C("ShardFees"){}D bG(q&S,O&cs,I o=0){x.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){x.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){x.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){x.dE(S,cb,o+1);}};z block::gen::ShardHashes tSH;bw FullMcBlockExtra:C<block::gen::hp>{block::gen::hp::Record ib;FullShardFees shard_fees;FullMcBlockExtra():C("hp"){}D bG(q&S,O&cs,I o=0){(cs.eH(16)== 0xcca5);(cs.fetch_bool_to(ib.key_block));(tSH.fetch_to(cs,ib.shard_hashes));shard_fees.cK(S,cs,o+1);fB(cs);}D ct(q&S,K&cb,I o=0){cb.dh(ib.key_block,1);tSH.store_from(cb,ib.shard_hashes);shard_fees.es(S,cb,o+1);fh(cb);}D bP(q&S,O&cs,I o=0){(cs.fetch_bool_to(ib.key_block));(tSH.fetch_to(cs,ib.shard_hashes));shard_fees.cS(S,cs,o+1);fB(cs);}D cu(q&S,K&cb,I o=0){cb.dh(0xcca5,16).dh(ib.key_block,1);tSH.store_from(cb,ib.shard_hashes);shard_fees.dE(S,cb,o+1);fh(cb);}};bw gk;bw FullOutMsgDescr:C<OutMsgDescr>{ev<FullOutMsg,ew> x{256,tOM,tCC};FullOutMsgDescr():C("OutMsgDescr"){}D bG(q&S,O&cs,I o=0){x.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){x.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){x.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){x.dE(S,cb,o+1);}};bw gk:C<ImportFees>,fp<ImportFees>{gk():C("ImportFees",tIF),fp(tIF){}};bw FullInMsgDescr:C<InMsgDescr>{ev<FullInMsg,gk> x{256,tIM,tIF};FullInMsgDescr():C("InMsgDescr",InMsgDescr()){}D bG(q&S,O&cs,I o=0){x.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){x.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){x.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){x.dE(S,cb,o+1);}};z block::gen::hp tMBE{};z block::gen::RefT tRMBE{tMBE};z block::gen::Maybe tMRMBE(tRMBE);bw FullBlockExtra:C<block::gen::BlockExtra>{block::gen::BlockExtra::Record ib;FullInMsgDescr hr;FullOutMsgDescr gB;gp gl;FullMaybe<FullMcBlockExtra> custom;FullBlockExtra():C("BlockExtra"),custom(true){}D bG(q&S,O&cs,I o=0){((cs.eH(32)== 0x4a33f6fd));hr.bp(S,cs.bM(),o+1);gB.bp(S,cs.bM(),o+1);gl.bp(S,cs.bM(),o+1);ccs=cs.fetch_subslice(512).dY();custom.cK(S,cs,o+1);}D ct(q&S,K&cb,I o=0){cb.bm(hr.cA(S,o+1)).bm(gB.cA(S,o+1)).bm(gl.cA(S,o+1)).dD(ccs);custom.es(S,cb,o+1);}D bP(q&S,O&cs,I o=0){hr.bZ(S,cs.bM(),o+1);gB.bZ(S,cs.bM(),o+1);gl.bZ(S,cs.bM(),o+1);ccs=cs.fetch_subslice(512).dY();custom.cS(S,cs,o+1);}D cu(q&S,K&cb,I o=0){cb.dh(0x4a33f6fd,32).bm(hr.bn(S,o+1)).bm(gB.bn(S,o+1)).bm(gl.bn(S,o+1)).dD(ccs);custom.dE(S,cb,o+1);}};bw FullBlock:C<block::gen::Block>{I global_id=-1;Ref<Cell> info;FullValueFlow value_flow;FullMERKLE_UPDATE<hN> fZ;FullBlockExtra bR;FullBlock():C("Block"),fZ(hN()){}D bG(q&S,O&cs,I o=0){(cs.advance(32));(cs.fetch_int_to(32,global_id));(cs.fetch_ref_to(info));value_flow.bp(S,cs.bM(),o+1);fZ.bp(S,cs.bM(),o+1);bR.bp(S,cs.bM(),o+1);}D ct(q&S,K&cb,I o=0){cb.dh(global_id,32).bm(info).bm(value_flow.cA(S,o+1)).bm(fZ.cA(S,o+1)).bm(bR.cA(S,o+1));}D bP(q&S,O&cs,I o=0){(
cs.fetch_int_to(32,global_id)&&cs.fetch_ref_to(info));value_flow.bZ(S,cs.bM(),o+1);fZ.bZ(S,cs.bM(),o+1);bR.bZ(S,cs.bM(),o+1);}D cu(q&S,K&cb,I o=0){cb.dh(0x11ef55aa,32).dh(global_id,32).bm(info).bm(value_flow.bn(S,o+1)).bm(fZ.bn(S,o+1)).bm(bR.bn(S,o+1));}};gZ NullStream:public ostream{gZ NullBuffer:public streambuf{public:I overflow(I c){u c;}}m_nb;public:NullStream():ostream(&m_nb){}};
#define cB 0
#define dy 1
#define ga 2
#define gX 4
#define hO 5
#define fV 6
#define SZ_ERROR_WRITE 9
#define SZ_ERROR_FAIL 11
#define RINOK(x){z I _result_=(x);if(_result_!=0)u _result_;}
dN f gq Z;dN f short UInt16;dN f I g;bw gC{y (*Write)(gC* p,z D *eQ,y cV);};
#define MY_offsetof(cz,m)offsetof(cz,m)
#define Z7_container_of(ptr,cz,m)((cz *)(D *)((gq*)(D *)(1 ? (ptr):&((cz *)0)->m)- MY_offsetof(cz,m)))
#define Z7_CONTAINER_FROM_VTBL(ptr,cz,m)Z7_container_of(ptr,cz,m)
#define gb(ptr,cz,m,p)cz *p=Z7_CONTAINER_FROM_VTBL(ptr,cz,m);
#define UNUSED_VAR(x)(D)x;
D *SzAlloc(y cV){u cV==0 ? 0:malloc(cV);}
#define GetUi16(p)(*(z UInt16 *)(z D *)(p))
#define GetUi32(p)(*(z g *)(z D *)(p))
#define SetUi32(p,v){*(g *)(D *)(p)= (v);}
#define UNUSED_VAR(x)(D)x;
dN g dF;dN bw{z Z *eo;g bH,posLimit,streamPos,cm,cn,dG;Z ee,fD,bigHash,fE;g gD;dF *bf,*son;g hashMask,fF;Z *cj;g ch,dZ,fr;g N;y eR;g cI,cW;Z gE,dL,_pad2_[2];I fG;g crc[256];y numRefs;fH dH;}B;
#define fd(p)((g)((p)->streamPos-(p)->bH))
#define gx(p,a,b){(p)->fE=1;(p)->eo=(a);(p)->eR=(b);}
#define ic(p,a)(p)->bH-=(a);(p)->streamPos-=(a);
dN D (*Mf_Init_Func)(D *object);dN g (*hx)(D *object);dN z Z * (*gU)(D *object);dN g * (*Mf_GetMatches_Func)(D *object,g *M);dN D (*Mf_Skip_Func)(D *object,g);bw IMatchFinder2{Mf_Init_Func Init;hx dl;gU bI;Mf_GetMatches_Func fs;Mf_Skip_Func Skip;};
#define gr (1 << 10)
#define kHash3Size (1 << 16)
#define bA (gr)
#define cX (gr+kHash3Size)
#define hW 5
#define ep 10
#define hc (1 << 7)
#define hd (1 << 16)
#define kBlockSizeReserveMin (1 << 24)
#define fI 0
#define hy ((g)0)
#define dT(p)fd(p)
#define dz cX
#define HASH2_CALC hv=GetUi16(bv);
#define HASH3_CALC{g hX=p->crc[bv[0]] ^ bv[1];h2=hX&(gr-1);hv=(hX ^ ((g)bv[2] << 8))&p->hashMask;}
#define HASH4_CALC{g hX=p->crc[bv[0]] ^ bv[1];h2=hX&(gr-1);hX^=((g)bv[2] << 8);h3=hX&(kHash3Size-1);hv=(hX ^ (p->crc[bv[3]] << hW))&p->hashMask;}
#define HASH5_CALC{g hX=p->crc[bv[0]] ^ bv[1];h2=hX&(gr-1);hX^=((g)bv[2] << 8);h3=hX&(kHash3Size-1);hX^=(p->crc[bv[3]] << hW);hv=(hX ^ (p->crc[bv[4]] << ep))&p->hashMask;}
L D LzInWindow_Free(B *p){{free(p->cj);p->cj=0;}}L I LzInWindow_Create2(B *p,g ch){if(ch==0)
u 0;if(!p->cj||p->ch!=ch){LzInWindow_Free(p);p->ch=ch;p->cj=(Z *)SzAlloc(ch);}u(p->cj!=0);}L z Z *gc(D *p){u((B *)p)->eo;}L g gy(D *p){u dT((B *)p);}L D gF(B *p){if(p->ee||p->fG!=cB)
u;g curSize=0xFFFFFFFF-dT(p);if(curSize > p->eR)
curSize=(g)p->eR;p->streamPos+=curSize;p->eR-=curSize;if(p->eR==0)
p->ee=1;}D MatchFinder_MoveBlock(B *p){z y cy=(y)(p->eo-p->cj)- p->dZ;z y keepBefore=(cy&(hc-1))+ p->dZ;p->eo=p->cj+keepBefore;memmove(p->cj,p->cj+(cy&~((y)hc-1)),keepBefore+(y)dT(p));}I MatchFinder_NeedMove(B *p){if(p->fE)
u 0;if(p->ee||p->fG!=cB)
u 0;u((y)(p->cj+p->ch-p->eo)<= p->fr);}L D gV(B *p){p->fF=32;p->fD=1;p->N=4;p->gE=2;p->dL=0;p->bigHash=0;}
#define kCrcPoly 0xEDB88320
D MatchFinder_Construct(B *p){f i;p->eo=0;p->cj=0;p->fE=0;p->bf=0;p->dH=(fH)(int64_t)-1;gV(p);cr(i=0;i<256;i++){g r=(g)i;f j;cr(j=0;j<8;j++)
r=(r >> 1)^ (kCrcPoly&((g)0-(r&1)));p->crc[i]=r;}}
#undef kCrcPoly
L D ef(B *p){free(p->bf);p->bf=0;}D MatchFinder_Free(B *p){ef(p);LzInWindow_Free(p);}L dF* AllocRefs(y hz){z y sizeInBytes=(y)hz * id(dF);if(sizeInBytes / id(dF)!= hz)
u 0;u(dF *)SzAlloc(sizeInBytes);}L g GetBlockSize(B *p,g cI){g ch=(p->dZ+p->fr);if(p->dZ<cI||ch<p->dZ)
u 0;{z g kBlockSizeMax=(g)0-(g)hd;z g gm=kBlockSizeMax-ch;z g reserve=(ch >> (ch<((g)1 << 30)? 1:2))
+ (1 << 12)+ hc+hd;if(ch >= kBlockSizeMax
|| gm<kBlockSizeReserveMin)
u 0;if(reserve >= gm)
ch=kBlockSizeMax;U{ch+=reserve;ch&= ~(g)(hd-1);}}u ch;}L g fQ(B *p,g hs){if(p->N==2)
u(1 << 16)- 1;if(hs!=0)
hs--;hs |= (hs >> 1);hs |= (hs >> 2);hs |= (hs >> 4);hs |= (hs >> 8);if(hs >= (1 << 24)){if(p->N==3)
hs=(1 << 24)- 1;}hs |= (1 << 16)- 1;if(p->N >= 5)
hs |= (256 << ep)- 1;u hs;}L g gd(B *p,g hs){if(p->N==2)
u(1 << 16)- 1;if(hs!=0)
hs--;hs |= (hs >> 1);hs |= (hs >> 2);hs |= (hs >> 4);hs |= (hs >> 8);hs >>= 1;if(hs >= (1 << 24)){if(p->N==3)
hs=(1 << 24)- 1;U
hs >>= 1;}hs |= (1 << 16)- 1;if(p->N >= 5)
hs |= (256 << ep)- 1;u hs;}I MatchFinder_Create(B *p,g cI,g keepAddBufferBefore,g gD,g eq){p->dZ=cI+keepAddBufferBefore+1;eq+=gD;if(eq<p->N)
eq=p->N;p->fr=eq;if(p->fE)
p->ch=0;if(p->fE||LzInWindow_Create2(p,GetBlockSize(p,cI))){y fJ;{g hs;g hsCur;if(p->dL!=0){f hA=p->dL;z f nbMax=(p->N==2 ? 16:(p->N==3 ? 24:32));if(hA > nbMax)
hA=nbMax;if(hA >= 32)
hs=(g)0-1;U
hs=((g)1 << hA)- 1;hs |= (1 << 16)- 1;if(p->N >= 5)
hs |= (256 << ep)- 1;{z g hs2=fQ(p,cI);if(hs > hs2)
hs=hs2;}hsCur=hs;if(p->dH<cI){z g hs2=fQ(p,(g)p->dH);if(hsCur > hs2)
hsCur=hs2;}}U{hs=gd(p,cI);hsCur=hs;if(p->dH<cI){hsCur=gd(p,(g)p->dH);if(hsCur > hs)
hsCur=hs;}}p->hashMask=hsCur;fJ=hs;fJ++;if(fJ<hs)
u 0;{g cW=0;if(p->N > 2&&p->gE<=2)cW+=gr;if(p->N > 3&&p->gE<=3)cW+=kHash3Size;fJ+=cW;p->cW=cW;}}p->gD=gD;{y newSize;y numSons;z g ft=cI+1;p->cI=cI;p->dG=ft;numSons=ft;if(p->fD)
numSons <<= 1;newSize=fJ+numSons;if(numSons<ft||newSize<numSons)
u 0;
#define hB 0xF
newSize=(newSize+hB)&~(y)hB;if(p->bf&&p->numRefs >= newSize)
u 1;ef(p);p->numRefs=newSize;p->bf=AllocRefs(newSize);if(p->bf){p->son=p->bf+fJ;u 1;}}}MatchFinder_Free(p);u 0;}L D gG(B *p){g k;g n=hy-p->bH;if(n==0)
n=(g)(I)-1;k=p->dG-p->cn;if(k<n)
n=k;k=dT(p);{z g ksa=p->fr;g mm=p->gD;if(k > ksa)
k-=ksa;U if(k >= mm){k-=mm;k++;}U{mm=k;if(k!=0)
k=1;}p->cm=mm;}if(k<n)
n=k;p->posLimit=p->bH+n;}D MatchFinder_Init_LowHash(B *p){y i;dF *eI=p->bf;z y eW=p->cW;cr(i=0;i<eW;i++)
eI[i]=fI;}D MatchFinder_Init_HighHash(B *p){y i;dF *eI=p->bf+p->cW;z y eW=(y)p->hashMask+1;cr(i=0;i<eW;i++)
eI[i]=fI;}D MatchFinder_Init_4(B *p){if(!p->fE)
p->eo=p->cj;{p->bH=p->streamPos=1;}p->fG=cB;p->ee=0;}
#define CYC_TO_POS_OFFSET 0
D MatchFinder_Init(D *_p){B *p=(B *)_p;MatchFinder_Init_HighHash(p);MatchFinder_Init_LowHash(p);MatchFinder_Init_4(p);gF(p);p->cn=(p->bH-CYC_TO_POS_OFFSET);gG(p);}
#define eX(i){g v=eI[i];if(v<fK)v=fK;eI[i]=v-fK;}
#define DEFAULT_SaturSub LzFind_SaturSub_32
L
D
LzFind_SaturSub_32(g fK,dF *eI,z dF *lim){do{eX(0)eX(1)eI+=2;eX(0)eX(1)eI+=2;eX(0)eX(1)eI+=2;eX(0)eX(1)eI+=2;}bK(eI!=lim);}D gs(g fK,dF *eI,y eW){
#define eD (1 << 7)
cr(;eW!=0&&((f)(dm)eI&(eD-1))!= 0;eW--){eX(0)
eI++;}{z y k_Align_Mask=(eD / 4-1);dF *lim=eI+(eW&~(y)k_Align_Mask);eW&= k_Align_Mask;if(eI!=lim){DEFAULT_SaturSub(fK,eI,lim);}eI=lim;}cr(;eW!=0;eW--){eX(0)
eI++;}}L D ge(B *p){if(
p->fr==dT(p)){if(MatchFinder_NeedMove(p))
MatchFinder_MoveBlock(p);gF(p);}if(p->bH==hy)
if(dT(p)>= p->N){z g fK=(p->bH-p->cI-1);ic(p,fK)
gs(fK,p->bf,(y)p->hashMask+1+p->cW);{y numSonRefs=p->dG;if(p->fD)
numSonRefs <<= 1;gs(fK,p->son,numSonRefs);}}if(p->cn==p->dG)
p->cn=0;gG(p);}L g * Hc_GetMatchesSpec(y cm,g bc,g bH,z Z *bv,dF *son,y bx,g bS,g fF,g *d,f bU){z Z *lim=bv+cm;son[bx]=bc;do{g dV;if(bc==0)
bN;dV=bH-bc;if(dV >= bS)
bN;{dm diff;bc=son[bx-dV+((dV > bx)? bS:0)];diff=(dm)0-(dm)dV;if(bv[bU]==bv[(dm)bU+diff]){z Z *c=bv;bK(*c==c[diff]){if(++c==lim){d[0]=(g)(lim-bv);d[1]=dV-1;u d+2;}}{z f H=(f)(c-bv);if(bU<H){bU=H;d[0]=(g)H;d[1]=dV-1;d+=2;}}}}}bK(--fF);u d;}g * GetMatchesSpec1(g cm,g bc,g bH,z Z *bv,dF *son,y bx,g bS,g fF,g *d,g bU){dF *ptr0=son+((y)bx << 1)+ 1;dF *ptr1=son+((y)bx << 1);f len0=0,len1=0;g gH;gH=(g)(bH-bS);if((g)bH<=bS)
gH=0;if(gH<bc)
do{z g dV=bH-bc;{dF *hY=son+((y)(bx-dV+((dV > bx)? bS:0))<< 1);z Z *pb=bv-dV;f H=(len0<len1 ? len0:len1);z g pair0=hY[0];if(pb[H]==bv[H]){if(++H!=cm&&pb[H]==bv[H])
bK(++H!=cm)
if(pb[H]!=bv[H])
bN;if(bU<H){bU=(g)H;*d++=(g)H;*d++=dV-1;if(H==cm){*ptr1=pair0;*ptr0=hY[1];u d;}}}if(pb[H]<bv[H]){*ptr1=bc;bc=hY[1];ptr1=hY+1;len1=H;}U{*ptr0=bc;bc=hY[0];ptr0=hY;len0=H;}}}bK(--fF&&gH<bc);*ptr0=*ptr1=fI;u d;}L D fL(g cm,g bc,g bH,z Z *bv,dF *son,y bx,g bS,g fF){dF *ptr0=son+((y)bx << 1)+ 1;dF *ptr1=son+((y)bx << 1);f len0=0,len1=0;g gH;gH=(g)(bH-bS);if((g)bH<=bS)
gH=0;if(
gH<bc)
do{z g dV=bH-bc;{dF *hY=son+((y)(bx-dV+((dV > bx)? bS:0))<< 1);z Z *pb=bv-dV;f H=(len0<len1 ? len0:len1);if(pb[H]==bv[H]){bK(++H!=cm)
if(pb[H]!=bv[H])
bN;{if(H==cm){*ptr1=hY[0];*ptr0=hY[1];u;}}}if(pb[H]<bv[H]){*ptr1=bc;bc=hY[1];ptr1=hY+1;len1=H;}U{*ptr0=bc;bc=hY[0];ptr0=hY;len0=H;}}}bK(--fF&&gH<bc);*ptr0=*ptr1=fI;u;}
#define ea p->cn++;p->eo++;{z g pos1=p->bH+1;p->bH=pos1;if(pos1==p->posLimit)ge(p);}
#define ej ea u M;
L D hC(B *p){ea
}
#define hD(minLen,ret_op)g hv;z Z *bv;g bc;g cm=p->cm;if(cm<minLen){hC(p);ret_op;}bv=p->eo;
#define de(minLen)hD(minLen,u M)
#define ie(minLen)do{hD(minLen,eY)
#define MF_PARAMS(p)cm,bc,p->bH,p->eo,p->son,p->cn,p->dG,p->fF
#define ig fL(MF_PARAMS(p));ea }bK(--hz);
#define gf(_maxLen_,func)M=func(MF_PARAMS(p),M,(g)_maxLen_);ej
#define dR(_maxLen_)gf(_maxLen_,GetMatchesSpec1)
#define gI(_maxLen_)gf(_maxLen_,Hc_GetMatchesSpec)
#define fu{z dm diff=(dm)0-(dm)d2;z Z *c=bv+bU;z Z *lim=bv+cm;cr(;c!=lim;c++)if(*(c+diff)!= *c)bN;bU=(f)(c-bv);}
L g* ih(D *_p,g *M){B *p=(B *)_p;de(2)
HASH2_CALC
bc=p->bf[hv];p->bf[hv]=p->bH;dR(1)
}
#define SET_mmm mmm=p->dG;if(bH<mmm)mmm=bH;
L g* ii(D *_p,g *M){B *p=(B *)_p;g mmm;g h2,d2,bH;f bU;g *bf;de(3)
HASH3_CALC
bf=p->bf;bH=p->bH;d2=bH-bf[h2];bc=(bf+bA)[hv];bf[h2]=bH;(bf+bA)[hv]=bH;SET_mmm
bU=2;if(d2<mmm&&*(bv-d2)== *bv){fu
M[0]=(g)bU;M[1]=d2-1;M+=2;if(bU==cm){fL(MF_PARAMS(p));ej
}}dR(bU)
}L g* ij(D *_p,g *M){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bH;f bU;g *bf;de(4)
HASH4_CALC
bf=p->bf;bH=p->bH;d2=bH-bf [h2];d3=bH-(bf+bA)[h3];bc=(bf+cX)[hv];bf [h2]=bH;(bf+bA)[h3]=bH;(bf+cX)[hv]=bH;SET_mmm
bU=3;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){M[0]=2;M[1]=d2-1;M+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;M[1]=d3-1;M+=2;}U
bN;}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;M[1]=d3-1;M+=2;}U
bN;fu
M[-2]=(g)bU;if(bU==cm){fL(MF_PARAMS(p));ej
}bN;}dR(bU)
}L g* ik(D *_p,g *M){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bH;f bU;g *bf;de(5)
HASH5_CALC
bf=p->bf;bH=p->bH;d2=bH-bf [h2];d3=bH-(bf+bA)[h3];bc=(bf+dz)[hv];bf [h2]=bH;(bf+bA)[h3]=bH;(bf+dz)[hv]=bH;SET_mmm
bU=4;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){M[0]=2;M[1]=d2-1;M+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){M[1]=d3-1;M+=2;d2=d3;}U
bN;}U if(d3<mmm&&*(bv-d3)== *bv){M[1]=d3-1;M+=2;d2=d3;}U
bN;M[-2]=3;if(*(bv-d2+3)!= bv[3])
bN;fu
M[-2]=(g)bU;if(bU==cm){fL(MF_PARAMS(p));ej
}bN;}dR(bU)
}L g* il(D *_p,g *M){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bH;f bU;g *bf;de(4)
HASH4_CALC
bf=p->bf;bH=p->bH;d2=bH-bf [h2];d3=bH-(bf+bA)[h3];bc=(bf+cX)[hv];bf [h2]=bH;(bf+bA)[h3]=bH;(bf+cX)[hv]=bH;SET_mmm
bU=3;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){M[0]=2;M[1]=d2-1;M+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;M[1]=d3-1;M+=2;}U
bN;}U if(d3<mmm&&*(bv-d3)== *bv){d2=d3;M[1]=d3-1;M+=2;}U
bN;fu
M[-2]=(g)bU;if(bU==cm){p->son[p->cn]=bc;ej
}bN;}gI(bU)
}L g * im(D *_p,g *M){B *p=(B *)_p;g mmm;g h2,h3,d2,d3,bH;f bU;g *bf;de(5)
HASH5_CALC
bf=p->bf;bH=p->bH;d2=bH-bf [h2];d3=bH-(bf+bA)[h3];bc=(bf+dz)[hv];bf [h2]=bH;(bf+bA)[h3]=bH;(bf+dz)[hv]=bH;SET_mmm
bU=4;cr(;;){if(d2<mmm&&*(bv-d2)== *bv){M[0]=2;M[1]=d2-1;M+=2;if(*(bv-d2+2)== bv[2]){}U if(d3<mmm&&*(bv-d3)== *bv){M[1]=d3-1;M+=2;d2=d3;}U
bN;}U if(d3<mmm&&*(bv-d3)== *bv){M[1]=d3-1;M+=2;d2=d3;}U
bN;M[-2]=3;if(*(bv-d2+3)!= bv[3])
bN;fu
M[-2]=(g)bU;if(bU==cm){p->son[p->cn]=bc;ej
}bN;}gI(bU)
}L D Bt2_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ie(2){HASH2_CALC
bc=p->bf[hv];p->bf[hv]=p->bH;}ig
}L D Bt3_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ie(3){g h2;g *bf;HASH3_CALC
bf=p->bf;bc=(bf+bA)[hv];bf[h2]=(bf+bA)[hv]=p->bH;}ig
}L D Bt4_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ie(4){g h2,h3;g *bf;HASH4_CALC
bf=p->bf;bc=(bf+cX)[hv];bf [h2]=(bf+bA)[h3]=(bf+cX)[hv]=p->bH;}ig
}L D Bt5_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;ie(5){g h2,h3;g *bf;HASH5_CALC
bf=p->bf;bc=(bf+dz)[hv];bf [h2]=(bf+bA)[h3]=(bf+dz)[hv]=p->bH;}ig
}
#define HC_SKIP_HEADER(minLen)do{if(p->cm<minLen){hC(p);hz--;eY;}{z Z *bv;g *bf;g *son;g bH=p->bH;g num2=hz;{z g gm=p->posLimit-bH;if(num2 > gm)num2=gm;}hz-=num2;{z g cycPos=p->cn;son=p->son+cycPos;p->cn=cycPos+num2;}bv=p->eo;bf=p->bf;do{g bc;g hv;
#define HC_SKIP_FOOTER bv++;bH++;*son++=bc;}bK(--num2);p->eo=bv;p->bH=bH;if(bH==p->posLimit)ge(p);}}bK(hz);
L D Hc4_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;HC_SKIP_HEADER(4)
g h2,h3;HASH4_CALC
bc=(bf+cX)[hv];bf [h2]=(bf+bA)[h3]=(bf+cX)[hv]=bH;HC_SKIP_FOOTER
}L D Hc5_MatchFinder_Skip(D *_p,g hz){B *p=(B *)_p;HC_SKIP_HEADER(5)
g h2,h3;HASH5_CALC
bc=(bf+dz)[hv];bf [h2]=(bf+bA)[h3]=(bf+dz)[hv]=bH;HC_SKIP_FOOTER
}D MatchFinder_CreateVTable(B *p,IMatchFinder2 *eJ){eJ->Init=MatchFinder_Init;eJ->dl=gy;eJ->bI=gc;if(!p->fD){if(p->N<=4){eJ->fs=il;eJ->Skip=Hc4_MatchFinder_Skip;}U{eJ->fs=im;eJ->Skip=Hc5_MatchFinder_Skip;}}U if(p->N==2){eJ->fs=ih;eJ->Skip=Bt2_MatchFinder_Skip;}U if(p->N==3){eJ->fs=ii;eJ->Skip=Bt3_MatchFinder_Skip;}U if(p->N==4){eJ->fs=ij;eJ->Skip=Bt4_MatchFinder_Skip;}U{eJ->fs=ik;eJ->Skip=Bt5_MatchFinder_Skip;}}
#undef ea
#undef ej
dN UInt16 G;
#define dn 5
bw CLzmaProps{Z lc;Z lp;Z pb;Z _pad_;g fv;};
#define bt 20
bw ex{CLzmaProps prop;G *A;G *probs_1664;Z *dic;y cC;y bC;z Z *eQ;g Q;g fw;g bh;g co;g ba[4];g E;g cD;g gt;f bW;Z eg[bt];};
#define LzmaDec_CONSTRUCT(p){(p)->dic=0;(p)->A=0;}
dN enum{LZMA_FINISH_ANY,}ELzmaFinishMode;dN enum{fx,gW,fR,cL,fe
}ELzmaStatus;
#define kTopValue ((g)1 << 24)
#define bl 11
#define bX (1 << bl)
#define RC_INIT_SIZE 5
#define dS 5
#define NORMALIZE if(Q<kTopValue){Q <<= 8;fw=(fw << 8)|(*eQ++);}
#define eZ(p)he=*(p);NORMALIZE;bound=(Q >> bl)* (g)he;if(fw<bound)
#define fa(p)Q=bound;*(p)= (G)(he+((bX-he)>> dS));
#define fc(p)Q-=bound;fw-=bound;*(p)= (G)(he-(he >> dS));
#define GET_BIT2(p,i,A0,A1)eZ(p){fa(p)i=(i+i);A0;}U{fc(p)i=(i+i)+ 1;A1;}
#define cc(A,i){GET_BIT2(A+i,i,;,;);}
#define REV_BIT(p,i,A0,A1)eZ(p+i){fa(p+i)A0;}U{fc(p+i)A1;}
#define REV_BIT_VAR(p,i,m)REV_BIT(p,i,i+=m;m+=m,m+=m;i+=m;)
#define REV_BIT_CONST(p,i,m)REV_BIT(p,i,i+=m;,i+=m * 2;)
#define REV_BIT_LAST(p,i,m)REV_BIT(p,i,i-=m ,;)
#define TREE_DECODE(A,cv,i){i=1;do{cc(A,i);}bK(i<cv);i-=cv;}
#define TREE_6_DECODE(A,i){i=1;cc(A,i)cc(A,i)cc(A,i)cc(A,i)cc(A,i)cc(A,i)i-=0x40;}
#define cM cc(F,fi)
#define cE bu+=bu;cw=dO;dO&= bu;probLit=F+(dO+cw+fi);GET_BIT2(probLit,fi,dO^=cw;,;)
#define hf if(Q<kTopValue){if(eQ >= fM)u hg;Q <<= 8;fw=(fw << 8)|(*eQ++);}
#define ek(p)he=*(p);hf bound=(Q >> bl)* (g)he;if(fw<bound)
#define cG Q=bound;
#define cH Q-=bound;fw-=bound;
#define GET_BIT2_(p,i,A0,A1)ek(p){cG i=(i+i);A0;}U{cH i=(i+i)+ 1;A1;}
#define GET_BIT_(p,i)GET_BIT2_(p,i,;,;)
#define TREE_DECODE_(A,cv,i){i=1;do{GET_BIT_(A+i,i)}bK(i<cv);i-=cv;}
#define REV_BIT_(p,i,m)ek(p+i){cG i+=m;m+=m;}U{cH m+=m;i+=m;}
#define hP 4
#define kNumPosStatesMax (1 << hP)
#define bO 3
#define R (1 << bO)
#define dq 8
#define df (1 << dq)
#define LenLow 0
#define LenHigh (LenLow+2 * (kNumPosStatesMax << bO))
#define kNumLenProbs (LenHigh+df)
#define LenChoice LenLow
#define LenChoice2 (LenLow+(1 << bO))
#define bi 12
#define kNumStates2 16
#define dW 7
#define dA 4
#define cF 14
#define bd (1 << (cF >> 1))
#define bq 6
#define W 4
#define ck 4
#define dU (1 << ck)
#define kMatchMinLen 2
#define cq (kMatchMinLen+R * 2+df)
#define cJ (1 << 9)
#define fS (cJ-1)
#define kStartOffset 1664
#define GET_PROBS p->probs_1664
#define SpecPos (-kStartOffset)
#define IsRep0Long (SpecPos+bd)
#define RepLenCoder (IsRep0Long+(kNumStates2 << hP))
#define LenCoder (RepLenCoder+kNumLenProbs)
#define IsMatch (LenCoder+kNumLenProbs)
#define Align (IsMatch+(kNumStates2 << hP))
#define IsRep (Align+dU)
#define IsRepG0 (IsRep+bi)
#define IsRepG1 (IsRepG0+bi)
#define IsRepG2 (IsRepG1+bi)
#define PosSlot (IsRepG2+bi)
#define Literal (PosSlot+(W << bq))
#define NUM_BASE_PROBS (Literal+kStartOffset)
#define LZMA_LIT_SIZE 0x300
#define gJ(p)(NUM_BASE_PROBS+((g)LZMA_LIT_SIZE << ((p)->lc+(p)->lp)))
#define CALC_POS_STATE(bh,fj)(((bh)&(fj))<< 4)
#define eK (P+E)
#define gK (P)
#define LZMA_DIC_MIN (1 << 12)
L
I LzmaDec_DecodeReal_3(ex *p,y cv,z Z *fM){G *A=GET_PROBS;f E=(f)p->E;g rep0=p->ba[0],rep1=p->ba[1],rep2=p->ba[2],rep3=p->ba[3];f fj=((f)1 << (p->prop.pb))- 1;f lc=p->prop.lc;f lpMask=((f)0x100 << p->prop.lp)- ((f)0x100 >> lc);Z *dic=p->dic;y cC=p->cC;y bC=p->bC;g bh=p->bh;g co=p->co;f H=0;z Z *eQ=p->eQ;g Q=p->Q;g fw=p->fw;do{G *F;g bound;f he;f P=CALC_POS_STATE(bh,fj);F=A+IsMatch+eK;eZ(F){f fi;fa(F)
F=A+Literal;if(bh!=0||co!=0)
F+=(g)3 * ((((bh << 8)+ dic[(bC==0 ? cC:bC)- 1])&lpMask)<< lc);bh++;if(E<dW){E-=(E<4)? E:3;fi=1;cM
cM
cM
cM
cM
cM
cM
cM
}U{f bu=dic[bC-rep0+(bC<rep0 ? cC:0)];f dO=0x100;E-=(E<10)? 3:6;fi=1;{f cw;G *probLit;cE
cE
cE
cE
cE
cE
cE
cE
}}dic[bC++]=(Z)fi;eY;}{fc(F)
F=A+IsRep+E;eZ(F){fa(F)
E+=bi;F=A+LenCoder;}U{fc(F)
F=A+IsRepG0+E;eZ(F){fa(F)
F=A+IsRep0Long+eK;eZ(F){fa(F)
dic[bC]=dic[bC-rep0+(bC<rep0 ? cC:0)];bC++;bh++;E=E<dW ? 9:11;eY;}fc(F)
}U{g cg;fc(F)
F=A+IsRepG1+E;eZ(F){fa(F)
cg=rep1;}U{fc(F)
F=A+IsRepG2+E;eZ(F){fa(F)
cg=rep2;}U{fc(F)
cg=rep3;rep3=rep2;}rep2=rep1;}rep1=rep0;rep0=cg;}E=E<dW ? 8:11;F=A+RepLenCoder;}{G *cl=F+LenChoice;eZ(cl){fa(cl)
cl=F+LenLow+gK;H=1;cc(cl,H)
cc(cl,H)
cc(cl,H)
H-=8;}U{fc(cl)
cl=F+LenChoice2;eZ(cl){fa(cl)
cl=F+LenLow+gK+(1 << bO);H=1;cc(cl,H)
cc(cl,H)
cc(cl,H)
}U{fc(cl)
cl=F+LenHigh;TREE_DECODE(cl,(1 << dq),H)
H+=R * 2;}}}if(E >= bi){g cg;F=A+PosSlot +
((H<W ? H:W-1)<< bq);TREE_6_DECODE(F,cg)
if(cg >= dA){f cY=(f)cg;f cQ=(f)(((cg >> 1)- 1));cg=(2 | (cg&1));if(cY<cF){cg <<= cQ;F=A+SpecPos;{g m=1;cg++;do{REV_BIT_VAR(F,cg,m)
}bK(--cQ);cg-=m;}}U{cQ-=ck;do{NORMALIZE
Q >>= 1;{g t;fw-=Q;t=(0-((g)fw >> 31));cg=(cg << 1)+ (t+1);fw+=Q&t;}}bK(--cQ);F=A+Align;cg <<= ck;{f i=1;REV_BIT_CONST(F,i,1)
REV_BIT_CONST(F,i,2)
REV_BIT_CONST(F,i,4)
REV_BIT_LAST (F,i,8)
cg |= i;}if(cg==(g)0xFFFFFFFF){H=cq;E-=bi;bN;}}}rep3=rep2;rep2=rep1;rep1=rep0;rep0=cg+1;E=(E<bi+dW)? dW:dW+3;if(cg >= (co==0 ? bh:co)){H+=cJ+kMatchMinLen;bN;}}H+=kMatchMinLen;{y gm;f curLen;y bH;if((gm=cv-bC)== 0){bN;}curLen=((gm<H)? (f)gm:H);bH=bC-rep0+(bC<rep0 ? cC:0);bh+=(g)curLen;H-=curLen;if(curLen<=cC-bH){Z *gu=dic+bC;dm hh=(dm)bH-(dm)bC;z Z *lim=gu+curLen;bC+=(y)curLen;do
*(gu)= (Z)*(gu+hh);bK(++gu!=lim);}U{do{dic[bC++]=dic[bH];if(++bH==cC)
bH=0;}bK(--curLen!=0);}}}}bK(bC<cv&&eQ<fM);NORMALIZE
p->eQ=eQ;p->Q=Q;p->fw=fw;p->cD=(g)H;p->bC=bC;p->bh=bh;p->ba[0]=rep0;p->ba[1]=rep1;p->ba[2]=rep2;p->ba[3]=rep3;p->E=(g)E;if(H >= cJ)
u dy;u cB;}L D LzmaDec_WriteRem(ex *p,y cv){f H=(f)p->cD;if(H==0)
u;{y bC=p->bC;Z *dic;y cC;y rep0;{y gm=cv-bC;if(gm<H){H=(f)(gm);if(H==0)
u;}}if(p->co==0&&p->prop.fv-p->bh<=H)
p->co=p->prop.fv;p->bh+=(g)H;p->cD-=(g)H;dic=p->dic;rep0=p->ba[0];cC=p->cC;do{dic[bC]=dic[bC-rep0+(bC<rep0 ? cC:0)];bC++;}bK(--H);p->bC=bC;}}
#define kRange0 0xFFFFFFFF
#define kBound0 ((kRange0 >> bl)<< (bl-1))
#define kBadRepCode (kBound0+(((kRange0-kBound0)>> bl)<< (bl-1)))
L I hE(ex *p,y cv,z Z *fM){if(p->co==0){g gm=p->prop.fv-p->bh;if(cv-p->bC > gm)
cv=p->bC+gm;}{I du=LzmaDec_DecodeReal_3(p,cv,fM);if(p->co==0&&p->bh >= p->prop.fv)
p->co=p->prop.fv;u du;}}dN enum{hg,DUMMY_LIT,DUMMY_MATCH,DUMMY_REP
}ELzmaDummy;
#define eE(hi)((hi)== DUMMY_MATCH)
L ELzmaDummy LzmaDec_TryDummy(z ex *p,z Z *eQ,z Z **bufOut){g Q=p->Q;g fw=p->fw;z Z *fM=*bufOut;z G *A=GET_PROBS;f E=(f)p->E;ELzmaDummy du;cr(;;){z G *F;g bound;f he;f P=CALC_POS_STATE(p->bh,((f)1 << p->prop.pb)- 1);F=A+IsMatch+eK;ek(F){cG
F=A+Literal;if(p->co!=0||p->bh!=0)
F+=((g)LZMA_LIT_SIZE *
((((p->bh)&(((f)1 << (p->prop.lp))- 1))<< p->prop.lc)+
((f)p->dic[(p->bC==0 ? p->cC:p->bC)- 1] >> (8-p->prop.lc))));if(E<dW){f fi=1;do{GET_BIT_(F+fi,fi)}bK(fi<0x100);}U{f bu=p->dic[p->bC-p->ba[0] +
(p->bC<p->ba[0] ? p->cC:0)];f dO=0x100;f fi=1;do{f cw;z G *probLit;bu+=bu;cw=dO;dO&= bu;probLit=F+(dO+cw+fi);GET_BIT2_(probLit,fi,dO^=cw;,;)
}bK(fi<0x100);}du=DUMMY_LIT;}U{f H;cH
F=A+IsRep+E;ek(F){cG
E=0;F=A+LenCoder;du=DUMMY_MATCH;}U{cH
du=DUMMY_REP;F=A+IsRepG0+E;ek(F){cG
F=A+IsRep0Long+eK;ek(F){cG
bN;}U{cH
}}U{cH
F=A+IsRepG1+E;ek(F){cG
}U{cH
F=A+IsRepG2+E;ek(F){cG
}U{cH
}}}E=bi;F=A+RepLenCoder;}{f cv,cy;z G *cl=F+LenChoice;ek(cl){cG
cl=F+LenLow+gK;cy=0;cv=1 << bO;}U{cH
cl=F+LenChoice2;ek(cl){cG
cl=F+LenLow+gK+(1 << bO);cy=R;cv=1 << bO;}U{cH
cl=F+LenHigh;cy=R * 2;cv=1 << dq;}}TREE_DECODE_(cl,cv,H)
H+=cy;}if(E<4){f cY;F=A+PosSlot +
((H<W-1 ? H:W-1)<<
bq);TREE_DECODE_(F,1 << bq,cY)
if(cY >= dA){f cQ=((cY >> 1)- 1);if(cY<cF){F=A+SpecPos+((2 | (cY&1))<< cQ);}U{cQ-=ck;do{hf
Q >>= 1;fw-=Q&(((fw-Q)>> 31)- 1);}bK(--cQ);F=A+Align;cQ=ck;}{f i=1;f m=1;do{REV_BIT_(F,i,m)
}bK(--cQ);}}}}bN;}hf
*bufOut=eQ;u du;}D LzmaDec_InitDicAndState(ex *p,I initDic,I initState){p->cD=cq+1;p->bW=0;if(initDic){p->bh=0;p->co=0;p->cD=cq+2;}if(initState)
p->cD=cq+2;}D LzmaDec_Init(ex *p){p->bC=0;LzmaDec_InitDicAndState(p,1,1);}
#define dj *fk=fR;u dy;
I LzmaDec_DecodeToDic(ex *p,y dicLimit,z Z *hh,y *cN,ELzmaFinishMode finishMode,ELzmaStatus *fk){y er=*cN;(*cN)= 0;*fk=fx;if(p->cD > cq){if(p->cD > cq+2)
u p->cD==fS ? SZ_ERROR_FAIL:dy;cr(;er > 0&&p->bW<RC_INIT_SIZE;(*cN)++,er--)
p->eg[p->bW++]=*hh++;if(p->bW!=0&&p->eg[0]!=0)
u dy;if(p->bW<RC_INIT_SIZE){*fk=cL;u cB;}p->fw=((g)p->eg[1] << 24)
| ((g)p->eg[2] << 16)
| ((g)p->eg[3] << 8)
| ((g)p->eg[4]);if(p->co==0
&&p->bh==0
&&p->fw >= kBadRepCode)
u dy;p->Q=0xFFFFFFFF;p->bW=0;if(p->cD > cq+1){y gt=gJ(&p->prop);y i;G *A=p->A;cr(i=0;i<gt;i++)
A[i]=bX >> 1;p->ba[0]=p->ba[1]=p->ba[2]=p->ba[3]=1;p->E=0;}p->cD=0;}cr(;;){if(p->cD==cq){if(p->fw!=0)
u dy;*fk=gW;u cB;}LzmaDec_WriteRem(p,dicLimit);{I ey=0;if(p->bC >= dicLimit){if(p->cD==0&&p->fw==0){*fk=fe;u cB;}if(finishMode==LZMA_FINISH_ANY){*fk=fR;u cB;}if(p->cD!=0){dj
}ey=1;}if(p->bW==0){z Z *fM;I bF=-1;if(er<bt||ey){z Z *bufOut=hh+er;ELzmaDummy hi=LzmaDec_TryDummy(p,hh,&bufOut);if(hi==hg){y i;if(er >= bt)
bN;(*cN)+= er;p->bW=(f)er;cr(i=0;i<er;i++)
p->eg[i]=hh[i];*fk=cL;u cB;}bF=(I)(bufOut-hh);if((f)bF > bt)
bN;if(ey&&!eE(hi)){f i;(*cN)+= (f)bF;p->bW=(f)bF;cr(i=0;i<(f)bF;i++)
p->eg[i]=hh[i];dj
}fM=hh;}U
fM=hh+er-bt;p->eQ=hh;{I du=hE(p,dicLimit,fM);y bT=(y)(p->eQ-hh);if(bF<0){if(bT > er)
bN;}U if((f)bF!=bT)
bN;hh+=bT;er-=bT;(*cN)+= bT;if(du!=cB){p->cD=cJ;u dy;}}eY;}{f gm=p->bW;f ahead=0;I bF=-1;bK(gm<bt&&ahead<er)
p->eg[gm++]=hh[ahead++];if(gm<bt||ey){z Z *bufOut=p->eg+gm;ELzmaDummy hi=LzmaDec_TryDummy(p,p->eg,&bufOut);if(hi==hg){if(gm >= bt)
bN;p->bW=gm;(*cN)+= (y)ahead;*fk=cL;u cB;}bF=(I)(bufOut-p->eg);if((f)bF<p->bW)
bN;if(ey&&!eE(hi)){(*cN)+= (f)bF-p->bW;p->bW=(f)bF;dj
}}p->eQ=p->eg;{I du=hE(p,dicLimit,p->eQ);y bT=(y)(p->eQ-p->eg);gm=p->bW;if(bF<0){if(bT > bt)
bN;if(bT<gm)
bN;}U if((f)bF!=bT)
bN;bT-=gm;hh+=bT;er-=bT;(*cN)+= bT;p->bW=0;if(du!=cB){p->cD=cJ;u dy;}}}}}p->cD=fS;u SZ_ERROR_FAIL;}D LzmaDec_FreeProbs(ex *p){free(p->A);p->A=0;}I LzmaProps_Decode(CLzmaProps *p,z Z *V,f cV){g fv;Z d;if(cV<dn)
u gX;U
fv=V[1] | ((g)V[2] << 8)|((g)V[3] << 16)|((g)V[4] << 24);if(fv<LZMA_DIC_MIN)
fv=LZMA_DIC_MIN;p->fv=fv;d=V[0];if(d >= (9 * 5 * 5))
u gX;p->lc=(Z)(d % 9);d /= 9;p->pb=(Z)(d / 5);p->lp=(Z)(d % 5);u cB;}L I LzmaDec_AllocateProbs2(ex *p,z CLzmaProps *propNew){g gt=gJ(propNew);if(!p->A||gt!=p->gt){LzmaDec_FreeProbs(p);p->A=(G *)SzAlloc(gt * id(G));if(!p->A)
u ga;p->probs_1664=p->A+1664;p->gt=gt;}u cB;}I LzmaDec_AllocateProbs(ex *p,z Z *bL,f gn){CLzmaProps propNew;RINOK(LzmaProps_Decode(&propNew,bL,gn))
RINOK(LzmaDec_AllocateProbs2(p,&propNew))
p->prop=propNew;u cB;}I LzmaDecode(Z *gu,y *eS,z Z *hh,y *cN,z Z *propData,f propSize,ELzmaFinishMode finishMode,ELzmaStatus *fk){ex p;I du;y outSize=*eS,er=*cN;*eS=*cN=0;*fk=fx;if(er<RC_INIT_SIZE)
u fV;LzmaDec_CONSTRUCT(&p)
RINOK(LzmaDec_AllocateProbs(&p,propData,propSize))
p.dic=gu;p.cC=outSize;LzmaDec_Init(&p);*cN=er;du=LzmaDec_DecodeToDic(&p,outSize,hh,cN,finishMode,fk);*eS=p.bC;if(du==cB&&*fk==cL)
du=fV;LzmaDec_FreeProbs(&p);u du;}bw dX{I ff;g bj;I lc,lp,pb,algo,fb,fD,N;f dL;g mc;f cU;I eF;fH reduceSize,affinity;};bw bk;
#define hF ((g)15 << 28)
#define kTopValue ((g)1 << 24)
#define bl 11
#define bX (1 << bl)
#define dS 5
#define bY (bX >> 1)
#define by 4
#define eL 4
#define REP_LEN_COUNT 64
D LzmaEncProps_Init(dX *p){p->ff=5;p->bj=p->mc=0;p->reduceSize=(fH)(int64_t)-1;p->lc=p->lp=p->pb=p->algo=p->fb=p->fD=p->N=p->eF=-1;p->dL=0;p->cU=0;p->affinity=0;}D LzmaEncProps_Normalize(dX *p){I ff=p->ff;if(ff<0)ff=5;p->ff=ff;if(p->bj==0)
p->bj=(f)ff<=4 ?
(g)1 << (ff * 2+16):(f)ff<=id(y)/ 2+4 ?
(g)1 << (ff+20):(g)1 << (id(y)/ 2+24);if(p->bj > p->reduceSize){g v=(g)p->reduceSize;z g kReduceMin=((g)1 << 12);if(v<kReduceMin)
v=kReduceMin;if(p->bj > v)
p->bj=v;}if(p->lc<0)p->lc=3;if(p->lp<0)p->lp=0;if(p->pb<0)p->pb=2;if(p->algo<0)p->algo=(f)ff<5 ? 0:1;if(p->fb<0)p->fb=(f)ff<7 ? 32:64;if(p->fD<0)p->fD=(p->algo==0 ? 0:1);if(p->N<0)p->N=(p->fD ? 4:5);if(p->mc==0)p->mc=(16+((f)p->fb >> 1))>> (p->fD ? 0:1);if(p->eF<0)
p->eF=1;}
#define gL (11+id(y)/ 8 * 3)
#define kDicLogSizeMaxCompress ((gL-1)* 2+7)
L D LzmaEnc_FastPosInit(Z *el){f gg;el[0]=0;el[1]=1;el+=2;cr(gg=2;gg<gL * 2;gg++){y k=((y)1 << ((gg >> 1)- 1));y j;cr(j=0;j<k;j++)
el[j]=(Z)gg;el+=k;}}
#define BSR2_RET(bH,du){f zz=(bH<(1 << (gL+6)))? 6:6+gL-1;du=p->el[bH >> zz]+(zz * 2);}
#define GetPosSlot1(bH)p->el[bH]
#define GetPosSlot2(bH,du){BSR2_RET(bH,du);}
#define GetPosSlot(bH,du){if(bH<bd)du=p->el[bH&(bd-1)];U BSR2_RET(bH,du);}
#define bb 4
dN UInt16 CState;dN UInt16 CExtra;bw ez{g J;CState E,bR;g H,bo,ba[bb];};
#define hj (1 << 11)
#define kPackReserve (hj * 8)
#define W 4
#define bq 6
#define kDicLogSizeMax 32
#define kDistTableSizeMax (kDicLogSizeMax * 2)
#define ck 4
#define dU (1 << ck)
#define kAlignMask (dU-1)
#define dA 4
#define cF 14
#define bd (1 << (cF >> 1))
#define LZMA_PB_MAX 4
#define LZMA_LC_MAX 8
#define LZMA_LP_MAX 4
#define bJ (1 << LZMA_PB_MAX)
#define bO 3
#define R (1 << bO)
#define dq 8
#define df (1 << dq)
#define hG (R * 2+df)
#define dP 2
#define bB (dP+hG-1)
#define bi 12
bw CLenEnc{G low[bJ << (bO+1)],high[df];};bw CLenPriceEnc{f tableSize;g eM[bJ][hG];};
#define dB(p,P,H)((p)->eM[P][(y)(H)- dP])
bw dr{g Q;f cache;fH low,cacheSize;Z *eQ,*bufLim,*cj;gC* dM;fH bT;I du;};bw CSaveState{G *dg;f E;g ba[bb];G cT[1 << ck],isRep[bi],isRepG0[bi],isRepG1[bi],isRepG2[bi],fy[bi][bJ],gv[bi][bJ],eT[W][1 << bq],in[bd];CLenEnc lenProbs,gM;};bw bk{D *bz;IMatchFinder2 ci;f optCur,optEnd,fN,bg;g bE;f E,ce,cf;g ba[bb];f lpMask,fj;G *dg;dr rc;g eA;f lc,lp,pb,lclp;I fastMode,cU,finished,needInit;fH nowPos64;f hk;I fl;f distTableSize;g bj;I fG;B matchFinderBase;g be[bX >> by],eB[bB * 2+2];g alignPrices[dU],ca[W][kDistTableSizeMax],hl[W][bd];G cT[1 << ck],isRep[bi],isRepG0[bi],isRepG1[bi],isRepG2[bi],fy[bi][bJ],gv[bi][bJ],eT[W][1 << bq],in[bd];CLenEnc lenProbs,gM;Z el[1 << gL];CLenPriceEnc lenEnc,eN;ez bQ[hj];CSaveState hm;};
#define MFB (p->matchFinderBase)
I LzmaEnc_SetProps(bk* p,z dX *props2){dX bL=*props2;LzmaEncProps_Normalize(&bL);if(bL.lc > LZMA_LC_MAX
|| bL.lp > LZMA_LP_MAX
|| bL.pb > LZMA_PB_MAX)
u hO;if(bL.bj > hF)
bL.bj=hF;{z fH dict64=bL.bj;if(dict64 > ((fH)1 << kDicLogSizeMaxCompress))
u hO;}p->bj=bL.bj;{f fb=(f)bL.fb;if(fb<5)
fb=5;if(fb > bB)
fb=bB;p->ce=fb;}p->lc=(f)bL.lc;p->lp=(f)bL.lp;p->pb=(f)bL.pb;p->fastMode=(bL.algo==0);MFB.fD=(Z)(bL.fD ? 1:0);{f N=4;if(bL.fD){if(bL.N<2)N=2;U if(bL.N<4)N=(f)bL.N;}if(bL.N >= 5)N=5;MFB.N=N;MFB.dL=(Z)bL.dL;}MFB.fF=bL.mc;p->cU=(I)bL.cU;u cB;}D LzmaEnc_SetDataSize(bk* p,fH expectedDataSiize){MFB.dH=expectedDataSiize;}
#define kState_Start 0
#define kState_LitAfterMatch 4
#define kState_LitAfterRep 5
#define kState_MatchAfterLit 7
#define hZ 8
L z Z fW[bi] ={0,0,0,0,1,2,3,4,5,6,4,5};L z Z fm[bi] ={7,7,7,7,7,7,7,10,10,10,10,10};L z Z hQ[bi] ={8,8,8,8,8,8,8,11,11,11,11,11};L z Z hH[bi]={9,9,9,9,9,9,9,11,11,11,11,11};
#define IsLitState(s)((s)< 7)
#define GetLenToPosState2(H)(((H)< W-1)? (H):W-1)
#define GetLenToPosState(H)(((H)< W+1)? (H)- 2:W-1)
#define hR (1 << 30)
L D RangeEnc_Construct(dr *p){p->dM=0;p->cj=0;}
#define hS(p)((y)(p)->bT+(y)((p)->eQ-(p)->cj)+ (y)(p)->cacheSize)
#define RC_BUF_SIZE (1 << 16)
L I RangeEnc_Alloc(dr *p){if(!p->cj){p->cj=(Z *)SzAlloc(RC_BUF_SIZE);if(!p->cj)
u 0;p->bufLim=p->cj+RC_BUF_SIZE;}u 1;}L D RangeEnc_Free(dr *p){free(p->cj);p->cj=0;}L D RangeEnc_Init(dr *p){p->Q=0xFFFFFFFF;p->cache=0;p->low=0;p->cacheSize=0;p->eQ=p->cj;p->bT=0;p->du=cB;}L D fg(dr *p){z y hz=(y)(p->eQ-p->cj);if(p->du==cB){if(hz!=p->dM->Write(p->dM,p->cj,hz))
p->du=SZ_ERROR_WRITE;}p->bT+=hz;p->eQ=p->cj;}L D RangeEnc_ShiftLow(dr *p){g low=(g)p->low;f high=(f)(p->low >> 32);p->low=(g)(low << 8);if(low<(g)0xFF000000||high!=0){{Z *eQ=p->eQ;*eQ++=(Z)(p->cache+high);p->cache=(f)(low >> 24);p->eQ=eQ;if(eQ==p->bufLim)
fg(p);if(p->cacheSize==0)
u;}high+=0xFF;cr(;;){Z *eQ=p->eQ;*eQ++=(Z)(high);p->eQ=eQ;if(eQ==p->bufLim)
fg(p);if(--p->cacheSize==0)
u;}}p->cacheSize++;}L D RangeEnc_FlushData(dr *p){I i;cr(i=0;i<5;i++)
RangeEnc_ShiftLow(p);}
#define RC_NORM(p)if(Q<kTopValue){Q <<= 8;RangeEnc_ShiftLow(p);}
#define cO(p,F)he=*(F);dI=(Q >> bl)* he;
#define gN(p,F,cw){g mask;cO(p,F)mask=0-(g)cw;Q&= mask;mask&= dI;Q-=mask;(p)->low+=mask;mask=(g)cw-1;Q+=dI&mask;mask&= (bX-((1 << dS)- 1));mask+=((1 << dS)- 1);he+=(g)((I)(mask-he)>> dS);*(F)= (G)he;RC_NORM(p)}
#define gO(p,F)Q=dI;*(F)= (G)(he+((bX-he)>> dS));
#define RC_BIT_1_BASE(p,F)Q-=dI;(p)->low+=dI;*(F)= (G)(he-(he >> dS));
#define RC_BIT_0(p,F)gO(p,F)RC_NORM(p)
#define fO(p,F)RC_BIT_1_BASE(p,F)RC_NORM(p)
L D RangeEnc_EncodeBit_0(dr *p,G *F){g Q,he,dI;Q=p->Q;cO(p,F)
RC_BIT_0(p,F)
p->Q=Q;}L D LitEnc_Encode(dr *p,G *A,g cx){g Q=p->Q;cx |= 0x100;do{g he,dI;G *F=A+(cx >> 8);g cw=(cx >> 7)&1;cx <<= 1;gN(p,F,cw)
}bK(cx<0x10000);p->Q=Q;}L D LitEnc_EncodeMatched(dr *p,G *A,g cx,g bu){g Q=p->Q;g dO=0x100;cx |= 0x100;do{g he,dI;G *F;g cw;bu <<= 1;F=A+(dO+(bu&dO)+ (cx >> 8));cw=(cx >> 7)&1;cx <<= 1;dO&= ~(bu ^ cx);gN(p,F,cw)
}bK(cx<0x10000);p->Q=Q;}L D LzmaEnc_InitPriceTables(g *be){g i;cr(i=0;i<(bX >> by);i++){z f kCyclesBits=eL;g w=(i << by)+ (1 << (by-1));f bitCount=0;f j;cr(j=0;j<kCyclesBits;j++){w=w * w;bitCount <<= 1;bK(w >= ((g)1 << 16)){w >>= 1;bitCount++;}}be[i]=(g)(((f)bl << kCyclesBits)- 15-bitCount);}}
#define GET_PRICE(F,cw)p->be[((F)^ (f)(((-(I)(cw)))&(bX-1)))>> by]
#define cP(F,cw)be[((F)^ (f)((-((I)(cw)))&(bX-1)))>> by]
#define cZ(F)p->be[(F)>> by]
#define dv(F)p->be[((F)^ (bX-1))>> by]
#define em(F)be[(F)>> by]
#define en(F)be[((F)^ (bX-1))>> by]
L g LitEnc_GetPrice(z G *A,g cx,z g *be){g J=0;cx |= 0x100;do{f cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);}bK(cx >= 2);u J;}L g dw(z G *A,g cx,g bu,z g *be){g J=0;g dO=0x100;cx |= 0x100;do{bu <<= 1;J+=cP(A[dO+(bu&dO)+ (cx >> 8)],(cx >> 7)&1);cx <<= 1;dO&= ~(bu ^ cx);}bK(cx<0x10000);u J;}L D RcTree_ReverseEncode(dr *rc,G *A,f hA,f cx){g Q=rc->Q;f m=1;do{g he,dI;f cw=cx&1;cx >>= 1;gN(rc,A+m,cw)
m=(m << 1)| cw;}bK(--hA);rc->Q=Q;}L D LenEnc_Init(CLenEnc *p){f i;cr(i=0;i<(bJ << (bO+1));i++)
p->low[i]=bY;cr(i=0;i<df;i++)
p->high[i]=bY;}L D LenEnc_Encode(CLenEnc *p,dr *rc,f cx,f P){g Q,he,dI;G *A=p->low;Q=rc->Q;cO(rc,A)
if(cx >= R){fO(rc,A)
A+=R;cO(rc,A)
if(cx >= R * 2){fO(rc,A)
rc->Q=Q;LitEnc_Encode(rc,p->high,cx-R * 2);u;}cx-=R;}{f m;f cw;RC_BIT_0(rc,A)
A+=(P << (1+bO));cw=(cx >> 2);gN(rc,A+1,cw)m=(1 << 1)+ cw;cw=(cx >> 1)&1;gN(rc,A+m,cw)m=(m << 1)+ cw;cw=cx&1;gN(rc,A+m,cw)
rc->Q=Q;}}L D SetPrices_3(z G *A,g startPrice,g *eM,z g *be){f i;cr(i=0;i<8;i+=2){g J=startPrice;g F;J+=cP(A[1 ],(i >> 2));J+=cP(A[2+(i >> 2)],(i >> 1)&1);F=A[4+(i >> 1)];eM[i ]=J+em(F);eM[i+1]=J+en(F);}}L D dk(
CLenPriceEnc *p,f numPosStates,z CLenEnc *enc,z g *be){g b;{f F=enc->low[0];g a,c;f P;b=en(F);a=em(F);c=b+em(enc->low[R]);cr(P=0;P<numPosStates;P++){g *eM=p->eM[P];z G *A=enc->low+(P << (1+bO));SetPrices_3(A,a,eM,be);SetPrices_3(A+R,c,eM+R,be);}}{f i=p->tableSize;if(i > R * 2){z G *A=enc->high;g *eM=p->eM[0]+R * 2;i-=R * 2-1;i >>= 1;b+=en(enc->low[R]);do{f cx=--i+(1 << (dq-1));g J=b;do{z f cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);}bK(cx >= 2);{z f F=A[(y)i+(1 << (dq-1))];eM[(y)i * 2 ]=J+em(F);eM[(y)i * 2+1]=J+en(F);}}bK(i);{f P;z y hz=(p->tableSize-R * 2)* id(p->eM[0][0]);cr(P=1;P<numPosStates;P++)
memcpy(p->eM[P]+R * 2,p->eM[0]+R * 2,hz);}}}}
#define ea(p,hz){p->cf+=(hz);p->ci.Skip(p->bz,(g)(hz));}
L f dQ(bk *p,f *numPairsRes){f bg;p->cf++;p->bE=p->ci.dl(p->bz);{z g *d=p->ci.fs(p->bz,p->eB);bg=(f)(d-p->eB);}*numPairsRes=bg;if(bg==0)
u 0;{z f H=p->eB[(y)bg-2];if(H!=p->ce)
u H;{g bE=p->bE;if(bE > bB)
bE=bB;{z Z *p1=p->ci.bI(p->bz)- 1;z Z *p2=p1+H;z dm dif=(dm)-1-(dm)p->eB[(y)bg-1];z Z *lim=p1+bE;cr(;p2!=lim&&*p2==p2[dif];p2++){}u(f)(p2-p1);}}}}
#define hn ((g)(I)-1)
#define MakeAs_Lit(p){(p)->bo=hn;(p)->bR=0;}
#define MakeAs_ShortRep(p){(p)->bo=0;(p)->bR=0;}
#define IsShortRep(p)((p)->bo==0)
#define GetPrice_ShortRep(p,E,P)(cZ(p->isRepG0[E])+ cZ(p->gv[E][P]))
#define hT(p,E,P)(dv(p->fy[E][P])+ dv(p->gv[E][P]))+ dv(p->isRep[E])+ cZ(p->isRepG0[E])
L g GetPrice_PureRep(z bk *p,f ds,y E,y P){g J;g F=p->isRepG0[E];if(ds==0){J=cZ(F);J+=dv(p->gv[E][P]);}U{J=dv(F);F=p->isRepG1[E];if(ds==1)
J+=cZ(F);U{J+=dv(F);J+=GET_PRICE(p->isRepG2[E],ds-2);}}u J;}L f Backward(bk *p,f bv){f wr=bv+1;p->optEnd=wr;cr(;;){g bo=p->bQ[bv].bo;f H=(f)p->bQ[bv].H;f bR=(f)p->bQ[bv].bR;bv-=H;if(bR){wr--;p->bQ[wr].H=(g)H;bv-=bR;H=bR;if(bR==1){p->bQ[wr].bo=bo;bo=hn;}U{p->bQ[wr].bo=0;H--;wr--;p->bQ[wr].bo=hn;p->bQ[wr].H=1;}}if(bv==0){p->eA=bo;p->optCur=wr;u H;}wr--;p->bQ[wr].bo=bo;p->bQ[wr].H=(g)H;}}
#define LIT_PROBS(bH,prevByte)(p->dg+(g)3 * (((((bH)<< 8)+ (prevByte))&p->lpMask)<< p->lc))
L f GetOptimum(bk *p,g dJ){f gP,bv;g ba[bb];f gQ[bb];g *eB;{g bE;f bg,bD,eO,i,P;g fz,dC;z Z *V;Z da,bu;p->optCur=p->optEnd=0;if(p->cf==0)
bD=dQ(p,&bg);U{bD=p->fN;bg=p->bg;}bE=p->bE;if(bE<2){p->eA=hn;u 1;}if(bE > bB)
bE=bB;V=p->ci.bI(p->bz)- 1;eO=0;cr(i=0;i<bb;i++){f H;z Z *db;ba[i]=p->ba[i];db=V-ba[i];if(V[0]!=db[0]||V[1]!=db[1]){gQ[i]=0;eY;}cr(H=2;H<bE&&V[H]==db[H];H++){}gQ[i]=H;if(H > gQ[eO])
eO=i;if(H==bB)
bN;}if(gQ[eO] >= p->ce){f H;p->eA=(g)eO;H=gQ[eO];ea(p,H-1)
u H;}eB=p->eB;
#define gh eB
if(bD >= p->ce){p->eA=gh[(y)bg-1]+bb;ea(p,bD-1)
u bD;}da=*V;bu=*(V-ba[0]);gP=gQ[eO];if(gP<=bD)
gP=bD;if(gP<2&&da!=bu){p->eA=hn;u 1;}p->bQ[0].E=(CState)p->E;P=(dJ&p->fj);{z G *A=LIT_PROBS(dJ,*(V-1));p->bQ[1].J=cZ(p->fy[p->E][P])+
(!IsLitState(p->E)?
dw(A,da,bu,p->be):LitEnc_GetPrice(A,da,p->be));}MakeAs_Lit(&p->bQ[1])
fz=dv(p->fy[p->E][P]);dC=fz+dv(p->isRep[p->E]);if(bu==da&&gQ[0]==0){g fA=dC+GetPrice_ShortRep(p,p->E,P);if(fA<p->bQ[1].J){p->bQ[1].J=fA;MakeAs_ShortRep(&p->bQ[1])
}if(gP<2){p->eA=p->bQ[1].bo;u 1;}}p->bQ[1].H=1;p->bQ[0].ba[0]=ba[0];p->bQ[0].ba[1]=ba[1];p->bQ[0].ba[2]=ba[2];p->bQ[0].ba[3]=ba[3];cr(i=0;i<bb;i++){f eP=gQ[i];g J;if(eP<2)
eY;J=dC+GetPrice_PureRep(p,i,p->E,P);do{g dK=J+dB(&p->eN,P,eP);ez *bQ =&p->bQ[eP];if(dK<bQ->J){bQ->J=dK;bQ->H=(g)eP;bQ->bo=(g)i;bQ->bR=0;}}bK(--eP >= 2);}{f H=gQ[0]+1;if(H<=bD){f dO=0;g gR=fz+cZ(p->isRep[p->E]);if(H<2)
H=2;U
bK(H > gh[dO])
dO+=2;cr(;;H++){ez *bQ;g bo=gh[(y)dO+1];g J=gR+dB(&p->lenEnc,P,H);f lenToPosState=GetLenToPosState(H);if(bo<bd)
J+=p->hl[lenToPosState][bo&(bd-1)];U{f gg;GetPosSlot2(bo,gg)
J+=p->alignPrices[bo&kAlignMask];J+=p->ca[lenToPosState][gg];}bQ =&p->bQ[H];if(J<bQ->J){bQ->J=J;bQ->H=(g)H;bQ->bo=bo+bb;bQ->bR=0;}if(H==gh[dO]){dO+=2;if(dO==bg)
bN;}}}}bv=0;}cr(;;){f bE;g cp;f newLen,bg,prev,E,P,startLen;g gw,fz,dC;I nextIsLit;Z da,bu;z Z *V;ez *fn,*eh;if(++bv==gP)
bN;if(bv >= hj-64){f j,best;g J=p->bQ[bv].J;best=bv;cr(j=bv+1;j<=gP;j++){g dK=p->bQ[j].J;if(J >= dK){J=dK;best=j;}}{f dV=best-bv;if(dV!=0){ea(p,dV)
}}bv=best;bN;}newLen=dQ(p,&bg);if(newLen >= p->ce){p->bg=bg;p->fN=newLen;bN;}fn =&p->bQ[bv];dJ++;prev=bv-fn->H;if(fn->H==1){E=(f)p->bQ[prev].E;if(IsShortRep(fn))
E=hH[E];U
E=fW[E];}U{z ez *eU;g b0;g bo=fn->bo;if(fn->bR){prev-=(f)fn->bR;E=hZ;if(fn->bR==1)
E=(bo<bb ? hZ:kState_MatchAfterLit);}U{E=(f)p->bQ[prev].E;if(bo<bb)
E=hQ[E];U
E=fm[E];}eU =&p->bQ[prev];b0=eU->ba[0];if(bo<bb){if(bo==0){ba[0]=b0;ba[1]=eU->ba[1];ba[2]=eU->ba[2];ba[3]=eU->ba[3];}U{ba[1]=b0;b0=eU->ba[1];if(bo==1){ba[0]=b0;ba[2]=eU->ba[2];ba[3]=eU->ba[3];}U{ba[2]=b0;ba[0]=eU->ba[bo];ba[3]=eU->ba[bo ^ 1];}}}U{ba[0]=(bo-bb+1);ba[1]=b0;ba[2]=eU->ba[1];ba[3]=eU->ba[2];}}fn->E=(CState)E;fn->ba[0]=ba[0];fn->ba[1]=ba[1];fn->ba[2]=ba[2];fn->ba[3]=ba[3];V=p->ci.bI(p->bz)- 1;da=*V;bu=*(V-ba[0]);P=(dJ&p->fj);{g curPrice=fn->J;f F=p->fy[E][P];fz=curPrice+dv(F);gw=curPrice+cZ(F);}eh =&p->bQ[(y)bv+1];nextIsLit=0;if((eh->J<hR
&&bu==da)
|| gw > eh->J
)
gw=0;U{z G *A=LIT_PROBS(dJ,*(V-1));gw+=(!IsLitState(E)?
dw(A,da,bu,p->be):LitEnc_GetPrice(A,da,p->be));if(gw<eh->J){eh->J=gw;eh->H=1;MakeAs_Lit(eh)
nextIsLit=1;}}dC=fz+dv(p->isRep[E]);cp=p->bE;{f hX=hj-1-bv;if(cp > hX)
cp=(g)hX;}if(IsLitState(E))
if(bu==da)
if(dC<eh->J)
if(
eh->H<2
|| (eh->bo!=0
)){g fA=dC+GetPrice_ShortRep(p,E,P);if(fA<eh->J){eh->J=fA;eh->H=1;MakeAs_ShortRep(eh)
nextIsLit=0;}}if(cp<2)
eY;bE=(cp<=p->ce ? cp:p->ce);if(!nextIsLit
&&gw!=0
&&bu!=da
&&cp > 2){z Z *db=V-ba[0];if(V[1]==db[1]&&V[2]==db[2]){f H;f cv=p->ce+1;if(cv > cp)
cv=cp;cr(H=3;H<cv&&V[H]==db[H];H++){}{f state2=fW[E];f dd=(dJ+1)&p->fj;g J=gw+hT(p,state2,dd);{f cy=bv+H;if(gP<cy)
gP=cy;{g dK;ez *bQ;H--;dK=J+dB(&p->eN,dd,H);bQ =&p->bQ[cy];if(dK<bQ->J){bQ->J=dK;bQ->H=(g)H;bQ->bo=0;bQ->bR=1;}}}}}}startLen=2;{f ds=0;cr(;ds<bb;ds++){f H;g J;z Z *db=V-ba[ds];if(V[0]!=db[0]||V[1]!=db[1])
eY;cr(H=2;H<bE&&V[H]==db[H];H++){}{f cy=bv+H;if(gP<cy)
gP=cy;}{f di=H;J=dC+GetPrice_PureRep(p,ds,E,P);do{g dK=J+dB(&p->eN,P,di);ez *bQ =&p->bQ[bv+di];if(dK<bQ->J){bQ->J=dK;bQ->H=(g)di;bQ->bo=(g)ds;bQ->bR=0;}}bK(--di >= 2);}if(ds==0)startLen=H+1;{f di=H+1;f cv=di+p->ce;if(cv > cp)
cv=cp;di+=2;if(di<=cv)
if(V[di-2]==db[di-2])
if(V[di-1]==db[di-1]){f state2=hQ[E];f dd=(dJ+H)&p->fj;J+=dB(&p->eN,P,H)
+ cZ(p->fy[state2][dd])
+ dw(LIT_PROBS(dJ+H,V[(y)H-1]),V[H],db[H],p->be);state2=kState_LitAfterRep;dd=(dd+1)&p->fj;J+=hT(p,state2,dd);cr(;di<cv&&V[di]==db[di];di++){}di-=H;{{f cy=bv+H+di;if(gP<cy)
gP=cy;{g dK;ez *bQ;di--;dK=J+dB(&p->eN,dd,di);bQ =&p->bQ[cy];if(dK<bQ->J){bQ->J=dK;bQ->H=(g)di;bQ->bR=(CExtra)(H+1);bQ->bo=(g)ds;}}}}}}}}if(newLen > bE){newLen=bE;cr(bg=0;newLen > gh[bg];bg+=2);gh[bg]=(g)newLen;bg+=2;}if(newLen >= startLen){g gR=fz+cZ(p->isRep[E]);g bo;f dO,cY,H;{f cy=bv+newLen;if(gP<cy)
gP=cy;}dO=0;bK(startLen > gh[dO])
dO+=2;bo=gh[(y)dO+1];GetPosSlot2(bo,cY)
cr(H=startLen;;H++){g J=gR+dB(&p->lenEnc,P,H);{ez *bQ;f lenNorm=H-2;lenNorm=GetLenToPosState2(lenNorm);if(bo<bd)
J+=p->hl[lenNorm][bo&(bd-1)];U
J+=p->ca[lenNorm][cY]+p->alignPrices[bo&kAlignMask];bQ =&p->bQ[bv+H];if(J<bQ->J){bQ->J=J;bQ->H=(g)H;bQ->bo=bo+bb;bQ->bR=0;}}if(H==gh[dO]){z Z *db=V-bo-1;f di=H+1;f cv=di+p->ce;if(cv > cp)
cv=cp;di+=2;if(di<=cv)
if(V[di-2]==db[di-2])
if(V[di-1]==db[di-1]){cr(;di<cv&&V[di]==db[di];di++){}di-=H;{f state2=fm[E];f dd=(dJ+H)&p->fj;f cy;J+=cZ(p->fy[state2][dd]);J+=dw(LIT_PROBS(dJ+H,V[(y)H-1]),V[H],db[H],p->be);state2=kState_LitAfterMatch;dd=(dd+1)&p->fj;J+=hT(p,state2,dd);cy=bv+H+di;if(gP<cy)
gP=cy;{g dK;ez *bQ;di--;dK=J+dB(&p->eN,dd,di);bQ =&p->bQ[cy];if(dK<bQ->J){bQ->J=dK;bQ->H=(g)di;bQ->bR=(CExtra)(H+1);bQ->bo=bo+bb;}}}}dO+=2;if(dO==bg)
bN;bo=gh[(y)dO+1];GetPosSlot2(bo,cY)
}}}}do
p->bQ[gP].J=hR;bK(--gP);u Backward(p,bv);}
#define ChangePair(smallDist,bigDist)(((bigDist)>> 7)>(smallDist))
L f GetOptimumFast(bk *p){g bE,eC;f bD,bg,ds,eP,i;z Z *V;if(p->cf==0)
bD=dQ(p,&bg);U{bD=p->fN;bg=p->bg;}bE=p->bE;p->eA=hn;if(bE<2)
u 1;if(bE > bB)
bE=bB;V=p->ci.bI(p->bz)- 1;eP=ds=0;cr(i=0;i<bb;i++){f H;z Z *db=V-p->ba[i];if(V[0]!=db[0]||V[1]!=db[1])
eY;cr(H=2;H<bE&&V[H]==db[H];H++){}if(H >= p->ce){p->eA=(g)i;ea(p,H-1)
u H;}if(H > eP){ds=i;eP=H;}}if(bD >= p->ce){p->eA=p->eB[(y)bg-1]+bb;ea(p,bD-1)
u bD;}eC=0;if(bD >= 2){eC=p->eB[(y)bg-1];bK(bg > 2){g dist2;if(bD!=p->eB[(y)bg-4]+1)
bN;dist2=p->eB[(y)bg-3];if(!ChangePair(dist2,eC))
bN;bg-=2;bD--;eC=dist2;}if(bD==2&&eC >= 0x80)
bD=1;}if(eP >= 2)
if(eP+1 >= bD
|| (eP+2 >= bD&&eC >= (1 << 9))
|| (eP+3 >= bD&&eC >= (1 << 15))){p->eA=(g)ds;ea(p,eP-1)
u eP;}if(bD<2||bE<=2)
u 1;{f len1=dQ(p,&p->bg);p->fN=len1;if(len1 >= 2){g newDist=p->eB[(y)p->bg-1];if((len1 >= bD&&newDist<eC)
|| (len1==bD+1&&!ChangePair(eC,newDist))
|| (len1 > bD+1)
|| (len1+1 >= bD&&bD >= 3&&ChangePair(newDist,eC)))
u 1;}}V=p->ci.bI(p->bz)- 1;cr(i=0;i<bb;i++){f H,cv;z Z *db=V-p->ba[i];if(V[0]!=db[0]||V[1]!=db[1])
eY;cv=bD-1;cr(H=2;;H++){if(H >= cv)
u 1;if(V[H]!=db[H])
bN;}}p->eA=eC+bb;if(bD!=2){ea(p,bD-2)
}u bD;}L D WriteEndMarker(bk *p,f P){g Q;Q=p->rc.Q;{g he,dI;G *F =&p->fy[p->E][P];cO(&p->rc,F)
fO(&p->rc,F)
F =&p->isRep[p->E];cO(&p->rc,F)
RC_BIT_0(&p->rc,F)
}p->E=fm[p->E];p->rc.Q=Q;LenEnc_Encode(&p->lenProbs,&p->rc,0,P);Q=p->rc.Q;{G *A=p->eT[0];f m=1;do{g he,dI;cO(p,A+m)
fO(&p->rc,A+m)
m=(m << 1)+ 1;}bK(m<(1 << bq));}{f hA=30-ck;do{Q >>= 1;p->rc.low+=Q;RC_NORM(&p->rc)
}bK(--hA);}{G *A=p->cT;f m=1;do{g he,dI;cO(p,A+m)
fO(&p->rc,A+m)
m=(m << 1)+ 1;}bK(m<dU);}p->rc.Q=Q;}L I CheckErrors(bk *p){if(p->fG!=cB)
u p->fG;if(p->rc.du!=cB)
p->fG=SZ_ERROR_WRITE;if(MFB.fG!=cB)
p->fG=8;if(p->fG!=cB)
p->finished=1;u p->fG;}L I Flush(bk *p,g nowPos){p->finished=1;if(p->cU)
WriteEndMarker(p,nowPos&p->fj);RangeEnc_FlushData(&p->rc);fg(&p->rc);u CheckErrors(p);}L D FillAlignPrices(bk *p){f i;z g *be=p->be;z G *A=p->cT;cr(i=0;i<dU / 2;i++){g J=0;f cx=i;f m=1;f cw;g F;cw=cx&1;cx >>= 1;J+=cP(A[m],cw);m=(m << 1)+ cw;cw=cx&1;cx >>= 1;J+=cP(A[m],cw);m=(m << 1)+ cw;cw=cx&1;cx >>= 1;J+=cP(A[m],cw);m=(m << 1)+ cw;F=A[m];p->alignPrices[i ]=J+em(F);p->alignPrices[i+8]=J+en(F);}}L D hI(bk *p){g tempPrices[bd];f i,lps;z g *be=p->be;p->hk=0;cr(i=dA / 2;i<bd / 2;i++){f cY=GetPosSlot1(i);f eG=(cY >> 1)- 1;f base=((2 | (cY&1))<< eG);z G *A=p->in+(y)base * 2;g J=0;f m=1;f cx=i;f cy=(f)1 << eG;base+=i;if(eG)
do{f cw=cx&1;cx >>= 1;J+=cP(A[m],cw);m=(m << 1)+ cw;}bK(--eG);{f F=A[m];tempPrices[base ]=J+em(F);tempPrices[base+cy]=J+en(F);}}cr(lps=0;lps<W;lps++){f gg;f distTableSize2=(p->distTableSize+1)>> 1;g *ca=p->ca[lps];z G *A=p->eT[lps];cr(gg=0;gg<distTableSize2;gg++){g J;f cw;f cx=gg+(1 << (bq-1));f F;cw=cx&1;cx >>= 1;J=cP(A[cx],cw);cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);cw=cx&1;cx >>= 1;J+=cP(A[cx],cw);F=A[(y)gg+(1 << (bq-1))];ca[(y)gg * 2 ]=J+em(F);ca[(y)gg * 2+1]=J+en(F);}{g dV=((g)((cF / 2-1)- ck)<< eL);cr(gg=cF / 2;gg<distTableSize2;gg++){ca[(y)gg * 2 ]+=dV;ca[(y)gg * 2+1]+=dV;dV+=((g)1 << eL);}}{g *dp=p->hl[lps];dp[0]=ca[0];dp[1]=ca[1];dp[2]=ca[2];dp[3]=ca[3];cr(i=4;i<bd;i+=2){g slotPrice=ca[GetPosSlot1(i)];dp[i ]=slotPrice+tempPrices[i];dp[i+1]=slotPrice+tempPrices[i+1];}}}}L D LzmaEnc_Construct(bk *p){RangeEnc_Construct(&p->rc);MatchFinder_Construct(&MFB);{dX bL;LzmaEncProps_Init(&bL);LzmaEnc_SetProps((bk*)(D *)p,&bL);}LzmaEnc_FastPosInit(p->el);LzmaEnc_InitPriceTables(p->be);p->dg=0;p->hm.dg=0;}bk* LzmaEnc_Create(){D *p;p=SzAlloc(id(bk));if(p)
LzmaEnc_Construct((bk *)p);u(bk*)p;}L D gS(bk *p){free(p->dg);free(p->hm.dg);p->dg=0;p->hm.dg=0;}L D LzmaEnc_Destruct(bk *p){MatchFinder_Free(&MFB);gS(p);RangeEnc_Free(&p->rc);}D LzmaEnc_Destroy(bk* p){LzmaEnc_Destruct(p);free(p);}L I LzmaEnc_CodeOneBlock(bk *p,g maxPackSize,g maxUnpackSize){g eb,startPos32;if(p->needInit){p->ci.Init(p->bz);p->needInit=0;}if(p->finished)
u p->fG;RINOK(CheckErrors(p))
eb=(g)p->nowPos64;startPos32=eb;if(p->nowPos64==0){f bg;Z da;if(p->ci.dl(p->bz)== 0)
u Flush(p,eb);dQ(p,&bg);RangeEnc_EncodeBit_0(&p->rc,&p->fy[kState_Start][0]);da=*(p->ci.bI(p->bz)- p->cf);LitEnc_Encode(&p->rc,p->dg,da);p->cf--;eb++;}if(p->ci.dl(p->bz)!= 0)
cr(;;){g bo;f H,P;g Q,he,dI;G *A;if(p->fastMode)
H=GetOptimumFast(p);U{f oci=p->optCur;if(p->optEnd==oci)
H=GetOptimum(p,eb);U{z ez *bQ =&p->bQ[oci];H=bQ->H;p->eA=bQ->bo;p->optCur=oci+1;}}P=(f)eb&p->fj;Q=p->rc.Q;A =&p->fy[p->E][P];cO(&p->rc,A)
bo=p->eA;if(bo==hn){Z da;z Z *V;f E;RC_BIT_0(&p->rc,A)
p->rc.Q=Q;V=p->ci.bI(p->bz)- p->cf;A=LIT_PROBS(eb,*(V-1));da=*V;E=p->E;p->E=fW[E];if(IsLitState(E))
LitEnc_Encode(&p->rc,A,da);U
LitEnc_EncodeMatched(&p->rc,A,da,*(V-p->ba[0]));}U{fO(&p->rc,A)
A =&p->isRep[p->E];cO(&p->rc,A)
if(bo<bb){fO(&p->rc,A)
A =&p->isRepG0[p->E];cO(&p->rc,A)
if(bo==0){RC_BIT_0(&p->rc,A)
A =&p->gv[p->E][P];cO(&p->rc,A)
if(H!=1){RC_BIT_1_BASE(&p->rc,A)
}U{gO(&p->rc,A)
p->E=hH[p->E];}}U{fO(&p->rc,A)
A =&p->isRepG1[p->E];cO(&p->rc,A)
if(bo==1){gO(&p->rc,A)
bo=p->ba[1];}U{fO(&p->rc,A)
A =&p->isRepG2[p->E];cO(&p->rc,A)
if(bo==2){gO(&p->rc,A)
bo=p->ba[2];}U{RC_BIT_1_BASE(&p->rc,A)
bo=p->ba[3];p->ba[3]=p->ba[2];}p->ba[2]=p->ba[1];}p->ba[1]=p->ba[0];p->ba[0]=bo;}RC_NORM(&p->rc)
p->rc.Q=Q;if(H!=1){LenEnc_Encode(&p->gM,&p->rc,H-dP,P);--p->fl;p->E=hQ[p->E];}}U{f cY;RC_BIT_0(&p->rc,A)
p->rc.Q=Q;p->E=fm[p->E];LenEnc_Encode(&p->lenProbs,&p->rc,H-dP,P);bo-=bb;p->ba[3]=p->ba[2];p->ba[2]=p->ba[1];p->ba[1]=p->ba[0];p->ba[0]=bo+1;p->hk++;GetPosSlot(bo,cY){g cx=(g)cY+(1 << bq);Q=p->rc.Q;A=p->eT[GetLenToPosState(H)];do{G *F=A+(cx >> bq);g cw=(cx >> (bq-1))&1;cx <<= 1;gN(&p->rc,F,cw)
}bK(cx<(1 << bq * 2));p->rc.Q=Q;}if(bo >= dA){f eG=((cY >> 1)- 1);if(bo<bd){f base=((2 | (cY&1))<< eG);RcTree_ReverseEncode(&p->rc,p->in+base,eG,(f)(bo));}U{g pos2=(bo | 0xF)<< (32-eG);Q=p->rc.Q;do{Q >>= 1;p->rc.low+=Q&(0-(pos2 >> 31));pos2+=pos2;RC_NORM(&p->rc)
}bK(pos2!=0xF0000000);{f m=1;f cw;cw=bo&1;bo >>= 1;gN(&p->rc,p->cT+m,cw)m=(m << 1)+ cw;cw=bo&1;bo >>= 1;gN(&p->rc,p->cT+m,cw)m=(m << 1)+ cw;cw=bo&1;bo >>= 1;gN(&p->rc,p->cT+m,cw)m=(m << 1)+ cw;cw=bo&1;gN(&p->rc,p->cT+m,cw)
p->rc.Q=Q;}}}}}eb+=(g)H;p->cf-=H;if(p->cf==0){g bT;if(!p->fastMode){if(p->hk >= 64){FillAlignPrices(p);hI(p);dk(&p->lenEnc,(f)1 << p->pb,&p->lenProbs,p->be);}if(p->fl<=0){p->fl=REP_LEN_COUNT;dk(&p->eN,(f)1 << p->pb,&p->gM,p->be);}}if(p->ci.dl(p->bz)== 0)
bN;bT=eb-startPos32;if(maxPackSize){if(bT+hj+300 >= maxUnpackSize
|| hS(&p->rc)+ kPackReserve >= maxPackSize)
bN;}U if(bT >= (1 << 17)){p->nowPos64+=eb-startPos32;u CheckErrors(p);}}}p->nowPos64+=eb-startPos32;u Flush(p,eb);}
#define kBigHashDicLimit ((g)1 << 24)
L I LzmaEnc_Alloc(bk *p,g ec){g beforeSize=hj;g bj;if(!RangeEnc_Alloc(&p->rc))
u ga;{z f lclp=p->lc+p->lp;if(!p->dg||!p->hm.dg||p->lclp!=lclp){gS(p);p->dg=(G *)SzAlloc(((y)0x300 * id(G))<< lclp);p->hm.dg=(G *)SzAlloc(((y)0x300 * id(G))<< lclp);if(!p->dg||!p->hm.dg){gS(p);u ga;}p->lclp=lclp;}}MFB.bigHash=(Z)(p->bj > kBigHashDicLimit ? 1:0);bj=p->bj;if(bj==((g)2 << 30)||
bj==((g)3 << 30)){bj-=1;}if(beforeSize+bj<ec)
beforeSize=ec-bj;{if(!MatchFinder_Create(&MFB,bj,beforeSize,p->ce,bB+1))
u ga;p->bz =&MFB;MatchFinder_CreateVTable(&MFB,&p->ci);}u cB;}L D LzmaEnc_Init(bk *p){f i;p->E=0;p->ba[0]=p->ba[1]=p->ba[2]=p->ba[3]=1;RangeEnc_Init(&p->rc);cr(i=0;i<(1 << ck);i++)
p->cT[i]=bY;cr(i=0;i<bi;i++){f j;cr(j=0;j<bJ;j++){p->fy[i][j]=bY;p->gv[i][j]=bY;}p->isRep[i]=bY;p->isRepG0[i]=bY;p->isRepG1[i]=bY;p->isRepG2[i]=bY;}{cr(i=0;i<W;i++){G *A=p->eT[i];f j;cr(j=0;j<(1 << bq);j++)
A[j]=bY;}}{cr(i=0;i<bd;i++)
p->in[i]=bY;}{z y hz=(y)0x300 << (p->lp+p->lc);y k;G *A=p->dg;cr(k=0;k<hz;k++)
A[k]=bY;}LenEnc_Init(&p->lenProbs);LenEnc_Init(&p->gM);p->optEnd=0;p->optCur=0;{cr(i=0;i<hj;i++)
p->bQ[i].J=hR;}p->cf=0;p->fj=((f)1 << p->pb)- 1;p->lpMask=((g)0x100 << p->lp)- ((f)0x100 >> p->lc);}L D LzmaEnc_InitPrices(bk *p){if(!p->fastMode){hI(p);FillAlignPrices(p);}p->lenEnc.tableSize=p->eN.tableSize=p->ce+1-dP;p->fl=REP_LEN_COUNT;dk(&p->lenEnc,(f)1 << p->pb,&p->lenProbs,p->be);dk(&p->eN,(f)1 << p->pb,&p->gM,p->be);}L I LzmaEnc_AllocAndInit(bk *p,g ec){f i;cr(i=cF / 2;i<kDicLogSizeMax;i++)
if(p->bj<=((g)1 << i))
bN;p->distTableSize=i * 2;p->finished=0;p->fG=cB;p->nowPos64=0;p->needInit=1;RINOK(LzmaEnc_Alloc(p,ec))
LzmaEnc_Init(p);LzmaEnc_InitPrices(p);u cB;}I LzmaEnc_MemPrepare(bk* p,z Z *hh,y cN,g ec){gx(&MFB,hh,cN)
LzmaEnc_SetDataSize(p,cN);u LzmaEnc_AllocAndInit(p,ec);}D LzmaEnc_Finish(bk* p){UNUSED_VAR(p)
}dN bw{gC vt;Z *V;y gm;I overflow;}fT;L y SeqOutStreamBuf_Write(gC* pp,z D *V,y cV){gb(pp,fT,vt,p)
if(p->gm<cV){cV=p->gm;p->overflow=1;}if(cV!=0){memcpy(p->V,V,cV);p->gm-=cV;p->V+=cV;}u cV;}L I LzmaEnc_Encode2(bk *p){I du=cB;cr(;;){du=LzmaEnc_CodeOneBlock(p,0,0);if(du!=cB||p->finished)
bN;}LzmaEnc_Finish((bk*)(D *)p);u du;}I LzmaEnc_WriteProperties(bk* p,Z *bL,y *cV){if(*cV<dn)
u hO;*cV=dn;{z g bj=p->bj;g v;bL[0]=(Z)((p->pb * 5+p->lp)* 9+p->lc);if(bj >= ((g)1 << 21)){z g kDictMask=((g)1 << 20)- 1;v=(bj+kDictMask)&~kDictMask;if(v<bj)
v=bj;}U{f i=11 * 2;do{v=(g)(2+(i&1))<< (i >> 1);i++;}bK(v<bj);}SetUi32(bL+1,v)
u cB;}}I LzmaEnc_MemEncode(bk* p,Z *gu,y *eS,z Z *hh,y cN,I cU){I du;fT dM;dM.vt.Write=SeqOutStreamBuf_Write;dM.V=gu;dM.gm=*eS;dM.overflow=0;p->cU=cU;p->rc.dM =&dM.vt;du=LzmaEnc_MemPrepare(p,hh,cN,0);if(du==cB){du=LzmaEnc_Encode2(p);if(du==cB&&p->nowPos64!=cN)
du=SZ_ERROR_FAIL;}*eS-=(y)dM.gm;if(dM.overflow)
u 7;u du;}I LzmaEncode(Z *gu,y *eS,z Z *hh,y cN,z dX *bL,Z *propsEncoded,y *gn,I cU){bk* p=LzmaEnc_Create();I du;if(!p)
u ga;du=LzmaEnc_SetProps(p,bL);if(du==cB){du=LzmaEnc_WriteProperties(p,propsEncoded,gn);if(du==cB)
du=LzmaEnc_MemEncode(p,gu,eS,hh,cN,cU);}LzmaEnc_Destroy(p);u du;}I LzmaCompress(f gq*gu,y *eS,z f gq*hh,y cN,f gq*outProps,y *outPropsSize,I ff,f bj,I lc,I lp,I pb,I fb,I eF
){dX bL;LzmaEncProps_Init(&bL);bL.ff=ff;bL.bj=bj;bL.lc=lc;bL.lp=lp;bL.pb=pb;bL.fb=fb;bL.eF=eF;u LzmaEncode(gu,eS,hh,cN,&bL,outProps,outPropsSize,0);}I LzmaUncompress(f gq*gu,y *eS,z f gq*hh,y *cN,z f gq*bL,y gn){ELzmaStatus fk;u LzmaDecode(gu,eS,hh,cN,bL,(f)gn,LZMA_FINISH_ANY,&fk);}namespace sz{td::dx compress(td::Slice V){bV cV=V.cV();y ho=1 << 22;bV gu=new f gq[ho];f gq outProps[dn] ={};y outPropsSize=dn;I ff=9;f bj=1<<20;I lc=8;I lp=0;I pb=0;I fb=273;I eF=1;I du=LzmaCompress(gu,&ho,(f gq*)V.V(),cV,outProps,&outPropsSize,ff,bj,lc,lp,pb,fb,eF);(du==cB);u td::dx((gq*)gu,ho);}td::dx decompress(td::Slice V){bV cV=V.cV();y ho=1 << 22;bV gu=new f gq[ho];f gq bL[dn] ={0x8,0,0,0,0x80
};y gn=dn;I du=LzmaUncompress(gu,&ho,(f gq*)V.V(),&cV,bL,gn);(du==cB||du==fV);u td::dx((gq*)gu,ho);}}td::dx serialize_boc_opt(ostream&out,Ref<Cell> cell){(!cell.is_null());BagOfCells boc;boc.add_root(cell);(boc.import_cells().is_ok());bV cV=boc.estimate_serialized_size(0);td::dx bs{cV};bV eo=(f gq*)bs.V();boc_writers::BufferWriter ht{eo,eo+cV};vector<I> backrefs(boc.hJ.cV(),0);cr(I i=0;i<boc.cell_count;++i){z bV&cell=boc.hJ[i];cr(I j=0;j<cell.ref_num;++j){backrefs[cell.gT[j]]+=1;}}bV hK=ht.store_ptr;bV hu=[&](gi label){};I fX=0;cr(I i=0;i<boc.cell_count;++i){I k=boc.cell_count-1-i;z bV&hL=boc.hJ[k];I br=backrefs.at(k);if(br!=1){fX++;}}I rs=0;bK(fX >= (1LL << (rs << 3))){rs++;}bV store_byte=[&](f long long eu){ht.store_uint(eu,1);};bV bm=[&](f long long eu){ht.store_uint(eu,rs);};bV overwrite_ref=[&](I dJ,f long long eu){bV ptr=ht.store_ptr;ht.store_ptr=ht.store_start+dJ;bm(eu);ht.store_ptr=ptr;};store_byte(rs);hu("ref-cV");bm(fX);hu("cell-hz");vector<I> idx_to_ref(boc.hJ.cV(),-1);vector<hY<I,y>> refs_to_set;function<D(I,z vm::BagOfCells::CellInfo&)> store_cell;store_cell=[&](I idx,z vm::BagOfCells::CellInfo&hL){f gq eQ[256] ={};z Ref<DataCell>&dc=hL.dc_ref;I mask=0;cr(f j=0;j<hL.ref_num;++j){I gT=hL.gT[j];I br=backrefs.at(gT);if(br==1){mask |= (1 << j);}}I s=dc->serialize(eQ,256,false);eQ[0]=(eQ[0]&15)+ mask * 16;ht.store_bytes(eQ,s);hu("V");cr(f j=0;j<hL.ref_num;++j){I gT=hL.gT[j];I br=backrefs.at(gT);(br > 0);if(br==1){store_cell(gT,boc.hJ[gT]);}U{refs_to_set.emplace_back(gT,ht.dJ());bm(0);hu("ref");}}};I cells_cnt=0;cr(I i=0;i<boc.cell_count;++i){I k=boc.cell_count-1-i;z bV&hL=boc.hJ[k];I br=backrefs.at(k);if(br!=1){store_cell(k,hL);idx_to_ref[k]=cells_cnt++;}}cr(z bV&p:refs_to_set){overwrite_ref(p.second,idx_to_ref[p.first]);}bs.truncate(ht.dJ());u bs;}bw DeserializedCell{bool special;I bits;gi V;vector<hY<I,I>> refs{};};Ref<Cell> deserialize_boc_opt(ostream&out,td::Slice V){(!V.empty());I start=0;bV hK=0;bV hw=[&](gi label){if(hK<start){out << label << ":" << td::ConstBitPtr{(z f gq*)V.V()+ hK}.to_hex((start-hK)* 8)<< endl;hK=start;}};bV read_byte=[&](){(V.cV()> start);u(f gq)V[start++];};bV read_int=[&](I bytes){f long long du=0;bK(bytes > 0){du=(du << 8)+ read_byte();--bytes;}u du;};bV read_bytes=[&](I bytes){gi bs(bytes,0);cr(I i=0;i<bytes;++i){bs.at(i)= read_byte();}u bs;};bV ref_byte_size=(I)read_byte();hw("ref-cV");bV read_ref=[&](){u read_int(ref_byte_size);};bV cell_num=read_ref();hw("cell-hz");vector<DeserializedCell> ei;vector<array<hY<I,I>,4>> cells_refs;vector<I> ref_to_cd_idx(cell_num);function<D(I)> read_cell;read_cell=[&](I idx){bV d1=read_byte();bV d2=read_byte();bV ref_num=d1&7;bV special=(d1&8)> 0;bV mask=(d1 >> 4)&15;bV bytes=(d2+1)>> 1;bV V=read_bytes(bytes);hw("V");bV bits=bytes * 8;if(V.cV()> 0&&(d2&1)){f gq last_byte=V[V.cV()- 1];I fP=8;bK(fP&&(last_byte&1)== 0){fP--;last_byte >>= 1;}if(fP){last_byte=(last_byte&254)<< (8-fP);fP--;}V[V.cV()- 1]=last_byte;bits-=(8-fP);}if(idx!=-1){ref_to_cd_idx.at(idx)= ei.cV();}ei.push_back(DeserializedCell{special,bits,V,});I dc_idx=ei.cV()- 1;cr(I i=0;i<ref_num;++i){bool is_embedded=(mask >> i)&1;if(is_embedded){ei.at(dc_idx).refs.push_back({ei.cV(),-1});read_cell(-1);}U{ei.at(dc_idx).refs.push_back({-1,read_ref()});hw("ref");}}};cr(I i=0;i<cell_num;++i){read_cell(i);}vector<Ref<Cell>> cells(ei.cV());cr(I i=ei.cV()- 1;i >= 0;--i){bV&cd=ei[i];K cb;cb.store_bits(cd.V.V(),cd.bits);cr(z bV&ref:cd.refs){I idx=ref.first;if(idx==-1){(ref.second!=-1);idx=ref_to_cd_idx.at(ref.second);}bV&cell=cells.at(idx);(!cell.is_null());cb.bm(cell);}cells[i]=cb.finalize(cd.special);}u std::move(cells[0]);}td::dx do_compress(td::Slice V){u sz::compress(V);}td::dx do_decompress(td::Slice V){u sz::decompress(V);}td::dx compress(td::Slice V){NullStream ofs;Ref<Cell> block_root=std_boc_deserialize(V).move_as_ok();FullBlock block;q load_std_ctx{ofs};block.bp(load_std_ctx,block_root,0,true);q pack_opt_ctx{ofs};bV opt_block_cell=block.cA(pack_opt_ctx);bV opt_ser=serialize_boc_opt(ofs,opt_block_cell);bV compressed=do_compress(opt_ser);u compressed;}td::dx decompress(td::Slice V){NullStream ofs;bV decompressed=do_decompress(V);bV opt_deser=deserialize_boc_opt(ofs,decompressed);FullBlock opt_block;q parse_opt_ctx{ofs};opt_block.bZ(parse_opt_ctx,opt_deser,0,true);q pack_std_ctx{ofs};bV un_opt_block_cell=opt_block.bn(pack_std_ctx);bV boc=std_boc_serialize(un_opt_block_cell,31).move_as_ok();u boc;}I main(
){gi mode;cin >> mode;gi base64_data;cin >> base64_data;td::dx V(td::base64_decode(base64_data));V=(mode=="compress")? compress(V):decompress(V);cout << td::str_base64_encode(V)<< endl;}