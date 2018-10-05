#include "conf.h"
#include <cassert>
#include <sstream>
//#include <typeinfo>
//#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include "configuration.h"
#include "json.h"
#include "servers.h"

namespace ibiki
{

configuration_t::configuration_t()
        : servers_()
{
  network_.cache_read = NETWORK_CACHE_READ_DEFAULT;
  network_.cache_write = NETWORK_CACHE_WRITE_DEFAULT;
  network_.max_connections = MAX_NETWORK_CONNECTIONS_DEFAULT;
  network_.debug = false;
  global_.debug = true;
  global_.threads = GLOBAL_THREADS_COUNT_DEFAULT;
}

bool configuration_t::read_json(std::istream * const ist, hid_t * const hid) throw()
{
    assert(ist && *ist);
    assert(hid);

    json::parser_t parser = json::parser_t(hid);
    if (!parser.parse(ist, true, true)) return false;

    if (!parser.extract().c(
                "global").f(
                "threads", &global_.threads, GLOBAL_THREADS_COUNT_DEFAULT).f(
                "debug", &global_.debug, false)
       ) return false;
    if(global_.threads < 1)
    {
        hid->send_error(
            "DATA",
            "Invalid Thread Count",
            "There must be atleast one thread to run."
        );
        return false;
    }

    if (!parser.extract().c(
                "network").f(
                "key", &network_.key).f(
                "max connections", &network_.max_connections, MAX_NETWORK_CONNECTIONS_DEFAULT).f(
                "debug", &network_.debug, false).f(
                "read cache", &network_.cache_read, NETWORK_CACHE_READ_DEFAULT).f(
                "write cache", &network_.cache_write, NETWORK_CACHE_WRITE_DEFAULT )
       ) return false;
    if (network_.max_connections < 3)
    {
        hid->send_error(
            "DATA",
            "Invalid Max Connections",
            "Max Connections must be at least 2. Atleast several hundred is suggested."
        );
        return false;
    }
    if (network_.key.length() != NETWORK_KEY_SIZE)
    {
        hid->send_error(
            "DATA",
            "Invalid Network Key",
            hid->f( "Network key must be exactly %1% characters long. Given network key \"%2%\" is %3%") %
            NETWORK_KEY_SIZE %
            network_.key %
            network_.key.length()
        );
        return false;
    }
    if(network_.cache_read < 2 || network_.cache_write < 2)
    {
      hid->send_error(
            "DATA",
            "Invalid Network Cache Size",
            "Network cache size must be atleast 1"
        );
        return false;
    }
    ///convert cache size from mb to bytes
    network_.cache_read *= 1048576u;
    network_.cache_write *= 1048576u;

    json::parser_t::extract_t::extract_array_obj_t extract_servers;
    if (!parser.extract().ao("servers", &extract_servers)) return false;
    foreach(json::parser_t::extract_t &extract, extract_servers) // NOLINT
    {
        server_t s;
        int32_t command_port;
        int32_t data_port;
        std::string string_ip;

        if (!extract.f(
          "hostname", &s.hostname).f(
          "ip", &string_ip).f(
          "command-port", &command_port).f(
          "data-port", &data_port).f(
          "netobj cache", &s.netobj_cache)
          )
          return false;
        if (data_port < 1 || data_port > 65535 || command_port < 1 || command_port > 65535)
        {
            hid->send_error("DATA", "Invalid Port Number",  hid->f("Invalid port specified for %1%. Must be below 65535 and above 0.") % extract.reference());
            return false;
        }
        if(s.netobj_cache <= 0)
        {
            hid->send_error("DATA", "Invalid Netobject Cache Size",  hid->f("Server: %1%. Net object cache size must be above 0.") % extract.reference());
            return false;
        }
        ///convert cache size from mb to bytes
        s.netobj_cache *= 1048576u;
        ///setup endpoints
        boost::system::error_code ec;
        boost::asio::ip::address ip = boost::asio::ip::address::from_string(string_ip, ec);
        if(ec)
        {
            hid->send_error("DATA", hid->f("Invalid Server IP %1%") % string_ip,  ec);
            return false;
        }
        s.data_endpoint.address(ip);
        s.command_endpoint.address(ip);
        s.data_endpoint.port(data_port);
        s.command_endpoint.port(command_port);

        servers_.push_back(s);
    }

    return true;
}

}
