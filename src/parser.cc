#include <argparse.h>

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>

using namespace hgl::ap;
using namespace std::literals::string_view_literals;

static constexpr auto _strbuf_size = 256;
static thread_local char _strbug[_strbuf_size];

[[noreturn]] static void _throw_parse_error(const char * fmt, ...)
{
    std::va_list ap;

    va_start(ap, fmt);
    std::vsnprintf(_strbug, _strbuf_size, fmt, ap);
    va_end(ap);

    throw ArgumentParseError(_strbug);
}

[[noreturn]] static void _throw_std_invalid_argument(const char * fmt, ...)
{
    std::va_list ap;

    va_start(ap, fmt);
    std::vsnprintf(_strbug, _strbuf_size, fmt, ap);
    va_end(ap);

    throw std::invalid_argument(_strbug);
}

[[noreturn]] static void _throw_0a_req_1a_given(
    const char * text, const ArgumentAcceptor * opt)
{
    std::string buffer;
    opt->get_name(buffer);
    _throw_parse_error("\"%s\": option %s consumes 0 argument "
        "but 1 is given", text, buffer.c_str());
}

[[noreturn]] static void _throw_na_req_1a_given(
    const char * text, int req_n, const ArgumentAcceptor * opt)
{
    std::string buffer;
    opt->get_name(buffer);
    _throw_parse_error("\"%s\": option %s consumes %i argument "
        "but 1 is given", text, buffer.c_str(), req_n);
}

[[noreturn]] static void _throw_unknown_opt(const char * text, std::string_view opt)
{
    std::string buffer(opt);
    _throw_parse_error("\"%s\": unknown option: %s", text, buffer.c_str());
}

[[noreturn]] static void _throw_duplicated_opt(const char * text, std::string_view opt)
{
    std::string buffer(opt);
    _throw_parse_error("\"%s\": duplicated option: %s", text, buffer.c_str());
}

[[noreturn]] static void _throw_unexpected_arg(const char * text)
{
    _throw_parse_error("\"%s\": unexpected argument", text);
}

[[noreturn]] static void _throw_too_many_args(const ArgumentAcceptor * aa)
{
    std::string name;
    aa->get_name(name);
    _throw_parse_error("too may arguments for option %s", name.c_str());
}

[[noreturn]] static void _throw_too_few_args(const ArgumentAcceptor * aa)
{
    std::string name;
    aa->get_name(name);
    _throw_parse_error("too few arguments for option %s", name.c_str());
}

void ArgumentParser::chech_health()
{
}

bool ArgumentParser::is_duplicated(int type, std::string_view optname)
{
    if (type == 1)
    {
        for (ArgumentAcceptor * acceptor: acceptors)
        {
            const bool fc = acceptor->completed, fa = acceptor->accepting_shortopt;
            acceptor->completed = false, acceptor->accepting_shortopt = true;
            const bool r = acceptor->acceptable(optname.front()) >= 0;
            acceptor->completed = fc, acceptor->accepting_shortopt = fa;
            if (r) return true;
        }
    }
    else if (type == 2)
    {
        for (ArgumentAcceptor * acceptor: acceptors)
        {
            const bool fc = acceptor->completed, fa = acceptor->accepting_longopt;
            acceptor->completed = false, acceptor->accepting_longopt = true;
            const bool r = acceptor->acceptable(optname) >= 0;
            acceptor->completed = fc, acceptor->accepting_longopt = fa;
            if (r) return true;
        }
    }
    else
    {
        assert(false);
    }

    return false;
}

