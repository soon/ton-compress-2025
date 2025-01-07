
#define private public
#define final

#include <iostream>
#include <sstream>
#include "block/block-auto.h"
#include "block/block-parse.h"
#include "common/util.h"
#include "vm/boc.h"
#include "vm/cells/CellSlice.h"
#include <fstream>
#include <set>
#include "td/utils/lz4.h"
#include "td/utils/misc.h"
#include "td/utils/buffer.h"
#include "td/utils/misc.h"
#include "crypto/vm/boc-writers.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

using namespace vm;
using namespace std;

set<string> enabled_optimizations{
	"Block",
	"BlockExtra",
	
	"HashmapAugE",
	"HashmapAug",
	"HashmapAugNode",

	"InMsgDescr",
	"OutMsgDescr",
	"McBlockExtra",
	"ShardFees",

	
	"ShardAccountBlocks",

	"MERKLE_UPDATE",
	
	
	
	
	

	"Maybe",
};

CellSlice to_cs(Ref<Cell> cell)
{
	bool can_be_special = false;
	return load_cell_slice_special(std::move(cell), can_be_special);
}

struct ParseContext {
	ostream& out;

	
};




template <class T_TLB>
struct BaseFullCell
{
	
	CellSlice incoming_cs;
	
	CellSlice std_cell_cs;
	
	CellSlice ccs;

	string name;
	T_TLB type;
	int special_type = 0;

	BaseFullCell(string name): BaseFullCell(name, T_TLB()) {

	}

	BaseFullCell(string name, T_TLB type): name(name), type(type) {

	}

	virtual ~BaseFullCell() {}

	virtual void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) {
	}

	virtual void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		do_pack_std(ctx, cb, indent);
	}

	virtual void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) {
		do_unpack_std(ctx, cs, indent);
	}

	virtual void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
	}

	bool is_enabled() {
		return enabled_optimizations.count(name) > 0;
	}

	bool is_pruned_branch() {
		return special_type == 1;
	}

	void unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0, bool check_empty = false) {
		auto e = is_enabled();
		special_type = (int)cs.special_type();
		

		incoming_cs = cs;

		if (is_pruned_branch()) {
			cs.advance(288);
			
			return;
		}

		if (e) {
			do_unpack_std(ctx, cs, indent);
		} else {
			std_cell_cs = type.fetch(cs).write();
			
		}
		
	}

	void pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		auto e = is_enabled();

		

		if (is_pruned_branch()) {
			cb.append_cellslice(incoming_cs);
		} else if (e) {
			do_pack_opt(ctx, cb, indent);
		} else {
			cb.append_cellslice(std_cell_cs);
		}

		
	}

	void unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0, bool check_empty = false) {
		auto e = is_enabled();


		incoming_cs = cs;

		if (e) {
			do_unpack_opt(ctx, cs, indent);
		} else {
			std_cell_cs = type.fetch(cs).write();
		}

		
	}

	void pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		auto e = is_enabled();

		

		if (is_pruned_branch()) {
			cb.append_cellslice(incoming_cs);
		} else if (e) {
			do_pack_std(ctx, cb, indent);
		} else {
			if (!std_cell_cs.is_valid()) {
				throw runtime_error(name + ": optimization is disabled, but std_cell_cs is empty, meaning it was never set");
			}
			cb.append_cellslice(std_cell_cs);
		}

		
	}

	Ref<Cell> make_std_cell(ParseContext& ctx, int indent = 0) {
		CellBuilder cb;
		pack_std(ctx, cb, indent);
		
		return cb.finalize(special_type!=0);
	}

	Ref<Cell> make_opt_cell(ParseContext& ctx, int indent = 0) {
		CellBuilder cb;
		pack_opt(ctx, cb, indent);
		
		return cb.finalize(special_type!=0);
	}

	void cell_unpack_std(ParseContext& ctx, Ref<Cell> cell_ref, int indent = 0, bool check_empty = false)
	{
		auto cs = to_cs(std::move(cell_ref));

		unpack_std(ctx, cs, indent, check_empty);
	}

	void cell_unpack_opt(ParseContext& ctx, Ref<Cell> cell_ref, int indent = 0, bool check_empty = false)
	{
		auto cs = to_cs(std::move(cell_ref));
		unpack_opt(ctx, cs, indent, check_empty);
	}

	void fetch_remaining(CellSlice& cs) {
		ccs = cs;
		cs.advance(cs.size());
		cs.advance_refs(cs.size_refs());
	}

	void append_remaining(CellBuilder& cb) {
		cb.append_cellslice(ccs);
	}
};

struct AugDataProvider {
	virtual ~AugDataProvider() {}

	virtual CellSlice calc_aug_data() {
		throw runtime_error("aug data requested but not implemented");
	}
};

template <class T_TLB>
struct AddValues {

	T_TLB add_type;

	AddValues(T_TLB add_type): add_type(add_type) {}

	AddValues(): AddValues(T_TLB{}) {}

	virtual ~AddValues() {}

	virtual CellSlice add_values(CellSlice& cs1, CellSlice& cs2) {
		CellBuilder cb;
		CHECK(add_type.add_values(cb, cs1, cs2));
		return cb.as_cellslice();
	}
};

using namespace block::tlb;

const CurrencyCollection tCC;
const OutMsg tOM;
const AccountBlock tAB;
const ImportFees tIF;
const InMsg tIM;
const EnqueuedMsg tEM;
const UInt tU64{64};
const ShardFeeCreated tSFC;




template <class TValue, class TExtra>
struct FullHashmapAug;


template <class TValue, class TExtra>
struct FullHashmapAugNode : BaseFullCell<block::gen::HashmapAugNode>
{
	

	int tag = -1;
	int n = -1;
	Ref<FullHashmapAug<TValue, TExtra>> left;
	Ref<FullHashmapAug<TValue, TExtra>> right;

	TValue value;
	TExtra extra;

	FullHashmapAugNode(int m,  const TLB &X, const TLB &Y) : BaseFullCell("HashmapAugNode", block::gen::HashmapAugNode(m, X, Y)) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = type.check_tag(cs);

		if (tag == 0) 
		{
			extra.unpack_std(ctx, cs, indent + 1);
			value.unpack_std(ctx, cs, indent + 1);
		}
		else
		{
			int n;
			add_r1(n, 1, type.m_);

			left = Ref<FullHashmapAug<TValue, TExtra>>(true, n, type.X_, type.Y_);
			left.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

			right = Ref<FullHashmapAug<TValue, TExtra>>(true, n, type.X_, type.Y_);
			right.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

			extra.unpack_std(ctx, cs, indent + 1);

		}
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {

		if (tag == 0) 
		{
			value.pack_opt(ctx, cb, indent + 1);
		}
		else {
			int n;
			CHECK(add_r1(n, 1, type.m_));
			cb.store_ref(left.write().make_opt_cell(ctx, indent + 1));
			cb.store_ref(right.write().make_opt_cell(ctx, indent + 1));
		}
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = type.check_tag(cs);

		if (tag == 0) 
		{
			value.unpack_opt(ctx, cs, indent + 1);
			auto extra_cs = value.calc_aug_data();
			extra.unpack_opt(ctx, extra_cs, indent + 1, true);
		}
		else
		{
			int n;
			add_r1(n, 1, type.m_);

			left = Ref<FullHashmapAug<TValue, TExtra>>(true, n, type.X_, type.Y_);
			left.write().cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);

			right = Ref<FullHashmapAug<TValue, TExtra>>(true, n, type.X_, type.Y_);
			right.write().cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);

			auto left_extra_cs = to_cs(left.write().node.extra.make_std_cell(ctx));
			auto right_extra_cs = to_cs(right.write().node.extra.make_std_cell(ctx));

			auto extra_cs = extra.add_values(left_extra_cs, right_extra_cs);
			extra.unpack_opt(ctx, extra_cs, indent + 1, true);
		}
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {

		if (tag == 0) 
		{
			extra.pack_std(ctx, cb, indent + 1);
			value.pack_std(ctx, cb, indent + 1);
		}
		else {
			int n;
			CHECK(add_r1(n, 1, type.m_));
			cb.store_ref(left.write().make_std_cell(ctx, indent + 1));
			cb.store_ref(right.write().make_std_cell(ctx, indent + 1));
			extra.pack_std(ctx, cb, indent + 1);
		}
	}
};

template <class TValue, class TExtra>
struct FullHashmapAug : BaseFullCell<block::gen::HashmapAug>, td::CntObject
{
	
	Ref<CellSlice> label;
	int n, m, l;

	FullHashmapAugNode<TValue, TExtra> node;

	FullHashmapAug(int n, const TLB &X, const TLB &Y) : BaseFullCell("HashmapAug", block::gen::HashmapAug(n, X, Y)), node(n, X, Y) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		n = type.m_;
		CHECK(block::gen::HmLabel{n}.fetch_to(cs, label, l));
		m = n - l;
		node.type.m_ = m;
		node.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		int l, m;
		CHECK(tlb::store_from(cb, HmLabel{type.m_}, label, l));
		CHECK(add_r1(m, l, type.m_));
		node.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(
			(n = type.m_) >= 0
      && block::gen::HmLabel{type.m_}.fetch_to(cs, label, l)
      && add_r1(m, l, type.m_)
		);
		node.type.m_ = m;
		node.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		int l, m;
		CHECK(
				tlb::store_from(cb, block::gen::HmLabel{type.m_}, label, l)
      && add_r1(m, l, type.m_)
		);
		node.pack_std(ctx, cb, indent + 1);
	}
};




template <class TValue, class TExtra>
struct FullHashmapAugE : BaseFullCell<block::gen::HashmapAugE>
{
	

	block::gen::HashmapAugE::Record_ahme_root r;

	int tag = -1;
	FullHashmapAug<TValue, TExtra> root;
	TExtra extra;

	FullHashmapAugE(int n, const TLB &X, const TLB &Y) : BaseFullCell("HashmapAugE", block::gen::HashmapAugE(n, X, Y)), root(n, X, Y) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		tag = type.check_tag(cs);

		if (tag == block::gen::HashmapAugE::ahme_empty)
		{
			CHECK(cs.fetch_ulong(1) == 0);
			extra.unpack_std(ctx, cs, indent + 1);
		}
		else 
		{
			CHECK(type.unpack(cs, r));

			root.cell_unpack_std(ctx, r.root, indent + 1);
			extra.unpack_std(ctx, r.extra.write(), indent + 1);
		}
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{

		if (tag == HashmapAugE::ahme_empty) {
 			cb.store_long(0, 1);
			extra.pack_opt(ctx, cb, indent + 1); 
		} else {
			cb.store_long(1, 1).store_ref(root.make_opt_cell(ctx, indent + 1));
		}
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		tag = type.check_tag(cs);

		if (tag == HashmapAugE::ahme_empty)
		{
			CHECK(cs.fetch_ulong(1) == 0);
			extra.unpack_opt(ctx, cs, indent + 1);
		}
		else
		{
			CHECK(cs.fetch_ulong(1) == 1 && (r.n = type.m_) >= 0);
			auto root_ref = cs.fetch_ref();
			root.cell_unpack_opt(ctx, root_ref, indent + 1);
			extra = root.node.extra;
		}
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {

		if (tag == HashmapAugE::ahme_empty) {
			cb.store_long(0, 1);
			extra.pack_std(ctx, cb, indent + 1);
		} else {
			cb.store_long(1, 1)
			.store_ref(root.make_std_cell(ctx, indent + 1));
			extra.pack_std(ctx, cb, indent + 1);
		} 
	}
};

































template <class T>
struct FullMaybe : BaseFullCell<TLB> {
	
	

	T value;
	int tag = -1;
	bool is_ref;

	FullMaybe(bool is_ref=false) : BaseFullCell("Maybe"), is_ref(is_ref) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = cs.fetch_ulong(1);
		if (tag) {
			if (is_ref) {
				value.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
			} else {
				value.unpack_std(ctx, cs, indent + 1);
			}
		}
	}

	virtual void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {

  	CHECK(cb.store_long_bool(tag, 1));
		if (tag) {
			if (is_ref) {
				cb.store_ref(value.make_opt_cell(ctx, indent + 1));
			} else {
				value.pack_opt(ctx, cb, indent + 1);
			}
		}
	}

	virtual void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = cs.fetch_ulong(1);
		if (tag) {
			if (is_ref) {
				value.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
			} else {
				value.unpack_opt(ctx, cs, indent + 1);
			}
		}
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {

  	CHECK(cb.store_long_bool(tag, 1));
		if (tag) {
			if (is_ref) {
				cb.store_ref(value.make_std_cell(ctx, indent + 1));
			} else {
				value.pack_std(ctx, cb, indent + 1);
			}
		}
	}
};

















































































































































struct FullImportFees;

struct FullInMsg : BaseFullCell<InMsg>, AugDataProvider {
    FullInMsg() : BaseFullCell("InMsg", InMsg()) {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(type.get_import_fees(cb, cs_copy));
		return cb.as_cellslice();
	}
};

