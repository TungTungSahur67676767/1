// TCP-клиент со слайдов 43–44. Я сохранил порядок действий и названия из лекции.
#define _WINSOCK_DEPRECATED_NO_WARNINGS // Я оставляю старые функции из лекции без предупреждений.
#include <winsock2.h> // Я подключаю сокеты Windows.
#include <iostream> // Я подключаю ввод и вывод.
#include <string> // Я подключаю строки C++.
#include <cstring> // Я подключаю memcpy для копирования адреса.
#pragma comment(lib, "Ws2_32.lib") // Я подключаю библиотеку Winsock при сборке.
using namespace std; // Я пишу cout и string без std::.
#define SRV_HOST "localhost" // Я подключаюсь к своему компьютеру, как в лекции.
#define SRV_PORT 1234 // Я указываю порт сервера.
#define CLNT_PORT 1235 // Я оставляю фиксированный порт клиента из лекции.
#define BUF_SIZE 64 // Я оставляю 64 байта: до 63 байт текста и ноль в конце.

int main() { // Я начинаю программу клиента.
    WSADATA ws{}; // Я выделяю настоящую структуру Winsock, а не буфер char.
    int err = WSAStartup(0x0202, &ws); // Я запускаю Winsock 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // Я проверяю запуск.
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // Я создаю IPv4 TCP-сокет; 0 выбирает протокол по типу.
    if (s == INVALID_SOCKET) { // Я проверяю создание сокета.
        cout << "socket: " << WSAGetLastError() << endl; // Я показываю код ошибки.
        WSACleanup(); return 1; // Я освобождаю Winsock и выхожу.
    } // Я закончил проверку сокета.
    sockaddr_in clnt_sin{}, srv_sin{}; // Я обнуляю структуры своего адреса и адреса сервера.
    clnt_sin.sin_family = AF_INET; // Я выбираю IPv4.
    clnt_sin.sin_addr.s_addr = 0; // Я разрешаю системе выбрать локальный IP.
    clnt_sin.sin_port = htons(CLNT_PORT); // Я перевожу порт 1235 в сетевой порядок байт.
    if (bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin)) == SOCKET_ERROR) { // Я закрепляю свой порт.
        cout << "bind: " << WSAGetLastError() << endl; // Я сообщаю, если порт занят.
        closesocket(s); WSACleanup(); return 1; // Я закрываю ресурсы и выхожу.
    } // Я закончил привязку клиента.
    hostent* hp = gethostbyname(SRV_HOST); // Я узнаю IPv4-адрес по имени localhost.
    if (hp == nullptr) { // Я не использую адрес, если поиск не удался.
        cout << "gethostbyname: " << WSAGetLastError() << endl; // Я показываю ошибку поиска.
        closesocket(s); WSACleanup(); return 1; // Я освобождаю ресурсы.
    } // Я закончил проверку имени.
    srv_sin.sin_port = htons(SRV_PORT); // Я задаю удалённый порт 1234.
    srv_sin.sin_family = AF_INET; // Я указываю IPv4 для сервера.
    memcpy(&srv_sin.sin_addr, hp->h_addr_list[0], sizeof(srv_sin.sin_addr)); // Я копирую первый IPv4.
    if (connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin)) == SOCKET_ERROR) { // Я подключаюсь к серверу.
        cout << "connect: " << WSAGetLastError() << endl; // Я показываю ошибку подключения.
        closesocket(s); WSACleanup(); return 1; // Я закрываю ресурсы.
    } // Я установил соединение.
    cout << "Connected to " << SRV_HOST << ':' << SRV_PORT << endl; // Я подтверждаю подключение.
    char buf[BUF_SIZE]{}; // Я готовлю буфер для ответа сервера.
    string mst; // Я храню свой ответ в строке, как в лекции.
    do { // Я сначала получаю вопрос, потом отправляю ответ.
        int used = 0, from_len = 0; // Я считаю байты, чтобы не выйти за буфер.
        do { // Я собираю одну строку TCP до нулевого байта.
            from_len = recv(s, buf + used, 1, 0); // Я читаю байт; TCP может выдавать данные частями.
            if (from_len <= 0) break; // Я прекращаю чтение при закрытии или ошибке.
            ++used; // Я учитываю принятый байт.
        } while (buf[used - 1] != '\0' && used < BUF_SIZE); // Я жду конец строки, не переполняя буфер.
        if (from_len <= 0) { // Я различаю ошибку и закрытие TCP.
            cout << "recv: " << (from_len == 0 ? 0 : WSAGetLastError()) << " (0 = closed)\n"; // Я объясняю код.
            err = 1; break; // Я завершаю незаконченный диалог.
        } // Я проверил результат приёма.
        if (buf[used - 1] != '\0') { cout << "Message too long\n"; err = 1; break; } // Я отвергаю переполнение.
        cout << "S=>C: " << buf << endl; // Я показываю сообщение сервера.
        do { // Я запрашиваю короткий ответ, который поместится в буфер.
            cout << "C=>S: "; // Я показываю, что сейчас вводит клиент.
            if (!getline(cin, mst)) mst = "Bye"; // Я читаю ответ; при конце ввода завершаю диалог.
            if (mst.size() >= BUF_SIZE) cout << "Maximum 63 bytes; try again.\n"; // Я прошу сократить текст.
        } while (mst.size() >= BUF_SIZE); // Я не отправляю слишком длинный ответ.
        int msg_size = (int)mst.size() + 1, sent = 0, n = 0; // Я включаю '\0' как границу сообщения.
        while (sent < msg_size) { // Я досылаю всю строку, если send передал только часть.
            n = send(s, mst.c_str() + sent, msg_size - sent, 0); // Я отправляю оставшиеся байты.
            if (n <= 0) break; // Я прекращаю отправку при ошибке или отсутствии прогресса.
            sent += n; // Я учитываю реально отправленные байты.
        } // Я закончил отправку строки.
        if (sent != msg_size) { cout << "send failed\n"; err = 1; break; } // Я проверяю полноту отправки.
    } while (mst != "Bye"); // Я заканчиваю после отправки Bye, как в лекции.
    closesocket(s); // Я закрываю свой сокет.
    WSACleanup(); // Я освобождаю библиотеку Winsock.
    return err; // Я возвращаю 0 при штатном завершении.
} // Я закончил программу клиента.
