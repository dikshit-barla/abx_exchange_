#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#else
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <errno.h>

// ANSI Color codes
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

struct Packet {
    char symbol[5];
    char buySellindicator;
    int32_t quantity;
    int32_t price;
    int32_t packetSequence;
};

class ProgressBar {
    int width;
    float progress;
public:
    ProgressBar(int w = 50) : width(w), progress(0) {}

    void update(float value) {
        progress = value;
        int pos = width * progress;
        std::cout << "\r" << BLUE << "[";
        for (int i = 0; i < width; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << "%" << RESET << std::flush;
    }
};

class ABXClient {
private:
    #ifdef _WIN32
        WSADATA wsaData;
        SOCKET socketFd;
    #else
        int socketFd;
    #endif

    const char* hostname;
    const int port;
    std::vector<Packet> packets;
    std::set<int> receivedSequences;
    std::chrono::steady_clock::time_point startTime;

    void displayHeader() {
        std::cout << "\n" << BOLD << CYAN;
        std::cout << "+----------------------------------------+\n";
        std::cout << "|           ABX Exchange Client           |\n";
        std::cout << "+----------------------------------------+\n" << RESET;
    }

    void displayStats() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

        std::cout << "\n" << BOLD << YELLOW;
        std::cout << "+-------------- Statistics --------------+\n";
        std::cout << "| Total Packets: " << std::setw(21) << packets.size() << " |\n";
        std::cout << "| Execution Time: " << std::setw(19) << duration << "s |\n";
        std::cout << "| Packets/Second: " << std::setw(19) << packets.size() / (duration ? duration : 1) << " |\n";
        std::cout << "+--------------------------------------+\n" << RESET;
    }

