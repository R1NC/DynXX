function TestNetHttpRequest(url)
    local method = DynXX.Net.Http.Method.Post
    local paramMap = {
        p0 = 123,
        p1 = 'abc'
    }
    local headerMap = {
        ['User-Agent'] = 'DynXX',
        ['Cache-Control'] = 'no-cache',
    }
    local rawNodyBytes = {}
    local timeout = 10 * 1000
    return DynXX.Net.Http.request(url, method, paramMap, headerMap, rawNodyBytes, timeout)
end

function TestNetHttpSetConfigs()
    DynXX.Net.Http.setCertPath('certs/ca.pem')
    DynXX.Net.Http.setProxy({host = '127.0.0.1', port = 8080, username = 'u', password = 'p'})
    DynXX.Net.Http.setDnsConfigs({{host = 'example.com', port = 443, address = '1.2.3.4'}})
    -- clear the global configs so they do not leak into other tests
    DynXX.Net.Http.setCertPath()
    DynXX.Net.Http.setProxy()
    DynXX.Net.Http.setDnsConfigs()
    return true
end

function TestDeviceInfo()
    local deviceInfo = {
        platform = DynXX.Device.platform(),
        manufacturer = DynXX.Device.manufacturer(),
        name = DynXX.Device.name(),
        osVersion = DynXX.Device.osVersion(),
        cpuArch = DynXX.Device.cpuArch()
    }
    DynXX.Log.print(DynXX.Log.Level.Debug, JSON.stringify(deviceInfo))
end

function TestCoding()
    local s = 'DynXX'

    DynXX.Log.print(DynXX.Log.Level.Debug, 'Case.Upper: ' .. DynXX.Coding.Case.upper(s))
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Case.Lower: ' .. DynXX.Coding.Case.lower(s))

    local bytes = DynXX.Coding.str2Bytes(s)
    local str = DynXX.Coding.bytes2Str(bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'str2Bytes: ' .. JSON.stringify(bytes))
    DynXX.Log.print(DynXX.Log.Level.Debug, 'bytes2Str: ' .. str)

    local hexStr = DynXX.Coding.Hex.bytes2Str(bytes)
    local hexRoundTrip = DynXX.Coding.bytes2Str(DynXX.Coding.Hex.str2Bytes(hexStr))
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hex.str2Bytes: ' .. hexStr)
    assert(hexRoundTrip == s, 'hex roundtrip mismatch')
end

