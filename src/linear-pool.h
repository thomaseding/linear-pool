#pragma once


#include <string.h>

#include <memory>
#include <type_traits>



class LinearPool {
	LinearPool(LinearPool const &) = delete;
	void operator= (LinearPool const &) = delete;

public:
	~LinearPool()
	{
		while (head_memory_buffer != nullptr) {
			char * next;
			memcpy(&next, head_memory_buffer + buffer_capacity, sizeof(char *));
			delete [] head_memory_buffer;
			head_memory_buffer = next;
		}
	}

	LinearPool(size_t buffer_capacity)
		: open_byte_count(buffer_capacity)
		, buffer_capacity(buffer_capacity)
		, head_memory_buffer(NewBuffer())
		, open_memory(head_memory_buffer)
	{
		char * next = nullptr;
		memcpy(open_memory + open_byte_count, &next, sizeof(char *));
	}

	LinearPool(LinearPool && other)
	{
		*this = std::move(other);
	}

	void operator= (LinearPool && other)
	{
		open_byte_count = other.open_byte_count;
		buffer_capacity = other.buffer_capacity;
		head_memory_buffer = other.head_memory_buffer;
		open_memory = other.open_memory;

		other.head_memory_buffer = nullptr;
	}

	template <typename T, typename... Args>
	T * New(Args... && args)
	{
		void * p = Alloc(sizeof(T), std::alignment_of<T>::value);
		if (p == nullptr) {
			return nullptr;
		}
		return new(p) T(std::forward<Args>(args)...);
	}

	template <typename T>
	static void Delete(T * p)
	{
		p->~T();
	}

private:
	char * NewBuffer()
	{
		return new char[buffer_capacity + sizeof(char *)];
	}

	void * Alloc(size_t amount, size_t alignment)
	{
		void * p = open_memory;
		void * aligned = std::align(alignment, amount, p, open_byte_count);
		p = static_cast<char *>(p) + amount;

		auto aligned_amount = static_cast<size_t>(static_cast<char *>(p) - open_memory);


		if (aligned_amount <= open_byte_count) {
			open_byte_count -= aligned_amount;
			open_memory = static_cast<char *>(p);
			return aligned;
		}

		if (amount > buffer_capacity) {
			return nullptr;
		}

		char * next = NewBuffer();
		if (next == nullptr) {
			return nullptr;
		}

		memcpy(open_memory + open_byte_count, &next, sizeof(char *));
		open_byte_count = buffer_capacity - amount;
		open_memory = next + amount;

		return next;
	}

private:
	size_t open_byte_count;
	size_t buffer_capacity;
	char * head_memory_buffer;
	char * open_memory;
};





