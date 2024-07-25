#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "Inc.hpp"

// Comment: Look like it's broken
template <typename Ty_>
class RingBuffer {
public:
	RingBuffer(const size_t bufferSize_, const size_t bufferCount_) :
		_pp(bufferSize_ * bufferCount_),
		_bufferSize(bufferSize_),
		_bufferCount(bufferCount_),
		_bufferPtr(nullptr),
		_gpos(_pp - 1),
		_spos(0) {
		if (_bufferCount < 3)
			throw std::runtime_error(
				"Buffer count should be more than 3, cause writer and reader can't join the same buffer index."
			);
		createBuffer();
	}

	~RingBuffer() {
		for (size_t i = 0; i < _bufferCount; i++)
			delete[] _bufferPtr[i];
	}

	friend std::ostream& operator>>(std::ostream&, const RingBuffer<Ty_>& buffer_);

	friend std::istream& operator<<(std::istream&, RingBuffer<Ty_>& buffer_);

	Ty_& read() const {
		return internalRead();
	}

	void read(Ty_* buffer_, const size_t offset_, const size_t length_) const {
		const size_t limit = offset_ + length_;
		for (size_t i = offset_; i < limit; i++)
			buffer_[i] = internalRead();
	}

	void write(const Ty_& value_) {
		internalWrite(value_);
	}

	void write(const Ty_* buffer_, const size_t offset_, const size_t length_) {
		const size_t limit = offset_ + length_;
		for (size_t i = offset_; i < limit; i++)
			internalWrite(buffer_[i]);
	}

private:
	mutable std::condition_variable _cond;
	mutable std::mutex _rmtx;
	mutable std::mutex _wmtx;

	struct buffer {
		Ty_* ptr = nullptr;
	};

protected:
	void createBuffer() {
		if (_bufferPtr != nullptr)
			throw std::runtime_error("Can not create buffer, while another already occured.");

		_bufferPtr = new buffer[_bufferCount];
		for (size_t i = 0; i < _bufferCount; i++)
			_bufferPtr[i].ptr = new Ty_[_bufferSize];
	}

	void refreshBuffer(const size_t index_) {
		delete[] _bufferPtr[index_].ptr;
		_bufferPtr[index_].ptr = new Ty_[_bufferSize];
	}

	void increment(int_fast32_t& pos_) const {
		++pos_;
		while (pos_ >= _pp)
			pos_ -= _pp;
	}

	void increment(std::atomic_int_fast32_t& pos_) const {
		++pos_;
		while (pos_ >= _pp)
			pos_ -= _pp;
	}

	Ty_& internalRead() const {
		int_fast32_t gpos = _gpos;
		increment(gpos);

		if ((gpos / _bufferSize) == (_spos / _bufferSize)) {
			std::unique_lock<std::mutex> lock(_rmtx);
			_cond.wait(
				lock,
				[this, gpos] {
					return (gpos / _bufferSize != _spos / _bufferSize);
				}
			);
		}

		increment(_gpos);

		const size_t bufferIndex = static_cast<size_t>(gpos / _bufferSize);
		buffer& b = _bufferPtr[bufferIndex];

		Ty_& val = b.ptr[_gpos - bufferIndex * _bufferSize];
		_cond.notify_one();

		return val;
	}

	void internalWrite(const Ty_& val_) {
		int_fast32_t spos = _spos;
		increment(spos);

		if ((spos / _bufferSize) == (_gpos / _bufferSize)) {
			std::unique_lock<std::mutex> lock(_wmtx);
			_cond.wait(
				lock,
				[this, spos] {
					return (_gpos / _bufferSize != spos / _bufferSize);
				}
			);
		}

		const size_t bufferIndex = static_cast<size_t>(_spos / _bufferSize);
		buffer& b = _bufferPtr[bufferIndex];

		b.ptr[_spos - bufferIndex * _bufferSize] = val_;
		increment(_spos);
		_cond.notify_one();
	}

private:
	size_t _pp;

	size_t _bufferSize;
	size_t _bufferCount;

	buffer* _bufferPtr;

	mutable std::atomic_int_fast32_t _gpos;
	mutable std::atomic_int_fast32_t _spos;
};

template <typename Ty_>
std::ostream& operator>>(std::ostream& stream_, const RingBuffer<Ty_>& buffer_) {
	stream_ << buffer_.read();
	return stream_;
}

template <typename Ty_>
std::istream& operator<<(std::istream& stream_, RingBuffer<Ty_>& buffer_) {
	const Ty_ val;
	stream_ >> val;
	buffer_.write(val);
	return stream_;
}
