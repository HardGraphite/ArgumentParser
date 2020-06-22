#include <argparse.h>
#include <iostream>

using namespace hgl;

int main(int argc, char const *argv[])
{
    ArgumentParser::Argument a1("a1", "Alpha");
    ArgumentParser::Argument a2("a2", "Beta");

    a1.set_value("1,2,3,4");
    a2.set_value("a=1,b=2,c=3");

    for (auto & x : a1.as_list<std::string>())
        std::cout << x << std::endl;

    for (auto & [k, v] : a2.as_map<std::string>())
        std::cout << k << " = " << v << std::endl;

    return 0;
}
