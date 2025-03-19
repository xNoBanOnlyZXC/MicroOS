#include "mx.h"

TextEditor::TextEditor() : cursorRow(0), cursorCol(0), terminalRows(0), terminalCols(0), isModified(false), shouldExit(false) {}

String repeatChar(char c, int count);

void TextEditor::begin(const String &filePath) {
    fileName = filePath;
    loadFile();
    getTerminalSize();
    saveTerminalState();
    displayContent();
}

void TextEditor::run() {
    while (!shouldExit) {
        if (Serial.available()) {
            char input = Serial.read();
            processInput(input);
            displayContent();
        }
    }
    ESP.restart();
}

void TextEditor::loadFile() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return;
    }
    File file = LittleFS.open(fileName, "r");
    if (!file) {
        Serial.println("File not found, creating new one...");
        fileContent = "";
        return;
    }
    fileContent = file.readString();
    file.close();
}

void TextEditor::saveFile() {
    File file = LittleFS.open(fileName, "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.print(fileContent);
    file.close();
    Serial.printf("\033[%d;%dH", terminalRows, 1);
    Serial.print("File saved successfully!             ");
    delay(2000);
    isModified = false;
}

void TextEditor::displayContent() {
    Serial.write("\033[2J\033[H");
    String lines[terminalRows];
    int start = 0, end = fileContent.indexOf('\n'), index = 0;
    while (end != -1 && index < terminalRows) {
        lines[index++] = fileContent.substring(start, end);
        start = end + 1;
        end = fileContent.indexOf('\n', start);
    }
    if (start < fileContent.length() && index < terminalRows) {
        lines[index++] = fileContent.substring(start);
    }
    while (index < terminalRows) {
        lines[index++] = " ";
    }
    for (int i = 0; i < terminalRows; i++) {
        if (i < terminalRows - 1) {
            Serial.println(lines[i]);
        } else {
            Serial.print(lines[i]);
        }
    }
    Serial.printf("\033[%d;%dH", terminalRows, 1);
    Serial.print("Use Ctrl+S to save, Ctrl+X to cancel, Ctrl+C to exit");
    Serial.printf("\033[%d;%dH", cursorRow + 1, cursorCol + 1);
}

void TextEditor::processInput(char input) {
    if (input == '\r' || input == '\n') {
        fileContent += '\n';
        cursorRow++;
        cursorCol = 0;
        isModified = true;
    } else if (input == 127 || input == 8) {
        if (cursorCol > 0) {
            int lastNewline = fileContent.lastIndexOf('\n', cursorRow * terminalCols + cursorCol - 1);
            if (lastNewline != -1) {
                fileContent.remove(lastNewline + cursorCol - 1, 1);
                cursorCol--;
                isModified = true;
            }
        }
    } else if (input == 19) { // Ctrl+S
        saveFile();
    } else if (input == 24) { // Ctrl+X
        loadFile();
        cursorRow = 0;
        cursorCol = 0;
        isModified = false;
    } else if (input == 3) { // Ctrl+C
        if (isModified) {
            Serial.printf("\033[%d;%dH", terminalRows, 1);
            Serial.print("Unsaved changes! Use Ctrl+S or Ctrl+X first.");
            return;
        }
        shouldExit = true;
    } else if (input == 27) { // Escape sequences for arrow keys
        while (Serial.available() < 2);
        char seq1 = Serial.read();
        char seq2 = Serial.read();
        if (seq1 == '[') {
            if (seq2 == 'A') {
                if (cursorRow > 0) cursorRow--;
            } else if (seq2 == 'B') {
                if (cursorRow < terminalRows - 2) {
                    cursorRow++;
                    ensureLineExists(cursorRow);
                }
            } else if (seq2 == 'C') {
                if (cursorCol < terminalCols - 1) cursorCol++;
            } else if (seq2 == 'D') {
                if (cursorCol > 0) cursorCol--;
            }
        }
    } else {
        fileContent += input;
        cursorCol++;
        isModified = true;
    }
}

void TextEditor::saveTerminalState() {
    savedTerminalContent = "";
    for (int i = 0; i < terminalRows - 1; i++) {
        savedTerminalContent += "\n";
    }
}

void TextEditor::restoreTerminalState() {
    Serial.write("\033[2J\033[H");
    Serial.print(savedTerminalContent);
    Serial.printf("\033[%d;%dH", 1, 1);
}

void TextEditor::getTerminalSize() {
    Serial.write("\033[18t");
    delay(200); // Увеличиваем задержку для получения полного ответа
    String response = "";
    while (Serial.available()) {
        char c = Serial.read();
        response += c;
    }
    int rowsStart = response.indexOf(';') + 1;
    int colsStart = response.indexOf(';', rowsStart) + 1;
    if (rowsStart != -1 && colsStart != -1) {
        terminalRows = response.substring(rowsStart, colsStart - 1).toInt();
        terminalCols = response.substring(colsStart, response.indexOf('t')).toInt();
    } else {
        terminalRows = 24;
        terminalCols = 80;
    }
}

void TextEditor::ensureLineExists(int row) {
    int currentLines = 0;
    int start = 0, end = fileContent.indexOf('\n');
    while (end != -1) {
        currentLines++;
        start = end + 1;
        end = fileContent.indexOf('\n', start);
    }
    if (start < fileContent.length()) currentLines++;
    while (currentLines <= row) {
        fileContent += repeatChar(' ', terminalCols) + '\n';
        currentLines++;
    }
}

String repeatChar(char c, int count) {
    String result = "";
    for (int i = 0; i < count; i++) {
        result += c;
    }
    return result;
}