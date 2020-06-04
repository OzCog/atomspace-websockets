//
// Created by Xabush Semrie on 5/28/20.
//

#include <opencog/atoms/base/Handle.h>
#include <opencog/atoms/base/Node.h>
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
    try {
        std::string ss(pattern);
        h = opencog::parseExpression(ss, *atomspace);
    } catch (std::runtime_error& err) {
        throw err;
    }
    std::cout << "AtomSpace Count - Nodes:  " << atomspace->get_num_nodes() << " Links: " << atomspace->get_num_links() <<
    std::endl;
    ValuePtr valPtr = h->execute(atomspace.get());

    auto queueVal = std::dynamic_pointer_cast<Atom>(valPtr);
    std::vector<std::string> result;

    for(auto& r: queueVal->getOutgoingSet()){
        result.push_back(r->to_string());
    }
    return result;
}

std::vector<std::string> AtomSpaceManager::getAtomspaces() const
{
    return _atomIds;
}

AtomSpacePtr AtomSpaceManager::getAtomspace(const std::string &id)
{
    auto search =  _atomspaceMap.find(id);
    if(search == _atomspaceMap.end()){
        throw std::runtime_error("Atomspace with id " + id + " not found");
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
