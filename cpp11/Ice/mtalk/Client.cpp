// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/UUID.h>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <MTalk.h>

using namespace std;

class ChatApp : public Ice::Application
{
public:

    ChatApp();
    virtual int run(int, char*[]);

    void discoveredPeer(string, shared_ptr<MTalk::PeerPrx>);
    void connect(string, Ice::Identity, shared_ptr<Ice::Connection>);
    void message(string);
    void disconnect(Ice::Identity, shared_ptr<Ice::Connection>, bool);
    void closed();

private:

    void doConnect(string);
    void doList();
    void doDisconnect();
    void doMessage(string);
    void failed(const Ice::LocalException&);
    void usage();

    string _name;
    shared_ptr<Ice::ObjectAdapter> _multicastAdapter;
    shared_ptr<Ice::ObjectAdapter> _peerAdapter;
    shared_ptr<MTalk::PeerPrx> _local;
    shared_ptr<MTalk::PeerPrx> _remote;
    map<string, shared_ptr<MTalk::PeerPrx>> _peers;
    std::mutex _mutex;
    std::condition_variable _condition;
};

//
// This servant listens for multicast messages.
//
class DiscoveryI : public MTalk::Discovery
{
public:

    DiscoveryI(ChatApp* app) :
        _app(app)
    {
    }

    virtual void announce(string name, shared_ptr<MTalk::PeerPrx> peer, const Ice::Current&)
    {
        _app->discoveredPeer(name, peer);
    }

private:

    ChatApp* _app;
};

//
// This servant listens for incoming connections from peers.
//
class IncomingPeerI : public MTalk::Peer
{
public:

    IncomingPeerI(ChatApp* app) :
        _app(app)
    {
    }

    virtual void connect(string name, Ice::Identity id, const Ice::Current& current)
    {
        _app->connect(name, id, current.con);
    }

    virtual void message(string text, const Ice::Current&)
    {
        _app->message(text);
    }

    virtual void disconnect(const Ice::Current& current)
    {
        _app->disconnect(current.id, current.con, true);
    }

private:

    ChatApp* _app;
};

//
// This servant handles an outgoing session with a peer.
//
class OutgoingPeerI : public MTalk::Peer
{
public:

    OutgoingPeerI(ChatApp* app) :
        _app(app)
    {
    }

    virtual void connect(string, Ice::Identity, const Ice::Current&)
    {
        throw MTalk::ConnectionException("already connected");
    }

    virtual void message(string text, const Ice::Current&)
    {
        _app->message(text);
    }

    virtual void disconnect(const Ice::Current& current)
    {
        _app->disconnect(current.id, current.con, false);
    }

private:

    ChatApp* _app;
};

//
// This thread periodically broadcasts a discover message.
//
class DiscoverThread
{
public:

    DiscoverThread(shared_ptr<MTalk::DiscoveryPrx> d, string n, shared_ptr<MTalk::PeerPrx> p) :
        _discovery(d), _name(n), _proxy(p), _destroy(false)
    {
    }

    void start()
    {
        thread t([this]()
            {
                this->run();
            });
        _thread = move(t);
    }

    void destroy()
    {
        {
            unique_lock<mutex> lock(_mutex);
            _destroy = true;
            _condition.notify_one();
        }

        if(_thread.joinable())
        {
            _thread.join();
        }
    }

    virtual void run()
    {
        while(true)
        {
            {
                unique_lock<mutex> lock(_mutex);
                _condition.wait_for(lock, chrono::seconds(2));

                if(_destroy)
                {
                    break;
                }

            }

            _discovery->announce(_name, _proxy);
        }
    }

private:

    shared_ptr<MTalk::DiscoveryPrx> _discovery;
    string _name;
    shared_ptr<MTalk::PeerPrx> _proxy;
    bool _destroy;
    std::mutex _mutex;
    std::condition_variable _condition;
    std::thread _thread;
};

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    ChatApp app;
    return app.main(argc, argv, "config");
}

ChatApp::ChatApp() :
    //
    // Since this is an interactive demo we don't want any signal handling.
    //
    Ice::Application(Ice::SignalPolicy::NoSignalHandling)
{
}

