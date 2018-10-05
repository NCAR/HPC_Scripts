#include "conf.h"
#include <cassert>
#include <sstream>
#include <typeinfo>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include "json.h"

namespace ibiki
{

namespace json
{

bool parser_t::filter_comments(std::istream * const ist, std::ostream * const ost)
{
    assert(ist && ost);
    bool filter = false;
    bool in_ws = true;
    char c = '\0';

    while (ist->good() && ost->good())
    {
        ist->get(c);
        bool newline = (c == '\n' || c == '\r');
        bool ws = (c == ' ' || c == '\t' || newline);

        filter = (in_ws && c == '#') ? true : filter;
        if (newline && filter)
            filter = false;

        in_ws = newline || (in_ws && ws);

        if (!filter)
            *ost << c;
    }

    return true;
}

hid_t::format_t parser_t::extract_t::get_type_name(const json_spirit::Value_type &type)
{
    switch (type)
    {
    case json_spirit::null_type:
        return "Null";
    case json_spirit::obj_type:
        return "Object";
    case json_spirit::array_type:
        return "Array";
    case json_spirit::str_type:
        return "String";
    case json_spirit::bool_type:
        return "Boolean";
    case json_spirit::int_type:
        return "Integer";
    case json_spirit::real_type:
        return "Real Number";
    default:
        assert(0);
        return "Unknown";
    }
}

parser_t::parser_t(hid_t * const hid)
        : json_top_(), hid_(hid), success_(false)
{
    assert(hid);
}

parser_t::extract_t parser_t::extract()
{
    assert(hid_);
    assert(success_);

    return extract_t(&json_top_.get_obj(), hid_, "/");
}

bool parser_t::parse(std::istream * const ist, bool strip_comments, bool slow_parse)
{
    assert(ist && *ist);

    std::stringstream fist; ///filtered ist
    if (strip_comments)
    {
        const bool result = filter_comments(ist, &fist);
        if (!result)
        {
            hid_->send_error("DATA", "Read Error", "Unable to read stream to filter comments.");
            return false;
        }
    }

    if (slow_parse)
    {
        try {
            json_spirit::read_or_throw ( strip_comments ? fist : *ist, json_top_ );
        } catch ( json_spirit::Error_position json_err )
        {
            hid_->send_error("DATA",
                             hid_->f("Invalid JSON Syntax line: %1% column: %2%") %
                             json_err.line_ %
                             json_err.column_,
                             hid_->f(json_err.reason_.c_str())
                            );
            return false;
        }
    }
    else ///quick parse
    {
        if (!json_spirit::read(strip_comments ? fist : *ist, json_top_))
        {
            hid_->send_error("DATA", "Invalid JSON Syntax", "Quick Parsed. No details provided.");
            return false;
        }
    }

    if (json_top_.type() != json_spirit::obj_type)
    {
        hid_->send_error("DATA", "json structure error", "expecting an object {} to start json data.");
        return false;
    }

    success_ = true;
    return true;
}

const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, std::string * const obj)
{
    assert(obj);
    result_t result;

    if (json_value.type() == json_spirit::str_type)
    {
        *obj = json_value.get_str();
        result.success = true;
    }
    else if (json_value.type() == json_spirit::bool_type)
    {
        *obj = json_value.get_bool() ? "true" : "false";
        result.success = true;
    }
    else if ( json_value.type() == json_spirit::int_type)
    {
        try
        {
            *obj = boost::lexical_cast<std::string>(json_value.get_int64());
            result.success = true;
        }
        catch (boost::bad_lexical_cast &)
        {
            result.error = hid_t::format_t("Conversion Error from Int64 to String");
        }
    }
    else if ( json_value.type() == json_spirit::real_type)
    {
        try
        {
            *obj = boost::lexical_cast<std::string>(json_value.get_real());
            result.success = true;
        }
        catch (boost::bad_lexical_cast &)
        {
            result.error = hid_t::format_t("Conversion Error from Double to String");
        }
    }
    else
    {
        result.error = hid_t::format_t("Invalid Value Type");
    }

    return result;
}

/**
 * @brief Parse Integer Type
 * @return true if type was integer (false for everything else)
 */
template<typename T>
bool parse_int_type(const json_spirit::mValue &json_value, T * const obj, parser_t::extract_t::result_t * const result)
{
    assert(obj);
    if (json_value.type() == json_spirit::str_type)
    {
        try
        {
            *obj = boost::lexical_cast<T>(json_value.get_str());
            result->success = true;
        }
        catch (boost::bad_lexical_cast &)
        {
            result->error = hid_t::format_t("Conversion Error String \"%1%\" to Integer") % json_value.get_str();
        }
    }
    else if (json_value.type() == json_spirit::bool_type)
    {
        *obj = json_value.get_bool() ? 1 : 0;
        result->success = true;
    }
    else if ( json_value.type() == json_spirit::int_type)
    {
        return true;
    }
    else if ( json_value.type() == json_spirit::real_type)
    {
        *obj = static_cast<T>(json_value.get_real());
        result->success = true;
    }
    else
    {
        result->error =  hid_t::format_t("Invalid Value Type");
    }

    return false;
}

const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, uint64_t * const obj)
{
    assert(obj);
    result_t result;
    if(parse_int_type(json_value, obj, &result))
    {
        *obj = json_value.get_uint64();
        result.success = true;
    }

    return result;
}

