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
#include <list>

namespace hgl::ap
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

    /// arguments acceptor
    class ArgumentAcceptor
    {
    protected:
        bool completed         : 1;
        bool accepting_longopt : 1; ///< accepting arg with long option name
        bool accepting_shortopt: 1; ///< accepting arg with short option name
        bool accepting_restarg : 1; ///< accepting arg with no option name
        bool required          : 1;

        bool          _bit_0: 1;
        bool          _bit_1: 1;
        bool          _bit_2: 1;
        std::uint8_t  _u8;
        std::uint16_t _u16;

        /**
         * @brief test whether a long option name is acceptable
         *
         * @param long_opt option name
         * @return neg: not acceptable; pos or 0: number of arguments to consume
         *
         * @note check accepting_longopt before call this
         */
        virtual int acceptable(std::string_view long_opt) const noexcept;
        /**
         * @brief test whether a short option name is acceptable
         *
         * @param short_opt option name
         * @return neg: not acceptable; pos or 0: number of arguments to consume
         *
         * @note check accepting_shortopt before call this
         */
        virtual int acceptable(char short_opt) const noexcept;
        /**
         * @brief test whether no-option-name arg is acceptable
         *
         * @return neg: not acceptable; pos or 0: number of arguments to consume
         *
         * @note check accepting_restarg before call this
         */
        virtual int acceptable(std::nullptr_t) const noexcept;

        /**
         * @brief accept an option with out attached data
         */
        virtual void accept(std::nullptr_t);
        /**
         * @brief parse data from text and record the value
         *
         * @param text text from command line args (it can be empty)
         */
        virtual void accept(std::string_view text);
        /**
         * @brief parse data from a few text and record the value
         *
         * @param n number of arguments
         * @param text text from command line args
         *
         * @note if `n` == 0, accept(std::nullptr_t) will be called by default;
         *  if `n` == 1, accept(std::string_view text) will be called by default
         */
        virtual void accept(int n, const char ** text);
        /// @see void accept(std::nullptr_t)
        virtual void accept(std::string_view opt_name, std::nullptr_t);
        /// @see void accept(std::string_view text)
        virtual void accept(std::string_view opt_name, std::string_view text);
        /// @see void accept(int n, const char ** text)
        virtual void accept(std::string_view opt_name, int n, const char ** text);

        void mark_completed() noexcept;

        ArgumentAcceptor(
            bool completed, bool accepting_longopt,
            bool accepting_shortop, bool accepting_restarg,
            bool required,  std::uint8_t _u8 = 0, std::uint16_t _u16 = 0):
            completed(completed), accepting_longopt(accepting_longopt),
            accepting_shortopt(accepting_shortop), accepting_restarg(accepting_restarg),
            required(required), _u8(_u8), _u16(_u16) {}

        virtual void print_useage(std::ostream & out) const noexcept = 0;
        virtual void print_helpinfo(std::ostream & out) const noexcept = 0;

        friend class ArgumentParser;

    public:
        /**
         * @brief get accepter name
         *
         * @param[out] name accepter name
         */
        virtual void get_name(std::string & name) const noexcept = 0;
    };

    /// argument parser
    class ArgumentParser
    {
    private:
        struct ArgAcceptorArray
        {
            using value_type = ArgumentAcceptor * const;
            value_type * p_beg, * p_end;
            auto begin() const noexcept { return p_beg; }
            auto end() const noexcept { return p_end; }
        };

        std::string_view prog_name;
        ArgAcceptorArray acceptors;

        void chech_health();
        bool is_duplicated(int, std::string_view);

    public:
        ArgumentParser() = default;
        ArgumentParser(std::initializer_list<ArgumentAcceptor*> aas);
        ArgumentParser(ArgumentAcceptor * const * aa_begin, ArgumentAcceptor * const * aa_end);

        /**
         * @brief re-assign acceptors array
         *
         * @param begin frist elem of the array of acceptors
         * @param end the one after last elem of the array of acceptors
         */
        void set_acceptors(ArgumentAcceptor * const * begin, ArgumentAcceptor * const * end) noexcept;

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


    /// option
    class Option: public ArgumentAcceptor
    {
    protected:
        std::string_view _long_opt;
        const char     * help_info;

        auto & long_opt() noexcept { return _long_opt; }
        auto & short_opt() noexcept { return reinterpret_cast<char&>(_u8); }
        auto & n_args() noexcept { return _u16; }
        auto & long_opt() const noexcept { return _long_opt; }
        auto & short_opt() const noexcept { return reinterpret_cast<const char&>(_u8); }
        auto & n_args() const noexcept { return _u16; }

        virtual int acceptable(std::string_view long_opt) const noexcept override;
        virtual int acceptable(char short_opt) const noexcept override;
        virtual void accept(std::string_view opt_name, std::string_view text) override;
        virtual void accept(std::string_view opt_name, int n, const char ** text) override;
        using ArgumentAcceptor::accept;

    public:
        static constexpr char no_short_option = '\0';
        static constexpr std::string_view no_long_option = "";

        /**
         * @brief construct a new Option
         *
         * @param short_option short option name
         * @param long_option  long option name
         * @param required     whether the option must be provided
         * @param param_num    number of arguments this option consumes
         * @param help         help infomation
         *
         * @throw std::invalid_argument if any argument is invalid
         */
        Option(char short_option, std::string_view long_option,
            bool required = true, int param_num = 1, const char * help = nullptr);

        virtual void get_name(std::string & name) const noexcept override;
        virtual void print_useage(std::ostream & out) const noexcept override;
        virtual void print_helpinfo(std::ostream & out) const noexcept override;
    };

    /// text argument (no option name)
    class TextArg: public ArgumentAcceptor
    {
    protected:
        std::string_view name;

        virtual int acceptable(std::nullptr_t) const noexcept override;
        virtual void accept(std::string_view text) override;

    public:
        std::string_view text;

        TextArg(std::string_view name, bool required);

        virtual void get_name(std::string & name) const noexcept override;
        virtual void print_useage(std::ostream & out) const noexcept override;
        virtual void print_helpinfo(std::ostream & out) const noexcept override;
    };


    /// option that takes one value
    template <typename T> struct SignleValueOption: Option
    {
        using value_type = T;

        value_type value;

        SignleValueOption(char short_option, std::string_view long_option,
            bool required = true, const char * help = nullptr):
            Option(short_option, long_option, required, 1, help) {}
    };

    template <> struct SignleValueOption<bool>: Option
    {
        using value_type = bool;

        SignleValueOption(char short_option, std::string_view long_option,
            bool required = true, const char * help = nullptr):
            Option(short_option, long_option, required, 1, help) {}

        value_type value() const noexcept { return _bit_0; }
        void value(value_type v) noexcept { _bit_0 = v; }
    };

    struct FlagOption: SignleValueOption<bool>
    {
        FlagOption(char short_option, std::string_view long_option,
            bool required = true, const char * help = nullptr):
        SignleValueOption<bool>(short_option, long_option, required, help)
        { n_args() = 0; }

    protected:
        virtual int acceptable(std::string_view long_opt) const noexcept override;
        virtual void accept(std::string_view text, std::nullptr_t) override;
    };

    struct BoolOption: SignleValueOption<bool>
    {
        using SignleValueOption<bool>::SignleValueOption;

    protected:
        virtual void accept(std::string_view text) override;
    };

    struct IntOption: SignleValueOption<long>
    {
        using SignleValueOption<long>::SignleValueOption;

    protected:
        virtual void accept(std::string_view text) override;
    };

    struct FloatOption: SignleValueOption<double>
    {
        using SignleValueOption<double>::SignleValueOption;

    protected:
        virtual void accept(std::string_view text) override;
    };

    struct StringOption: SignleValueOption<std::string_view>
    {
        using SignleValueOption<std::string_view>::SignleValueOption;

    protected:
        virtual void accept(std::string_view text) override;
    };

    /// throw pointer to self when accepting
    class SpecialOption: public Option
    {
    private:
        virtual void accept(std::nullptr_t) override;

    public:
        SpecialOption(char short_option, std::string_view long_option,
            const char * help = nullptr):
            Option(short_option, long_option, false, 0, help) {}
    };

} // namespace hgl::ap


inline void hgl::ap::ArgumentAcceptor::mark_completed() noexcept
{
    completed = true;
    accepting_longopt = false;
    accepting_shortopt = false;
    accepting_restarg = false;
}

inline hgl::ap::ArgumentParser::ArgumentParser(
    std::initializer_list<ArgumentAcceptor*> aas):
    acceptors{.p_beg=aas.begin(), .p_end=aas.end()}
{
#ifndef NDEBUG
    this->chech_health();
#endif // NDEBUG
}

inline hgl::ap::ArgumentParser::ArgumentParser(
    ArgumentAcceptor * const * aa_begin, ArgumentAcceptor * const * aa_end):
    acceptors{.p_beg=aa_begin, .p_end=aa_end}
{
#ifndef NDEBUG
    this->chech_health();
#endif // NDEBUG
}

inline void hgl::ap::ArgumentParser::set_acceptors(
    ArgumentAcceptor * const * begin, ArgumentAcceptor * const * end) noexcept
{
    this->acceptors.p_beg = begin;
    this->acceptors.p_end = end;
}
