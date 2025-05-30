# Ice Context

The Context demo shows the 3 different ways to set a request context in a client.

A request context is a `Dictionary<String, String>` carried by all requests. It is empty by default, and the application is free to set any entry in this dictionary.

> [!NOTE]
> The Ice protocol does not support response contexts, only request contexts.

You can build the demo by running:

```shell
swift build
```

First, start the server:

```shell
.build/debug/Server
```

In a separate terminal, start the client:

```shell
.build/debug/Client
```