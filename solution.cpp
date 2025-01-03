// minify-remove:start
#define private public
#define FIXED_FLOAT(x) fixed <<setprecision(2)<<(x)
// minify-remove:end
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

	"MERKLE_UPDATE",
	"ShardState",
	"ShardStateUnsplit",
	"OutMsgQueueInfo",
	// "OutMsgQueue", // nothing to compress as it is part of merkle update, many pruned branches

	// "ShardStateUnsplit_aux",
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

		ctx.indent(indent) << (name + ".pack_opt ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << "-:" 
											 << added_bin.length() << ":" 
											 << added_refs 
											 << "[" << bin_to_hex(added_bin) << "]" << endl;
		ctx.indent(indent) << (name + ".pack_opt full ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}" 
											 << (int)new_cs.special_type() << ":" 
											 << cb.size() << ":" 
											 << cb.size_refs() 
											 << "[" << new_cs.as_bitslice().to_hex() << "]" << endl;
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
		ctx.indent(indent) << (name + ".pack_std begin ") 
		    							 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}"
											 << (int)current_cs.special_type() << ":"
											 << cb.size() << ":"
											 << cb.size_refs() 
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
		ctx.indent(indent) << (name + ".pack_std ") 
											 << (e ? "" : "(disabled) ") 
											 << "{" << special_type << "}"
											 << (int)new_cs.special_type() << ":"
											 << cb.size() << ":"
											 << cb.size_refs() 
											 << "[" << new_cs.as_bitslice().to_hex() << "]" << endl;
		ctx.indent(indent) << (name + ".pack_std ") 
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
											 << "[" << cs.as_bitslice().to_binary() << "]" << endl;
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
											 << "[" << cs.as_bitslice().to_binary() << "]" << endl;
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
};

