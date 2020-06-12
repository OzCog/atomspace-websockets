//
// Created by Xabush Semrie on 5/28/20.
//

#include <opencog/atoms/base/Handle.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/QueueValue.h>
#include <opencog/persist/file/fast_load.h>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <fstream>

#include "AtomSpaceManager.h"
#include "Timer.h"

AtomSpacePtr AtomSpaceManager::loadAtomSpace(const std::string &fname, const std::string &id) {
    //Check if the id exists
    auto res = _atomspaceMap.find(id);
    if (res != _atomspaceMap.end()) {
        throw std::runtime_error("An Atomspace with id " + id + " already exists");
    }

    AtomSpacePtr atomspace = std::make_shared<AtomSpace>();

    load_file(fname, *atomspace);

    return atomspace;

}

AtomSpacePtr AtomSpaceManager::loadDirectory(const std::string &dirname, const std::string &id) {
    //Check if the directory exists
    fs::path p(dirname);
    AtomSpacePtr atomspace = std::make_shared<AtomSpace>();
    if (fs::exists(p)) {
        for (fs::directory_entry &entry : fs::recursive_directory_iterator(p)) {
            if(entry.path().extension() == ".scm") {
                std::cout << "Parsing " << entry.path().string() << std::endl;
                load_file(entry.path().string(), *atomspace);
            }
        }
    } else {
        throw std::runtime_error("No such directory " + dirname);
    }

    return atomspace;
}


bool AtomSpaceManager::removeAtomSpace(const std::string &id) {
    auto res = _atomspaceMap.find(id);
    if (res == _atomspaceMap.end()) {
        return false;
    }

    _atomspaceMap.erase(id);
    _atomIds.erase(std::remove(_atomIds.begin(), _atomIds.end(), id));
    return true;

}


std::vector<std::string> AtomSpaceManager::executePattern(const std::string &id, std::string_view &pattern) const {
    auto res = _atomspaceMap.find(id);
    if (res == _atomspaceMap.end()) {
        throw std::runtime_error("An Atomspace with id " + id + " not found");
    }

    Handle h;
    AtomSpacePtr atomspace = res->second;
    std::string ss(pattern);

    try {
        h = opencog::parseExpression(ss, *atomspace);
    } catch(std::runtime_error &err) {
        throw err;
    }

    if(h == nullptr){
        throw std::runtime_error("Invalid Pattern Matcher query: " + ss);
    }

    std::vector<std::string> result;

    if (h->is_executable()) {

        ValuePtr valPtr = h->execute(atomspace.get());
        if (h->get_type() == BIND_LINK || h->get_type() == GET_LINK) {
            auto outgoingSet = std::dynamic_pointer_cast<Atom>(valPtr);

            for (auto &atom : outgoingSet->getOutgoingSet()) {
                result.push_back(atom->to_string());
            }

        } else if (h->get_type() == QUERY_LINK || h->get_type() == MEET_LINK) {
            auto queueVal = std::dynamic_pointer_cast<QueueValue>(valPtr)->wait_and_take_all();

            while (!queueVal.empty()) {
                result.push_back(queueVal.front()->to_string());
                queueVal.pop();
            }
        }
    } else { // not a pattern matching query
        atomspace->remove_atom(h, true);
        std::cerr << "Only send pattern matching query to execute patterns. " <<
                  ss << " is not a pattern matching query" << std::endl;
    }

    return result;
}

std::vector<std::string> AtomSpaceManager::getAtomspaces() const {
    return _atomIds;
}

AtomSpacePtr AtomSpaceManager::getAtomspace(const std::string &id) const {
    auto search = _atomspaceMap.find(id);
    if (search == _atomspaceMap.end()) {
        return nullptr;
    }
    return search->second;
}

void AtomSpaceManager::loadFromSettings(const std::string &fname) {
    std::ifstream fstream(fname);

    json inputJson;

    if (!fstream.is_open())
        throw std::runtime_error("Cannot find file >>" + fname + "<<");

    fstream >> inputJson;

    AtomSpacePtr atomspace;
    for (const auto &j : inputJson) {
        std::cout << "Loading Atomspace " << j["id"] << std::endl;
        if (j.find("scmFile") != j.end()) { //load from scm file
            atomspace = loadAtomSpace(j["scmFile"], j["id"]);
        } else if (j.find("pathDir") != j.end()) {
            atomspace = loadDirectory(j["pathDir"], j["id"]);
        }

        _atomspaceMap.insert({j["id"], atomspace});
        _atomIds.push_back(j["id"]);
        std::cout << "Atomspace " << j["id"] << " Loaded!" << std::endl;
    }

}
