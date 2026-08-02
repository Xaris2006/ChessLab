#pragma once

#include <utility>
#include <unordered_map>

namespace Chess
{
	struct PairMoveHash {
		std::size_t operator()(const std::pair<uint8_t, uint8_t>& p) const noexcept {
			return (static_cast<std::size_t>(p.first) << 8) | p.second;
		}
	};

	using SearchResultMoveData = std::unordered_map<std::pair<uint8_t, uint8_t>, std::tuple<size_t, size_t, size_t, size_t, size_t, uint16_t>, PairMoveHash>;

}