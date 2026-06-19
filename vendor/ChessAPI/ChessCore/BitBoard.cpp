#include "BitBoard.h"

namespace Chess
{
	bool BitBoard::At(int index) const
	{
		return (bool)(1ULL << index & m_data);
	}
	
	bool BitBoard::At(int indexX, int indexY) const
	{
		return (bool)(1ULL << (indexX + 8 * indexY) & m_data);
	}

	void BitBoard::Set(int index, bool value)
	{
		if (value)
			m_data |= (1ULL << index);
		else
			m_data &= ~(1ULL << index);
	}

	void BitBoard::Set(int indexX, int indexY, bool value)
	{
		if (value)
			m_data |= (1ULL << (indexX + 8 * indexY));
		else
			m_data &= ~(1ULL << (indexX + 8 * indexY));
	}

	void BitBoard::SetMulty(int indexA, int indexB, bool value)
	{
		if (value)
			m_data |= ((1ULL << indexA) | (1ULL << indexB));
		else
			m_data &= ~((1ULL << indexA) | (1ULL << indexB));
	}

	void BitBoard::Flip(int index)
	{
		m_data ^= (1ULL << index);
	}

	void BitBoard::FlipMulty(int indexA, int indexB)
	{
		m_data ^= ((1ULL << indexA) | (1ULL << indexB));
	}

	void BitBoard::ApplyMaskMany_Set(BitBoard& bbA, BitBoard& bbB, int index, bool value)
	{
		if (value)
		{
			uint64_t mask = (1ULL << index);

			bbA.m_data |= mask;
			bbB.m_data |= mask;
		}
		else
		{
			uint64_t mask = ~(1ULL << index);

			bbA.m_data &= mask;
			bbB.m_data &= mask;
		}
	}

	void BitBoard::ApplyMaskMany_Flip(BitBoard& bbA, BitBoard& bbB, int index)
	{
		uint64_t mask = (1ULL << index);

		bbA.m_data ^= mask;
		bbB.m_data ^= mask;
	}

	uint64_t& BitBoard::Data()
	{
		return m_data;
	}

	uint64_t BitBoard::Data() const
	{
		return m_data;
	}

	BitBoard BitBoard::operator+(const BitBoard& other) const
	{
		return BitBoard(m_data | other.m_data);
	}
}