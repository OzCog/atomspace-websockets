//
// Created by Xabush Semrie on 6/4/20.
//

#ifndef ATOMSPACE_SERVER_WORKER_H
#define ATOMSPACE_SERVER_WORKER_H

#include <thread>
#include <vector>
#include <memory>
#include <string>
#include "uWebSockets/App.h"
#include "manager/AtomSpaceManager.h"
using namespace uWS;

typedef std::shared_ptr<std::thread> ThreadPtr;

struct PerSocketData {
};

class Worker{
private:
    static void findSimilarNames(AtomSpacePtr&, std::string& type, std::string& name,
            std::vector<std::string>& result);
public:

    void work(const AtomSpaceManager& atomManager);
    ThreadPtr _thread;

private:
    std::shared_ptr<App> _app;

};


#endif //ATOMSPACE_SERVER_WORKER_H
