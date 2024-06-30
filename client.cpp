#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>

class TCPClient {
public:
    /**
 * @brief Конструктор для класса TCPClient.
 *
 * @param name Имя клиента.
 * @param server_ip IP адрес сервера.
 * @param server_port Номер порта серврера.
 * @param interval Промежуток времени в секундах между отправкой сообшений на сервер.
 *
 * @note Предполагается что клиент запушен на том же устройстве, что и сервер.
 * @note IP адрес сервера настроен на "127.0.01" по умолчанию.
 */
TCPClient(const std::string &name, const std::string &server_ip, int server_port, int interval)
        : client_name(name), server_ip(server_ip), server_port(server_port), interval(interval) {}

    void start() {
        while (true) {
            sendMessage();
            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }

private:
    std::string client_name;
    std::string server_ip;
    int server_port;
    int interval;

    /**
 * @brief Функция для получения текущего времени в формате "YYYY-MM-DD HH:MM:SS.sss".
 *
 * @return Строка, содержащая текущее время в указанном формате.
 *
 * @note Эта функция использует библиотеку chrono для получения текущего времени и библиотеку iomanip для форматирования миллисекунд.
 * @note Время возвращается в локальной часовой зоне.
 */
std::string getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X") << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return ss.str();
}

    void sendMessage() {
        int sock = 0;
        struct sockaddr_in serv_addr;
        char buffer[1024] = {0};

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Ошибка при создании сокета." << std::endl;
            return;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(server_port);

        if (inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr) <= 0) {
            std::cerr << "Недействительный адрес/ Адрес не поддерживается" << std::endl;
            return;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cerr << "Ошибка при подключении" << std::endl;
            return;
        }

        std::string message = "[" + getCurrentTime() + "] \"" + client_name + "\"";
        send(sock, message.c_str(), message.length(), 0);
        close(sock);
    }
};

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        std::cerr << "Использование: " << argv[0] << " <client_name> <server_ip> <server_port> <interval>" << std::endl;
        return 1;
    }

    std::string client_name = argv[1];
    std::string server_ip = "127.0.0.1"; // Сервер запушен на том же устройстве, что и клиент
    int server_port = std::stoi(argv[2]);
    int interval = std::stoi(argv[3]);

    TCPClient client(client_name, server_ip, server_port, interval);
    client.start();

    return 0;
}
