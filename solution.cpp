// minify-remove:start
#define FIXED_FLOAT(x) fixed <<setprecision(2)<<(x)
// minify-remove:end

#include <iostream>
#include <sstream>
#include <fstream>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include "common/util.h"
#include "vm/cells/CellSlice.h"
#include "td/utils/lz4.h"
#include "td/utils/misc.h"
#include "td/utils/buffer.h"
#include<errno.h>
#include<stddef.h>
#include<string.h>
#include<stdint.h>

#define private public
#define final
#include "block/block-auto.h"
#include "block/block-parse.h"
#include "vm/boc.h"
#include "crypto/vm/boc-writers.h"

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

	// "AccountBlock",
	"ShardAccountBlocks",

	"MERKLE_UPDATE",
	// "ShardState",
	// "ShardStateUnsplit",
	// "OutMsgQueueInfo",
	// "OutMsgQueue",
	// "ShardStateUnsplit_aux",

	"Maybe",
	"ValueFlow",
};

CellSlice to_cs(Ref<Cell> cell)
{
	// minify-remove
	CHECK(!cell.is_null())
	bool can_be_special = false;
	return load_cell_slice_special(std::move(cell), can_be_special);
}

struct ParseContext {
	ostream& out;

	// minify-remove:start
	ostream& indent(int x) {
		while (x-- > 0)
		{
			out << ' ';
		}
		return out;
	}
	// minify-remove:end
};

// minify-remove:start
string bin_to_hex(const string& bin) {
	unsigned char buff[2048];
  auto len = td::bitstring::parse_bitstring_binary_literal(buff, sizeof(buff) * 8, bin.c_str(), bin.c_str() + bin.length());
	td::BitSlice bs(buff, len);
	return bs.to_hex();
}
// minify-remove:end


