#define private public

#include <iostream>
#include <sstream>
#include "block/block-auto.h"
#include "common/util.h"
#include "vm/boc.h"
#include "vm/cells/CellSlice.h"
#include <fstream>
#include "td/utils/lz4.h"

using vm::BagOfCells;
using vm::Cell;
using vm::CellSerializationInfo;
using vm::DataCell;
using vm::Ref;

namespace td
{
	uint32 crc32c(Slice data);
}

// struct MyBagOfCells : public BagOfCells
// {
// 	td::Result<long long> deserialize(const td::Slice &data, int max_roots)
// 	{
// 		clear();
// 		long long size_est = info.parse_serialized_header(data);
// 		// LOG(INFO) << "estimated size " << size_est << ", true size " << data.size();
// 		if (size_est == 0)
// 		{
// 			return td::Status::Error(PSLICE() << "cannot deserialize bag-of-cells: invalid header, error " << size_est);
// 		}
// 		if (size_est < 0)
// 		{
// 			// LOG(ERROR) << "cannot deserialize bag-of-cells: not enough bytes (" << data.size() << " present, " << -size_est
// 			//<< " required)";
// 			return size_est;
// 		}

// 		if (size_est > (long long)data.size())
// 		{
// 			// LOG(ERROR) << "cannot deserialize bag-of-cells: not enough bytes (" << data.size() << " present, " << size_est
// 			//<< " required)";
// 			return -size_est;
// 		}
// 		// LOG(INFO) << "estimated size " << size_est << ", true size " << data.size();
// 		if (info.root_count > max_roots)
// 		{
// 			return td::Status::Error("Bag-of-cells has more root cells than expected");
// 		}
// 		if (info.has_crc32c)
// 		{
// 			// unsigned crc_computed = td::crc32c(td::Slice{data.ubegin(), data.uend() - 4});
// 			// unsigned crc_stored = td::as<unsigned>(data.uend() - 4);
// 			// if (crc_computed != crc_stored)
// 			// {
// 			// 	return td::Status::Error(PSLICE() << "bag-of-cells CRC32C mismatch: expected " << td::format::as_hex(crc_computed)
// 			// 																		<< ", found " << td::format::as_hex(crc_stored));
// 			// }
// 		}

// 		cell_count = info.cell_count;
// 		std::vector<td::uint8> cell_should_cache;
// 		if (info.has_cache_bits)
// 		{
// 			cell_should_cache.resize(cell_count, 0);
// 		}
// 		roots.clear();
// 		roots.resize(info.root_count);
// 		auto *roots_ptr = data.substr(info.roots_offset).ubegin();
// 		for (int i = 0; i < info.root_count; i++)
// 		{
// 			int idx = 0;
// 			if (info.has_roots)
// 			{
// 				idx = (int)info.read_ref(roots_ptr + i * info.ref_byte_size);
// 			}
// 			if (idx < 0 || idx >= info.cell_count)
// 			{
// 				return td::Status::Error(PSLICE() << "bag-of-cells invalid root index " << idx);
// 			}
// 			roots[i].idx = info.cell_count - idx - 1;
// 			if (info.has_cache_bits)
// 			{
// 				auto &cnt = cell_should_cache[idx];
// 				if (cnt < 2)
// 				{
// 					cnt++;
// 				}
// 			}
// 		}
// 		if (info.has_index)
// 		{
// 			index_ptr = data.substr(info.index_offset).ubegin();
// 			// TODO: should we validate index here
// 		}
// 		else
// 		{
// 			index_ptr = nullptr;
// 			unsigned long long cur = 0;
// 			custom_index.reserve(info.cell_count);

// 			auto cells_slice = data.substr(info.data_offset, info.data_size);

// 			for (int i = 0; i < info.cell_count; i++)
// 			{
// 				vm::CellSerializationInfo cell_info;
// 				auto status = cell_info.init(cells_slice, info.ref_byte_size);
// 				if (status.is_error())
// 				{
// 					return td::Status::Error(PSLICE()
// 																	 << "invalid bag-of-cells failed to deserialize cell #" << i << " " << status.error());
// 				}
// 				cells_slice = cells_slice.substr(cell_info.end_offset);
// 				cur += cell_info.end_offset;
// 				custom_index.push_back(cur);
// 			}
// 			if (!cells_slice.empty())
// 			{
// 				return td::Status::Error(PSLICE() << "invalid bag-of-cells last cell #" << info.cell_count - 1 << ": end offset "
// 																					<< cur << " is different from total data size " << info.data_size);
// 			}
// 		}
// 		auto cells_slice = data.substr(info.data_offset, info.data_size);
// 		std::vector<Ref<vm::DataCell>> cell_list;
// 		cell_list.reserve(cell_count);
// 		std::array<td::Ref<Cell>, 4> refs_buf;
// 		for (int i = 0; i < cell_count; i++)
// 		{
// 			// reconstruct cell with index cell_count - 1 - i
// 			int idx = cell_count - 1 - i;
// 			auto r_cell = my_deserialize_cell(idx, cells_slice, cell_list, info.has_cache_bits ? &cell_should_cache : nullptr);
// 			if (r_cell.is_error())
// 			{
// 				return td::Status::Error(PSLICE() << "invalid bag-of-cells failed to deserialize cell #" << idx << " "
// 																					<< r_cell.error());
// 			}
// 			cell_list.push_back(r_cell.move_as_ok());
// 			DCHECK(cell_list.back().not_null());
// 		}
// 		if (info.has_cache_bits)
// 		{
// 			for (int idx = 0; idx < cell_count; idx++)
// 			{
// 				auto should_cache = cell_should_cache[idx] > 1;
// 				auto stored_should_cache = get_cache_entry(idx);
// 				if (should_cache != stored_should_cache)
// 				{
// 					return td::Status::Error(PSLICE() << "invalid bag-of-cells cell #" << idx << " has wrong cache flag "
// 																						<< stored_should_cache);
// 				}
// 			}
// 		}
// 		custom_index.clear();
// 		index_ptr = nullptr;
// 		root_count = info.root_count;
// 		dangle_count = info.absent_count;
// 		for (auto &root_info : roots)
// 		{
// 			root_info.cell = cell_list[root_info.idx];
// 		}
// 		cell_list.clear();
// 		return size_est;
// 	}

