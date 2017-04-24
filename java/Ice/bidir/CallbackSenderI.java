// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import Demo.*;
import java.util.concurrent.*;

class CallbackSenderI implements CallbackSender
{
    @Override
    synchronized public void addClient(com.zeroc.Ice.Identity ident, com.zeroc.Ice.Current current)
    {
        System.out.println("adding client `" + com.zeroc.Ice.Util.identityToString(ident) + "'");

        com.zeroc.Ice.ObjectPrx base = current.con.createProxy(ident);
        CallbackReceiverPrx client = CallbackReceiverPrx.uncheckedCast(base);
        _clients.add(client);
    }

    public void destroy()
    {
        System.out.println("destroying callback sender");
        _executorService.shutdown();
        try
        {
            _executorService.awaitTermination(10, TimeUnit.SECONDS);
        }
        catch(InterruptedException interrupted)
        {
            // ignored
        }
    }

    public void start()
    {
        _executorService.scheduleAtFixedRate(() -> invokeCallback(), 2, 2, TimeUnit.SECONDS);
    }

    synchronized private void invokeCallback()
    {
        if(!_clients.isEmpty())
        {
            _num++;

            //
            // Invoke callback on all clients; it's safe to do it within the synchronization
            // because Ice guarantees these async invocations never block the calling thread.
            //
            // The exception callback, if called asynchronously, is called by a thread from
            // the Ice client thread pool, and never the calling thread.
            // We use whenCompleteAsync instead of whenComplete to ensure that if the
            // call completes immediately we delay the removeClient until after the iteration.
            //
            for(CallbackReceiverPrx p : _clients)
            {
                p.callbackAsync(_num).whenCompleteAsync(
                    (r, t) ->
                    {
                        if(t != null)
                        {
                            removeClient(p, t);
                        }
                    },
                    _executorService);
            }
        }
    }

    synchronized private void removeClient(CallbackReceiverPrx p, Throwable t)
    {
        System.err.println("removing client `" +
                           com.zeroc.Ice.Util.identityToString(p.ice_getIdentity()) +
                           "':");
        t.printStackTrace();
        _clients.remove(p);
    }

    private ScheduledExecutorService _executorService = Executors.newSingleThreadScheduledExecutor();
    private java.util.List<CallbackReceiverPrx> _clients = new java.util.ArrayList<>();
    private int _num = 0;
}
