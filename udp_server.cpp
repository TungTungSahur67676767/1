// UDP-эхо-сервер со слайдов 48–50. Я возвращаю клиенту ровно то, что он прислал.
#define _WINSOCK_DEPRECATED_NO_WARNINGS // Я оставляю учебные функции gethostbyaddr и inet_ntoa.
#include <winsock2.h> // Я подключаю сокеты Windows.
#include <iostream> // Я подключаю вывод в консоль.
#pragma comment(lib, "Ws2_32.lib") // Я подключаю библиотеку Winsock при сборке.
using namespace std; // Я пишу cout без std::.
#define PORT 666 // Я оставляю порт UDP-сервера из лекции.

int main() { // Я начинаю программу UDP-сервера.
    char buff[1024]; // Я оставляю буфер сервера из лекции.
    cout << "UDP DEMO ECHO-SERVER\n"; // Я показываю название программы.
    WSADATA ws{}; // Я выделяю отдельную структуру для Winsock.
    int err = WSAStartup(0x0202, &ws); // Я запускаю Winsock 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // Я проверяю запуск.
    SOCKET Lsock = socket(AF_INET, SOCK_DGRAM, 0); // Я создаю IPv4 UDP-сокет; 0 выбирает UDP по типу.
    if (Lsock == INVALID_SOCKET) { // Я проверяю создание сокета.
        cout << "socket: " << WSAGetLastError() << endl; // Я показываю ошибку.
        WSACleanup(); return 1; // Я освобождаю библиотеку и выхожу.
    } // Я закончил проверку сокета.
    sockaddr_in Laddr{}; // Я обнуляю структуру адреса сервера.
    Laddr.sin_family = AF_INET; // Я использую IPv4.
    Laddr.sin_addr.s_addr = INADDR_ANY; // Я принимаю датаграммы на всех локальных IPv4-интерфейсах.
    Laddr.sin_port = htons(PORT); // Я записываю порт 666 в сетевом порядке байт.
    if (bind(Lsock, (sockaddr*)&Laddr, sizeof(Laddr)) == SOCKET_ERROR) { // Я закрепляю адрес и порт.
        cout << "bind: " << WSAGetLastError() << endl; // Я показываю ошибку привязки.
        closesocket(Lsock); WSACleanup(); return 1; // Я освобождаю ресурсы.
    } // Я готов принимать датаграммы без listen и accept.
    cout << "UDP server: port " << PORT << ". Start the client.\n" << flush; // Я показываю готовность.
    while (1) { // Я по очереди принимаю датаграммы от клиентов.
        sockaddr_in Caddr{}; // Я готовлю место для адреса отправителя.
        int Caddr_size = sizeof(Caddr); // Я передаю размер этой структуры.
        int bsize = recvfrom(Lsock, buff, sizeof(buff) - 1, 0, (sockaddr*)&Caddr, &Caddr_size); // Я получаю текст и адрес.
        if (bsize == SOCKET_ERROR) { // Я проверяю ошибку до обращения к буферу.
            err = WSAGetLastError(); cout << "recvfrom: " << err << endl; // Я запоминаю и показываю код.
            if (err == WSAEMSGSIZE) { err = 0; continue; } // Я пропускаю слишком большую датаграмму.
            break; // Я завершаю сервер при другой ошибке, чтобы освободить сокет.
        } // Я закончил проверку приёма; ноль для UDP допустим.
        HOSTENT* hst = gethostbyaddr((char*)&Caddr.sin_addr, sizeof(Caddr.sin_addr), AF_INET); // Я узнаю имя отправителя.
        cout << "NEW DATAGRAM!\n" << (hst ? hst->h_name : "Unknown host") << '\n'; // Я вывожу имя, если оно найдено.
        cout << inet_ntoa(Caddr.sin_addr) << ':' << ntohs(Caddr.sin_port) << '\n'; // Я показываю IP и порт клиента.
        buff[bsize] = '\0'; // Я добавляю ноль для вывода текста; для UDP это не разделитель сообщений.
        cout << "C=>S: " << buff << endl; // Я показываю полученное сообщение.
        if (sendto(Lsock, buff, bsize, 0, (sockaddr*)&Caddr, sizeof(Caddr)) == SOCKET_ERROR) { // Я возвращаю те же байты.
            err = WSAGetLastError(); cout << "sendto: " << err << endl; break; // Я сообщаю об ошибке отправки.
        } // Я отправил эхо-ответ по адресу отправителя.
    } // Я закончил цикл при ошибке; обычно сервер ждёт до Ctrl+C.
    closesocket(Lsock); // Я закрываю UDP-сокет при выходе из цикла.
    WSACleanup(); // Я освобождаю Winsock; при Ctrl+C программу завершает ОС.
    return err != 0; // Я возвращаю 1, если завершился из-за ошибки.
} // Я закончил программу UDP-сервера.
