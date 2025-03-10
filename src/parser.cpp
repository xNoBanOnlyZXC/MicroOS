#include <Arduino.h>

#define MAX_FUNCTIONS 10
#define MAX_VARIABLES 10

// Структуры для хранения функций и переменных
struct Function {
    String name;
    String body;
};

Function functions[MAX_FUNCTIONS];
int functionCount = 0;

String variables[MAX_VARIABLES];
int variableCount = 0;

// Стек для управления циклами
struct Loop {
    String type; // "for" или "while"
    int currentIteration;
    int maxIterations;
};

Loop loopStack[10];
int loopCount = 0;

// Стек для управления условиями
struct Condition {
    bool isActive;
    bool isTrue;
};

Condition conditionStack[10];
int conditionCount = 0;

void parseLine(String line); // Прототип функции

// Функция для получения значения из строки
String getValue(String command, int index) {
    int count = 0;
    int start = 0;
    int end = 0;
    for (int i = 0; i < command.length(); i++) {
        if (command[i] == ' ' || command[i] == ',') {
            count++;
            if (count == index) {
                start = i + 1;
            } else if (count == index + 1) {
                end = i;
                break;
            }
        }
    }
    return command.substring(start, end);
}

// Выполнение команды
void executeCommand(String command) {
    if (command.startsWith("pin_mode")) {
        int pin = getValue(command, 1).toInt();
        String mode = getValue(command, 2);
        pinMode(pin, mode == "OUTPUT" ? OUTPUT : INPUT);
        Serial.printf("Установлен пин %d в режим %s\n", pin, mode.c_str());
    } else if (command.startsWith("digital_write")) {
        int pin = getValue(command, 1).toInt();
        String state = getValue(command, 2);
        digitalWrite(pin, state == "HIGH" ? HIGH : LOW);
        Serial.printf("Установлено значение %s на пин %d\n", state.c_str(), pin);
    } else if (command.startsWith("wait")) {
        int delayTime = getValue(command, 1).toInt();
        delay(delayTime);
        Serial.printf("Пауза %d мс\n", delayTime);
    } else {
        Serial.println("Неизвестная команда");
    }
}

// Оценка условия
bool evaluateCondition(String condition) {
    if (condition.indexOf("==") != -1) {
        String left = getValue(condition, 1);
        String right = getValue(condition, 3);
        return left.toInt() == right.toInt();
    } else if (condition.indexOf("!=") != -1) {
        String left = getValue(condition, 1);
        String right = getValue(condition, 3);
        return left.toInt() != right.toInt();
    } else if (condition.indexOf(">") != -1) {
        String left = getValue(condition, 1);
        String right = getValue(condition, 3);
        return left.toInt() > right.toInt();
    } else if (condition.indexOf("<") != -1) {
        String left = getValue(condition, 1);
        String right = getValue(condition, 3);
        return left.toInt() < right.toInt();
    }
    return false;
}

// Выполнение функции
void executeFunction(Function func, String args) {
    // Разбираем аргументы
    int argCount = 0;
    String argValues[10];
    int start = 0;
    for (int i = 0; i < args.length(); i++) {
        if (args[i] == ',') {
            argValues[argCount++] = args.substring(start, i);
            start = i + 1;
        }
    }
    argValues[argCount++] = args.substring(start);

    // Заменяем параметры в теле функции
    String body = func.body;
    for (int i = 0; i < argCount; i++) {
        String placeholder = "{" + String(i) + "}";
        body.replace(placeholder, argValues[i]);
    }

    // Выполняем тело функции
    Serial.printf("Выполняется функция: %s\n", func.name.c_str());
    String line;
    int pos = 0;
    while ((pos = body.indexOf('\n', pos)) != -1) {
        line = body.substring(0, pos);
        parseLine(line);
        body = body.substring(pos + 1);
    }
}

// Обработка цикла for
void handleForLoop(String line) {
    int start = line.indexOf('(');
    int end = line.indexOf(')');
    String params = line.substring(start + 1, end);

    // Разбираем параметры: for i in range(5)
    String varName = getValue(params, 1);
    int maxIterations = getValue(params, 3).toInt();

    // Добавляем цикл в стек
    loopStack[loopCount].type = "for";
    loopStack[loopCount].currentIteration = 0;
    loopStack[loopCount].maxIterations = maxIterations;
    loopCount++;
}

// Обработка условия if
void handleIfCondition(String line) {
    int start = line.indexOf('(');
    int end = line.indexOf(')');
    String condition = line.substring(start + 1, end);

    // Проверяем условие
    bool isTrue = evaluateCondition(condition);

    // Добавляем условие в стек
    conditionStack[conditionCount].isActive = true;
    conditionStack[conditionCount].isTrue = isTrue;
    conditionCount++;
}

// Обработка блока else
void handleElseCondition() {
    if (conditionCount > 0) {
        conditionStack[conditionCount - 1].isTrue = !conditionStack[conditionCount - 1].isTrue;
    }
}

// Обработка конца блока
void handleEndBlock() {
    if (loopCount > 0) {
        Loop &currentLoop = loopStack[loopCount - 1];
        if (currentLoop.type == "for") {
            currentLoop.currentIteration++;
            if (currentLoop.currentIteration < currentLoop.maxIterations) {
                return; // Продолжаем выполнение цикла
            }
        }
        loopCount--; // Завершаем цикл
    }

    if (conditionCount > 0) {
        conditionCount--; // Завершаем условие
    }
}

// Универсальная функция для получения строки из источника данных
String (*getNextLineFunc)();

void setNextLineFunction(String (*func)()) {
    getNextLineFunc = func;
}

String getNextLine() {
    if (getNextLineFunc) {
        return getNextLineFunc();
    }
    return "";
}

// Загрузка и выполнение программы
void loadProgram() {
    while (true) {
        String line = getNextLine();
        if (line.isEmpty()) break;
        parseLine(line);
    }
}