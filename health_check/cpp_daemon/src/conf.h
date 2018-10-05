#ifndef SRC_CONF_H_
#define SRC_CONF_H_

///Forward Declarations
namespace boost
{
namespace system
{
class system_error;
}
namespace asio
{
class io_service;
}
class exception;
}

namespace ibiki
{
class hid_t;
class configuration_t;
class lserver_t;
class serialize_writer_t;
class serialize_reader_t;
class object_cache_t;
class lserver_t;
}

typedef const char * cstring;

/**
  * @brief Network Authorization Key Size (bytes)
  * All keys must be exactly this length
  * size = sha1sum output
  */
#define NETWORK_KEY_SIZE 40u
/**
  * @brief Default Max Network Connection Count
  * can be overriden with config
  * @warning only config_t should use this define
  */
#define MAX_NETWORK_CONNECTIONS_DEFAULT 500
/**
  * @brief Default Network read cache size
  * can be overriden with config
  * @warning only config_t should use this define
  */
#define NETWORK_CACHE_READ_DEFAULT 5
/**
  * @brief Default Network write cache size
  * can be overriden with config
  * @warning only config_t should use this define
  */
#define NETWORK_CACHE_WRITE_DEFAULT 5
/**
  * @brief Default number of threads
  * can be overriden with config
  * @warning only config_t should use this define
  */
#define GLOBAL_THREADS_COUNT_DEFAULT 3

/**
  * assume little endian but assert it in code
  * where it matters
  */
#define LITTLE_ENDIAN_ARCH true
/**
  * @brief Max size of the dispatcher posts pool
  * keeps the pool from leaking ram forever if there
  * is a burst of posts
  * this should be increased based on average number
  * of posts
  */
#define DISPATCHER_POST_POOL_MAX 0u

#include "env_config.h"

//Make sure THREAD SAFE is Active
#define BOOST_SPIRIT_THREADSAFE

//make it more legible
#define foreach         BOOST_FOREACH
#define reverse_foreach BOOST_REVERSE_FOREACH

//#include <boost/random/random_device.hpp>

/**
 * @brief Disables copy and assign operators for classes
 * @code
class Foo {
 public:
  Foo(int f);
  ~Foo();

 private:
  DISALLOW_COPY_AND_ASSIGN(Foo);
};
 * @endcode
 * @see http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml?showone=Copy_Constructors#Copy_Constructors
 */
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)


typedef unsigned char uchar;


#endif  // SRC_CONF_H_
