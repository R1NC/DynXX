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
    local inJson = JSON.stringify({
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
    local inJson = JSON.stringify({
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

function NGenXX.Store.SQLite.open(id)
    local inJson = JSON.stringify({
        _id = id
    })
    return ngenxx_store_sqlite_openL(inJson)
end

function NGenXX.Store.SQLite.execute(_conn, _sql)
    local inJson = JSON.stringify({
        conn = _conn,
        sql = _sql
    })
    return ngenxx_store_sqlite_executeL(inJson)
end

NGenXX.Store.SQLite.Query = {}

function NGenXX.Store.SQLite.Query.create(_conn, _sql)
    local inJson = JSON.stringify({
        conn = _conn,
        sql = _sql
    })
    return ngenxx_store_sqlite_query_doL(inJson)
end

function NGenXX.Store.SQLite.Query.readRow(_query_result)
    local inJson = JSON.stringify({
        query_result = _query_result
    })
    return ngenxx_store_sqlite_query_read_rowL(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnText(_query_result, _column)
    local inJson = JSON.stringify({
        query_result = _query_result,
        column = _column
    })
    return ngenxx_store_sqlite_query_read_column_textL(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnInteger(_query_result, _column)
    local inJson = JSON.stringify({
        query_result = _query_result,
        column = _column
    })
    return ngenxx_store_sqlite_query_read_column_integerL(inJson)
end

function NGenXX.Store.SQLite.Query.readColumnFloat(_query_result, _column)
    local inJson = JSON.stringify({
        query_result = _query_result,
        column = _column
    })
    return ngenxx_store_sqlite_query_read_column_floatL(inJson)
end

function NGenXX.Store.SQLite.query.drop(_query_result)
    local inJson = JSON.stringify({
        query_result = _query_result
    })
    ngenxx_store_sqlite_query_dropL(inJson)
end

function NGenXX.Store.SQLite.close(_conn)
    local inJson = JSON.stringify({
        conn = _conn
    })
    ngenxx_store_sqlite_closeL(inJson)
end



NGenXX.Store.KV = {}

function NGenXX.Store.KV.open(id)
    local inJson = JSON.stringify({
        _id = id
    })
    return ngenxx_store_kv_openL(inJson)
end

function NGenXX.Store.KV.readString(_conn, _k)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k
    })
    return ngenxx_store_kv_read_stringL(inJson)
end

function NGenXX.Store.KV.writeString(_conn, _k, s)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k,
        v = s
    })
    return ngenxx_store_kv_write_stringL(inJson)
end

function NGenXX.Store.KV.readInteger(_conn, _k)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k
    })
    return ngenxx_store_kv_read_integerL(inJson)
end

function NGenXX.Store.KV.writeInteger(_conn, _k, i)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k,
        v = i
    })
    return ngenxx_store_kv_write_integerL(inJson)
end

function NGenXX.Store.KV.readFloat(_conn, _k)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k
    })
    return ngenxx_store_kv_read_floatL(inJson)
end

function NGenXX.Store.KV.writeFloat(_conn, _k, f)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k,
        v = f
    })
    return ngenxx_store_kv_write_floatL(inJson)
end

function NGenXX.Store.KV.allKeys(_conn)
    local inJson = JSON.stringify({
        conn = _conn
    })
    local outJson = ngenxx_store_kv_all_keysL(inJson)
    return JSON.parse(outJson);
end

function NGenXX.Store.KV.contains(_conn, _k)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k
    })
    return ngenxx_store_kv_containsL(inJson)
end

function NGenXX.Store.KV.remove(_conn, _k)
    local inJson = JSON.stringify({
        conn = _conn,
        k = _k
    })
    ngenxx_store_kv_removeL(inJson)
end

function NGenXX.Store.KV.clear(_conn)
    local inJson = JSON.stringify({
        conn = _conn
    })
    ngenxx_store_kv_clearL(inJson)
end

function NGenXX.Store.KV.close(_conn)
    local inJson = JSON.stringify({
        conn = _conn
    })
    ngenxx_store_kv_closeL(inJson)
end



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


