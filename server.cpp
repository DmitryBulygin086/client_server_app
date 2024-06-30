#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <chrono>
#include <ctime>
#include <arpa/inet.h>
#include <unistd.h>

std::mutex log_mutex;

class TCPServer {
public:
    TCPServer(int port) : port(port) {}

    void start() {
        int server_fd; // Сокет для сервера
        int new_socket; // Сокет для новых подключений
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        std::cout << "Сервер прослушивает порт " << port << std::endl;

        while (true) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

             // Создание нового потока для нового подключения.
            // 
            // Отделение нового потока от основного потока 
            // с помощью метода .detach(). Это позволяет серверу
            // продолжать обслуживать новые подключения параллельно.
            std::thread(&TCPServer::handleClient, this, new_socket).detach();
        }
    }

private:
    int port;

    void handleClient(int client_socket) {
        char buffer[1024] = {0};
        int valread = read(client_socket, buffer, 1024);

        if (valread > 0) {
            std::string message(buffer, valread);
            logMessage(message);
        }

        close(client_socket);
    }

    void logMessage(const std::string &message) {
        std::lock_guard<std::mutex> guard(log_mutex);
        std::ofstream log_file("log.txt", std::ios_base::app);
        log_file << message << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <порт>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    TCPServer server(port);
    server.start();

    return 0;
}
