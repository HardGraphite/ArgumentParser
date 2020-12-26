#include <argparse.h>
#include <stdexcept>

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <sstream>

using namespace hgl::ap;

int ArgumentAcceptor::acceptable(std::string_view long_opt) const noexcept
{
    return -1;
}

int ArgumentAcceptor::acceptable(char short_opt) const noexcept
{
    return -1;
}

int ArgumentAcceptor::acceptable(std::nullptr_t) const noexcept
{
    return -1;
}

[[noreturn]] static void
_throw_bad_accept(const ArgumentAcceptor * aa, int argn, const char ** args)
{
    std::stringstream ss;
    std::string name;

    aa->get_name(name);
    ss << "bad arguments for " << name << ':' << ' ';

    while (argn -- > 0)
        ss << *(args++);

    throw ArgumentParseError(ss.str());
}

void ArgumentAcceptor::accept(std::nullptr_t)
{
    _throw_bad_accept(this, 0, nullptr);
}

void ArgumentAcceptor::accept(std::string_view text)
{
    const char * arg[1] = {text.data()};
    _throw_bad_accept(this, 1, arg);
}

void ArgumentAcceptor::accept(int n, const char ** text)
{
    assert(n >= 0);

    if (n == 0)
        this->accept(nullptr);
    else if (n == 1)
        this->accept(*text);
    else
        _throw_bad_accept(this, n, text);
}

void ArgumentAcceptor::accept(std::string_view opt_name, std::nullptr_t)
{
    this->accept(nullptr);
}

void ArgumentAcceptor::accept(std::string_view opt_name, std::string_view text)
{
    this->accept(text);
}

void ArgumentAcceptor::accept(std::string_view opt_name, int n, const char ** text)
{
    this->accept(n, text);
}


int Option::acceptable(std::string_view long_opt) const noexcept
{
    assert(this->accepting_longopt);
    return long_opt == this->long_opt() ? this->n_args() : -1;
}

int Option::acceptable(char short_opt) const noexcept
{
    assert(this->accepting_shortopt);
    return short_opt == this->short_opt() ? this->n_args() : -1;
}

void Option::accept(std::string_view opt_name, std::string_view text)
{
    assert((opt_name.size() > 1 && opt_name == long_opt())
        || (!opt_name.empty() && opt_name.front() == short_opt()));
    this->accept(text);
}

void Option::accept(std::string_view opt_name, int n, const char ** text)
{
    assert((opt_name.size() > 1 && opt_name == long_opt())
        || (!opt_name.empty() && opt_name.front() == short_opt()));
    this->accept(n, text);
}

Option::Option(char short_option, std::string_view long_option,
    bool required, int param_num, const char * help):
    ArgumentAcceptor(!required, long_option != no_long_option,
        short_option != no_short_option, false, required, short_option, param_num),
    _long_opt(long_option), help_info(help == nullptr ? "" : help)
{
    assert(param_num == this->n_args());

    if (short_option == no_short_option && long_option == no_long_option)
        throw std::invalid_argument("neither short_option nor long_option is provided");
}

void Option::get_name(std::string & name) const noexcept
{
    name.clear();

    if (this->long_opt() != no_long_option)
    {
        name.reserve(this->long_opt().size());

        for (char ch : this->long_opt())
            name += (ch == '-') ? '_' : std::toupper(ch);
    }
    else if (this->short_opt() != no_short_option)
    {
        name = std::toupper(this->short_opt());
    }
    else
    {
        assert(false);
        name = "<?>";
    }
}


void Option::print_useage(std::ostream & out) const noexcept
{
    static std::string buffer; // not thread safe !!

    if (!this->required) out << '[';

    if (this->short_opt() != no_short_option)
        out << '-' << this->short_opt();
    else
        out << "--" << this->long_opt();

    if (this->n_args())
    {
        out << ' ';
        this->get_name(buffer);
            out << buffer;
        if (this->n_args() > 1)
            out << "...";
    }

    if (!this->required) out << ']';
}

void Option::print_helpinfo(std::ostream & out) const noexcept
{
    static std::string name, buffer; // not thread safe !!
    constexpr auto left_width = 25;

    this->get_name(name);
    buffer.clear();

    auto print_args = [&] {
        if (this->n_args() == 0)
            return;
        buffer += ' ';
        buffer += name;
        if (this->n_args() > 1)
            buffer += "...";
    };

    if (this->short_opt() != no_short_option)
    {
        buffer += '-';
        buffer += this->short_opt();
        print_args();
    }

    if (this->long_opt() != no_long_option)
    {
        if (this->short_opt() != no_short_option)
            buffer += ", ";

        buffer += "--";
        buffer += this->long_opt();
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

    out << ' ' << ' ' << this->help_info << '\n';
}


TextArg::TextArg(std::string_view name, bool required):
    ArgumentAcceptor(!required, false, false, true, required), name(name)
{
}

int TextArg::acceptable(std::nullptr_t) const noexcept
{
    return this->accepting_restarg ? 1 : -1;
}

void TextArg::accept(std::string_view text)
{
    this->text = text;

    this->mark_completed();
}

void TextArg::get_name(std::string & name) const noexcept
{
    name.clear();
    name.reserve(this->name.size());
    for (char ch: this->name)
        name += std::toupper(ch);
}

void TextArg::print_useage(std::ostream & out) const noexcept
{
    if (!this->required) out << '[';
    out << this->name;
    if (!this->required) out << ']';
}

void TextArg::print_helpinfo(std::ostream & out) const noexcept
{
}


int FlagOption::acceptable(std::string_view long_opt) const noexcept
{
    assert(this->accepting_longopt);

    using namespace std::literals::string_view_literals;

    const bool ok = long_opt == this->long_opt() || (
#ifdef __cpp_lib_starts_ends_with
        long_opt.startwith("no-")
#else
        long_opt.substr(0, 3) == "no-"sv
#endif
    && this->long_opt() == long_opt.substr(3));

    return ok ? this->n_args() : -1;
}

void FlagOption::accept(std::string_view text, std::nullptr_t)
{
    using namespace std::literals::string_view_literals;

    const bool no =
#ifdef __cpp_lib_starts_ends_with
        text.startwith("no-")
#else
        text.substr(0, 3) == "no-"sv
#endif
    &&
#ifdef __cpp_lib_starts_ends_with
        !this->long_opt().startwith("no-")
#else
        this->long_opt().substr(0, 3) != "no-"sv
#endif
    ;

    this->value(!no);
}

void BoolOption::accept(std::string_view text)
{
    if (text == "1" || text == "true" || text == "on" || text == "yes" || text.empty())
        this->value(true);
    else if (text == "0" || text == "false" || text == "off" || text == "no")
        this->value(false);
    else
    {
        std::string msg("not a valid bool literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }

    this->mark_completed();
}

void IntOption::accept(std::string_view text)
{
    auto str = text.data();
    char * end;

    this->value = std::strtol(str, &end, 0);

    if (this->value == 0 && str == end)
    {
        std::string msg("not a valid int literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }

    this->mark_completed();
}

void FloatOption::accept(std::string_view text)
{
    auto str = text.data();
    char * end;

    this->value = std::strtod(str, &end);

    if (this->value == 0 && str == end)
    {
        std::string msg("not a valid float literal: ");
        msg += text;
        throw ArgumentParseError(std::move(msg));
    }

    this->mark_completed();
}

void StringOption::accept(std::string_view text)
{
    this->value = text;

    this->mark_completed();
}

void SpecialOption::accept(std::nullptr_t)
{
    throw this;
}
