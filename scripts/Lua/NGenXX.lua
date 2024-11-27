NGenXX = {}



NGenXX.Log = {}

NGenXX.Log.Level = {
    Debug = 3,
    Info = 4,
    Warn = 5,
    Error = 6,
    Fatal = 7,
    None = 8
}

function NGenXX.Log.print(_level, _content)
    inJson = JSON.stringify({
        level = _level,
        content = _content
    })
    ngenxx_log_printL(inJson)
end



NGenXX.DeviceInfo = {}

NGenXX.DeviceInfo.Platform = {
    Unknown = 0,
    Android = 1,
    ApplePhone = 2,
    ApplePad = 3,
    AppleMac = 4,
    AppleWatch = 5,
    AppleTV = 6,
    HarmonyOS = 7,
    Windows = 8,
    Linux = 9,
    Web = 10
}

NGenXX.DeviceInfo.CpuArch = {
    Unknown = 0,
    X86 = 1,
    X86_64 = 2,
    IA64 = 3,
    ARM = 4,
    ARM_6 = 5
}



NGenXX.Net = {}
NGenXX.Net.Http = {}

NGenXX.Net.Http.Method = {
    Get = 0,
    Post = 1,
    Put = 2
}

function NGenXX.Net.Http.request(_url, _method, _timeout)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Send Net HTTP Req from Lua..')
    inJson = JSON.stringify({
        url = _url,
        method = _method,
        timeout = _timeout
    })
    return ngenxx_net_http_requestL(inJson)
end



NGenXX.Coding = {}



NGenXX.Crypto = {}



NGenXX.Store = {}

NGenXX.Store.SQLite = {}



NGenXX.Store.KV = {}



NGenXX.Z = {}

NGenXX.Z.Format = {
    ZLib = 0,
    GZip = 1,
    Raw = 2
}

NGenXX.Z.ZipMode = {
    Default = -1,
    PreferSpeed = 1,
    PreferSize = 9
}


