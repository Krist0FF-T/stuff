#ifndef COMMANDS_H
#define COMMANDS_H

#include <map>

enum commands {
    COMMAND_FILL,
    COMMAND_REPLACE,
    COMMAND_TP,
    COMMAND_REACH,
    COMMAND_SPEED,
    COMMAND_RAND_REPLACE
};

static const std::map<std::string, int> commands {
    {"fill", COMMAND_FILL},
    {"replace", COMMAND_REPLACE},
    {"tp", COMMAND_TP},
    {"reach", COMMAND_REACH},
    {"speed", COMMAND_SPEED},
    {"rand_replace", COMMAND_RAND_REPLACE}
};

#endif