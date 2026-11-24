#include <winsock2.h> // сокеты Windows
#include <iostream> // ввод и вывод
#include <string> // строки C++
#pragma comment(lib, "Ws2_32.lib") // Winsock при сборке
using namespace std; 
#define SRV_PORT 1234 // Я оставляю серверный порт из лекции.
#define BUF_SIZE 64 
const string QUEST = "Who are you?\n"; 

int main() { 
    WSADATA ws{}; // структурa для Winsock.
    int err = WSAStartup(0x0202, &ws); // Winsock версии 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // проверка запуска.
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // IPv4 TCP-сокет для ожидания клиентов.
    if (s == INVALID_SOCKET) { // проверяка создание сокета.
        cout << "socket: " << WSAGetLastError() << endl; // код ошибки
        WSACleanup(); return 1;
    } //закончил проверку.



    sockaddr_in sin{}, from_sin{}; //обнуляю адрес сервера и место для адреса клиента.
    sin.sin_family = AF_INET; // IPv4.
    sin.sin_addr.s_addr = 0; // принимаю подключения на всех локальных IPv4-адресах.
    sin.sin_port = htons(SRV_PORT); // Язаписываю порт 1234 в сетевом порядке байт.
    if (bind(s, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR) { //адрес и порт за сокетом.
        cout << "bind: " << WSAGetLastError() << endl; // ошибку привязки.
        closesocket(s); WSACleanup(); return 1;
    }



    string msg, msg1; // храню своё сообщение и ответ клиента
    if (listen(s, 3) == SOCKET_ERROR) { //  включаю ожидание; 3 относится к очереди подключений
        cout << "listen: " << WSAGetLastError() << endl; // ошибка ожидания.
        closesocket(s); WSACleanup(); return 1;
    }


    cout << "TCP server: port " << SRV_PORT << ". Start the client.\n" << flush; // готовность.
    bool running = true; // выйти из сервера при завершении консольного ввода.
    while (running) { 
        int from_len = sizeof(from_sin); // передаю размер структуры адреса клиента.
        SOCKET s_new = accept(s, (sockaddr*)&from_sin, &from_len); // отдельный сокет клиента.
        if (s_new == INVALID_SOCKET) { cout << "accept: " << WSAGetLastError() << endl; err = 1; break; } // Я проверяю приём.
        cout << "new connected client!" << endl; // сообщаю о подключении.
        char buf[BUF_SIZE]{}; // готовлю буфер ответа клиента.
        msg = QUEST; // начинаю каждый диалог с Who are you?
        while (1) { // чередую отправку вопроса и получение ответа.
            int msg_size = (int)msg.size() + 1, sent = 0, n = 0; // ноль как границу строки.
            while (sent < msg_size) { //досылаю сообщение, если send отправил не всё.
                n = send(s_new, msg.c_str() + sent, msg_size - sent, 0); //отправляю оставшиеся байты.
                if (n <= 0) break; // останавливаюсь при ошибке или отсутствии прогресса.
                sent += n; 
            }


            if (sent != msg_size) { cout << "send failed\n"; break; } // прерываю неудачный диалог.
            int used = 0;
            do { 
                from_len = recv(s_new, buf + used, 1, 0); 
                if (from_len <= 0) break; // ошибка или закрытие соединения.
                ++used; // Я учитываю принятый байт.
            } while (buf[used - 1] != '\0' && used < BUF_SIZE);
            if (from_len <= 0) { // проверяю результат приёма.
                cout << "recv: " << (from_len == 0 ? 0 : WSAGetLastError()) << " (0 = closed)\n"; // причина.
                break;
            }



            if (buf[used - 1] != '\0') { cout << "Message too long\n"; break; } // не читаю за буфером.
            msg1 = buf; // Я превращаю полученную C-строку в string, как в лекции.
            cout << "C=>S: " << msg1 << endl; // ответ клиента.
            if (msg1 == "Bye") break; // завершаю диалог по команде клиента.
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
