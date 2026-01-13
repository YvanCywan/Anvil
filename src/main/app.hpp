#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <fmt/core.h>
#include "cli.hpp"
#include "build_command.hpp"
#include "clean_command.hpp"
#include "run_command.hpp"
#include "test_command.hpp"

namespace anvil {
    class App {
    public:
        static int run(int argc, char *argv[]) {
            fmt::print("[Anvil] Starting...\n");
            CommandRegistry registry;
            registry.registerCommand(std::make_unique<BuildCommand>());
            registry.registerCommand(std::make_unique<CleanCommand>());
            registry.registerCommand(std::make_unique<RunCommand>());
            registry.registerCommand(std::make_unique<TestCommand>());

            if (argc < 2) {
                registry.printHelp();
                return 0;
            }

            const std::string commandName = argv[1];
            std::vector<std::string> args;
            for (int i = 2; i < argc; ++i) {
                args.emplace_back(argv[i]);
            }

            return registry.execute(commandName, args, argv[0]);
        }
    };
}
