#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cerrno>

using namespace std;

// --------------------------------------------------
// Open and configure serial port
// --------------------------------------------------
int openSerial(const string& port)
{
    int serial = open(port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);

    if (serial < 0)
        return -1;

    struct termios tty{};

    if (tcgetattr(serial, &tty) != 0)
    {
        close(serial);
        return -1;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(serial, TCSANOW, &tty) != 0)
    {
        close(serial);
        return -1;
    }

    tcflush(serial, TCIOFLUSH);

    return serial;
}

// --------------------------------------------------
// Send command to NodeMCU
// --------------------------------------------------
void sendCommand(int serial, const string& command)
{
    string data = command + "\n";

    ssize_t result = write(serial, data.c_str(), data.size());

    if (result < 0)
    {
        cerr << "ERROR: Failed to send command." << endl;
        return;
    }

    // Give NodeMCU a moment to respond
    usleep(100000);

    char buffer[256];

    int bytesRead = read(serial, buffer, sizeof(buffer) - 1);

    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0';

        string response(buffer);

        // Display only useful ESP8266 response
        size_t pos = response.find("RELAY");

        if (pos == string::npos)
            pos = response.find("LED");

        if (pos != string::npos)
        {
            cout << "ESP8266: "
                 << response.substr(pos);
        }
    }
}

// --------------------------------------------------
// Main program
// --------------------------------------------------
int main()
{
    cout << endl;
    cout << "================================" << endl;
    cout << "       PRUTHVI HOME AUTOMATION" << endl;
    cout << "================================" << endl;
    cout << endl;

    // --------------------------------------------------
    // Automatic NodeMCU serial port detection
    // --------------------------------------------------
    const char* ports[] =
    {
        "/dev/ttyUSB0",
        "/dev/ttyUSB1"
    };

    int serial = -1;
    const char* connectedPort = nullptr;

    for (const char* port : ports)
    {
        serial = openSerial(port);

        if (serial != -1)
        {
            connectedPort = port;
            break;
        }
    }

    if (serial == -1)
    {
        cerr << "ERROR: NodeMCU serial port not found." << endl;
        cerr << "Please connect the NodeMCU through USB." << endl;

        return 1;
    }

    cout << "NodeMCU connected on "
         << connectedPort << endl;

    cout << endl;

    // --------------------------------------------------
    // Home Automation Menu
    // --------------------------------------------------
    while (true)
    {
        cout << "================================" << endl;
        cout << "       PRUTHVI HOME AUTOMATION" << endl;
        cout << "================================" << endl;

        cout << endl;

        cout << "1.  LED ON" << endl;
        cout << "2.  LED OFF" << endl;

        cout << "3.  BULB 1 ON" << endl;
        cout << "4.  BULB 1 OFF" << endl;

        cout << "5.  BULB 2 ON" << endl;
        cout << "6.  BULB 2 OFF" << endl;

        cout << "0.  EXIT" << endl;

        cout << endl;
        cout << "Enter choice: ";

        int choice;

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(10000, '\n');

            cout << "Invalid input." << endl;
            cout << endl;

            continue;
        }

        cout << endl;

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

                cout << "PRUTHVI HOME AUTOMATION CLOSED."
                     << endl;

                return 0;

            default:
                cout << "Invalid choice. Please select 0-6."
                     << endl;
                break;
        }

        cout << endl;
    }

    close(serial);

    return 0;
}
