#pragma once
#include <iostream>
#include <cstdint>
#include <string>
#include <vector>

template <typename T>
class Array
{
public:
	/// <summary>
	/// Get the AutoArray's contents
	/// </summary>
	/// <returns>A vector of AutoArray's contents</returns>
	std::vector<T> GetContents();

	/// <returns>A reference to the value at the desired index</returns>
	T& IndexAt(int index);

	/// <summary>
	/// Removes the desired index from the AutoArray
	/// </summary>
	/// <param name="index">The index to be removed</param>
	void Remove(uint32_t index);

	uint32_t* size() { return reinterpret_cast<uint32_t*>(this + 0x8); }
};

class RString
{
public:
	/// <returns>A pointer to the length of the RString</returns>
	size_t* length() { return reinterpret_cast<size_t*>(this + 0x8); }
	
	/// <returns>A pointer to the contents of the RString</returns>
	char* contents() { return reinterpret_cast<char*>(this + 0x10); }
};