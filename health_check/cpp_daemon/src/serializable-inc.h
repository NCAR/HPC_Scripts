#include "conf.h"
#include "serializable.h"
#include "cache_object.h"

#ifndef SRC_SERIALIZABLE_INC_H_
#define SRC_SERIALIZABLE_INC_H_

namespace ibiki
{

template<serialize_t::typeid_t id>
serializable<id>::~serializable()
{
    if(cache_)
        cache_->release(this);
}

template<serialize_t::typeid_t id>
serializable<id>::serializable()
        : cache_(NULL), serialize_size_(0u)
{

}

}

#endif  // SRC_SERIALIZABLE_INC_H_
