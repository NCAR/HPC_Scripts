#include "conf.h"
#include "serialize.h"

#ifndef SRC_SERIALIZABLE_H_
#define SRC_SERIALIZABLE_H_

namespace ibiki
{

/**
 * @brief serializable class object
 * @abstract
 * all objects must be a child of serializable
 * 1 thread at a time
 * @param id serializable object identifier
 *  use id 0 if serializable is to not be used by netobj
 *  if used by netobj, must have globally unique id
 */
template<serialize_t::typeid_t id>
class serializable
{
public:
    typedef serialize_t::typeid_t typeid_t;
    static const typeid_t tid()
    {
        return id;
    }

    /**
     * @brief ctor (non associated with a cache)
     * these should only be used for creating/filling out the serializable objects
     */
    serializable();
    /**
     * @brief dtor
     */
    ~serializable();

private:
    friend class serialize_writer_t;
    friend class object_cache_t;

    object_cache_t * cache_;

    /**
     * @brief Data Size
     * required by cache for accounting
     * since sizeof wont work correctly
     */
    size_t serialize_size_;
};

}

#include "serializable-inc.h"
#endif  // SRC_SERIALIZABLE_H_