template <class TExtra>
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
const ImportFees tIF;
const InMsg tIM;
const EnqueuedMsg tEM;
const UInt tU64{64};

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
			extra.pack_opt(ctx, cb, indent + 1);
		} else {
			cb.store_long(1, 1).store_ref(root.make_opt_cell(ctx, indent + 1));
			extra.pack_opt(ctx, cb, indent + 1);
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

struct FullUInt : BaseFullCell<UInt>, AddValues<UInt> {
	FullUInt(string name, UInt type) : BaseFullCell(name, type), AddValues(type) {}
};

struct FullUInt64 : FullUInt {
    FullUInt64() : FullUInt("Unit64", tU64) {}
};


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

// struct FullMaybe : BaseFullCell<block::gen::Maybe> {
//     FullMaybe() : BaseFullCell("Maybe", block::gen::t_Maybe) {}
// };

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

struct FullInMsg : BaseFullCell<InMsg>, AugDataProvider<FullImportFees> {
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

struct FullOutMsg : BaseFullCell<OutMsg>, AugDataProvider<FullCurrencyCollection> {
    FullOutMsg() : BaseFullCell("OutMsg", OutMsg()) {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(type.get_export_value(cb, cs_copy));
		return cb.as_cellslice();
	}
};

struct EnqueuedMsgAug : FullUInt64 {
	CellSlice add_values(CellSlice& cs1, CellSlice& cs2) override {
		CellBuilder cb;
		unsigned long long x, y;
		CHECK(
			cs1.fetch_ulong_bool(64, x) && 
			cs2.fetch_ulong_bool(64, y) &&
      cb.store_ulong_rchk_bool(std::min(x, y), 64)
		);
		return cb.as_cellslice();
	}
};

struct FullEnqueuedMsg : BaseFullCell<EnqueuedMsg>, AugDataProvider<EnqueuedMsgAug> {
    FullEnqueuedMsg() : BaseFullCell("EnqueuedMsg") {}

	CellSlice calc_aug_data() override {
		CellBuilder cb;
		auto cs_copy = std_cell_cs;
		CHECK(Aug_OutMsgQueue().eval_leaf(cb, cs_copy));
		return cb.as_cellslice();
	}
};

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

struct FullOutMsgQueue : BaseFullCell<OutMsgQueue> {
	/* 
	_ (HashmapAugE 352 EnqueuedMsg uint64) = OutMsgQueue;

	*/

	FullHashmapAugE<FullEnqueuedMsg, EnqueuedMsgAug> x{352, tEM, tU64};

	FullOutMsgQueue(): BaseFullCell("OutMsgQueue", OutMsgQueue()) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
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


struct FullOutMsgQueueInfo : BaseFullCell<OutMsgQueueInfo> {
	/*
	
	_ 
		out_queue:OutMsgQueue 
		proc_info:ProcessedInfo
  	extra:(Maybe OutMsgQueueExtra) 
	= OutMsgQueueInfo;
	*/

	CellSlice ics;
	FullOutMsgQueue out_queue;

	FullOutMsgQueueInfo() : BaseFullCell("OutMsgQueueInfo") {}


	virtual void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) {
		out_queue.unpack_std(ctx, cs, indent + 1);
		ics = cs;
		cs.advance(cs.size());
		cs.advance_refs(cs.size_refs());
	}

	virtual void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
		out_queue.pack_std(ctx, cb, indent + 1);
		cb.append_cellslice(ics);
	}
};

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

struct MyMERKLE_UPDATE : block::gen::MERKLE_UPDATE {
	MyMERKLE_UPDATE(const TLB& X) : MERKLE_UPDATE(X) {}

	bool skip(vm::CellSlice& cs) const override {
		return cs.advance_ext(0x208, 2);
  }
};

template <class T>
struct FullMERKLE_UPDATE : BaseFullCell<MyMERKLE_UPDATE> {
	/* Optimized by only storing 2 refs instead of tag, hash & levels */
	Ref<T> from_proof;
	Ref<T> to_proof;

	FullMERKLE_UPDATE(const T &type) : BaseFullCell("MERKLE_UPDATE", MyMERKLE_UPDATE(type.type)) {

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

// struct FullAccountBlock : BaseFullCell<block::gen::AccountBlock> {
//     FullAccountBlock() : BaseFullCell("AccountBlock", block::gen::t_AccountBlock) {}
// };

// struct FullShardAccountBlocks : BaseFullCell<block::gen::ShardAccountBlocks> {
//     FullShardAccountBlocks() : BaseFullCell("ShardAccountBlocks", block::gen::t_ShardAccountBlocks) {}
// };

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

struct FullShardStateUnsplit_aux : BaseFullCell<ShardState_aux> {
    FullShardStateUnsplit_aux() : BaseFullCell("ShardStateUnsplit_aux") {}

	CellSlice ics;
	Ref<Cell> r1, r2;

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		r1 = cs.fetch_ref();
		// r2 = cs.fetch_ref();
		ics = cs;
		// minify-remove
		CHECK(cs.size_refs() == 0);
	}

	// // virtual void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) {
	// // 	do_pack_std(ctx, cb, indent);
	// // }

	// // virtual void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) {
	// // 	do_unpack_std(ctx, cs, indent);
	// // }

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		// cb.append_cellslice(ics).store_ref(r1).store_ref(r2);
		cb.append_cellslice(ics);
		if (!r1.is_null()) {
			cb.store_ref(r1);
		}
		// if (!r2.is_null()) {
		// 	cb.store_ref(r2);
		// }
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

struct FullShardStateUnsplit : BaseFullCell<MyShardStateUnsplit> {
	/*
	todo can optimize tag
	
	shard_state#9023afe2 
		global_id:int32
  	shard_id:ShardIdent 
  	seq_no:uint32 
		vert_seq_no:#
  	gen_utime:uint32 
		gen_lt:uint64
  	min_ref_mc_seqno:uint32
  	out_msg_queue_info:^OutMsgQueueInfo
  	before_split:(## 1)
  	accounts:^ShardAccounts
  	^[ 
			overload_history:uint64 
			underload_history:uint64
  		total_balance:CurrencyCollection
  		total_validator_fees:CurrencyCollection
  		libraries:(HashmapE 256 LibDescr)
  		master_ref:(Maybe BlkMasterInfo) 
		]
  	custom:(Maybe ^McStateExtra)
  = ShardStateUnsplit;

	*/
	block::gen::ShardStateUnsplit::Record record;

	FullShardStateUnsplit_aux aux;

	FullOutMsgQueueInfo omqi;
	CellSlice ics;

    FullShardStateUnsplit() : BaseFullCell("ShardStateUnsplit") {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		omqi.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);

		ics = cs;
		cs.advance(cs.size());
		cs.advance_refs(cs.size_refs());
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_ref(omqi.make_std_cell(ctx, indent + 1));

		cb.append_cellslice(ics);
	}
};

struct FullShardState : BaseFullCell<ShardState>, td::CntObject {
    int tag = -1;

		FullShardStateUnsplit shard_state;
		FullShardStateUnsplit left;
		FullShardStateUnsplit right;

		FullShardState() : BaseFullCell("ShardState", ShardState()) {
			
		}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		tag = type.get_tag(cs);
		CHECK(tag == type.shard_state || tag == type.split_state);
		if (tag == type.shard_state) {
			shard_state.unpack_std(ctx, cs, indent + 1);
		} else {
			cs.advance(32);
			left.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
			right.cell_unpack_std(ctx, cs.fetch_ref(), indent + 1);
		}

		// minify-remove
		CHECK(cs.empty_ext());
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		CHECK(tag == type.shard_state || tag == type.split_state);

		if (tag == type.shard_state) {
			shard_state.pack_std(ctx, cb, indent + 1);
		} else {
			cb.store_long(type.split_state, 32)
			.store_ref(left.make_std_cell(ctx, indent + 1))
			.store_ref(right.make_std_cell(ctx, indent + 1));
		}
	}
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

// struct FullValueFlow : BaseFullCell<block::gen::ValueFlow> {
//     FullValueFlow() : BaseFullCell("ValueFlow", block::gen::t_ValueFlow) {}
// };

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

// struct FullShardFeeCreated : BaseFullCell<block::gen::ShardFeeCreated> {
//     FullShardFeeCreated() : BaseFullCell("ShardFeeCreated", block::gen::t_ShardFeeCreated) {}
// };

// struct FullShardFees : BaseFullCell<block::gen::ShardFees> {
//     FullShardFees() : BaseFullCell("ShardFees", block::gen::t_ShardFees) {}
// };

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

// struct FullMcBlockExtra : BaseFullCell<block::gen::McBlockExtra> {
//     FullMcBlockExtra() : BaseFullCell("McBlockExtra", block::gen::t_McBlockExtra) {}
// };

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

	FullOutMsgDescr() : BaseFullCell("OutMsgDescr", OutMsgDescr()) {}

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

	FullBlockExtra(): BaseFullCell("BlockExtra", block::gen::t_BlockExtra) {}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		CHECK(type.unpack(cs, record));

		in_msg_descr.cell_unpack_std(ctx, record.in_msg_descr, indent + 1);
		out_msg_descr.cell_unpack_std(ctx, record.out_msg_descr, indent + 1);
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_ref(in_msg_descr.make_opt_cell(ctx, indent + 1))
				.store_ref(out_msg_descr.make_opt_cell(ctx, indent + 1))
				.store_ref(record.account_blocks)
				.store_bits(record.rand_seed.cbits(), 256)
				.store_bits(record.created_by.cbits(), 256);
		CHECK(tMRMBE.store_from(cb, record.custom));
	}

	void do_unpack_opt(ParseContext& ctx, CellSlice& cs, int indent = 0) override {
		in_msg_descr.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);
		out_msg_descr.cell_unpack_opt(ctx, cs.fetch_ref(), indent + 1);

		CHECK(
      cs.fetch_ref_to(record.account_blocks)
      && cs.fetch_bits_to(record.rand_seed.bits(), 256)
      && cs.fetch_bits_to(record.created_by.bits(), 256)
      && tMRMBE.fetch_to(cs, record.custom)
		);
	}

	void do_pack_std(ParseContext& ctx, CellBuilder& cb, int indent = 0) override {
		cb.store_long(0x4a33f6fd, 32)
				.store_ref(in_msg_descr.make_std_cell(ctx, indent + 1))
				.store_ref(out_msg_descr.make_std_cell(ctx, indent + 1))
				.store_ref(record.account_blocks)
				.store_bits(record.rand_seed.cbits(), 256)
				.store_bits(record.created_by.cbits(), 256);
		CHECK(tMRMBE.store_from(cb, record.custom));
	}
};

