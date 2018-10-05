#include "conf.h"
#include <iostream>
#include <queue>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include "threaded.h"
#include "configuration.h"

#ifndef SRC_HID_H_
#define SRC_HID_H_

namespace ibiki
{

/**
  * @short Human Interface Device
  * Provides a generic way to interact with user
  * as the user may interact from anything such
  * as console or some remote tool or even some
  * http server.
  * Currently only one way communication will
  * be implemented but two way may happen.
  *
  * this should make sending user messages transparent
  * for functions no matter how they connect
  *
  * All communication is based on discrete messages
  * as we assume multiple threads will be interacting
  * with a hid at one time.
  *
  * Messages from the user should alway go through the
  * format_t objects as they will handle i18n and l10n
  * The format_t objects can only be made from cstrings
  * in an attempt to insure that generated data is not
  * handed to format_t since it won't be very useful.
  * Take care to use format_t correct to allow translation
  * and interface with non console systems
  */
class hid_t: private async_threaded_simple<boost::mutex>
{
public:
    class format_t;
    /**
      * @short Syslog Severity Level
      * @see http://pubs.opengroup.org/onlinepubs/000095399/functions/syslog.html
      */
    class kPriority
    {
    public:
        enum Priority {
            kEmergency,
            kAlert,
            kCritical,
            kError,
            kWarning,
            kNotice,
            kInfo,
            kDebug
        };
    };

    /**
      * @brief Format String
      * @param format_string String to use as Format
      * @return format instance to use with values
      * @see boost::format
      */
    format_t format(cstring format_string)
    {
        return format_t(format_string);
    }
    /// @see format
    format_t f(cstring format_string)
    {
        return format(format_string);
    }

    /**
      * @brief Format class to extend boost::format
      * Enforces the a way to do i18n & l10n (eventually)
      * @see boost::format
      * @warning not thread safe
      */
    class format_t
    {
    public:

        /**
          * @brief operator %
          */
        template<class T>
        format_t& operator%(const T& other)
        {
            assert(obj);
            *obj % other;
            return *this;
        }
        format_t& operator%(const format_t& other);
        format_t& operator%(const boost::asio::ip::tcp::endpoint& other);

        /**
          * @brief ctor
          * @param format format string
          * we want implicit conversion for cstrings
          * this will allow functions to demand format_t
          * objects with out making the calling function
          * to have create a format_t instance for normal
          * cstrings
          */
        format_t(cstring format); // NOLINT

        /**
         * @brief ctor (empty)
         */
        explicit format_t();

    private:
        friend class hid_t;

        /**
        * @brief Convert to a string
        * Only hid_t should be using the final string
        */
        std::string str() const
        {
            assert(obj);
            return obj ? obj->str() : "";
        }

        /**
          * @brief Boost Format Instance
          */
        boost::shared_ptr<boost::format> obj;
    };


    /**
     * @brief Global Initialization
     * this is a special function to be only called once from main()
     * it initializes all of the static interfaces and configures
     * the hid system
     */
    static void global_initialization();

    /**
      * @brief Send Message async
      * @param priority Priority (or type) of Message
      * @param msg Message to send to user
      * @warning locks mutex()
      */
    void send(const kPriority::Priority priority, const format_t &msg);

    /**
      * @brief Send Debug Message async
      * @param dtype Debug Message Type
      * @param msg Message
      * @warning locks mutex()
      */
    void send_debug(const format_t &dtype, const format_t &msg);

    /**
      * @brief Send Error Message async
      * @param etype Error Type (User, System, Network, etc)
      * @param error Actual Error
      * @param detail Detailed Error message with help if possible for fixing
      * @warning locks mutex()
      */
    void send_error(const format_t &etype, const format_t &error, const format_t &detail);

    /**
     * @brief Send Error Message async
     * @param etype Error Type (User, System, Network, etc)
     * @param error Actual Error
     * @param exception Boost Exception
     * @warning locks mutex()
     */
    void send_error(const format_t &etype, const format_t &error, const boost::exception &exception);

    /**
     * @brief Send Error Message async
     * @param etype Error Type (User, System, Network, etc)
     * @param error Actual Error
     * @param system_error Boost System Error
     * @warning locks mutex()
     */
    void send_error(const format_t &etype, const format_t &error, const boost::system::system_error &system_error);

    /**
      * @brief Send Warning Message async
      * @param wtype Warning Type (User, System, Network, etc)
      * @param warning Actual Warning
      * @param detail Detailed Warning message with help if possible for fixing
      * @warning locks mutex()
      */
    void send_warning(const format_t &wtype, const format_t &warning, const format_t &detail);

    /**
     * @brief ctor Hid uses Syslog for messages
     */
    explicit hid_t(const configuration_t &config, boost::asio::io_service *io_service);

    /**
     * @brief ctor Hid uses I/O Streams
     * @warning streams must exist for duration of instance
     */
    explicit hid_t(const configuration_t &config, boost::asio::io_service *io_service, std::istream *ist, std::ostream *ost);

private:
    /**
      * @brief Send Messages to Syslog?
      * @warning Do not change these after initialization
      */
    const bool use_syslog;
    /**
      * @brief Input iostream (may be null)
      */
    std::istream * const input_;
    /**
      * @brief Ouput iostream (may be null)
      */
    std::ostream * const output_;

    /**
      * @brief Message Container
      * Forces readonly gets
      */
    class message_t
    {
    public:
        const kPriority::Priority &priority() const {
            return priority_;
        }
        const format_t &message() const {
            return message_;
        }

        message_t()
                : priority_(kPriority::kAlert), message_() {}

        message_t(const kPriority::Priority priority, const format_t &message)
                : priority_(priority), message_(message) {}

        message_t(const message_t &other)
                : priority_(other.priority_), message_(other.message_) {}

        message_t& operator=(const message_t &other)
        {
            priority_ = other.priority_;
            message_ = other.message_;
            return *this;
        }

    private:
        kPriority::Priority priority_;
        format_t message_;
    };

    /**
      * @brief list of messages to send output
      */
    std::queue<message_t> outbound;

    /**
      * @brief IO Strand used to insure that communications are not concurrent
      */
    boost::asio::io_service::strand iostrand_;

     /**
      * @brief Global Configutation
      */
    const configuration_t &config_;

    /**
      * @short Adds message to Outbound and schedules sending
      * @warning write lock mutex() before calling
      */
    void queue_outbound(const message_t &message);

    /**
     * @brief Process outbound callback
     */
    void process_outbound();

    /**
      * @short Send Syslog Message
      * @param priority Severity Level
      * @param message Message to send to syslog
      * safe to call in any thread at any time!
      */
    void syslog(const kPriority::Priority priority, const std::string &message) const;

    /**
      * @brief initialize syslog
      * only call once from global_initialization
      */
    static void init_syslog();

    DISALLOW_COPY_AND_ASSIGN ( hid_t );
};

}

#endif  // SRC_HID_H_
