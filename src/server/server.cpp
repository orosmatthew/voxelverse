#include "server.hpp"

#include "../common/assert.hpp"
#include "../common/logger.hpp"

Server::Server()
    : m_exit(false)
    , m_server(nullptr)
{
    init_logger();
    VV_REL_ASSERT(enet_initialize() == 0, "[Server] Failed to initialize ENet");

    constexpr ENetAddress address { .host = ENET_HOST_ANY, .port = c_server_port };
    m_server = enet_host_create(
        &address,
        7, // clients and/or outgoing connections
        2, // max channels to be used
        0, // incoming bandwith
        0); // outoing bandwith
    VV_REL_ASSERT(m_server != nullptr, "[Server] Unable to create ENetHost");
    m_thread = std::thread([this] { this->start(); });
    LOG->info("[Server] Started");
}

Server::~Server()
{
    m_thread.join();
    enet_host_destroy(m_server);
    // TODO: client may still be using enet
    enet_deinitialize();
}

void Server::start() const
{
    ENetEvent event;
    while (!m_exit && enet_host_service(m_server, &event, 1000) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            LOG->info("[Server] Client connected from {}:{}", event.peer->address.host, event.peer->address.port);
            // optionally store info with event
            // event.peer->data = "info here";
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            LOG->info("[Server] Client disconnected from {}:{}", event.peer->address.host, event.peer->address.port);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
        default:
            break;
        }
    }
}