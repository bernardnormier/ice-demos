//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

module BackPressure
{
    sequence<byte> ByteSeq;

    interface DataStore
    {
        void upload(ByteSeq data);
        void sleep(int seconds);
    }
}