void ArgumentParser::operator()(int argc, const char * argv[])
{
    using arg_t = const char *;

    if (argc == 0)
        return;

    {
        this->prog_name = argv[0];
        const auto slash_pos = this->prog_name.find('/');
        if (slash_pos != this->prog_name.npos)
            this->prog_name.remove_prefix(slash_pos + 1);
    }

    arg_t * iter = argv + 1;
    const arg_t * iter_end = argv + argc;
    std::string_view cur_opt, value;

    auto accept_args = [&] (ArgumentAcceptor * acceptor, int n_args) {
        assert(n_args >= 1);

        const char ** args_begin = iter;

        --iter;
        for (int i = 0; i < n_args; i++)
        {
            ++iter;

            if (iter == iter_end || **iter == '-')
                _throw_too_few_args(acceptor);
        }

        acceptor->accept(cur_opt, n_args, args_begin);
    };

    for (; iter < iter_end; ++iter)
    {
        cur_opt = *iter;

        if (cur_opt == "--"sv)
        {
            for (++iter; iter != iter_end; ++iter)
            {
                for (ArgumentAcceptor * acceptor: this->acceptors)
                {
                    if (!acceptor->accepting_restarg)
                        continue;

                    auto n = acceptor->acceptable(nullptr);
                    assert(n > 0);

                    accept_args(acceptor, n);

                    goto _NEXT_LOOP;
                }
            }
        }
        else if (cur_opt == "-"sv)
        {
            for (ArgumentAcceptor * acceptor: this->acceptors)
            {
                if (!(acceptor->accepting_restarg && acceptor->acceptable(nullptr) == 1))
                    continue;

                acceptor->accept(cur_opt, cur_opt);

                goto _NEXT_LOOP;
            }

            _throw_unexpected_arg(*iter);
        }
#ifdef __cpp_lib_starts_ends_with
        else if (cur_opt.starts_with("--"sv))
#else
        else if (cur_opt.substr(0, 2) == "--"sv)
#endif
        {
            cur_opt.remove_prefix(2);

            const auto equal_pos = cur_opt.find('=');
            if (equal_pos != cur_opt.npos) // e.g. "xxx=yyy"
            {
                value = cur_opt.substr(equal_pos + 1); // "yyy"
                cur_opt.remove_suffix(value.size() + 1); // "xxx"
            }

            for (ArgumentAcceptor * acceptor: this->acceptors)
            {
                if (!acceptor->accepting_longopt)
                    continue;

                const auto n = acceptor->acceptable(cur_opt);
                if (n < 0)
                    continue;

                if (n == 0)
                {
                    if (equal_pos != cur_opt.npos)
                        _throw_0a_req_1a_given(*iter, acceptor);

                    acceptor->accept(cur_opt, nullptr);
                }
                else
                {
                    if (equal_pos != cur_opt.npos)
                    {
                        if (n != 1)
                            _throw_na_req_1a_given(*iter, n, acceptor);

                        acceptor->accept(cur_opt, value);
                    }
                    else
                    {
                        ++iter;
                        accept_args(acceptor, n);
                    }
                }

                goto _NEXT_LOOP;
            }

            this->is_duplicated(2, cur_opt) ?
                _throw_duplicated_opt(*iter, cur_opt):
                _throw_unknown_opt(*iter, cur_opt);
        }
#ifdef __cpp_lib_starts_ends_with
        else if (cur_opt.starts_with('-'))
#else
        else if (!cur_opt.empty() && cur_opt.front() == '-')
#endif
        {
            cur_opt.remove_prefix(1);

            const bool has_value = cur_opt.length() > 1;
            if (has_value) // e.g. "-fZZZ"
            {
                value = cur_opt.substr(1); // "ZZZ"
                cur_opt.remove_suffix(value.size()); // "f"
            }

            for (ArgumentAcceptor * acceptor: this->acceptors)
            {
                if (!acceptor->accepting_shortopt)
                    continue;

                const auto n = acceptor->acceptable(cur_opt.front());
                if (n < 0)
                    continue;

                if (n == 0)
                {
                    if (has_value)
                        _throw_0a_req_1a_given(*iter, acceptor);

                    acceptor->accept(cur_opt, nullptr);
                }
                else
                {
                    if (has_value)
                    {
                        if (n != 1)
                            _throw_na_req_1a_given(*iter, n, acceptor);

                        acceptor->accept(cur_opt, value);
                    }
                    else
                    {
                        ++iter;
                        accept_args(acceptor, n);
                    }
                }

                goto _NEXT_LOOP;
            }

            this->is_duplicated(1, cur_opt) ?
                _throw_duplicated_opt(*iter, cur_opt):
                _throw_unknown_opt(*iter, cur_opt);
        }
        else
        {
            for (ArgumentAcceptor * acceptor: this->acceptors)
            {
                if (!acceptor->accepting_restarg)
                    continue;

                const auto n = acceptor->acceptable(nullptr);
                assert(n > 0);

                accept_args(acceptor, n);

                goto _NEXT_LOOP;
            }

            _throw_unexpected_arg(*iter);
        }

    _NEXT_LOOP:;
    }

    for (ArgumentAcceptor * acceptor: this->acceptors)
    {
        if (!acceptor->completed)
        {
            std::string name;
            acceptor->get_name(name);
            _throw_parse_error("no enough arguments for %s", name.c_str());
        }
    }
}


std::ostream & ArgumentParser::print_help(std::ostream & out) const noexcept
{
    out << "Usage: " << this->prog_name << ' ';
    for (ArgumentAcceptor * acceptor: this->acceptors)
    {
        acceptor->print_useage(out);
        out << ' ';
    }
    out << '\n' << '\n';

    out << "Options:\n";
    for (ArgumentAcceptor * acceptor: this->acceptors)
        acceptor->print_helpinfo(out);

    return out;
}
