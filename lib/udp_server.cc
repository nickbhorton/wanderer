#include "udp_server.h"

UdpServer::UdpServer(std::string server_address, int port)
    : address{}, valid{false}
{
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "socket() failed UdpServer\n";
        return;
    }
    // Filling server information
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY;
    inet_pton(address.sin_family, server_address.c_str(), &address.sin_addr);
    address.sin_port = htons(port);

    if (bind(fd, (const struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "bind() failed UdpServer\n";
        return;
    }
    valid = true;
}

UdpServer::~UdpServer() { close(fd); }

auto UdpServer::send(std::string const& msg, sockaddr_in const& to_address)
    -> int
{
    if (!valid) {
        return -1;
    }
    ssize_t sent_count = sendto(
        fd,
        msg.data(),
        msg.size(),
        MSG_CONFIRM,
        (const struct sockaddr*)&to_address,
        sizeof(to_address)
    );
    return sent_count;
}
auto UdpServer::recv() -> std::optional<std::tuple<std::string, sockaddr_in>>
{
    if (!valid) {
        return {};
    }
    sockaddr_in client_address{};
    socklen_t client_address_length =
        sizeof(client_address); // len is value/result

    size_t constexpr max_udp_packet_size{65536};
    char buffer[max_udp_packet_size];
    int n = recvfrom(
        fd,
        (char*)buffer,
        max_udp_packet_size,
        MSG_WAITALL,
        (struct sockaddr*)&client_address,
        &client_address_length
    );
    std::string payload(buffer, n);
    return {{payload, client_address}};
}
