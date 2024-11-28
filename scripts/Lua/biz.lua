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