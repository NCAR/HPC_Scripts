#include "conf.h"
#include<string>
#include<vector>
#include <boost/asio.hpp>

#ifndef SRC_CONFIGURATION_H_
#define SRC_CONFIGURATION_H_

namespace ibiki
{

/**
 * @short Master Configuration Object
 * Holds/maintains the configuration for the entire application
 * Meant to be shared read-only with basicly everything
 * care must be taken to destroy everything that has
 * access to configuration before destroying it
 * configuration must also not be changed if other objects are
 * referencing it
 */
class configuration_t
{
public:
    configuration_t();

    /**
      * @short Read JSON Configuration file
      * @param ist Configuration File
      * @param hid hid to send msgs/errors
      * @return true on success
      * @warning do not run this if this is referenced by anything else
      */
    bool read_json(std::istream * const ist, hid_t * const hid) throw();


    /**
      * @short Server Configuration
      * holds all the configuration data relating to a single server
      */
    struct server_t
    {
        /**
          *@short Endpoint used for Command Traffic
          */
        boost::asio::ip::tcp::endpoint command_endpoint;
        /**
          *@short Endpoint used for Data Traffic
          */
        boost::asio::ip::tcp::endpoint data_endpoint;
        /**
          *@short Server Hostname (may be kernel name or manual set)
          */
        std::string hostname;
        /**
         * @brief size of net object cache
         */
        u_int64_t netobj_cache;
    };

    struct network_t
    {
        /**
          *@short Network Authorization Key
          *NETWORK_KEY_SIZE has the exact size
          */
        std::string key;

        /**
          * @brief Max number of network connections
          */
        int max_connections;

        /**
          * @brief Cache size for read operations
          */
        int cache_read;

        /**
          * @brief Cache size for write operations
          */
        int cache_write;

        /**
          * @brief debug network ops
          */
        bool debug;
    };

    struct global_t
    {
        /**
          * @brief number of threads to run
          */
        int threads;

        /**
          * @brief enable debug messages
          */
        bool debug;
    };

    /**
      *@short Get List of Servers
      */
    const std::vector<server_t>& servers() const {
        return servers_;
    }

    /**
      *@short Get Network Config
      */
    const network_t& network() const {
        return network_;
    }

    /**
      *@short Get Global Config
      */
    const global_t& global() const {
        return global_;
    }
private:

    /**
      *@short List of Servers
      */
    std::vector<server_t> servers_;
    network_t network_;
    global_t global_;

    DISALLOW_COPY_AND_ASSIGN ( configuration_t );
};

}

#endif  // SRC_CONFIGURATION_H_
