// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

module Demo
{

interface DiscoverReply
{
    void reply(Object* obj);
};

interface Discover
{
    void lookup(DiscoverReply* replyHere);
};

};

