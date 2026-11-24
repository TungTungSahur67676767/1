// UDP-клиент со слайдов 51–54. Я отправляю текст и жду такой же текст обратно.
#define _WINSOCK_DEPRECATED_NO_WARNINGS // Я оставляю функции inet_addr и gethostbyname из лекции.
#include <winsock2.h> // Я подключаю сокеты Windows.
#include <string> // Я подключаю строки C++.
#include <iostream> // Я подключаю ввод и вывод.
#include <cstring> // Я подключаю memcpy для копирования адреса.
#pragma comment(lib, "Ws2_32.lib") // Я подключаю библиотеку Winsock при сборке.
using namespace std; // Я пишу cout и string без std::.
#define PORT 666 // Я указываю порт UDP-сервера, как в лекции.
#define SERVERADDR "127.0.0.1" // Я отправляю сообщения серверу на своём компьютере.

int main() { // Я начинаю программу UDP-клиента.
    char buff[10 * 1014]; // Я сохраняю размер приёмного буфера из слайда 51.
    cout << "UDP Demo Client\nType quit to quit\n"; // Я подсказываю команду выхода.
    WSADATA ws{}; // Я выделяю структуру для Winsock, не используя текстовый буфер.
    int err = WSAStartup(0x0202, &ws); // Я запускаю Winsock 2.2.
    if (err != 0) { cout << "WSAStartup: " << err << endl; return 1; } // Я проверяю запуск.
    SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0); // Я создаю IPv4 UDP-сокет.
    if (my_sock == INVALID_SOCKET) { // Я проверяю создание сокета.
        cout << "socket: " << WSAGetLastError() << endl; // Я показываю ошибку.
        WSACleanup(); return 1; // Я освобождаю библиотеку и выхожу.
    } // Я закончил проверку.
    sockaddr_in Daddr{}; // Я обнуляю структуру адреса сервера.
    Daddr.sin_family = AF_INET; // Я указываю IPv4.
    Daddr.sin_port = htons(PORT); // Я задаю порт 666 в сетевом порядке байт.
    Daddr.sin_addr.s_addr = inet_addr(SERVERADDR); // Я сначала пробую разобрать числовой IP.
    if (Daddr.sin_addr.s_addr == INADDR_NONE) { // Я проверяю ошибку inet_addr, а не просто ненулевой адрес.
        HOSTENT* hst = gethostbyname(SERVERADDR); // Я пробую найти IPv4 по имени, как в лекции.
        if (hst == nullptr) { // Я проверяю результат поиска.
            cout << "Unknown host: " << WSAGetLastError() << endl; // Я показываю ошибку.
            closesocket(my_sock); WSACleanup(); return 1; // Я закрываю ресурсы.
        } // Я нашёл адрес сервера.
        memcpy(&Daddr.sin_addr, hst->h_addr_list[0], sizeof(Daddr.sin_addr)); // Я копирую первый IPv4.
    } // Я закончил подготовку адреса.
    while (1) { // Я повторяю отправку и получение датаграмм.
        cout << "S<=C: "; // Я показываю приглашение ввода из лекции.
        string SS; // Я создаю строку для сообщения.
        if (!getline(cin, SS) || SS == "quit") break; // Я выхожу по quit или концу ввода, ничего не отправляя.
        if (SS.size() > 1023) { cout << "Maximum 1023 bytes; try again.\n"; continue; } // Я учитываю буфер сервера.
        if (sendto(my_sock, SS.c_str(), (int)SS.size(), 0, (sockaddr*)&Daddr, sizeof(Daddr)) == SOCKET_ERROR) { // Я отправляю датаграмму.
            err = WSAGetLastError(); cout << "sendto: " << err << endl; break; // Я обрабатываю ошибку отправки.
        } // Я отправил сообщение; это ещё не подтверждает его доставку.
        sockaddr_in SRaddr{}; // Я готовлю место для адреса отправителя ответа.
        int SRaddr_size = sizeof(SRaddr); // Я указываю размер этой структуры.
        int n = recvfrom(my_sock, buff, sizeof(buff) - 1, 0, (sockaddr*)&SRaddr, &SRaddr_size); // Я жду одну ответную датаграмму.
        if (n == SOCKET_ERROR) { // Я проверяю ошибку, прежде чем использовать n как индекс.
            err = WSAGetLastError(); cout << "recvfrom: " << err << endl; break; // Я показываю ошибку.
        } // Я принял ответ; даже нулевая длина допустима для UDP.
        buff[n] = '\0'; // Я добавляю ноль, чтобы вывести принятые байты как текст.
        cout << "S=>C: " << buff << endl; // Я вывожу эхо-ответ сервера.
    } // Я закончил обмен сообщениями.
    closesocket(my_sock); // Я закрываю UDP-сокет.
    WSACleanup(); // Я освобождаю Winsock.
    return err != 0; // Я возвращаю 0 при штатном выходе.
} // Я закончил программу UDP-клиента.
