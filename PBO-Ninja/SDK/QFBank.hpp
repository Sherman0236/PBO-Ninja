#pragma once
#include "Engine.hpp"
#include <string>
#include <filesystem>
#include <format>
#include <fstream>

/// <summary>
/// A class representing a file buffer interface
/// </summary>
class IFileBuffer
{
public:
	/// <summary>
	/// Reads a specified section of the file buffer into a vector of bytes
	/// </summary>
	/// <returns>A vector of bytes</returns>
	std::vector<uint8_t> Read(uint32_t startOffset = 0, size_t length = 0)
	{
		// if length is not specified, read the entire buffer
		if (length == 0)
		{
			length = size();
		}

		// allocate a buffer to store the data
		auto buffer = reinterpret_cast<uint8_t*>(malloc(length));

		typedef bool(__fastcall* fnFileBuffer_Read)(IFileBuffer*, void*, uint64_t, size_t);
		fnFileBuffer_Read read = (fnFileBuffer_Read)((*(uint64_t**)this)[10]);
		
		// if the read function exists (it should always exist), call it to read the data into the buffer
		if (read != nullptr)
		{
			read(this, buffer, startOffset, length);
		}

		// create a vector from the buffer and return it as the result
		auto result = std::vector<uint8_t>(buffer, buffer + length);

		// free the buffer
		free((void*)buffer);

		return result;
	}

	const size_t size() { return *reinterpret_cast<uint32_t*>(this + 0x18); }
};

/// <summary>
/// A class representing a bank of files (an EBO/PBO that is loaded in memory)
/// </summary>
class QFBank
{
public:
	/// <summary>
	/// An 'as in memory' structure representing information about a bank file
	/// </summary>
#pragma pack(push, 1)
	struct FileInfoO
	{
		RString* m_name;
		uint32_t m_compressedMagic;
		uint32_t m_uncompressedMagic;
		uint32_t m_startOffset;
		uint32_t m_time;
		uint32_t m_length;
	};
#pragma pack(pop)

	/// <summary>
	/// Constructs the file name based on the open name (path) of the bank
	/// </summary>
	/// <returns>The file name (with extension) of the bank</returns>
	std::string GetFileName()
	{
		return std::filesystem::path(openName()->contents()).filename().string();
	}

	/// <summary>
	/// Constructs a display name based on the open name and prefix
	/// </summary>
	/// <returns>The generated display name of the bank</returns>
	std::string GetDisplayName()
	{
		auto fileName = std::filesystem::path(openName()->contents()).filename().string();
		return std::format("{}{}", prefix()->contents(), fileName);
	}

	/// <summary>
	/// Dumps the bytes of the file bank. If the bank is encrypted, the return will be decrypted
	/// </summary>
	/// <returns>The file bank's unencrypted bytes</returns>
	std::vector<uint8_t> GetBytes(uint32_t startOffset = 0, size_t length = 0);

	/// <summary>
	/// Gets the sha hash of the file bank (stored at the last 20 bytes of the file)
	/// and generates a string to represent it
	/// </summary>
	/// <returns>The sha hash string</returns>
	std::string GetHash();

	/// <summary>
	/// Iterates over a '2D array' containing information about each of the bank's files
	/// </summary>
	/// <returns>A vector of FileInfoO objects</returns>
	std::vector<FileInfoO> GetFiles();

	/// <summary>
	/// The open name is the name provided by the QFBank::Open call
	/// </summary>
	/// <returns>The path to the bank on disk</returns>
	RString*& openName() { return *reinterpret_cast<RString**>(this + 0x18); }

	/// <summary>
	/// The prefix is the relative path of the folder the bank resides in
	/// </summary>
	/// <returns>The bank's prefix</returns>
	RString*& prefix() { return *reinterpret_cast<RString**>(this + 0x38); }

	/// <summary>
	/// The file buffer is how the game reads the qfbank's data fromdisk
	/// </summary>
	/// <returns>The bank's file buffer</returns>
	IFileBuffer* fileBuffer() { return *reinterpret_cast<IFileBuffer**>(this + 0x70); }

	bool* locked() { return reinterpret_cast<bool*>(this + 0x30); }
	bool* lockable() { return reinterpret_cast<bool*>(this + 0x31); }
	bool* verifySignature() { return reinterpret_cast<bool*>(this + 0x32); }
	bool* isPatched() { return reinterpret_cast<bool*>(this + 0x33); }
	bool* loaded() { return reinterpret_cast<bool*>(this + 0x78); }
	bool* error() { return reinterpret_cast<bool*>(this + 0x79); }
	const uint32_t headerStart() { return *reinterpret_cast<uint32_t*>(this + 0x80); }
};