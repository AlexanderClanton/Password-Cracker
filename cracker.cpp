#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <crypt.h>
#include <string>
#include <vector>
#include "cracker.h"
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>

// Function declarations
static void crackO(Message& mgz, unsigned int startO, unsigned int endO);
static bool crackH(int n, char arr[], int len, int L, char arr2[], char arr4[]);
// static void receive(Message mgz);
// static void send(Message mgz);

int main() {
    // Create a message object
    Message msg;

    // Get host name and print
    char arr[20];
    gethostname(arr, 20);
    std::cout << arr << std::endl;

    // Server names
    char arrN[35] = "aclanton";
    char arrT[20] = "thor";
    char arrO[20] = "olaf";
    char arrNO[20] = "nogbad";
    char arrI[20] = "noggin";

    // Setup UDP socket for multicast
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    bzero((char*) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(get_multicast_port());

    if (bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
        exit(-1);

    struct ip_mreq multicastRequest;
    multicastRequest.imr_multiaddr.s_addr = get_multicast_address();
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&multicastRequest, sizeof(multicastRequest)) < 0)
        exit(-1);

    // Receive multicast messages until one from 'aclanton' is received
    for (;;) {
        recvfrom(sockfd, &msg, sizeof(msg), 0, NULL, 0);
        if (strcmp(msg.cruzid, arrN) == 0) {
            break;
        }
    }

    close(sockfd);

    // Convert values from network to host byte order
    msg.num_passwds = ntohl(msg.num_passwds);
    msg.port = ntohs(msg.port);

    // Divide work among servers
    unsigned int start;
    unsigned int end;
    unsigned int remainder = msg.num_passwds - (msg.num_passwds / 4);

    bool serv1 = false, serv2 = false, serv3 = false, serv4 = false, serv4S = false;

    if ((strcmp(arr, arrT) == 0) && msg.num_passwds < 4) {
        start = 0;
        serv4S = true;
        end = msg.num_passwds;
    }
    else if (strcmp(arr, arrT) == 0) {
        start = 0;
        serv1 = true;
        end = msg.num_passwds / 4;
    }
    else if (strcmp(arr, arrO) == 0) {
        start = msg.num_passwds / 4;
        serv2 = true;
        end = start + msg.num_passwds / 4;
        if (remainder == 3) end += 1;
    }
    else if (strcmp(arr, arrNO) == 0) {
        start = msg.num_passwds / 4;
        serv3 = true;
        start = start * 2;
        if (remainder == 3) {
            start += 1;
            end = start + msg.num_passwds / 4 + 1;
        }
        else if (remainder == 2) {
            end = start + msg.num_passwds / 4 + 1;
        }
    }
    else if (strcmp(arr, arrI) == 0) {
        start = msg.num_passwds / 4;
        start = start * 3;
        serv4 = true;
        if (remainder == 3) {
            start += 2;
            end = msg.num_passwds;
        }
        else if (remainder == 2) {
            start += 1;
            end = msg.num_passwds;
        }
    }
    // Crack the message
crackO(msg, start, end);

// Hold a message object
Message Hold;

// If the first server is active, handle its specific tasks
if (serv1) {
    // Setup a UDP socket for multicast
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(get_multicast_port());

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        exit(-1);

    struct ip_mreq multicastRequest;
    multicastRequest.imr_multiaddr.s_addr = get_multicast_address();
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);
    
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&multicastRequest, sizeof(multicastRequest)) < 0)
        exit(-1);

    // Continuously receive messages and process based on cruzid
    while (true) {
        recvfrom(sockfd, &Hold, sizeof(Hold), 0, NULL, 0);

        // Process message from 'olaf'
        if (strcmp(Hold.cruzid, arrO) == 0 && !serv2) {
            for (unsigned int a = Hold.num_passwds; a < Hold.port; a++) {
                memcpy(msg.passwds[a], Hold.passwds[a], 4);
                msg.passwds[a][4] = '\0';
            }
            std::cout << "got olaf" << std::endl;
            serv2 = true;
        }

        // Process message from 'nogbad'
        if (strcmp(Hold.cruzid, arrNO) == 0 && !serv3) {
            for (unsigned int a = Hold.num_passwds; a < Hold.port; a++) {
                memcpy(msg.passwds[a], Hold.passwds[a], 4);
                msg.passwds[a][4] = '\0';
            }
            std::cout << "got nogbad" << std::endl;
            serv3 = true;
        }

        // Process message from 'noggin'
        if (strcmp(Hold.cruzid, arrI) == 0 && !serv4) {
            for (unsigned int a = Hold.num_passwds; a < Hold.port; a++) {
                memcpy(msg.passwds[a], Hold.passwds[a], 4);
                msg.passwds[a][4] = '\0';
            }
            std::cout << "got noggin" << std::endl;
            serv4 = true;
        }

        if (serv2 && serv3 && serv4) break;
    }
    close(sockfd);
} 
// If the second server is active, handle its specific tasks
else if (serv2) {
    msg.num_passwds = start;
    msg.port = end;
    unsigned int ti = 0;
    strcpy(msg.cruzid, "olaf");
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int ttl = 1;

    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl)) < 0)
        exit(-1);

    struct sockaddr_in multicastAddr;
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = get_multicast_address();
    multicastAddr.sin_port = htons(get_multicast_port());

    while (true) {
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
        sleep(1);
        if (ti > 75) break;
        ti++;
    }
    close(sockfd);
} 
// If the third server is active, handle its specific tasks
else if (serv3) {
    msg.num_passwds = start;
    msg.port = end;
    unsigned int ti = 0;
    strcpy(msg.cruzid, "nogbad");
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int ttl = 1;

    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl)) < 0)
        exit(-1);

    struct sockaddr_in multicastAddr;
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = get_multicast_address();
    multicastAddr.sin_port = htons(get_multicast_port());

    while (true) {
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
        sleep(1);
        if (ti > 75) break;
        ti++;
    }
    close(sockfd);
}
// If third server is active, handle its specific tasks
if (serv3) {
    msg.num_passwds = start;
    msg.port = end;
    unsigned int ti = 0;
    strcpy(msg.cruzid, "nogbad");
    msg.cruzid[6] = '\0';

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int ttl = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
        exit(-1);

    struct sockaddr_in multicastAddr;
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = get_multicast_address();
    multicastAddr.sin_port = htons(get_multicast_port());

    while (ti <= 75) {
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
        sleep(1);
        ti++;
    }

    close(sockfd);
}
// If fourth server is active, handle its specific tasks
else if (serv4) {
    msg.num_passwds = start;
    msg.port = end;
    unsigned int ti = 0;
    strcpy(msg.cruzid, "noggin");
    msg.cruzid[6] = '\0';

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    int ttl = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
        exit(-1);

    struct sockaddr_in multicastAddr;
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = get_multicast_address();
    multicastAddr.sin_port = htons(get_multicast_port());

    while (ti <= 75) {
        sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&multicastAddr, sizeof(multicastAddr));
        sleep(1);
        ti++;
    }

    close(sockfd);
}
// If first or fourth server are active, handle their tasks
if (serv1 || serv4S) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) exit(-1);

    struct hostent *server = gethostbyname(msg.hostname);
    if (server == NULL) exit(-1);

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, &serv_addr.sin_addr.s_addr, server->h_length);

    serv_addr.sin_port = htons(msg.port);
    msg.num_passwds = htonl(msg.num_passwds);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) exit(-1);

    write(sockfd, &msg, sizeof(msg));
    close(sockfd);
}

