#pragma once

#include "xxhash.h"

#include <filesystem>
#include <fstream>
#include <array>

#define HASH_LENGTH 8

inline std::array<uint8_t, HASH_LENGTH> xxHashFile(const std::filesystem::path& path, float* persentage)
{
	constexpr size_t BUF_SIZE = 64 * 1024 * 1024; // 64 MB buffer
	std::unique_ptr<char[]> buffer(new char[BUF_SIZE]);

	std::ifstream file(path, std::ios::binary);
	if (!file)
	{
		//printf("Cannot open file");
		return {};
	}

	XXH3_state_t* state = XXH3_createState();
	XXH3_64bits_reset(state);

	file.seekg(0, std::ios_base::end);
	size_t maxIndex = file.tellg();
	file.seekg(0, std::ios_base::beg);

	while (file) {

		file.read(buffer.get(), BUF_SIZE);
		std::streamsize read_bytes = file.gcount();
		
		if (persentage)
		{
			*persentage = std::min(0.0f + float(file.tellg() / (double)maxIndex) * 0.4f, 0.4f);
		}
		
		if (read_bytes > 0) {
			XXH3_64bits_update(state, buffer.get(), static_cast<size_t>(read_bytes));
		}
	}

	uint64_t hash = XXH3_64bits_digest(state);
	XXH3_freeState(state);

	std::array<uint8_t, HASH_LENGTH> ret;
	// Convert the 64-bit hash to a byte array
	ret[0] = (hash >> 56) & 0xFF;
	ret[1] = (hash >> 48) & 0xFF;
	ret[2] = (hash >> 40) & 0xFF;
	ret[3] = (hash >> 32) & 0xFF;
	ret[4] = (hash >> 24) & 0xFF;
	ret[5] = (hash >> 16) & 0xFF;
	ret[6] = (hash >> 8) & 0xFF;
	ret[7] = hash & 0xFF;


	// Fill the rest with zeros
	for (size_t i = 8; i < HASH_LENGTH; ++i)
		ret[i] = 0;

	return ret;
}

inline std::array<uint8_t, HASH_LENGTH> HashFile(const std::filesystem::path& path, float* persentage)
{
	return xxHashFile(path, persentage);
}