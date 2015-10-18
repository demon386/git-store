// A thin wrapper around boost::program_options to support subcommand
#include <cstdlib>
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

namespace command {

namespace po = boost::program_options;

// This callback is for adding options for subcommands.
using CommandCallbackT = std::function<void(po::options_description&)>;
using CommandNameT = std::string;
using CommandDescT = std::string;
using CommandMapT = std::map<CommandNameT, std::tuple<CommandDescT, CommandCallbackT>>;

inline CommandMapT& global_command_map() {
    static CommandMapT map;
    return map;
}

void dummy_callback(po::options_description&) {
    // do nothing at all.
}

void add_subcommand(CommandNameT command_name,
        CommandDescT command_desc,
        CommandCallbackT callback) {
    auto& callback_map = global_command_map();
    callback_map[std::move(command_name)] = std::make_tuple(
            std::move(command_desc),
            std::move(callback));
}

void add_subcommand(CommandNameT command_name,
        CommandDescT command_desc) {
    add_subcommand(std::move(command_name),
            std::move(command_desc),
            dummy_callback);
}

po::variables_map parse_args(int argc, char* argv[]) {
    // Inspired by
    // http://www.hitmaroc.net/553491-4141-how-implement-subcommands-using-boost-program-options.html
    po::options_description commands("Subcommands: init,read,write");

    commands.add_options()
        ("help,h", "Help screen")
        ("command", po::value<std::string>(), "subcommand")
        ("subargs", po::value<std::vector<std::string>>(),
         "Positional arguments for subcommands");

    po::positional_options_description pos;
    pos.add("command", 1).
        add("subargs", -1);

    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(argc, argv).
        options(commands).
        positional(pos).
        allow_unregistered().
        run();

    po::store(parsed, vm);

    // @todo should exists a better way to judge whether the first positional
    // cmd is given.
    try {
        std::string cmd = vm["command"].as<std::string>();
    } catch(...) {
        std::cout << commands << std::endl;
        std::exit(1);
    }
    std::string cmd = vm["command"].as<std::string>();

    if (!global_command_map().count(cmd)) {
        std::cout << "Unrecognized command: " << cmd << std::endl;
        std::cout << commands << std::endl;
        std::exit(-1);
    } else {
        const auto& callback = std::get<1>(global_command_map().at(cmd));
        po::options_description sub_opts("sub-options for `" + cmd + "`");
        callback(sub_opts);
        sub_opts.add_options()
            ("help,h", "Help screen");

        std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
        opts.erase(opts.begin());

        try {
            po::store(po::command_line_parser(opts).options(sub_opts).run(), vm);
        } catch (boost::program_options::unknown_option& e) {
            std::cout << "Unknown options!" << std::endl;
            std::cout << sub_opts << std::endl;
            std::exit(1);
        }
        if (vm.count("help")) {
            std::cout << sub_opts << std::endl;
            std::exit(0);
        }
    }
    return vm;
}

} // namespace: command

