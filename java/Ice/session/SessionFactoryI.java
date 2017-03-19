// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

import Demo.*;

class SessionFactoryI implements SessionFactory
{
    @Override
    public SessionPrx create(String name, com.zeroc.Ice.Current current)
    {
        SessionI session = new SessionI(name);
        SessionPrx proxy = SessionPrx.uncheckedCast(current.adapter.addWithUUID(session));

        synchronized(this)
        {
            //
            // With this demo, the connection cannot have an old session associated with it
            //
            assert(_connectionMap.get(current.con) == null);
            _connectionMap.put(current.con, proxy);
        }

        //
        // Never close this connection from the client and turn on heartbeats with a timeout of 30s
        //
        current.con.setACM(java.util.OptionalInt.of(30),
                           java.util.Optional.of(com.zeroc.Ice.ACMClose.CloseOff),
                           java.util.Optional.of(com.zeroc.Ice.ACMHeartbeat.HeartbeatAlways));
        current.con.setCloseCallback(con -> deadClient(con));

        return proxy;
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }

    void deadClient(com.zeroc.Ice.Connection con)
    {
        SessionPrx session = null;
        synchronized(this)
        {
            session = _connectionMap.remove(con);
        }

        if(session != null)
        {
            try
            {
                session.destroy();
                System.out.println("Cleaned up dead client.");
            }
            catch(com.zeroc.Ice.ObjectNotExistException ex)
            {
                // The client already destroyed this session
            }
            catch(com.zeroc.Ice.ConnectionRefusedException ex)
            {
                // Server is shutting down
            }
        }
    }

    private java.util.Map<com.zeroc.Ice.Connection, SessionPrx> _connectionMap = new java.util.HashMap<>();
}
