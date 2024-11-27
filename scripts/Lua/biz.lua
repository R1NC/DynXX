function TestNetHttpRequest(url)
    method = NGenXX.Net.Http.Method.Get
    timeout = 10 * 1000
    return NGenXX.Net.Http.request(url, method, timeout)
end

function TestDeviceInfo()
    deviceInfo = {
        platform = NGenXX.Device.platform(),
        manufacturer = NGenXX.Device.manufacturer(),
        name = NGenXX.Device.name(),
        osVersion = NGenXX.Device.osVersion(),
        cpuArch = NGenXX.Device.cpuArch()
    }
    NGenXX.Log.print(NGenXX.Log.Level.Debug, JSON.stringify(deviceInfo))
end
