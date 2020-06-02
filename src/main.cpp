#include <iostream>
#include <uWebSockets/App.h>
#include <nlohmann/json.hpp>
#include <thread>
#include <memory>
#include <vector>
#include <string>
#include <boost/program_options.hpp>

#include "manager/AtomSpaceManager.h"

using namespace opencog;
using namespace uWS;
using json = nlohmann::json;

namespace po = boost::program_options;

void addServer(const std::string& fname) {
    AtomSpaceManager atomSpaceManager;

    atomSpaceManager.loadFromSettings(fname);

    auto atomspaces = atomSpaceManager.getAtomspaces();

    struct PerSocketData {

    };
    App server;
    //create n threads where n is the number of available cores
    typedef std::shared_ptr<std::thread> ThreadPtr;
    std::vector<ThreadPtr> threads(std::thread::hardware_concurrency());

    std::transform(threads.begin(), threads.end(), threads.begin(), [&](const ThreadPtr &t) {
        return std::make_shared<std::thread>([&]() {
            for (auto id : atomspaces) {
                App().ws<PerSocketData>("/" + id, {
                        /* Settings */
                        .compression = uWS::SHARED_COMPRESSOR,
                        .maxPayloadLength = 16 * 1024,
                        .maxBackpressure = 1 * 1024 * 1204,
                        .idleTimeout = 10,
                        /* Handlers */
                        .open = [&](auto *ws, auto *req) {
                            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
                            std::cout << "Connected to Atomspace " << id << std::endl;
                        },
                        .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
                            for(const auto& res : atomSpaceManager.executePattern(id, message)) {
                                ws->send(res, OpCode::TEXT);
                            }
                            ws->send("eof", OpCode::TEXT);

                        },
                        .drain = [](auto *ws) {
                            /* Check ws->getBufferedAmount() here */
                        },
                        .ping = [](auto *ws) {
                            /* Not implemented yet */
                        },
                        .pong = [](auto *ws) {
                            /* Not implemented yet */
                        },
                        .close = [&](auto *ws, int code, std::string_view message) {
                            std::cout << "Connection closed to Atomspace " << id << std::endl;
                            /* You may access ws->getUserData() here */
                        }
                }).listen(9001, [](auto *listenSocket) {
                    if (listenSocket) {
                        std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << 9001
                                  << std::endl;
                    }  else {
                        std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 9001" << std::endl;
                    }
                }).run();
            }
        });
    });
    std::for_each(threads.begin(), threads.end(), [](const ThreadPtr &t) {
        t->join();
    });
}

int main(int argc, char* argv[]) {

    po::options_description desc("Usage");
    desc.add_options()
            ("help", "show help message")
            ("config, C", po::value<std::string>(), "path to the setting json file");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")){
        std::cout << desc << std::endl;
        return 1;
    }

    if(vm.count("config")){
        addServer(vm["config"].as<std::string>());
        return 0;
    } else {
        std::cout << "Please set path to the setting file. \n" << desc << std::endl;
        return 1;
    }
}
