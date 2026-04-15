--- DynXX API for LUA.
-- @module DynXX
DynXX = {}

local InJsonVoid = ''

--- Get DynXX runtime version string.
-- @treturn string
function DynXX.version()
    return dynxx_get_version()
end

--- Get DynXX root path.
-- @treturn string
function DynXX.root()
    return dynxx_root_path()
end

-- @submodule DynXX.Log
DynXX.Log = {}

--- Log levels.
DynXX.Log.Level = {
    Debug = 3, -- Debug level
    Info  = 4, -- Info level
    Warn  = 5, -- Warning level
    Error = 6, -- Error level
    Fatal = 7, -- Fatal error level
    None  = 8  -- No logging
}

--- Print a log message with a level.
-- @tparam number level
-- @tparam string content
function DynXX.Log.print(level, content)
    local inJson = JSON.stringify({
        ["level"] = level,
        ["content"] = content
    })
    dynxx_log_print(inJson)
end

-- @submodule DynXX.Device
DynXX.Device = {}

--- Device platforms.
DynXX.Device.Platform = {
    Unknown     = 0, -- Unknown platform
    Android     = 1, -- Android OS
    ApplePhone  = 2, -- iPhone
    ApplePad    = 3, -- iPad
    AppleMac    = 4, -- macOS
    AppleWatch  = 5, -- Apple Watch
    AppleTV     = 6, -- AppleTV
    HarmonyOS   = 7, -- HarmonyOS
    Windows     = 8, -- Microsoft Windows
    Linux       = 9, -- Linux
    Web         = 10 -- Web browser
}

--- CPU architectures.
DynXX.Device.CpuArch = {
    Unknown = 0, -- Unknown architecture
    X86     = 1, -- x86
    X86_64  = 2, -- x86_64
    IA64    = 3, -- IA64
    ARM     = 4, -- ARM
    ARM_6   = 5  -- ARMv6
}

--- Get current device platform.
-- @treturn number
function DynXX.Device.platform()
    return dynxx_device_type(InJsonVoid)
end

--- Get current device name.
-- @treturn string
function DynXX.Device.name()
    return dynxx_device_name(InJsonVoid)
end

--- Get current device manufacturer.
-- @treturn string
function DynXX.Device.manufacturer()
    return dynxx_device_manufacturer(InJsonVoid)
end

--- Get current operating system version.
-- @treturn string
function DynXX.Device.osVersion()
    return dynxx_device_os_version(InJsonVoid)
end

--- Get current CPU architecture.
-- @treturn number
function DynXX.Device.cpuArch()
    return dynxx_device_cpu_arch(InJsonVoid)
end

-- @submodule DynXX.Net
DynXX.Net = {}

-- @submodule DynXX.Net.Http
DynXX.Net.Http = {}

--- HTTP methods.
DynXX.Net.Http.Method = {
    Get  = 0, -- GET method
    Post = 1, -- POST method
    Put  = 2  -- PUT method
}

