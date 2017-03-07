// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <NestedI.h>

using namespace std;
using namespace Ice;
using namespace Demo;

NestedI::NestedI(shared_ptr<NestedPrx> self) :
    _self(move(self))
{
}

void
NestedI::nestedCall(Int level, shared_ptr<NestedPrx> proxy, const Ice::Current&)
{
    cout << level << endl;
    if(--level > 0)
    {
        //
        // Ensure the invocation times out if the nesting level is too
        // high and there are no more threads in the thread pool to
        // dispatch the call.
        //
        proxy->ice_invocationTimeout(5000)->nestedCall(level, _self);
    }
}
