#include <meadow_server.h>

int main()
{
    TcpServer server("127.0.0.1", 50000);

    while (true) {
        server.find_connection(5);
        if (server.is_valid()) {
            std::string query{server.read_serialized_string_connection()};
            std::cout << "PROFILE SERVER: " << query << "\n";
            server.write_serialized_string_connection(query);
            server.close_connection();
        }
    }
}
