#ifndef RAPIDJSON_FILEREADSTREAM_H_
#define RAPIDJSON_FILEREADSTREAM_H_

#include "rapidjson.h"
#include <cstdio>

namespace rapidjson {

//! Wrapper of C file stream for input using fread().
/*!
	\implements Stream
*/
class FileReadStream {
public:
	typedef char Ch;	//!< Character type. Only support char.

	FileReadStream(FILE* fp, char* buffer, size_t bufferSize) : fp_(fp), buffer_(buffer), bufferSize_(bufferSize), bufferLast_(0), current_(buffer_), readCount_(0), count_(0), eof_(false) { 
		RAPIDJSON_ASSERT(fp_ != 0);
		Read();
	}

	char Peek() const { return *current_; }
	char Take() { char c = *current_; Read(); return c; }
	size_t Tell() const { return count_ + (current_ - buffer_); }

	// Not implemented
	void Put(char c) { RAPIDJSON_ASSERT(false); }
	void Flush() { RAPIDJSON_ASSERT(false); } 
	char* PutBegin() { RAPIDJSON_ASSERT(false); return 0; }
	size_t PutEnd(char*) { RAPIDJSON_ASSERT(false); return 0; }

private:
	void Read() {
		if (current_ < bufferLast_)
			++current_;
		else
			FillBuffer();
	}
	
	void FillBuffer() {
		if (!eof_) {
			count_ += readCount_;
			readCount_ = fread(buffer_, 1, bufferSize_, fp_);
			bufferLast_ = buffer_ + readCount_ - 1;
			current_ = buffer_;

			if (readCount_ < bufferSize_) {
				buffer_[readCount_] = '\0';
				++bufferLast_;
				eof_ = true;
			}
		}
	}

	FILE* fp_;
	char *buffer_;
	size_t bufferSize_;
	char *bufferLast_;
	char *current_;
	size_t readCount_;
	size_t count_;	//!< Number of characters read
	bool eof_;
};

} // namespace rapidjson

#endif // RAPIDJSON_FILESTREAM_H_