function TestCrypto(s)
    local bytes = DynXX.Coding.str2Bytes(s)

    local md5Bytes = DynXX.Crypto.Hash.md5(bytes)
    local md5HexStr = DynXX.Coding.Hex.bytes2Str(md5Bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hash.md5: ' .. md5HexStr)
    local sha256Bytes = DynXX.Crypto.Hash.sha256(bytes)
    local sha256HexStr = DynXX.Coding.Hex.bytes2Str(sha256Bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hash.sha256: ' .. sha256HexStr)

    local sha1Bytes = DynXX.Crypto.Hash.sha1(bytes)
    local sha1HexStr = DynXX.Coding.Hex.bytes2Str(sha1Bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hash.sha1: ' .. sha1HexStr)

    local noNewLines = true
    local base64EncodedBytes = DynXX.Crypto.Base64.encode(bytes, noNewLines)
    local base64EncodedStr = DynXX.Coding.bytes2Str(base64EncodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Base64.encode: ' .. base64EncodedStr)
    local base64DecodedBytes = DynXX.Crypto.Base64.decode(base64EncodedBytes, noNewLines)
    local base64DecodedStr = DynXX.Coding.bytes2Str(base64DecodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Base64.decode: ' .. base64DecodedStr)

    local key = 'qwertyuiop123456'
    local keyBytes = DynXX.Coding.str2Bytes(key)
    local aesEncodedBytes = DynXX.Crypto.Aes.encrypt(bytes, keyBytes)
    local aesEncodedStr = DynXX.Coding.Hex.bytes2Str(aesEncodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Aes.encrypt: ' .. aesEncodedStr)
    local aesDecodedBytes = DynXX.Crypto.Aes.decrypt(aesEncodedBytes, keyBytes)
    local aesDecodedStr = DynXX.Coding.bytes2Str(aesDecodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Aes.decrypt: ' .. aesDecodedStr)

    local ivBytes = DynXX.Crypto.rand(12)
    local tagBits = 15 * 8
    local aesGcmEncodedBytes = DynXX.Crypto.Aes.Gcm.encrypt(bytes, keyBytes, ivBytes, tagBits)
    local aesGcmEncodedStr = DynXX.Coding.Hex.bytes2Str(aesGcmEncodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Aes.Gcm.encrypt: ' .. aesGcmEncodedStr)
    local aesGcmDecodedBytes = DynXX.Crypto.Aes.Gcm.decrypt(aesGcmEncodedBytes, keyBytes, ivBytes, tagBits)
    local aesGcmDecodedStr = DynXX.Coding.bytes2Str(aesGcmDecodedBytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Aes.Gcm.decrypt: ' .. aesGcmDecodedStr)
end

function TestKV()
    local conn = DynXX.KV.open('test4lua')
    DynXX.KV.writeString(conn, 's', 'DynXX')
    DynXX.KV.writeInteger(conn, 'i', 1234567890)
    DynXX.KV.writeFloat(conn, 'f', 0.987654321)
    local keys = DynXX.KV.allKeys(conn)
    for _, k in ipairs(keys) do
        local v = DynXX.KV.readString(conn, k)
        if k == 's' then
            DynXX.Log.print(DynXX.Log.Level.Debug, k .. ': ' .. DynXX.KV.readString(conn, k))
        elseif k == 'i' then
            DynXX.Log.print(DynXX.Log.Level.Debug, k .. ': ' .. DynXX.KV.readInteger(conn, k))
        elseif k == 'f' then
            DynXX.Log.print(DynXX.Log.Level.Debug, k .. ': ' .. DynXX.KV.readFloat(conn, k))
        end
    end
    -- Assigned to locals first: `DynXX.KV.contains` returns no value, so feeding the call
    -- straight into `tostring()` would raise "value expected".
    local containsBefore = DynXX.KV.contains(conn, 's')
    local removed = DynXX.KV.remove(conn, 's')
    local containsAfter = DynXX.KV.contains(conn, 's')
    DynXX.Log.print(DynXX.Log.Level.Debug, 'KV contains: ' .. tostring(containsBefore)
        .. ', remove: ' .. tostring(removed) .. ', contains after: ' .. tostring(containsAfter))
    DynXX.KV.clear(conn)
    DynXX.KV.close(conn)
end

function TestSQLite()
    local sqlPrepareData = [[
        DROP TABLE IF EXISTS TestTable;
        CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
        INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
        ('iOS', 1, 0.111111111),
        ('Android', 2, 0.2222222222),
        ('HarmonyOS', 3, 0.3333333333);
    ]]
    local sqlQuery = 'SELECT * FROM TestTable;'

    local conn = DynXX.SQLite.open('test.lua')
    DynXX.SQLite.execute(conn, sqlPrepareData)
    local query = DynXX.SQLite.Query.create(conn, sqlQuery)
    while (DynXX.SQLite.Query.readRow(query)) do
        local s = DynXX.SQLite.Query.readColumnText(query, 's')
        local i = DynXX.SQLite.Query.readColumnInteger(query, 'i')
        local f = DynXX.SQLite.Query.readColumnFloat(query, 'f')
        DynXX.Log.print(DynXX.Log.Level.Debug, s .. ' | ' .. i .. ' | ' .. f)
    end
    DynXX.SQLite.Query.drop(query)
    DynXX.SQLite.close(conn)
end

function TestCoroutine(url)
    local co = coroutine.create(
        function(_url)
            return TestNetHttpRequest(_url)
        end
    )
    DynXX.Log.print(DynXX.Log.Level.Debug, 'sent http request on ' .. tostring(co) .. ' ...')
    local _, res = coroutine.resume(co, url)
    return res
end

function TestTimer()
    local count = 0
    local timer
    local timerF = function()
        count = count + 1
        TestSQLite()
        if count == 3 then
            Timer.remove(timer)
        end
    end
    timer = Timer.add(1234, true, timerF)
end

function TestVersion()
    local version = DynXX.version()
    local rootPath = DynXX.root()
    DynXX.Log.print(DynXX.Log.Level.Debug, 'version: ' .. version .. ', root: ' .. rootPath)
    assert(version ~= '', 'version empty')
    assert(rootPath ~= '', 'root path empty')
end

function TestNetHttpDownload(url)
    -- The endpoint is unreachable on purpose: the binding is what is under test here,
    -- the transfer itself is covered by Net.test.cxx.
    local downloaded = DynXX.Net.Http.download(url, 'dynxx_lua_download.tmp', 1000)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'download: ' .. tostring(downloaded))
end

function TestZ()
    local inS = 'DynXX Lua zip test'
    local inBytes = DynXX.Coding.str2Bytes(inS)

    local zipped = DynXX.Z.zipBytes(inBytes, DynXX.Z.Format.GZip)
    local bytesOutS = DynXX.Coding.bytes2Str(DynXX.Z.unZipBytes(zipped, DynXX.Z.Format.GZip))
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Z bytes roundtrip: ' .. bytesOutS)
    assert(bytesOutS == inS, 'zip bytes roundtrip mismatch')

    -- Drive the streaming bindings directly: the file wrapper above them (DynXX.Z.zipFile)
    -- is broken because DynXX.Z._.Stream does not exist.
    local zip = DynXX.Z._.zipInit(DynXX.Z.ZipMode.Default, DynXX.Z.DefaultBufferSize, DynXX.Z.Format.GZip)
    DynXX.Z._.zipInput(zip, inBytes, true)
    local streamZipped = DynXX.Z._.zipProcessDo(zip)
    local zipFinished = DynXX.Z._.zipProcessFinished(zip)
    DynXX.Z._.zipRelease(zip)

    local unzip = DynXX.Z._.unZipInit(DynXX.Z.DefaultBufferSize, DynXX.Z.Format.GZip)
    DynXX.Z._.unZipInput(unzip, streamZipped, true)
    local streamOutS = DynXX.Coding.bytes2Str(DynXX.Z._.unZipProcessDo(unzip))
    local unzipFinished = DynXX.Z._.unZipProcessFinished(unzip)
    DynXX.Z._.unZipRelease(unzip)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Z stream finished: ' .. tostring(zipFinished) .. ' / ' .. tostring(unzipFinished))
    assert(streamOutS == inS, 'zip stream roundtrip mismatch')

    return bytesOutS
end