return 0;

}
// Function to crack the password
bool crackH(int n, char arr[], int len, int L, char arr2[], char arr4[]) {
    char arr3[5];
    char salt[3];
    
    // Extract the salt from arr2
    salt[0] = arr2[0];
    salt[1] = arr2[1];
    
    // Calculate the characters for the password using the given alphabet
    for(int x = 0; x < L; x++) {
        arr3[x] = arr[n % len];
        n /= len;
    }

    // Use the crypt_r function to hash the password
    struct crypt_data data;
    data.initialized = 0;
    char *valz = crypt_r(arr3, salt, &data);

    // If the hashed value matches the target, update arr4 and return true
    if(strcmp(arr2, valz) == 0) {
        for(unsigned int z = 0; z < 4; z++) {
            arr4[z] = arr3[z];
        }
        return true;
    }

    return false;
}

// Function to perform cracking with multiple threads
void crackO(Message &mgz, unsigned int startO, unsigned int endO) {
    const unsigned int totperm2 = 615680;
    const unsigned int ex = 16;
    
    unsigned int startZ = 0;
    unsigned int endZ = totperm2 + ex;
    char arr2[14];
    bool globalMatchFound = false;
    unsigned int x, a;
    unsigned int threadCounter = 0;
    
    std::vector<std::thread> threadsContainer;

    // Lambda function to handle cracking for a range of passwords
    auto lambdaCrack = [&](unsigned int start, unsigned int end) {
        unsigned int current = start;
        bool localMatchFound = false;
        char localPassword[5];
        
        while(current < end && !globalMatchFound) {
            localMatchFound = crackH(current, mgz.alphabet, 62, 4, arr2, localPassword);
            
            if(localMatchFound) {
                for(unsigned int i = 0; i < 4; i++) {
                    mgz.passwds[x][i] = localPassword[i];
                }
                mgz.passwds[x][4] = '\0';
                globalMatchFound = true;
            }

            current++;
        }
    };

    // Iterate over the given password range
    for(x = startO; x < endO; x++) {
        // Copy password to local array
        for(a = 0; a < 13; a++) {
            arr2[a] = mgz.passwds[x][a];
        }
          
        // Create 24 threads to perform cracking
        for(unsigned int f = 0; f < 24; f++) {
            threadsContainer.emplace_back(lambdaCrack, startZ, endZ);
            
            if(threadCounter == 0) {
                startZ += endZ;
            } else {
                startZ += totperm2;
            }
            threadCounter++;
            endZ += totperm2;
        }

        // Join all threads after they finish
        for(auto &t : threadsContainer) {
            t.join();
        }
        
        // Reset counters and flags
        threadCounter = 0;
        globalMatchFound = false;
        startZ = 0;
        endZ = totperm2 + ex;
        threadsContainer.clear();
    }
}





   
