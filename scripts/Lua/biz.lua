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
end

function TestCrypto(s)
    local bytes = DynXX.Coding.str2Bytes(s)

    local md5Bytes = DynXX.Crypto.Hash.md5(bytes)
    local md5HexStr = DynXX.Coding.Hex.bytes2Str(md5Bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hash.md5: ' .. md5HexStr)
    local sha256Bytes = DynXX.Crypto.Hash.sha256(bytes)
    local sha256HexStr = DynXX.Coding.Hex.bytes2Str(sha256Bytes)
    DynXX.Log.print(DynXX.Log.Level.Debug, 'Hash.sha256: ' .. sha256HexStr)

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
