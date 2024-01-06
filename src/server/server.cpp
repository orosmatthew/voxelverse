#include "server.hpp"

#include <sstream>

#include "../common/assert.hpp"
#include "../common/logger.hpp"

Server::Server(const bool cleanup_enet)
    : m_cleanup_enet(cleanup_enet)
    , m_exit(false)
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
    m_exit = true;
    m_thread.join();
    enet_host_destroy(m_server);
    if (m_cleanup_enet) {
        enet_deinitialize();
    }
}

static std::string host_ip_to_string(uint32_t host)
{
    uint32_t ip[4];
    ip[0] = host & 0xff;
    host >>= 8;
    ip[1] = host & 0xff;
    host >>= 8;
    ip[2] = host & 0xff;
    host >>= 8;
    ip[3] = host & 0xff;
    std::stringstream ss;
    ss << static_cast<int>(ip[0]) << "." << static_cast<int>(ip[1]) << "." << static_cast<int>(ip[2]) << "."
       << static_cast<int>(ip[3]);
    return ss.str();
}

static void send_hello_packet(ENetPeer* peer)
{
    const std::string msg = "Hello World!";
    ENetPacket* packet = enet_packet_create(msg.data(), msg.length(), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void Server::start() const
{
    ENetEvent event;
    while (!m_exit) {
        while (enet_host_service(m_server, &event, 1000) > 0) {
            switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                LOG->info(
                    "[Server] Client connected from {}:{}",
                    host_ip_to_string(event.peer->address.host),
                    event.peer->address.port);
                // optionally store info with event
                // event.peer->data = "info here";
                send_hello_packet(event.peer);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                LOG->info(
                    "[Server] Client disconnected from {}:{}",
                    host_ip_to_string(event.peer->address.host),
                    event.peer->address.port);
                break;
            case ENET_EVENT_TYPE_RECEIVE:
            default:
                break;
            }
        }
    }
    LOG->info("[Server] Stopping");
}