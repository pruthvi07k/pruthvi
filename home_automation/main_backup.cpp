#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

using namespace std;

int openSerial(const string& port)
{
    int fd = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    if (fd < 0)
        return -1;

    termios tty{};

    if (tcgetattr(fd, &tty) != 0)
    {
        close(fd);
        return -1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;

    tcsetattr(fd, TCSANOW, &tty);

    return fd;
}

void sendCommand(int serial, const string& command)
{
    string data = command + "\n";

    write(serial, data.c_str(), data.size());

    char buffer[256]{};

    int n = read(serial, buffer, sizeof(buffer) - 1);

    if (n > 0)
    {
        buffer[n] = '\0';
        cout << "ESP8266: " << buffer;
    }
}

int main()
{
    int serial = openSerial("/dev/ttyUSB0");

    if (serial < 0)
    {
        cerr << "Unable to connect to NodeMCU.\n";
        return 1;
    }

    int choice;

    while (true)
    {
        cout << "\n";
        cout << "================================\n";
        cout << "       PRUTHVI HOME AUTOMATION\n";
        cout << "================================\n";
        cout << "\n";

        cout << "1.  LED ON\n";
        cout << "2.  LED OFF\n";
        cout << "3.  BULB 1 ON\n";
        cout << "4.  BULB 1 OFF\n";
        cout << "5.  BULB 2 ON\n";
        cout << "6.  BULB 2 OFF\n";
        cout << "0.  EXIT\n";

        cout << "\nEnter choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
                sendCommand(serial, "LED_ON");
                break;

            case 2:
                sendCommand(serial, "LED_OFF");
                break;

            case 3:
                sendCommand(serial, "RELAY1_ON");
                break;

            case 4:
                sendCommand(serial, "RELAY1_OFF");
                break;

            case 5:
                sendCommand(serial, "RELAY2_ON");
                break;

            case 6:
                sendCommand(serial, "RELAY2_OFF");
                break;

            case 0:
                close(serial);
                cout << "\nPRUTHVI HOME AUTOMATION CLOSED.\n";
                return 0;

            default:
                cout << "\nInvalid choice.\n";
        }
    }
}
