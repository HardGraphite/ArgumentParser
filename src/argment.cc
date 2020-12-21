#include <argparse.h>
#include <stdexcept>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <limits>

using namespace hgl::ap;

OptionWrapper::OptionWrapper(char short_option, std::string_view long_option,
    bool required, int param_num, const char * help):
    long_opt(long_option), help_info(help == nullptr ? "" : help),
    short_opt(short_option), n_args(param_num), args_given(0),
    is_required(required), has_default(false), takes_0_or_1_arg(false)
{
    if (param_num < 0)
    {
        if (param_num == -1)
        {
            n_args = 1;
            takes_0_or_1_arg = true;
        }
        else
            n_args = std::numeric_limits<decltype(n_args)>::max();
    }

    if (short_option == no_short_option && long_option == no_long_option)
        throw std::invalid_argument("neither short_option nor long_option is provided");
}

void OptionWrapper::get_name(std::string & name) const noexcept
{
    name.clear();

    if (this->long_opt != no_long_option)
    {
        name.reserve(this->long_opt.size());

        for (char ch : this->long_opt)
            name += (ch == '-') ? '_' : std::toupper(ch);
    }
    else if (this->short_opt != no_short_option)
    {
        name = std::toupper(this->short_opt);
    }
    else
    {
        assert(false);
        name = "<?>";
    }
}

void OptionWrapper::print_help(std::ostream & out) const noexcept
{
    static std::string name, buffer; // not thread safe !!
    constexpr auto left_width = 20;

    this->get_name(name);
    buffer.clear();

    auto print_args = [&] {
        if (this->n_args == 0)
            return;
        buffer += ' ';
        buffer += name;
        if (this->n_args > 1)
            buffer += "...";
    };

    if (this->short_opt != no_short_option)
    {
        buffer += '-';
        buffer += this->short_opt;
        print_args();
    }

    if (this->long_opt != no_long_option)
    {
        if (this->short_opt != no_short_option)
            buffer += ", ";

        buffer += "--";
        buffer += this->long_opt;
        print_args();
    }


    if (this->help_info == nullptr)
    {
        out << buffer;
        return;
    }

    auto pos_diff = buffer.length();
    if (pos_diff > left_width)
    {
        out << buffer << '\n';
        for (int i = 0; i < left_width; i++)
            out << ' ';
    }
    else
    {
        out << std::setfill(' ') << std::setw(left_width) << std::left << buffer;
    }

    out << ' ' << ' ' << this->help_info;
}


void FlagOption::parse_from_text(std::string_view text)
{
    if (text == "1" || text == "true" || text == "on" || text == "yes" || text.empty())
        this->value = true;
    else if (text == "0" || text == "false" || text == "off" || text == "no")
        this->value = false;
    else
    {
        std::string msg("no a valid bool literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }
}

void IntOption::parse_from_text(std::string_view text)
{
    auto str = text.data();
    char * end;

    this->value = std::strtol(str, &end, 0);

    if (this->value == 0 && str == end)
    {
        std::string msg("no a valid int literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }
}

void FloatOption::parse_from_text(std::string_view text)
{
    auto str = text.data();
    char * end;

    this->value = std::strtod(str, &end);

    if (this->value == 0 && str == end)
    {
        std::string msg("no a valid float literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }
}

void StringOption::parse_from_text(std::string_view text)
{
    this->value = text;
}
