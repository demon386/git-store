#include <iostream>
#include <map>
#include "command.hpp"
#include "subprocess.hpp"

namespace po = boost::program_options;

void check_git_cmd_available() {
    if (subprocess::run_cmd("which git", false) != 0) {
        std::cout << "git executable not available" << std::endl;
        std::exit(1);
    }
}

void process_init_cmd(const po::variables_map& options) {
    subprocess::run_cmd("git init && git config gc.pruneExpire never");
    std::cout << "gc.pruneExpire has already been set to `never`" << std::endl;
}

void process_write(const po::variables_map& options) {

}

using ProcessFuncT = void (*) (const po::variables_map& options);

int main(int argc, char *argv[])
{
    check_git_cmd_available();
    command::add_subcommand("init", "Initialize git repo.");

    const po::variables_map options = command::parse_args(argc, argv);

    std::map<std::string, ProcessFuncT> process_func_map = {
        {"init", process_init_cmd}
    };

    const auto& cmd = options["command"].as<std::string>();
    auto func = process_func_map.at(cmd);
    func(options);
    return 0;
}
