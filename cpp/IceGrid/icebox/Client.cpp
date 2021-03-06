// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Hello.h>

using namespace std;
using namespace Demo;

class HelloClient : public Ice::Application
{
public:

    virtual int run(int, char*[]);
};

int
#ifdef _WIN32
wmain(int argc, wchar_t* argv[])
#else
main(int argc, char* argv[])
#endif
{
    HelloClient app;
    return app.main(argc, argv, "config.client");
}


int
HelloClient::run(int argc, char* argv[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    HelloPrx hello = HelloPrx::uncheckedCast(communicator()->propertyToProxy("Hello.Proxy"));
    if(!hello)
    {
        cerr << argv[0] << ": invalid or missing Hello.Proxy property" << endl;
        return EXIT_FAILURE;
    }
    
    hello->sayHello();
    
    return EXIT_SUCCESS;
}
