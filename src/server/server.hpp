#pragma once

#include <thread>

#include <enet/enet.h>

constexpr int c_server_port = 27878;

class Server {
public:
    Server();

    Server(const Server& other) = delete;
    Server& operator=(const Server& other) = delete;

    ~Server();

private:
    void start() const;

    bool m_exit;
    std::thread m_thread;
    ENetHost* m_server;
};