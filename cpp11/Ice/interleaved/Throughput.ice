// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#pragma once

module Demo
{

sequence<byte> ByteSeq;
const int ByteSeqSize = 500000;

interface Throughput
{
    ["marshaled-result", "cpp:array"] ByteSeq echoByteSeq(["cpp:array"] ByteSeq seq);

    void shutdown();
};

};
