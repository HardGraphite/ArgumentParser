#include <argparse.h>
#include <iostream>

using namespace hgl;

int main(int argc, char const *argv[])
{
    ArgumentParser ap {
        ArgumentParser::Argument("bool", "bool", 'b', 1, "0"),
        ArgumentParser::Argument("int", "int", 'i', 1, "0"),
        ArgumentParser::Argument("float", "float", 'f', 1, ""),
        ArgumentParser::Argument("string", "string", 's', 1, ""),
        ArgumentParser::Argument("list", "list", 'l', 1, ""),
        ArgumentParser::Argument("map", "list", 'm', 1, ""),
    };

    // ap.help(std::cout);
    ap.parse(argc, argv);

    std::cout << "bool: " << ap["bool"].as<bool>() << '\n';
    std::cout << "int: " << ap["int"].as<int>() << '\n';
    std::cout << "float: " << ap["float"].as<float>() << '\n';
    std::cout << "string: " << ap["string"].as<const char*>() << '\n';
    std::cout << '\n';

    std::cout << "list: ";
    for (auto & x : ap["list"].as_list<>())
        std::cout << x << ", ";
    std::cout << '\n' << '\n';

    std::cout << "map: ";
    for (auto & [k, v] : ap["map"].as_map<>())
        std::cout << k << " = " << v << ", ";
    std::cout << '\n' << '\n';

    std::cout << "rest: ";
    for (auto & x : ap.get_rest())
        std::cout << x << ", ";
    std::cout << '\n' << '\n';

    return 0;
}
