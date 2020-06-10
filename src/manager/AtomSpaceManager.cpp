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

void AtomSpaceManager::loadAtomSpace(const std::string& fname, const std::string& id)
{
    //Check if the id exists
    auto res = _atomspaceMap.find(id);
    if(res != _atomspaceMap.end()){
        throw std::runtime_error("An Atomspace with id " + id + " already exists");
    }

    AtomSpacePtr atomspace = std::make_shared<AtomSpace>();

    load_file(fname, *atomspace);

    _atomspaceMap.insert({id, atomspace});
    _atomIds.push_back(id);

}

void AtomSpaceManager::loadDirectory(const std::string& dirname, const std::string &id)
{
    //Check if the directory exists
    fs::path p(dirname);
    AtomSpacePtr atomspace = std::make_shared<AtomSpace>();
    if(fs::exists(p)){
        for(fs::directory_entry& entry : fs::directory_iterator(p)){
            std::cout << "Parsing " << entry.path().string() << std::endl;
           load_file(entry.path().string(), *atomspace);
           _atomspaceMap.insert({id, atomspace});
           _atomIds.push_back(id);
        }
    } else {
        throw std::runtime_error("No such directory " + dirname);
    }

}


bool AtomSpaceManager::removeAtomSpace(const std::string& id)
{
    auto res = _atomspaceMap.find(id);
    if(res == _atomspaceMap.end()){
        return false;
    }

    _atomspaceMap.erase(id);
    _atomIds.erase(std::remove(_atomIds.begin(), _atomIds.end(), id));
    return true;

}


std::vector<std::string> AtomSpaceManager::executePattern(const std::string& id, std::string_view& pattern) const
{
    auto res = _atomspaceMap.find(id);
    if(res == _atomspaceMap.end()){
        throw std::runtime_error("An Atomspace with id " + id + " not found");
    }

    Handle h;
    AtomSpacePtr atomspace = res->second;
    std::string ss(pattern);
    h = opencog::parseExpression(ss, *atomspace);

    std::vector<std::string> result;

    if (h->is_executable()){

        ValuePtr valPtr = h->execute(atomspace.get());
        if (h->get_type() == BIND_LINK || h->get_type() == GET_LINK){
            auto outgoingSet = std::dynamic_pointer_cast<Atom>(valPtr);

            for(auto& atom : outgoingSet->getOutgoingSet()){
                result.push_back(atom->to_string());
            }

        }
        else if (h->get_type() == QUERY_LINK || h->get_type() == MEET_LINK) {
            auto queueVal = std::dynamic_pointer_cast<QueueValue>(valPtr)->wait_and_take_all();

            while(!queueVal.empty()){
                result.push_back(queueVal.front()->to_string());
                queueVal.pop();
            }
        }
    }
    else { // not a pattern matching query
        atomspace->remove_atom(h, true);
        std::cerr << "Only send pattern matching query to execute patterns. " <<
            ss << " is not a pattern matching query" << std::endl;
    }

    return result;
}

std::vector<std::string> AtomSpaceManager::getAtomspaces() const
{
    return _atomIds;
}

AtomSpacePtr AtomSpaceManager::getAtomspace(const std::string &id) const
{
    auto search =  _atomspaceMap.find(id);
    if(search == _atomspaceMap.end()){
        return nullptr;
    }
    return search->second;
}

void AtomSpaceManager::loadFromSettings(const std::string &fname)
{
    std::ifstream fstream(fname);

    json inputJson;

    if(!fstream.is_open())
        throw std::runtime_error("Cannot find file >>" + fname + "<<");

    fstream >> inputJson;

    for(const auto& j : inputJson) {
        std::cout << "Loading Atomspace " << j["id"] << std::endl;
        if(j.find("scmFile") != j.end()){ //load from scm file
            loadAtomSpace(j["scmFile"], j["id"]);
        } else if(j.find("pathDir") != j.end()){
            std::cout << "Loading Atomspace " << j["id"] << " in dir " << j["pathDir"];
            loadDirectory(j["pathDir"], j["id"]);
        }
        std::cout << "Atomspace " << j["id"] << " Loaded!" << std::endl;
    }

}
