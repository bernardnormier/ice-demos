// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include "stdafx.h"
#include <HelloI.h>

using namespace std;

HelloI::HelloI(const LogIPtr& log, CDialog* dialog) :
    _log(log), _dialog(dialog)
{
}

void
HelloI::sayHello(int delay, const Ice::Current&)
{
    if(delay != 0)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(delay));
    }
    _log->message("Hello World!");
}

void
HelloI::shutdown(const Ice::Current&)
{
    if(_dialog)
    {
        _log->message("Shutting down...");
        _dialog->EndDialog(0);
        _dialog = 0;
    }
}
