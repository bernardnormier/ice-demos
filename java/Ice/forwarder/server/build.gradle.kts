// Copyright (c) ZeroC, Inc.

dependencies {
    // Add the Ice library as an implementation dependency.
    implementation("com.zeroc:ice:3.8.0-+")
}

sourceSets {
    main {
        // Add the Greeter.ice file from the parent slice directory to the main source set.
        slice {
            srcDirs("../slice")
        }
    }
}

application {
    // Specify the main entry point for the application.
    mainClass.set("com.example.ice.forwarder.server.Server")
    applicationDefaultJvmArgs = listOf("-ea") // Enables assertions
}
