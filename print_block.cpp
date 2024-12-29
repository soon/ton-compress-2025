#include <iostream>
#include <sstream>
#include "common/util.h"
#include "vm/boc.h"

#include "tlb-impl.cpp"


int main() {
	std::string block_base64;
	std::cin >> block_base64;

	if (block_base64 == "compress") {
		std::cin >> block_base64;
	}

	td::Ref<vm::Cell> block_root = vm::std_boc_deserialize(td::base64_decode(block_base64)).move_as_ok();

	std::ostringstream ss;
	tlb::gen::Block().print_ref(std::cout, block_root, 0, 10'000'000);
}