const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, int64_t * const obj)
{
    assert(obj);
    result_t result;
    if(parse_int_type(json_value, obj, &result))
    {
        *obj = json_value.get_int64();
        result.success = true;
    }

    return result;
}

const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, int * const obj)
{
    assert(obj);
    result_t result;
    if(parse_int_type(json_value, obj, &result))
    {
        *obj = json_value.get_int();
        result.success = true;
    }

    return result;
}

const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, double * const obj)
{
    assert(obj);
    result_t result;
    if (json_value.type() == json_spirit::str_type)
    {
        try
        {
            *obj = boost::lexical_cast<double>(json_value.get_str());
            result.success = true;
        }
        catch (boost::bad_lexical_cast &)
        {
            result.error =  hid_t::format_t("Conversion Error String \"%1%\" to Double") % json_value.get_str();
        }
    }
    else if (json_value.type() == json_spirit::bool_type)
    {
        *obj = json_value.get_bool() ? 1 : 0;
        result.success = true;
    }
    else if ( json_value.type() == json_spirit::real_type)
    {
        *obj = json_value.get_real();
        result.success = true;
    }
    else if ( json_value.type() == json_spirit::int_type)
    {
        *obj = static_cast<double>(json_value.get_real());
        result.success = true;
    }
    else
    {
        result.error =  hid_t::format_t("Invalid Value Type");
    }

    return result;
}
const parser_t::extract_t::result_t parser_t::extract_t::extract_value(const json_spirit::mValue &json_value, bool * const obj)
{
    assert(obj);
    result_t result;
    if (json_value.type() == json_spirit::str_type)
    {
        const std::string value = boost::to_lower_copy(json_value.get_str());
        if (value == "true" || value == "on" || value == "1")
        {
            *obj = true;
            result.success = true;
        }
        else if (value == "false" || value == "off" || value == "0")
        {
            *obj = false;
            result.success = true;
        }
        else
        {
            result.error = hid_t::format_t("Conversion Error String \"%1%\" to bool. Value must be true/on/1 or false/off/0.") % json_value.get_str();
        }
    }
    else if (json_value.type() == json_spirit::bool_type)
    {
        *obj = json_value.get_bool();
        result.success = true;
    }
    else if ( json_value.type() == json_spirit::int_type)
    {
        if (json_value.get_int64() == 1)
        {
            *obj = true;
            result.success = true;
        }
        else if (json_value.get_int64() == 0)
        {
            *obj = false;
            result.success = true;
        }
        else
        {
            result.error = hid_t::format_t("Conversion Error Integer to bool. Value must be 1 or 0.");
        }
    }
    else if (json_value.type() == json_spirit::real_type)
    {
        if (json_value.get_real() == 1)
        {
            *obj = true;
            result.success = true;
        }
        else if (json_value.get_real() == 0)
        {
            *obj = false;
            result.success = true;
        }
        else
        {
            result.error = hid_t::format_t("Conversion Error Double to bool. Value must be 1 or 0.");
        }
    }
    else
    {
        result.error = hid_t::format_t("Invalid Value Type");
    }

    return result;
}