    bool connectToServer() {
        #ifdef _WIN32
            socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (socketFd == INVALID_SOCKET) {
                std::cerr << RED << "* Socket creation failed: " << WSAGetLastError() << RESET << std::endl;
                return false;
            }
        #else
            socketFd = socket(AF_INET, SOCK_STREAM, 0);
            if (socketFd < 0) {
                std::cerr << RED << "* Socket creation failed" << RESET << std::endl;
                return false;
            }
        #endif

        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(hostname);

        if (connect(socketFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            #ifdef _WIN32
                std::cerr << RED << "* Connection failed: " << WSAGetLastError() << RESET << std::endl;
                closesocket(socketFd);
            #else
                std::cerr << RED << "* Connection failed" << RESET << std::endl;
                close(socketFd);
            #endif
            return false;
        }

        std::cout << GREEN << "+ Connected to server" << RESET << std::endl;
        return true;
    }

    void sendRequest(uint8_t callType, uint8_t resendSeq = 0) {
        uint8_t request[2] = {callType, resendSeq};
        if (send(socketFd, reinterpret_cast<const char*>(request), sizeof(request), 0) < 0) {
            std::cerr << RED << "* Failed to send request" << RESET << std::endl;
        }
    }

    bool receivePacket(Packet& packet) {
        uint8_t buffer[17];
        const size_t bufferSize = sizeof(buffer);  // Define buffer size as constant
        int bytesRead = 0;
        size_t totalBytesRead = 0;
    
        while (totalBytesRead < bufferSize) {  // Now comparing same types (size_t with size_t)
            bytesRead = recv(socketFd, 
                            reinterpret_cast<char*>(buffer) + totalBytesRead, 
                            static_cast<int>(bufferSize - totalBytesRead), 
                            0);
            
            if (bytesRead <= 0) {
                if (bytesRead == 0) {
                    return false; // Connection closed
                }
                #ifdef _WIN32
                    if (WSAGetLastError() == WSAEINTR) continue;
                    std::cerr << RED << "* Error receiving data: " << WSAGetLastError() << RESET << std::endl;
                #else
                    if (errno == EINTR) continue;
                    std::cerr << RED << "* Error receiving data: " << strerror(errno) << RESET << std::endl;
                #endif
                return false;
            }
            totalBytesRead += static_cast<size_t>(bytesRead);
        }
    
        memcpy(packet.symbol, buffer, 4);
        packet.symbol[4] = '\0';
        packet.buySellindicator = buffer[4];
        packet.quantity = ntohl(*reinterpret_cast<int32_t*>(buffer + 5));
        packet.price = ntohl(*reinterpret_cast<int32_t*>(buffer + 9));
        packet.packetSequence = ntohl(*reinterpret_cast<int32_t*>(buffer + 13));
        
        return true;
    }

    void processPacket(const Packet& packet) {
        packets.push_back(packet);
        receivedSequences.insert(packet.packetSequence);
        
        static const char spinner[] = {'|', '/', '-', '\\'};
        static int spinPos = 0;
        std::cout << "\r" << CYAN << spinner[spinPos++ % 4] << " Processing packet " 
                 << std::setw(4) << packet.packetSequence << " [" << packet.symbol << "]" << RESET << std::flush;
    }

    std::string packetToJSON(const Packet& packet, bool isLast) {
        std::stringstream ss;
        ss << "    {\n";
        ss << "        \"symbol\": \"" << packet.symbol << "\",\n";
        ss << "        \"buySellindicator\": \"" << packet.buySellindicator << "\",\n";
        ss << "        \"quantity\": " << packet.quantity << ",\n";
        ss << "        \"price\": " << packet.price << ",\n";
        ss << "        \"packetSequence\": " << packet.packetSequence << "\n";
        ss << "    }" << (isLast ? "\n" : ",\n");
        return ss.str();
    }

    void closeSocket() {
        #ifdef _WIN32
            closesocket(socketFd);
        #else
            close(socketFd);
        #endif
    }

    void requestMissingSequences(int maxSequence) {
        std::cout << "\n" << MAGENTA << "-> Checking for missing sequences..." << RESET << std::endl;
        
        ProgressBar progressBar;
        int missingCount = 0;
        int retrievedCount = 0;
        
        for (int seq = 1; seq <= maxSequence; ++seq) {
            progressBar.update(float(seq) / maxSequence);
            
            if (receivedSequences.find(seq) == receivedSequences.end()) {
                missingCount++;
                std::cout << "\n" << YELLOW << "! Requesting missing sequence: " << seq << RESET;
                
                if (!connectToServer()) {
                    std::cerr << RED << " * Failed" << RESET << std::endl;
                    continue;
                }
    
                sendRequest(2, seq);
                
                Packet packet;
                if (receivePacket(packet)) {
                    processPacket(packet);
                    retrievedCount++;
                    std::cout << GREEN << " + Retrieved successfully" << RESET;
                }
                
                closeSocket();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    
        if (retrievedCount == missingCount) {
            std::cout << "\n" << GREEN << "+ SUCCESS: All " << missingCount 
                     << " missing packets were retrieved successfully!" << RESET << std::endl;
        } else {
            std::cout << "\n" << YELLOW << "! WARNING: Retrieved " << retrievedCount 
                     << " out of " << missingCount << " missing packets." << RESET << std::endl;
        }
    
        std::cout << CYAN << "\nRetrieval Summary:" << RESET << std::endl;
        std::cout << "----------------" << std::endl;
        std::cout << "Total Sequences: " << maxSequence << std::endl;
        std::cout << "Initially Missing: " << missingCount << std::endl;
        std::cout << "Successfully Retrieved: " << retrievedCount << std::endl;
        std::cout << "Retrieval Rate: " << (retrievedCount * 100.0 / missingCount) << "%" << std::endl;
    }
    
    void generateJSON() {
        std::cout << "\n" << CYAN << "-> Generating JSON output..." << RESET << std::endl;
        
        std::ofstream file("output.json");
        file << "[\n";
        
        ProgressBar progressBar;
        size_t totalPackets = packets.size();
        
        for (size_t i = 0; i < totalPackets; ++i) {
            float progress = static_cast<float>(i + 1) / totalPackets;  
            progressBar.update(progress);
            
            bool isLast = (i == totalPackets - 1);
            file << packetToJSON(packets[i], isLast);
        }
        
        file << "]" << std::endl;
        file.close();
        
        progressBar.update(1.0);  
        std::cout << "\n" << GREEN << "+ JSON file generated successfully" << RESET << std::endl;
    }

public:
    ABXClient() : hostname("127.0.0.1"), port(3000) {
        #ifdef _WIN32
            int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
            if (result != 0) {
                std::cerr << "WSAStartup failed: " << result << std::endl;
                exit(1);
            }
        #endif
    }

    ~ABXClient() {
        #ifdef _WIN32
            WSACleanup();
        #endif
    }

    void run() {
        startTime = std::chrono::steady_clock::now();
        displayHeader();
        
        std::cout << CYAN << "-> Initializing connection..." << RESET << std::endl;
        
        if (!connectToServer()) {
            std::cerr << RED << "* Initial connection failed" << RESET << std::endl;
            return;
        }
        
        std::cout << CYAN << "-> Requesting initial packet stream..." << RESET << std::endl;
        sendRequest(1);

        Packet packet;
        while (receivePacket(packet)) {
            processPacket(packet);
        }

        closeSocket();
        std::cout << "\n" << GREEN << "+ Initial stream completed" << RESET << std::endl;

        int maxSequence = 0;
        for (const auto& p : packets) {
            maxSequence = std::max(maxSequence, p.packetSequence);
        }

        requestMissingSequences(maxSequence);

        std::cout << CYAN << "-> Sorting packets..." << RESET;
        std::sort(packets.begin(), packets.end(), 
            [](const Packet& a, const Packet& b) {
                return a.packetSequence < b.packetSequence;
            });
        std::cout << GREEN << " +" << RESET << std::endl;

        generateJSON();
        displayStats();
        
        std::cout << BOLD << GREEN << "\n+ Processing complete! Check output.json for results.\n" << RESET << std::endl;
    }
};

int main() {
    try {
        ABXClient client;
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}