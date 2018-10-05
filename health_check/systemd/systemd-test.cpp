#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

//nate@cisl-prescott:~/git/node-sentinel/build$ busctl call org.freedesktop.systemd1 /org/freedesktop/systemd1 org.freedesktop.systemd1.Manager GetUnitProcesses "s" cups.service
//a(sus) 6 "/system.slice/cups.service" 1028 "/usr/sbin/cupsd -l" "/system.slice/cups.service" 1102 "/usr/lib/cups/notifier/dbus dbus://" "/system.slice/cups.service" 1106 "/usr/lib/cups/notifier/dbus dbus://" "/system.slice/cups.service" 1114 "/usr/lib/cups/notifier/dbus dbus://" "/system.slice/cups.service" 3056 "/usr/lib/cups/notifier/dbus dbus://" "/system.slice/cups.service" 3057 "/usr/lib/cups/notifier/dbus dbus://"
//
///https://github.com/systemd/systemd/blob/8ef3d66d14122d809d0724b4bdfffd61d3d89b40/src/shared/bus-unit-util.c


//http://0pointer.net/blog/the-new-sd-bus-api-of-systemd.html

int main(int argc, char *argv[]) {
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;
        sd_bus *bus = NULL;
        const char *path;
        int r;

        /* Connect to the system bus */
        r = sd_bus_open_system(&bus);
        if (r < 0) {
                fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
                goto finish;
        }

        /* Issue the method call and store the respons message in m */
        r = sd_bus_call_method(bus,
                               "org.freedesktop.systemd1",           /* service to contact */
                               "/org/freedesktop/systemd1",          /* object path */
                               "org.freedesktop.systemd1.Manager",   /* interface name */
                               "StartUnit",                          /* method name */
                               &error,                               /* object to return error in */
                               &m,                                   /* return message on success */
                               "ss",                                 /* input signature */
                               "cups.service",                       /* first argument */
                               "replace");                           /* second argument */
        if (r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                goto finish;
        }

        /* Parse the response message */
        r = sd_bus_message_read(m, "o", &path);
        if (r < 0) {
                fprintf(stderr, "Failed to parse response message: %s\n", strerror(-r));
                goto finish;
        }

        printf("Queued service job as %s.\n", path);

finish:
        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        sd_bus_unref(bus);

        return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


