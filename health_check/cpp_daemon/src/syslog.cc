#include<sstream>
#include<cassert>
#include<syslog.h>
#include <boost/thread.hpp>
#include "hid.h"

namespace ibiki
{

void hid_t::init_syslog()
{
    int logopt = LOG_PID | LOG_NDELAY | LOG_NOWAIT;
    int facility = LOG_USER;
    ///Open syslog but have it use the program name
    ///have syslog opened immediately and be multiprocess safe
    openlog(NULL, logopt, facility);
}

void hid_t::syslog(const hid_t::kPriority::Priority priority, const std::string &message) const
{
    int unix_priority;
    switch (priority)
    {
    case kPriority::kEmergency:
        unix_priority = LOG_EMERG;
        break;
    case kPriority::kAlert:
        unix_priority = LOG_ALERT;
        break;
    case kPriority::kCritical:
        unix_priority = LOG_CRIT;
        break;
    case kPriority::kError:
        unix_priority = LOG_ERR;
        break;
    case kPriority::kWarning:
        unix_priority = LOG_WARNING;
        break;
    case kPriority::kNotice:
        unix_priority = LOG_NOTICE;
        break;
    case kPriority::kInfo:
        unix_priority = LOG_INFO;
        break;
    case kPriority::kDebug:
        unix_priority = LOG_DEBUG;
        break;
    default:
        ///code should never ever get here
        abort();
    }

    if (priority == kPriority::kDebug)
    {
        ///debug messages will have thread attached
        boost::thread::id id = boost::this_thread::get_id();
        std::stringstream sstr;
        sstr << id;
        ::syslog(unix_priority, "thread[%s] %s", sstr.str().c_str(), message.c_str());
    }
    else
        ::syslog(unix_priority, "%s", message.c_str());
}

}
