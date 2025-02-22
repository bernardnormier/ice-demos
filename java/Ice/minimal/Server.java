// Copyright (c) ZeroC, Inc.

import com.zeroc.Ice.*;

public class Server {
    public static void main(String[] args) {
        var mainThread = Thread.currentThread();
        var shutdownThread =
            new Thread(
                () -> {
                // Interrupt and join the main thread. The shutdown thread must wait until the cleanup
                // is complete.
                mainThread.interrupt();
                try {
                    mainThread.join();
                } catch (InterruptedException e) {
                    assert false : "Shutdown thread cannot be interrupted";
                }
            });
        Runtime.getRuntime().addShutdownHook(shutdownThread);

        try (Communicator communicator = Util.initialize(args)) {
            ObjectAdapter adapter =
                communicator.createObjectAdapterWithEndpoints("Hello", "default -h localhost -p 10000");
            adapter.add(new Printer(), Util.stringToIdentity("hello"));
            adapter.activate();

            // Wait for Ctrl+C
            try {
                communicator.waitForShutdown();
            } catch (OperationInterruptedException e) {
                System.out.print("Caught Ctrl+C, ");
            }
            System.out.println("exiting...");
        }
    }
}