int
ChatApp::run(int argc, char*[])
{
    if(argc > 1)
    {
        cerr << appName() << ": too many arguments" << endl;
        return EXIT_FAILURE;
    }

    //
    // Create two object adapters. Their endpoints are defined in the configuration file 'config'.
    //
    _multicastAdapter = communicator()->createObjectAdapter("Multicast");
    _peerAdapter = communicator()->createObjectAdapter("Peer");

    //
    // Install a servant with the well-known identity "discover". This servant receives multicast messages.
    //
    _multicastAdapter->add(make_shared<DiscoveryI>(this), Ice::stringToIdentity("discover"));
    _multicastAdapter->activate();

    //
    // Install a servant with the well-known identity "peer".
    //
    _local = Ice::uncheckedCast<MTalk::PeerPrx>(
        _peerAdapter->add(make_shared<IncomingPeerI>(this), Ice::stringToIdentity("peer")));
    _peerAdapter->activate();

    while(_name.empty())
    {
        cout << ">>>> Enter your name: " << flush;
        getline(cin, _name);
    }

    //
    // Construct a datagram proxy that we'll use for multicast.
    //
    // Note that we need to disable collocation optimization because we also create an object adapter with
    // the same endpoint and we always want our discovery announcements to be broadcast on the network.
    //
    auto discovery = Ice::uncheckedCast<MTalk::DiscoveryPrx>(
        _communicator->propertyToProxy("Discovery.Proxy")->ice_datagram()->ice_collocationOptimized(false));

    auto thread = make_shared<DiscoverThread>(discovery, _name, _local);
    thread->start();

    usage();

    cout << ">>>> Ready." << endl;

    do
    {
        string s;
        cout << "";
        getline(cin, s);

        if(!s.empty())
        {
            if(s[0] == '/')
            {
                if(s.size() > 8 && s.substr(0, 8) == "/connect")
                {
                    doConnect(s);
                }
                else if(s == "/disconnect")
                {
                    doDisconnect();
                }
                else if(s == "/list")
                {
                    doList();
                }
                else if(s == "/quit")
                {
                    break;
                }
                else
                {
                    usage();
                }
            }
            else
            {
                doMessage(s);
            }
        }
    }
    while(cin.good());

    thread->destroy();

    //
    // There may still be objects (connections and servants) that hold pointers to this object, so we destroy
    // the communicator here to make sure they get cleaned up first.
    //
    communicator()->destroy();

    return EXIT_SUCCESS;
}

void
ChatApp::discoveredPeer(string name, shared_ptr<MTalk::PeerPrx> peer)
{
    unique_lock<mutex> lock(_mutex);

    //
    // We also receive multicast messages that we send, so ignore requests from ourself.
    //
    if(name == _name)
    {
        return;
    }

    auto p = _peers.find(name);
    if(p == _peers.end())
    {
        cout << endl << ">>>> Discovered peer " << name << endl;
        _peers[name] = peer;
    }
    else
    {
        p->second = peer;
    }
}

void
ChatApp::connect(string name, Ice::Identity id, shared_ptr<Ice::Connection> con)
{
    //
    // Called for a new incoming connection request.
    //

    unique_lock<mutex> lock(_mutex);

    if(_remote)
    {
        throw MTalk::ConnectionException("already connected");
    }

    //
    // Install a connection callback and enable ACM heartbeats.
    //
    con->setCloseCallback(
        [this](shared_ptr<Ice::Connection>)
        {
            this->closed();
        });
    con->setACM(30, Ice::ACMClose::CloseOff, Ice::ACMHeartbeat::HeartbeatAlways);

    _remote = Ice::uncheckedCast<MTalk::PeerPrx>(con->createProxy(id))->ice_invocationTimeout(5000);

    auto info = con->getInfo();
    if(info->underlying)
    {
        info = info->underlying;
    }
    auto ipInfo = dynamic_pointer_cast<Ice::IPConnectionInfo>(info);
    assert(ipInfo);
    cout << ">>>> Incoming connection from " << name << " with address " << ipInfo->remoteAddress << endl;
}

void
ChatApp::message(string text)
{
    unique_lock<mutex> lock(_mutex);

    if(_remote)
    {
        cout << "Peer says: " << text << endl;
    }
}

void
ChatApp::disconnect(Ice::Identity id, shared_ptr<Ice::Connection> con, bool incoming)
{
    unique_lock<mutex> lock(_mutex);

    if(_remote)
    {
        cout << ">>>> Peer disconnected" << endl;
        _remote = 0;
    }

    if(!incoming)
    {
        _peerAdapter->remove(id);
    }

    con->close(Ice::ConnectionClose::Gracefully);
}

