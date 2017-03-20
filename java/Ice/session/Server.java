// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

public class Server extends Ice.Application
{
    @Override
    public int run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("SessionFactory");

        adapter.add(new SessionFactoryI(), Ice.Util.stringToIdentity("SessionFactory"));
        adapter.activate();
        communicator().waitForShutdown();

        return 0;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int status = app.main("Server", args, "config.server");
        System.exit(status);
    }
}
