#pragma once

#include <exception>
#include <initializer_list>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace hgl
{
    // argument parser
    class ArgumentParser
    {
    public:
        // parser error
        class Error: public std::exception
        {
        private:
            std::string msg;

        public:
            Error(std::string && msg): msg(std::move(msg)) {}
            virtual const char * what() const noexcept {return msg.c_str();}
        };

        // argument
        class Argument
        {
        public:
            static constexpr char   NoShortArg = '\0';
            static constexpr char * NoLongArg  = nullptr;
            static constexpr char * NoDefault   = nullptr;

        protected:
            bool         nonset;
            std::string  value;

            template <typename _T> static _T convert(const std::string &);

            // "a,b,c" ==> ["a", "b", "c"]
            static std::vector<std::string> convert_to_list(const std::string &);
            // "a=A,b=B,c=C" => {"a": A, "b": B, "c": C}
            static std::map<std::string, std::string> convert_to_map(const std::string &);

        public:
            const char    *  name;
            const char    *  long_opt;  // e.g. "argument"
            const char       short_opt; // e.g. 'a'
            const unsigned char number;

            Argument(const char * name,
                const char * long_opt = NoLongArg, char short_opt = NoShortArg,
                unsigned char number = 1, const char * default_value = NoDefault);

            // get value
            template <typename _T> _T as() const
                {return convert<_T>(this->value);}

            // get value as list
            template <typename _T = const char *> std::vector<_T> as_list() const
            {
                auto str_vec = convert_to_list(this->value);
                std::vector<_T> res;
                res.reserve(str_vec.size());
                for (auto & v : str_vec)
                    res.push_back(convert<_T>(v));
                return res;
            }

            // get value as map
            template <typename _T = const char *> std::map<std::string, _T> as_map() const
            {
                auto str_map = convert_to_map(this->value);
                std::map<std::string, _T> res;
                for (auto & [k, v] : str_map)
                    res[k] = convert<_T>(v);
                return res;
            }

            Argument & set_value(const std::string & s) noexcept;
            Argument & set_value(const char * s) noexcept;

            bool operator==(const Argument & a) const noexcept;
            bool operator<(const Argument & a) const;
        };

    protected:
        std::set<Argument> args;
        std::vector<const char *> rest_args;
        const char *       progname;

    public:
        ArgumentParser() {}

        ArgumentParser(std::initializer_list<Argument> && args)
            {this->add_args(std::move(args));}

        // add an argument
        void add_arg(Argument && a);
        // add arguments
        void add_args(std::initializer_list<Argument> && l);

        // print help message
        std::ostream & help(std:: ostream & os) const noexcept;

        // parse arguments
        void parse(int argc, const char * argv[]);
        // parse arguments
        void operator()(int argc, const char * argv[])
            {this->parse(argc, argv);}

        // get parsed result
        const Argument & operator[](const char * name) const;
        // get
        const std::vector<const char*>& get_rest() const noexcept
            {return this->rest_args;}

    };

    // template <typename _T> _T ArgumentParser::Argument::convert(const std::string &)
    //     {std::enable_if<false>::type;}
    template <> inline bool ArgumentParser::Argument::convert(const std::string & s)
        {return std::atoi(s.c_str()) != 0;}
    template <> inline int ArgumentParser::Argument::convert(const std::string & s)
        {return std::atoi(s.c_str());}
    template <> inline long ArgumentParser::Argument::convert(const std::string & s)
        {return std::atol(s.c_str());}
    template <> inline float ArgumentParser::Argument::convert(const std::string & s)
        {return std::atof(s.c_str());}
    template <> inline double ArgumentParser::Argument::convert(const std::string & s)
        {return std::atof(s.c_str());}
    template <> inline const char * ArgumentParser::Argument::convert(const std::string & s)
        {return s.c_str();}
    template <> inline std::string ArgumentParser::Argument::convert(const std::string & s)
        {return std::string(s);}
    template <> inline const std::string & ArgumentParser::Argument::convert(const std::string & s)
        {return s;}
} // namespace hgl
