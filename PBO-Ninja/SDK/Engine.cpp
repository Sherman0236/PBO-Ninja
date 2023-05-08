#include "Engine.hpp"
#include "QFBank.hpp"

template <typename T>
std::vector<T> Array<T>::GetContents()
{
	T* contents = *reinterpret_cast<T**>(this);
	if (contents == nullptr)
	{
		throw std::runtime_error("array has not been allocated");
	}

	std::vector<T> result = {};

	for (uint32_t i = 0; i < *size(); i++)
	{
		result.push_back(contents[i]);
	}

	return result;
}

template <typename T>
T& Array<T>::IndexAt(int index)
{
	T* contents = *reinterpret_cast<T**>(this);
	if (contents == nullptr)
	{
		throw std::runtime_error("array has not been allocated");
	}

	return contents[index];
}

template <typename T>
void Array<T>::Remove(uint32_t index)
{
	T* contents = *reinterpret_cast<T**>(this);
	if (contents == nullptr)
	{
		throw std::runtime_error("array has not been allocated");
	}

	auto originalSize = *size();
	*size() -= 1;

	for (uint32_t i = index; i < originalSize - 1; i++)
	{
		contents[i] = contents[i + 1];
	}

	contents[originalSize] = 0;
}

template class Array<QFBank*>;