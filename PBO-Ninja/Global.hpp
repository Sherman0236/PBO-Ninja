#pragma once
#include "SDK/Engine.hpp"
#include "SDK/QFBank.hpp"
#include "Utils.hpp"

// only a single .data offset is required
#define O_GFileBanks 0x40E3920

/// <summary>
/// Holds variables that are used throughout the project
/// </summary>
namespace Global
{
	inline Array<QFBank*>* s_fileBanks = nullptr;
}