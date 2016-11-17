//
//  Robot Theatre client in C++
//  Connects to the Robot Theatre server specified by the IP Address and sends commands.
//

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <zmq.hpp>

using namespace std;

enum class CMD
{
    PAGE,
    WALK,
    RST,
    EXIT
};

string ToString(const CMD& cmd)
{
    switch (cmd)
    {
    case CMD::PAGE: return "page"; break;
    case CMD::WALK: return "walk"; break;
    case CMD::RST: return "reset"; break;
    case CMD::EXIT: return "exit"; break;
    default: return "exit"; break;
    }
}

void CmdLineHelp()
{
    cout << left << setw(7) << "-ip  " << setw(12) << "<ip_address> " << " ... " << "robot ip address." << endl;
    cout << left << setw(7) << "-page" << setw(12) << "<ID>         " << " ... " << "execute the specified motion page." << endl;
    cout << left << setw(7) << "-walk" << setw(12) << "             " << " ... " << "enable the walking mode." << endl;
    cout << left << setw(7) << "-rst" << setw(12)  << "             " << " ... " << "put the servos at rest." << endl;
    cout << left << setw(7) << "-exit" << setw(12) << "             " << " ... " << "shutdown the rtserver on the robot." << endl;
    cout << endl;
}

int main(int argc, char* argv[])
{
    string ipAddress = "";
    CMD cmd = CMD::EXIT;
    string pageID = "";

    if (argc < 2)
    {
        CmdLineHelp();
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-ip"  )) { ipAddress = argv[++i]; continue; }
        if (0 == strcmp(argv[i], "-page")) { cmd = CMD::PAGE; pageID = argv[++i]; continue; }
        if (0 == strcmp(argv[i], "-walk")) { cmd = CMD::WALK; continue; }
        if (0 == strcmp(argv[i], "-rst"))  { cmd = CMD::RST;  continue; }
        if (0 == strcmp(argv[i], "-exit")) { cmd = CMD::EXIT; continue; }
    }

    string port = "5555";
    stringstream address;
    address << "tcp://" << ipAddress << ":" << port;

    //  Prepare our context and socket
    zmq::context_t context(2);
    zmq::socket_t socket(context, ZMQ_REQ);

    //cout << "Connecting to " << ipAddress << "Robot Theatre server..." << endl;
    socket.connect(address.str());

    cout << "cmd " << ToString(cmd) << " " << pageID << "... ";
    zmq::message_t cmdRequest(ToString(cmd).length() + 1);
    memcpy(cmdRequest.data(), ToString(cmd).c_str(), ToString(cmd).length() + 1);
    socket.send(cmdRequest);

    if (CMD::PAGE == cmd)
    {
        //  each send must be followed by a recv - for some stupid reason
        // crashes without this
        zmq::message_t reply;
        socket.recv(&reply);

        zmq::message_t quantaRequest(pageID.length() + 1);
        memcpy(quantaRequest.data(), pageID.c_str(), pageID.length()+1);
        socket.send(quantaRequest);
    }

    if (CMD::EXIT != cmd)
    {
        //  Get the reply.
        zmq::message_t reply;
        socket.recv(&reply);
        cout << (const char*)reply.data() << endl;
    }

    return 0;
}