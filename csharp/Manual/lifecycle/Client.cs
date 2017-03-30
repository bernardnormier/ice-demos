// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using Filesystem;
using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceLifecycleClient")]
[assembly: AssemblyDescription("Ice lifecycle demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public static int Main(string[] args)
    {
        try
        {
            //
            // The new communicator is automatically destroyed (disposed) at the end of the
            // using statement
            //
            using(var communicator = Ice.Util.initialize(ref args))
            {
                //
                // Create a proxy for the root directory
                //
                var @base = communicator.stringToProxy("RootDir:default -h localhost -p 10000");

                //
                // Down-cast the proxy to a Directory proxy.
                //
                var rootDir = DirectoryPrxHelper.checkedCast(@base);
                if(rootDir == null)
                {
                    throw new Error("Invalid proxy");
                }

                var p = new Parser(rootDir);
                return p.parse();
            }
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }
    }

    private static class Error : SystemException
    {
        public Error(String msg)
            : base(msg)
        {
        }
    }
}
