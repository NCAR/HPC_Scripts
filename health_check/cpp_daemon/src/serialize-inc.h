#include "conf.h"
#include <cassert>
#include "serialize.h"

// #include <iostream>
// #include <iomanip>

#ifndef SRC_SERIALIZE_INC_H_
#define SRC_SERIALIZE_INC_H_

namespace ibiki
{

template<typename T>
bool serialize_reader_t::operator >> (T &other) // NOLINT
{
    other.serialize(*this);
    return success_;
}

template<typename T>
bool serialize_writer_t::operator << (const T &constother)
{
    ///need to remove const protection, which is poor coding
    ///but other won't written by this class
    T &other = const_cast<T&>(constother);
    other.serialize(*this);
    return success_;
}

template<typename T>
void serialize_writer_t::write_integer(const T &other)
{
    assert(LITTLE_ENDIAN_ARCH);

    if (buffer_)
    {
        assert(buffer_->size() >= bytes_written_ + sizeof(other));
        const uchar * ptr = static_cast<const uchar *>(static_cast<const void *>(&other));
        for (int i = sizeof(other); i != 0; --i)
            *cur++ = ptr[i - 1];
    }

//     std::cout << "write int["<<sizeof(other)<<"]: "<< std::dec << other << " = " << std::hex<< std::setw(sizeof(other)*2) << std::setfill('0') << other << std::endl;

    bytes_written_ += sizeof(other);
}

template<typename T>
void serialize_reader_t::read_integer(T &other) //NOLINT
{
    assert(LITTLE_ENDIAN_ARCH);
    assert(buffer_->size() >= bytes_read_ + sizeof(other));
    uchar * ptr = static_cast<uchar *>(static_cast<void *>(&other));
    for (int i = sizeof(other); i != 0; --i)
        ptr[i - 1] = *cur++;

//     std::cout << "read int["<<sizeof(other)<<"]: "<< std::dec << other << " = " << std::hex<< std::setw(sizeof(other)*2) << std::setfill('0') << other << std::endl;

    bytes_read_ += sizeof(other);
}

}

#endif  // SRC_SERIALIZE_INC_H_
