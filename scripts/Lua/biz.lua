function TestNetHttpRequest(url)
    local method = NGenXX.Net.Http.Method.Post
    local paramMap = {
        p0 = 123,
        p1 = 'abc'
    }
    local headerMap = {
        ['User-Agent'] = 'NGenXX',
        ['Cache-Control'] = 'no-cache',
    }
    local rawNodyBytes = {}
    local timeout = 10 * 1000
    return NGenXX.Net.Http.request(url, method, paramMap, headerMap, rawNodyBytes, timeout)
end

function TestDeviceInfo()
    local deviceInfo = {
        platform = NGenXX.Device.platform(),
        manufacturer = NGenXX.Device.manufacturer(),
        name = NGenXX.Device.name(),
        osVersion = NGenXX.Device.osVersion(),
        cpuArch = NGenXX.Device.cpuArch()
    }
    NGenXX.Log.print(NGenXX.Log.Level.Debug, JSON.stringify(deviceInfo))
end

function TestCoding()
    local s = 'NGenXX'

    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Case.Upper: ' .. NGenXX.Coding.Case.upper(s))
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Case.Lower: ' .. NGenXX.Coding.Case.lower(s))

    local bytes = NGenXX.Coding.str2Bytes(s)
    local str = NGenXX.Coding.bytes2Str(bytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'str2Bytes: ' .. JSON.stringify(bytes))
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'bytes2Str: ' .. str)
end

function TestCrypto(s)
    local bytes = NGenXX.Coding.str2Bytes(s)

    local md5Bytes = NGenXX.Crypto.Hash.md5(bytes)
    local md5HexStr = NGenXX.Coding.Hex.bytes2Str(md5Bytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Hash.md5: ' .. md5HexStr)
    local sha256Bytes = NGenXX.Crypto.Hash.sha256(bytes)
    local sha256HexStr = NGenXX.Coding.Hex.bytes2Str(sha256Bytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Hash.sha256: ' .. sha256HexStr)

    local base64EncodedBytes = NGenXX.Crypto.Base64.encode(bytes)
    local base64EncodedStr = NGenXX.Coding.bytes2Str(base64EncodedBytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Base64.encode: ' .. base64EncodedStr)
    local base64DecodedBytes = NGenXX.Crypto.Base64.decode(base64EncodedBytes)
    local base64DecodedStr = NGenXX.Coding.bytes2Str(base64DecodedBytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Base64.decode: ' .. base64DecodedStr)

    local key = 'qwertyuiop123456'
    local keyBytes = NGenXX.Coding.str2Bytes(key)
    local aesEncodedBytes = NGenXX.Crypto.Aes.encrypt(bytes, keyBytes)
    local aesEncodedStr = NGenXX.Coding.Hex.bytes2Str(aesEncodedBytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Aes.encrypt: ' .. aesEncodedStr)
    local aesDecodedBytes = NGenXX.Crypto.Aes.decrypt(aesEncodedBytes, keyBytes)
    local aesDecodedStr = NGenXX.Coding.bytes2Str(aesDecodedBytes)
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'Aes.decrypt: ' .. aesDecodedStr)
end

function TestStoreKV()
    local conn = NGenXX.Store.KV.open('test4lua')
    NGenXX.Store.KV.writeString(conn, 's', 'NGenXX')
    NGenXX.Store.KV.writeInteger(conn, 'i', 1234567890)
    NGenXX.Store.KV.writeFloat(conn, 'f', 0.987654321)
    local keys = NGenXX.Store.KV.allKeys(conn)
    for _, k in ipairs(keys) do
        local v = NGenXX.Store.KV.readString(conn, k)
        if k == 's' then
            NGenXX.Log.print(NGenXX.Log.Level.Debug, k .. ': ' .. NGenXX.Store.KV.readString(conn, k))
        elseif k == 'i' then
            NGenXX.Log.print(NGenXX.Log.Level.Debug, k .. ': ' .. NGenXX.Store.KV.readInteger(conn, k))
        elseif k == 'f' then
            NGenXX.Log.print(NGenXX.Log.Level.Debug, k .. ': ' .. NGenXX.Store.KV.readFloat(conn, k))
        end
    end
    NGenXX.Store.KV.close(conn)
end

function TestStoreSQLite()
    local sqlPrepareData = [[
        DROP TABLE IF EXISTS TestTable;
        CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, s TEXT, i INTEGER, f FLOAT);
        INSERT OR IGNORE INTO TestTable (s, i, f) VALUES
        ('iOS', 1, 0.111111111),
        ('Android', 2, 0.2222222222),
        ('HarmonyOS', 3, 0.3333333333);
    ]]
    local sqlQuery = 'SELECT * FROM TestTable;'
    
    local conn = NGenXX.Store.SQLite.open('test.lua')
    NGenXX.Store.SQLite.execute(conn, sqlPrepareData)
    local query = NGenXX.Store.SQLite.Query.create(conn, sqlQuery)
    while(NGenXX.Store.SQLite.Query.readRow(query)) do
        local s = NGenXX.Store.SQLite.Query.readColumnText(query, 's')
        local i = NGenXX.Store.SQLite.Query.readColumnInteger(query, 'i')
        local f = NGenXX.Store.SQLite.Query.readColumnFloat(query, 'f')
        NGenXX.Log.print(NGenXX.Log.Level.Debug, s .. ' | ' .. i .. ' | ' .. f)
    end
    NGenXX.Store.SQLite.Query.drop(query)
    NGenXX.Store.SQLite.close(conn)
end

function TestCoroutine(url)
    local co = coroutine.create(
        function (_url)
            return TestNetHttpRequest(_url)
        end
    )
    NGenXX.Log.print(NGenXX.Log.Level.Debug, 'sent http request on ' .. tostring(co) .. ' ...')
    local _, res = coroutine.resume(co, url)
    return res
end

function TestTimer()
    local count = 0
    local timer
    local timerF = function()
        count  = count + 1
        TestStoreSQLite()
        if count == 3 then
            Timer.remove(timer)
        end
    end
    timer = Timer.add(1234, true, timerF)
end
