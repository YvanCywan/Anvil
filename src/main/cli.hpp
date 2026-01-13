#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <fmt/core.h>

namespace anvil {
    class Command {
    public:
        virtual ~Command() = default;

        virtual std::string getName() const = 0;

        virtual std::string getDescription() const = 0;

        virtual int execute(const std::vector<std::string> &args, const std::string &exePath) = 0;
    };

    class CommandRegistry {
    public:
        void registerCommand(std::unique_ptr<Command> cmd) {
            commands[cmd->getName()] = std::move(cmd);
        }

        int execute(const std::string &name, const std::vector<std::string> &args, const std::string &exePath) {
            auto it = commands.find(name);
            if (it != commands.end()) {
                return it->second->execute(args, exePath);
            }
            fmt::print(stderr, "Unknown command: {}\n", name);
            printHelp();
            return 1;
        }

        void printHelp() const {
            fmt::print("Usage: anvil <command> [options]\n");
            fmt::print("Available commands:\n");
            for (const auto &pair: commands) {
                fmt::print("  {} - {}\n", pair.first, pair.second->getDescription());
            }
        }

    private:
        std::map<std::string, std::unique_ptr<Command> > commands;
    };
}
