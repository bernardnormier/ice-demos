// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <ThroughputI.h>

Demo::Throughput::EchoByteSeqMarshaledResult
ThroughputI::echoByteSeq(std::pair<const Ice::Byte*, const Ice::Byte*> seq, const Ice::Current& current)
{
    return EchoByteSeqMarshaledResult(seq, current);
}

void
ThroughputI::shutdown(const Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
