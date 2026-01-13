#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <iostream>

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
            std::cerr << "Unknown command: " << name << std::endl;
            printHelp();
            return 1;
        }

        void printHelp() const {
            std::cout << "Usage: anvil <command> [options]" << std::endl;
            std::cout << "Available commands:" << std::endl;
            for (const auto &pair: commands) {
                std::cout << "  " << pair.first << " - " << pair.second->getDescription() << std::endl;
            }
        }

    private:
        std::map<std::string, std::unique_ptr<Command> > commands;
    };
}
