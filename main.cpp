#include <iostream>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>
#include "command.hpp"
#include "subprocess.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

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

void process_write_cmd(const po::variables_map& options) {
    if (!options.count("subargs")) {
        return;
    }
    auto subargs = options["subargs"].as<std::vector<std::string>>();
    if (subargs.size() == 0) {
        return;
    }
    auto subargs_str = boost::algorithm::join(subargs, " ");
    int ret = 0;
    ret = subprocess::run_cmd("git reset --mixed");
    if (ret != 0) {
        return;
    }

    if (subargs.size() == 1 && !fs::is_directory(subargs_str)
            && !options.count("tree")) {
        std::cout << "hashing blob object" << std::endl;
        ret = subprocess::run_cmd("git hash-object -w " + subargs_str);
    } else {
        std::cout << "hashing tree" << std::endl;
        ret = subprocess::run_cmd("git add " + subargs_str);
        if (ret != 0) {
            return;
        }
        ret = subprocess::run_cmd("git write-tree && git reset --mixed");
    }
    if (ret == 0) {
        if (options.count("delete")) {
            // @todo: check file is inside git repo.
            // to avoid accidently delete files under / or things like that.
            subprocess::run_cmd("rm -rf " + subargs_str);
        }
    }
}

void process_read_cmd(const po::variables_map& options) {
    if (!options.count("subargs")) {
        return;
    }
    auto subargs = options["subargs"].as<std::vector<std::string>>();
    if (subargs.size() != 1) {
        std::cout << "should given exactly one SHA-1" << std::endl;
    }
    subprocess::run_cmd("git reset --mixed");
    int ret = subprocess::run_cmd("git read-tree " + subargs.at(0));
    if (ret == 0) {
        subprocess::run_cmd("git checkout-index -a");
        subprocess::run_cmd("git reset --mixed");
    } else {
        std::cerr << "Trying to find blob object with this SHA-1" << std::endl;
        subprocess::run_cmd("git show " + subargs.at(0));
    }
}

using ProcessFuncT = void (*) (const po::variables_map& options);

int main(int argc, char *argv[])
{
    check_git_cmd_available();
    command::add_subcommand("init", "Initialize git repo.");
    command::add_subcommand("write", "Write content into git repo.",
            [](po::options_description& subopts) {
            subopts.add_options()
            ("tree,t", "give tree's hash even given just one file")
            ("delete,d", "delete added files");
            });
    command::add_subcommand("read", "read content into current dir.");

    const po::variables_map options = command::parse_args(argc, argv);

    std::map<std::string, ProcessFuncT> process_func_map = {
        {"init", process_init_cmd},
        {"write", process_write_cmd},
        {"read", process_read_cmd}
    };

    const auto& cmd = options["command"].as<std::string>();
    auto func = process_func_map.at(cmd);
    func(options);
    return 0;
}
