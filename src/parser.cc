#include <argparse.h>

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

[[noreturn]] static void _throw_0a_req_qa_given(
    const char * text, const OptionWrapper * opt)
{
    std::string buffer;
    opt->get_name(buffer);
    _throw_parse_error("\"%s\": option %s consumes 0 argument "
        "but 1 is given", text, buffer.c_str());
}

[[noreturn]] static void _throw_not_restargs_recv(const char * text)
{
    _throw_parse_error("\"%s\": unexpected rest-arguments", text);
}

[[noreturn]] static void _throw_unknown_opt(const char * text, std::string_view opt)
{
    std::string buffer(opt);
    _throw_parse_error("\"%s\": unknown option: %s", text, buffer.c_str());
}

[[noreturn]] static void _throw_too_many_args(const char * text, std::string_view opt)
{
    std::string buffer(opt);
    _throw_parse_error("\"%s\": too may arguments for option %s", text, buffer.c_str());
}

void ArgumentParser::chech_options()
{
    for (OptionWrapper * opt: this->options)
    {
        if (opt->short_opt != opt->no_short_option)
        {
            for (OptionWrapper * opt1: this->options)
            {
                if (opt1->short_opt == opt1->no_short_option || opt1 == opt)
                    continue;

                if (opt1->short_opt == opt->short_opt)
                    _throw_std_invalid_argument("more than one options have %s name '%c'", "short", opt->short_opt);
            }
        }


        if (opt->long_opt != opt->no_long_option)
        {
            for (OptionWrapper * opt1: this->options)
            {
                if (opt1->long_opt == opt1->no_long_option || opt1 == opt)
                    continue;

                if (opt1->long_opt == opt->long_opt)
                    _throw_std_invalid_argument("more than one options have %s name \"%s\"", "long", opt->long_opt);
            }
        }
    }
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

    for (; iter < iter_end; ++iter)
    {
        cur_opt = *iter;

        if (cur_opt == "--"sv)
        {
            if (this->rest_args == nullptr)
                _throw_not_restargs_recv(*iter);

            for (++iter; iter != iter_end; ++iter)
                this->rest_args->append(*iter);
        }
        else if (cur_opt == "-"sv)
        {
            if (this->rest_args == nullptr)
                _throw_not_restargs_recv(*iter);

            this->rest_args->append(cur_opt);
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

            for (OptionWrapper * opt: this->options)
            {
                if (opt->long_opt == opt->no_long_option
                 || opt->long_opt != cur_opt)
                    continue;

                if (opt->args_given >= opt->n_args
                 && !(opt->takes_0_or_1_arg && opt->args_given <= 1))
                    _throw_too_many_args(*iter, cur_opt);

                if (equal_pos != cur_opt.npos)
                {
                    if (opt->n_args == 0)
                        _throw_0a_req_qa_given(*iter, opt);

                    opt->parse_from_text(value);
                    opt->args_given++;
                }

                if (opt->n_args == 0)
                {
                    opt->parse_from_text(""sv);
                    goto _NEXT_LOOP;
                }

                for (++iter;
                    opt->args_given < opt->n_args && iter != iter_end && **iter != '-';
                    opt->args_given++, ++iter)
                {
                    opt->parse_from_text(*iter);
                }
                --iter;

                goto _NEXT_LOOP;
            }

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

            for (OptionWrapper * opt: this->options)
            {
                if (opt->short_opt == opt->no_short_option
                 || opt->short_opt != cur_opt.front())
                    continue;

                if (opt->args_given >= opt->n_args && opt->args_given > 0
                 && !(opt->takes_0_or_1_arg && opt->args_given <= 1))
                    _throw_too_many_args(*iter, cur_opt);

                if (has_value)
                {
                    if (opt->n_args == 0)
                        _throw_0a_req_qa_given(*iter, opt);

                    opt->parse_from_text(value);
                    opt->args_given++;
                }

                if (opt->n_args == 0)
                {
                    opt->parse_from_text(""sv);
                    goto _NEXT_LOOP;
                }

                for (++iter;
                    opt->args_given < opt->n_args && iter != iter_end && **iter != '-';
                    opt->args_given++, ++iter)
                {
                    opt->parse_from_text(*iter);
                }
                --iter;

                goto _NEXT_LOOP;
            }

            _throw_unknown_opt(*iter, cur_opt);
        }
        else
        {
            if (this->rest_args == nullptr)
                _throw_not_restargs_recv(*iter);

            this->rest_args->append(cur_opt);
        }

    _NEXT_LOOP:;
    }

    for (OptionWrapper * opt: this->options)
    {
        if (!(opt->args_given >= opt->n_args
         || !opt->is_required
         || opt->has_default
         || (opt->n_args == 1 && opt->takes_0_or_1_arg)))
        {
            std::string name;
            opt->get_name(name);
            _throw_parse_error("option %s takes %i arg(s) but %i were given",
                name.c_str(), opt->n_args, opt->args_given);
        }
    }
}


std::ostream & ArgumentParser::print_help(std::ostream & out) const noexcept
{
    out << "Usage: " << this->prog_name << " [OPTIONS]";
    if (this->rest_args != nullptr)
        out << " ...";
    out << '\n' << '\n';

    out << "Options:\n";
    for (OptionWrapper * opt: this->options)
    {
        opt->print_help(out);
        out << '\n';
    }

    return out;
}
