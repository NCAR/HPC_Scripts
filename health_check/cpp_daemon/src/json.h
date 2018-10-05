#include "conf.h"
#include<string>
#include "json_spirit.h"
#include "hid.h"

#ifndef SRC_JSON_H_
#define SRC_JSON_H_

namespace ibiki
{

namespace json
{

class parser_t
{
public:
    class extract_t
    {
    public:
        /**
          * @brief Were all operations successful?
          */
        operator const bool() const {
            return success_;
        }

        /**
          * @short Get Reference String
          */
        const std::string & reference() const {
            return reference_;
        }

        /**
          * @brief Extract Field and report to hid on failure
          * @param field_name Field Name
          * @param obj Object to Extract into from field
          * @return class with results
          */
        template<typename T>
        extract_t & field(const char *field_name, T * const obj);
        template<typename T>
        extract_t & f(const char *field_name, T * const obj)
        {
            return field(field_name, obj);
        }

        /**
          * @brief Extract Field and report to hid on failure
          * @param field_name Field Name
          * @param obj Object to Extract into from field
          * @param def default value if field not found
          * @return class with results
          */
        template<typename T>
        extract_t & field(const char *field_name, T * const obj, const T &def);
        template<typename T>
        extract_t & f(const char *field_name, T * const obj, const T &def)
        {
            return field(field_name, obj, def);
        }

        /**
          * @brief Extract Child Object
          * @param field_name Field Name
          */
        extract_t child(const char *field_name);
        extract_t c(const char *field_name)
        {
            return child(field_name);
        }

        typedef std::vector<extract_t>  extract_array_obj_t;

        /**
        * @brief Extract Child Array of Objects
        * @param field_name Field Name
        * @param container Container to fill with extract_t objects
        * @return vector of extract objects
        */
        extract_t & child_array_objects(const char *field_name, extract_array_obj_t * const container);
        extract_t & ao(const char *field_name, extract_array_obj_t * const container)
        {
            return child_array_objects(field_name, container);
        }

        /**
        * @brief Extract Child Array of String/Real/Bool/Integer
        * @param field_name Field Name
        * @return vector of T
        */
        template<typename T>
        extract_t & child_array(const char *field_name, std::vector<T> * const container);
        template<typename T>
        extract_t & a(const char *field_name, std::vector<T> * const container)
        {
            return child_array(field_name, container);
        }
        /**
          * @brief copy ctor
          */
        extract_t(const extract_t &other);
        /**
          * @brief assignment op
          */
        extract_t& operator=(const extract_t &other);

    private:
        friend class parser_t;

        /**
        * @brief Get Type Name as String
        * @param type Type to get name of
        * @return string of type name
        */
        static hid_t::format_t get_type_name(const json_spirit::Value_type &type);

        /**
          * @brief ctor with reference for child extracts
          * @param json_obj Json Object
          * @param hid Hid to send errors
          * @param refernce Position of data in JSON tree, used as reference for errors
          */
        extract_t(const json_spirit::mObject * json_obj, hid_t * const hid, const std::string &reference);

        /**
          * @brief Extract Result Struct
          * Used to return if an extraction was success or not and why not
          */
        struct result_t
        {
public:
            /**
              * @brief was extraction successful?
              */
            bool success;
            /**
              * @brief error description
              */
            hid_t::format_t error;

            /**
              * @brief ctor with sucess being false
              */
            result_t() : success(false), error() {}
        };

        /**
        * @short Extract Field from Json Object
        * @param json_obj Json Object
        * @param field Field Name
        * @param obj Object to Extract into from field
        * @return struct with results
        * Note: Will attempt to convert field value to typeof obj
        */
        static const result_t extract_value(const json_spirit::mValue &json_value, std::string * const obj);
        static const result_t extract_value(const json_spirit::mValue &json_value, double * const obj);
        static const result_t extract_value(const json_spirit::mValue &json_value, uint64_t * const obj);
        static const result_t extract_value(const json_spirit::mValue &json_value, int64_t * const obj);
        static const result_t extract_value(const json_spirit::mValue &json_value, int * const obj);
        static const result_t extract_value(const json_spirit::mValue &json_value, bool * const obj);

        /**
          * @brief Json object to query
          */
        const json_spirit::mObject * json_obj_;
        /**
          * @brief refernce Position of data in JSON tree, used as reference for errors
          */
        std::string reference_;
        /**
          * @brief Attached hid
          */
        hid_t * hid_;
        /**
          * @brief Were all operations successful?
          */
        bool success_;
    };

public:
    /**
      * @brief ctor
      * @param hid HID to report errors
      */
    explicit parser_t(hid_t * const hid);

    /**
      * @brief Parse JSON Stream
      * @param ist JSON Stream to Parse
      * @param strip_comments Remove comments from ist?
      * @param slow_parse Attempt to pull out most verbose errors possible (only use for human written json)
      */
    bool parse(std::istream * const ist, bool strip_comments, bool slow_parse);

    /**
      * @brief Get Extraction Instance
      */
    extract_t extract();

private:
    /**
    * @short Filter Comments from Stream
    * @param ist Stream to Read (must not be null)
    * @param ost Copy of ist with all comments removed (must not be null)
    * A comment is considered: ^[ \t]{0,}#?{0,}$
    * this will not remove comments after text as it would require
    * it to be aware of the structure of the data
    * @warning this will read all of ist and place it all in ost
    */
    static bool filter_comments(std::istream * const ist, std::ostream * const ost);

    /**
     * @brief Json TOP Value
     */
    json_spirit::mValue json_top_;
    /**
      * @brief Attached hid
      */
    hid_t * const hid_;
    /**
      * @brief Were all operations successful?
      */
    bool success_;
};

}

}

#include "json-inc.h"

#endif  // SRC_JSON_H_
