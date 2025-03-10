#pragma once

#include <Arduino.h>
#include <LittleFS.h>

class TextEditor {
public:
    TextEditor();
    void begin(const String &filePath);
    void run();

private:
    String fileName;
    String fileContent;
    int cursorRow;
    int cursorCol;
    int terminalRows;
    int terminalCols;
    bool isModified;
    bool shouldExit;

    String savedTerminalContent;

    void loadFile();
    void saveFile();
    void displayContent();
    void processInput(char input);
    void saveTerminalState();
    void restoreTerminalState();
    void getTerminalSize();
    void ensureLineExists(int row);
};