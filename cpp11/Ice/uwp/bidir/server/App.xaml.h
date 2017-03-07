﻿// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

#include "App.g.h"

namespace bidir
{

ref class App sealed
{
public:
    App();
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^) override;
};

}
