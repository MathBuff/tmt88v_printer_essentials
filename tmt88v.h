#pragma once
#include <string>
#include <vector>
#include <fstream>
class tmt88v {
private:
    const char* printer_ip = "192.168.86.200";  // IP of the Epson TM-T88V
    std::string text = "Hello, Epson TM-T88V!\n"; // Default text to print

    // GeneralSettings
    bool debugMessages = false; // Debug messages enabled by default
    int cut_mode = 2;          // 0 = none, 1 = partial cut, 2 = full cut
    bool cut_padding = true;   // true adds 4 new lines padding at the end
    bool word_text_wrapping = false; // Enable text wrapping
    int page_width = 56;       // Default page width for font A normal size

    // Font style and scale settings
        // font_style_index: 0 = Font B (smaller), 1 = Font A (normal)
        // font_scale_index: 0=1x, 1=2x, 2=3x, 3=4x magnification
    int font_style_index = 0;  // Smallest base font by default (Font B)
    int font_scale_index = 0;  // Normal size (1x scale)
        //0,0 is the smallest font size, at that size page width is 56 characters.

    //Here are message related codes to send to the printer for font sizes:
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

    void wrapText(std::string& contents, int maxWidth);//Text Wrapping Helper

public:
    // Getters
    const char* getPrinterIP() const;
    const std::string& getText() const;
    int getCutMode() const;
    bool getCutPadding() const;
    bool getWordTextWrapping() const;
    int getPageWidth() const;
    int getFontStyleIndex() const;
    int getFontScaleIndex() const;
    const std::vector<std::vector<unsigned char>>& getFontStyleCmds() const;
    const std::vector<std::vector<unsigned char>>& getFontSizeCmds() const;
    bool getDebugMessages() const;
    void setDebugMessages(bool debug);

    // Setters
    void setPrinterIP(const char* ip);
    void setText(const std::string& newText);
    void setCutMode(int mode);
    void setCutPadding(bool padding);
    void setWordTextWrapping(bool wrapping);
    void setPageWidth(int width);
    void setFontStyleIndex(int index);
    void setFontScaleIndex(int index);

    //special
    bool loadTextFromFile(const std::string& filename);
    bool print();
    

};