struct FullCurrencyCollection : BaseFullCell<CurrencyCollection>, AddValues<CurrencyCollection>
{
	FullCurrencyCollection() : BaseFullCell("CurrencyCollection") {}
};

struct FullOutMsg : BaseFullCell<OutMsg>, AugDataProvider {
    FullOutMsg() : BaseFullCell("OutMsg", OutMsg()) {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(type.get_export_value(cb, cs_copy));
		return cb.as_cellslice();
	}
};




































































































































































template <class T>
struct FullMERKLE_UPDATE : BaseFullCell<TLB> {
	
	Ref<T> from_proof, to_proof;

	FullMERKLE_UPDATE(const T &type) : BaseFullCell("MERKLE_UPDATE") {

	}

	void do_unpack_std(ParseContext &ctx, CellSlice &cs, int indent = 0) override {
		CHECK(cs.advance(520)); 

		from_proof = Ref<T>(true);
		from_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

		to_proof = Ref<T>(true);
		to_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_ref(from_proof.write().make_std_cell(ctx, indent + 1));
		cb.store_ref(to_proof.write().make_std_cell(ctx, indent + 1));
		special_type = 0;
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		from_proof = Ref<T>(true);
		from_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

		to_proof = Ref<T>(true);
		to_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
	}

	Ref<Cell> make_std_cell(ParseContext& ctx, int indent = 0) {
	  return CellBuilder::create_merkle_update(
				from_proof.write().make_std_cell(ctx, indent + 1),
				to_proof.write().make_std_cell(ctx, indent + 1)
		);
	}
};









struct FullAccountBlock : BaseFullCell<AccountBlock>, AugDataProvider {
	FullAccountBlock() : BaseFullCell("AccountBlock") {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(Aug_ShardAccountBlocks().eval_leaf(cb, cs_copy));
		return cb.as_cellslice();
	}
};

struct FullShardAccountBlocks : BaseFullCell<block::gen::ShardAccountBlocks> {
	FullHashmapAugE<FullAccountBlock, FullCurrencyCollection> x{256, tAB, tCC};

	FullShardAccountBlocks() : BaseFullCell("ShardAccountBlocks") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		x.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		x.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_std(ctx, cb, indent + 1);
	}
};

















































































struct MyMcStateExtra : block::gen::McStateExtra {
  bool skip(vm::CellSlice& cs) const override {
		return cs.advance(16)
      && block::gen::ShardHashes().skip(cs)
      && cs.advance_ext(0x100, 2)
      && tCC.skip(cs);
	}
};

struct FullMcStateExtra : BaseFullCell<MyMcStateExtra> {
    FullMcStateExtra() : BaseFullCell("McStateExtra", MyMcStateExtra()) {}
};

const block::gen::ShardStateUnsplit_aux tSSUa;
const block::gen::RefT tRMSE{MyMcStateExtra()};
const block::gen::Maybe tMRMSE{tRMSE};

struct MyShardStateUnsplit : block::gen::ShardStateUnsplit {
	bool skip(vm::CellSlice& cs) const {
		return cs.advance_ext(0x169, 3)
				&& tMRMSE.skip(cs);
	}
};

































struct FullShardState : BaseFullCell<ShardState>, td::CntObject {
  

	
	
	

		FullShardState() : BaseFullCell("ShardState") {
			
		}

	
	
	
	
	
	
	
	
	
	

	
	
	

	
	

	
	
	
	
	
	
	
	
};

















































struct FullShardFeeCreated : BaseFullCell<ShardFeeCreated>, AugDataProvider, AddValues<ShardFeeCreated> {
    FullShardFeeCreated() : BaseFullCell("ShardFeeCreated") {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(Aug_ShardFees().eval_leaf(cb, cs_copy));
		return cb.as_cellslice();
	}
};

struct FullShardFees : BaseFullCell<block::gen::ShardFees> {
	FullHashmapAugE<FullShardFeeCreated, FullShardFeeCreated> x{96, tSFC, tSFC};

	FullShardFees() : BaseFullCell("ShardFees") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		x.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		x.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_std(ctx, cb, indent + 1);
	}
};










































































struct FullMcBlockExtra : BaseFullCell<block::gen::McBlockExtra> {
	
block::gen::McBlockExtra::Record record;

FullShardFees shard_fees;

FullMcBlockExtra() : BaseFullCell("McBlockExtra") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(cs.fetch_ulong(16) == 0xcca5);
		CHECK(cs.fetch_bool_to(record.key_block)); 
		CHECK(block::gen::t_ShardHashes.fetch_to(cs, record.shard_hashes));
		shard_fees.unpack_std(ctx, cs, indent + 1);
		fetch_remaining(cs);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(record.key_block, 1);
		block::gen::t_ShardHashes.store_from(cb, record.shard_hashes);
		shard_fees.pack_opt(ctx, cb, indent + 1);
		append_remaining(cb);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(cs.fetch_bool_to(record.key_block)); 
		CHECK(block::gen::t_ShardHashes.fetch_to(cs, record.shard_hashes));
		shard_fees.unpack_opt(ctx, cs, indent + 1);
		fetch_remaining(cs);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(0xcca5, 16).store_long(record.key_block, 1);
		block::gen::t_ShardHashes.store_from(cb, record.shard_hashes);
		shard_fees.pack_std(ctx, cb, indent + 1);
		append_remaining(cb);
	}
};


























































































































































































































































































struct FullImportFees;

struct FullOutMsgDescr : BaseFullCell<OutMsgDescr> {
	FullHashmapAugE<FullOutMsg, FullCurrencyCollection> x{256, tOM, tCC};

	FullOutMsgDescr() : BaseFullCell("OutMsgDescr") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		x.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		x.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_std(ctx, cb, indent + 1);
	}
};






struct FullImportFees : BaseFullCell<ImportFees>, AddValues<ImportFees>
{
	FullImportFees() : BaseFullCell("ImportFees", tIF), AddValues(tIF) {}
};

struct FullInMsgDescr : BaseFullCell<InMsgDescr>
{
	FullHashmapAugE<FullInMsg, FullImportFees> x{256, tIM, tIF};

	FullInMsgDescr(): BaseFullCell("InMsgDescr", InMsgDescr()) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		x.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		x.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		x.pack_std(ctx, cb, indent + 1);
	}
};

const block::gen::McBlockExtra tMBE{};
const block::gen::RefT tRMBE{tMBE};
const block::gen::Maybe tMRMBE(tRMBE);

struct FullBlockExtra : BaseFullCell<block::gen::BlockExtra>
{
	
	block::gen::BlockExtra::Record record;

	FullInMsgDescr in_msg_descr;
	FullOutMsgDescr out_msg_descr;
	FullShardAccountBlocks account_blocks;
	FullMaybe<FullMcBlockExtra> custom;

	FullBlockExtra(): BaseFullCell("BlockExtra"), custom(true) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK((cs.fetch_ulong(32) == 0x4a33f6fd));

		in_msg_descr.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		out_msg_descr.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		account_blocks.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		ccs = cs.fetch_subslice(512).write();
		custom.unpack_std(ctx, cs, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_ref(in_msg_descr.make_opt_cell(ctx, indent + 1))
				.store_ref(out_msg_descr.make_opt_cell(ctx, indent + 1))
				.store_ref(account_blocks.make_opt_cell(ctx, indent + 1))
				.append_cellslice(ccs);
		custom.pack_opt(ctx, cb, indent + 1);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		in_msg_descr.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		out_msg_descr.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		account_blocks.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		ccs = cs.fetch_subslice(512).write();
		custom.unpack_opt(ctx, cs, indent + 1);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(0x4a33f6fd, 32)
				.store_ref(in_msg_descr.make_std_cell(ctx, indent + 1))
				.store_ref(out_msg_descr.make_std_cell(ctx, indent + 1))
				.store_ref(account_blocks.make_std_cell(ctx, indent + 1))
				.append_cellslice(ccs);
		custom.pack_std(ctx, cb, indent + 1);
	}
};

struct FullBlock : BaseFullCell<block::gen::Block>
{
	block::gen::Block::Record record;

	FullMERKLE_UPDATE<FullShardState> state_update;
	FullBlockExtra extra;

	FullBlock(): BaseFullCell("Block"), state_update(FullShardState()) {

	}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		CHECK(type.unpack(cs, record));

		state_update.cell_unpack_std(ctx, record.state_update, indent + 1);
		extra.cell_unpack_std(ctx, record.extra, indent + 1, true);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{
		
		cb.store_long(record.global_id, 32)
				.store_ref(record.info)
				.store_ref(record.value_flow)
				.store_ref(state_update.make_opt_cell(ctx, indent + 1))
				.store_ref(extra.make_opt_cell(ctx, indent + 1));
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		CHECK(
			cs.fetch_int_to(32, record.global_id)
      && cs.fetch_ref_to(record.info)
      && cs.fetch_ref_to(record.value_flow)
		);
		state_update.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		extra.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1, true);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{
		cb.store_long(0x11ef55aa, 32)
				.store_long(record.global_id, 32)
				.store_ref(record.info)
				.store_ref(record.value_flow)
				.store_ref(state_update.make_std_cell(ctx, indent + 1))
				.store_ref(extra.make_std_cell(ctx, indent + 1));
	}
};



class NullStream : public ostream {
    class NullBuffer : public streambuf {
    public:
        int overflow( int c ) { return c; }
    } m_nb;
public:
    NullStream() : ostream( &m_nb ) {}
};




#define Y else
#define O if
#define Q for

using I=int;

using namespace std;







namespace LQ {


typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;


void E(const char* msg=0) {

  throw runtime_error(msg);
}





struct Reader {
  virtual I get() = 0;  
  virtual I read(char* buf, I n); 
  virtual ~Reader() {}
};

struct Writer {
  virtual void put(I c) = 0;  
  virtual void write(const char* buf, I n);  
  virtual ~Writer() {}
};


I toU16(const char* p);







template <class T>
class Array {
  T *data;     
  size_t n;    
  I offset;  
  void operator=(const Array&);  
  Array(const Array&);  
public:
  Array(size_t sz=0, I ex=0): data(0), n(0), offset(0) {
    resize(sz, ex);} 
  void resize(size_t sz, I ex=0); 
  ~Array() {resize(0);}  
  size_t size() const {return n;}  
  I isize() const {return I(n);}  
  T& operator[](size_t i) {
    
    return data[i];}
  T& operator()(size_t i) {
    
    return data[i&(n-1)];}
};


template<class T>
void Array<T>::resize(size_t sz, I ex) {

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
  O (!data) n=0, E(); 
  offset=64-(((char*)data-(char*)0)&63);

  data=(T*)((char*)data+offset);
}


typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE} CompType;

class Decoder;  


class ZP {
public:
  ZP();
  ~ZP();
  void clear();           
  void inith();           
  void initp();           
  double memory();        
  void run(U32 input);    
  I read(Reader* in2);  
  bool write(Writer* out2, bool pp); 
  I step(U32 input, I mode);  

  Writer* output;         

  U32 H(I i) {return h(i);}  

  void flush();           
  void outc(I ch) {     
    O (ch<0 || (outbuf[bufptr]=ch, ++bufptr==outbuf.isize())) flush();
  }

  
  Array<U8> D;   
  I cend;           
  I hbegin, hend;   

private:
  
  Array<U8> m;        
  Array<U32> h, r;       

  Array<char> outbuf; 
  I bufptr;         
  U32 a, b, c, d;     
  I f, pc, rcode_size;              


  U8* rcode;          

  
  void init(I hbits, I mbits);  
  I execute();  

  void div(U32 x) {O (x) a/=x; Y a=0;}
  void mod(U32 x) {O (x) a%=x; Y a=0;}
  void swap(U32& x) {a^=x; x^=a; a^=x;}
  void swap(U8& x)  {a^=x; x^=a; a^=x;}
  void err() { E("ZP execution E");}
};








