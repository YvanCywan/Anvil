#include <iostream>
#include <string>
#include <vector>
#include "cli.hpp"
#include "build_command.hpp"
#include "clean_command.hpp"

int main(int argc, char *argv[]) {
    anvil::CommandRegistry registry;
    registry.registerCommand(std::make_unique<anvil::BuildCommand>());
    registry.registerCommand(std::make_unique<anvil::CleanCommand>());


    if (argc < 2) {
        registry.printHelp();
        return 0;
    }

    const std::string commandName = argv[1];
    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }

    return registry.execute(commandName, args);
}
