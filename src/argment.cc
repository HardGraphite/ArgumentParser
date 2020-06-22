#include <argparse.h>
#include <cstring>

using namespace hgl;

ArgumentParser::Argument::Argument(const char * name,
        const char * long_opt, char short_opt,
        unsigned char number, const char * default_value):
    nonset(true), name(name), long_opt(long_opt), short_opt(short_opt), number(number)
{
    if (short_opt == NoShortArg && long_opt == NoLongArg)
        throw Error("this argument has neither long name nor short name");

    if (default_value != NoDefault)
    {
        this->value = default_value;
        this->nonset = false;
    }
}

std::vector<std::string>
ArgumentParser::Argument::convert_to_list(const std::string & s)
{
    std::vector<std::string> res;

    if (s.empty())
        return res;

    std::size_t pos_beg = 0;
    std::size_t pos_end = s.find_first_of(',', 0);

    while (pos_end != std::string::npos)
    {
        res.emplace_back(s, pos_beg, pos_end - pos_beg);
        pos_beg = pos_end + 1;
        pos_end = s.find_first_of(',', pos_beg);
    }
    res.emplace_back(s, pos_beg);

    return res;
}

std::map<std::string, std::string>
ArgumentParser::Argument::convert_to_map(const std::string & s)
{
    std::map<std::string, std::string> res;

    if (s.empty())
        return res;

    std::size_t pos_beg = 0;
    std::size_t pos_end = s.find_first_of(',', 0);

    auto add_pair = [&]
    {
        auto pos_eq = s.find_first_of('=', pos_beg);
        if (pos_eq == std::string::npos || pos_eq > pos_end)
            res[s.substr(pos_beg, pos_end - pos_beg)] = "";
        else
            res[s.substr(pos_beg, pos_eq - pos_beg)]
                = s.substr(pos_eq + 1, pos_end - pos_eq - 1);
    };

    while (pos_end != std::string::npos)
    {
        add_pair();
        pos_beg = pos_end + 1;
        pos_end = s.find_first_of(',', pos_beg);
    }
    add_pair();

    return res;
}

ArgumentParser::Argument &
ArgumentParser::Argument::set_value(const std::string & s) noexcept
{
    this->value = s;
    this->nonset = false;
    return *this;
}

ArgumentParser::Argument &
ArgumentParser::Argument::set_value(const char * s) noexcept
{
    this->value = s;
    this->nonset = false;
    return *this;
}

bool ArgumentParser::Argument::operator==(const Argument & a) const noexcept
{
#if false
    return (this->short_opt == a.short_opt)
        && ((this->long_opt == NoLongArg) ?
            (a.long_opt == NoLongArg) :
            (a.long_opt == NoLongArg ?
                true : std::strcmp(this->long_opt, a.long_opt) == 0));
#else
    return std::strcmp(this->name, a.name) == 0;
#endif
}

bool ArgumentParser::Argument::operator<(const Argument & a) const
{
#if false
    if (this->short_opt != NoShortArg && a.short_opt != NoShortArg)
        return this->short_opt < a.short_opt;
    else if (this->long_opt != NoLongArg && a.long_opt != NoLongArg)
        return std::strcmp(this->long_opt, a.long_opt) < 0;
    else
        throw Error("this argument has neither long name nor short name");
#else
    return std::strcmp(this->name, a.name) < 0;
#endif
}
