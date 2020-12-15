/**
 * @file argparse.h
 * @brief HGL command line argument parser
 */

#pragma once

#include <exception>
#include <initializer_list>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#ifndef HGLAP_NO_EXTENSION
#include <type_traits>
#endif // HGLAP_NO_EXTENSION

namespace hgl
{
    /// parse error
    class ArgumentParseError: public std::exception
    {
    private:
        std::string msg;

    public:
        ArgumentParseError() = default;
        ArgumentParseError(const char * msg): msg(msg) { }
        ArgumentParseError(const std::string & msg): msg(msg) { }
        ArgumentParseError(std::string && msg): msg(std::move(msg)) { }

        const char * what() const noexcept override { return msg.c_str(); }
    };

    /// option wrapper, base of any option types
    class OptionWrapper
    {
    protected:
        std::string_view long_opt;
        const char     * help_info;
        char             short_opt;
        unsigned char    n_args;
        unsigned char    args_given;
        bool             is_required: 1;
        bool             has_default: 1;
        bool             takes_0_or_1_arg: 1;

        /**
         * @brief parse data from text and record the value
         * 
         * @param text text from command line args
         *  (if this option takes no arguemtn i.e. n_args==0, text will be empty)
         */
        virtual void parse_from_text(std::string_view text) = 0;

        friend class ArgumentParser;

    public:
        static constexpr char no_short_option = '\0';
        static constexpr std::string_view no_long_option = "";

        /**
         * @brief construct a new OptionWrapper
         * 
         * @param short_option short option name
         * @param long_option  long option name
         * @param required     whether the option must be provided
         * @param param_num    number of arguments this option consumes
         *  (specially, `-2` means more than 1, `-1` means 0 or 1)
         * @param help         help infomation
         * 
         * @throw std::invalid_argument if any argument is invalid
         */
        OptionWrapper(char short_option, std::string_view long_option,
            bool required = true, int param_num = 1, const char * help = nullptr);

        /**
         * @brief set default value
         * 
         * @param _default default value
         * @return ref to self
         */
        OptionWrapper & set_default(std::string_view _default) noexcept;

        /**
         * @brief get option name
         * 
         * @param [OUT] name option name
         */
        virtual void get_name(std::string & name) const noexcept;

        /**
         * @brief print help infomation
         * 
         * @param out output stream
         */
        virtual void print_help(std::ostream & out) const noexcept;
    };

    /// rest aruments
    class RestArgsWrapper
    {
    public:
        virtual void append(std::string_view text) = 0;
    };

    /// argument parser
    class ArgumentParser
    {
    private:
        std::string_view            prog_name;
        std::vector<OptionWrapper*> options;
        RestArgsWrapper           * rest_args;

        void chech_options();

    public:
        ArgumentParser() = default;
        ArgumentParser(std::initializer_list<OptionWrapper*> opts);
        ArgumentParser(std::initializer_list<OptionWrapper*> opts, RestArgsWrapper * rest);

        /**
         * @brief parse command line arguments
         * 
         * @param argc number of command line arguments
         * @param argv command line argument vector
         * 
         * @throw ArgumentParseError if error occurs
         */
        void operator()(int argc, const char * argv[]);

        /**
         * @brief print help infomation
         * 
         * @param out output stream
         */
        std::ostream & print_help(std::ostream & out) const noexcept;
    };

#ifndef HGLAP_NO_EXTENSION

    struct FlagOption: OptionWrapper
    {
        bool value;

        using OptionWrapper::OptionWrapper;

        virtual void parse_from_text(std::string_view text) override;
    };

    struct IntOption: OptionWrapper
    {
        long value;

        using OptionWrapper::OptionWrapper;

        virtual void parse_from_text(std::string_view text) override;
    };

    struct FloatOption: OptionWrapper
    {
        double value;

        using OptionWrapper::OptionWrapper;

        virtual void parse_from_text(std::string_view text) override;
    };

    struct StringOption: OptionWrapper
    {
        std::string_view value;

        using OptionWrapper::OptionWrapper;

        virtual void parse_from_text(std::string_view text) override;
    };

    template <typename SVAlloc = std::allocator<std::string_view>>
    struct RestArguments: RestArgsWrapper, std::vector<std::string_view, SVAlloc>
    {
        virtual void append(std::string_view text) override;
    };

#endif // HGLAP_NO_EXTENSION

} // namespace hgl


inline hgl::OptionWrapper &
hgl::OptionWrapper::set_default(std::string_view _default) noexcept
{
    this->parse_from_text(_default);
    this->has_default = true;
    return *this;
}

inline
hgl::ArgumentParser::ArgumentParser(std::initializer_list<OptionWrapper*> opts):
    options(opts), rest_args(nullptr)
{
#ifndef NDEBUG
    this->chech_options();
#endif // NDEBUG
}

inline
hgl::ArgumentParser::ArgumentParser(std::initializer_list<OptionWrapper*> opts,
    RestArgsWrapper * rest): options(opts), rest_args(rest)
{
#ifndef NDEBUG
    this->chech_options();
#endif // NDEBUG
}

#ifndef HGLAP_NO_EXTENSION

template <typename SVAlloc>
inline void hgl::RestArguments<SVAlloc>::append(std::string_view text)
{
    this->emplace_back(text);
};

#endif // HGLAP_NO_EXTENSION
