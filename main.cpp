#include <iostream>
#include <command.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
    command::add_subcommand("ls", "ls balabalal",
            [](po::options_description& sub_opts) {
            sub_opts.add_options()
            ("hidden", "hide file");
            });
    command::parse_args(argc, argv);
    return 0;
}
