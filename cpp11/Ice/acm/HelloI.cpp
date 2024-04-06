//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <HelloI.h>

#include <chrono>

using namespace std;

void
HelloI::sayHello(const Ice::Current&)
{
    cout << "sleeping for 30 seconds..." << endl;
    this_thread::sleep_for(chrono::seconds(30));
    cout << "Hello World!" << endl;
}