void
ChatApp::closed()
{
    unique_lock<mutex> lock(_mutex);

    _remote = 0;

    cout << ">>>> Connection to peer closed" << endl;
}

void
ChatApp::doConnect(string cmd)
{
    auto sp = cmd.find(' ');
    if(sp == string::npos)
    {
        usage();
        return;
    }
    sp = cmd.find_first_not_of(' ', sp);
    if(sp == string::npos)
    {
        usage();
        return;
    }
    auto name = cmd.substr(sp);

    shared_ptr<MTalk::PeerPrx> remote;
    {
        unique_lock<mutex> lock(_mutex);

        if(_remote)
        {
            cout << ">>>> Already connected" << endl;
            return;
        }

        auto p = _peers.find(name);
        if(p == _peers.end())
        {
            cout << ">>>> No peer found matching `" << name << "'" << endl;
            return;
        }
        _remote = p->second;
        remote = _remote;
    }

    //
    // Generate a UUID for our callback servant. We have to pass this identity to
    // the remote peer so that it can invoke callbacks on the servant over a
    // bidirectional connection.
    //
    auto id = Ice::stringToIdentity(Ice::generateUUID());

    try
    {
        cout << ">>>> Connecting to " << name << endl;

        //
        // Configure an object adapter for the connection and add the servant. This enables
        // us to receive callbacks via this connection. Calling ice_getConnection() blocks
        // until the connection to the peer is established.
        //
        auto con = remote->ice_getConnection();
        con->setAdapter(_peerAdapter);
        _peerAdapter->add(make_shared<OutgoingPeerI>(this), id);

        //
        // Install a connection callback and enable ACM heartbeats.
        //
        con->setCloseCallback(
            [this](shared_ptr<Ice::Connection>)
            {
                this->closed();
            });
        con->setACM(30, Ice::ACMClose::CloseOff, Ice::ACMHeartbeat::HeartbeatAlways);

        //
        // Now we're ready to notify the peer that we'd like to connect.
        //
        remote->connect(_name, id);
        cout << ">>>> Connected to " << name << endl;
    }
    catch(const MTalk::ConnectionException& ex)
    {
        cout << ">>>> Connection failed: " << ex.reason << endl;
        try
        {
            _peerAdapter->remove(id);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        _remote = 0;
        return;
    }
    catch(const Ice::Exception& ex)
    {
        cout << ">>>> " << ex << endl;
        try
        {
            _peerAdapter->remove(id);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        _remote = 0;
        return;
    }
}

void
ChatApp::doList()
{
    unique_lock<mutex> lock(_mutex);

    if(_peers.empty())
    {
        cout << ">>>> No known peers" << endl;
    }
    else
    {
        for(auto p = _peers.begin(); p != _peers.end(); ++p)
        {
            cout << ">>>> " << p->first << endl;
        }
    }
}

void
ChatApp::doDisconnect()
{
    shared_ptr<MTalk::PeerPrx> peer;

    {
        unique_lock<mutex> lock(_mutex);

        if(!_remote)
        {
            cout << ">>>> Not connected" << endl;
            return;
        }

        peer = _remote;
        _remote = 0;
    }

    try
    {
        peer->disconnect();
    }
    catch(const Ice::LocalException&)
    {
    }
}

void
ChatApp::doMessage(string text)
{
    shared_ptr<MTalk::PeerPrx> peer;

    {
        unique_lock<mutex> lock(_mutex);

        if(!_remote)
        {
            cout << ">>>> Not connected" << endl;
            return;
        }

        peer = _remote;
    }

    try
    {
        peer->message(text);
    }
    catch(const Ice::LocalException& ex)
    {
        failed(ex);
    }
}

void
ChatApp::failed(const Ice::LocalException& ex)
{
    shared_ptr<MTalk::PeerPrx> peer;

    {
        unique_lock<mutex> lock(_mutex);
        peer = _remote;
        _remote = 0;
    }

    cout << ">>>> Action failed:" << endl << ex << endl;

    if(peer)
    {
        auto con = peer->ice_getCachedConnection();
        if(con)
        {
            con->close(Ice::ConnectionClose::Forcefully);
        }
    }
}

void
ChatApp::usage()
{
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  /connect <name>" << endl;
    cout << "  /disconnect" << endl;
    cout << "  /list" << endl;
    cout << "  /quit" << endl;
    cout << endl;
}
