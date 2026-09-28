#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

const int PORT = 8090;

int serialPort = -1;

bool ledState = false;
bool relay1State = false;
bool relay2State = false;


// ============================================================
// SERIAL
// ============================================================

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

    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 10;

    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        close(fd);
        return -1;
    }

    tcflush(fd, TCIOFLUSH);

    return fd;
}


bool sendSerialCommand(const string& command)
{
    if (serialPort < 0)
        return false;

    string data = command + "\n";

    ssize_t written =
        write(serialPort, data.c_str(), data.size());

    if (written < 0)
        return false;

    usleep(100000);

    char buffer[256];

    int bytes =
        read(serialPort, buffer, sizeof(buffer) - 1);

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        cout << "NodeMCU: "
             << buffer;
    }

    return true;
}


// ============================================================
// JSON HELPERS
// ============================================================

string getJsonValue(const string& json, const string& key)
{
    string search = "\"" + key + "\"";

    size_t pos = json.find(search);

    if (pos == string::npos)
        return "";

    pos = json.find(':', pos);

    if (pos == string::npos)
        return "";

    pos++;

    while (pos < json.size() && json[pos] == ' ')
        pos++;

    if (pos >= json.size())
        return "";

    if (json[pos] == '"')
    {
        pos++;

        size_t end = json.find('"', pos);

        if (end == string::npos)
            return "";

        return json.substr(pos, end - pos);
    }

    return "";
}


// ============================================================
// API RESPONSE
// ============================================================

string statusJson()
{
    string json = "{";

    json += "\"led\":\"";
    json += ledState ? "ON" : "OFF";
    json += "\",";

    json += "\"bulb1\":\"";
    json += relay1State ? "ON" : "OFF";
    json += "\",";

    json += "\"bulb2\":\"";
    json += relay2State ? "ON" : "OFF";
    json += "\",";

    json += "\"serial\":";
    json += serialPort >= 0 ? "true" : "false";

    json += "}";

    return json;
}


string httpResponse(
    const string& body,
    const string& contentType = "application/json")
{
    string response;

    response += "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: " + contentType + "\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type\r\n";
    response += "Content-Length: " +
                to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;

    return response;
}


string errorResponse(
    const string& message)
{
    string body =
        "{\"success\":false,\"error\":\"" +
        message +
        "\"}";

    string response;

    response += "HTTP/1.1 400 Bad Request\r\n";
    response += "Content-Type: application/json\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Content-Length: " +
                to_string(body.size()) + "\r\n";
    response += "Connection: close\r\n";
    response += "\r\n";
    response += body;

    return response;
}


// ============================================================
// DEVICE CONTROL
// ============================================================

bool controlDevice(
    const string& device,
    const string& state)
{
    bool turnOn = state == "ON";

    if (device == "led")
    {
        if (turnOn)
        {
            if (!sendSerialCommand("LED_ON"))
                return false;

            ledState = true;
        }
        else
        {
            if (!sendSerialCommand("LED_OFF"))
                return false;

            ledState = false;
        }

        return true;
    }


    if (device == "bulb1")
    {
        if (turnOn)
        {
            if (!sendSerialCommand("RELAY1_ON"))
                return false;

            relay1State = true;
        }
        else
        {
            if (!sendSerialCommand("RELAY1_OFF"))
                return false;

            relay1State = false;
        }

        return true;
    }


    if (device == "bulb2")
    {
        if (turnOn)
        {
            if (!sendSerialCommand("RELAY2_ON"))
                return false;

            relay2State = true;
        }
        else
        {
            if (!sendSerialCommand("RELAY2_OFF"))
                return false;

            relay2State = false;
        }

        return true;
    }

    return false;
}


// ============================================================
// HTTP REQUEST
// ============================================================

