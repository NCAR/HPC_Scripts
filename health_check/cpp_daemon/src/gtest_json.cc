#include<gtest/gtest.h>
#include "json_spirit.h"

/**
 * @brief Loads a simple JSON string and verify
 */
TEST ( json_spirit, load_simple )
{
    const std::string value = "{ \"house_number\" : 42, \"road\" : \"East Street\", \"town\" : \"Newtown\" }";
    json_spirit::Value jsonv;
    {
        const bool ret = json_spirit::read ( value, jsonv );
        ASSERT_TRUE ( ret );
    }
    ASSERT_EQ ( jsonv.type(), json_spirit::obj_type );
    const json_spirit::Object &top = jsonv.get_obj();
    ASSERT_EQ ( top.size(), 3u );
    ASSERT_EQ ( top.at ( 0 ).name_, "house_number" );
    ASSERT_EQ ( top.at ( 0 ).value_, 42 );
    ASSERT_EQ ( top.at ( 1 ).name_, "road" );
    ASSERT_EQ ( top.at ( 1 ).value_, "East Street" );
    ASSERT_EQ ( top.at ( 2 ).name_, "town" );
    ASSERT_EQ ( top.at ( 2 ).value_, "Newtown" );
}

/**
 * @brief Generates and then loads a simple JSON string to verify generated string
 */
TEST ( json_spirit, generate_simple )
{
    json_spirit::Object obj;
    obj.push_back ( json_spirit::Pair ( "house_number", 42 ) );
    obj.push_back ( json_spirit::Pair ( "road", "East \"Street\"" ) );
    obj.push_back ( json_spirit::Pair ( "town", "Newtown" ) );

    //std::string out = json_spirit::write(obj, json_spirit::raw_utf8);
    const std::string output = json_spirit::write_formatted ( obj );
    ASSERT_GT ( output.size(), 0u );

    json_spirit::Value jsonv;
    {
        const bool ret = json_spirit::read ( output, jsonv );
        ASSERT_TRUE ( ret );
    }
    ASSERT_EQ ( jsonv.type(), json_spirit::obj_type );
    const json_spirit::Object &top = jsonv.get_obj();
    ASSERT_EQ ( top.size(), 3u );
    ASSERT_EQ ( top.at ( 0 ).name_, "house_number" );
    ASSERT_EQ ( top.at ( 0 ).value_, 42 );
    ASSERT_EQ ( top.at ( 1 ).name_, "road" );
    ASSERT_EQ ( top.at ( 1 ).value_, "East \"Street\"" );
    ASSERT_EQ ( top.at ( 2 ).name_, "town" );
    ASSERT_EQ ( top.at ( 2 ).value_, "Newtown" );
}

/**
 * @brief Loads a invalid JSON string using both read methods
 */
TEST ( json_spirit, load_invalid )
{
    const std::string value = "{ \"ho\"use_num\"ber\" : 42, \"road\" : \"East Street\", \"town\" : \"Newtown\" }";
    json_spirit::Value jsonv;
    {
        const bool ret = json_spirit::read ( value, jsonv );
        ASSERT_FALSE ( ret );
    }
    {
        try {
            json_spirit::read_or_throw ( value, jsonv );
            //should never reach here
            ASSERT_TRUE ( false );
        } catch ( json_spirit::Error_position json_err ) {
            ASSERT_EQ ( json_err.line_, 1u );
        } catch ( ... ) {
            //should never reach here
            ASSERT_TRUE ( false );
        }
    }
}