template <class T_TLB>
struct BaseFullCell
{
	// keep in mind these cs may contain other cells in the end
	CellSlice incoming_cs;
	// and this may be null if optimization is enabled for cell and we're reading opt cs
	CellSlice std_cell_cs;
	// custom cell slice, useful to read until the end of the cell
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
		// minify-remove
		throw runtime_error(name + " optimization is enabled but not implemented");
	}

	virtual void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		do_pack_std(ctx, cb, indent);
	}

	virtual void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) {
		do_unpack_std(ctx, cs, indent);
	}

	virtual void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		// minify-remove
		throw runtime_error(name + " enabled but not implemented");
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
		// minify-remove:start
		ctx.indent(indent) << (name + ".unpack_std ") 
											 << (e ? "" : "(disabled) ") 
											 << special_type << ":" 
											 << cs.size() << ":" 
											 << cs.size_refs() 
											 << "[" << cs.as_bitslice().to_hex() << "]"
											 << endl;
		ctx.indent(indent) << (name + ".unpack_std ") 
											 << (e ? "" : "(disabled) ") 
											 << special_type << ":" 
											 << cs.size() << ":" 
											 << cs.size_refs() 
											 << "[" << cs.as_bitslice().to_binary() << "]" 
											 << endl;
		// minify-remove:end

		incoming_cs = cs;

		if (is_pruned_branch()) {
			cs.advance(288);
			// minify-remove:start
			ctx.indent(indent) << (name + ".unpack_std ") << "^ Pruned branch" << endl;
			CHECK(cs.empty_ext());
			CHECK(incoming_cs.size() == 288);
			// minify-remove:end
			return;
		}

		if (e) {
			do_unpack_std(ctx, cs, indent);
		} else {
			std_cell_cs = type.fetch(cs).write();
			// minify-remove:start
			ctx.indent(indent) << (name + ".unpack_std: fetched ") 
												 << special_type << ":" 
												 << std_cell_cs.size() << ":" 
												 << std_cell_cs.size_refs() 
												 << "[" << std_cell_cs.as_bitslice().to_hex() << "]" 
												 << endl;
			ctx.indent(indent) << (name + ".unpack_std: fetched ") 
												 << special_type << ":" 
												 << std_cell_cs.size() << ":" 
												 << std_cell_cs.size_refs() 
												 << "[" << std_cell_cs.as_bitslice().to_binary() << "]" << endl;
			// minify-remove:end
		}
		// minify-remove:start
		if (check_empty) {
			CHECK(cs.empty_ext());
		}
		// minify-remove:end
	}

	void pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		auto e = is_enabled();

		// minify-remove:start
		auto current_bin = cb.as_cellslice().as_bitslice().to_binary();
		auto current_refs = cb.size_refs();
		// minify-remove:end

		if (is_pruned_branch()) {
			cb.append_cellslice(incoming_cs);
		} else if (e) {
			do_pack_opt(ctx, cb, indent);
		} else {
			cb.append_cellslice(std_cell_cs);
		}

		// minify-remove:start
		auto new_cs = cb.as_cellslice();
		auto new_bs = new_cs.as_bitslice();
		auto new_bin = new_bs.to_binary();
		auto added_bin = new_bin.substr(current_bin.length());
		auto added_refs = cb.size_refs() - current_refs;

		ctx.indent(indent) << (name + ".pack_opt added ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << "-:" 
											 << added_bin.length() << ":" 
											 << added_refs 
											 << "[" << bin_to_hex(added_bin) << "]" << endl;
		ctx.indent(indent) << (name + ".pack_opt added ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << "-:" 
											 << added_bin.length() << ":" 
											 << added_refs 
											 << "[" << added_bin << "]" << endl;
		ctx.indent(indent) << (name + ".pack_opt full ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << (int)new_cs.special_type() << ":" 
											 << cb.size() << ":" 
											 << cb.size_refs() 
											 << "[" << new_cs.as_bitslice().to_hex() << "]" << endl;
		ctx.indent(indent) << (name + ".pack_opt full ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << (int)new_cs.special_type() << ":" 
											 << cb.size() << ":" 
											 << cb.size_refs() 
											 << "[" << new_cs.as_bitslice().to_binary() << "]" << endl;
		// minify-remove:end
	}

	void unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0, bool check_empty = false) {
		auto e = is_enabled();

		// minify-remove
		ctx.indent(indent) << (name + ".unpack_opt ") << (e ? "" : "(disabled) ") << cs.as_bitslice().to_hex() << endl;
		// minify-remove
		ctx.indent(indent) << (name + ".unpack_opt ") << (e ? "" : "(disabled) ") << cs.as_bitslice().to_binary() << endl;

		incoming_cs = cs;

		if (e) {
			do_unpack_opt(ctx, cs, indent);
		} else {
			std_cell_cs = type.fetch(cs).write();
			// minify-remove
			ctx.indent(indent) << (name + ".unpack_opt: fetched ") <<  std_cell_cs.as_bitslice().to_hex() << endl;
		}

		// minify-remove:start
		if (check_empty) {
			CHECK(cs.empty_ext());
		}
		// minify-remove:end
	}

	void pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		auto e = is_enabled();

		// minify-remove:start
		auto current_cs = cb.as_cellslice();
		auto current_bin = current_cs.as_bitslice().to_binary();
		auto current_refs = cb.size_refs();

		ctx.indent(indent) << (name + ".pack_std begin ") 
		    							 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}"
											 << (int)current_cs.special_type() << ":"
											 << cb.size() << ":"
											 << current_refs
											 << "[" << current_cs.as_bitslice().to_hex() << "]" << endl;
		// minify-remove:end

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

		// minify-remove:start
		auto new_cs = cb.as_cellslice();
		auto new_bs = new_cs.as_bitslice();
		auto new_bin = new_bs.to_binary();
		auto added_bin = new_bin.substr(current_bin.length());
		auto added_refs = cb.size_refs() - current_refs;

		ctx.indent(indent) << (name + ".pack_std added ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << "-:" 
											 << added_bin.length() << ":" 
											 << added_refs 
											 << "[" << bin_to_hex(added_bin) << "]" << endl;
		ctx.indent(indent) << (name + ".pack_std added ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << "-:" 
											 << added_bin.length() << ":" 
											 << added_refs 
											 << "[" << added_bin << "]" << endl;
		ctx.indent(indent) << (name + ".pack_std full ") 
		 									 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}"
											 << (int)new_cs.special_type() << ":"
											 << cb.size() << ":"
											 << cb.size_refs() 
											 << "[" << new_cs.as_bitslice().to_hex() << "]" << endl;
		ctx.indent(indent) << (name + ".pack_std full ") 
										<< (e ? "" : "(disabled) ") 
										<< "{" << special_type << "}"
										<< (int)new_cs.special_type() << ":"
										<< cb.size() << ":"
										<< cb.size_refs() 
										<< "[" << new_cs.as_bitslice().to_binary() << "]" << endl;
		// minify-remove:end
	}

	Ref<Cell> make_std_cell(ParseContext& ctx, int indent = 0) {
		// minify-remove
		ctx.indent(indent) << "------------------- " << name << " std cell begin -------------------" << endl;
		CellBuilder cb;
		pack_std(ctx, cb, indent);
		// minify-remove:start
		auto cs = cb.as_cellslice();
		ctx.indent(indent) << "------------------- " << name << " std cell final -------------------" << endl;
		ctx.indent(indent) << "{" << special_type << "}"
											 << (int)cs.special_type() << ":"
											 << cb.size() << ":"
											 << cb.size_refs() 
											 << "[" << cs.as_bitslice().to_hex() << "]" << endl;
		ctx.indent(indent) << "------------------- " << name << " std cell end -------------------" << endl;
		// minify-remove:end
		return cb.finalize(special_type!=0);
	}

	Ref<Cell> make_opt_cell(ParseContext& ctx, int indent = 0) {
		// minify-remove
		ctx.indent(indent) << "------------------- " << name << " opt cell begin -------------------" << endl;
		CellBuilder cb;
		pack_opt(ctx, cb, indent);
		// minify-remove:start
		auto cs = cb.as_cellslice();
		ctx.indent(indent) << "------------------- " << name << " opt cell final -------------------" << endl;
		ctx.indent(indent) << "{" << special_type << "}"
											 << (int)cs.special_type() << ":"
											 << cb.size() << ":"
											 << cb.size_refs() 
											 << "[" << cs.as_bitslice().to_hex() << "]" << endl;
		ctx.indent(indent) << "------------------- " << name << " opt cell end -------------------" << endl;
		// minify-remove:end
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
		// minify-remove
		CHECK(!cs.empty_ext());
		ccs = cs;
		cs.advance(cs.size());
		cs.advance_refs(cs.size_refs());
		// minify-remove
		CHECK(cs.empty_ext());
	}

	void append_remaining(CellBuilder& cb) {
		// minify-remove
		CHECK(!ccs.empty_ext());
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

/// HashmapAug 


template <class TValue, class TExtra>
struct FullHashmapAug;


template <class TValue, class TExtra>
struct FullHashmapAugNode : BaseFullCell<block::gen::HashmapAugNode>
{
	/*

	ahmn_leaf#_
		{X:Type}
		{Y:Type}

		extra:Y
		value:X
	= HashmapAugNode 0 X Y;

	ahmn_fork#_
		{n:#}
		{X:Type}
		{Y:Type}

		left:^(HashmapAug n X Y)
		right:^(HashmapAug n X Y)
		extra:Y
	= HashmapAugNode (n + 1) X Y;

	*/

	int tag = -1;
	int n = -1;
	Ref<FullHashmapAug<TValue, TExtra>> left;
	Ref<FullHashmapAug<TValue, TExtra>> right;

	TValue value;
	TExtra extra;

	FullHashmapAugNode(int m,  const TLB &X, const TLB &Y) : BaseFullCell("HashmapAugNode", block::gen::HashmapAugNode(m, X, Y)) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = type.check_tag(cs);
		// minify-remove
		CHECK(tag == block::gen::HashmapAugNode::ahmn_leaf || tag == block::gen::HashmapAugNode::ahmn_fork);

		if (tag == 0) // HashmapAugNode::ahmn_leaf
		{
			// minify-remove
			CHECK(type.m_ == 0);
			extra.unpack_std(ctx, cs, indent + 1);
			value.unpack_std(ctx, cs, indent + 1);
			// minify-remove
			CHECK(cs.empty_ext());
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

			// minify-remove
			CHECK(cs.empty_ext());
		}
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		// minify-remove
		CHECK(tag == HashmapAugNode::ahmn_leaf || tag == HashmapAugNode::ahmn_fork);

		if (tag == 0) // HashmapAugNode::ahmn_leaf
		{
			// minify-remove
			CHECK(type.m_ == 0);
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
		// minify-remove
		CHECK(tag == HashmapAugNode::ahmn_leaf || tag == HashmapAugNode::ahmn_fork);

		if (tag == 0) // HashmapAugNode::ahmn_leaf
		{
			// minify-remove
			CHECK(type.m_ == 0);
			value.unpack_opt(ctx, cs, indent + 1);
			auto extra_cs = value.calc_aug_data();
			extra.unpack_opt(ctx, extra_cs, indent + 1, true);
			// minify-remove
			CHECK(cs.empty_ext());
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
		// minify-remove
		CHECK(tag == HashmapAugNode::ahmn_leaf || tag == HashmapAugNode::ahmn_fork);

		if (tag == 0) // HashmapAugNode::ahmn_leaf
		{
			// minify-remove
			CHECK(type.m_ == 0);
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
	/*

	ahm_edge#_
		{n:#}
		{X:Type}
		{Y:Type}
		{l:#}
		{m:#}
		label:(HmLabel ~l n) {n = (~m) + l}
		node:(HashmapAugNode m X Y)
	= HashmapAug n X Y;

	*/
	Ref<CellSlice> label;
	int n, m, l;

	FullHashmapAugNode<TValue, TExtra> node;

	FullHashmapAug(int n, const TLB &X, const TLB &Y) : BaseFullCell("HashmapAug", block::gen::HashmapAug(n, X, Y)), node(n, X, Y) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		n = type.m_;
		// minify-remove
		CHECK(n >= 0);
		CHECK(block::gen::HmLabel{n}.fetch_to(cs, label, l));
		m = n - l;
		node.type.m_ = m;
		node.unpack_std(ctx, cs, indent + 1);
		// minify-remove
		CHECK(cs.empty_ext());
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


/*
_ (HashmapAugE 256 InMsg ImportFees) = InMsgDescr; // done 
_ (HashmapAugE 256 OutMsg CurrencyCollection) = OutMsgDescr; // done 
_ (HashmapAugE 352 EnqueuedMsg uint64) = OutMsgQueue; // within MU
_ (HashmapAugE 256 AccountDispatchQueue uint64) = DispatchQueue; // within MU
_ (HashmapAugE 256 ShardAccount DepthBalanceInfo) = ShardAccounts; // within MU
_ (HashmapAugE 256 AccountBlock CurrencyCollection) = ShardAccountBlocks; // done
_ (HashmapAugE 96 ShardFeeCreated ShardFeeCreated) = ShardFees; // done
_ (HashmapAugE 32 KeyExtBlkRef KeyMaxLt) = OldMcBlocksInfo;
block_create_stats_ext#34 counters:(HashmapAugE 256 CreatorStats uint32) = BlockCreateStats;
*/

template <class TValue, class TExtra>
struct FullHashmapAugE : BaseFullCell<block::gen::HashmapAugE>
{
	/*
	ahme_empty$0
		{n:#}
		{X:Type}
		{Y:Type}

		extra:Y
	= HashmapAugE n X Y;

	ahme_root$1
		{n:#}
		{X:Type}
		{Y:Type}

		root:^(HashmapAug n X Y)
		extra:Y
	= HashmapAugE n X Y;
	*/

	block::gen::HashmapAugE::Record_ahme_root r;

	int tag = -1;
	FullHashmapAug<TValue, TExtra> root;
	TExtra extra;

	FullHashmapAugE(int n, const TLB &X, const TLB &Y) : BaseFullCell("HashmapAugE", block::gen::HashmapAugE(n, X, Y)), root(n, X, Y) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		tag = type.check_tag(cs);
		// minify-remove
		CHECK(tag == block::gen::HashmapAugE::ahme_empty || tag == block::gen::HashmapAugE::ahme_root)

		if (tag == block::gen::HashmapAugE::ahme_empty)
		{
			CHECK(cs.fetch_ulong(1) == 0);
			extra.unpack_std(ctx, cs, indent + 1);
			// minify-remove
			CHECK(cs.empty_ext());
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
		// minify-remove
		CHECK(tag == HashmapAugE::ahme_empty || tag == HashmapAugE::ahme_root)

		if (tag == HashmapAugE::ahme_empty) {
 			cb.store_long(0, 1);
			extra.pack_opt(ctx, cb, indent + 1); // todo don't pack calc automatically
		} else {
			cb.store_long(1, 1).store_ref(root.make_opt_cell(ctx, indent + 1));
		}
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		tag = type.check_tag(cs);
		// minify-remove
		CHECK(tag == HashmapAugE::ahme_empty || tag == HashmapAugE::ahme_root)

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
		// minify-remove
		CHECK(tag == HashmapAugE::ahme_empty || tag == HashmapAugE::ahme_root)

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


/// gen start

// struct FullUInt : BaseFullCell<UInt>, AddValues<UInt> {
// 	FullUInt(string name, UInt type) : BaseFullCell(name, type), AddValues(type) {}
// };

// struct FullUInt64 : FullUInt {
//     FullUInt64() : FullUInt("Unit64", tU64) {}
// };


// struct FullUnit : BaseFullCell<block::gen::Unit> {
//     FullUnit() : BaseFullCell("Unit", block::gen::t_Unit) {}
// };

// struct FullTrue : BaseFullCell<block::gen::True> {
//     FullTrue() : BaseFullCell("True", block::gen::t_True) {}
// };

// struct FullBool : BaseFullCell<block::gen::Bool> {
//     FullBool() : BaseFullCell("Bool", block::gen::t_Bool) {}
// };

// struct FullBoolFalse : BaseFullCell<block::gen::BoolFalse> {
//     FullBoolFalse() : BaseFullCell("BoolFalse", block::gen::t_BoolFalse) {}
// };

// struct FullBoolTrue : BaseFullCell<block::gen::BoolTrue> {
//     FullBoolTrue() : BaseFullCell("BoolTrue", block::gen::t_BoolTrue) {}
// };

template <class T>
struct FullMaybe : BaseFullCell<TLB> {
	// nothing$0 {X:Type} = Maybe X;
	// just$1 {X:Type} value:X = Maybe X;

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
		// minify-remove
		CHECK(tag == 0 || tag == 1);

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
		// minify-remove
		CHECK(tag == 0 || tag == 1);

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

// struct FullEither : BaseFullCell<block::gen::Either> {
//     FullEither() : BaseFullCell("Either", block::gen::t_Either) {}
// };

// struct FullBoth : BaseFullCell<block::gen::Both> {
//     FullBoth() : BaseFullCell("Both", block::gen::t_Both) {}
// };

// struct FullBit : BaseFullCell<block::gen::Bit> {
//     FullBit() : BaseFullCell("Bit", block::gen::t_Bit) {}
// };

// struct FullHashmap : BaseFullCell<block::gen::Hashmap> {
//     FullHashmap() : BaseFullCell("Hashmap", block::gen::t_Hashmap) {}
// };

// struct FullHashmapNode : BaseFullCell<block::gen::HashmapNode> {
//     FullHashmapNode() : BaseFullCell("HashmapNode", block::gen::t_HashmapNode) {}
// };

// struct FullHmLabel : BaseFullCell<block::gen::HmLabel> {
//     FullHmLabel() : BaseFullCell("HmLabel", block::gen::t_HmLabel) {}
// };

// struct FullUnary : BaseFullCell<block::gen::Unary> {
//     FullUnary() : BaseFullCell("Unary", block::gen::t_Unary) {}
// };

// struct FullHashmapE : BaseFullCell<block::gen::HashmapE> {
//     FullHashmapE() : BaseFullCell("HashmapE", block::gen::t_HashmapE) {}
// };

// struct FullBitstringSet : BaseFullCell<block::gen::BitstringSet> {
//     FullBitstringSet() : BaseFullCell("BitstringSet", block::gen::t_BitstringSet) {}
// };

// struct FullVarHashmap : BaseFullCell<block::gen::VarHashmap> {
//     FullVarHashmap() : BaseFullCell("VarHashmap", block::gen::t_VarHashmap) {}
// };

// struct FullVarHashmapNode : BaseFullCell<block::gen::VarHashmapNode> {
//     FullVarHashmapNode() : BaseFullCell("VarHashmapNode", block::gen::t_VarHashmapNode) {}
// };

// struct FullVarHashmapE : BaseFullCell<block::gen::VarHashmapE> {
//     FullVarHashmapE() : BaseFullCell("VarHashmapE", block::gen::t_VarHashmapE) {}
// };

// struct FullPfxHashmap : BaseFullCell<block::gen::PfxHashmap> {
//     FullPfxHashmap() : BaseFullCell("PfxHashmap", block::gen::t_PfxHashmap) {}
// };

// struct FullPfxHashmapNode : BaseFullCell<block::gen::PfxHashmapNode> {
//     FullPfxHashmapNode() : BaseFullCell("PfxHashmapNode", block::gen::t_PfxHashmapNode) {}
// };

// struct FullPfxHashmapE : BaseFullCell<block::gen::PfxHashmapE> {
//     FullPfxHashmapE() : BaseFullCell("PfxHashmapE", block::gen::t_PfxHashmapE) {}
// };

// struct FullMsgAddressExt : BaseFullCell<block::gen::MsgAddressExt> {
//     FullMsgAddressExt() : BaseFullCell("MsgAddressExt", block::gen::t_MsgAddressExt) {}
// };

// struct FullAnycast : BaseFullCell<block::gen::Anycast> {
//     FullAnycast() : BaseFullCell("Anycast", block::gen::t_Anycast) {}
// };

// struct FullMsgAddressInt : BaseFullCell<block::gen::MsgAddressInt> {
//     FullMsgAddressInt() : BaseFullCell("MsgAddressInt", block::gen::t_MsgAddressInt) {}
// };

// struct FullMsgAddress : BaseFullCell<block::gen::MsgAddress> {
//     FullMsgAddress() : BaseFullCell("MsgAddress", block::gen::t_MsgAddress) {}
// };

// struct FullVarUInteger : BaseFullCell<block::gen::VarUInteger> {
//     FullVarUInteger() : BaseFullCell("VarUInteger", block::gen::t_VarUInteger) {}
// };

// struct FullVarInteger : BaseFullCell<block::gen::VarInteger> {
//     FullVarInteger() : BaseFullCell("VarInteger", block::gen::t_VarInteger) {}
// };

// struct FullGrams : BaseFullCell<block::gen::Grams> {
//     FullGrams() : BaseFullCell("Grams", block::gen::t_Grams) {}
// };

// struct FullCoins : BaseFullCell<block::gen::Coins> {
//     FullCoins() : BaseFullCell("Coins", block::gen::t_Coins) {}
// };

// struct FullExtraCurrencyCollection : BaseFullCell<block::gen::ExtraCurrencyCollection> {
//     FullExtraCurrencyCollection() : BaseFullCell("ExtraCurrencyCollection", block::gen::t_ExtraCurrencyCollection) {}
// };

// struct FullCommonMsgInfo : BaseFullCell<block::gen::CommonMsgInfo> {
//     FullCommonMsgInfo() : BaseFullCell("CommonMsgInfo", block::gen::t_CommonMsgInfo) {}
// };

// struct FullCommonMsgInfoRelaxed : BaseFullCell<block::gen::CommonMsgInfoRelaxed> {
//     FullCommonMsgInfoRelaxed() : BaseFullCell("CommonMsgInfoRelaxed", block::gen::t_CommonMsgInfoRelaxed) {}
// };

// struct FullTickTock : BaseFullCell<block::gen::TickTock> {
//     FullTickTock() : BaseFullCell("TickTock", block::gen::t_TickTock) {}
// };

// struct FullStateInit : BaseFullCell<block::gen::StateInit> {
//     FullStateInit() : BaseFullCell("StateInit", block::gen::t_StateInit) {}
// };

// struct FullStateInitWithLibs : BaseFullCell<block::gen::StateInitWithLibs> {
//     FullStateInitWithLibs() : BaseFullCell("StateInitWithLibs", block::gen::t_StateInitWithLibs) {}
// };

// struct FullSimpleLib : BaseFullCell<block::gen::SimpleLib> {
//     FullSimpleLib() : BaseFullCell("SimpleLib", block::gen::t_SimpleLib) {}
// };

// struct FullMessage : BaseFullCell<block::gen::Message> {
//     FullMessage() : BaseFullCell("Message", block::gen::t_Message) {}
// };

// struct FullMessageRelaxed : BaseFullCell<block::gen::MessageRelaxed> {
//     FullMessageRelaxed() : BaseFullCell("MessageRelaxed", block::gen::t_MessageRelaxed) {}
// };

// struct FullMessageAny : BaseFullCell<block::gen::MessageAny> {
//     FullMessageAny() : BaseFullCell("MessageAny", block::gen::t_MessageAny) {}
// };

// struct FullIntermediateAddress : BaseFullCell<block::gen::IntermediateAddress> {
//     FullIntermediateAddress() : BaseFullCell("IntermediateAddress", block::gen::t_IntermediateAddress) {}
// };

// struct FullMsgMetadata : BaseFullCell<block::gen::MsgMetadata> {
//     FullMsgMetadata() : BaseFullCell("MsgMetadata", block::gen::t_MsgMetadata) {}
// };

// struct FullMsgEnvelope : BaseFullCell<block::gen::MsgEnvelope> {
//     FullMsgEnvelope() : BaseFullCell("MsgEnvelope", block::gen::t_MsgEnvelope) {}
// };

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

// Disabled as part of merkle update
//
// struct EnqueuedMsgAug : FullUInt64 {
// 	CellSlice add_values(CellSlice& cs1, CellSlice& cs2) override {
// 		CellBuilder cb;
// 		unsigned long long x, y;
// 		CHECK(
// 			cs1.fetch_ulong_bool(64, x) && 
// 			cs2.fetch_ulong_bool(64, y) &&
//       cb.store_ulong_rchk_bool(std::min(x, y), 64)
// 		);
// 		return cb.as_cellslice();
// 	}
// };


// Disabled as part of merkle update
//
// struct FullEnqueuedMsg : BaseFullCell<EnqueuedMsg>, AugDataProvider {
//     FullEnqueuedMsg() : BaseFullCell("EnqueuedMsg") {}
//
// 	CellSlice calc_aug_data() override {
// 		CellBuilder cb;
// 		auto cs_copy = std_cell_cs;
// 		CHECK(Aug_OutMsgQueue().eval_leaf(cb, cs_copy));
// 		return cb.as_cellslice();
// 	}
// };

// struct FullProcessedUpto : BaseFullCell<block::gen::ProcessedUpto> {
//     FullProcessedUpto() : BaseFullCell("ProcessedUpto", block::gen::t_ProcessedUpto) {}
// };

// struct FullProcessedInfo : BaseFullCell<block::gen::ProcessedInfo> {
//     FullProcessedInfo() : BaseFullCell("ProcessedInfo", block::gen::t_ProcessedInfo) {}
// };

// struct FullIhrPendingSince : BaseFullCell<block::gen::IhrPendingSince> {
//     FullIhrPendingSince() : BaseFullCell("IhrPendingSince", block::gen::t_IhrPendingSince) {}
// };

// struct FullIhrPendingInfo : BaseFullCell<block::gen::IhrPendingInfo> {
//     FullIhrPendingInfo() : BaseFullCell("IhrPendingInfo", block::gen::t_IhrPendingInfo) {}
// };

// struct FullAccountDispatchQueue : BaseFullCell<block::gen::AccountDispatchQueue> {
//     FullAccountDispatchQueue() : BaseFullCell("AccountDispatchQueue", block::gen::t_AccountDispatchQueue) {}
// };

// struct FullDispatchQueue : BaseFullCell<block::gen::DispatchQueue> {
//     FullDispatchQueue() : BaseFullCell("DispatchQueue", block::gen::t_DispatchQueue) {}
// };

// struct FullOutMsgQueueExtra : BaseFullCell<block::gen::OutMsgQueueExtra> {
//     FullOutMsgQueueExtra() : BaseFullCell("OutMsgQueueExtra", block::gen::t_OutMsgQueueExtra) {}
// };


// Disabled as part of merkle update
//
// struct FullOutMsgQueue : BaseFullCell<OutMsgQueue> {
// 	/* 
// 	_ (HashmapAugE 352 EnqueuedMsg uint64) = OutMsgQueue;
//
// 	*/
//
// 	FullHashmapAugE<FullEnqueuedMsg, EnqueuedMsgAug> x{352, tEM, tU64};
//
// 	FullOutMsgQueue(): BaseFullCell("OutMsgQueue", OutMsgQueue()) {}
//
// 	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
// 		x.unpack_std(ctx, cs, indent + 1);
// 	}
//
// 	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
// 		x.pack_opt(ctx, cb, indent + 1);
// 	}
//
// 	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
// 		x.unpack_opt(ctx, cs, indent + 1);
// 	}
//
// 	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
// 		x.pack_std(ctx, cb, indent + 1);
// 	}
// };

// Disabled as part of merkle update
//
// struct FullOutMsgQueueInfo : BaseFullCell<OutMsgQueueInfo> {
// 	/*
//	
// 	_ 
// 		out_queue:OutMsgQueue 
// 		proc_info:ProcessedInfo
//   	extra:(Maybe OutMsgQueueExtra) 
// 	= OutMsgQueueInfo;
// 	*/
//
// 	FullOutMsgQueue out_queue;
//
// 	FullOutMsgQueueInfo() : BaseFullCell("OutMsgQueueInfo") {}
//
//
// 	virtual void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) {
// 		out_queue.unpack_std(ctx, cs, indent + 1);
// 		fetch_remaining(cs);
// 	}
//
// 	virtual void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
// 		out_queue.pack_std(ctx, cb, indent + 1);
// 		append_remaining(cb);
// 	}
// };


// struct FullStorageUsed : BaseFullCell<block::gen::StorageUsed> {
//     FullStorageUsed() : BaseFullCell("StorageUsed", block::gen::t_StorageUsed) {}
// };

// struct FullStorageUsedShort : BaseFullCell<block::gen::StorageUsedShort> {
//     FullStorageUsedShort() : BaseFullCell("StorageUsedShort", block::gen::t_StorageUsedShort) {}
// };

// struct FullStorageInfo : BaseFullCell<block::gen::StorageInfo> {
//     FullStorageInfo() : BaseFullCell("StorageInfo", block::gen::t_StorageInfo) {}
// };

// struct FullAccount : BaseFullCell<block::gen::Account> {
//     FullAccount() : BaseFullCell("Account", block::gen::t_Account) {}
// };

// struct FullAccountStorage : BaseFullCell<block::gen::AccountStorage> {
//     FullAccountStorage() : BaseFullCell("AccountStorage", block::gen::t_AccountStorage) {}
// };

// struct FullAccountState : BaseFullCell<block::gen::AccountState> {
//     FullAccountState() : BaseFullCell("AccountState", block::gen::t_AccountState) {}
// };

// struct FullAccountStatus : BaseFullCell<block::gen::AccountStatus> {
//     FullAccountStatus() : BaseFullCell("AccountStatus", block::gen::t_AccountStatus) {}
// };

// struct FullShardAccount : BaseFullCell<block::gen::ShardAccount> {
//     FullShardAccount() : BaseFullCell("ShardAccount", block::gen::t_ShardAccount) {}
// };

// struct FullDepthBalanceInfo : BaseFullCell<block::gen::DepthBalanceInfo> {
//     FullDepthBalanceInfo() : BaseFullCell("DepthBalanceInfo", block::gen::t_DepthBalanceInfo) {}
// };

// struct FullShardAccounts : BaseFullCell<block::gen::ShardAccounts> {
//     FullShardAccounts() : BaseFullCell("ShardAccounts", block::gen::t_ShardAccounts) {}
// };

// struct FullTransaction_aux : BaseFullCell<block::gen::Transaction_aux> {
//     FullTransaction_aux() : BaseFullCell("Transaction_aux", block::gen::t_Transaction_aux) {}
// };

// struct FullTransaction : BaseFullCell<block::gen::Transaction> {
//     FullTransaction() : BaseFullCell("Transaction", block::gen::t_Transaction) {}
// };

// struct MyMERKLE_UPDATE : block::gen::MERKLE_UPDATE {
// 	MyMERKLE_UPDATE(const TLB& X) : MERKLE_UPDATE(X) {}

// 	bool skip(vm::CellSlice& cs) const override {
// 		return cs.advance_ext(0x208, 2);
//   }
// };

// don't need block::gen::MERKLE_UPDATE here, therefore just using TLB
template <class T>
struct FullMERKLE_UPDATE : BaseFullCell<TLB> {
	/* Optimized by only storing 2 refs instead of tag, hash & levels */
	Ref<T> from_proof, to_proof;

	FullMERKLE_UPDATE(const T &type) : BaseFullCell("MERKLE_UPDATE") {

	}

	void do_unpack_std(ParseContext &ctx, CellSlice &cs, int indent = 0) override {
		CHECK(cs.advance(520)); // 8 + 256 * 2

		from_proof = Ref<T>(true);
		from_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

		to_proof = Ref<T>(true);
		to_proof.write().cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		
		// minify-remove
		CHECK(cs.size_ext() == 0x20); // we don't need to fetch depth
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

// struct FullHASH_UPDATE : BaseFullCell<block::gen::HASH_UPDATE> {
//     FullHASH_UPDATE() : BaseFullCell("HASH_UPDATE", block::gen::t_HASH_UPDATE) {}
// };

// struct FullMERKLE_PROOF : BaseFullCell<block::gen::MERKLE_PROOF> {
//     FullMERKLE_PROOF() : BaseFullCell("MERKLE_PROOF", block::gen::t_MERKLE_PROOF) {}
// };

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

// struct FullTrStoragePhase : BaseFullCell<block::gen::TrStoragePhase> {
//     FullTrStoragePhase() : BaseFullCell("TrStoragePhase", block::gen::t_TrStoragePhase) {}
// };

// struct FullAccStatusChange : BaseFullCell<block::gen::AccStatusChange> {
//     FullAccStatusChange() : BaseFullCell("AccStatusChange", block::gen::t_AccStatusChange) {}
// };

// struct FullTrCreditPhase : BaseFullCell<block::gen::TrCreditPhase> {
//     FullTrCreditPhase() : BaseFullCell("TrCreditPhase", block::gen::t_TrCreditPhase) {}
// };

// struct FullTrComputePhase_aux : BaseFullCell<block::gen::TrComputePhase_aux> {
//     FullTrComputePhase_aux() : BaseFullCell("TrComputePhase_aux", block::gen::t_TrComputePhase_aux) {}
// };

// struct FullTrComputePhase : BaseFullCell<block::gen::TrComputePhase> {
//     FullTrComputePhase() : BaseFullCell("TrComputePhase", block::gen::t_TrComputePhase) {}
// };

// struct FullComputeSkipReason : BaseFullCell<block::gen::ComputeSkipReason> {
//     FullComputeSkipReason() : BaseFullCell("ComputeSkipReason", block::gen::t_ComputeSkipReason) {}
// };

// struct FullTrActionPhase : BaseFullCell<block::gen::TrActionPhase> {
//     FullTrActionPhase() : BaseFullCell("TrActionPhase", block::gen::t_TrActionPhase) {}
// };

// struct FullTrBouncePhase : BaseFullCell<block::gen::TrBouncePhase> {
//     FullTrBouncePhase() : BaseFullCell("TrBouncePhase", block::gen::t_TrBouncePhase) {}
// };

// struct FullSplitMergeInfo : BaseFullCell<block::gen::SplitMergeInfo> {
//     FullSplitMergeInfo() : BaseFullCell("SplitMergeInfo", block::gen::t_SplitMergeInfo) {}
// };

// struct FullTransactionDescr : BaseFullCell<block::gen::TransactionDescr> {
//     FullTransactionDescr() : BaseFullCell("TransactionDescr", block::gen::t_TransactionDescr) {}
// };

// struct FullSmartContractInfo : BaseFullCell<block::gen::SmartContractInfo> {
//     FullSmartContractInfo() : BaseFullCell("SmartContractInfo", block::gen::t_SmartContractInfo) {}
// };

// struct FullOutList : BaseFullCell<block::gen::OutList> {
//     FullOutList() : BaseFullCell("OutList", block::gen::t_OutList) {}
// };

// struct FullLibRef : BaseFullCell<block::gen::LibRef> {
//     FullLibRef() : BaseFullCell("LibRef", block::gen::t_LibRef) {}
// };

// struct FullOutAction : BaseFullCell<block::gen::OutAction> {
//     FullOutAction() : BaseFullCell("OutAction", block::gen::t_OutAction) {}
// };

// struct FullOutListNode : BaseFullCell<block::gen::OutListNode> {
//     FullOutListNode() : BaseFullCell("OutListNode", block::gen::t_OutListNode) {}
// };

// struct FullShardIdent : BaseFullCell<block::gen::ShardIdent> {
//     FullShardIdent() : BaseFullCell("ShardIdent", block::gen::t_ShardIdent) {}
// };

// struct FullExtBlkRef : BaseFullCell<block::gen::ExtBlkRef> {
//     FullExtBlkRef() : BaseFullCell("ExtBlkRef", block::gen::t_ExtBlkRef) {}
// };

// struct FullBlockIdExt : BaseFullCell<block::gen::BlockIdExt> {
//     FullBlockIdExt() : BaseFullCell("BlockIdExt", block::gen::t_BlockIdExt) {}
// };

// struct FullBlkMasterInfo : BaseFullCell<block::gen::BlkMasterInfo> {
//     FullBlkMasterInfo() : BaseFullCell("BlkMasterInfo", block::gen::t_BlkMasterInfo) {}
// };

// struct FullShardStateUnsplit_aux : BaseFullCell<ShardState_aux> {
//     FullShardStateUnsplit_aux() : BaseFullCell("ShardStateUnsplit_aux") {}
// };

// struct MyMcStateExtra : block::gen::McStateExtra {
//   bool skip(vm::CellSlice& cs) const override {
// 		return cs.advance(16)
//       && block::gen::ShardHashes().skip(cs)
//       && cs.advance_ext(0x100, 2)
//       && tCC.skip(cs);
// 	}
// };

// struct FullMcStateExtra : BaseFullCell<MyMcStateExtra> {
//     FullMcStateExtra() : BaseFullCell("McStateExtra", MyMcStateExtra()) {}
// };

// const block::gen::ShardStateUnsplit_aux tSSUa;
// const block::gen::RefT tRMSE{MyMcStateExtra()};
// const block::gen::Maybe tMRMSE{tRMSE};

// struct MyShardStateUnsplit : block::gen::ShardStateUnsplit {
// 	bool skip(vm::CellSlice& cs) const {
// 		return cs.advance_ext(0x169, 3)
// 				&& tMRMSE.skip(cs);
// 	}
// };

// Disabled as part of merkle update
// 
// struct FullShardStateUnsplit : BaseFullCell<MyShardStateUnsplit> {
// 	/*
// 	todo can optimize tag
	
// 	shard_state#9023afe2 
// 		global_id:int32
//   	shard_id:ShardIdent 
//   	seq_no:uint32 
// 		vert_seq_no:#
//   	gen_utime:uint32 
// 		gen_lt:uint64
//   	min_ref_mc_seqno:uint32
//   	out_msg_queue_info:^OutMsgQueueInfo
//   	before_split:(## 1)
//   	accounts:^ShardAccounts
//   	^[ 
// 			overload_history:uint64 
// 			underload_history:uint64
//   		total_balance:CurrencyCollection
//   		total_validator_fees:CurrencyCollection
//   		libraries:(HashmapE 256 LibDescr)
//   		master_ref:(Maybe BlkMasterInfo) 
// 		]
//   	custom:(Maybe ^McStateExtra)
//   = ShardStateUnsplit;

// 	*/
// 	block::gen::ShardStateUnsplit::Record record;

// 	FullShardStateUnsplit_aux aux;

// 	FullOutMsgQueueInfo omqi;
// 	CellSlice ics;

//     FullShardStateUnsplit() : BaseFullCell("ShardStateUnsplit") {}

// 	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
// 		omqi.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

// 		ics = cs;
// 		cs.advance(cs.size());
// 		cs.advance_refs(cs.size_refs());
// 	}

// 	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
// 		cb.store_ref(omqi.make_std_cell(ctx, indent + 1));

// 		cb.append_cellslice(ics);
// 	}
// };



// Disabled as part of merkle update
//
struct FullShardState : BaseFullCell<ShardState>, td::CntObject {
  //   int tag = -1;

	// 	FullShardStateUnsplit shard_state;
	// 	FullShardStateUnsplit left;
	// 	FullShardStateUnsplit right;

		FullShardState() : BaseFullCell("ShardState") {
			
		}

	// void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
	// 	tag = type.get_tag(cs);
	// 	CHECK(tag == type.shard_state || tag == type.split_state);
	// 	if (tag == type.shard_state) {
	// 		shard_state.unpack_std(ctx, cs, indent + 1);
	// 	} else {
	// 		cs.advance(32);
	// 		left.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
	// 		right.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
	// 	}

	// 	// minify-remove
	// 	CHECK(cs.empty_ext());
	// }

	// void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
	// 	CHECK(tag == type.shard_state || tag == type.split_state);

	// 	if (tag == type.shard_state) {
	// 		shard_state.pack_std(ctx, cb, indent + 1);
	// 	} else {
	// 		cb.store_long(type.split_state, 32)
	// 		.store_ref(left.make_std_cell(ctx, indent + 1))
	// 		.store_ref(right.make_std_cell(ctx, indent + 1));
	// 	}
	// }
};

// struct FullLibDescr : BaseFullCell<block::gen::LibDescr> {
//     FullLibDescr() : BaseFullCell("LibDescr", block::gen::t_LibDescr) {}
// };

// struct FullBlockInfo : BaseFullCell<block::gen::BlockInfo> {
//     FullBlockInfo() : BaseFullCell("BlockInfo", block::gen::t_BlockInfo) {}
// };

// struct FullBlkPrevInfo : BaseFullCell<block::gen::BlkPrevInfo> {
//     FullBlkPrevInfo() : BaseFullCell("BlkPrevInfo", block::gen::t_BlkPrevInfo) {}
// };

// struct FullTYPE_1657 : BaseFullCell<block::gen::TYPE_1657> {
//     FullTYPE_1657() : BaseFullCell("TYPE_1657", block::gen::t_TYPE_1657) {}
// };

// struct FullTYPE_1658 : BaseFullCell<block::gen::TYPE_1658> {
//     FullTYPE_1658() : BaseFullCell("TYPE_1658", block::gen::t_TYPE_1658) {}
// };

struct FullValueFlow : BaseFullCell<block::gen::ValueFlow> {
    FullValueFlow() : BaseFullCell("ValueFlow") {}

	unsigned long long tag = -1;

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = cs.fetch_ulong(32);
		fetch_remaining(cs);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long((int)(tag == 0xb8e48dfb), 1);
		append_remaining(cb);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = cs.fetch_long(1) ? 0xb8e48dfb : 0x3ebf98b7;
		fetch_remaining(cs);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(tag, 32);
		append_remaining(cb);
	}
};

// struct FullBinTree : BaseFullCell<block::gen::BinTree> {
//     FullBinTree() : BaseFullCell("BinTree", block::gen::t_BinTree) {}
// };

// struct FullFutureSplitMerge : BaseFullCell<block::gen::FutureSplitMerge> {
//     FullFutureSplitMerge() : BaseFullCell("FutureSplitMerge", block::gen::t_FutureSplitMerge) {}
// };

// struct FullShardDescr_aux : BaseFullCell<block::gen::ShardDescr_aux> {
//     FullShardDescr_aux() : BaseFullCell("ShardDescr_aux", block::gen::t_ShardDescr_aux) {}
// };

// struct FullShardDescr : BaseFullCell<block::gen::ShardDescr> {
//     FullShardDescr() : BaseFullCell("ShardDescr", block::gen::t_ShardDescr) {}
// };

// struct FullShardHashes : BaseFullCell<block::gen::ShardHashes> {
//     FullShardHashes() : BaseFullCell("ShardHashes", block::gen::t_ShardHashes) {}
// };

// struct FullBinTreeAug : BaseFullCell<block::gen::BinTreeAug> {
//     FullBinTreeAug() : BaseFullCell("BinTreeAug", block::gen::t_BinTreeAug) {}
// };

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


// struct FullConfigParams : BaseFullCell<block::gen::ConfigParams> {
//     FullConfigParams() : BaseFullCell("ConfigParams", block::gen::t_ConfigParams) {}
// };

// struct FullValidatorInfo : BaseFullCell<block::gen::ValidatorInfo> {
//     FullValidatorInfo() : BaseFullCell("ValidatorInfo", block::gen::t_ValidatorInfo) {}
// };

// struct FullValidatorBaseInfo : BaseFullCell<block::gen::ValidatorBaseInfo> {
//     FullValidatorBaseInfo() : BaseFullCell("ValidatorBaseInfo", block::gen::t_ValidatorBaseInfo) {}
// };

// struct FullKeyMaxLt : BaseFullCell<block::gen::KeyMaxLt> {
//     FullKeyMaxLt() : BaseFullCell("KeyMaxLt", block::gen::t_KeyMaxLt) {}
// };

// struct FullKeyExtBlkRef : BaseFullCell<block::gen::KeyExtBlkRef> {
//     FullKeyExtBlkRef() : BaseFullCell("KeyExtBlkRef", block::gen::t_KeyExtBlkRef) {}
// };

// struct FullOldMcBlocksInfo : BaseFullCell<block::gen::OldMcBlocksInfo> {
//     FullOldMcBlocksInfo() : BaseFullCell("OldMcBlocksInfo", block::gen::t_OldMcBlocksInfo) {}
// };

// struct FullCounters : BaseFullCell<block::gen::Counters> {
//     FullCounters() : BaseFullCell("Counters", block::gen::t_Counters) {}
// };

// struct FullCreatorStats : BaseFullCell<block::gen::CreatorStats> {
//     FullCreatorStats() : BaseFullCell("CreatorStats", block::gen::t_CreatorStats) {}
// };

// struct FullBlockCreateStats : BaseFullCell<block::gen::BlockCreateStats> {
//     FullBlockCreateStats() : BaseFullCell("BlockCreateStats", block::gen::t_BlockCreateStats) {}
// };

// struct FullMcStateExtra_aux : BaseFullCell<block::gen::McStateExtra_aux> {
//     FullMcStateExtra_aux() : BaseFullCell("McStateExtra_aux", block::gen::t_McStateExtra_aux) {}
// };

// struct FullSigPubKey : BaseFullCell<block::gen::SigPubKey> {
//     FullSigPubKey() : BaseFullCell("SigPubKey", block::gen::t_SigPubKey) {}
// };

// struct FullCryptoSignatureSimple : BaseFullCell<block::gen::CryptoSignatureSimple> {
//     FullCryptoSignatureSimple() : BaseFullCell("CryptoSignatureSimple", block::gen::t_CryptoSignatureSimple) {}
// };

// struct FullCryptoSignaturePair : BaseFullCell<block::gen::CryptoSignaturePair> {
//     FullCryptoSignaturePair() : BaseFullCell("CryptoSignaturePair", block::gen::t_CryptoSignaturePair) {}
// };

// struct FullCertificate : BaseFullCell<block::gen::Certificate> {
//     FullCertificate() : BaseFullCell("Certificate", block::gen::t_Certificate) {}
// };

// struct FullCertificateEnv : BaseFullCell<block::gen::CertificateEnv> {
//     FullCertificateEnv() : BaseFullCell("CertificateEnv", block::gen::t_CertificateEnv) {}
// };

// struct FullSignedCertificate : BaseFullCell<block::gen::SignedCertificate> {
//     FullSignedCertificate() : BaseFullCell("SignedCertificate", block::gen::t_SignedCertificate) {}
// };

// struct FullCryptoSignature : BaseFullCell<block::gen::CryptoSignature> {
//     FullCryptoSignature() : BaseFullCell("CryptoSignature", block::gen::t_CryptoSignature) {}
// };

// struct FullMcBlockExtra_aux : BaseFullCell<block::gen::McBlockExtra_aux> {
//     FullMcBlockExtra_aux() : BaseFullCell("McBlockExtra_aux", block::gen::t_McBlockExtra_aux) {}
// };

const block::gen::ShardHashes tSH;

struct FullMcBlockExtra : BaseFullCell<block::gen::McBlockExtra> {
	/*

	masterchain_block_extra#cca5
  key_block:(## 1)
  shard_hashes:ShardHashes
  shard_fees:ShardFees
  ^[ prev_blk_signatures:(HashmapE 16 CryptoSignaturePair)
     recover_create_msg:(Maybe ^InMsg)
     mint_msg:(Maybe ^InMsg) ]
  config:key_block?ConfigParams
= McBlockExtra;
	
	*/
block::gen::McBlockExtra::Record record;

FullShardFees shard_fees;

FullMcBlockExtra() : BaseFullCell("McBlockExtra") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(cs.fetch_ulong(16) == 0xcca5);
		CHECK(cs.fetch_bool_to(record.key_block)); 
		CHECK(tSH.fetch_to(cs, record.shard_hashes));
		shard_fees.unpack_std(ctx, cs, indent + 1);
		fetch_remaining(cs);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(record.key_block, 1);
		tSH.store_from(cb, record.shard_hashes);
		shard_fees.pack_opt(ctx, cb, indent + 1);
		append_remaining(cb);
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(cs.fetch_bool_to(record.key_block)); 
		CHECK(tSH.fetch_to(cs, record.shard_hashes));
		shard_fees.unpack_opt(ctx, cs, indent + 1);
		fetch_remaining(cs);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(0xcca5, 16).store_long(record.key_block, 1);
		tSH.store_from(cb, record.shard_hashes);
		shard_fees.pack_std(ctx, cb, indent + 1);
		append_remaining(cb);
	}
};

// struct FullValidatorDescr : BaseFullCell<block::gen::ValidatorDescr> {
//     FullValidatorDescr() : BaseFullCell("ValidatorDescr", block::gen::t_ValidatorDescr) {}
// };

// struct FullValidatorSet : BaseFullCell<block::gen::ValidatorSet> {
//     FullValidatorSet() : BaseFullCell("ValidatorSet", block::gen::t_ValidatorSet) {}
// };

// struct FullBurningConfig : BaseFullCell<block::gen::BurningConfig> {
//     FullBurningConfig() : BaseFullCell("BurningConfig", block::gen::t_BurningConfig) {}
// };

// struct FullGlobalVersion : BaseFullCell<block::gen::GlobalVersion> {
//     FullGlobalVersion() : BaseFullCell("GlobalVersion", block::gen::t_GlobalVersion) {}
// };

// struct FullConfigProposalSetup : BaseFullCell<block::gen::ConfigProposalSetup> {
//     FullConfigProposalSetup() : BaseFullCell("ConfigProposalSetup", block::gen::t_ConfigProposalSetup) {}
// };

// struct FullConfigVotingSetup : BaseFullCell<block::gen::ConfigVotingSetup> {
//     FullConfigVotingSetup() : BaseFullCell("ConfigVotingSetup", block::gen::t_ConfigVotingSetup) {}
// };

// struct FullConfigProposal : BaseFullCell<block::gen::ConfigProposal> {
//     FullConfigProposal() : BaseFullCell("ConfigProposal", block::gen::t_ConfigProposal) {}
// };

// struct FullConfigProposalStatus : BaseFullCell<block::gen::ConfigProposalStatus> {
//     FullConfigProposalStatus() : BaseFullCell("ConfigProposalStatus", block::gen::t_ConfigProposalStatus) {}
// };

// struct FullWorkchainFormat : BaseFullCell<block::gen::WorkchainFormat> {
//     FullWorkchainFormat() : BaseFullCell("WorkchainFormat", block::gen::t_WorkchainFormat) {}
// };

// struct FullWcSplitMergeTimings : BaseFullCell<block::gen::WcSplitMergeTimings> {
//     FullWcSplitMergeTimings() : BaseFullCell("WcSplitMergeTimings", block::gen::t_WcSplitMergeTimings) {}
// };

// struct FullWorkchainDescr : BaseFullCell<block::gen::WorkchainDescr> {
//     FullWorkchainDescr() : BaseFullCell("WorkchainDescr", block::gen::t_WorkchainDescr) {}
// };

// struct FullComplaintPricing : BaseFullCell<block::gen::ComplaintPricing> {
//     FullComplaintPricing() : BaseFullCell("ComplaintPricing", block::gen::t_ComplaintPricing) {}
// };

// struct FullBlockCreateFees : BaseFullCell<block::gen::BlockCreateFees> {
//     FullBlockCreateFees() : BaseFullCell("BlockCreateFees", block::gen::t_BlockCreateFees) {}
// };

// struct FullStoragePrices : BaseFullCell<block::gen::StoragePrices> {
//     FullStoragePrices() : BaseFullCell("StoragePrices", block::gen::t_StoragePrices) {}
// };

// struct FullGasLimitsPrices : BaseFullCell<block::gen::GasLimitsPrices> {
//     FullGasLimitsPrices() : BaseFullCell("GasLimitsPrices", block::gen::t_GasLimitsPrices) {}
// };

// struct FullParamLimits : BaseFullCell<block::gen::ParamLimits> {
//     FullParamLimits() : BaseFullCell("ParamLimits", block::gen::t_ParamLimits) {}
// };

// struct FullBlockLimits : BaseFullCell<block::gen::BlockLimits> {
//     FullBlockLimits() : BaseFullCell("BlockLimits", block::gen::t_BlockLimits) {}
// };

// struct FullMsgForwardPrices : BaseFullCell<block::gen::MsgForwardPrices> {
//     FullMsgForwardPrices() : BaseFullCell("MsgForwardPrices", block::gen::t_MsgForwardPrices) {}
// };

// struct FullCatchainConfig : BaseFullCell<block::gen::CatchainConfig> {
//     FullCatchainConfig() : BaseFullCell("CatchainConfig", block::gen::t_CatchainConfig) {}
// };

// struct FullConsensusConfig : BaseFullCell<block::gen::ConsensusConfig> {
//     FullConsensusConfig() : BaseFullCell("ConsensusConfig", block::gen::t_ConsensusConfig) {}
// };

// struct FullValidatorTempKey : BaseFullCell<block::gen::ValidatorTempKey> {
//     FullValidatorTempKey() : BaseFullCell("ValidatorTempKey", block::gen::t_ValidatorTempKey) {}
// };

// struct FullValidatorSignedTempKey : BaseFullCell<block::gen::ValidatorSignedTempKey> {
//     FullValidatorSignedTempKey() : BaseFullCell("ValidatorSignedTempKey", block::gen::t_ValidatorSignedTempKey) {}
// };

// struct FullMisbehaviourPunishmentConfig : BaseFullCell<block::gen::MisbehaviourPunishmentConfig> {
//     FullMisbehaviourPunishmentConfig() : BaseFullCell("MisbehaviourPunishmentConfig", block::gen::t_MisbehaviourPunishmentConfig) {}
// };

// struct FullSizeLimitsConfig : BaseFullCell<block::gen::SizeLimitsConfig> {
//     FullSizeLimitsConfig() : BaseFullCell("SizeLimitsConfig", block::gen::t_SizeLimitsConfig) {}
// };

// struct FullSuspendedAddressList : BaseFullCell<block::gen::SuspendedAddressList> {
//     FullSuspendedAddressList() : BaseFullCell("SuspendedAddressList", block::gen::t_SuspendedAddressList) {}
// };

// struct FullPrecompiledSmc : BaseFullCell<block::gen::PrecompiledSmc> {
//     FullPrecompiledSmc() : BaseFullCell("PrecompiledSmc", block::gen::t_PrecompiledSmc) {}
// };

// struct FullPrecompiledContractsConfig : BaseFullCell<block::gen::PrecompiledContractsConfig> {
//     FullPrecompiledContractsConfig() : BaseFullCell("PrecompiledContractsConfig", block::gen::t_PrecompiledContractsConfig) {}
// };

// struct FullOracleBridgeParams : BaseFullCell<block::gen::OracleBridgeParams> {
//     FullOracleBridgeParams() : BaseFullCell("OracleBridgeParams", block::gen::t_OracleBridgeParams) {}
// };

// struct FullJettonBridgePrices : BaseFullCell<block::gen::JettonBridgePrices> {
//     FullJettonBridgePrices() : BaseFullCell("JettonBridgePrices", block::gen::t_JettonBridgePrices) {}
// };

// struct FullJettonBridgeParams : BaseFullCell<block::gen::JettonBridgeParams> {
//     FullJettonBridgeParams() : BaseFullCell("JettonBridgeParams", block::gen::t_JettonBridgeParams) {}
// };

// // struct FullConfigParam : BaseFullCell<block::gen::ConfigParam> {
// //     FullConfigParam() : BaseFullCell("ConfigParam", block::gen::t_ConfigParam) {}
// // };

// struct FullBlockSignaturesPure : BaseFullCell<block::gen::BlockSignaturesPure> {
//     FullBlockSignaturesPure() : BaseFullCell("BlockSignaturesPure", block::gen::t_BlockSignaturesPure) {}
// };

// struct FullBlockSignatures : BaseFullCell<block::gen::BlockSignatures> {
//     FullBlockSignatures() : BaseFullCell("BlockSignatures", block::gen::t_BlockSignatures) {}
// };

// struct FullBlockProof : BaseFullCell<block::gen::BlockProof> {
//     FullBlockProof() : BaseFullCell("BlockProof", block::gen::t_BlockProof) {}
// };

// // struct FullProofChain : BaseFullCell<block::gen::ProofChain> {
// //     FullProofChain() : BaseFullCell("ProofChain", block::gen::t_ProofChain) {}
// // };

// struct FullTopBlockDescr : BaseFullCell<block::gen::TopBlockDescr> {
//     FullTopBlockDescr() : BaseFullCell("TopBlockDescr", block::gen::t_TopBlockDescr) {}
// };

// struct FullTopBlockDescrSet : BaseFullCell<block::gen::TopBlockDescrSet> {
//     FullTopBlockDescrSet() : BaseFullCell("TopBlockDescrSet", block::gen::t_TopBlockDescrSet) {}
// };

// struct FullProducerInfo : BaseFullCell<block::gen::ProducerInfo> {
//     FullProducerInfo() : BaseFullCell("ProducerInfo", block::gen::t_ProducerInfo) {}
// };

// struct FullComplaintDescr : BaseFullCell<block::gen::ComplaintDescr> {
//     FullComplaintDescr() : BaseFullCell("ComplaintDescr", block::gen::t_ComplaintDescr) {}
// };

// struct FullValidatorComplaint : BaseFullCell<block::gen::ValidatorComplaint> {
//     FullValidatorComplaint() : BaseFullCell("ValidatorComplaint", block::gen::t_ValidatorComplaint) {}
// };

// struct FullValidatorComplaintStatus : BaseFullCell<block::gen::ValidatorComplaintStatus> {
//     FullValidatorComplaintStatus() : BaseFullCell("ValidatorComplaintStatus", block::gen::t_ValidatorComplaintStatus) {}
// };

// struct FullVmCellSlice : BaseFullCell<block::gen::VmCellSlice> {
//     FullVmCellSlice() : BaseFullCell("VmCellSlice", block::gen::t_VmCellSlice) {}
// };

// // struct FullVmTupleRef : BaseFullCell<block::gen::VmTupleRef> {
// //     FullVmTupleRef() : BaseFullCell("VmTupleRef", block::gen::t_VmTupleRef) {}
// // };

// // struct FullVmTuple : BaseFullCell<block::gen::VmTuple> {
// //     FullVmTuple() : BaseFullCell("VmTuple", block::gen::t_VmTuple) {}
// // };

// struct FullVmStackValue : BaseFullCell<block::gen::VmStackValue> {
//     FullVmStackValue() : BaseFullCell("VmStackValue", block::gen::t_VmStackValue) {}
// };

// struct FullVmStack : BaseFullCell<block::gen::VmStack> {
//     FullVmStack() : BaseFullCell("VmStack", block::gen::t_VmStack) {}
// };

// // struct FullVmStackList : BaseFullCell<block::gen::VmStackList> {
// //     FullVmStackList() : BaseFullCell("VmStackList", block::gen::t_VmStackList) {}
// // };

// struct FullVmSaveList : BaseFullCell<block::gen::VmSaveList> {
//     FullVmSaveList() : BaseFullCell("VmSaveList", block::gen::t_VmSaveList) {}
// };

// struct FullVmGasLimits_aux : BaseFullCell<block::gen::VmGasLimits_aux> {
//     FullVmGasLimits_aux() : BaseFullCell("VmGasLimits_aux", block::gen::t_VmGasLimits_aux) {}
// };

// struct FullVmGasLimits : BaseFullCell<block::gen::VmGasLimits> {
//     FullVmGasLimits() : BaseFullCell("VmGasLimits", block::gen::t_VmGasLimits) {}
// };

// struct FullVmLibraries : BaseFullCell<block::gen::VmLibraries> {
//     FullVmLibraries() : BaseFullCell("VmLibraries", block::gen::t_VmLibraries) {}
// };

// struct FullVmControlData : BaseFullCell<block::gen::VmControlData> {
//     FullVmControlData() : BaseFullCell("VmControlData", block::gen::t_VmControlData) {}
// };

// struct FullVmCont : BaseFullCell<block::gen::VmCont> {
//     FullVmCont() : BaseFullCell("VmCont", block::gen::t_VmCont) {}
// };

// struct FullDNS_RecordSet : BaseFullCell<block::gen::DNS_RecordSet> {
//     FullDNS_RecordSet() : BaseFullCell("DNS_RecordSet", block::gen::t_DNS_RecordSet) {}
// };

// // struct FullTextChunkRef : BaseFullCell<block::gen::TextChunkRef> {
// //     FullTextChunkRef() : BaseFullCell("TextChunkRef", block::gen::t_TextChunkRef) {}
// // };

// // struct FullTextChunks : BaseFullCell<block::gen::TextChunks> {
// //     FullTextChunks() : BaseFullCell("TextChunks", block::gen::t_TextChunks) {}
// // };

// struct FullText : BaseFullCell<block::gen::Text> {
//     FullText() : BaseFullCell("Text", block::gen::t_Text) {}
// };

// struct FullProtoList : BaseFullCell<block::gen::ProtoList> {
//     FullProtoList() : BaseFullCell("ProtoList", block::gen::t_ProtoList) {}
// };

// struct FullProtocol : BaseFullCell<block::gen::Protocol> {
//     FullProtocol() : BaseFullCell("Protocol", block::gen::t_Protocol) {}
// };

// struct FullSmcCapList : BaseFullCell<block::gen::SmcCapList> {
//     FullSmcCapList() : BaseFullCell("SmcCapList", block::gen::t_SmcCapList) {}
// };

// struct FullSmcCapability : BaseFullCell<block::gen::SmcCapability> {
//     FullSmcCapability() : BaseFullCell("SmcCapability", block::gen::t_SmcCapability) {}
// };

// struct FullDNSRecord : BaseFullCell<block::gen::DNSRecord> {
//     FullDNSRecord() : BaseFullCell("DNSRecord", block::gen::t_DNSRecord) {}
// };

// struct FullChanConfig : BaseFullCell<block::gen::ChanConfig> {
//     FullChanConfig() : BaseFullCell("ChanConfig", block::gen::t_ChanConfig) {}
// };

// struct FullChanState : BaseFullCell<block::gen::ChanState> {
//     FullChanState() : BaseFullCell("ChanState", block::gen::t_ChanState) {}
// };

// struct FullChanPromise : BaseFullCell<block::gen::ChanPromise> {
//     FullChanPromise() : BaseFullCell("ChanPromise", block::gen::t_ChanPromise) {}
// };

// struct FullChanSignedPromise : BaseFullCell<block::gen::ChanSignedPromise> {
//     FullChanSignedPromise() : BaseFullCell("ChanSignedPromise", block::gen::t_ChanSignedPromise) {}
// };

// struct FullChanMsg : BaseFullCell<block::gen::ChanMsg> {
//     FullChanMsg() : BaseFullCell("ChanMsg", block::gen::t_ChanMsg) {}
// };

// struct FullChanSignedMsg : BaseFullCell<block::gen::ChanSignedMsg> {
//     FullChanSignedMsg() : BaseFullCell("ChanSignedMsg", block::gen::t_ChanSignedMsg) {}
// };

// struct FullChanOp : BaseFullCell<block::gen::ChanOp> {
//     FullChanOp() : BaseFullCell("ChanOp", block::gen::t_ChanOp) {}
// };

// struct FullChanData : BaseFullCell<block::gen::ChanData> {
//     FullChanData() : BaseFullCell("ChanData", block::gen::t_ChanData) {}
// };


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


/// gen end



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
	/*

	block_extra
		in_msg_descr:^InMsgDescr
		out_msg_descr:^OutMsgDescr
		account_blocks:^ShardAccountBlocks
		rand_seed:bits256
		created_by:bits256
		custom:(Maybe ^McBlockExtra)
	= BlockExtra;

	*/
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
	int global_id = -1;
	Ref<Cell> info;
	FullValueFlow value_flow;
	FullMERKLE_UPDATE<FullShardState> state_update;
	FullBlockExtra extra;

	FullBlock(): BaseFullCell("Block"), state_update(FullShardState()) {

	}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		CHECK(cs.advance(32));
		CHECK(cs.fetch_int_to(32, global_id));
		CHECK(cs.fetch_ref_to(info));
		value_flow.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		state_update.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		extra.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		// minify-remove
		CHECK(cs.empty_ext());
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{
		// todo store bit (in case of main) or long for id;
		cb.store_long(global_id, 32)
				.store_ref(info)
				.store_ref(value_flow.make_opt_cell(ctx, indent + 1))
				.store_ref(state_update.make_opt_cell(ctx, indent + 1))
				.store_ref(extra.make_opt_cell(ctx, indent + 1));
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		CHECK(
			cs.fetch_int_to(32, global_id)
      && cs.fetch_ref_to(info)
		);
		value_flow.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		state_update.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		extra.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		// minify-remove
		CHECK(cs.empty_ext());
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{
		cb.store_long(0x11ef55aa, 32)
				.store_long(global_id, 32)
				.store_ref(info)
				.store_ref(value_flow.make_std_cell(ctx, indent + 1))
				.store_ref(state_update.make_std_cell(ctx, indent + 1))
				.store_ref(extra.make_std_cell(ctx, indent + 1));
	}
};

// minify-remove:start

template <class T>
void do_print_delta(T actual, T prev) {
	auto delta = static_cast<make_signed_t<T>>(actual - prev);
	auto pct = prev == 0 ? 0 : static_cast<float>(delta) / prev;

	cout << showpos << (delta) << "; " << FIXED_FLOAT(pct * 100) << "%" << noshowpos;
}

template <class T>
void print_delta(const char* label, T actual, T prev) {
	cout << label << ": " << actual << " (";
	do_print_delta(actual, prev);
	cout << ")" << endl;
}

// minify-remove:end

class NullStream : public ostream {
    class NullBuffer : public streambuf {
    public:
        int overflow( int c ) { return c; }
    } m_nb;
public:
    NullStream() : ostream( &m_nb ) {}
};

#include "7z_impl.cpp"


td::BufferSlice serialize_boc_opt(ostream& out, Ref<Cell> cell) {
	CHECK(!cell.is_null());
  BagOfCells boc;
  boc.add_root(cell);
  CHECK(boc.import_cells().is_ok());
	auto size = boc.estimate_serialized_size(0);
	td::BufferSlice bs{size};
	auto buffer = (unsigned char*)bs.data();

	boc_writers::BufferWriter writer{buffer, buffer + size};

	// calc backrefs to the cell
	vector<int> backrefs(boc.cell_list_.size(), 0);
	for (int i = 0; i < boc.cell_count; ++i) {
    const auto& cell = boc.cell_list_[i];
		for (int j = 0; j < cell.ref_num; ++j) {
			backrefs[cell.ref_idx[j]] += 1;
		}
  }

	auto last_print = writer.store_ptr;
	auto print_writer = [&](string label) {
		// minify-remove:start
		if (last_print < writer.store_ptr) {
			out << label << ": " << td::ConstBitPtr{last_print}.to_hex((writer.store_ptr - last_print) * 8) << endl;
			last_print = writer.store_ptr;
		}
		// minify-remove:end
	};

	int expected_cells_cnt = 0;
  for (int i = 0; i < boc.cell_count; ++i) {
		int k = boc.cell_count - 1 - i;
    const auto& dc_info = boc.cell_list_[k];
		int br = backrefs.at(k);

		if (br != 1) {
			expected_cells_cnt++;
		} 
  }

	int rs = 0;
	while (expected_cells_cnt >= (1LL << (rs << 3))) {
    rs++;
  }

	auto store_byte = [&](unsigned long long value) { writer.store_uint(value, 1); };
	auto store_ref = [&](unsigned long long value) { writer.store_uint(value, rs); };
	auto overwrite_ref = [&](int position, unsigned long long value) { 
		auto ptr = writer.store_ptr;
		writer.store_ptr = writer.store_start + position;
		store_ref(value);
		writer.store_ptr = ptr;
	};

	store_byte(rs);
	print_writer("ref-size");

  store_ref(expected_cells_cnt);
	print_writer("cell-num");

	vector<int> idx_to_ref(boc.cell_list_.size(), -1);
	vector<pair<int, size_t>> refs_to_set;

	function<void(int, const vm::BagOfCells::CellInfo&)> store_cell;
	store_cell = [&](int idx, const vm::BagOfCells::CellInfo& dc_info) { 
		unsigned char buf[256] = {};
    const Ref<DataCell>& dc = dc_info.dc_ref;

		// calc mask, 1 means ref is embedded, 0 means stored as a separate cell
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
				// embed cell
				store_cell(ref_idx, boc.cell_list_[ref_idx]);
			} else {
				// remember to set cell idx later
				refs_to_set.emplace_back(ref_idx, writer.position());
				store_ref(0);
				print_writer("ref");
			}
    }
	};

	// store all cells with 0 (root) or >1 (many parents) backrefs
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

	// replace ref placeholders with actual cell ids
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

	function<void(int)> read_cell;
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

		// correct bits and last byte
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
			// remember top-level cell (ref) to actual cell count
			ref_to_cd_idx.at(idx) = cells_data.size();
		}
		cells_data.push_back(DeserializedCell{
				special, bits, data,
		});
		int dc_idx = cells_data.size() - 1;

		for (int i = 0; i < ref_num; ++i) {
			bool is_embedded = (mask >> i) & 1;
			if (is_embedded) {
				// preserve index of the cell about to be loaded
				cells_data.at(dc_idx).refs.push_back({cells_data.size(), -1});
				read_cell(-1);
			} else {
				// read ref instead and map it later
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
	return sz::compress(data);
}

td::BufferSlice do_decompress(td::Slice data) {
	return sz::decompress(data);
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

// minify-remove:start

void check_decompress() {
	ifstream fin("out.txt");

	string base64_data;
	fin >> base64_data;

	string mode = "decompress";

	td::BufferSlice data(td::base64_decode(base64_data));

	if (mode == "compress") {
		data = compress(data);
	} else {
		data = decompress(data);
	}

	cout << td::str_base64_encode(data) << endl;
}

// minify-remove:end

int main(
	// minify-remove
	int argc, const char * argv[]
)
{
	// minify-remove:start

	// check_decompress();

	// ifstream fin(argc > 1 ? argv[1] : "tests/1-001.txt");
	// ifstream fin(argc > 1 ? argv[1] : "tests/1-006.txt");
	ifstream fin(argc > 1 ? argv[1] : "tests-blocks/43888181.txt");
	ofstream fout_source_cells("analysis-01-source-cells.txt");
	ofstream fout_final_cells("analysis-02-final-cells.txt");
	ofstream fout_parse_std("analysis-03-parse-std.txt");
	ofstream fout_pack_opt("analysis-04-pack-opt.txt");
	ofstream fout_parse_opt("analysis-05-parse-opt.txt");
	ofstream fout_pack_std("analysis-06-pack-std.txt");
	ofstream fout_compressed_b64("analysis-07-compressed-b64.txt");
	ofstream fout_boc_ser("analysis-08-boc-ser.txt");
	ofstream fout_boc_deser("analysis-09-boc-deser.txt");

	string block_base64;
	fin >> block_base64;

	if (block_base64 == "compress")
	{
		fin >> block_base64;
	}

	cout << "Deserializing block from b64..." << endl; 
	auto original_test_block = td::base64_decode(block_base64);

	Ref<Cell> block_root = vm::std_boc_deserialize(original_test_block).move_as_ok();
	auto original_input_size = original_test_block.size();
	cout << "Size: " << original_input_size << endl;

	auto original_block_bin = vm::std_boc_serialize(block_root).move_as_ok();
	auto original_block_base64 = td::str_base64_encode(original_block_bin);
	auto original_size = original_block_bin.size();

	BagOfCells std_boc;
	std_boc.add_root(block_root);
	CHECK(std_boc.import_cells().is_ok());
	cout << "Cells count: " << std_boc.cell_count << endl;

	{
		int max_backrefs = 0;
		std::vector<int> backrefs(std_boc.cell_list_.size(), 0);
		for (const auto& cell: std_boc.cell_list_) {
			for (int j = 0; j < cell.ref_num; ++j) {
				backrefs[cell.ref_idx[j]] += 1;
				max_backrefs = std::max(backrefs[cell.ref_idx[j]], max_backrefs);
			}
		}

		for (int i = 0; i < std_boc.cell_list_.size(); ++i) {
			const auto& cell = std_boc.cell_list_.at(i);
			auto hex = cell.dc_ref->to_hex();
			transform(hex.begin(), hex.end(), hex.begin(), ::toupper);

			fout_source_cells << std::setw(std::ceil(std::log10(max_backrefs)) + 1) << backrefs[i] << " : " 
												<< hex.substr(0, 2) << " " 
												<< hex.substr(2, 2) << " " 
												<< hex.substr(4);
			if (cell.ref_num > 0) {
				fout_source_cells << " (";
				for (int j = 0; j < cell.ref_num; ++j) {
					fout_source_cells << cell.ref_idx.at(j) << (((j + 1) == cell.ref_num) ? ")" : ", ");
				}
			}
			fout_source_cells << endl;
		}
	}

	cout << "\n\nEnabled optimizations: " << enabled_optimizations.size() << endl; 
	for (const auto& x: enabled_optimizations) {
		cout << "+ " << x << endl; 
	}

	cout << "\nLoading standard block..." << endl; 
	FullBlock block;
	ParseContext load_std_ctx{fout_parse_std};
	block.cell_unpack_std(load_std_ctx, block_root, 0, true);

	cout << "\nOptimizing block..." << endl; 
	ParseContext pack_opt_ctx{fout_pack_opt};
	auto opt_block_cell = block.make_opt_cell(pack_opt_ctx);

	BagOfCells opt_boc;
	opt_boc.add_root(opt_block_cell);
	CHECK(opt_boc.import_cells().is_ok());
	print_delta("Optimized block cells count", opt_boc.cell_count, std_boc.cell_count);

	// auto opt_ser = vm::std_boc_serialize(opt_block_cell).move_as_ok();
	auto opt_ser = serialize_boc_opt(fout_boc_ser, opt_block_cell);
	print_delta("Optimized block size", opt_ser.size(), original_size);

	if (argc > 1) {
		auto input_path = string(argv[1]);
		ofstream fout_opt_ser("out/" + input_path + "-opt-ser.txt");
		CHECK(fout_opt_ser.is_open());
		fout_opt_ser << td::str_base64_encode(opt_ser);
	}

	auto compressed = do_compress(opt_ser);
	print_delta("Compressed size", compressed.size(), opt_ser.size());

	if (argc > 1) {
		auto input_path = string(argv[1]);
		ofstream fout_compressed("out/" + input_path + "-compressed.txt");
		CHECK(fout_compressed.is_open());
		fout_compressed << td::str_base64_encode(compressed);
	}

	cout << "Total compression delta: ";
	do_print_delta(compressed.size(), original_input_size);
	cout << endl;

	cout << "Score: " << (1000 * static_cast<float>(original_input_size * 2) / (original_input_size + compressed.size())) << endl;

	auto compressed_b64 = td::str_base64_encode(compressed);
	fout_compressed_b64 << compressed_b64;

	auto compressed_b64_decoded = td::base64_decode(compressed_b64);
	auto decompressed = do_decompress(compressed_b64_decoded);
	// auto opt_deser = vm::std_boc_deserialize(decompressed, false, true).move_as_ok();
	auto opt_deser = deserialize_boc_opt(fout_boc_deser, decompressed);

	cout << "\nLoading optimized block..." << endl; 
	FullBlock opt_block;
	ParseContext parse_opt_ctx{fout_parse_opt};
	opt_block.cell_unpack_opt(parse_opt_ctx, opt_deser, 0, true);

	cout << "\nUn-optimizing block..." << endl; 
	ParseContext pack_std_ctx{fout_pack_std};
	auto un_opt_block_cell = opt_block.make_std_cell(pack_std_ctx);

	BagOfCells un_opt_boc;
	un_opt_boc.add_root(un_opt_block_cell);
	CHECK(un_opt_boc.import_cells().is_ok());
	print_delta("Un-optimized block cells count", un_opt_boc.cell_count, std_boc.cell_count);

	{
		int max_backrefs = 0;
		std::vector<int> backrefs(un_opt_boc.cell_list_.size(), 0);
		for (const auto& cell: un_opt_boc.cell_list_) {
			for (int j = 0; j < cell.ref_num; ++j) {
				backrefs[cell.ref_idx[j]] += 1;
				max_backrefs = std::max(backrefs[cell.ref_idx[j]], max_backrefs);
			}
		}

		for (int i = 0; i < un_opt_boc.cell_list_.size(); ++i) {
			const auto& cell = un_opt_boc.cell_list_.at(i);
			auto hex = cell.dc_ref->to_hex();
			transform(hex.begin(), hex.end(), hex.begin(), ::toupper);

			fout_final_cells << std::setw(std::ceil(std::log10(max_backrefs)) + 1) << backrefs[i] << " : " 
												<< hex.substr(0, 2) << " " 
												<< hex.substr(2, 2) << " " 
												<< hex.substr(4);
			if (cell.ref_num > 0) {
				fout_final_cells << " (";
				for (int j = 0; j < cell.ref_num; ++j) {
					fout_final_cells << cell.ref_idx.at(j) << (((j + 1) == cell.ref_num) ? ")" : ", ");
				}
			}
			fout_final_cells << endl;
		}
	}

	auto un_opt_bin = vm::std_boc_serialize(un_opt_block_cell, 31).move_as_ok();
	print_delta("Un-optimized block size", un_opt_bin.size(), original_input_size);

	auto un_opt_base64 = td::str_base64_encode(un_opt_bin);

	cout << "Matching: " << (un_opt_base64 == block_base64) << endl;

	if (un_opt_base64 != block_base64) {
		for (int i = 0; i < block_base64.size(); ++i) {
			if (block_base64[i] != un_opt_base64.at(i)) {
				cout << "First mismatch starting at " << i << endl;
				break;
			}
		}
	}

	CHECK(un_opt_base64 == block_base64);

	return 0; // there's code below, if you remove that code will be executed

	// ogiinal          229,919
	// lz4              200,368
	// lz4 with zeroing 199,103

	// >>> (2 * 229919) / (229919 + 200368)
	// 1.0686774176305582
	// >>> (2 * 229919) / (229919 + 200368) * 1000
	// 1068.6774176305582
	// >>> (2 * 229919) / (229919 + 199103) * 1000
	// 1071.8284843201513

	// Enabled optimizations: 6
	// + Block
	// + BlockExtra
	// + HashmapAug
	// + HashmapAugE
	// + HashmapAugNode
	// + InMsgDescr

	// Loading standard block...

	// Optimizing block...
	// Optimized block cells count: 7198 (+0; +0.00%)
	// Optimized block size: 228506 (-1413; -0.61%)
	// Compressed size: 198898 (-29608; -12.96%)
	// Score: 1146.21

/*
Enabled optimizations: 0

Loading standard block...

Optimizing block...
Optimized block cells count: 7198 (+0; +0.00%)
Optimized block size: 229919 (+0; +0.00%)
Compressed size: 200368 (-29551; -12.85%)
Total compression delta: -66653; -24.96%
Score: 1142.61
*/

/*

Enabled optimizations: 6
+ Block
+ BlockExtra
+ HashmapAug
+ HashmapAugE
+ HashmapAugNode
+ InMsgDescr

Loading standard block...

Optimizing block...
Optimized block cells count: 7198 (+0; +0.00%)
Optimized block size: 228506 (-1413; -0.61%)
Compressed size: 198898 (-29608; -12.96%)
Total compression delta: -68123; -25.51%
Score: 1146.21

*/

/*
Enabled optimizations: 7
+ Block
+ BlockExtra
+ HashmapAug
+ HashmapAugE
+ HashmapAugNode
+ InMsgDescr
+ OutMsgDescr

Loading standard block...

Optimizing block...
Optimized block cells count: 7198 (+0; +0.00%)
Optimized block size: 227693 (-2226; -0.97%)
Compressed size: 198074 (-29619; -13.01%)
Total compression delta: -68947; -25.82%
Score: 1148.24
*/

	// minify-remove:end

	// minify-remove
	{
		string mode;
		cin >> mode;
		// minify-remove
		CHECK(mode == "compress" || mode == "decompress");

		string base64_data;
		cin >> base64_data;

		// minify-remove
		CHECK(!base64_data.empty());

		td::BufferSlice data(td::base64_decode(base64_data));

		data = (mode == "compress") ? compress(data) : decompress(data);

		cout << td::str_base64_encode(data) << endl;
	// minify-remove
	}
}
