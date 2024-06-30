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
    /**
 * @brief Конструктор класса TCPServer.
 *
 * Инициализирует новый экземпляр TCPServer с указанным портом.
 *
 * @param port Номер порта, на котором будет прослушиваться сервер.
 */
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

    /**
 * @brief Обрабатывает клиентский сокет и получает сообщение от клиента.
 *
 * Этот метод читает данные из клиентского сокета, создает из них сообщение,
 * и затем вызывает метод logMessage для записи сообщения в журнал.
 *
 * @param client_socket Сокет, через который сервер получает данные от клиента.
 */
void handleClient(int client_socket) {
    char buffer[1024] = {0};  // Буфер для хранения полученных данных от клиента.
    int valread = read(client_socket, buffer, 1024);  // Чтение данных из сокета.

    if (valread > 0) {  // Если данные были прочитаны успешно.
        std::string message(buffer, valread);  // Создание сообщения из полученных данных.
        logMessage(message);  // Запись сообщения в журнал.
    }

    close(client_socket);  // Закрытие клиентского сокета.
}

    /**
 * @brief Записывает сообщение в журнал.
 *
 * Этот метод открывает файл журнала, блокирует mutex для предотвращения конфликта 
 * между потоками, записывает сообщение в конец файла и закрывает файл.
 *
 * @param message Сообщение, которое необходимо записать в журнал.
 */
void logMessage(const std::string &message) {
    std::lock_guard<std::mutex> guard(log_mutex);  // Блокировка mutex'а для предотвращения конфликта между потоками.
    std::ofstream log_file("log.txt", std::ios_base::app);  // Открытие файла журнала в режиме добавления.
    log_file << message << std::endl;  // Запись сообщения в конец файла.
    log_file.close();  // Закрытие файла журнала.
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
