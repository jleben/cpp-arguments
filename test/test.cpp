#include "../arguments.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstring>

using namespace std;

struct FakeArgs
{
    FakeArgs(const vector<string> & args)
    {
        for (auto & arg : args)
            arg_ptr.push_back(strdup(arg.data()));
    }

    ~FakeArgs()
    {
        for (auto * p : arg_ptr)
            free(p);
    }

    int argc() { return arg_ptr.size(); }
    char** argv() { return arg_ptr.data(); }

    int count = 0;
    vector<char*> arg_ptr;
};

bool test_option_value()
{
    int a = 0;

    FakeArgs args({ "program", "a", "1" });

    Arguments::Parser parser;
    parser.add_option("a", a);
    parser.parse(args.argc(), args.argv());

    return a == 1;
}

bool test_option_missing_value()
{
    int a = 0;

    FakeArgs args({ "program", "a" });

    Arguments::Parser parser;
    parser.add_option("a", a);

    try { parser.parse(args.argc(), args.argv()); }
    catch (Arguments::Parser::Error &) { return true; }

    return false;
}

bool test_option_equals_value()
{
    int a = 0;

    FakeArgs args({ "program", "a=1" });

    Arguments::Parser parser;
    parser.add_option("a", a);
    parser.parse(args.argc(), args.argv());

    return a==1;
}

bool test_option_equals_missing_value()
{
    int a = 0;

    FakeArgs args({ "program", "a=" });

    Arguments::Parser parser;
    parser.add_option("a", a);

    try { parser.parse(args.argc(), args.argv()); }
    catch (Arguments::Parser::Error &) { return true; }

    return false;
}

bool test_multi_option_value()
{
    int a = 0;
    int b = 0;

    FakeArgs args({ "program", "b", "2", "a", "1" });

    Arguments::Parser parser;
    parser.add_option("a", a);
    parser.add_option("b", b);
    parser.parse(args.argc(), args.argv());

    return a == 1 and b == 2;
}

bool test_switch()
{
    bool a = false;

    FakeArgs args({ "program", "-a" });

    Arguments::Parser parser;
    parser.add_switch("-a", a);
    parser.parse(args.argc(), args.argv());

    return a == true;
}

bool test_remaining_arg()
{
    string remaining;

    FakeArgs args({ "program", "a" });

    Arguments::Parser parser;
    parser.remaining_argument(remaining);
    parser.parse(args.argc(), args.argv());

    return remaining == "a";
}

bool test_remaining_args()
{
    vector<string> remaining;

    FakeArgs args({ "program", "a", "b", "c", "d" });

    Arguments::Parser parser;
    parser.remaining_arguments(remaining);
    parser.parse(args.argc(), args.argv());

    return remaining == vector<string>({"a", "b", "c", "d"});
}

bool test_combination()
{
    string a;
    bool b = false;
    vector<string> r;

    Arguments::Parser parser;
    parser.add_option("-a", a);
    parser.add_switch("-b", b);
    parser.remaining_arguments(r);

    FakeArgs args({ "program", "A", "-a", "xyz", "B", "-b", "C" });

    parser.parse(args.argc(), args.argv());

    bool ok =
            a == "xyz" and
            b == true and
            r == vector<string>({ "A", "B", "C" });

    return ok;
}

template <typename F>
void run(const string & name, F f)
{
    bool ok = false;
    try
    {
        ok = f();
    }
    catch(std::exception &e)
    {
        cerr << name << ": Failed: Exception: " << e.what() << endl;
        return;
    }

    if (ok)
        cerr << name << ": OK." << endl;
    else
        cerr << name << ": Failed." << endl;
}

int main()
{
    run("option value", test_option_value);
    run("option (missing)", test_option_missing_value);
    run("option=value", test_option_equals_value);
    run("option=(missing)", test_option_equals_missing_value);
    run("multi option value", test_multi_option_value);
    run("switch", test_switch);
    run("remaining arg", test_remaining_arg);
    run("remaining args", test_remaining_args);
    run("combination", test_combination);
}
