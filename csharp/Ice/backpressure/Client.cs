//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using BackPressure;

public class Client
{
    private const int UploadCount = 400_000;

    public static async Task<int> Main(string[] args)
    {
        try
        {
            using var communicator = Ice.Util.initialize(ref args);

            var dataStore = DataStorePrxHelper.uncheckedCast(
                communicator.stringToProxy("datastore:tcp -h localhost -p 4061"));

            var dataStoreOneway = DataStorePrxHelper.uncheckedCast(
                dataStore.ice_oneway());

            Task sleepTask = dataStore.sleepAsync(20);

            var data = new byte[10 * 1024];

            for (int i = 0; i < UploadCount; ++i)
            {
                await dataStoreOneway.uploadAsync(data);
                Console.WriteLine($"Upload #{i}");
            }

            Console.WriteLine("Waiting for sleep task to complete");
            await sleepTask;

            return 0;
        }
        catch(Exception ex)
        {
            Console.Error.WriteLine(ex);
            return 1;
        }
    }
}
