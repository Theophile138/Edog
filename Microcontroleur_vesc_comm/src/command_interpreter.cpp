#include "command_interpreter.h"

void Commande_interpreter::begin(Stream* stream) {
  _dateStream = stream;
  commandCount = 0;
}

void Commande_interpreter::addCommand(const String& name, CommandFunction func) {
  if (commandCount < MAX_COMMANDS) {
    commands[commandCount++] = {name, func};
  }
}

void Commande_interpreter::handle() {
  while (_dateStream->available()) {
    char c = _dateStream->read();
    if (c == '\n') {
      buffer.trim();
      if (buffer.length() > 0) {
        RunCommand(buffer);
      }
      buffer = ""; 
    } else {
      buffer += c;
    }
  }
}

void Commande_interpreter::RunCommand(const String& cmdLine) {
  
  int espaceIndex = cmdLine.indexOf(' ');
  String cmdName;
  String args;

  if (espaceIndex == -1) { // Pas d'argument
    cmdName = cmdLine;
    args = "";
  } else {
    cmdName = cmdLine.substring(0, espaceIndex);
    args = cmdLine.substring(espaceIndex + 1);
  }

  for (int i = 0; i < commandCount; i++) {
    if (commands[i].name == cmdName) {

      commands[i].function(args);
      return;
    }
  }

  _dateStream->println("Commande inconnue : " + cmdName);
}

int Commande_interpreter::splitArgs(const String& input, String args[], int maxArgs) {
    int argCount = 0;
    int len = input.length();
    int i = 0;

    while (i < len && argCount < maxArgs) {

        while (i < len && isWhitespace(input.charAt(i))) {
            i++;
        }
        if (i >= len) break; 

        int start = i;

        while (i < len && !isWhitespace(input.charAt(i))) {
            i++;
        }

        int end = i;
        args[argCount++] = input.substring(start, end);
    }

    return argCount;
}

void Commande_interpreter::println(const String& message) {
  if (_dateStream) {
    _dateStream->println(message); 
  }
}

void Commande_interpreter::print(const String& message) {
  if (_dateStream) {
    _dateStream->print(message);
  }
}

bool Commande_interpreter::isWhitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\r');
}
  