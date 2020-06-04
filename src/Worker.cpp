//
// Created by Xabush Semrie on 6/4/20.
//

#include "Worker.h"

void Worker::work(const AtomSpaceManager &atomManager) {

    auto endpoints = atomManager.getAtomspaces();
    _app = std::make_shared<App>();

    for (const auto &id: endpoints) {
        _app->ws<PerSocketData>("/" + id, {
                .compression = uWS::SHARED_COMPRESSOR,
                .maxPayloadLength = 16 * 1024 * 1024,
                .idleTimeout = 0,
                .maxBackpressure = 1 * 1024 * 1204,

                .open = [&](auto *ws, auto *req) {
                    std::cout << "Connected to " << id << std::endl;
                },
                .message = [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
                    for (const auto &res : atomManager.executePattern(id, message)) {
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
                    std::cout << "Connection closed to  " << id << std::endl;
                }
        });
    }

    _app->listen(9001, [](auto *token){}).run();
}