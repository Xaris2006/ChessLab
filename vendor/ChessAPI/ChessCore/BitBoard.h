#pragma once

#include <cstdint>

namespace Chess
{
	class BitBoard
	{
	public:
		BitBoard(uint64_t data) : m_data(data) {}
		BitBoard() = default;
		~BitBoard() = default;

		bool At(int index) const;
		bool At(int indexX, int indexY) const;

		void Set(int index, bool value);
		void Set(int indexX, int indexY, bool value);
		void SetMulty(int indexA, int indexB, bool value);

		void Flip(int index);
		void FlipMulty(int indexA, int indexB);
		
		static void ApplyMaskMany_Set(BitBoard& bbA, BitBoard& bbB, int index, bool value);
		static void ApplyMaskMany_Flip(BitBoard& bbA, BitBoard& bbB, int index);

		uint64_t& Data();
		uint64_t Data() const;

		BitBoard operator+(const BitBoard& other) const;

	private:
		uint64_t m_data = 0;

	};
}
