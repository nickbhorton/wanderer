#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class UdpServer
{
    sockaddr_in address;
    bool valid;
    int fd;

public:
    UdpServer(std::string server_address, int port);
    ~UdpServer();
    UdpServer& operator=(UdpServer const&) = delete;
    UdpServer& operator=(UdpServer&&) = delete;
    UdpServer(UdpServer const&) = delete;
    UdpServer(UdpServer&&) = delete;

    auto send(std::string const& msg, sockaddr_in const& to_address) -> int;
    auto recv() -> std::optional<std::tuple<std::string, sockaddr_in>>;
};
