#include "conf.h"
#include <cassert>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <netinet/in.h>
// #include <boost/foreach.hpp>
#include <boost/detail/endian.hpp>
#include "serialize.h"
#include "cache_object.h"

namespace ibiki
{

const serialize_t::string_size_t serialize_t::STRING_SIZE_MAX = UINT16_MAX;

serialize_t::serialize_t()
        : success_(true)
{
}

serialize_writer_t::serialize_writer_t()
        : bytes_written_(0u)
{
}

serialize_writer_t::serialize_writer_t(const boost::shared_ptr<buffer_t> write_buffer)
        : buffer_(write_buffer), bytes_written_(0u)
{
    assert(write_buffer);

    if (buffer_)
    {
        assert(buffer_->begin() != buffer_->end());
        cur = buffer_->begin();
    }
}

serialize_reader_t::serialize_reader_t(const boost::shared_ptr<const buffer_t> read_buffer)
        : buffer_(read_buffer), bytes_read_(0u), cur(buffer_->begin())
{
    assert(read_buffer);
    assert(buffer_->begin() != buffer_->end());
}

serialize_reader_t& serialize_reader_t::operator % (std::string &other) //NOLINT
{
    string_size_t size;
    read_integer(size);
    assert(buffer_->size() >= bytes_read_ + size);
    other.resize(size);
    other.assign(cur, cur + size);
    cur += size;
    bytes_read_ += size;

    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (uchar &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (uint16_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (int16_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (int32_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (uint32_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (int64_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_reader_t& serialize_reader_t::operator % (uint64_t &other) //NOLINT
{
    read_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const std::string &other) //NOLINT
{
    const string_size_t real_size = other.length();
    string_size_t size;

    assert(real_size <= STRING_SIZE_MAX);
    if (real_size > STRING_SIZE_MAX)
    {
        size = STRING_SIZE_MAX;
        success_ = false;
    }
    else
        size = static_cast<string_size_t>(real_size);

    write_integer(size);

    if (buffer_)
    {
        assert(buffer_->size() >= bytes_written_ + size);
        for (string_size_t i = 0; i < size; ++i)
            *cur++ = static_cast<uchar>(other[i]);
    }

    bytes_written_ += size;

    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const uchar &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const int16_t &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const uint16_t &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const int32_t &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const uint32_t &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const int64_t &other)
{
    write_integer(other);
    return *this;
}

serialize_writer_t& serialize_writer_t::operator % (const uint64_t &other)
{
    write_integer(other);
    return *this;
}

}