--- Send an HTTP request.
-- @tparam string url
-- @tparam number method
-- @tparam[opt] table param_map
-- @tparam[opt] table header_map
-- @tparam[opt] number[] raw_body_bytes
-- @tparam[opt] number timeout
-- @treturn string
function DynXX.Net.Http.request(url, method, param_map, header_map, raw_body_bytes, timeout)
    param_map = param_map or {}
    local paramStr = ""
    for k, v in pairs(param_map) do
        if string.len(paramStr) == 0 then
            paramStr = paramStr .. '?'
        else
            paramStr = paramStr .. '&'
        end
        paramStr = paramStr .. k .. '=' .. v
    end

    local headerArray = {}
    header_map = header_map or {}
    for k, v in pairs(header_map) do
        table.insert(headerArray, k .. '=' .. v)
    end

    timeout = timeout or (15 * 1000)

    local inDict = {
        ["url"] = url,
        ["method"] = method,
        ["params"] = paramStr,
        ["header_v"] = headerArray,
        ["timeout"] = timeout
    }

    if (raw_body_bytes ~= nil and #raw_body_bytes > 0) then
        inDict["rawBodyBytes"] = raw_body_bytes
    end

    local inJson = JSON.stringify(inDict)
    return dynxx_net_http_request(inJson)
end

--- Download a URL to local file.
-- @tparam string url
-- @tparam string file
-- @tparam[opt] number timeout
-- @treturn string
function DynXX.Net.Http.download(url, file, timeout)
    timeout = timeout or (15 * 1000)
    local inJson = JSON.stringify({
        ["url"] = url,
        ["file"] = file,
        ["timeout"] = timeout
    })
    return dynxx_net_http_download(inJson)
end

-- @submodule DynXX.Coding
DynXX.Coding = {}

--- Decode bytes to UTF-8 string.
-- @tparam number[] bytes
-- @treturn string
function DynXX.Coding.bytes2Str(bytes)
    local inJson = JSON.stringify({
        ["inBytes"] = bytes
    })
    return dynxx_coding_bytes2str(inJson)
end

--- Encode UTF-8 string to bytes.
-- @tparam string str
-- @treturn number[]
function DynXX.Coding.str2Bytes(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    local outJson = dynxx_coding_str2bytes(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Coding.Case
DynXX.Coding.Case = {}

--- Convert string to uppercase.
-- @tparam string str
-- @treturn string
function DynXX.Coding.Case.upper(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    return dynxx_coding_case_upper(inJson)
end

--- Convert string to lowercase.
-- @tparam string str
-- @treturn string
function DynXX.Coding.Case.lower(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    return dynxx_coding_case_lower(inJson)
end

-- @submodule DynXX.Coding.Hex
DynXX.Coding.Hex = {}

--- Encode bytes to hex string.
-- @tparam number[] bytes
-- @treturn string
function DynXX.Coding.Hex.bytes2Str(bytes)
    local inJson = JSON.stringify({
        ["inBytes"] = bytes
    })
    return dynxx_coding_hex_bytes2str(inJson)
end

--- Decode hex string to bytes.
-- @tparam string str
-- @treturn number[]
function DynXX.Coding.Hex.str2Bytes(str)
    local inJson = JSON.stringify({
        ["str"] = str
    })
    local outJson = dynxx_coding_hex_str2bytes(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto
DynXX.Crypto = {}

--- Generate random bytes.
-- @tparam number len
-- @treturn number[]
function DynXX.Crypto.rand(len)
    local inJson = JSON.stringify({
        ["len"] = len
    })
    local outJson = dynxx_crypto_rand(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto.Aes
DynXX.Crypto.Aes = {}

--- AES encrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @treturn any
function DynXX.Crypto.Aes.encrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = dynxx_crypto_aes_encrypt(inJson)
    return JSON.parse(outJson)
end

--- AES decrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @treturn any
function DynXX.Crypto.Aes.decrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = dynxx_crypto_aes_decrypt(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto.Aes.Gcm
DynXX.Crypto.Aes.Gcm = {}

--- AES-GCM encrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @tparam number[] ivBytes
-- @tparam number tagBits
-- @tparam[opt] number[] aadBytes
-- @treturn number[]
function DynXX.Crypto.Aes.Gcm.encrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes)
    local inDict = {
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes,
        ["initVectorBytes"] = ivBytes,
        ["tagBits"] = tagBits
    }
    if (aadBytes ~= nil and #aadBytes > 0) then
        inDict["aadBytes"] = aadBytes
    end
    local inJson = JSON.stringify(inDict)
    local outJson = dynxx_crypto_aes_gcm_encrypt(inJson)
    return JSON.parse(outJson)
end

--- AES-GCM decrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @tparam number[] ivBytes
-- @tparam number tagBits
-- @tparam[opt] number[] aadBytes
-- @treturn number[]
function DynXX.Crypto.Aes.Gcm.decrypt(inBytes, keyBytes, ivBytes, tagBits, aadBytes)
    local inDict = {
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes,
        ["initVectorBytes"] = ivBytes,
        ["tagBits"] = tagBits
    }
    if (aadBytes ~= nil and #aadBytes > 0) then
        inDict["aadBytes"] = aadBytes
    end
    local inJson = JSON.stringify(inDict)
    local outJson = dynxx_crypto_aes_gcm_decrypt(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto.Rsa
DynXX.Crypto.Rsa = {}

--- Parse RSA key from base64.
-- @tparam string base64
-- @tparam boolean isPublic
-- @treturn number[]
function DynXX.Crypto.Rsa.genKey(base64, isPublic)
    local inJson = JSON.stringify({
        ["base64"] = base64,
        ["isPublic"] = isPublic
    })
    local outJson = dynxx_crypto_rsa_gen_key(inJson)
    return JSON.parse(outJson)
end

--- RSA encrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @treturn number[]
function DynXX.Crypto.Rsa.encrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = dynxx_crypto_rsa_encrypt(inJson)
    return JSON.parse(outJson)
end

--- RSA decrypt bytes.
-- @tparam number[] inBytes
-- @tparam number[] keyBytes
-- @treturn number[]
function DynXX.Crypto.Rsa.decrypt(inBytes, keyBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["keyBytes"] = keyBytes
    })
    local outJson = dynxx_crypto_rsa_decrypt(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto.Hash
DynXX.Crypto.Hash = {}

--- Calculate MD5 digest.
-- @tparam number[] inBytes
-- @treturn number[]
function DynXX.Crypto.Hash.md5(inBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes
    })
    local outJson = dynxx_crypto_hash_md5(inJson)
    return JSON.parse(outJson)
end

--- Calculate SHA1 digest.
-- @tparam number[] inBytes
-- @treturn number[]
function DynXX.Crypto.Hash.sha1(inBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes
    })
    local outJson = dynxx_crypto_hash_sha1(inJson)
    return JSON.parse(outJson)
end

--- Calculate SHA256 digest.
-- @tparam number[] inBytes
-- @treturn number[]
function DynXX.Crypto.Hash.sha256(inBytes)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes
    })
    local outJson = dynxx_crypto_hash_sha256(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.Crypto.Base64
DynXX.Crypto.Base64 = {}

--- Base64 encode bytes.
-- @tparam number[] inBytes
-- @tparam[opt] boolean noNewLines
-- @treturn number[]
function DynXX.Crypto.Base64.encode(inBytes, noNewLines)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["noNewLines"] = noNewLines and 1 or 0
    })
    local outJson = dynxx_crypto_base64_encode(inJson)
    return JSON.parse(outJson)
end

--- Base64 decode bytes.
-- @tparam number[] inBytes
-- @tparam[opt] boolean noNewLines
-- @treturn number[]
function DynXX.Crypto.Base64.decode(inBytes, noNewLines)
    local inJson = JSON.stringify({
        ["inBytes"] = inBytes,
        ["noNewLines"] = noNewLines and 1 or 0
    })
    local outJson = dynxx_crypto_base64_decode(inJson)
    return JSON.parse(outJson)
end

-- @submodule DynXX.SQLite
DynXX.SQLite = {}

--- Open SQLite database by identifier.
-- @tparam string id
-- @treturn number
function DynXX.SQLite.open(id)
    local inJson = JSON.stringify({
        ["_id"] = id
    })
    return dynxx_sqlite_open(inJson)
end

--- Execute SQLite SQL statement.
-- @tparam number conn
-- @tparam string sql
-- @treturn number
function DynXX.SQLite.execute(conn, sql)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["sql"] = sql
    })
    return dynxx_sqlite_execute(inJson)
end

--- Close SQLite connection.
-- @tparam number conn
function DynXX.SQLite.close(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    dynxx_sqlite_close(inJson)
end

-- @submodule DynXX.SQLite.Query
DynXX.SQLite.Query = {}

--- Create SQLite query cursor.
-- @tparam number conn
-- @tparam string sql
-- @treturn number
function DynXX.SQLite.Query.create(conn, sql)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["sql"] = sql
    })
    return dynxx_sqlite_query_do(inJson)
end

--- Advance SQLite query cursor.
-- @tparam number query_result
-- @treturn number
function DynXX.SQLite.Query.readRow(query_result)
    local inJson = JSON.stringify({
        ["query_result"] = query_result
    })
    return dynxx_sqlite_query_read_row(inJson)
end

--- Read text column from current row.
-- @tparam number query_result
-- @tparam number column
-- @treturn string
function DynXX.SQLite.Query.readColumnText(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return dynxx_sqlite_query_read_column_text(inJson)
end

--- Read integer column from current row.
-- @tparam number query_result
-- @tparam number column
-- @treturn number
function DynXX.SQLite.Query.readColumnInteger(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return dynxx_sqlite_query_read_column_integer(inJson)
end

--- Read float column from current row.
-- @tparam number query_result
-- @tparam number column
-- @treturn number
function DynXX.SQLite.Query.readColumnFloat(query_result, column)
    local inJson = JSON.stringify({
        ["query_result"] = query_result,
        ["column"] = column
    })
    return dynxx_sqlite_query_read_column_float(inJson)
end

--- Release SQLite query cursor.
-- @tparam number query_result
function DynXX.SQLite.Query.drop(query_result)
    local inJson = JSON.stringify({
        ["query_result"] = query_result
    })
    dynxx_sqlite_query_drop(inJson)
end

-- @submodule DynXX.KV
DynXX.KV = {}

--- Open KV storage by identifier.
-- @tparam string id
-- @treturn number
function DynXX.KV.open(id)
    local inJson = JSON.stringify({
        ["_id"] = id
    })
    return dynxx_kv_open(inJson)
end

--- Read string value by key.
-- @tparam number conn
-- @tparam string k
-- @treturn string
function DynXX.KV.readString(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return dynxx_kv_read_string(inJson)
end

--- Write string value by key.
-- @tparam number conn
-- @tparam string k
-- @tparam string s
-- @treturn number
function DynXX.KV.writeString(conn, k, s)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = s
    })
    return dynxx_kv_write_string(inJson)
end

--- Read integer value by key.
-- @tparam number conn
-- @tparam string k
-- @treturn number
function DynXX.KV.readInteger(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return dynxx_kv_read_integer(inJson)
end

--- Write integer value by key.
-- @tparam number conn
-- @tparam string k
-- @tparam number i
-- @treturn number
function DynXX.KV.writeInteger(conn, k, i)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = i
    })
    return dynxx_kv_write_integer(inJson)
end

--- Read float value by key.
-- @tparam number conn
-- @tparam string k
-- @treturn number
function DynXX.KV.readFloat(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return dynxx_kv_read_float(inJson)
end

--- Write float value by key.
-- @tparam number conn
-- @tparam string k
-- @tparam number f
-- @treturn number
function DynXX.KV.writeFloat(conn, k, f)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k,
        ["v"] = f
    })
    return dynxx_kv_write_float(inJson)
end

--- List all keys in KV storage.
-- @tparam number conn
-- @treturn any
function DynXX.KV.allKeys(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    local outJson = dynxx_kv_all_keys(inJson)
    return JSON.parse(outJson)
end

--- Check whether key exists.
-- @tparam number conn
-- @tparam string k
-- @treturn number
function DynXX.KV.contains(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    return dynxx_kv_contains(inJson)
end

--- Remove key from KV storage.
-- @tparam number conn
-- @tparam string k
function DynXX.KV.remove(conn, k)
    local inJson = JSON.stringify({
        ["conn"] = conn,
        ["k"] = k
    })
    dynxx_kv_remove(inJson)
end

--- Clear all KV entries.
-- @tparam number conn
function DynXX.KV.clear(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    dynxx_kv_clear(inJson)
end

--- Close KV storage connection.
-- @tparam number conn
function DynXX.KV.close(conn)
    local inJson = JSON.stringify({
        ["conn"] = conn
    })
    dynxx_kv_close(inJson)
end

-- @submodule DynXX.KV
DynXX.Z = {}

--- Compression formats.
DynXX.Z.Format = {
    ZLib = 0, -- ZLib format
    GZip = 1, -- GZip format
    Raw  = 2  -- Raw format (no header)
}

--- Compression modes.
DynXX.Z.ZipMode = {
    Default     = -1, -- Default compression
    PreferSpeed = 1,  -- Prefer compression speed
    PreferSize  = 9   -- Prefer compression size
}

--- Default buffer size (16KB).
DynXX.Z.DefaultBufferSize = 16 * 1024

--- Compress bytes in memory.
-- @tparam number[] inBytes
-- @tparam[opt] number format
-- @tparam[opt] number mode
-- @treturn number[]
function DynXX.Z.zipBytes(inBytes, format, mode)
    format = format or DynXX.Z.Format.ZLib
    mode = mode or DynXX.Z.ZipMode.Default
    local inJson = JSON.stringify({
        ["mode"] = mode,
        ["bufferSize"] = DynXX.Z.DefaultBufferSize,
        ["format"] = format,
        ["inBytes"] = inBytes
    })
    local outJson = dynxx_z_bytes_unzip(inJson)
    return JSON.parse(outJson)
end

--- Decompress bytes in memory.
-- @tparam number[] inBytes
-- @tparam[opt] number format
-- @treturn number[]
function DynXX.Z.unZipBytes(inBytes, format)
    format = format or DynXX.Z.Format.ZLib
    local inJson = JSON.stringify({
        ["bufferSize"] = DynXX.Z.DefaultBufferSize,
        ["format"] = format,
        ["inBytes"] = inBytes
    })
    local outJson = dynxx_z_bytes_unzip(inJson)
    return JSON.parse(outJson)
end

--- Compress a file to another file.
-- @tparam string inFilePath
-- @tparam string outFilePath
-- @tparam number mode
-- @tparam number bufferSize
-- @tparam number format
-- @treturn boolean
function DynXX.Z.zipFile(inFilePath, outFilePath, mode, bufferSize, format)
    local inF = io.open(inFilePath, 'r')
    local outF = io.open(outFilePath, 'w')
    if (inF == nil or outF == nil) then
        return false
    end

    local res = DynXX.Z._.zipStream(
        function()
            return inF:read(bufferSize)
        end,
        function(bytes)
            outF:write(bytes)
        end,
        function()
            outF:flush()
        end,
        mode,
        bufferSize,
        format
    )

    outF:close()
    inF:close()
    return res
end

--- Decompress a file to another file.
-- @tparam string inFilePath
-- @tparam string outFilePath
-- @tparam number bufferSize
-- @tparam number format
-- @treturn boolean
function DynXX.Z.unZipFile(inFilePath, outFilePath, bufferSize, format)
    local inF = io.open(inFilePath, 'r')
    local outF = io.open(outFilePath, 'w')
    if (inF == nil or outF == nil) then
        return false
    end

    local res = DynXX.Z._.unZipStream(
        function()
            return inF:read(bufferSize)
        end,
        function(bytes)
            outF:write(bytes)
        end,
        function()
            outF:flush()
        end,
        bufferSize,
        format
    )

    outF:close()
    inF:close()
    return res
end

DynXX.Z._ = {}

function DynXX.Z._.zipInit(mode, bufferSize, format)
    local inJson = JSON.stringify({
        ["mode"] = mode,
        ["bufferSize"] = bufferSize,
        ["format"] = format
    })
    return dynxx_z_zip_init(inJson)
end

function DynXX.Z._.zipInput(zip, bytes, finish)
    local inJson = JSON.stringify({
        ["zip"] = zip,
        ["inBytes"] = bytes,
        ["inFinish"] = finish and 1 or 0
    })
    return dynxx_z_zip_input(inJson)
end

function DynXX.Z._.zipProcessDo(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    local outJson = dynxx_z_zip_process_do(inJson)
    return JSON.parse(outJson)
end

function DynXX.Z._.zipProcessFinished(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    return dynxx_z_zip_process_finished(inJson)
end

function DynXX.Z._.zipRelease(zip)
    local inJson = JSON.stringify({
        ["zip"] = zip
    })
    dynxx_z_zip_release(inJson)
end

function DynXX.Z._.unZipInit(bufferSize, format)
    local inJson = JSON.stringify({
        ["bufferSize"] = bufferSize,
        ["format"] = format
    })
    return dynxx_z_unzip_init(inJson)
end

function DynXX.Z._.unZipInput(unzip, bytes, finish)
    local inJson = JSON.stringify({
        ["unzip"] = unzip,
        ["inBytes"] = bytes,
        ["inFinish"] = finish and 1 or 0
    })
    return dynxx_z_unzip_input(inJson)
end

function DynXX.Z._.unZipProcessDo(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    local outJson = dynxx_z_unzip_process_do(inJson)
    return JSON.parse(outJson)
end

function DynXX.Z._.unZipProcessFinished(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    return dynxx_z_unzip_process_finished(inJson)
end

function DynXX.Z._.unZipRelease(unzip)
    local inJson = JSON.stringify({
        ["unzip"] = unzip
    })
    dynxx_z_unzip_release(inJson)
end

function DynXX.Z._.stream(bufferSize, readFunc, writeFunc, flushFunc, z, inputFunc, processDoFunc, processFinishedFunc)
    local inputFinished = false
    local processFinished = false
    repeat
        local inBytes = readFunc()
        inputFinished = #inBytes < bufferSize
        DynXX.Log.print(DynXX.Log.Level.Debug, 'z <- len:' .. #inBytes .. ' finished:' .. inputFinished)

        local inputRet = inputFunc(z, inBytes, inputFinished)
        if (inputRet <= 0) then
            DynXX.Log.print(DynXX.Log.Level.Error, 'z input failed!')
            return false
        end

        processFinished = false
        repeat
            local outBytes = processDoFunc(z)
            if (#outBytes == 0) then
                DynXX.Log.print(DynXX.Log.Level.Error, 'z process failed!')
                return false
            end
            processFinished = processFinishedFunc(z)
            DynXX.Log.print(DynXX.Log.Level.Debug, 'z -> len:' .. #outBytes .. ' finished:' .. processFinished)

            writeFunc(outBytes)
        until (~processFinished)
    until (~inputFinished)

    flushFunc()
    return true
end

function DynXX.Z._.zipStream(readFunc, writeFunc, flushFunc, mode, bufferSize, format)
    local zip = DynXX.Z._.zipInit(mode, bufferSize, format)

    local res = DynXX.Z._.Stream(bufferSize, readFunc, writeFunc, flushFunc, zip,
        function(z, buffer, inputFinished)
            return DynXX.Z._.zipInput(z, buffer, inputFinished)
        end,
        function(z)
            return DynXX.Z._.zipProcessDo(z)
        end,
        function(z)
            return DynXX.Z._.zipProcessFinished(z)
        end
    )

    DynXX.Z._.zipRelease(zip)
    return res
end

function DynXX.Z._.unZipStream(readFunc, writeFunc, flushFunc, bufferSize, format)
    local unzip = DynXX.Z._.unZipInit(bufferSize, format)

    local res = DynXX.Z._.Stream(bufferSize, readFunc, writeFunc, flushFunc, unzip,
        function(z, buffer, inputFinished)
            return DynXX.Z._.unZipInput(z, buffer, inputFinished)
        end,
        function(z)
            return DynXX.Z._.unZipProcessDo(z)
        end,
        function(z)
            return DynXX.Z._.unZipProcessFinished(z)
        end
    )

    DynXX.Z._.unZipRelease(unzip)
    return res
end
