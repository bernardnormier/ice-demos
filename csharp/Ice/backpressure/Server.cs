//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class Server
{
    public static int Main(string[] args)
    {
        try
        {
            using(var communicator = Ice.Util.initialize(ref args))
            {
                // Shut down the communicator on Ctrl+C or Ctrl+Break.
                Console.CancelKeyPress += (sender, eventArgs) =>
                {
                    eventArgs.Cancel = true;
                    communicator.shutdown();
                };

                var adapter = communicator.createObjectAdapterWithEndpoints("BackPressure", "default -h 127.0.0.1 -p 4061");
                adapter.add(new Database(), Ice.Util.stringToIdentity("datastore"));
                adapter.activate();
                communicator.waitForShutdown();
            }
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }
        return 0;
    }
}