struct Component {
  size_t z;   
  size_t cxt;     
  size_t a, b, c; 
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




class StateTable {
public:
  U8 ns[1024]; 
  I next(I s, I y) {  
    
    
    return ns[s*4+y];
  }
  I cminit(I s) {  
    
    return ((ns[s*4+3]*2+1)<<22)/(ns[s*4+2]+ns[s*4+3]+1);
  }
  StateTable();
};




class P {
public:
  P(ZP&);
  ~P();
  void init();          
  I predict();        
  void update(I y);   
  I stat(I);        
  bool isModeled() {    
    
    return z.D[6]!=0;
  }
private:

  
  I c8;               
  I hmap4;            
  I p[256];           
  U32 h[256];           
  ZP& z;             
  Component comp[256];  
  bool initTables;      

  
  I predict0();       
  void update0(I y);  
  I dt2k[256];        
  I dt[1024];         
  U16 squasht[4096];    
  short stretcht[32768];
  StateTable st;        
  U8* pcode;            
  I pcode_size;       

  
  void train(Component& cr, I y) {
    
    U32& pn=cr.cm(cr.cxt);
    U32 count=pn&0x3ff;
    I E=y*32767-(cr.cm(cr.cxt)>>17);
    pn+=(E*dt[count]&-1024)+(count<cr.z);
  }

  
  I squash(I x) {
    
    
    return squasht[x+2048];
  }

  
  I stretch(I x) {
    
    
    return stretcht[x];
  }

  
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


  
  I assemble_p();
};




class Decoder: public Reader {
public:
  Reader* in;        
  Decoder(ZP& z);
  I decompress();  
  I skip();        
  void init();       
  I stat(I x) {return pr.stat(x);}
  I get() {        
    O (rpos==wpos) {
      rpos=0;
      wpos=in ? in->read(&buf[0], BUFSIZE) : 0;
    
    }
    return rpos<wpos ? U8(buf[rpos++]) : -1;
  }
  I buffered() {return wpos-rpos;}  
private:
  U32 low, high;     
  U32 curr;          
  U32 rpos, wpos;    
  P pr;      
  enum {BUFSIZE=1<<16};
  Array<char> buf;   
  I decode(I p); 
};



class PostProcessor {
  I state;   
  I hsize;   
  I ph, pm;  
public:
  ZP z;     
  PostProcessor(): state(0), hsize(0), ph(0), pm(0) {}
  void init(I h, I m);  
  I write(I c);  
  I getState() const {return state;}
  void setOutput(Writer* out) {z.output=out;}

};




class D {
public:
  D(): z(), dec(z), pp(), state(BLOCK), decode_state(FIRSTSEG) {}
  void setInput(Reader* in) {dec.in=in;}
  bool findBlock(double* memptr = 0);

  bool findFilename(Writer* = 0);
  void readComment(Writer* = 0);
  void setOutput(Writer* out) {pp.setOutput(out);}

  bool decompress(I n = -1);  

  void readSegmentEnd(char* sha1string = 0);
  I stat(I x) {return dec.stat(x);}
  I buffered() {return dec.buffered();}
private:
  ZP z;
  Decoder dec;
  PostProcessor pp;
  enum {BLOCK, FILENAME, COMMENT, DATA, SEGEND} state;  
  enum {FIRSTSEG, SEG, SKIP} decode_state;  
};



void decompress(Reader* in, Writer* out);




class Encoder {
public:
  Encoder(ZP& z, I size=0):
    out(0), low(1), high(0xFFFFFFFF), pr(z) {}
  void init();
  void compress(I c);  
  I stat(I x) {return pr.stat(x);}
  Writer* out;  
private:
  U32 low, high; 
  P pr;  
  Array<char> buf; 
  void encode(I y, I p); 
};






class Compiler {
public:
  Compiler(const char* in, I* args, ZP& hz, ZP& pz, Writer* out2);
private:
  const char* in;  
  I* args;       
  ZP& hz;       
  ZP& pz;       
  Writer* out2;    
  I line;        
  I state;       

  
  typedef enum {NONE,CONS,CM,ICM,MATCH,AVG,MIX2,MIX,ISSE,SSE,
    JT=39,JF=47,JMP=63,LJ=255,
    POST=256,PCOMP,END,IF,IFNOT,ELSE,ENDIF,DO,
    WHILE,UNTIL,QEVER,IFL,IFNOTL,ELSEL,SEMICOLON} CompType;

  void SE(const char* msg, const char* expected=0); 
  void next();                     
  bool matchToken(const char* tok);
  I rtoken(I low, I high);   
  I rtoken(const char* list[]);  
  void rtoken(const char* s);      
  I compile_comp(ZP& z);      

  
  class Stack {
    LQ::Array<U16> s;
    size_t top;
  public:
    Stack(I n): s(n), top(0) {}
    void push(const U16& x) {
      O (top>=s.size()) E();
      s[top++]=x;
    }
    U16 pop() {
      O (top<=0) E();
      return s[--top];
    }
  };

  Stack if_stack, do_stack;
};



class C {
public:
  C(): enc(z), in(0), state(INIT) {}
  void setOutput(Writer* out) {enc.out=out;}

  void startBlock(const char* config,     
                  I* args,              
                  Writer* pcomp_cmd = 0); 
  void startSegment(const char* filename = 0, const char* comment = 0);
  void setInput(Reader* i) {in=i;}
  void postProcess(const char* pcomp = 0, I len = 0);  
  bool compress(I n = -1);  
  void endSegment(const char* sha1string = 0);

  I stat(I x) {return enc.stat(x);}
private:
  ZP z, pz;  
  Encoder enc;  
  Reader* in;   


