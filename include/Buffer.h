#ifndef BUFFER_H
#define BUFFER_H
#include <vector>
class Buffer
{
public:
	static constexpr size_t IniteSize = 1024;
	Buffer() :
		_buffer(IniteSize),
		_readIndex(0),
		_writeIndex(0)
	{

	}
	void Swap(Buffer& buffer)
	{
		_buffer.swap(buffer._buffer);
		std::swap(buffer._readIndex, _readIndex);
		std::swap(buffer._writeIndex, _writeIndex);
	}
	size_t ReadableBytes()
	{
		return _writeIndex - _readIndex;
	}
	size_t writableBytes()
	{
		return _buffer.size() - _writeIndex;
	}
	const char* begin() const
	{
		return &*_buffer.begin();
	}
	char* begin()
	{
		return &*_buffer.begin();
	}
	const char* ReadPos()
	{
		return begin() + _readIndex;
	}
	char* WritePos()
	{
		return begin() + _writeIndex;
	}
	void RetrieveAll()
	{
		_readIndex = 0;
		_writeIndex = 0;
	}

	void Retrieve(size_t len)
	{
		if (ReadableBytes() > len)
		{
			_readIndex += len;
		}
		else
		{
			RetrieveAll();
		}
	}

	void HasWrite(size_t len)
	{
		
		_writeIndex += len;
		
	}
	void HasWriteToEnd()
	{
		_writeIndex = _buffer.size();
	}
	void AppendData(const char*data, size_t n)
	{
		if (writableBytes() < n)
		{
			ExtendSpace(n);
		}
		std::copy(data, data + n, WritePos());
		HasWrite(n);
	}
	void ExtendSpace(size_t len)
	{
		if (writableBytes() + _readIndex < len)
		{
			_buffer.resize(_writeIndex + len);
		}
		else
		{
			size_t readable = ReadableBytes();
			std::copy(begin() + _readIndex, begin() + _writeIndex, begin());
			_readIndex = 0;
			_writeIndex = readable;
		}
	}
	~Buffer()
	{

	}

private:
	std::vector<char> _buffer;
	size_t _readIndex;
	size_t _writeIndex;
};
#endif // !BUFFER_H

