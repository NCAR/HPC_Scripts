#include<gtest/gtest.h>

#include "hid.h"
#include <sstream>
/**
 * @brief Test hid send() using a stringstream
 */
TEST ( hid, send )
{
    boost::asio::io_service io_service;
    cstring buf = "test test por la test";
    std::stringstream in, out;
    ibiki::configuration_t conf;
    ibiki::hid_t hid(conf, &io_service, &in, &out);

    hid.send(ibiki::hid_t::kPriority::kInfo, buf);

    io_service.run();

    ASSERT_EQ(out.str(), std::string(buf) + "\n");
}
