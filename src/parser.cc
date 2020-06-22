#include <argparse.h>
#include <cstring>

using namespace hgl;

void ArgumentParser::add_arg(Argument && a)
{
    this->args.insert(std::move(a));
}

void ArgumentParser::add_args(std::initializer_list<Argument> && l)
{
    for (auto && a : l)
        this->args.insert(std::move(a));
}

std::ostream & ArgumentParser::help(std:: ostream & os) const noexcept
{
    for (auto & a : this->args)
    {
        os << a.name << "\t--" << a.long_opt << "\t -" << a.short_opt << '\n';
    }

    return os;
}

void ArgumentParser::parse(int argc, const char * argv[])
{
    this->progname = argv[0];

    for (int i = 1; i < argc; i++)
    {
        const char * argstr = argv[i];

        if (argstr[0] == '\0')
            throw Error("unexpected end-of-string");

        if (argstr[0] == '-')
        {
            if (argstr[1] == '-')
            {
                if (argstr[2] != '\0') // long arg
                {
                    argstr += 2;
                    bool not_found = true;
                    for (auto & a : this->args)
                    {
                        if (std::strcmp(argstr, a.long_opt) == 0)
                        {
                            if (a.number == 1)
                                if (++i == argc)
                                    throw Error("not enugh input args");
                                else
                                    const_cast<Argument&>(a).set_value(argv[i]);
                            else if (a.number == 0)
                                const_cast<Argument&>(a).set_value("1");
                            else
                                throw Error("not implemented");
                            not_found = false;
                            break;
                        }
                    }
                    if (not_found)
                        throw Error("no such argument");
                }
                else // "--"
                {
                    while (i < argc)
                    {
                        this->rest_args.push_back(argv[++i]);
                    }
                }
            }
            else if (argstr[0] != '\0') // short arg
            {
                argstr += 1;
                char argch = *argstr;
                bool not_found = true;
                for (auto & a : this->args)
                {
                    if (argch == a.short_opt)
                    {
                        if (a.number == 1)
                            if (argstr[1] != '\0')
                                const_cast<Argument&>(a).set_value(argstr + 1);
                            else if (++i == argc)
                                throw Error("not enugh input args");
                            else
                                const_cast<Argument&>(a).set_value(argv[i]);
                        else if (a.number == 0)
                            if (argstr[1] == '\0')
                                const_cast<Argument&>(a).set_value("1");
                            else
                                const_cast<Argument&>(a).set_value(argstr + 1);
                        else
                            throw Error("not implemented");
                        not_found = false;
                        break;
                    }
                }
                if (not_found)
                    throw Error("no such argument");
            }
            else // "-"
            {
                this->rest_args.push_back(argstr);
            }
        }
        else
        {
            this->rest_args.push_back(argstr);
        }
    }
}

const ArgumentParser::Argument &
ArgumentParser::operator[](const char * name) const
{
    for (auto & a : this->args)
    {
        if (std::strcmp(a.name, name) == 0)
            return a;
    }
    throw Error("no such argument");
}
