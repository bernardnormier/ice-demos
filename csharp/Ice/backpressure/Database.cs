//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using BackPressure;

public class Database : DataStoreDisp_
{
    public override void upload(byte[] data, Ice.Current current) =>
        Console.WriteLine($"Dispatching upload request with {data.Length} bytes");

    public override void sleep(int seconds, Ice.Current current)
    {
        Console.WriteLine($"Dispatching sleep request for {seconds} seconds");
        System.Threading.Thread.Sleep(TimeSpan.FromSeconds(seconds));
        Console.WriteLine("Sleep request completed");
    }
}