struct FullBlock : BaseFullCell<block::gen::Block>
{
	block::gen::Block::Record record;

	FullMERKLE_UPDATE<FullShardState> state_update;
	FullBlockExtra extra;

	FullBlock(): BaseFullCell("Block", block::gen::t_Block), state_update(FullShardState()) {

	}

	void do_unpack_std(ParseContext& ctx, CellSlice& cs, int indent = 0) override
	{
		CHECK(type.unpack(cs, record));

		state_update.cell_unpack_std(ctx, record.state_update, indent + 1);
		extra.cell_unpack_std(ctx, record.extra, indent + 1, true);
		// minify-remove
		CHECK(cs.empty_ext());
	}

	void do_pack_opt(ParseContext& ctx, CellBuilder& cb, int indent = 0) override
	{
		// todo store bit (in case of main) or long for id;
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

#include "zpaq_impl.cpp"

td::BufferSlice compress(td::Slice data) {
	NullStream ofs;

	Ref<Cell> block_root = std_boc_deserialize(data).move_as_ok();
	
	FullBlock block;
	ParseContext load_std_ctx{ofs};
	block.cell_unpack_std(load_std_ctx, block_root, 0, true);

	ParseContext pack_opt_ctx{ofs};
	auto opt_block_cell = block.make_opt_cell(pack_opt_ctx);

	BagOfCells opt_boc;
	opt_boc.add_root(opt_block_cell);
	CHECK(opt_boc.import_cells().is_ok());

	auto opt_ser = std_boc_serialize(opt_block_cell).move_as_ok();

	// auto compressed = td::lz4_compress(opt_ser);
	auto compressed = zpaq::compress(opt_ser);

	return compressed;
}

td::BufferSlice decompress(td::Slice data) {
	NullStream ofs;

	// auto decompressed = td::lz4_decompress(data, 10'000'000).move_as_ok();
	auto decompressed = zpaq::decompress(data);
	auto opt_deser = std_boc_deserialize(decompressed, false, true).move_as_ok();

	FullBlock opt_block;
	ParseContext parse_opt_ctx{ofs};
	opt_block.cell_unpack_opt(parse_opt_ctx, opt_deser, 0, true);

	ParseContext pack_std_ctx{ofs};
	auto un_opt_block_cell = opt_block.make_std_cell(pack_std_ctx);

	BagOfCells un_opt_boc;
	un_opt_boc.add_root(un_opt_block_cell);
	CHECK(un_opt_boc.import_cells().is_ok());

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

int main()
{
	// minify-remove:start

	// check_decompress();

	ifstream fin("tests/1-001.txt");
	ofstream fout_source("analysis-01-source-cells.txt");
	ofstream fout_final("analysis-02-final-cells.txt");
	ofstream fout_parse_std("analysis-03-parse-std.txt");
	ofstream fout_pack_opt("analysis-04-pack-opt.txt");
	ofstream fout_parse_opt("analysis-05-parse-opt.txt");
	ofstream fout_pack_std("analysis-06-pack-std.txt");
	ofstream fout_compressed_b64("analysis-07-compressed-b64.txt");

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

	for (const auto& cell: std_boc.cell_list_) {
		auto hex = cell.dc_ref->to_hex();
		transform(hex.begin(), hex.end(), hex.begin(), ::toupper);

		fout_source << hex.substr(0, 2) << " " << hex.substr(2, 2) << " " << hex.substr(4) << endl;
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

	auto opt_ser = vm::std_boc_serialize(opt_block_cell).move_as_ok();
	print_delta("Optimized block size", opt_ser.size(), original_size);

	auto compressed = td::lz4_compress(opt_ser);
	print_delta("Compressed size", compressed.size(), opt_ser.size());

	cout << "Total compression delta: ";
	do_print_delta(compressed.size(), original_input_size);
	cout << endl;

	cout << "Score: " << (1000 * static_cast<float>(original_input_size * 2) / (original_input_size + compressed.size())) << endl;

	auto compressed_b64 = td::str_base64_encode(compressed);
	fout_compressed_b64 << compressed_b64;

	auto compressed_b64_decoded = td::base64_decode(compressed_b64);
	auto decompressed = td::lz4_decompress(compressed_b64_decoded, 10'000'000).move_as_ok();
	auto opt_deser = vm::std_boc_deserialize(decompressed, false, true).move_as_ok();

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

	for (const auto& cell: un_opt_boc.cell_list_) {
		auto hex = cell.dc_ref->to_hex();
		transform(hex.begin(), hex.end(), hex.begin(), ::toupper);

		fout_final << hex.substr(0, 2) << " " << hex.substr(2, 2) << " " << hex.substr(4) << endl;
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
