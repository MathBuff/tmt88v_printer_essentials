#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>      // exit()
#include <cstring>      // memset()
#include <unistd.h>     // close()
#include <arpa/inet.h>  // inet_addr(), htons()
#include <sys/socket.h> // socket(), connect(), send()
#include <netinet/in.h> // sockaddr_in
#include <sstream>
#include <vector>

//This function performs text wrapping on a string if called
void wrapText(std::string& contents, int maxWidth) {
    std::istringstream input(contents);
    std::ostringstream output;
    std::string line;

    while (std::getline(input, line)) {
        std::istringstream wordStream(line);
        std::string word;
        std::string currentLine;

        while (wordStream >> word) {
            // Handle thicc words
            while (word.length() > maxWidth) {
                if (!currentLine.empty()) {
                    output << currentLine << "\n";
                    currentLine.clear();
                }
                output << word.substr(0, maxWidth) << "\n";
                word = word.substr(maxWidth);
            }

            if (currentLine.empty()) {
                currentLine = word;
            } else if (currentLine.length() + 1 + word.length() <= maxWidth) {
                currentLine += " " + word;
            } else {
                output << currentLine << "\n";
                currentLine = word;
            }
        }

        if (!currentLine.empty()) {
            output << currentLine << "\n";
        }

        // Preserve paragraph breaks
        if (!input.eof())
            output << "";
    }

    contents = output.str(); // Slam the new, wrapped version into the original variable like a boss
}

int main()
{
    const char* printer_ip = "192.168.0.161";  // 🛑 change this if needed

    // 👇 INITIALIZED CUT MODE SETTING (CONFIGURE THESE FOR BEHAVIORS)
    int cut_mode = 2; // 0 = none, 1 = partial cut, 2 = full cut 🧠 change this as needed
    bool cut_padding = 1; //0 means no cut padding appended to contents, 1 means 4 new lines added.
    bool word_text_wrapping = 0; //0 means no text wrapping, 1 means text wrapping.
    int page_width = 56;//56 characters was found to be width of the page in characters
    //To preserrve white space just turn word text wrapping off.

    
    /*READING IN THE FILE*/
    std::ifstream file("input.txt");
    if (!file) {
            std::cerr << "Failed to open input.txt" << std::endl;
            exit(1);
        }

        //WRITING FILE CHARACTERS INTO CONTENTS STRING FROM FILE
       
        std::string contents;

        contents = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        file.close();

    
    /*Setting Text Wrapping to contents*/
    if (word_text_wrapping == 1) {
        wrapText(contents, page_width); 
    }
    
    std::cout <<"====Here is what we are printing====" <<std::endl;
    std::cout << contents;
    std::cout << std::endl;
    std::cout << "====End of sample====" <<std::endl;
    
    
    /*ADDING PADDING TO CONTENTS*/
    if(cut_padding == 1){
        contents = contents + "\n\n\n\n";
         std::cout << "4 Lines of padding was appended to the messege\n";
    }

    //SOCKET SETUP, CONNECTION, AND SENDING

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        exit(1);
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9100);
    addr.sin_addr.s_addr = inet_addr(printer_ip);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect()");
        close(sock);
        exit(1);
    }

    std::cout << "Connected to printer at " << printer_ip << "\n";

    ssize_t sent = send(sock, contents.c_str(), contents.size(), 0);
    if (sent < 0) {
        perror("send()");
        close(sock);
        exit(1);
    }

    std::cout << "Sent " << sent << " bytes of data from input.txt\n";

    //👇 CUT COMMANDS
    if (cut_mode == 1) {
        unsigned char partial_cut[] = { 0x1D, 0x56, 0x01 };
        send(sock, partial_cut, sizeof(partial_cut), 0);
        std::cout << "✂️ Sent PARTIAL cut command\n";
    } else if (cut_mode == 2) {
        unsigned char full_cut[] = { 0x1D, 0x56, 0x00 };
        send(sock, full_cut, sizeof(full_cut), 0);
        std::cout << "🔪 Sent FULL cut command\n";
    } else {
        std::cout << "🚫 No cut command sent\n";
    }

    close(sock);

    return 0;
}