void handleClient(int client)
{
    char buffer[8192];

    memset(buffer, 0, sizeof(buffer));

    int received =
        recv(client, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0)
    {
        close(client);
        return;
    }

    string request(buffer);

    cout << "\n========== HTTP REQUEST ==========\n";
    cout << request << endl;


    // --------------------------------------------------------
    // OPTIONS
    // --------------------------------------------------------

    if (request.find("OPTIONS") == 0)
    {
        string response = httpResponse("");

        send(client,
             response.c_str(),
             response.size(),
             0);

        close(client);
        return;
    }


    // --------------------------------------------------------
    // GET /api/status
    // --------------------------------------------------------

    if (request.find("GET /api/status") == 0)
    {
        string body = statusJson();

        string response = httpResponse(body);

        send(client,
             response.c_str(),
             response.size(),
             0);

        close(client);
        return;
    }


    // --------------------------------------------------------
    // GET /
    // --------------------------------------------------------

    if (request.find("GET / ") == 0)
    {
        string body =
            "{\"project\":\"PRUTHVI HOME AUTOMATION\","
            "\"backend\":\"C++\","
            "\"status\":\"RUNNING\"}";

        string response = httpResponse(body);

        send(client,
             response.c_str(),
             response.size(),
             0);

        close(client);
        return;
    }


    // --------------------------------------------------------
    // POST /api/device
    // --------------------------------------------------------

    if (request.find("POST /api/device") == 0)
    {
        size_t bodyPosition =
            request.find("\r\n\r\n");

        if (bodyPosition == string::npos)
        {
            string response =
                errorResponse("Invalid HTTP request");

            send(client,
                 response.c_str(),
                 response.size(),
                 0);

            close(client);
            return;
        }

        string body =
            request.substr(bodyPosition + 4);

        cout << "JSON: " << body << endl;


        string device =
            getJsonValue(body, "device");

        string state =
            getJsonValue(body, "state");


        cout << "Device: " << device << endl;
        cout << "State : " << state << endl;


        if (device.empty() ||
            (state != "ON" && state != "OFF"))
        {
            string response =
                errorResponse("Invalid device or state");

            send(client,
                 response.c_str(),
                 response.size(),
                 0);

            close(client);
            return;
        }


        if (!controlDevice(device, state))
        {
            string response =
                errorResponse(
                    "NodeMCU serial command failed");

            send(client,
                 response.c_str(),
                 response.size(),
                 0);

            close(client);
            return;
        }


        string responseBody =
            "{\"success\":true,"
            "\"device\":\"" + device +
            "\","
            "\"state\":\"" + state +
            "\"}";


        string response =
            httpResponse(responseBody);


        send(client,
             response.c_str(),
             response.size(),
             0);

        close(client);
        return;
    }


    // --------------------------------------------------------
    // UNKNOWN ROUTE
    // --------------------------------------------------------

    string response =
        errorResponse("Route not found");

    send(client,
         response.c_str(),
         response.size(),
         0);

    close(client);
}


// ============================================================
// MAIN
// ============================================================

int main()
{
    cout << endl;
    cout << "========================================\n";
    cout << "       PRUTHVI C++ BACKEND\n";
    cout << "========================================\n";
    cout << endl;


    // --------------------------------------------------------
    // SERIAL CONNECTION
    // --------------------------------------------------------

    const char* ports[] =
    {
        "/dev/ttyUSB0",
        "/dev/ttyUSB1"
    };


    for (const char* port : ports)
    {
        serialPort = openSerial(port);

        if (serialPort >= 0)
        {
            cout << "NodeMCU connected: "
                 << port << endl;

            break;
        }
    }


    if (serialPort < 0)
    {
        cerr << "WARNING: NodeMCU not found.\n";
        cerr << "Backend will still start.\n";
    }


    // --------------------------------------------------------
    // SOCKET
    // --------------------------------------------------------

    int serverSocket =
        socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0)
    {
        cerr << "ERROR: Socket creation failed.\n";
        return 1;
    }


    int option = 1;

    setsockopt(
        serverSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        &option,
        sizeof(option)
    );


    sockaddr_in serverAddress{};

    serverAddress.sin_family = AF_INET;

    // Local network access.
    serverAddress.sin_addr.s_addr =
        INADDR_ANY;

    serverAddress.sin_port =
        htons(PORT);


    if (bind(
        serverSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)) < 0)
    {
        cerr << "ERROR: Bind failed.\n";

        close(serverSocket);

        return 1;
    }


    if (listen(serverSocket, 10) < 0)
    {
        cerr << "ERROR: Listen failed.\n";

        close(serverSocket);

        return 1;
    }


    cout << endl;
    cout << "C++ API running on port "
         << PORT << endl;

    cout << endl;

    cout << "API endpoints:\n";
    cout << "  GET  /api/status\n";
    cout << "  POST /api/device\n";

    cout << endl;

    cout << "PRUTHVI BACKEND READY.\n";
    cout << endl;


    // --------------------------------------------------------
    // SERVER LOOP
    // --------------------------------------------------------

    while (true)
    {
        sockaddr_in clientAddress{};

        socklen_t clientLength =
            sizeof(clientAddress);


        int client =
            accept(
                serverSocket,
                (sockaddr*)&clientAddress,
                &clientLength
            );


        if (client < 0)
        {
            cerr << "ERROR: Accept failed.\n";
            continue;
        }


        handleClient(client);
    }


    if (serialPort >= 0)
        close(serialPort);

    close(serverSocket);

    return 0;
}
