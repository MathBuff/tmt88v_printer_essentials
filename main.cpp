#include "tmt88v.h"

int main() {
    tmt88v printer;
    printer.loadTextFromFile("input.txt");
    printer.print();
    
    return 0;
}
