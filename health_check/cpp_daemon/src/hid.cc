#include "conf.h"
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/asio.hpp>
#include "gettext.h"
#include "hid.h"

namespace ibiki
{

void hid_t::global_initialization()
{
#ifndef NDEBUG
    //add check to make sure initialization is only called once
    static bool runonce = false;
    assert(runonce == false);
    runonce = true;
#endif  // NDEBUG

    ///setup syslog
    init_syslog();

    ///setup gnu gettext
    setlocale (LC_ALL, "");
    bindtextdomain ("test", "");
    textdomain ("test");
}

hid_t::format_t::format_t(cstring format)
{
    obj.reset(new boost::format(gettext(format)));
    assert(obj);
}

hid_t::format_t::format_t()
{
    obj.reset();
}

hid_t::format_t& hid_t::format_t::operator%(const hid_t::format_t& other)
{
    assert(other.obj);
    assert(obj);
    assert(other.obj->size() > 0);

    if (other.obj)
        *obj % other.obj->str();
    else
        *obj % "";

    return *this;
}

hid_t::format_t& hid_t::format_t::operator%(const boost::asio::ip::tcp::endpoint& other)
{
    assert(obj);

    boost::system::error_code ec;
    std::string address(other.address().to_string(ec));
    if (!ec)
        *this % (format_t("%1%:%2%") % address % other.port());
    else
        *this % format_t("<Invalid Address>");
    return *this;
}

void hid_t::process_outbound()
{
    message_t message;

    {
        boost::lock_guard<hid_t::mutex_t> lock ( mutex() );
        assert(outbound.size());
        message = outbound.front();
        outbound.pop();
    }

    if (use_syslog)
    {
        syslog(message.priority(), message.message().str());
    }

    if (output_ != NULL)
    {
        *output_ << message.message().str() << std::endl;
    }
}

void hid_t::queue_outbound ( const hid_t::message_t &message )
{
    assert_locked(mutex());
    outbound.push ( message );

    iostrand_.post ( boost::bind ( &hid_t::process_outbound, this  ) );
}

void hid_t::send ( const hid_t::kPriority::Priority priority, const format_t &msg )
{
    boost::lock_guard<mutex_t> lock(mutex());
    queue_outbound ( message_t( priority, msg));
}

void hid_t::send_debug(const format_t &dtype, const format_t &msg)
{
    if (!config_.global().debug) return;

    boost::lock_guard<mutex_t> lock(mutex());
    queue_outbound ( message_t ( kPriority::kDebug, f( "Debug[%1%]: %2%" ) % dtype % msg ) );
}

void hid_t::send_error ( const format_t &etype, const format_t &error, const format_t &detail )
{
    boost::lock_guard<mutex_t> lock(mutex());
    queue_outbound ( message_t ( kPriority::kError, f( "Error[%1%]: %2% Details: %3%" ) % etype % error % detail) );
}

void hid_t::send_error ( const format_t &etype, const format_t &error, const boost::exception &exception)
{
    boost::lock_guard<mutex_t> lock(mutex());
    queue_outbound ( message_t ( kPriority::kError, f( "Error[%1%]: %2% Details: %3%" ) % etype % error % boost::diagnostic_information(exception) ) );
}

void hid_t::send_error ( const format_t &etype, const format_t &error, const boost::system::system_error &system_error)
{
    boost::lock_guard<mutex_t> lock(mutex());

//       boost::system::error_code ec = e.code();
//       network_->hid_->lock_send_error("NETWORK", std::string("Unable to Listen on Socket"), std::string(e.what()) + " Error Category:" + ec.category().name());
//     queue_outbound ( message_t ( kPriority::kError, std::string ( "Error[" ) + etype + std::string ( "]: " ) + error + std::string ( ". Details: " ) + error_code.message() ) );
    queue_outbound ( message_t ( kPriority::kError, f( "Error[%1%]: %2% Details: %3%" ) % etype % error %+ system_error.what() ) );
}

void hid_t::send_warning ( const format_t &wtype, const format_t &warning, const format_t &detail )
{
    boost::lock_guard<mutex_t> lock(mutex());
    queue_outbound ( message_t ( kPriority::kError, f( "Warning[%1%]: %2% Details: %3%" ) % wtype % warning % detail ) );
}

hid_t::hid_t (const configuration_t &config, boost::asio::io_service *io_service )
        : async_threaded_simple<boost::mutex> ( io_service ), use_syslog ( true ), input_ ( NULL ), output_ ( NULL ), outbound(), iostrand_(*io_service), config_(config)
{
}

hid_t::hid_t ( const configuration_t &config, boost::asio::io_service *io_service, std::istream *ist, std::ostream *ost )
        : async_threaded_simple<boost::mutex> ( io_service ), use_syslog ( false ), input_ ( ist ), output_ ( ost ), outbound(), iostrand_(*io_service), config_(config)
{
    assert ( ost );
}

}