  enum {INIT, BLOCK1, SEG1, BLOCK2, SEG2} state;

};





class SB: public LQ::Reader, public LQ::Writer {
  unsigned char* p;  
  size_t al;         
  size_t wpos;       
  size_t rpos;       
  size_t limit;      
  const size_t init; 

  
  void reserve(size_t a) {
    
    O (a<=al) return;
    unsigned char* q=0;
    O (a>0) q=(unsigned char*)(p ? realloc(p, a) : malloc(a));
    O (a>0 && !q) E(); 
    p=q;
    al=a;
  }

  
  void lengthen(size_t n) {
    
    O (wpos+n>limit || wpos+n<wpos) E("SB overflow");
    O (wpos+n<=al) return;
    size_t a=al;
    while (wpos+n>=a) a=a*2+init;
    reserve(a);
  }

  
  void operator=(const SB&);
  SB(const SB&);

public:

  
  unsigned char* data() {
    
    return p;}

  
  SB(size_t n=0):
      p(0), al(0), wpos(0), rpos(0), limit(size_t(-1)), init(n>128?n:128) {}

  
  ~SB() {O (p) free(p);}

  
  size_t size() const {return wpos;}

  
  void put(I c) {  
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






void compress(Reader* in, Writer* out, const char* ME,
     const char* filename=0, const char* comment=0, bool dosha1=true);


void compressBlock(SB* in, Writer* out, const char* ME,
     const char* filename=0, const char* comment=0, bool dosha1=true);



I toU16(const char* p) {
  return (p[0]&255)+256*(p[1]&255);
}


I Reader::read(char* buf, I n) {
  I i=0, c;
  while (i<n && (c=get())>=0)
    buf[i++]=c;
  return i;
}

void Writer::write(const char* buf, I n) {
  Q (I i=0; i<n; ++i)
    put(U8(buf[i]));
}

void allocx(U8* &p, I &n, I newsize) {
  p=0;
  n=0;
}









const I compsize[256]={0,2,3,2,3,4,6,6,3,5};











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


StateTable::StateTable() {
  memcpy(ns, sns, sizeof(ns));
}





bool ZP::write(Writer* out2, bool pp) {
  O (D.size()<=6) return false;
  O (!pp) {  
    Q (I i=0; i<cend; ++i)
      out2->put(D[i]);
  }
  Y {  
    out2->put((hend-hbegin)&255);
    out2->put((hend-hbegin)>>8);
  }
  Q (I i=hbegin; i<hend; ++i)
    out2->put(D[i]);
  return true;
}


I ZP::read(Reader* in2) {

  
  I hsize=in2->get();
  hsize+=in2->get()*256;
  D.resize(hsize+300);
  cend=hbegin=hend=0;
  D[cend++]=hsize&255;
  D[cend++]=hsize>>8;
  while (cend<7) D[cend++]=in2->get(); 

  
  I n=D[cend-1];
  Q (I i=0; i<n; ++i) {
    I type=in2->get();  
    O (type<0 || type>255) E("unexpected end of file");
    D[cend++]=type;  
    I size=compsize[type];
    O (size<1) E("Invalid component type");
    O (cend+size>hsize) E("COMP overflows D");
    Q (I j=1; j<size; ++j)
      D[cend++]=in2->get();
  }
  O ((D[cend++]=in2->get())!=0) E("missing COMP END");

  
  hbegin=hend=cend+128;
  O (hend>hsize+129) E("missing HCOMP");
  while (hend<hsize+129) {
    
    I op=in2->get();
    O (op==-1) E("unexpected end of file");
    D[hend++]=op;
  }
  O ((D[hend++]=in2->get())!=0) E("missing HCOMP END");
  allocx(rcode, rcode_size, 0);  
  return cend+hend-hbegin;
}


void ZP::clear() {
  cend=hbegin=hend=0;  
  a=b=c=d=f=pc=0;      
  D.resize(0);
  h.resize(0);
  m.resize(0);
  r.resize(0);
  allocx(rcode, rcode_size, 0);
}


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


void ZP::inith() {
  init(D[2], D[3]); 
}


void ZP::initp() {
  init(D[4], D[5]); 
}


void ZP::flush() {
  O (output) output->write(&outbuf[0], bufptr);
  bufptr=0;
}


static double pow2(I x) {
  double r=1;
  Q (; x>0; x--) r+=r;
  return r;
}


double ZP::memory() {
  double mem=pow2(D[2]+2)+pow2(D[3])  
            +pow2(D[4]+2)+pow2(D[5])  
            +D.size();
  I cp=7;  
  Q (I i=0; i<D[6]; ++i) {  
    
    double size=pow2(D[cp+1]); 
    switch(D[cp]) {
      case CM: mem+=4*size; break;
      case ICM: mem+=64*size+1024; break;
      case MATCH: mem+=4*size+pow2(D[cp+2]); break; 
      case MIX2: mem+=2*size; break;
      case MIX: mem+=4*size*D[cp+3]; break; 
      case ISSE: mem+=64*size+2048; break;
      case SSE: mem+=128*size; break;
    }
    cp+=compsize[D[cp]];
  }
  return mem;
}


void ZP::init(I hbits, I mbits) {
  O (hbits>32) E("H too big");
  O (mbits>32) E("M too big");
  h.resize(1, hbits);
  m.resize(1, mbits);
  r.resize(256);
  a=b=c=d=pc=f=0;
}









I ZP::execute() {
  switch(D[pc++]) {
    case 0:err(); break; 
    case 1:++a; break; 
    case 2:--a; break; 
    case 3:a = ~a; break; 
    case 4:a = 0; break; 
    case 7:a = r[D[pc++]]; break; 
    case 8:swap(b); break; 
    case 9:++b; break; 
    case 10:--b; break; 
    case 11:b = ~b; break; 
    case 12:b = 0; break; 
    case 15:b = r[D[pc++]]; break; 
    case 16:swap(c); break; 
    case 17:++c; break; 
    case 18:--c; break; 
    case 19:c = ~c; break; 
    case 20:c = 0; break; 
    case 23:c = r[D[pc++]]; break; 
    case 24:swap(d); break; 
    case 25:++d; break; 
    case 26:--d; break; 
    case 27:d = ~d; break; 
    case 28:d = 0; break; 
    case 31:d = r[D[pc++]]; break; 
    case 32:swap(m(b)); break; 
    case 33:++m(b); break; 
    case 34:--m(b); break; 
    case 35:m(b) = ~m(b); break; 
    case 36:m(b) = 0; break; 
    case 39:O (f) pc+=((D[pc]+128)&255)-127; Y ++pc; break; 
    case 40:swap(m(c)); break; 
    case 41:++m(c); break; 
    case 42:--m(c); break; 
    case 43:m(c) = ~m(c); break; 
    case 44:m(c) = 0; break; 
    case 47:O (!f) pc+=((D[pc]+128)&255)-127; Y ++pc; break; 
    case 48:swap(h(d)); break; 
    case 49:++h(d); break; 
    case 50:--h(d); break; 
    case 51:h(d) = ~h(d); break; 
    case 52:h(d) = 0; break; 
    case 55:r[D[pc++]] = a; break; 
    case 56:return 0  ; 
    case 57:outc(a&255); break; 
    case 59:a = (a+m(b)+512)*773; break; 
    case 60:h(d) = (h(d)+a+512)*773; break; 
    case 63:pc+=((D[pc]+128)&255)-127; break; 
    case 64:break; 
    case 65:a = b; break; 
    case 66:a = c; break; 
    case 67:a = d; break; 
    case 68:a = m(b); break; 
    case 69:a = m(c); break; 
    case 70:a = h(d); break; 
    case 71:a = D[pc++]; break; 
    case 72:b = a; break; 
    case 73:break; 
    case 74:b=c; break; 
    case 75:b=d; break; 
    case 76:b=m(b); break; 
    case 77:b=m(c); break; 
    case 78:b=h(d); break; 
    case 79:b=D[pc++]; break; 
    case 80:c=a; break; 
    case 81:c=b; break; 
    case 82:break; 
    case 83:c=d; break; 
    case 84:c=m(b); break; 
    case 85:c=m(c); break; 
    case 86:c=h(d); break; 
    case 87:c=D[pc++]; break; 
    case 88:d=a; break; 
    case 89:d=b; break; 
    case 90:d=c; break; 
    case 91:break; 
    case 92:d=m(b); break; 
    case 93:d=m(c); break; 
    case 94:d=h(d); break; 
    case 95:d=D[pc++]; break; 
    case 96:m(b)=a; break; 
    case 97:m(b)=b; break; 
    case 98:m(b)=c; break; 
    case 99:m(b)=d; break; 
    case 100:break; 
    case 101:m(b)=m(c); break; 
    case 102:m(b)=h(d); break; 
    case 103:m(b)=D[pc++]; break; 
    case 104:m(c)=a; break; 
    case 105:m(c)=b; break; 
    case 106:m(c)=c; break; 
    case 107:m(c)=d; break; 
    case 108:m(c)=m(b); break; 
    case 109:break; 
    case 110:m(c)=h(d); break; 
    case 111:m(c)=D[pc++]; break; 
    case 112:h(d)=a; break; 
    case 113:h(d)=b; break; 
    case 114:h(d)=c; break; 
    case 115:h(d)=d; break; 
    case 116:h(d)=m(b); break; 
    case 117:h(d)=m(c); break; 
    case 118:break; 
    case 119:h(d)=D[pc++]; break; 
    case 128:a+=a; break; 
    case 129:a+=b; break; 
    case 130:a+=c; break; 
    case 131:a+=d; break; 
    case 132:a+=m(b); break; 
    case 133:a+=m(c); break; 
    case 134:a+=h(d); break; 
    case 135:a+=D[pc++]; break; 
    case 136:a-=a; break; 
    case 137:a-=b; break; 
    case 138:a-=c; break; 
    case 139:a-=d; break; 
    case 140:a-=m(b); break; 
    case 141:a-=m(c); break; 
    case 142:a-=h(d); break; 
    case 143:a-=D[pc++]; break; 
    case 144:a*=a; break; 
    case 145:a*=b; break; 
    case 146:a*=c; break; 
    case 147:a*=d; break; 
    case 148:a*=m(b); break; 
    case 149:a*=m(c); break; 
    case 150:a*=h(d); break; 
    case 151:a*=D[pc++]; break; 
    case 152:div(a); break; 
    case 153:div(b); break; 
    case 154:div(c); break; 
    case 155:div(d); break; 
    case 156:div(m(b)); break; 
    case 157:div(m(c)); break; 
    case 158:div(h(d)); break; 
    case 159:div(D[pc++]); break; 
    case 160:mod(a); break; 
    case 161:mod(b); break; 
    case 162:mod(c); break; 
    case 163:mod(d); break; 
    case 164:mod(m(b)); break; 
    case 165:mod(m(c)); break; 
    case 166:mod(h(d)); break; 
    case 167:mod(D[pc++]); break; 
    case 168:a&=a; break; 
    case 169:a&=b; break; 
    case 170:a&=c; break; 
    case 171:a&=d; break; 
    case 172:a&=m(b); break; 
    case 173:a&=m(c); break; 
    case 174:a&=h(d); break; 
    case 175:a&=D[pc++]; break; 
    case 176:a&=~a; break; 
    case 177:a&=~b; break; 
    case 178:a&=~c; break; 
    case 179:a&=~d; break; 
    case 180:a&=~m(b); break; 
    case 181:a&=~m(c); break; 
    case 182:a&=~h(d); break; 
    case 183:a&=~D[pc++]; break; 
    case 184:a|=a; break; 
    case 185:a|=b; break; 
    case 186:a|=c; break; 
    case 187:a|=d; break; 
    case 188:a|=m(b); break; 
    case 189:a|=m(c); break; 
    case 190:a|=h(d); break; 
    case 191:a|=D[pc++]; break; 
    case 192:a^=a; break; 
    case 193:a^=b; break; 
    case 194:a^=c; break; 
    case 195:a^=d; break; 
    case 196:a^=m(b); break; 
    case 197:a^=m(c); break; 
    case 198:a^=h(d); break; 
    case 199:a^=D[pc++]; break; 
    case 200:a<<=(a&31); break; 
    case 201:a<<=(b&31); break; 
    case 202:a<<=(c&31); break; 
    case 203:a<<=(d&31); break; 
    case 204:a<<=(m(b)&31); break; 
    case 205:a<<=(m(c)&31); break; 
    case 206:a<<=(h(d)&31); break; 
    case 207:a<<=(D[pc++]&31); break; 
    case 208:a>>=(a&31); break; 
    case 209:a>>=(b&31); break; 
    case 210:a>>=(c&31); break; 
    case 211:a>>=(d&31); break; 
    case 212:a>>=(m(b)&31); break; 
    case 213:a>>=(m(c)&31); break; 
    case 214:a>>=(h(d)&31); break; 
    case 215:a>>=(D[pc++]&31); break; 
    case 216:f=1; break; 
    case 217:f=(a==b); break; 
    case 218:f=(a==c); break; 
    case 219:f=(a==d); break; 
    case 220:f=(a==U32(m(b))); break; 
    case 221:f=(a==U32(m(c))); break; 
    case 222:f=(a==h(d)); break; 
    case 223:f=(a==U32(D[pc++])); break; 
    case 224:f=0;break; 
    case 225:f=(a<b); break; 
    case 226:f=(a<c); break; 
    case 227:f=(a<d); break; 
    case 228:f=(a<U32(m(b))); break; 
    case 229:f=(a<U32(m(c))); break; 
    case 230:f=(a<h(d)); break; 
    case 231:f=(a<U32(D[pc++])); break; 
    case 232:f=0;break; 
    case 233:f=(a>b); break; 
    case 234:f=(a>c); break; 
    case 235:f=(a>d); break; 
    case 236:f=(a>U32(m(b))); break; 
    case 237:f=(a>U32(m(c))); break; 
    case 238:f=(a>h(d)); break; 
    case 239:f=(a>U32(D[pc++])); break; 
    case 255:O((pc=hbegin+D[pc]+256*D[pc+1])>=hend)err();break;
    default: err();
  }
  return 1;
}






























































































































































































































































































































































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

P::P(ZP& zr):
    c8(1), hmap4(1), z(zr) {
  pcode=0;
  pcode_size=0;
  initTables=false;
}

P::~P() {
  allocx(pcode, pcode_size, 0);  
}


void P::init() {

  
  allocx(pcode, pcode_size, 0);

  
  z.inith();

  
  O (!initTables && isModeled()) {
    initTables=true;
    

    dt2k[0] = 0;
    Q (I i = 1; i < 256; ++i) {
        dt2k[i]=2048/i;
        
        
        
    }

    
    Q (I i = 0; i < 1024; ++i) {
        dt[i]=(1<<17)/(i*2+3)*2;
        
        
        
    }

    
    
    memset(squasht, 0, (1376 + 7)*2);

    Q (I i=1376 + 7; i < 1376 + 1344; ++i) {
        
        squasht[i]=static_cast<U16>(32768.0 / (1 + std::exp((i - 2048) * (-1.0 / 64))));
        
        
        
    }
    
    Q (I i=2720; i<4096; ++i) squasht[i]=32767;

    
    I k=16384;
    Q (I i=0; i<712; ++i)
      Q (I j=stdt[i]; j>0; --j)
        stretcht[k++]=i;

    Q (I i=0; i<16384; ++i)
      stretcht[i]=-stretcht[32767-i];

  }

  
  Q (I i=0; i<256; ++i) h[i]=p[i]=0;

  
  Q (I i=0; i<256; ++i)  
    comp[i].init();
  I n=z.D[6]; 
  const U8* cp=&z.D[7];  
  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  
        p[i]=(cp[1]-128)*4;
        break;
      case CM: 
        O (cp[1]>32) E("max size Q CM is 32");
        cr.cm.resize(1, cp[1]);  
        cr.z=cp[2]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=0x80000000;
        break;
      case ICM: 
        O (cp[1]>26) E("max size Q ICM is 26");
        cr.z=1023;
        cr.cm.resize(256);
        cr.ht.resize(64, cp[1]);
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=st.cminit(j);
        break;
      case MATCH:  
        O (cp[1]>32 || cp[2]>32) E("max size Q MATCH is 32 32");
        cr.cm.resize(1, cp[1]);  
        cr.ht.resize(1, cp[2]);  
        cr.ht(0)=1;
        break;
      case AVG: 
        O (cp[1]>=i) E("AVG j >= i");
        O (cp[2]>=i) E("AVG k >= i");
        break;
      case MIX2:  
        O (cp[1]>32) E("max size Q MIX2 is 32");
        O (cp[3]>=i) E("MIX2 k >= i");
        O (cp[2]>=i) E("MIX2 j >= i");
        cr.c=(size_t(1)<<cp[1]); 
        cr.a16.resize(1, cp[1]);  
        Q (size_t j=0; j<cr.a16.size(); ++j)
          cr.a16[j]=32768;
        break;
      case MIX: {  
        O (cp[1]>32) E("max size Q MIX is 32");
        O (cp[2]>=i) E("MIX j >= i");
        O (cp[3]<1 || cp[3]>i-cp[2]) E("MIX m not in 1..i-j");
        I m=cp[3];  
        
        cr.c=(size_t(1)<<cp[1]); 
        cr.cm.resize(m, cp[1]);  
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=65536/m;
        break;
      }
      case ISSE:  
        O (cp[1]>32) E("max size Q ISSE is 32");
        O (cp[2]>=i) E("ISSE j >= i");
        cr.ht.resize(64, cp[1]);
        cr.cm.resize(512);
        Q (I j=0; j<256; ++j) {
          cr.cm[j*2]=1<<15;
          cr.cm[j*2+1]=clamp512k(stretch(st.cminit(j)>>8)*1024);
        }
        break;
      case SSE: 
        O (cp[1]>32) E("max size Q SSE is 32");
        O (cp[2]>=i) E("SSE j >= i");
        O (cp[3]>cp[4]*4) E("SSE start > limit*4");
        cr.cm.resize(32, cp[1]);
        cr.z=cp[4]*4;
        Q (size_t j=0; j<cr.cm.size(); ++j)
          cr.cm[j]=squash((j&31)*64-992)<<17|cp[3];
        break;
      default: E("unknown component type");
    }
    
    cp+=compsize[*cp];
    
  }
}


I P::predict0() {

  
  I n=z.D[6];
  const U8* cp=&z.D[7];
  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  
        break;
      case CM:  
        cr.cxt=h[i]^hmap4;
        p[i]=stretch(cr.cm(cr.cxt)>>17);
        break;
      case ICM: 
        
        O (c8==1 || (c8&0xf0)==16) cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];
        p[i]=stretch(cr.cm(cr.cxt)>>8);
        break;
      case MATCH: 
                  
