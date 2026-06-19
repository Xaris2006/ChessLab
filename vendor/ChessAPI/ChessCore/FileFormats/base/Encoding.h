#pragma once

#include <stdint.h>

namespace Chess
{
	enum MoveEncoding : uint8_t
	{
		CLD = 0,
		CORE = 1,
		PGN = 2
	};
}