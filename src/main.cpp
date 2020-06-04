#include <iostream>
#include <uWebSockets/App.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <memory>
#include <vector>
#include <string>
#include <boost/program_options.hpp>

#include "manager/AtomSpaceManager.h"
#include "Worker.h"

using namespace opencog;
using json = nlohmann::json;

namespace po = boost::program_options;


void addServer(const std::string &fname) {

    AtomSpaceManager atomSpaceManager;
    //load the atomspaces from the setting file
    atomSpaceManager.loadFromSettings(fname);


    //create n workers where n is the number of available cores
    std::vector<Worker> workers;
    workers.resize(std::thread::hardware_concurrency());

    std::transform(workers.begin(), workers.end(), workers.begin(), [&atomSpaceManager](Worker &w) {
        w._thread = std::make_shared<std::thread>([&w, &atomSpaceManager]() {
            /* create uWebSocket worker and capture uWS::Loop, uWS::App objects. */
            w.work(atomSpaceManager);
        });
        return w;
    });

    std::for_each(workers.begin(), workers.end(), [](Worker &w) {
        w._thread->join();
    });

}

int main(int argc, char *argv[]) {

    po::options_description desc("Usage");
    desc.add_options()
            ("help", "show help message")
            ("config, C", po::value<std::string>(), "path to the setting json file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 1;
    }

    if (vm.count("config")) {
        addServer(vm["config"].as<std::string>());
        return 0;
    } else {
        std::cout << "Please set path to the setting file. \n" << desc << std::endl;
        return 1;
    }
}
