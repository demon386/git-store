#include <iostream>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include "command.hpp"
#include "subprocess.hpp"

namespace po = boost::program_options;

void check_git_cmd_available() {
    if (subprocess::run_cmd("which git", false) != 0) {
        std::cout << "git executable not available" << std::endl;
        std::exit(1);
    }
}

std::string join_str_vector(const std::vector<std::string> v, const char* delim) {
    std::ostringstream joined;
    std::copy(v.begin(), v.end(), std::ostream_iterator<std::string>(joined, delim));
    return joined.str();
}

void process_init_cmd(const po::variables_map& options) {
    subprocess::run_cmd("git init && git config gc.pruneExpire never");
    std::cout << "gc.pruneExpire has already been set to `never`" << std::endl;
}

bool is_directory(const std::string& path) {
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void process_write_cmd(const po::variables_map& options) {
    if (!options.count("subargs")) {
        return;
    }
    auto subargs = options["subargs"].as<std::vector<std::string>>();
    if (subargs.size() == 0) {
        return;
    }
    auto subargs_str = join_str_vector(subargs, " ");
    subprocess::run_cmd("git reset --mixed");

    int ret = 0;
    if (subargs.size() == 1 && !is_directory(subargs_str) && !options.count("tree")) {
        ret = subprocess::run_cmd("git hash-object " + subargs_str);
    } else {
        ret = subprocess::run_cmd("git update-index --add " + subargs_str);
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

    const po::variables_map options = command::parse_args(argc, argv);

    std::map<std::string, ProcessFuncT> process_func_map = {
        {"init", process_init_cmd},
        {"write", process_write_cmd}
    };

    const auto& cmd = options["command"].as<std::string>();
    auto func = process_func_map.at(cmd);
    func(options);
    return 0;
}
