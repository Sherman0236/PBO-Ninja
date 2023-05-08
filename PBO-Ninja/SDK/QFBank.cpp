#include "QFBank.hpp"
#include "../Global.hpp"

std::vector<uint8_t> QFBank::GetBytes(uint32_t startOffset, size_t length)
{
	return fileBuffer()->Read(startOffset, length);
}

std::string QFBank::GetHash()
{
	// the hash size is fixed at 20 bytes
	constexpr static uint32_t hashSize = 20;
	
	// get the hash bytes from the end of the file buffer
	auto hashBytes = GetBytes(uint32_t(fileBuffer()->size()) - hashSize, hashSize);

	std::string result = "";

	// iterate over the hash bytes and convert them to hex format
	for (auto& byte : hashBytes)
	{
		result += std::format("{:02X} ", byte);
	}

	return result;
}

std::vector<QFBank::FileInfoO> QFBank::GetFiles()
{
	uint64_t fileArrays = *reinterpret_cast<uint64_t*>(this + 0x48);

	// this is the number of allocated file arrays, NOT the actual number of file arrays
	uint32_t fileArraysAllocCount = *reinterpret_cast<uint32_t*>(this + 0x50);
	if (!fileArrays || fileArraysAllocCount <= 0)
	{
		return {};
	}

	std::vector<FileInfoO> result = {};

	for (uint32_t i = 0; i < fileArraysAllocCount; i++)
	{
		uint64_t fileArray = *reinterpret_cast<uint64_t*>(fileArrays + uint64_t(i) * 0x18);
		uint32_t fileArrayCount = *reinterpret_cast<uint32_t*>(fileArrays + (uint64_t(i) * 0x18) + 0x8);
		
		// skip this iteration of the loop if the file array is not allocated or has no files
		if (!fileArray || fileArrayCount <= 0)
		{
			continue;
		}

		// iterate over the files in the current file array and add them to the result vector
		for (uint32_t j = 0; j < fileArrayCount; j++)
		{
			result.push_back(*reinterpret_cast<FileInfoO*>(fileArray + uint64_t(j) * 0x20));
		}
	}

	return result;
}