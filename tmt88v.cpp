#include "tmt88v.h"

#include <fstream>
#include <sstream>
#include <cstring>      // memset()
#include <unistd.h>     // close()
#include <arpa/inet.h>  // inet_addr(), htons()
#include <sys/socket.h> // socket(), connect(), send()
#include <netinet/in.h> // sockaddr_in
#include <iostream>     //for console debug

//(PRIVATE)==================================
//Helper
void tmt88v::wrapText(std::string& contents, int maxWidth) {
    std::istringstream input(contents);
    std::ostringstream output;
    std::string line;

    while (std::getline(input, line)) {
        std::istringstream wordStream(line);
        std::string word, currentLine;
        while (wordStream >> word) {
            while (word.length() > maxWidth) {
                if (!currentLine.empty()) {
                    output << currentLine << "\n";
                    currentLine.clear();
                }
                output << word.substr(0, maxWidth) << "\n";
                word = word.substr(maxWidth);
            }
            if (currentLine.empty())
                currentLine = word;
            else if (currentLine.length() + 1 + word.length() <= maxWidth)
                currentLine += " " + word;
            else {
                output << currentLine << "\n";
                currentLine = word;
            }
        }
        if (!currentLine.empty())
            output << currentLine << "\n";
    }
    contents = output.str();
}

//(PUBLIC)====================================
// Getters
const char* tmt88v::getPrinterIP() const {
    return printer_ip;
}

const std::string& tmt88v::getText() const {
    return text;
}

int tmt88v::getCutMode() const {
    return cut_mode;
}

bool tmt88v::getCutPadding() const {
    return cut_padding;
}

bool tmt88v::getWordTextWrapping() const {
    return word_text_wrapping;
}

int tmt88v::getPageWidth() const {
    return page_width;
}

int tmt88v::getFontStyleIndex() const {
    return font_style_index;
}

int tmt88v::getFontScaleIndex() const {
    return font_scale_index;
}

const std::vector<std::vector<unsigned char>>& tmt88v::getFontStyleCmds() const {
    return font_style_cmds;
}

const std::vector<std::vector<unsigned char>>& tmt88v::getFontSizeCmds() const {
    return font_size_cmds;
}

bool tmt88v::getDebugMessages() const {
    return debugMessages;
}

void tmt88v::setDebugMessages(bool debug) {
    debugMessages = debug;
}

// Setters
void tmt88v::setPrinterIP(const char* ip) {
    printer_ip = ip;
}

void tmt88v::setText(const std::string& newText) {
    text = newText;
}

void tmt88v::setCutMode(int mode) {
    cut_mode = mode;
}

void tmt88v::setCutPadding(bool padding) {
    cut_padding = padding;
}

void tmt88v::setWordTextWrapping(bool wrapping) {
    word_text_wrapping = wrapping;
}

void tmt88v::setPageWidth(int width) {
    page_width = width;
}

void tmt88v::setFontStyleIndex(int index) {
    font_style_index = index;
}

void tmt88v::setFontScaleIndex(int index) {
    font_scale_index = index;
}

//Special
bool tmt88v::loadTextFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        return false; // Could not open file
    }

    // Read whole file into the string
    text.assign((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());

    return true;
}

bool tmt88v::print() {
    std::string contents = text;

    if (word_text_wrapping) {
        wrapText(contents, page_width);
    }

    if (debugMessages) {
        std::cout << "==== Here is what we are printing ====\n"
                  << contents
                  << "\n==== End of sample ====\n";
    }

    if (cut_padding) {
        contents += "\n\n\n\n";
        if (debugMessages) {
            std::cout << "4 lines of padding appended.\n";
        }
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket()");
        return false;
    }

    struct sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9100);
    addr.sin_addr.s_addr = inet_addr(printer_ip);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect()");
        close(sock);
        return false;
    }

    send(sock, font_style_cmds[font_style_index].data(), font_style_cmds[font_style_index].size(), 0);
    send(sock, font_size_cmds[font_scale_index].data(), font_size_cmds[font_scale_index].size(), 0);

    if (debugMessages) {
        std::cout << "Connected to printer at " << printer_ip << "\n";
    }

    ssize_t sent = send(sock, contents.c_str(), contents.size(), 0);
    if (sent < 0) {
        perror("send()");
        close(sock);
        return false;
    }

    if (debugMessages) {
        std::cout << "Sent " << sent << " bytes of data\n";
    }

    if (cut_mode == 1) {
        unsigned char partial_cut[] = {0x1D, 0x56, 0x01};
        send(sock, partial_cut, sizeof(partial_cut), 0);
        if (debugMessages) {
            std::cout << "✂️ Sent PARTIAL cut command\n";
        }
    } else if (cut_mode == 2) {
        unsigned char full_cut[] = {0x1D, 0x56, 0x00};
        send(sock, full_cut, sizeof(full_cut), 0);
        if (debugMessages) {
            std::cout << "🔪 Sent FULL cut command\n";
        }
    } else if (debugMessages) {
        std::cout << "🚫 No cut command sent\n";
    }

    close(sock);
    return true;
}