// 	td::Result<td::Ref<vm::DataCell>> my_deserialize_cell(int idx, td::Slice cells_slice,
// 																												td::Span<td::Ref<DataCell>> cells_span,
// 																												std::vector<td::uint8> *cell_should_cache)
// 	{
// 		TRY_RESULT(cell_slice, get_cell_slice(idx, cells_slice));
// 		std::array<td::Ref<Cell>, 4> refs_buf;

// 		CellSerializationInfo cell_info;
// 		TRY_STATUS(cell_info.init(cell_slice, info.ref_byte_size));
// 		if (cell_info.end_offset != cell_slice.size())
// 		{
// 			return td::Status::Error("unused space in cell serialization");
// 		}

// 		auto refs = td::MutableSpan<td::Ref<Cell>>(refs_buf).substr(0, cell_info.refs_cnt);
// 		for (int k = 0; k < cell_info.refs_cnt; k++)
// 		{
// 			int ref_idx = (int)info.read_ref(cell_slice.ubegin() + cell_info.refs_offset + k * info.ref_byte_size);
// 			if (ref_idx <= idx)
// 			{
// 				return td::Status::Error(PSLICE() << "bag-of-cells error: reference #" << k << " of cell #" << idx
// 																					<< " is to cell #" << ref_idx << " with smaller index");
// 			}
// 			if (ref_idx >= cell_count)
// 			{
// 				return td::Status::Error(PSLICE() << "bag-of-cells error: reference #" << k << " of cell #" << idx
// 																					<< " is to non-existent cell #" << ref_idx << ", only " << cell_count
// 																					<< " cells are defined");
// 			}
// 			refs[k] = cells_span[cell_count - ref_idx - 1];
// 			if (cell_should_cache)
// 			{
// 				auto &cnt = (*cell_should_cache)[ref_idx];
// 				if (cnt < 2)
// 				{
// 					cnt++;
// 				}
// 			}
// 		}

// 		return cell_info.create_data_cell(cell_slice, refs);
// 	}
// };

// td::Result<Ref<Cell>> my_std_boc_deserialize(td::Slice data, bool can_be_empty = false, bool allow_nonzero_level = false)
// {
// 	if (data.empty() && can_be_empty)
// 	{
// 		return Ref<Cell>();
// 	}
// 	MyBagOfCells boc;
// 	auto res = boc.deserialize(data, 1);
// 	if (res.is_error())
// 	{
// 		return res.move_as_error();
// 	}
// 	if (boc.get_root_count() != 1)
// 	{
// 		return td::Status::Error("bag of cells is expected to have exactly one root");
// 	}
// 	auto root = boc.get_root_cell();
// 	if (root.is_null())
// 	{
// 		return td::Status::Error("bag of cells has null root cell (?)");
// 	}
// 	if (!allow_nonzero_level && root->get_level() != 0)
// 	{
// 		return td::Status::Error("bag of cells has a root with non-zero level");
// 	}
// 	return std::move(root);
// }

using namespace block::gen;
std::ostream &trace_parse_r(int indent, const char *x)
{
	while (indent-- > 0)
	{
		std::cout << ' ';
	}
	std::cout << x;
	return std::cout;
}

using namespace block::gen;
void trace_parse(int indent, const char *x)
{
	trace_parse_r(indent, x) << std::endl;
}

/// gen start

