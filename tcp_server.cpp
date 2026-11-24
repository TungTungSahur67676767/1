// TCP-сервер со слайдов 45–46. Я сохраняю ручной диалог, а не автоматическое эхо.
#include <winsock2.h> // Я подключаю сокеты Windows.
#include <iostream> // Я подключаю ввод и вывод.
#include <string> // Я подключаю строки C++.
#pragma comment(lib, "Ws2_32.lib") // Я подключаю библиотеку Winsock при сборке.
using namespace std; // Я пишу cout и string без std::.
#define SRV_PORT 1234 // Я оставляю серверный порт из лекции.
#define BUF_SIZE 64 // Я отвожу 64 байта: до 63 байт текста и завершающий ноль.
const string QUEST = "Who are you?\n"; // Я первым задаю вопрос, как в лекции.

int main() { // Я начинаю программу сервера.
    WSADATA ws{}; // Я выделяю структуру для Winsock.
    int err = WSAStartup(0x0202, &ws); // Я запускаю Winsock версии 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // Я проверяю запуск.
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // Я создаю IPv4 TCP-сокет для ожидания клиентов.
    if (s == INVALID_SOCKET) { // Я проверяю создание сокета.
        cout << "socket: " << WSAGetLastError() << endl; // Я показываю код ошибки.
        WSACleanup(); return 1; // Я освобождаю библиотеку и выхожу.
    } // Я закончил проверку.
    sockaddr_in sin{}, from_sin{}; // Я обнуляю адрес сервера и место для адреса клиента.
    sin.sin_family = AF_INET; // Я использую IPv4.
    sin.sin_addr.s_addr = 0; // Я принимаю подключения на всех локальных IPv4-адресах.
    sin.sin_port = htons(SRV_PORT); // Я записываю порт 1234 в сетевом порядке байт.
    if (bind(s, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) { // Я закрепляю адрес и порт за сокетом.
        cout << "bind: " << WSAGetLastError() << endl; // Я показываю ошибку привязки.
        closesocket(s); WSACleanup(); return 1; // Я освобождаю ресурсы.
    } // Я закончил привязку.
    string msg, msg1; // Я храню своё сообщение и ответ клиента, как в лекции.
    if (listen(s, 3) == SOCKET_ERROR) { // Я включаю ожидание; 3 относится к очереди подключений.
        cout << "listen: " << WSAGetLastError() << endl; // Я показываю ошибку ожидания.
        closesocket(s); WSACleanup(); return 1; // Я освобождаю ресурсы.
    } // Я готов принимать подключения.
    cout << "TCP server: port " << SRV_PORT << ". Start the client.\n" << flush; // Я показываю готовность.
    bool running = true; // Я смогу выйти из сервера при завершении консольного ввода.
    while (running) { // Я обслуживаю клиентов по одному.
        int from_len = sizeof(from_sin); // Я передаю размер структуры адреса клиента.
        SOCKET s_new = accept(s, (sockaddr*)&from_sin, &from_len); // Я получаю отдельный сокет клиента.
        if (s_new == INVALID_SOCKET) { cout << "accept: " << WSAGetLastError() << endl; err = 1; break; } // Я проверяю приём.
        cout << "new connected client!" << endl; // Я сообщаю о подключении.
        char buf[BUF_SIZE]{}; // Я готовлю буфер ответа клиента.
        msg = QUEST; // Я начинаю каждый диалог с Who are you?
        while (1) { // Я чередую отправку вопроса и получение ответа.
            int msg_size = (int)msg.size() + 1, sent = 0, n = 0; // Я включаю ноль как границу строки.
            while (sent < msg_size) { // Я досылаю сообщение, если send отправил не всё.
                n = send(s_new, msg.c_str() + sent, msg_size - sent, 0); // Я отправляю оставшиеся байты.
                if (n <= 0) break; // Я останавливаюсь при ошибке или отсутствии прогресса.
                sent += n; // Я считаю реально отправленные байты.
            } // Я закончил отправку.
            if (sent != msg_size) { cout << "send failed\n"; break; } // Я прерываю неудачный диалог.
            int used = 0; // Я начинаю заполнять буфер с первого байта.
            do { // Я собираю строку из TCP-потока до нулевого байта.
                from_len = recv(s_new, buf + used, 1, 0); // Я принимаю байт без предположения о границах пакетов.
                if (from_len <= 0) break; // Я замечаю ошибку или закрытие соединения.
                ++used; // Я учитываю принятый байт.
            } while (buf[used - 1] != '\0' && used < BUF_SIZE); // Я не выхожу за пределы буфера.
            if (from_len <= 0) { // Я проверяю результат приёма.
                cout << "recv: " << (from_len == 0 ? 0 : WSAGetLastError()) << " (0 = closed)\n"; // Я показываю причину.
                break; // Я прекращаю общение с этим клиентом.
            } // Я закончил проверку приёма.
            if (buf[used - 1] != '\0') { cout << "Message too long\n"; break; } // Я не читаю за буфером.
            msg1 = buf; // Я превращаю полученную C-строку в string, как в лекции.
            cout << "C=>S: " << msg1 << endl; // Я вывожу ответ клиента.
            if (msg1 == "Bye") break; // Я завершаю диалог по команде клиента.
            do { // Я ввожу следующий ответ сервера вручную.
                cout << "S=>C: "; // Я показываю, что теперь очередь сервера.
                if (!getline(cin, msg)) { running = false; break; } // Я завершаю сервер при конце ввода.
                if (msg.size() >= BUF_SIZE) cout << "Maximum 63 bytes; try again.\n"; // Я ограничиваю ответ.
            } while (msg.size() >= BUF_SIZE); // Я прошу повторить слишком длинный ответ.
            if (!running) break; // Я выхожу из диалога при завершении ввода.
        } // Я закончил общение с текущим клиентом.
        closesocket(s_new); // Я закрываю только сокет клиента, не слушающий сокет.
        cout << "client is lost" << endl; // Я снова готов ждать клиента.
    } // Я закончил основной цикл сервера.
    closesocket(s); // Я закрываю слушающий сокет при выходе из цикла.
    WSACleanup(); // Я освобождаю библиотеку; при Ctrl+C программу завершает ОС.
    return err; // Я возвращаю результат работы сервера.
} // Я закончил программу сервера.
