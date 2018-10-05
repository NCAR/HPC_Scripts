#include "conf.h"
#include<string>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include "json_spirit.h"

#ifndef SRC_JSON_INC_H_
#define SRC_JSON_INC_H_

namespace ibiki
{

namespace json
{
template<typename T>
parser_t::extract_t & parser_t::extract_t::field(const char *field_name, T * const obj, const T &def)
{
    assert(hid_);
    assert(json_obj_);
    assert(obj);

    if (!success_)
    {
        //skip!
        return *this;
    }

    json_spirit::mObject::const_iterator field = json_obj_->find(field_name);
    if (field == json_obj_->end())
    {
        *obj = def;
        return *this;
    }
    else ///found, use normal method
      return this->field(field_name, obj);
}

template<typename T>
parser_t::extract_t & parser_t::extract_t::field(const char *field_name, T * const obj)
{
    assert(hid_);
    assert(json_obj_);
    assert(obj);

    if (!success_)
    {
        //skip!
        return *this;
    }

    json_spirit::mObject::const_iterator field = json_obj_->find(field_name);
    if (field == json_obj_->end())
    {
        success_ = false;
        hid_->send_error("DATA", "Missing Field", hid_->f("Unable to find Field %1% in %2%") % field_name % reference_);
        return *this;
    }

    const result_t result = extract_value(field->second, obj);

    success_ = result.success;
    if (!result.success)
    {
        success_ = false;
        hid_->send_error("DATA", "Invalid Field", hid_->f("Unable to extract Field from %1%/%2%. Json Error: %3%") % reference_ % field_name % result.error);
    }

    return *this;
}

template<typename T>
parser_t::extract_t & parser_t::extract_t::child_array(const char *field_name, std::vector<T> * const container)
{
    assert(hid_);
    assert(json_obj_);
    assert(container);
    assert(container->size() == 0);

    if (!success_)
    {
        //skip!
        return *this;
    }

    json_spirit::mObject::const_iterator field = json_obj_->find(field_name);
    if (field == json_obj_->end())
    {
        success_ = false;
        hid_->send_error("DATA", "Missing Field", hid_->f("Unable to find Field %1% in %2%") % field_name % reference_);
        return *this;
    }

    if (field->second.type() != json_spirit::array_type)
    {
        success_ = false;
        hid_->send_error("DATA", "Invalid Field  Type", hid_->f("Requested field %1%/%2% is %3% and not a JSON Array.") % reference_ % field_name % get_type_name(field->second.type()));
        return *this;
    }

    int index = 0;
    foreach(const json_spirit::mValue &value, field->second.get_array())
    {
        if (value.type() != json_spirit::str_type && value.type() != json_spirit::bool_type && value.type() != json_spirit::real_type && value.type() != json_spirit::int_type)
        {
            success_ = false;
            hid_->send_error("DATA", "Invalid Field  Type", hid_->f("Requested field %1%/%2%[%3%] is %4% and not a JSON String/Integer/Bool/Real.") % reference_ % field_name % index % get_type_name(field->second.type()));
            return *this;
        }

        T buffer;
        const result_t result = extract_value(value, buffer);
        if (!result.success)
        {
            success_ = false;
            hid_->send_error("DATA", "Invalid Field", hid_->f("Unable to extract Field %1%/%2%[%3%]. Extract Error: %4%") % reference_ % field_name % index % result.error);
            return *this;
        }

        container->push_back(buffer);

        ++index;
    }

    return *this;
}

}
}

#endif  // SRC_JSON_INC_H_
