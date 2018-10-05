#include "conf.h"
#include "cache.h"
#include "cache_buffer.h"

#ifndef SRC_SERIALIZE_H_
#define SRC_SERIALIZE_H_

namespace ibiki
{
/**
 * @brief Serializer
 * @abstract
 * trys to match boost::serialize as much as possible
 * but without all the nasty bells and whistles
 * that one might *easily* shoot themselves in the foot using
 */
class serialize_t
{
public:
    typedef buffer_cache_t::buffer_t buffer_t;
    typedef uint8_t typeid_t; ///type id

    /**
      * @brief Were all operations successful?
      * @warning must have mutex lock()
      */
    operator const bool() const {
        return success_;
    }

protected:
    typedef uint16_t string_size_t;
    static const string_size_t STRING_SIZE_MAX;

    /**
      * @brief ctor
      * only for child classes
      */
    serialize_t();

    /**
    * @brief Were all operations successful?
    */
    bool success_;

    DISALLOW_COPY_AND_ASSIGN(serialize_t);
};

/**
  * @brief Reader Serializer
  */
class serialize_reader_t : public serialize_t
{
public:
    explicit serialize_reader_t(const boost::shared_ptr<const buffer_t> read_buffer);

    /**
      * @brief read serialized copy of other to buffer
      * @param true on success
      */
    template<typename T>
    bool operator >> (T &other); // NOLINT

    serialize_reader_t& operator % (std::string &other); // NOLINT
    serialize_reader_t& operator % (uchar &other); // NOLINT
    serialize_reader_t& operator % (uint16_t &other); // NOLINT
    serialize_reader_t& operator % (int16_t &other); // NOLINT
    serialize_reader_t& operator % (int32_t &other); // NOLINT
    serialize_reader_t& operator % (uint32_t &other); // NOLINT
    serialize_reader_t& operator % (int64_t &other); // NOLINT
    serialize_reader_t& operator % (uint64_t &other); // NOLINT

    size_t bytes_read() const {
        return bytes_read_;
    }
private:
    /**
       * @brief Read integer (does not fix endian)
       */
    template<typename T>
    void read_integer(T &other); //NOLINT

    /**
      * @brief read buffer
      */
    const boost::shared_ptr<const buffer_t> buffer_;
    /**
      * @brief Number of bytes read
      */
    size_t bytes_read_;

    /**
     * @brief current position in buffer
     */
    buffer_t::const_iterator cur;

private:
    DISALLOW_COPY_AND_ASSIGN(serialize_reader_t);
};

/**
  * @brief Writer Serializer
  */
class serialize_writer_t : public serialize_t
{
public:

    /**
     * @brief ctor for writting
     */
    explicit serialize_writer_t(const boost::shared_ptr<buffer_t> write_buffer);
    /**
      * @brief ctor for writting (only counts bytes written)
      */
    explicit serialize_writer_t();

    /**
      * @brief write serialized copy of other to buffer
      * @param T must be child of serializable
      * @param true on success
      */
    template<typename T>
    bool operator << (const T& constother);

    serialize_writer_t& operator % (const std::string &other);
    serialize_writer_t& operator % (const uchar &other);
    serialize_writer_t& operator % (const uint16_t &other);
    serialize_writer_t& operator % (const int16_t &other);
    serialize_writer_t& operator % (const int32_t &other);
    serialize_writer_t& operator % (const uint32_t &other);
    serialize_writer_t& operator % (const int64_t &other);
    serialize_writer_t& operator % (const uint64_t &other);

    size_t bytes_written() const {
        return bytes_written_;
    }

private:
    /**
      * @brief Write integer (does not fix endian)
      */
    template<typename T>
    void write_integer(const T &other);

    //const_pointer_cast<int>()
    /**
      * @brief write buffer (may be null)
      */
    const boost::shared_ptr<buffer_t> buffer_;
    /**
      * @brief Number of bytes written
      */
    size_t bytes_written_;

    /**
     * @brief current position in buffer
     */
    buffer_t::iterator cur;

private:
    DISALLOW_COPY_AND_ASSIGN(serialize_writer_t);
};

}

#include "serialize-inc.h"
#endif  // SRC_SERIALIZE_H_