parser_t::extract_t::extract_t(const json_spirit::mObject * json_obj, hid_t * const hid, const std::string &reference)
        : json_obj_(json_obj), reference_(reference), hid_(hid), success_(true)
{
    assert(hid);
}

parser_t::extract_t parser_t::extract_t::child(const char *field_name)
{
    assert(hid_);
    assert(json_obj_);

    if (!success_)
    {
        //skip!
        return *this;
    }

    json_spirit::mObject::const_iterator field = json_obj_->find(field_name);
    if (field == json_obj_->end())
    {
        success_ = false;
        hid_->send_error("DATA", "Missing Field", hid_->f("Unable to find Field %1%/%2%") % reference_ % field_name);
        return *this;
    }

    if (field->second.type() != json_spirit::obj_type)
    {
        success_ = false;
        hid_->send_error("DATA", "Incorrect Field Type", hid_->f("Requested field is a %1% and not a JSON Object: %2%/%3%") % get_type_name(field->second.type()) % reference_ % field_name);
        return *this;
    }

    const json_spirit::mObject &obj = field->second.get_obj();

    return extract_t(&obj, hid_, reference_ + "/" + field_name + "/");
}

parser_t::extract_t& parser_t::extract_t::child_array_objects(const char *field_name, extract_array_obj_t * const container)
{
    assert(hid_);
    assert(json_obj_);
    assert(container);

    if (!success_)
    {
        //skip!
        return *this;
    }

    json_spirit::mObject::const_iterator field = json_obj_->find(field_name);
    if (field == json_obj_->end())
    {
        success_ = false;
        hid_->send_error("DATA", "Missing Field", hid_->f("Unable to find Field %1%/%2%") % reference_ % field_name);
        return *this;
    }

    if (field->second.type() != json_spirit::array_type)
    {
        success_ = false;
        hid_->send_error("DATA", "Incorrect Field Type", hid_->f("Requested field is a %1% and not a JSON Array: %2%/%3%") % get_type_name(field->second.type()) % reference_ % field_name);
        return *this;
    }

    int index = 0;
    foreach(const json_spirit::mValue &value, field->second.get_array())
    {
        std::string ref;
        try
        {
            ref = reference_ + field_name + "[" + boost::lexical_cast<std::string>(index) + "]";
        }
        catch (boost::bad_lexical_cast &)
        {
            assert(0);
            success_ = false;
            return *this;
        }

        if (value.type() != json_spirit::obj_type)
        {
            success_ = false;
            hid_->send_error("DATA", "Incorrect Field Type", hid_->f("Requested field is a %1% and not a JSON Object: %2%") % get_type_name(value.type()) % ref );
            return *this;
        }

        container->push_back(extract_t(&value.get_obj(), hid_, ref + "/"));

        ++index;
    }

    return *this;
}

parser_t::extract_t::extract_t(const parser_t::extract_t::extract_t &other)
        :  json_obj_(other.json_obj_), reference_(other.reference_), hid_(other.hid_), success_(other.success_)
{
    assert(json_obj_);
    assert(hid_);
}

parser_t::extract_t& parser_t::extract_t::operator=(const   parser_t::extract_t &other)
{
    json_obj_ = other.json_obj_;
    reference_ = other.reference_;
    hid_ = other.hid_;
    success_ = other.success_;

    assert(json_obj_);
    assert(hid_);

    return *this;
}
}
}
