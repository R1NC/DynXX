NGenXX = {}

local InJsonVoid = ''

function NGenXX.version()
    return ngenxx_get_versionL()
end

function NGenXX.root()
    return ngenxx_root_pathL()
end



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



NGenXX.Device = {}

NGenXX.Device.Platform = {
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

NGenXX.Device.CpuArch = {
    Unknown = 0,
    X86 = 1,
    X86_64 = 2,
    IA64 = 3,
    ARM = 4,
    ARM_6 = 5
}

function NGenXX.Device.platform()
    return ngenxx_device_typeL(InJsonVoid)
end

function NGenXX.Device.name()
    return ngenxx_device_nameL(InJsonVoid)
end

function NGenXX.Device.manufacturer()
    return ngenxx_device_manufacturerL(InJsonVoid)
end

function NGenXX.Device.osVersion()
    return ngenxx_device_os_versionL(InJsonVoid)
end

function NGenXX.Device.cpuArch()
    return ngenxx_device_cpu_archL(InJsonVoid)
end



NGenXX.Net = {}
NGenXX.Net.Http = {}

NGenXX.Net.Http.Method = {
    Get = 0,
    Post = 1,
    Put = 2
}

function NGenXX.Net.Http.request(_url, _method, _timeout)
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


