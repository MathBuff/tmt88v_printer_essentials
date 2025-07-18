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
            // Handle long words longer than maxWidth
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

    contents = output.str();
}

int main()
{
    const char* printer_ip = "192.168.86.200";  // Change this if needed

    // Settings
    int cut_mode = 2;          // 0 = none, 1 = partial cut, 2 = full cut
    bool cut_padding = true;   // true adds 4 new lines padding at the end
    bool word_text_wrapping = true; // Enable text wrapping
    int page_width = 56;       // Default page width for font A normal size

    // Font style and scale settings
    // font_style_index: 0 = Font B (smaller), 1 = Font A (normal)
    // font_scale_index: 0=1x, 1=2x, 2=3x, 3=4x magnification
    int font_style_index = 0;  // Smallest base font by default (Font B)
    int font_scale_index = 0;  // Normal size (1x scale)
    //0,0 is the smallest, page width is 56.

    const std::vector<std::vector<unsigned char>> font_style_cmds = {
        {0x1B, 0x4D, 0x01}, // Font B (smaller base font)
        {0x1B, 0x4D, 0x00}  // Font A (normal base font)
    };

    const std::vector<std::vector<unsigned char>> font_size_cmds = {
        {0x1D, 0x21, 0x00}, // 1x width & height (normal)
        {0x1D, 0x21, 0x11}, // 2x width & height
        {0x1D, 0x21, 0x22}, // 3x width & height
        {0x1D, 0x21, 0x33}  // 4x width & height
    };

    // Adjust page_width based on font style and scale (approximate)
    // For Font B normal size, width ~ 52 chars; Font A normal size ~ 42 chars.
    // Scaling reduces char count by scale factor.
    //int base_width = (font_style_index == 0) ? 52 : 42;
    //page_width = base_width / (font_scale_index + 1);

    // Read input.txt contents
    std::ifstream file("input.txt");
    if (!file) {
        std::cerr << "Failed to open input.txt" << std::endl;
        return 1;
    }
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // Wrap text if enabled
    if (word_text_wrapping) {
        wrapText(contents, page_width);
    }

    std::cout << "==== Here is what we are printing ====" << std::endl;
    std::cout << contents << std::endl;
    std::cout << "==== End of sample ====" << std::endl;

    // Add padding if enabled
    if (cut_padding) {
        contents += "\n\n\n\n";
        std::cout << "4 lines of padding appended." << std::endl;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        return 1;
    }

    // Set up printer address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9100);
    addr.sin_addr.s_addr = inet_addr(printer_ip);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect()");
        close(sock);
        return 1;
    }

    // Send font style command
    send(sock, font_style_cmds[font_style_index].data(), font_style_cmds[font_style_index].size(), 0);

    // Send font size (magnification) command
    send(sock, font_size_cmds[font_scale_index].data(), font_size_cmds[font_scale_index].size(), 0);

    std::cout << "Connected to printer at " << printer_ip << std::endl;

    // Send print data
    ssize_t sent = send(sock, contents.c_str(), contents.size(), 0);
    if (sent < 0) {
        perror("send()");
        close(sock);
        return 1;
    }
    std::cout << "Sent " << sent << " bytes of data from input.txt" << std::endl;

    // Send cut command
    if (cut_mode == 1) {
        unsigned char partial_cut[] = { 0x1D, 0x56, 0x01 };
        send(sock, partial_cut, sizeof(partial_cut), 0);
        std::cout << "✂️ Sent PARTIAL cut command" << std::endl;
    } else if (cut_mode == 2) {
        unsigned char full_cut[] = { 0x1D, 0x56, 0x00 };
        send(sock, full_cut, sizeof(full_cut), 0);
        std::cout << "🔪 Sent FULL cut command" << std::endl;
    } else {
        std::cout << "🚫 No cut command sent" << std::endl;
    }

    close(sock);

    return 0;
}

