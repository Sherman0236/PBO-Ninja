#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <optional>

namespace Signature
{
#define IN_RANGE(x, a, b) (x >= a && x <= b)
#define GET_BITS(x) (IN_RANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (IN_RANGE(x, '0', '9') ? x - '0': 0))
#define GET_BYTE(x) (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))

	/// Searches for the occurences of a given pattern within a module
	/// </summary>
	/// <param name="moduleName">The name of the module to search</param>
	/// <param name="pattern">The pattern to search for, formatted as pairs of hexadecimal characters separated by spaces (e.g., "AA BB CC DD")</param>
	/// <param name="returnFirst">If true, only the first signature match will be returned</param>
	/// <returns></returns>
	std::vector<uint64_t> FindSignatures(const char* moduleName, const char* pattern, bool returnFirst = false)
	{
		auto moduleHandle = GetModuleHandleA(moduleName);
		if (!moduleHandle)
		{
			return {};
		}

		// get the module information for the provided module name
		MODULEINFO moduleInfo;
		if (!K32GetModuleInformation(GetCurrentProcess(), moduleHandle, &moduleInfo, sizeof(MODULEINFO)))
		{
			return {};
		}

		// create the search range based on the module's base address and size
		auto searchStart = uint64_t(moduleInfo.lpBaseOfDll);
		auto searchEnd = uint64_t(moduleInfo.lpBaseOfDll) + moduleInfo.SizeOfImage;

		std::vector<uint64_t> results = {};
		uint64_t match = 0;
		const char* patternCursor = pattern;

		// loop through the memory within the search range
		for (uint64_t currentAddress = searchStart; currentAddress < searchEnd; currentAddress++)
		{
			// check if the pattern has been fully matched
			if (!*patternCursor)
			{
				results.push_back(match);
				if (returnFirst)
				{
					return results;
				}
			}

			// check if the current byte matches the pattern or is a wildcard
			if (*(uint8_t*)patternCursor == '\?' || *(uint8_t*)currentAddress == GET_BYTE(patternCursor))
			{
				// save the start address of the match
				if (!match)
				{
					match = currentAddress;
				}

				// check if this is the last byte sequence of the pattern
				if (!patternCursor[2])
				{
					results.push_back(match);
					if (returnFirst)
					{
						return results;
					}
				}

				// move the pattern cursor based on the current pattern character
				if (*(uint16_t*)patternCursor == '\?\?' || *(uint8_t*)patternCursor != '\?')
				{
					patternCursor += 3;
				}
				else
				{
					patternCursor += 2;
				}
			}
			else
			{
				// reset the pattern cursor and match
				patternCursor = pattern;
				match = 0;
			}
		}

		// sort the results vector
		std::sort(results.begin(), results.end());

		return results;
	}

	/// <summary>
	/// Searches for the first occurence of a given pattern within a module
	/// </summary>
	/// <param name="moduleName">The name of the module to search</param>
	/// <param name="pattern">The pattern to search for, formatted as pairs of hexadecimal characters separated by spaces (e.g., "AA BB CC DD")</param>
	std::optional<uint64_t> FindSignature(const char* moduleName, const char* pattern)
	{
		auto patterns = FindSignatures(moduleName, pattern, true);
		if (patterns.empty())
		{
			return std::nullopt;
		}
		return patterns.at(0);
	}

	/// <summary>
	/// Searches for a specific pointer address within a specified module
	/// </summary>
	/// <param name="moduleName">The name of the module to search</param>
	/// <param name="pattern">The pattern to search for, formatted as pairs of hexadecimal characters separated by spaces (e.g., "AA BB CC DD")</param>
	/// <returns></returns>
	std::optional<uint64_t> FindPointer(const char* moduleName, const char* signature)
	{
		// search for the signature within the module
		auto patternAddress = FindSignature(moduleName, signature);
		if (!patternAddress.has_value())
		{
			return std::nullopt;
		}

		// calculate the address of the pointer relative to the pattern address
		uint32_t relativeAddress = *reinterpret_cast<uint32_t*>(*patternAddress + 0x3);

		// calculate the actual pointer by adding the relative address to the instruction pointer
		return *patternAddress + 7 + relativeAddress;
	}
}