#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <functional>

namespace Arguments {

using std::string;
using std::istringstream;
using std::unordered_map;
using std::vector;
using std::function;

class Parser
{
    using Option_Parser = function<void(const vector<string> &)>;

    struct Option
    {
        Option_Parser parser;
        int param_count = 0;
    };

public:
    class Error : public std::exception
    {
        string msg;
    public:
        Error() {}
        Error(const string & msg): msg(msg) {}
        const char * what() const noexcept override
        {
            return msg.c_str();
        }
    };

    struct Invalid_Option_Param : public Error
    {
        Invalid_Option_Param(const string & o, const string & v):
            Error("Invalid value '" + v + "' for option " + o) {}
    };

    struct Option_Param_Error {};

    template <typename T>
    static
    void parse_value(const string & text, T & value)
    {
        istringstream text_stream(text);
        text_stream >> value;
        if (!text_stream || text_stream.tellg() < text.size())
            throw Option_Param_Error();
    }

    template <typename T>
    void add_option(const string & name, T & destination, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 1;

        option.parser = [name, &destination](const vector<string> & values)
        {
            auto & value = values[0];
            try {
                parse_value(value, destination);
            }
            catch (Option_Param_Error &) {
                throw Invalid_Option_Param(name, value);
            }
        };
    }

    void add_option(const string & name, string & destination, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 1;

        option.parser = [name, &destination](const vector<string> & params)
        {
            destination = params[0];
        };
    }

    void add_switch(const string & name, bool & destination, bool enable = true, const string & description = string())
    {
        auto & option = options[name];

        option.param_count = 0;

        option.parser = [name, enable, &destination](const vector<string> & params)
        {
            destination = enable;
        };
    }

    void save_remaining(vector<string> & destination)
    {
        remaining_args = &destination;
    }

    void parse(int argc, char * argv[])
    {
        // Skip executable name
        --argc;
        ++argv;

        int i = 0;
        for(; i < argc; ++i)
        {
            string name = argv[i];

            if (name[0] != '-')
                break;

            auto option_it = options.find(name);
            if (option_it == options.end())
                throw Error("Invalid option: " + name);

            auto & option = option_it->second;

            vector<string> params;

            while(params.size() < option.param_count)
            {
                ++i;
                if (i >= argc)
                    break;
                string param = argv[i];
                if (param[0] == '-')
                    break;
                params.push_back(param);
            }

            if (params.size() < option.param_count)
            {
                std::ostringstream msg;
                msg << "Option " << name << " requires " << option.param_count
                    << " parameters, but " << params.size() << " given.";
                throw Error(msg.str());
            }

            option.parser(params);
        }

        if (remaining_args)
        {
            for(; i < argc; ++i)
            {
                remaining_args->push_back(argv[i]);
            }
        }
        else if (i < argc)
        {
            throw Error(string("Invalid argument: ") + argv[i]);
        }
    }

private:

    unordered_map<string, Option> options;
    vector<string> * remaining_args = nullptr;
};

}
