#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <getopt.h>
#include "spdlog/spdlog.h"
#include "version.h"
#include "circuit.h"
#include "easygl/graphics.h"
#include "ui.h"
#include <thread>

using namespace std;

void print_usage() {
    cout << "Usage: ./a1 [-hv] [-dis] -f circuit_file [-p postscript_file]" << endl;
    cout << "\t-h: this help message" <<endl;
    cout << "\t-v: print version info" <<endl;
    cout << "\t-f circuit_file: the circuit file (required)" <<endl;
    cout << "\t-d: turn on debug log level" <<endl;
    cout << "\t-i: enable interactive (gui) mode" <<endl;
    cout << "\t-s: step through routing" <<endl;
    cout << "\t-p postscript_file: output image to ps file" <<endl;
}

void print_version() {
    spdlog::info("a1 - Troy Denton 2023");
    spdlog::info("Version {}.{}", VERSION_MAJOR, VERSION_MINOR);
    spdlog::info("Commit {}", GIT_COMMIT);
    spdlog::info("Built {}" , __TIMESTAMP__);
}

void route_thread(circuit* circ, bool interactive) {
    bool result = circ->route(interactive);
    if (result)
        spdlog::info("Successfully routed design");
    else
        spdlog::error("Could not route design");
}

int main(int n, char** args) {
    string file = "";
    string ps_file = "";

    bool interactive = false;
    bool step = false;

    for(;;)
    {
        switch(getopt(n, args, "vhf:disp:"))
        {
            case 'f':
                file = optarg;
                continue;

            case 'd':
                spdlog::set_level(spdlog::level::debug);
                continue;

            case 'v':
                print_version();
                return 0;

            case 'p':
                ps_file = optarg;
                continue;

            case 's':
                step = true;
                continue;


            case 'i':
                interactive = true;
                continue;
            case '?':
            case 'h':
            default :
                print_usage();
                return 1;

            case -1:
                break;
        }
        break;
    }

    if (file == "") {
        cerr << "Error: must provide input file" << endl;
        print_usage();
        return 1;
    }

    print_version();

    circuit* circ = new circuit(file);
    thread t1(route_thread, circ, step);

    if (!interactive)
        t1.join();

    if (interactive) {
        spdlog::info("Entering interactive mode");
        ui_init(circ);
        ui_teardown();
        t1.join();
    }
    if (ps_file != "") {
        ps_output(circ, ps_file);
    }
    spdlog::info("Total segments used: {}", circ->total_segments());

    spdlog::info("Exiting");
    delete(circ);
    return 0;
}
