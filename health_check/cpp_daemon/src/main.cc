#include "conf.h"
#include "hid.h"
#include "configuration.h"

int main ( int argc, char **argv )
{
    ibiki::hid_t::global_initialization();

    //  std::cout << "Project Ibiki: Version " << IBIKI_VERSION_MAJOR << '.' << IBIKI_VERSION_MINOR << std::endl; // NOLINT


    return 0;
}
