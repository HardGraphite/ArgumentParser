#include <argparse.h>

#include <iostream>

using namespace hgl::ap;

int main(int argc, char const *argv[])
{
    SpecialOption o_help('h', "help", "print help and exit");
    FlagOption o_flag(FloatOption::no_short_option, "flag", false, "flag option");
    BoolOption o_bool('b', "bool", false, "bool option");
    IntOption o_int('i', "int", true, "long value");
    FloatOption o_float(FloatOption::no_short_option, "float", true, "double value");
    StringOption o_string('s', "string", true, "string value");
    TextArg a_rest("name", false);

    ArgumentParser parser({&o_help, &o_flag, &o_bool, &o_int, &o_float, &o_string, &a_rest});

    try
    {
        parser(argc, argv);
    }
    catch (const SpecialOption * p)
    {
        if (p == &o_help)
        {
            parser.print_help(std::cout);
            return 0;
        }

        throw;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    std::ios::sync_with_stdio(false);

    std::cout
        << std::boolalpha << o_flag.value() << '\n'
        << std::boolalpha << o_bool.value() << '\n'
        << o_int.value << '\n'
        << o_float.value << '\n'
        << o_string.value << '\n'
        << a_rest.text << '\n'
    ;

    return 0;
}
