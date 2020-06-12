//
// Created by Xabush Semrie on 6/4/20.
//

#include <string_view>
#include <algorithm>
#include <regex>
#include <opencog/atoms/atom_types/NameServer.h>
#include "Worker.h"

static opencog::NameServer &namer = opencog::nameserver();

void Worker::work(const AtomSpaceManager &atomManager) {

    auto endpoints = atomManager.getAtomspaces();
    _app = std::make_shared<App>();

    for (const auto &id: endpoints) {
        _app->ws<PerSocketData>("/" + id, {
                .compression = uWS::SHARED_COMPRESSOR,
                .maxPayloadLength = 16 * 1024 * 1024,
                .idleTimeout = 0,
                .maxBackpressure = 1 * 1024 * 1204,

                .open = [&](auto *ws) {
                    std::cout << "Connected to " << id << std::endl;
                },
                .message = [&](WebSocket<false, true> *ws, std::string_view message, uWS::OpCode opCode) {
                    try{
                        for (const auto &res : atomManager.executePattern(id, message)) {
                            ws->send(res, OpCode::TEXT);
                        }
                        ws->send("eof", OpCode::TEXT);
                    } catch(std::runtime_error &err){
                        std::string_view msg(err.what());
                        std::cerr << msg << std::endl;
                        ws->send(msg, OpCode::TEXT);
                    }

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
                    std::cout << "Connection closed to  " << id << std::endl;
                }
        });
    }

    _app->get("/atomspaces", [&endpoints](HttpResponse<false> *res, HttpRequest *req) {
        json j;
        j["atomspaces"] = endpoints;
        res->writeHeader("Content-Type", "application/json")->end(j.dump());
    });

    _app->get("/atomspaces/:id", [&atomManager](auto *res, auto *req) {
        std::string id(req->getParameter(0));
        auto atomspace = atomManager.getAtomspace(id);
        if (atomspace == nullptr) {
            res->writeStatus("404 NOTFOUND")->end("Atomspace with id " + id + " not found");
        } else {
            json j;
            j["id"] = id;
            j["num_nodes"] = atomspace->get_num_nodes();
            j["num_links"] = atomspace->get_num_links();
            j["total"] = atomspace->get_size();

            res->writeHeader("Content-Type", "application/json")->end(j.dump());
        }
    });

    _app->get("/atomspaces/:id/similar", [&](HttpResponse<false> *res, HttpRequest *req) {
        std::string id(req->getParameter(0));
        auto atomspace = atomManager.getAtomspace(id);
        if (atomspace == nullptr) {
            res->writeStatus("404 NOTFOUND")->end("Atomspace with id " + id + " not found");
        }
        std::string type;
        std::string name;
        size_t pos = 0;
        std::string_view query = req->getQuery();
        if ((pos = query.find("&")) != std::string::npos) {
            auto first_query = query.substr(0, pos);
            auto second_query = query.substr(pos, query.size());

            size_t type_pos = 0;
            if ((type_pos = first_query.find("=")) != std::string::npos) {
                type = first_query.substr(type_pos + 1, first_query.size());
                type_pos = 0;
            }
            if ((type_pos = second_query.find("=")) != std::string::npos) {
                name = second_query.substr(type_pos + 1, second_query.size());
            }

            std::vector<std::string> similar_names;
            findSimilarNames(atomspace, type, name, similar_names);

            json j;
            j["name"] = name,
            j["type"] = type,
            j["similar"] = similar_names;
            res->writeHeader("Content-Type", "application/json")->end(j.dump());
        }
        else {
            std::string mesg = "http://<SERVER_ADDR>:<PORT>/atomspaces/<id>/similar?type=<type_name>&name=<atom_name>";

            res->writeStatus("400 BAD REQUEST")->tryEnd("Please send a proper request. E.g:- " + mesg);
        }
    });

    _app->listen("0.0.0.0", 9001, [](auto *token) {}).run();
}

void Worker::findSimilarNames(AtomSpacePtr& atomspace, std::string &type_name, std::string &name,
                              std::vector<std::string> &result) {

    std::string patt = name + ".+$";
    std::regex similar_regex(patt, std::regex_constants::ECMAScript | std::regex_constants::icase);

    opencog::Type type = namer.getType(type_name);
    if (type == opencog::NOTYPE || namer.isLink(type)) {
        std::cerr << type_name << " should be already defined or be a node type" << std::endl;
        return;
    }
    opencog::HandleSet atoms;
    atomspace->get_handleset_by_type(atoms, type);

    int i = 0; //limit the results
    for(auto& h : atoms){
        if(i > 4) break;
        if(std::regex_search(h->get_name(), similar_regex)){
            result.push_back(h->get_name());
            i++;
        }
    }

    //TODO why isn't this working???
//    atoms.erase(std::remove_if(atoms.begin(), atoms.end(), [&similar_regex](const opencog::Handle& h) {
//        return std::regex_search(h->get_name(), similar_regex);
//    }), atoms.end());
//
//    std::transform(atoms.begin(), atoms.end(), result.begin(), [](const opencog::Handle& h) {
//        return h->get_name();
//    });
}