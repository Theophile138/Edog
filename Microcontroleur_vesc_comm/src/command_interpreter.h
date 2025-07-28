#ifndef COMMAND_INTERPRETER_h
#define COMMAND_INTERPRETER_h

#include <Arduino.h>

static const int MAX_COMMANDS = 10;

typedef void (*CommandFunction)(String args);

struct Command {
  String name;
  CommandFunction function;
};

class Commande_interpreter
{
  public:
    void begin(Stream* dateStream);

    void handle();
    void addCommand(const String& name, CommandFunction func);

    static int splitArgs(const String& input, String args[], int maxArgs);
    void println(const String& message);
    void print(const String& message);

  private:
    Stream* _dateStream;  
    Command commands[MAX_COMMANDS];
    int commandCount;
    String buffer;

    void RunCommand(const String& cmdLine);
    static bool isWhitespace(char c);
};

#endif 