        O (cr.a==0) p[i]=0;
        Y {
          cr.c=(cr.ht(cr.z-cr.b)>>(7-cr.cxt))&1; 
          p[i]=stretch(dt2k[cr.a]*(cr.c*-2+1)&32767);
        }
        break;
      case AVG: 
        p[i]=(p[cp[1]]*cp[3]+p[cp[2]]*(256-cp[3]))>>8;
        break;
      case MIX2: { 
                   
        cr.cxt=((h[i]+(c8&cp[5]))&(cr.c-1));
        
        I w=cr.a16[cr.cxt];
        
        p[i]=(w*p[cp[2]]+(65536-w)*p[cp[3]])>>16;
        
      }
        break;
      case MIX: {  
                   
        I m=cp[3];
        
        cr.cxt=h[i]+(c8&cp[5]);
        cr.cxt=(cr.cxt&(cr.c-1))*m; 
        
        I* wt=(I*)&cr.cm[cr.cxt];
        p[i]=0;
        Q (I j=0; j<m; ++j)
          p[i]+=(wt[j]>>8)*p[cp[2]+j];
        p[i]=clamp2k(p[i]>>8);
      }
        break;
      case ISSE: { 
        
        O (c8==1 || (c8&0xf0)==16)
          cr.c=find(cr.ht, cp[1]+2, h[i]+16*c8);
        cr.cxt=cr.ht[cr.c+(hmap4&15)];  
        I *wt=(I*)&cr.cm[cr.cxt*2];
        p[i]=clamp2k((wt[0]*p[cp[2]]+wt[1]*64)>>16);
      }
        break;
      case SSE: { 
        cr.cxt=(h[i]+c8)*32;
        I pq=p[cp[2]]+992;
        O (pq<0) pq=0;
        O (pq>1983) pq=1983;
        I wt=pq&63;
        pq>>=6;
        
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


void P::update0(I y) {
  
  const U8* cp=&z.D[7];
  I n=z.D[6];


  Q (I i=0; i<n; ++i) {
    Component& cr=comp[i];
    switch(cp[0]) {
      case CONS:  
        break;
      case CM:  
        train(cr, y);
        break;
      case ICM: { 
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.ht[cr.c+(hmap4&15)], y);
        U32& pn=cr.cm(cr.cxt);
        pn+=I(y*32767-(pn>>8))>>2;
      }
        break;
      case MATCH: 
                  
                  
      {
        
        
        
        
        
        
        O (I(cr.c)!=y) cr.a=0;  
        cr.ht(cr.z)+=cr.ht(cr.z)+y;
        O (++cr.cxt==8) {
          cr.cxt=0;
          ++cr.z;
          cr.z&=(1<<cp[2])-1;
          O (cr.a==0) {  
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
      case AVG:  
        break;
      case MIX2: { 
                   
        
        
        I err=(y*32767-squash(p[i]))*cp[4]>>5;
        I w=cr.a16[cr.cxt];
        w+=(err*(p[cp[2]]-p[cp[3]])+(1<<12))>>13;
        O (w<0) w=0;
        O (w>65535) w=65535;
        cr.a16[cr.cxt]=w;
      }
        break;
      case MIX: {   
                    
        I m=cp[3];
        
        
        
        I err=(y*32767-squash(p[i]))*cp[4]>>4;
        I* wt=(I*)&cr.cm[cr.cxt];
        Q (I j=0; j<m; ++j)
          wt[j]=clamp512k(wt[j]+((err*p[cp[2]+j]+(1<<12))>>13));
      }
        break;
      case ISSE: { 
        
        I err=y*32767-squash(p[i]);
        I *wt=(I*)&cr.cm[cr.cxt*2];
        wt[0]=clamp512k(wt[0]+((err*p[cp[2]]+(1<<12))>>13));
        wt[1]=clamp512k(wt[1]+((err+16)>>5));
        cr.ht[cr.c+(hmap4&15)]=st.next(cr.cxt, y);
      }
        break;
      case SSE:  
        train(cr, y);
        break;
      default:
        assert(0);
    }
    cp+=compsize[cp[0]];
    
    
  }


  
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



Decoder::Decoder(ZP& z):
    in(0), low(1), high(0xFFFFFFFF), curr(0), rpos(0), wpos(0),
    pr(z), buf(BUFSIZE) {
}

void Decoder::init() {
  pr.init();
  O (pr.isModeled()) low=1, high=0xFFFFFFFF, curr=0;
  Y low=high=curr=0;
}


I Decoder::decode(I p) {
  O (curr<low || curr>high) E("archive corrupted");
  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  
  I y;
  O (curr<=mid) y=1, high=mid;  
  Y y=0, low=mid+1;
  while ((high^low)<0x1000000) { 
    high=high<<8|255;
    low=low<<8;
    low+=(low==0);
    I c=get();
    O (c<0) E("unexpected end of file");
    curr=curr<<8|c;
  }
  return y;
}


I Decoder::decompress() {
  O (pr.isModeled()) {  
    O (curr==0) {  
      Q (I i=0; i<4; ++i)
        curr=curr<<8|get();
    }
    O (decode(0)) {
      O (curr!=0) E("decoding end of stream");
      return -1;
    }
    Y {
      I c=1;
      while (c<256) {  
        I p=pr.predict()*2+1;
        c+=c+decode(p);
        pr.update(c&1);
      }
      return c-256;
    }
  }
  Y {
    O (curr==0) {
      Q (I i=0; i<4; ++i) curr=curr<<8|get();
      O (curr==0) return -1;
    }
    --curr;
    return get();
  }
}


I Decoder::skip() {
  I c=-1;
  O (pr.isModeled()) {
    while (curr==0)  
      curr=get();
    while (curr && (c=get())>=0)  
      curr=curr<<8|c;
    while ((c=get())==0) ;  
    return c;
  }
  Y {
    O (curr==0)  
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




void PostProcessor::init(I h, I m) {
  state=hsize=0;
  ph=h;
  pm=m;
  z.clear();
}



I PostProcessor::write(I c) {

  switch (state) {
    case 0:  
      O (c<0) E("Unexpected EOS");
      state=c+1;  
      O (state>2) E("unknown post processing type");
      O (state==1) z.clear();
      break;
    case 1:  
      z.outc(c);
      break;
    case 2: 
      O (c<0) E("Unexpected EOS");
      hsize=c;  
      state=3;
      break;
    case 3:  
      O (c<0) E("Unexpected EOS");
      hsize+=c*256;  
      O (hsize<1) E("Empty PCOMP");
      z.D.resize(hsize+300);
      z.cend=8;
      z.hbegin=z.hend=z.cend+128;
      z.D[4]=ph;
      z.D[5]=pm;
      state=4;
      break;
    case 4:  
      O (c<0) E("Unexpected EOS");
    
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





bool D::findBlock(double* memptr) {

  I c = dec.get();

  
  O (c!=1 && c!=2) E("unsupported ZPAQ level");
  z.read(&dec);
  O (c==1 && z.D.isize()>6 && z.D[6]==0)
    E("ZPAQ level 1 requires at least 1 component");
  O (memptr) *memptr=z.memory();
  state=FILENAME;
  decode_state=FIRSTSEG;
  return true;
}



bool D::findFilename(Writer* filename) {
    state=COMMENT;
    return true;
}


void D::readComment(Writer* comment) {
  state=DATA;
}


bool D::decompress(I n) {

  O (decode_state==SKIP) E("decompression after skipped segment");


  
  O (decode_state==FIRSTSEG) {
    dec.init();
    
    pp.init(z.D[4], z.D[5]);
    decode_state=SEG;
  }

  
  while ((pp.getState()&3)!=1)
    pp.write(dec.decompress());

  
  while (n) {
    I c=dec.decompress();
    pp.write(c);
    O (c==-1) {
      state=SEGEND;
      return false;
    }
    O (n>0) --n;
  }
  return true;
}




void D::readSegmentEnd(char* sha1string) {


  
  I c=0;
  O (state==DATA) {
    c=dec.skip();
    decode_state=SKIP;
  }
  Y O (state==SEGEND)
    c=dec.get();
  state=FILENAME;

  
  O (c==254) {
    O (sha1string) sha1string[0]=0;  
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



void decompress(Reader* in, Writer* out) {
  D d;
  d.setInput(in);
  d.setOutput(out);
  if (d.findBlock()) {       
      d.decompress();           
    }
}




void Encoder::init() {
  low=1;
  high=0xFFFFFFFF;
  pr.init();
  O (!pr.isModeled()) low=0, buf.resize(1<<16);
}


void Encoder::encode(I y, I p) {




  U32 mid=low+U32(((high-low)*U64(U32(p)))>>16);  

  O (y) high=mid; Y low=mid+1; 
  while ((high^low)<0x1000000) { 
    out->put(high>>24);  
    high=high<<8|255;
    low=low<<8;
    low+=(low==0); 
  }
}


void Encoder::compress(I c) {

  O (pr.isModeled()) {
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
  Y {
    E("unsupported");

    
    
    
    
    
    
    
    
    
  }
}




const char* compname[256]=
  {"","const","cm","icm","match","avg","mix2","mix","isse","sse",0};


const char* opcodelist[272]={
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



void Compiler::next() {

  Q (; *in; ++in) {
    O (*in=='\n') ++line;
    O (*in=='(') state+=1+(state<0);
    Y O (state>0 && *in==')') --state;
    Y O (state<0 && *in<=' ') state=0;
    Y O (state==0 && *in>' ') {state=-1; break;}
  }
  O (!*in) E("unexpected end of config");
}


I tolower(I c) {return (c>='A' && c<='Z') ? c+'a'-'A' : c;}


bool Compiler::matchToken(const char* word) {
  const char* a=in;
  Q (; (*a>' ' && *a!='(' && *word); ++a, ++word)
    O (tolower(*a)!=tolower(*word)) return false;
  return !*word && (*a<=' ' || *a=='(');
}


void Compiler::SE(const char* msg, const char* expected) {
    E("");















}



I Compiler::rtoken(const char* list[]) {


  next();
  Q (I i=0; list[i]; ++i)
    O (matchToken(list[i]))
      return i;
  SE(0);


  return -1; 
}


void Compiler::rtoken(const char* s) {

  next();

  O (!matchToken(s)) SE(0, s);
}



I Compiler::rtoken(I low, I high) {
  next();
  I r=0;
  O (in[0]=='$' && in[1]>='1' && in[1]<='9') {
    O (in[2]=='+') r=atoi(in+3);
    O (args) r+=args[in[1]-'1'];
  }
  Y O (in[0]=='-' || (in[0]>='0' && in[0]<='9')) r=atoi(in);

  Y SE(0);

  O (r<low) SE(0);

  O (r>high) SE(0);
  return r;
}



I Compiler::compile_comp(ZP& z) {
  I op=0;
  const I comp_begin=z.hend;
  while (true) {
    op=rtoken(opcodelist);
    O (op==POST || op==PCOMP || op==END) break;
    I operand=-1; 
    I operand2=-1;  
    O (op==IF) {
      op=JF;
      operand=0; 
      if_stack.push(z.hend+1); 
    }
    Y O (op==IFNOT) {
      op=JT;
      operand=0;
      if_stack.push(z.hend+1); 
    }
    Y O (op==IFL || op==IFNOTL) {  
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
      I a=if_stack.pop();  
    
      O (z.D[a-1]!=LJ) {  
        
        I j=z.hend-a+1+(op==LJ); 
        
        O (j>127) SE("O too big, try IFL, IFNOTL");
        z.D[a]=j;
      }
      Y {  
        I j=z.hend-comp_begin+2+(op==LJ);
        
        z.D[a]=j&255;
        z.D[a+1]=(j>>8)&255;
      }
      if_stack.push(z.hend+1);  
    }
    Y O (op==ENDIF) {
      I a=if_stack.pop();  
    
      I j=z.hend-a-1;  
    
      O (z.D[a-1]!=LJ) {
        
        O (j>127) SE("O too big, try IFL, IFNOTL, ELSEL\n");
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
    
      I j=a-z.hend-2;
    
      O (j>=-127) {  
        O (op==WHILE) op=JT;
        O (op==UNTIL) op=JF;
        O (op==QEVER) op=JMP;
        operand=j&255;
      }
      Y {  
        j=a-comp_begin;
        
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
    Y O ((op&7)==7) { 
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
  z.D[z.hend++]=(0); 
  return op;
}





Compiler::Compiler(const char* in_, I* args_, ZP& hz_, ZP& pz_,
                   Writer* out2_): in(in_), args(args_), hz(hz_), pz(pz_),
                   out2(out2_), if_stack(1000), do_stack(1000) {
  line=1;
  state=0;
  hz.clear();
  pz.clear();
  hz.D.resize(68000); 

  
  rtoken("comp");
  hz.D[2]=rtoken(0, 255);  
  hz.D[3]=rtoken(0, 255);  
  hz.D[4]=rtoken(0, 255);  
  hz.D[5]=rtoken(0, 255);  
  const I n=hz.D[6]=rtoken(0, 255);  
  hz.cend=7;
  Q (I i=0; i<n; ++i) {
    rtoken(i, i);
    CompType type=CompType(rtoken(compname));
    hz.D[hz.cend++]=type;
    I clen=LQ::compsize[type&255];
    O (clen<1 || clen>10) SE("invalid component");
    Q (I j=1; j<clen; ++j)
      hz.D[hz.cend++]=rtoken(0, 255);  
  }
  hz.D[hz.cend++];  
  hz.hbegin=hz.hend=hz.cend+128;

  
  rtoken("hcomp");
  I op=compile_comp(hz);

  
  I hsize=hz.cend-2+hz.hend-hz.hbegin;
  hz.D[0]=hsize&255;
  hz.D[1]=hsize>>8;

  
  O (op==POST) {
    rtoken(0, 0);
    rtoken("end");
  }

  
  Y O (op==PCOMP) {
    pz.D.resize(68000);
    pz.D[4]=hz.D[4];  
    pz.D[5]=hz.D[5];  
    pz.cend=8;
    pz.hbegin=pz.hend=pz.cend+128;

    
    next();
    while (*in && *in!=';') {
      O (out2)
        out2->put(*in);
      ++in;
    }
    O (*in) ++in;

    
    op=compile_comp(pz);
    I len=pz.cend-2+pz.hend-pz.hbegin;  
    
    pz.D[0]=len&255;
    pz.D[1]=len>>8;
    O (op!=END)
      SE("expected END");
  }
  Y O (op!=END)
    SE("expected END or POST 0 END or PCOMP cmd ; ... END");
}































































class MemoryReader: public Reader {
  const char* p;
public:
  MemoryReader(const char* p_): p(p_) {}
  I get() {return *p++&255;}
};
















void C::startBlock(const char* config, I* args, Writer* pcomp_cmd) {
  Compiler(config, args, z, pz, pcomp_cmd);
  enc.out->put(1+(z.D[6]==0));  
  z.write(enc.out, false);
  state=BLOCK1;
}


void C::startSegment(const char* filename, const char* comment) {
  O (state==BLOCK1) state=SEG1;
  O (state==BLOCK2) state=SEG2;
}




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
    Q (I i=0; i<len; ++i)
      enc.compress(pcomp[i]&255);
    
    
  }
  Y
    enc.compress(0);
  state=SEG2;
}


bool C::compress(I n) {
  O (state==SEG1)
    postProcess();


  const I BUFSIZE=1<<14;
  char buf[BUFSIZE];  
  while (n) {
    I nbuf=BUFSIZE;  
    O (n>=0 && n<nbuf) nbuf=n;
    I nr=in->read(buf, nbuf);
    O (nr<0 || nr>BUFSIZE || nr>nbuf) E("invalid read size");
    O (nr<=0) return false;
    O (n>=0) n-=nr;
    Q (I i=0; i<nr; ++i) {
      I ch=U8(buf[i]);
      enc.compress(ch);
    
    
    
    
    }
  }
  return true;
}


void C::endSegment(const char* sha1string) {
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









void compress(Reader* in, Writer* out, const char* ME,
              const char* filename, const char* comment, bool dosha1) {

  
  I bs=4;
  O (ME && ME[0] && ME[1]>='0' && ME[1]<='9') {
    bs=ME[1]-'0';
    O (ME[2]>='0' && ME[2]<='9') bs=bs*10+ME[2]-'0';
    O (bs>11) bs=11;
  }
  bs=(0x100000<<bs)-4096;

  
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



I P::predict() {
  return predict0();
}

void P::update(I y) {
  update0(y);

}

void ZP::run(U32 input) {
    pc=hbegin;
  a=input;
  while (execute()) ;

}











# define AS 256

#define BUCKET_A_SIZE AS
#define BUCKET_B_SIZE (AS*AS)






# define SSIT 8










# define SSB 1024





# define SS_MISORT_STACKSIZE 16



#define SS_SMERGE_STACKSIZE 32
#define TR_INSERTIONSORT_THRESHOLD 8
#define TR_STACKSIZE 64




# define SWAP(_a, _b) do { t = (_a); (_a) = (_b); (_b) = t; } while(0)


# define MIN(_a, _b) (((_a) < (_b)) ? (_a) : (_b))


# define MAX(_a, _b) (((_a) > (_b)) ? (_a) : (_b))

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
    O(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d;\
  } while(0)
#define STACK_POP5(_a, _b, _c, _d, _e)\
  do {\
    assert(0 <= ssize);\
    O(ssize == 0) { return; }\
    (_a) = stack[--ssize].a, (_b) = stack[ssize].b,\
    (_c) = stack[ssize].c, (_d) = stack[ssize].d, (_e) = stack[ssize].e;\
  } while(0)
#define BUCKET_A(_c0) bucket_A[(_c0)]

#define BUCKET_B(_c0, _c1) (bucket_B[((_c1) << 8) | (_c0)])
#define BB(_c0, _c1) (bucket_B[((_c0) << 8) | (_c1)])







#define R(v) v,v,v,v,v,v,v,v,v,v,v,v,v,v,v,v
static const I lg_table[256]= {
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



I
ss_ilg(I n) {











  return (n & 0xff00) ?
          8 + lg_table[(n >> 8) & 0xff] :
          0 + lg_table[(n >> 0) & 0xff];

}






static I dqq_table[256] = {0};





















I
ss_isqrt(I x) {
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







I
ss_compare(const unsigned char *T,
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







static
void
ss_insertionsort(const unsigned char *T, const I *PA,
                 I *F, I *L, I depth) {
  I *i, *j;
  I t;
  I r;

  Q(i = L - 2; F <= i; --i) {
    Q(t = *i, j = i + 1; 0 < (r = ss_compare(T, PA + t, PA + *j, depth));) {
      do { *(j - 1) = *j; } while((++j < L) && (*j < 0));
      O(L <= j) { break; }
    }
    O(r == 0) { *j = ~*j; }
    *(j - 1) = t;
  }
}








void
ss_fixdown(const unsigned char *Td, const I *PA,
           I *SA, I i, I size) {
  I j, k;
  I v;
  I c, d, e;

  Q(v = SA[i], c = Td[PA[v]]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = Td[PA[SA[k = j++]]];
    O(d < (e = Td[PA[SA[j]]])) { k = j; d = e; }
    O(d <= c) { break; }
  }
  SA[i] = v;
}


static
void
ss_heapsort(const unsigned char *Td, const I *PA, I *SA, I size) {
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


I *
ss_median5(const unsigned char *Td, const I *PA,
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


I *
ss_pivot(const unsigned char *Td, const I *PA, I *F, I *L) {
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





I *
ss_partition(const I *PA,
                    I *F, I *L, I depth) {
  I *a, *b;
  I t;
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


static
void
ss_mIrosort(const unsigned char *T, const I *PA,
              I *F, I *L,
              I depth) {
#define STACK_SIZE SS_MISORT_STACKSIZE
  struct { I *a, *b, c; I d; } stack[STACK_SIZE];
  const unsigned char *Td;
  I *a, *b, *c, *d, *e, *f;
  I s, t;
  I ssize;
  I limit;
  I v, x = 0;

  Q(ssize = 0, limit = ss_ilg(L - F);;) {

    O((L - F) <= SSIT) {

      O(1 < (L - F)) { ss_insertionsort(T, PA, F, L, depth); }

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

    
    a = ss_pivot(Td, PA, F, L);
    v = Td[PA[*a]];
    SWAP(*F, *a);

    
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




static
void
ss_inplacemerge(const unsigned char *T, const I *PA,
                I *F, I *M, I *L,
                I depth) {
  const I *p;
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





static
void
ss_mergeQward(const unsigned char *T, const I *PA,
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


static
void
ss_swapmerge(const unsigned char *T, const I *PA,
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







static
void
sssort(const unsigned char *T, const I *PA,
       I *F, I *L,
       I *buf, I bufsize,
       I depth, I n, I Lsuffix) {
  I *a;

  I *b, *M, *curbuf;
  I j, k, curbufsize, Z;

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
    
    I PAi[2]; PAi[0] = PA[*(F - 1)], PAi[1] = n - 2;
    Q(a = F, i = *(F - 1);
        (a < L) && ((*a < 0) || (0 < ss_compare(T, &(PAi[0]), PA + *a, depth)));
        ++a) {
      *(a - 1) = *a;
    }
    *(a - 1) = i;
  }
}




I
tr_ilg(I n) {
  return (n & 0xffff0000) ?
          ((n & 0xff000000) ?
            24 + lg_table[(n >> 24) & 0xff] :
            16 + lg_table[(n >> 16) & 0xff]) :
          ((n & 0x0000ff00) ?
             8 + lg_table[(n >>  8) & 0xff] :
             0 + lg_table[(n >>  0) & 0xff]);
}





static
void
tr_insertionsort(const I *X, I *F, I *L) {
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




void
tr_fixdown(const I *X, I *SA, I i, I size) {
  I j, k;
  I v;
  I c, d, e;

  Q(v = SA[i], c = X[v]; (j = 2 * i + 1) < size; SA[i] = SA[k], i = k) {
    d = X[SA[k = j++]];
    O(d < (e = X[SA[j]])) { k = j; d = e; }
    O(d <= c) { break; }
  }
  SA[i] = v;
}


static
void
tr_heapsort(const I *X, I *SA, I size) {
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





I *
tr_median3(const I *X, I *v1, I *v2, I *v3) {
  I *t;
  O(X[*v1] > X[*v2]) { SWAP(v1, v2); }
  O(X[*v2] > X[*v3]) {
    O(X[*v1] > X[*v3]) { return v1; }
    Y { return v3; }
  }
  return v2;
}


I *
tr_median5(const I *X,
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


I *
tr_pivot(const I *X, I *F, I *L) {
  I *M;
  I t;

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




typedef struct _tr tr;
struct _tr {
  I c;
  I r;
  I i;
  I n;
};

void
trbudget_init(tr *b, I c, I i) {
  b->c = c;
  b->r = b->i = i;
}

I
trbudget_check(tr *b, I size) {
  O(size <= b->r) { b->r -= size; return 1; }
  O(b->c == 0) { b->n += size; return 0; }
  b->r += b->i - size;
  b->c -= 1;
  return 1;
}




void
tr_partition(const I *X,
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
void
tr_copy(I *ISA, const I *SA,
        I *F, I *a, I *b, I *L,
        I depth) {
  
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
void
tr_partialcopy(I *ISA, const I *SA,
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
void
tr_Irosort(I *ISA, const I *X,
             I *SA, I *F, I *L,
             tr *T) {
#define STACK_SIZE TR_STACKSIZE
  struct { const I *a; I *b, *c; I d, e; }stack[STACK_SIZE];
  I *a, *b, *c;
  I t;
  I v, x = 0;
  I incr = X - ISA;
  I Z, next;
  I ssize, TR = -1;

  Q(ssize = 0, Z = tr_ilg(L - F);;) {

    O(Z < 0) {
      O(Z == -1) {
        
        tr_partition(X - incr, F, F, L, &a, &b, L - SA - 1);

        
        O(a < L) {
          Q(c = F, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
        }
        O(b < L) {
          Q(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; }
        }

        
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
        
        a = stack[--ssize].b, b = stack[ssize].c;
        O(stack[ssize].d == 0) {
          tr_copy(ISA, SA, F, a, b, L, X - ISA);
        } Y {
          O(0 <= TR) { stack[TR].d = -1; }
          tr_partialcopy(ISA, SA, F, a, b, L, X - ISA);
        }
        STACK_POP5(X, F, L, Z, TR);
      } Y {
        
        O(0 <= *F) {
          a = F;
          do { ISA[*a] = a - SA; } while((++a < L) && (0 <= *a));
          F = a;
        }
        O(F < L) {
          a = F; do { *a = ~*a; } while(*++a < 0);
          next = (ISA[*a] != X[*a]) ? tr_ilg(a - F + 1) : -1;
          O(++a < L) { Q(b = F, v = a - SA - 1; b < a; ++b) { ISA[*b] = v; } }

          
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

    
    a = tr_pivot(X, F, L);
    SWAP(*F, *a);
    v = X[*F];

    
    tr_partition(X, F, F + 1, L, &a, &b, v);
    O((L - F) != (b - a)) {
      next = (ISA[*a] != v) ? tr_ilg(b - a) : -1;

      
      Q(c = F, v = a - SA - 1; c < a; ++c) { ISA[*c] = v; }
      O(b < L) { Q(c = a, v = b - SA - 1; c < b; ++c) { ISA[*c] = v; } }

      
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






static
void
trsort(I *ISA, I *SA, I n, I depth) {
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





static
I
sort_typeBstar(const unsigned char *T, I *SA,
               I *bucket_A, I *bucket_B,
               I n) {
  I *PAb, *ISAb, *buf;




  I i, j, k, t, m, bufsize;
  I c0, c1;





  
  Q(i = 0; i < BUCKET_A_SIZE; ++i) { bucket_A[i] = 0; }
  Q(i = 0; i < BUCKET_B_SIZE; ++i) { bucket_B[i] = 0; }

  
  Q(i = n - 1, m = n, c0 = T[n - 1]; 0 <= i;) {
    
    do { ++BUCKET_A(c1 = c0); } while((0 <= --i) && ((c0 = T[i]) >= c1));
    O(0 <= i) {
      
      ++BB(c0, c1);
      SA[--m] = i;
      
      Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) {
        ++BUCKET_B(c0, c1);
      }
    }
  }
  m = n - m;


  
  Q(c0 = 0, i = 0, j = 0; c0 < AS; ++c0) {
    t = i + BUCKET_A(c0);
    BUCKET_A(c0) = i + j; 
    i = t + BUCKET_B(c0, c0);
    Q(c1 = c0 + 1; c1 < AS; ++c1) {
      j += BB(c0, c1);
      BB(c0, c1) = j; 
      i += BUCKET_B(c0, c1);
    }
  }

  O(0 < m) {
    
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

    
    trsort(ISAb, SA, m, 1);

    
    Q(i = n - 1, j = m, c0 = T[n - 1]; 0 <= i;) {
      Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) >= c1); --i, c1 = c0) { }
      O(0 <= i) {
        t = i;
        Q(--i, c1 = c0; (0 <= i) && ((c0 = T[i]) <= c1); --i, c1 = c0) { }
        SA[ISAb[--j]] = ((t == 0) || (1 < (t - i))) ? t : ~t;
      }
    }

    
    BUCKET_B(AS - 1, AS - 1) = n; 
    Q(c0 = AS - 2, k = m - 1; 0 <= c0; --c0) {
      i = BUCKET_A(c0 + 1) - 1;
      Q(c1 = AS - 1; c0 < c1; --c1) {
        t = i - BUCKET_B(c0, c1);
        BUCKET_B(c0, c1) = i; 

        
        Q(i = t, j = BB(c0, c1);
            j <= k;
            --i, --k) { SA[i] = SA[k]; }
      }
      BB(c0, c0 + 1) = i - BUCKET_B(c0, c0) + 1; 
      BUCKET_B(c0, c0) = i; 
    }
  }

  return m;
}


static
void
construct_SA(const unsigned char *T, I *SA,
             I *bucket_A, I *bucket_B,
             I n, I m) {
  I *i, *j, *k;
  I s;
  I c0, c1, c2;

  O(0 < m) {
    
    Q(c1 = AS - 2; 0 <= c1; --c1) {
      
      Q(i = SA + BB(c1, c1 + 1),
          j = SA + BUCKET_A(c1 + 1) - 1, k = NULL, c2 = -1;
          i <= j;
          --j) {
        O(0 < (s = *j)) {
        
        
        
          *j = ~s;
          c0 = T[--s];
          O((0 < s) && (T[s - 1] > c0)) { s = ~s; }
          O(c0 != c2) {
            O(0 <= c2) { BUCKET_B(c2, c1) = k - SA; }
            k = SA + BUCKET_B(c2 = c0, c1);
          }
        
          *k-- = s;
        } Y {
        
          *j = ~s;
        }
      }
    }
  }

  
  k = SA + BUCKET_A(c2 = T[n - 1]);
  *k++ = (T[n - 2] < c2) ? ~(n - 1) : (n - 1);
  
  Q(i = SA, j = SA + n; i < j; ++i) {
    O(0 < (s = *i)) {
    
      c0 = T[--s];
      O((s == 0) || (T[s - 1] < c0)) { s = ~s; }
      O(c0 != c2) {
        BUCKET_A(c2) = k - SA;
        k = SA + BUCKET_A(c2 = c0);
      }
    
      *k++ = s;
    } Y {
    
      *i = ~s;
    }
  }
}






I
divsufsort(const unsigned char *T, I *SA, I n) {
  I *bucket_A, *bucket_B;
  I m;
  I err = 0;

  
  O((T == NULL) || (SA == NULL) || (n < 0)) { return -1; }
  Y O(n == 0) { return 0; }
  Y O(n == 1) { SA[0] = 0; return 0; }
  Y O(n == 2) { m = (T[0] < T[1]); SA[m ^ 1] = 0, SA[m] = 1; return 0; }

  bucket_A = (I *)malloc(BUCKET_A_SIZE * sizeof(I));
  bucket_B = (I *)malloc(BUCKET_B_SIZE * sizeof(I));

  
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






std::string itos(int64_t x, I n=1) {


  std::string r;
  Q (; x || n>0; x/=10, --n) r=std::string(1, '0'+x%10)+r;
  return r;
}





























class LZBuffer: public LQ::Reader {
  LQ::Array<unsigned> ht;
  const unsigned char* in;    
  const I checkbits;        
  const I level;            
  const unsigned htsize;      
  const unsigned n;           
  unsigned i;                 
  const unsigned m;    
  const unsigned m2;   
  const unsigned maxMatch;    
  const unsigned maxLiteral;  
  const unsigned lookahead;   
  unsigned h1, h2;            
  const unsigned bucket;      
  const unsigned shift1, shift2;  
  const I mBoth;     
  const unsigned rb;          
  unsigned bits;              
  unsigned nbits;             
  unsigned rpos, wpos;        
  unsigned idx;               
  const unsigned* sa;         
  unsigned* isa;              
  enum {BUFSIZE=1<<14};       
  unsigned char buf[BUFSIZE]; 

  void write_literal(unsigned i, unsigned& lit);
  void write_match(unsigned len, unsigned off);
  void fill();  

  
  void putb(unsigned x, I k) {
    x&=(1<<k)-1;
    bits|=x<<nbits;
    nbits+=k;
    while (nbits>7) {
    
      buf[wpos++]=bits, bits>>=8, nbits-=8;
    }
  }

  
  void flush() {
    
    O (nbits>0) buf[wpos++]=bits;
    bits=nbits=0;
  }

  
  void put(I c) {
    
    buf[wpos++]=c;
  }

public:
  LZBuffer(SB& inbuf, I args[], const unsigned* sap=0);

  
  I get() {
    I c=-1;
    O (rpos==wpos) fill();
    O (rpos<wpos) c=buf[rpos++];
    O (rpos==wpos) rpos=wpos=0;
    return c;
  }

  
  I read(char* p, I n);
};























I lg(unsigned x) {
  unsigned r=0;
  O (x>=65536) r=16, x>>=16;
  O (x>=256) r+=8, x>>=8;
  O (x>=16) r+=4, x>>=4;

  return
    "\x00\x01\x02\x02\x03\x03\x03\x03\x04\x04\x04\x04\x04\x04\x04\x04"[x]+r;
}


I nbits(unsigned x) {
  I r;
  Q (r=0; x; x>>=1) r+=x&1;
  return r;
}



I LZBuffer::read(char* p, I n) {
  O (rpos==wpos) fill();
  I nr=n;
  O (nr>I(wpos-rpos)) nr=wpos-rpos;
  O (nr) memcpy(p, buf+rpos, nr);
  rpos+=nr;

  O (rpos==wpos) rpos=wpos=0;
  return nr;
}

LZBuffer::LZBuffer(SB& inbuf, I args[], const unsigned* sap):
    ht((args[1]&3)==3 ? (inbuf.size()+1)*!sap      
        : args[5]-args[0]<21 ? 1u<<args[5]         
        : (inbuf.size()*!sap)+(1u<<17<<args[0])),  
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




  O ((m<4 && level==1) || (m<1 && level==2))
    E("match length $3 too small");

  


  
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


void LZBuffer::fill() {

  
  O (level==3) {
    
    
    Q (; wpos<BUFSIZE && i<n+5; ++i) {
      O (i==0) put(n>0 ? in[n-1] : 255);
      Y O (i>n) put(idx&255), idx>>=8;
      Y O (sa[i-1]==0) idx=i, put(255);
      Y put(in[sa[i-1]-1]);
    }
    return;
  }

  
  unsigned lit=0;  
  const unsigned mask=(1<<checkbits)-1;
  while (i<n && wpos*2<BUFSIZE) {

    
    unsigned blen=m-1;  
    unsigned bp=0;  
    unsigned blit=0;  
    I bscore=0;  

    
    O (isa) {
      O (sa[isa[i&mask]]!=i) 
        Q (unsigned j=0; j<n; ++j)
          O ((sa[j]&~mask)==(i&~mask))
            isa[sa[j]&mask]=j;
      Q (unsigned h=0; h<=lookahead; ++h) {
        unsigned q=isa[(h+i)&mask];  
        
        O (sa[q]!=h+i) continue;
        Q (I j=-1; j<=1; j+=2) {  
          Q (unsigned k=1; k<=bucket; ++k) {
            unsigned p;  
            O (q+j*k<n && (p=sa[q+j*k]-h)<i) {
            
              unsigned l, l1;  
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

    
    
    
    Y O (level==1 || m<=64) {
      O (m2>0) {
        Q (unsigned k=0; k<=bucket; ++k) {
          unsigned p=ht[h2^k];
          O (p && (p&mask)==(in[i+3]&mask)) {
            p>>=checkbits;
            O (p<i && i+blen<=n && in[p+blen-1]==in[i+blen-1]) {
              unsigned l;  
              Q (l=lookahead; i+l<n && l<maxMatch && in[p+l]==in[i+l]; ++l);
              O (l>=m2+lookahead) {
                I l1;  
                Q (l1=lookahead; l1>0 && in[p+l1-1]==in[i+l1-1]; --l1);
                
                I score=I(l-l1)*8-lg(i-p)-8*(lit==0 && l1>0)-11;
                O (score>bscore) blen=l, bp=p, blit=l1, bscore=score;
              }
            }
          }
          O (blen>=128) break;
        }
      }

      
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

    
    
    
    const unsigned off=i-bp;  
    O (off>0 && bscore>0
        && blen-blit>=m+(level==2)*((off>=(1<<16))+(off>=(1<<24)))) {
      lit+=blit;
      write_literal(i+blit, lit);
      write_match(blen-blit, off);
    }

    
    Y {
      blen=1;
      ++lit;
    }

    
    O (isa)
      i+=blen;
    Y {
      while (blen--) {
        O (i+mBoth<n) {
          unsigned ih=((i*1234547)>>19)&bucket;
          const unsigned p=(i<<checkbits)|(in[i+3]&mask);
        
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

    
    O (lit>=maxLiteral)
      write_literal(i, lit);
  }

  

  O (i==n) {
    write_literal(n, lit);
    flush();
  }
}


void LZBuffer::write_literal(unsigned i, unsigned& lit) {



  O (level==1) {
    O (lit<1) return;
    I ll=lg(lit);
    
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
    
    while (lit>0) {
      unsigned lit1=lit;
      O (lit1>64) lit1=64;
      put(lit1-1);
      Q (unsigned j=i-lit; j<i-lit+lit1; ++j) put(in[j]);
      lit-=lit1;
    }
  }
}


void LZBuffer::write_match(unsigned len, unsigned off) {

  
  O (level==1) {
    
    
    
    
    I ll=lg(len)-1;
    
    off+=(1<<rb)-1;
    I lo=lg(off)-1-rb;
    
    putb((lo+8)>>3, 2);
    putb(lo&7, 3);     
    while (--ll>=2) {  
      putb(1, 1);
      putb((len>>ll)&1, 1);
    }
    putb(0, 1);
    putb(len&3, 2);    
    putb(off, rb);     
    putb(off>>rb, lo); 
  }

  
  Y {
    
    
    --off;
    while (len>0) {  
      const unsigned len1=len>m*2+63 ? m+63 :
          len>m+63 ? len-m : len;
    
    
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



std::string makeConfig(const char* ME, I args[]) {

  const char type=ME[0];


  
  args[0]=0;  
  args[1]=0;  
  args[2]=0;  
  args[3]=0;  
  args[4]=0;  
  args[5]=0;  
  args[6]=0;  
  args[7]=0;  
  args[8]=0;  
  O (isdigit(*++ME)) args[0]=0;
  Q (I i=0; i<9 && (isdigit(*ME) || *ME==',' || *ME=='.');) {
    O (isdigit(*ME))
      args[i]=args[i]*10+*ME-'0';
    Y O (++i<9)
      args[i]=0;
    ++ME;
  }

  
  O (type=='0')
    return "comp 0 0 0 0 0 hcomp end\n";

  
  std::string hdr, pcomp;
  const I level=args[1]&3;
  const bool doe8=args[1]>=4 && args[1]<=7;

  
  O (level==1) {
    E(""); 

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
  }

  
  Y O (level==2) {
    hdr="comp 9 16 0 $1+20 ";
    pcomp=
    "pcomp lzpre c ;\n"
    
    
    "  a> 255 O\n";
    O (doe8) {
        E("");
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    }
    pcomp+="b=0 c=0 d=0 a=0 r=a 1 r=a 2 halt endif c=a a=d a== 0 O a=c a>>= 6 a++ d=a a== 1 O a+=c r=a 1 a=0 r=a 2 Y d++ a=c a&= 63 a+= $3 r=a 1 a=0 r=a 2 endif Y a== 1 O a=c *b=a b++\n";
    O (!doe8) pcomp+=" out ";
    pcomp+="a=r 1 a-- a== 0 O d=0 endif r=a 1 Y a> 2 O a=r 2 a<<= 8 a|=c r=a 2 d-- Y a=r 2 a<<= 8 a|=c c=a a=b a-=c a-- c=a d=r 1 do a=*c *b=a c++ b++";
    O (!doe8) pcomp+=" out d-- a=d a> 0 while endif endif endif halt end ";
  }

  
  Y O (level==3) {  
    E("");

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
  }

  
  Y O (level==0) {
    E("");
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
  }
  Y
    E("Unsupported ME");
  
  
  
  
  
  
  
  
  I ncomp=0;  
  const I membits=args[0]+20;
  I sb=5;  
  std::string comp;
  std::string hcomp="hcomp\n"
    "c-- *c=a a+= 255 d=a *d=c\n";
  O (level==2) {  
    hcomp+=
    
    
    
    
    
    "  a=r 1 a== 0 O\n"
    
    "    a= "+itos(111+57*doe8)+"\n"
    
    "  Y a== 1 O\n"
    
    "    a=*c r=a 2\n"
    
    "    a> 63 O a>>= 6 a++ a++\n"
    
    "    Y a++ a++ endif\n"
    
    "  Y\n"
    "    a--\n"
    "  endif endif\n"
    
    "  r=a 1\n";
  }

  
  while (*ME && ncomp<254) {
    
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

    
    
    
    
    
    O (v[0]=='c') {
      while (v.size()<3) v.push_back(0);
      comp+=itos(ncomp)+" ";
      sb=11;  
      O (v[2]<256) sb+=lg(v[2]);
      Y sb+=6;
      Q (unsigned i=3; i<v.size(); ++i)
        O (v[i]<512) sb+=nbits(v[i])*3/4;
      O (sb>membits) sb=membits;
      O (v[1]%1000==0) comp+="icm "+itos(sb-6-v[1]/1000)+"\n";
      Y comp+="cm "+itos(sb-2-v[1]/1000)+" "+itos(v[1]%1000-1)+"\n";

      
      hcomp+="d= "+itos(ncomp)+" *d=0\n";
      O (v[2]>1 && v[2]<=255) {  
        O (lg(v[2])!=lg(v[2]-1))
          hcomp+="a=c a&= "+itos(v[2]-1)+" hashd\n";
        Y
          hcomp+="a=c a%= "+itos(v[2])+" hashd\n";
      }
      Y O (v[2]>=1000 && v[2]<=1255)  
        hcomp+="a= 255 a+= "+itos(v[2]-1000)+
               " d=a a=*d a-=c a> 255 O a= 255 endif d= "+
               itos(ncomp)+" hashd\n";

      
      Q (unsigned i=3; i<v.size(); ++i) {
        O (i==3) hcomp+="b=c ";
        O (v[i]==255)
          hcomp+="a=*b hashd\n";  
        Y O (v[i]>0 && v[i]<255)
          hcomp+="a=*b a&= "+itos(v[i])+" hashd\n";  
        Y O (v[i]>=256 && v[i]<512) { 
          hcomp+=
          "a=r 1 a> 1 O\n"  
          "  a=r 2 a< 64 O\n"  
          "    a=*b ";
          O (v[i]<511) hcomp+="a&= "+itos(v[i]-256);
          hcomp+=" hashd\n"
          "  Y\n"  
          "    a>>= 6 hashd a=r 1 hashd\n"
          "  endif\n"
          "Y\n"  
          "  a= 255 hashd a=r 2 hashd\n"
          "endif\n";
        }
        Y O (v[i]>=1256)  
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

    
    
    
    O (strchr("mts", v[0]) && ncomp>I(v[0]=='t')) {
        E("");

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
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

    
    O (v[0]=='a') {
        E("");
    
    
    
    
    
    
    
    
    }

    
    
    
    
    O (v[0]=='w') {
        E("");


    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    }
  }
  return hdr+itos(ncomp)+"\n"+comp+hcomp+"halt\n"+pcomp;
}







void compressBlock(SB* in, Writer* out, const char* method_,
                   const char* filename, const char* comment, bool dosha1) {
  std::string ME=method_;
  const unsigned n=in->size();  
  const I arg0=MAX(lg(n+4095)-20, 0);  


  
  
  unsigned type=0;
  O (isdigit(ME[0])) {
    I commas=0, arg[4]={0};
    Q (I i=1; i<I(ME.size()) && commas<4; ++i) {
      O (ME[i]==',' || ME[i]=='.') ++commas;
      Y O (isdigit(ME[i])) arg[commas]=arg[commas]*10+ME[i]-'0';
    }
    O (commas==0) type=512;
    Y type=arg[1]*4+arg[2];
  }

  











  
  O (isdigit(ME[0])) {
    const I level=ME[0]-'0';
    

    
    const I doe8=(type&2)*2;
    ME="x"+itos(arg0);
    std::string htsz=","+itos(19+arg0+(arg0<=6));  
    std::string sasz=","+itos(21+arg0);            

    
    O (level==0)
      ME="0"+itos(arg0)+",0";

    
    Y O (level==1) {
        E("");
    
    
    
    
    
    
    
    
    
    }

    
    Y O (level==2) {
        E("");
    
    
    
    
    
    
    }

    
    Y O (level==3) {
      O (type<20)  
        ME+=",0";
      Y O (type<48)  
        ME+=","+itos(1+doe8)+",4,0,3"+htsz;
      Y O (type>=640 || (type&1))  
        ME+=","+itos(3+doe8)+"ci1";
      Y  
        ME+=","+itos(2+doe8)+",12,0,7"+sasz+",1c0,0,511i2";
    }

    
    Y O (level==4) {
        E("");
    
    
    
    
    
    
    
    
    
    
    
    
    
    }

    
    Y {  
        E("");
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    }
  }

  
  std::string config;
  I args[9]={0};
  config=makeConfig(ME.c_str(), args);

  LQ::C co;
  co.setOutput(out);

  SB pcomp_cmd;
  co.startBlock(config.c_str(), args, &pcomp_cmd);
  std::string cs=itos(n);
  co.startSegment(filename, cs.c_str());
  O (args[1]>=1 && args[1]<=7 && args[1]!=4) {  
    LZBuffer lz(*in, args);
    co.setInput(&lz);
    co.compress();
  }
  Y { 
    E("unsupported");
  }

  co.endSegment(0);
}

}  


namespace zpaq {
  td::BufferSlice compress(td::Slice data) {
    I size = td::narrow_cast<int>(data.size());

    LQ::SB in, out;
    in.write(data.data(), size);

    LQ::compressBlock(&in, &out, "3", 0, 0, false);

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



td::BufferSlice serialize_boc_opt(ostream& out, Ref<Cell> cell) {
	CHECK(!cell.is_null());
  BagOfCells boc;
  boc.add_root(cell);
  CHECK(boc.import_cells().is_ok());
	auto size = boc.estimate_serialized_size(0);
	td::BufferSlice bs{size};
	auto buffer = (unsigned char*)bs.data();

	boc_writers::BufferWriter writer{buffer, buffer + size};

	
	std::vector<int> backrefs(boc.cell_list_.size(), 0);
	for (int i = 0; i < boc.cell_count; ++i) {
    const auto& cell = boc.cell_list_[i];
		for (int j = 0; j < cell.ref_num; ++j) {
			backrefs[cell.ref_idx[j]] += 1;
		}
  }

	auto last_print = writer.store_ptr;
	auto print_writer = [&](string label) {
		if (last_print < writer.store_ptr) {
			out << label << ": " << td::ConstBitPtr{last_print}.to_hex((writer.store_ptr - last_print) * 8) << endl;
			last_print = writer.store_ptr;
		}
	};

	auto store_byte = [&](unsigned long long value) { writer.store_uint(value, 1); };
	auto store_ref = [&](unsigned long long value) { writer.store_uint(value, boc.info.ref_byte_size); };
	auto overwrite_ref = [&](int position, unsigned long long value) { 
		auto ptr = writer.store_ptr;
		writer.store_ptr = writer.store_start + position;
		store_ref(value);
		writer.store_ptr = ptr;
	};

	store_byte(boc.info.ref_byte_size);
	print_writer("ref-size");

  store_ref(0); 
	print_writer("cell-num");

	vector<int> idx_to_ref(boc.cell_list_.size(), -1);
	vector<pair<int, size_t>> refs_to_set;

	std::function<void(int, const vm::BagOfCells::CellInfo&)> store_cell;
	store_cell = [&](int idx, const vm::BagOfCells::CellInfo& dc_info) { 
		unsigned char buf[256] = {};
    const Ref<DataCell>& dc = dc_info.dc_ref;

		
		int mask = 0;
		for (unsigned j = 0; j < dc_info.ref_num; ++j) {
			int ref_idx = dc_info.ref_idx[j];
			int br = backrefs.at(ref_idx);
			if (br == 1) {
				mask |= (1 << j);
			}
		}

		int s = dc->serialize(buf, 256, false);
		buf[0] = (buf[0] & 15) + mask * 16;
    writer.store_bytes(buf, s);
		print_writer("data");

		for (unsigned j = 0; j < dc_info.ref_num; ++j) {
			int ref_idx = dc_info.ref_idx[j];
			int br = backrefs.at(ref_idx);
			CHECK(br > 0);
			if (br == 1) {
				
				store_cell(ref_idx, boc.cell_list_[ref_idx]);
			} else {
				
				refs_to_set.emplace_back(ref_idx, writer.position());
				store_ref(0);
				print_writer("ref");
			}
    }
	};

	
	int cells_cnt = 0;
  for (int i = 0; i < boc.cell_count; ++i) {
		int k = boc.cell_count - 1 - i;
    const auto& dc_info = boc.cell_list_[k];
		int br = backrefs.at(k);

		if (br != 1) {
			store_cell(k, dc_info);
			idx_to_ref[k] = cells_cnt++;
		} 
  }

	
	overwrite_ref(1, cells_cnt);

	
	for (const auto& p: refs_to_set) {
		overwrite_ref(p.second, idx_to_ref[p.first]);
	}

	bs.truncate(writer.position());
	return bs;
}

struct DeserializedCell {
	bool special;
	int bits;
	string data;
	vector<pair<int, int>> refs{};
};

Ref<Cell> deserialize_boc_opt(ostream& out, td::Slice data) {
	CHECK(!data.empty());

	int start = 0;
	
	auto last_print = 0;
	auto print_reader = [&](string label) {
		if (last_print < start) {
			out << label << ": " << td::ConstBitPtr{(const unsigned char*)data.data() + last_print}.to_hex((start - last_print) * 8) << endl;
			last_print = start;
		}
	};

	auto read_byte = [&]() { 
		CHECK(data.size() > start);
		return (unsigned char)data[start++];
	};
	auto read_int = [&](int bytes) {
		unsigned long long res = 0;
		while (bytes > 0) {
			res = (res << 8) + read_byte();
			--bytes;
		}
		return res;
	};
	auto read_bytes = [&](int bytes) {
		string bs(bytes, 0);
		for (int i = 0; i < bytes; ++i) {
			bs.at(i) = read_byte();
		}
		return bs;
	};

	auto ref_byte_size = (int)read_byte();
	print_reader("ref-size");

	auto read_ref = [&]() { return read_int(ref_byte_size); };

	auto cell_num = read_ref();
	print_reader("cell-num");
	
	vector<DeserializedCell> cells_data;
	vector<array<pair<int, int>, 4>> cells_refs;
	vector<int> ref_to_cd_idx(cell_num);

	std::function<void(int)> read_cell;
	read_cell = [&](int idx) { 
		auto d1 = read_byte();
		auto d2 = read_byte();

		auto ref_num = d1 & 7;
		auto special = (d1 & 8) > 0;
		auto mask = (d1 >> 4) & 15;

		auto bytes = (d2 + 1) >> 1;

		auto data = read_bytes(bytes);
		print_reader("data");
		auto bits = bytes * 8;

		
		if (data.size() > 0 && (d2 & 1)) {
			unsigned char last_byte = data[data.size() - 1];
			int significant = 8;
			while (significant && (last_byte & 1) == 0) {
				significant--;
				last_byte >>= 1;
			}
			if (significant) {
				last_byte = (last_byte & 254) << (8 - significant);
				significant--;
			}
			data[data.size() - 1] = last_byte;
			bits -= (8 - significant);
		}

		if (idx != -1) {
			
			ref_to_cd_idx.at(idx) = cells_data.size();
		}
		cells_data.push_back(DeserializedCell{
				special, bits, data,
		});
		int dc_idx = cells_data.size() - 1;

		for (int i = 0; i < ref_num; ++i) {
			bool is_embedded = (mask >> i) & 1;
			if (is_embedded) {
				
				cells_data.at(dc_idx).refs.push_back({cells_data.size(), -1});
				read_cell(-1);
			} else {
				
				cells_data.at(dc_idx).refs.push_back({-1, read_ref()});
				print_reader("ref");
			}
		}
	};

	for (int i = 0; i < cell_num; ++i) {
		read_cell(i);
	}

	vector<Ref<Cell>> cells(cells_data.size());

	for (int i = cells_data.size() - 1; i >= 0; --i) {
		auto& cd = cells_data[i];
		CellBuilder cb;
		cb.store_bits(cd.data.data(), cd.bits);
		for (const auto& ref: cd.refs) {
			int idx = ref.first;
			if (idx == -1) {
				CHECK(ref.second != -1);
				idx = ref_to_cd_idx.at(ref.second);
			}
			auto& cell = cells.at(idx);
			CHECK(!cell.is_null());
			cb.store_ref(cell);
		}
		cells[i] = cb.finalize(cd.special);
	}

	return std::move(cells[0]);
}

td::BufferSlice do_compress(td::Slice data) {
	
	return zpaq::compress(data);
}

td::BufferSlice do_decompress(td::Slice data) {
	
	return zpaq::decompress(data);
}

td::BufferSlice compress(td::Slice data) {
	NullStream ofs;

	Ref<Cell> block_root = std_boc_deserialize(data).move_as_ok();
	
	FullBlock block;
	ParseContext load_std_ctx{ofs};
	block.cell_unpack_std(load_std_ctx, block_root, 0, true);

	ParseContext pack_opt_ctx{ofs};
	auto opt_block_cell = block.make_opt_cell(pack_opt_ctx);

	
	auto opt_ser = serialize_boc_opt(ofs, opt_block_cell);
	auto compressed = do_compress(opt_ser);

	return compressed;
}

td::BufferSlice decompress(td::Slice data) {
	NullStream ofs;

	auto decompressed = do_decompress(data);
	
	auto opt_deser = deserialize_boc_opt(ofs, decompressed);

	FullBlock opt_block;
	ParseContext parse_opt_ctx{ofs};
	opt_block.cell_unpack_opt(parse_opt_ctx, opt_deser, 0, true);

	ParseContext pack_std_ctx{ofs};
	auto un_opt_block_cell = opt_block.make_std_cell(pack_std_ctx);

	auto boc = std_boc_serialize(un_opt_block_cell, 31).move_as_ok();
	return boc;
}



int main(
)
{
	

		string mode;
		cin >> mode;

		string base64_data;
		cin >> base64_data;


		td::BufferSlice data(td::base64_decode(base64_data));

		data = (mode == "compress") ? compress(data) : decompress(data);

		cout << td::str_base64_encode(data) << endl;
}
