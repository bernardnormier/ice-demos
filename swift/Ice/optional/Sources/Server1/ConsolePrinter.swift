// Copyright (c) ZeroC, Inc.

import Ice

/// ConsolePrinter is an Ice servant that implements Slice interface WeatherStation. It prints all information
/// reported by the sensors to the console.
class ConsolePrinter: WeatherStation {
    // Implements the protocol method `report`` from the WeatherStation protocol generated by the Slice compiler.
    func report(sensorId: String, reading: AtmosphericConditions?, current _: Ice.Current) {
        switch reading {
        case .none:
            print("\(sensorId) reported no reading to station v1")
        case .some(let conditions):
            print("\(sensorId) reported \(conditions) to station v1")
        }
    }
}
