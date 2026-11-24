#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string> 
#include <cstring> 
#pragma comment(lib, "Ws2_32.lib") // библиотекa Winsock при сборке.
using namespace std;
#define SRV_HOST "localhost" //подключаюсь к своему компьютеру
#define SRV_PORT 1234 //порт сервера.
#define CLNT_PORT 1235 
#define BUF_SIZE 64 

int main() {
    //проверяем сокет
    WSADATA ws{};
    int err = WSAStartup(0x0202, &ws); //Winsock 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // проверка запускп
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0); // создаю IPv4 TCP-сокет; 0 выбирает протокол по типу.
    if (s == INVALID_SOCKET) { // проверка создания сокета
        cout << "socket: " << WSAGetLastError() << endl; // вывод ошибки
        WSACleanup(); return 1; // Я освобождаю Winsock и выхожу.
    } // конец проверки сокета



    //привязываем клиента
    sockaddr_in clnt_sin{}, srv_sin{}; // обнуление адреса и адреса сервера
    clnt_sin.sin_family = AF_INET; // IPv4.
    clnt_sin.sin_addr.s_addr = 0; // Я разрешаю системе выбрать локальный IP.
    clnt_sin.sin_port = htons(CLNT_PORT); // перевожу порт 1235 в сетевой порядок байт.
    if (bind(s, (sockaddr*)&clnt_sin, sizeof(clnt_sin)) == SOCKET_ERROR) { // закреп своего порта.
        cout << "bind: " << WSAGetLastError() << endl; // оповещение о том что порт возможно занят.
        closesocket(s); WSACleanup(); return 1; // выход
    } // конец привязки клиента


    hostent* hp = gethostbyname(SRV_HOST); // узнаю IPv4-адрес по имени localhost.
    if (hp == nullptr) { // Я не использую адрес, если поиск не удался.
        cout << "gethostbyname: " << WSAGetLastError() << endl;
        closesocket(s); WSACleanup(); return 1;
    } // конец проверки имени


    srv_sin.sin_port = htons(SRV_PORT); // удалённый порт 1234.
    srv_sin.sin_family = AF_INET; // IPv4 для сервера.
    memcpy(&srv_sin.sin_addr, hp->h_addr_list[0], sizeof(srv_sin.sin_addr)); // Я копирую первый IPv4.
    if (connect(s, (sockaddr*)&srv_sin, sizeof(srv_sin)) == SOCKET_ERROR) { // подключаемся к серверу
        cout << "connect: " << WSAGetLastError() << endl; // вывод ошибки подключения 
        closesocket(s); WSACleanup(); return 1;
    } // соединение установлено


    cout << "Connected to " << SRV_HOST << ':' << SRV_PORT << endl;
    char buf[BUF_SIZE]{}; // Я готовлю буфер для ответа сервера.
    string mst;
    do { // сначала получаю вопрос, потом отправляю ответ.
        int used = 0, from_len = 0; // считаю байты, чтобы не выйти за буфер.
        do { // сброка одной строки TCP до нулевого байта.
            from_len = recv(s, buf + used, 1, 0); //  читаю байт; TCP может выдавать данные частями.
            if (from_len <= 0) break; // прекращение чтения при ошибке и тд
            ++used; // учёт принятого байта
        } while (buf[used - 1] != '\0' && used < BUF_SIZE); // ждем конец строки, не переполняя буфер.

        if (from_len <= 0) { // различаем ошибку и закрытие TCP.
            cout << "recv: " << (from_len == 0 ? 0 : WSAGetLastError()) << " (0 = closed)\n";
            err = 1; break;
        } 

        if (buf[used - 1] != '\0') { cout << "Message too long\n"; err = 1; break; } // переполнение
        cout << "S=>C: " << buf << endl; // показываем сообщение сервера.
        do { // запрашиваю короткий ответ, который поместится в буфер.
            cout << "C=>S: "; // что сейчас вводит клиент.
            if (!getline(cin, mst)) mst = "Bye"; // сервер читает ответ; при конце ввода завершает диалог.
            if (mst.size() >= BUF_SIZE) cout << "Maximum 63 bytes; try again.\n"; // сервер просит сократить текст.
        } while (mst.size() >= BUF_SIZE); // не отправляю слишком длинный ответ.
        int msg_size = (int)mst.size() + 1, sent = 0, n = 0; // '\0' как границу сообщения.
        while (sent < msg_size) { // сервер досылаю всю строку, если send передал только часть.
            n = send(s, mst.c_str() + sent, msg_size - sent, 0); // сервер отправляю оставшиеся байты.
            if (n <= 0) break; // сервер прекращаю отправку при ошибке или отсутствии прогресса.
            sent += n; // сервер учитывает реально отправленные байты.
        }
        if (sent != msg_size) { cout << "send failed\n"; err = 1; break; } // Я проверяю полноту отправки.
    } while (mst != "Bye"); // конец при bye 
    closesocket(s); // закрываю свой сокет.
    WSACleanup(); //освобождаю библиотеку Winsock.
    return err; //возвращаю 0 при штатном завершении.
