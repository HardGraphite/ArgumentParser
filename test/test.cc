#include <argparse.h>

#include <iostream>

using namespace hgl;

struct HelpOption: FlagOption
{
    HelpOption(): FlagOption('h', "help", false, 0, "print help info and exit")
    {
        value = false;
    }

    virtual void parse_from_text(std::string_view text) override
    {
        throw *this;
    }
};

int main(int argc, char const *argv[])
{
    HelpOption o_help;
    FlagOption o_flag('f', "flag", false, -1, "flag option");
    IntOption o_int('l', "long", true, 1, "long value");
    FloatOption o_float('d', "double", true, 1, "double value");
    StringOption o_string('s', "string", true, 1, "string value");
    RestArguments rest;

    o_flag.set_default("on");

    ArgumentParser parser({&o_help, &o_flag, &o_int, &o_float, &o_string}, &rest);

    try
    {
        parser(argc, argv);
    }
    catch (const HelpOption &)
    {
        parser.print_help(std::cout);
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    std::ios::sync_with_stdio(false);

    std::cout
        << std::boolalpha << o_flag.value << '\n'
        << o_int.value << '\n'
        << o_float.value << '\n'
        << o_string.value << '\n'
    ;

    for (auto & a: rest)
        std::cout << a << '\t';
    std::cout << '\n';

    return 0;
}