// struct FullUnit {
//     Unit::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Unit.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Unit.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullTrue {
//     True::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_True.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_True.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullBool {
//     Bool::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Bool.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Bool.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullBoolFalse
{
	BoolFalse::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BoolFalse.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BoolFalse.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBoolTrue
{
	BoolTrue::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BoolTrue.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BoolTrue.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullMaybe {
//     Maybe::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Maybe.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Maybe.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullEither {
//     Either::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Either.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Either.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullBoth {
//     Both::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Both.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Both.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullBit
{
	Bit::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Bit.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Bit.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullHashmap {
//     Hashmap::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Hashmap.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Hashmap.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullHashmapNode {
//     HashmapNode::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_HashmapNode.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_HashmapNode.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullHmLabel {
//     HmLabel::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_HmLabel.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_HmLabel.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullUnary {
//     Unary::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Unary.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Unary.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullHashmapE {
//     HashmapE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_HashmapE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_HashmapE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullBitstringSet {
//     BitstringSet::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_BitstringSet.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_BitstringSet.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullHashmapAugE {
//     HashmapAugE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_HashmapAugE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_HashmapAugE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVarHashmap {
//     VarHashmap::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VarHashmap.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VarHashmap.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVarHashmapNode {
//     VarHashmapNode::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VarHashmapNode.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VarHashmapNode.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVarHashmapE {
//     VarHashmapE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VarHashmapE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VarHashmapE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullPfxHashmap {
//     PfxHashmap::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_PfxHashmap.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_PfxHashmap.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullPfxHashmapNode {
//     PfxHashmapNode::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_PfxHashmapNode.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_PfxHashmapNode.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullPfxHashmapE {
//     PfxHashmapE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_PfxHashmapE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_PfxHashmapE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullMsgAddressExt {
//     MsgAddressExt::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MsgAddressExt.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MsgAddressExt.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullAnycast
{
	Anycast::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Anycast.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Anycast.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullMsgAddressInt {
//     MsgAddressInt::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MsgAddressInt.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MsgAddressInt.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullMsgAddress {
//     MsgAddress::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MsgAddress.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MsgAddress.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVarUInteger {
//     VarUInteger::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VarUInteger.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VarUInteger.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVarInteger {
//     VarInteger::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VarInteger.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VarInteger.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullCoins
{
	Coins::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Coins.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Coins.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullExtraCurrencyCollection
{
	ExtraCurrencyCollection::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullExtraCurrencyCollection.unpack");
		CHECK(t_ExtraCurrencyCollection.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ExtraCurrencyCollection.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullCommonMsgInfo {
//     CommonMsgInfo::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_CommonMsgInfo.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_CommonMsgInfo.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullCommonMsgInfoRelaxed {
//     CommonMsgInfoRelaxed::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_CommonMsgInfoRelaxed.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_CommonMsgInfoRelaxed.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullTickTock
{
	TickTock::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TickTock.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TickTock.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStateInit
{
	StateInit::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StateInit.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StateInit.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStateInitWithLibs
{
	StateInitWithLibs::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StateInitWithLibs.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StateInitWithLibs.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullSimpleLib
{
	SimpleLib::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SimpleLib.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SimpleLib.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullMessage {
//     Message::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Message.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Message.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullMessageRelaxed {
//     MessageRelaxed::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MessageRelaxed.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MessageRelaxed.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullMessageAny
{
	MessageAny::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_MessageAny.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_MessageAny.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullIntermediateAddress {
//     IntermediateAddress::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_IntermediateAddress.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_IntermediateAddress.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullMsgMetadata
{
	MsgMetadata::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_MsgMetadata.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_MsgMetadata.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullMsgEnvelope {
//     MsgEnvelope::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MsgEnvelope.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MsgEnvelope.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullInMsg
{
	// InMsg::Record_msg_import_ext record_msg_import_ext;
	// InMsg::Record_msg_import_ihr record_msg_import_ihr;
	// InMsg::Record_msg_import_imm record_msg_import_imm;
	// InMsg::Record_msg_import_fin record_msg_import_fin;
	// InMsg::Record_msg_import_tr record_msg_import_tr;
	// InMsg::Record_msg_discard_fin record_msg_discard_fin;
	// InMsg::Record_msg_discard_tr record_msg_discard_tr;
	// InMsg::Record_msg_import_deferred_fin record_msg_import_deferred_fin;
	// InMsg::Record_msg_import_deferred_tr record_msg_import_deferred_tr;

	int tag = -1;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullInMsg.unpack");
		tag = t_InMsg.check_tag(cs_ref.write());

		// if (tag == InMsg::msg_import_ext) {
		// 	CHECK(t_InMsg.unpack(cs_ref.write(), record_msg_import_ext));
		// }
	}
};

// struct FullOutMsg {
//     OutMsg::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_OutMsg.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_OutMsg.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullEnqueuedMsg
{
	EnqueuedMsg::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_EnqueuedMsg.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_EnqueuedMsg.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOutMsgDescr
{
	OutMsgDescr::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OutMsgDescr.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OutMsgDescr.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOutMsgQueue
{
	OutMsgQueue::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OutMsgQueue.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OutMsgQueue.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullProcessedUpto
{
	ProcessedUpto::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ProcessedUpto.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ProcessedUpto.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullProcessedInfo
{
	ProcessedInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ProcessedInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ProcessedInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullIhrPendingSince
{
	IhrPendingSince::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_IhrPendingSince.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_IhrPendingSince.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullIhrPendingInfo
{
	IhrPendingInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_IhrPendingInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_IhrPendingInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullAccountDispatchQueue
{
	AccountDispatchQueue::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_AccountDispatchQueue.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_AccountDispatchQueue.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullDispatchQueue
{
	DispatchQueue::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_DispatchQueue.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_DispatchQueue.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOutMsgQueueExtra
{
	OutMsgQueueExtra::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OutMsgQueueExtra.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OutMsgQueueExtra.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOutMsgQueueInfo
{
	OutMsgQueueInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OutMsgQueueInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OutMsgQueueInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStorageUsed
{
	StorageUsed::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StorageUsed.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StorageUsed.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStorageUsedShort
{
	StorageUsedShort::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StorageUsedShort.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StorageUsedShort.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStorageInfo
{
	StorageInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StorageInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StorageInfo.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullAccount {
//     Account::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_Account.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_Account.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullAccountStorage
{
	AccountStorage::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_AccountStorage.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_AccountStorage.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullAccountState {
//     AccountState::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_AccountState.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_AccountState.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullAccountStatus {
//     AccountStatus::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_AccountStatus.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_AccountStatus.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullShardAccount
{
	ShardAccount::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardAccount.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardAccount.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullDepthBalanceInfo
{
	DepthBalanceInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_DepthBalanceInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_DepthBalanceInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardAccounts
{
	ShardAccounts::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardAccounts.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardAccounts.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTransaction_aux
{
	Transaction_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Transaction_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Transaction_aux.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTransaction
{
	Transaction::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Transaction.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Transaction.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullMERKLE_UPDATE {
//     MERKLE_UPDATE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MERKLE_UPDATE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MERKLE_UPDATE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullHASH_UPDATE {
//     HASH_UPDATE::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_HASH_UPDATE.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_HASH_UPDATE.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullMERKLE_PROOF {
//     MERKLE_PROOF::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_MERKLE_PROOF.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_MERKLE_PROOF.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullAccountBlock
{
	AccountBlock::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_AccountBlock.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_AccountBlock.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardAccountBlocks
{
	ShardAccountBlocks::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardAccountBlocks.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardAccountBlocks.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTrStoragePhase
{
	TrStoragePhase::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TrStoragePhase.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TrStoragePhase.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullAccStatusChange {
//     AccStatusChange::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_AccStatusChange.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_AccStatusChange.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullTrCreditPhase
{
	TrCreditPhase::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TrCreditPhase.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TrCreditPhase.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTrComputePhase_aux
{
	TrComputePhase_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TrComputePhase_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TrComputePhase_aux.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullTrComputePhase {
//     TrComputePhase::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_TrComputePhase.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_TrComputePhase.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullComputeSkipReason {
//     ComputeSkipReason::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ComputeSkipReason.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ComputeSkipReason.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullTrActionPhase
{
	TrActionPhase::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TrActionPhase.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TrActionPhase.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullTrBouncePhase {
//     TrBouncePhase::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_TrBouncePhase.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_TrBouncePhase.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullSplitMergeInfo
{
	SplitMergeInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SplitMergeInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SplitMergeInfo.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullTransactionDescr {
//     TransactionDescr::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_TransactionDescr.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_TransactionDescr.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullSmartContractInfo
{
	SmartContractInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SmartContractInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SmartContractInfo.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullOutList {
//     OutList::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_OutList.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_OutList.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullLibRef {
//     LibRef::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_LibRef.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_LibRef.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullOutAction {
//     OutAction::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_OutAction.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_OutAction.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullOutListNode
{
	OutListNode::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OutListNode.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OutListNode.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardIdent
{
	ShardIdent::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardIdent.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardIdent.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullExtBlkRef
{
	ExtBlkRef::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ExtBlkRef.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ExtBlkRef.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockIdExt
{
	BlockIdExt::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockIdExt.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockIdExt.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlkMasterInfo
{
	BlkMasterInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlkMasterInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlkMasterInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardStateUnsplit_aux
{
	ShardStateUnsplit_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardStateUnsplit_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardStateUnsplit_aux.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardStateUnsplit
{
	ShardStateUnsplit::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardStateUnsplit.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardStateUnsplit.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullShardState {
//     ShardState::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ShardState.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ShardState.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullLibDescr
{
	LibDescr::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_LibDescr.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_LibDescr.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockInfo
{
	BlockInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockInfo.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullBlkPrevInfo {
//     BlkPrevInfo::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_BlkPrevInfo.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_BlkPrevInfo.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullTYPE_1657
{
	TYPE_1657::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TYPE_1657.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TYPE_1657.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTYPE_1658
{
	TYPE_1658::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TYPE_1658.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TYPE_1658.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullValueFlow {
//     ValueFlow::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ValueFlow.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ValueFlow.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullBinTree {
//     BinTree::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_BinTree.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_BinTree.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullFutureSplitMerge {
//     FutureSplitMerge::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_FutureSplitMerge.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_FutureSplitMerge.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullShardDescr_aux
{
	ShardDescr_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardDescr_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardDescr_aux.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullShardDescr {
//     ShardDescr::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ShardDescr.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ShardDescr.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullShardHashes
{
	ShardHashes::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardHashes.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardHashes.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullBinTreeAug {
//     BinTreeAug::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_BinTreeAug.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_BinTreeAug.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullShardFeeCreated
{
	ShardFeeCreated::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardFeeCreated.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardFeeCreated.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullShardFees
{
	ShardFees::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ShardFees.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ShardFees.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullConfigParams
{
	ConfigParams::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ConfigParams.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ConfigParams.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullValidatorInfo
{
	ValidatorInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullValidatorBaseInfo
{
	ValidatorBaseInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorBaseInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorBaseInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullKeyMaxLt
{
	KeyMaxLt::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_KeyMaxLt.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_KeyMaxLt.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullKeyExtBlkRef
{
	KeyExtBlkRef::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_KeyExtBlkRef.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_KeyExtBlkRef.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOldMcBlocksInfo
{
	OldMcBlocksInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OldMcBlocksInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OldMcBlocksInfo.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCounters
{
	Counters::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Counters.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Counters.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCreatorStats
{
	CreatorStats::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_CreatorStats.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_CreatorStats.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullBlockCreateStats {
//     BlockCreateStats::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_BlockCreateStats.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_BlockCreateStats.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullMcStateExtra_aux
{
	McStateExtra_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_McStateExtra_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_McStateExtra_aux.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullMcStateExtra
{
	McStateExtra::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_McStateExtra.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_McStateExtra.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullSigPubKey
{
	SigPubKey::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SigPubKey.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SigPubKey.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCryptoSignatureSimple
{
	CryptoSignatureSimple::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_CryptoSignatureSimple.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_CryptoSignatureSimple.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCryptoSignaturePair
{
	CryptoSignaturePair::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_CryptoSignaturePair.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_CryptoSignaturePair.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCertificate
{
	Certificate::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Certificate.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Certificate.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullCertificateEnv
{
	CertificateEnv::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_CertificateEnv.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_CertificateEnv.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullSignedCertificate
{
	SignedCertificate::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SignedCertificate.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SignedCertificate.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullCryptoSignature {
//     CryptoSignature::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_CryptoSignature.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_CryptoSignature.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullMcBlockExtra_aux
{
	McBlockExtra_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_McBlockExtra_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_McBlockExtra_aux.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullMcBlockExtra
{
	McBlockExtra::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_McBlockExtra.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_McBlockExtra.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullValidatorDescr {
//     ValidatorDescr::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ValidatorDescr.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ValidatorDescr.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullValidatorSet {
//     ValidatorSet::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ValidatorSet.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ValidatorSet.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullBurningConfig
{
	BurningConfig::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BurningConfig.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BurningConfig.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullGlobalVersion
{
	GlobalVersion::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_GlobalVersion.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_GlobalVersion.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullConfigProposalSetup
{
	ConfigProposalSetup::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ConfigProposalSetup.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ConfigProposalSetup.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullConfigVotingSetup
{
	ConfigVotingSetup::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ConfigVotingSetup.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ConfigVotingSetup.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullConfigProposal
{
	ConfigProposal::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ConfigProposal.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ConfigProposal.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullConfigProposalStatus
{
	ConfigProposalStatus::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ConfigProposalStatus.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ConfigProposalStatus.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullWorkchainFormat {
//     WorkchainFormat::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_WorkchainFormat.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_WorkchainFormat.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullWcSplitMergeTimings
{
	WcSplitMergeTimings::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_WcSplitMergeTimings.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_WcSplitMergeTimings.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullWorkchainDescr {
//     WorkchainDescr::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_WorkchainDescr.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_WorkchainDescr.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullComplaintPricing
{
	ComplaintPricing::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ComplaintPricing.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ComplaintPricing.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockCreateFees
{
	BlockCreateFees::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockCreateFees.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockCreateFees.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullStoragePrices
{
	StoragePrices::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_StoragePrices.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_StoragePrices.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullGasLimitsPrices {
//     GasLimitsPrices::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_GasLimitsPrices.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_GasLimitsPrices.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullParamLimits
{
	ParamLimits::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ParamLimits.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ParamLimits.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockLimits
{
	BlockLimits::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockLimits.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockLimits.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullMsgForwardPrices
{
	MsgForwardPrices::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_MsgForwardPrices.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_MsgForwardPrices.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullCatchainConfig {
//     CatchainConfig::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_CatchainConfig.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_CatchainConfig.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullConsensusConfig {
//     ConsensusConfig::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ConsensusConfig.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ConsensusConfig.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullValidatorTempKey
{
	ValidatorTempKey::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorTempKey.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorTempKey.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullValidatorSignedTempKey
{
	ValidatorSignedTempKey::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorSignedTempKey.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorSignedTempKey.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullMisbehaviourPunishmentConfig
{
	MisbehaviourPunishmentConfig::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_MisbehaviourPunishmentConfig.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_MisbehaviourPunishmentConfig.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullSizeLimitsConfig {
//     SizeLimitsConfig::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_SizeLimitsConfig.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_SizeLimitsConfig.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullSuspendedAddressList
{
	SuspendedAddressList::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_SuspendedAddressList.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_SuspendedAddressList.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullPrecompiledSmc
{
	PrecompiledSmc::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_PrecompiledSmc.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_PrecompiledSmc.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullPrecompiledContractsConfig
{
	PrecompiledContractsConfig::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_PrecompiledContractsConfig.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_PrecompiledContractsConfig.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullOracleBridgeParams
{
	OracleBridgeParams::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_OracleBridgeParams.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_OracleBridgeParams.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullJettonBridgePrices
{
	JettonBridgePrices::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_JettonBridgePrices.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_JettonBridgePrices.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullJettonBridgeParams {
//     JettonBridgeParams::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_JettonBridgeParams.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_JettonBridgeParams.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullConfigParam {
//     ConfigParam::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ConfigParam.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ConfigParam.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullBlockSignaturesPure
{
	BlockSignaturesPure::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockSignaturesPure.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockSignaturesPure.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockSignatures
{
	BlockSignatures::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockSignatures.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockSignatures.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullBlockProof
{
	BlockProof::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_BlockProof.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_BlockProof.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullProofChain {
//     ProofChain::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ProofChain.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ProofChain.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullTopBlockDescr
{
	TopBlockDescr::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TopBlockDescr.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TopBlockDescr.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullTopBlockDescrSet
{
	TopBlockDescrSet::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_TopBlockDescrSet.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_TopBlockDescrSet.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullProducerInfo
{
	ProducerInfo::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ProducerInfo.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ProducerInfo.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullComplaintDescr {
//     ComplaintDescr::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ComplaintDescr.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ComplaintDescr.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullValidatorComplaint
{
	ValidatorComplaint::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorComplaint.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorComplaint.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullValidatorComplaintStatus
{
	ValidatorComplaintStatus::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ValidatorComplaintStatus.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ValidatorComplaintStatus.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullVmCellSlice
{
	VmCellSlice::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmCellSlice.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmCellSlice.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullVmTupleRef {
//     VmTupleRef::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VmTupleRef.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VmTupleRef.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVmTuple {
//     VmTuple::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VmTuple.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VmTuple.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullVmStackValue {
//     VmStackValue::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VmStackValue.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VmStackValue.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullVmStack
{
	VmStack::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmStack.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmStack.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullVmStackList {
//     VmStackList::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VmStackList.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VmStackList.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullVmSaveList
{
	VmSaveList::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmSaveList.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmSaveList.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullVmGasLimits_aux
{
	VmGasLimits_aux::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmGasLimits_aux.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmGasLimits_aux.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullVmGasLimits
{
	VmGasLimits::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmGasLimits.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmGasLimits.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullVmLibraries
{
	VmLibraries::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmLibraries.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmLibraries.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullVmControlData
{
	VmControlData::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_VmControlData.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_VmControlData.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullVmCont {
//     VmCont::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_VmCont.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_VmCont.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullDNS_RecordSet
{
	DNS_RecordSet::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_DNS_RecordSet.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_DNS_RecordSet.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullTextChunkRef {
//     TextChunkRef::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_TextChunkRef.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_TextChunkRef.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullTextChunks {
//     TextChunks::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_TextChunks.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_TextChunks.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullText
{
	Text::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Text.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Text.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullProtoList {
//     ProtoList::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ProtoList.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ProtoList.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullProtocol
{
	Protocol::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_Protocol.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_Protocol.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullSmcCapList {
//     SmcCapList::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_SmcCapList.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_SmcCapList.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullSmcCapability {
//     SmcCapability::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_SmcCapability.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_SmcCapability.cell_unpack(std::move(cell_ref), record));

// 	}
// };

// struct FullDNSRecord {
//     DNSRecord::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_DNSRecord.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_DNSRecord.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullChanConfig
{
	ChanConfig::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanConfig.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanConfig.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullChanState {
//     ChanState::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ChanState.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ChanState.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullChanPromise
{
	ChanPromise::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanPromise.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanPromise.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullChanSignedPromise
{
	ChanSignedPromise::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanSignedPromise.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanSignedPromise.cell_unpack(std::move(cell_ref), record));
	}
};

// struct FullChanMsg {
//     ChanMsg::Record record;

//     void unpack(Ref<vm::CellSlice> cs_ref) {
// 		CHECK(t_ChanMsg.unpack(cs_ref.write(), record));
// 	}

// 	void cell_unpack(Ref<vm::Cell> cell_ref) {
// 		CHECK(t_ChanMsg.cell_unpack(std::move(cell_ref), record));

// 	}
// };

struct FullChanSignedMsg
{
	ChanSignedMsg::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanSignedMsg.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanSignedMsg.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullChanOp
{
	ChanOp::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanOp.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanOp.cell_unpack(std::move(cell_ref), record));
	}
};

struct FullChanData
{
	ChanData::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref)
	{
		CHECK(t_ChanData.unpack(cs_ref.write(), record));
	}

	void cell_unpack(Ref<vm::Cell> cell_ref)
	{
		CHECK(t_ChanData.cell_unpack(std::move(cell_ref), record));
	}
};

/// gen end

struct FullVarUInteger16
{
	VarUInteger::Record record;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullVarUInteger16.unpack");
		CHECK(t_VarUInteger_16.unpack(cs_ref.write(), record));
	}
};

struct FullGrams
{
	Grams::Record record;

	FullVarUInteger16 amount;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullGrams.unpack");
		CHECK(t_Grams.unpack(cs_ref.write(), record));

		amount.unpack(record.amount, indent + 1);
	}
};

struct FullCurrencyCollection
{
	CurrencyCollection::Record record;

	FullGrams grams;
	FullExtraCurrencyCollection other;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullCurrencyCollection.unpack");
		CHECK(t_CurrencyCollection.unpack(cs_ref.write(), record));

		grams.unpack(record.grams, indent + 1);
		other.unpack(record.other, indent + 1);
	}
};

struct FullImportFees
{
	ImportFees::Record record;

	FullGrams fees_collected;
	FullCurrencyCollection value_imported;

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		CHECK(t_ImportFees.unpack(cs_ref.write(), record));

		fees_collected.unpack(record.fees_collected, indent + 1);
		value_imported.unpack(record.value_imported, indent + 1);
	}
};

template <class TValue, class TAug>
struct FullHashmapAug;

template <class TValue, class TAug>
struct FullHashmapAugNode
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

	HashmapAugNode::Record_ahmn_leaf record_ahmn_leaf;
	HashmapAugNode::Record_ahmn_fork record_ahmn_fork;

	HashmapAugNode type;

	int tag = -1;
	Ref<FullHashmapAug<TValue, TAug>> left;
	Ref<FullHashmapAug<TValue, TAug>> right;

	TValue value;
	TAug extra;

	FullHashmapAugNode(int m, const TLB &X, const TLB &Y) : type(m, X, Y) {}

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		// trace_parse(indent, "FullHashmapAugNode.unpack");
		
		tag = type.check_tag(cs_ref.write());

		// trace_parse_r(indent, "FullHashmapAugNode.m") << type.m_ << std::endl;

		if (tag == HashmapAugNode::ahmn_leaf)
		{
			// trace_parse(indent, "FullHashmapAugNode.unpack leaf");

			CHECK(type.unpack(cs_ref.write(), record_ahmn_leaf));

			auto cs = record_ahmn_leaf.extra.write();
			// trace_parse_r(indent, "HashmapAugNode::ahmn_leaf: ") << cs.data_bits().to_hex(cs.size()) << std::endl;
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;

			value.unpack(record_ahmn_leaf.value, indent + 1);
			extra.unpack(record_ahmn_leaf.extra, indent + 1);

			(td::BitPtr{const_cast<unsigned char*>(cs.data()), (int)cs.cur_pos()}).fill(0, cs.size());
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;
		}
		else if (tag == HashmapAugNode::ahmn_fork)
		{
			// trace_parse(indent, "FullHashmapAugNode.unpack fork");
			CHECK(type.unpack(cs_ref.write(), record_ahmn_fork));

			auto cs = record_ahmn_fork.extra.write();
			// trace_parse_r(indent, "HashmapAugNode::ahmn_fork: ") << cs.data_bits().to_hex(cs.size()) << std::endl;
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;

			if (record_ahmn_fork.left.not_null())
			{
				left = Ref<FullHashmapAug<TValue, TAug>>(true, type.m_ - 1, type.X_, type.Y_);
				// trace_parse(indent, "FullHashmapAugNode.unpack left");
				left.write().cell_unpack(record_ahmn_fork.left, indent + 1);
			}
			if (record_ahmn_fork.right.not_null())
			{
				right = Ref<FullHashmapAug<TValue, TAug>>(true, type.m_ - 1, type.X_, type.Y_);
				// trace_parse(indent, "FullHashmapAugNode.unpack right");
				right.write().cell_unpack(record_ahmn_fork.right, indent + 1);
			}

			extra.unpack(record_ahmn_fork.extra, indent + 1);

			(td::BitPtr{const_cast<unsigned char*>(cs.data()), (int)cs.cur_pos()}).fill(0, cs.size());
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;
		}
		else
		{
			throw std::runtime_error("not supported");
		}
	}
};

template <class TValue, class TAug>
struct FullHashmapAug : public td::CntObject
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
	HashmapAug::Record record;

	const HashmapAug type;

	FullHashmapAugNode<TValue, TAug> node;

	FullHashmapAug(int n, const TLB &X, const TLB &Y) : type(n, X, Y), node(n, X, Y) {}

	void cell_unpack(Ref<vm::Cell> cell_ref, int indent = 0)
	{
		// trace_parse(indent, "FullHashmapAug.cell_unpack");
		// trace_parse_r(indent, "FullHashmapAug.m=") << type.m_ << std::endl;
		CHECK(type.cell_unpack(cell_ref, record));

		node.type.m_ = record.m;

		node.unpack(record.node, indent + 1);
	}
};

struct FullInMsgDescrXHM : FullHashmapAug<FullInMsg, FullImportFees>
{
	FullInMsgDescrXHM() : FullHashmapAug(256, t_InMsg, t_ImportFees)
	{
	}
};

const auto t_InMsgDescrX = t_HashmapAugE_256_InMsg_ImportFees;

template <class TValue, class TAug>
struct FullHashmapAugE
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

	HashmapAugE::Record_ahme_empty record_ahme_empty;
	HashmapAugE::Record_ahme_root record_ahme_root;

	const HashmapAugE type;

	int tag = -1;
	FullHashmapAug<TValue, TAug> root;
	TAug extra;

	FullHashmapAugE(int n, const TLB &X, const TLB &Y) : type(n, X, Y), root(n, X, Y) {}

	void unpack(Ref<vm::CellSlice> cs_ref, int indent = 0)
	{
		tag = type.check_tag(cs_ref.write());

		if (tag == HashmapAugE::ahme_empty)
		{
			CHECK(type.unpack(cs_ref.write(), record_ahme_empty));

			auto cs = record_ahme_empty.extra.write();
			// trace_parse_r(indent, "HashmapAugE::empty: ") << cs.data_bits().to_hex(cs.size()) << std::endl;
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;

			extra.unpack(record_ahme_empty.extra, indent + 1);

			(td::BitPtr{const_cast<unsigned char*>(cs.data()), (int)cs.cur_pos()}).fill(0, cs.size());
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;
		}
		else if (tag == HashmapAugE::ahme_root)
		{
			CHECK(type.unpack(cs_ref.write(), record_ahme_root));

			auto cs = record_ahme_root.extra.write();
			// trace_parse_r(indent, "HashmapAugE::root: ") << cs.data_bits().to_hex(cs.size()) << std::endl;
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;

			root.cell_unpack(record_ahme_root.root, indent + 1);
			extra.unpack(record_ahme_root.extra, indent + 1);

			(td::BitPtr{const_cast<unsigned char*>(cs.data()), (int)cs.cur_pos()}).fill(0, cs.size());
			std::cout << cs.data_bits().to_hex(cs.size()) << std::endl;
		}
		else
		{
			throw std::runtime_error("not supported");
		}
	}
};

struct FullInMsgDescr
{
	block::gen::InMsgDescr::Record record;

	FullHashmapAugE<FullInMsg, FullImportFees> x{256, t_InMsg, t_ImportFees};

	void cell_unpack(Ref<vm::Cell> cell_ref, int indent = 0)
	{
		CHECK(t_InMsgDescr.cell_unpack(std::move(cell_ref), record));

		x.unpack(record.x, indent + 1);
	}
};

struct FullBlockExtra
{
	block::gen::BlockExtra::Record record;

	FullInMsgDescr in_msg_descr;

	void cell_unpack(Ref<vm::Cell> cell_ref, int indent = 0)
	{
		// trace_parse(indent, "FullBlockExtra.cell_unpack");
		CHECK(t_BlockExtra.cell_unpack(std::move(cell_ref), record));

		in_msg_descr.cell_unpack(record.in_msg_descr, indent + 1);
	}
};

struct FullBlock
{
	block::gen::Block::Record record;

	FullBlockExtra extra;

	void cell_unpack(Ref<vm::Cell> cell_ref, int indent = 0)
	{
		// trace_parse(indent, "FullBlock.cell_unpack");
		CHECK(t_Block.cell_unpack(std::move(cell_ref), record));

		extra.cell_unpack(record.extra, indent + 1);
	}
};

int main()
{
	std::ifstream fin("tests/1-001.txt");

	std::string block_base64;
	fin >> block_base64;

	if (block_base64 == "compress")
	{
		fin >> block_base64;
	}

	td::Ref<vm::Cell> block_root = vm::std_boc_deserialize(td::base64_decode(block_base64)).move_as_ok();
	// FullBlock block;
	// block.cell_unpack(block_root);

	auto bs = vm::std_boc_serialize(block_root).move_as_ok();
	std::cout << bs.size() << std::endl;

	auto compressed = td::lz4_compress(bs);
	std::cout << compressed.size() << std::endl;

	// ogiinal          229,919
	// lz4              200,368
	// lz4 with zeroing 199,103

	// >>> (2 * 229919) / (229919 + 200368)
	// 1.0686774176305582
	// >>> (2 * 229919) / (229919 + 200368) * 1000
	// 1068.6774176305582
	// >>> (2 * 229919) / (229919 + 199103) * 1000
	// 1071.8284843201513
}
