#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import sys, threading, traceback, Ice

Ice.loadSlice('Session.ice')
import Demo

class HelloI(Demo.Hello):
    def __init__(self, name, id):
        self._name = name
        self._id = id

    def sayHello(self, c):
        print("Hello object #" + str(self._id) + " for session `" + self._name + "' says:\n" + \
              "Hello " + self._name + "!")


class SessionI(Demo.Session):
    def __init__(self, name):
        self._name = name
        self._lock = threading.Lock()
        self._destroy = False # true if destroy() was called, false otherwise.
        self._nextId = 0 # The id of the next hello object. This is used for tracing purposes.
        self._objs = [] # List of per-client allocated Hello objects.

        print("The session " + self._name + " is now created.")

    def createHello(self, current):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()

            hello = Demo.HelloPrx.uncheckedCast(current.adapter.addWithUUID(HelloI(self._name, self._nextId)))
            self._nextId = self._nextId + 1
            self._objs.append(hello)
            return hello
        finally:
            self._lock.release()

    def getName(self, current):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            return self._name
        finally:
            self._lock.release()

    def destroy(self, current):
        self._lock.acquire()
        try:
            if self._destroy:
                raise Ice.ObjectNotExistException()
            self._destroy = True
            print("The session " + self._name + " is now destroyed.")
            try:
                current.adapter.remove(current.id)
                for p in self._objs:
                    current.adapter.remove(p.ice_getIdentity())
            except Ice.ObjectAdapterDeactivatedException as ex:
                # This method is called on shutdown of the server, in
                # which case this exception is expected.
                pass
            self._objs = []
        finally:
            self._lock.release()


class SessionFactoryI(Demo.SessionFactory):
    def __init__(self):
        self._lock = threading.Lock()
        self._connectionMap = {}

    def create(self, name, current):

        class ClosedCallbackI(Ice.ConnectionCallback):
            def __init__(self, sessionFactory):
                self._sessionFactory = sessionFactory
                
            def heartbeat(self, con):
                pass # ignored

            def closed(self, con):
                self._sessionFactory.deadClient(con)


        session = SessionI(name)
        proxy = Demo.SessionPrx.uncheckedCast(current.adapter.addWithUUID(session))

        self._lock.acquire()
        try:
            #
            # With this demo, the connection cannot have an old session associated with it
            #
            assert current.con.toString() not in self._connectionMap
            self._connectionMap[current.con.toString()] = proxy
        finally:
            self._lock.release()

        #
        # Never close this connection from the client and turn on heartbeats with a timeout of 30s
        #
        current.con.setACM(30, Ice.ACMClose.CloseOff, Ice.ACMHeartbeat.HeartbeatAlways)
        current.con.setCallback(ClosedCallbackI(self))
        return proxy

    def shutdown(self, current):
        print("Shutting down...")
        current.adapter.getCommunicator().shutdown()

    def deadClient(self, con):
        session = None
        self._lock.acquire()
        try:
            session = self._connectionMap.pop(con.toString(), None)
        finally:
            self._lock.release()

        if(session != None):
            try:
                session.destroy()
                print("Cleaned up dead client.")
            except Ice.ObjectNotExistException as ex:
                # The client already destroyed this session
                pass
            except Ice.ConnectionRefusedException as ex:
                # during server shutdown
                pass

class Server(Ice.Application):
    def run(self, args):
        if len(args) > 1:
            print(self.appName() + ": too many arguments")
            return 1

        adapter = self.communicator().createObjectAdapter("SessionFactory")
        adapter.add(SessionFactoryI(), Ice.stringToIdentity("SessionFactory"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

app = Server()
sys.exit(app.main(sys.argv, "config.server"))
