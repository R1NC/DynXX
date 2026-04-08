import util from '@ohos.util';
import DynXX, { HttpMethod, LogLevel } from 'libdynxx.so';

@Entry
@Component
struct Index {
  @State message: string = 'Click to test NAPI..';

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(23)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            DynXX.logSetLevel(LogLevel.Debug)
            
            let root = getContext().filesDir;
            DynXX.init(root);

            DynXX.logPrint(LogLevel.Debug, `DeviceType: ${DynXX.deviceType()}`);
            DynXX.logPrint(LogLevel.Debug, `DeviceName: ${DynXX.deviceName()}`);
            DynXX.logPrint(LogLevel.Debug, `DeviceManufacturer: ${DynXX.deviceManufacturer()}`);
            DynXX.logPrint(LogLevel.Debug, `DeviceOsVersion: ${DynXX.deviceOsVersion()}`);
            DynXX.logPrint(LogLevel.Debug, `DeviceCpuArch: ${DynXX.deviceCpuArch()}`);

            let textEncoder = new util.TextEncoder()
            let textDecoder = new util.TextDecoder()
            let inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`";
            let inputBytes = Array.from(textEncoder.encodeInto(inputStr))
            let keyStr = "MNBVCXZLKJHGFDSA"
            let keyBytes = Array.from(textEncoder.encodeInto(keyStr))
            let aesEncodedBytes = DynXX.cryptoAesEncrypt(inputBytes, keyBytes)
            let aesDecodedBytes = DynXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
            let aesDecodedStr = textDecoder.decodeToString(Uint8Array.from(aesDecodedBytes))
            DynXX.logPrint(LogLevel.Debug, `AES->${aesDecodedStr}`)
            let aesgcmIvBytes = DynXX.cryptoRand(12)
            let aesgcmTagBits = 15 * 8
            let aesgcmEncodedBytes = DynXX.cryptoAesGcmEncrypt(inputBytes, keyBytes, aesgcmIvBytes, aesgcmTagBits)
            let aesgcmDecodedBytes = DynXX.cryptoAesGcmDecrypt(aesgcmEncodedBytes, keyBytes, aesgcmIvBytes, aesgcmTagBits)
            let aesgcmDecodedStr = textDecoder.decodeToString(Uint8Array.from(aesgcmDecodedBytes))
            DynXX.logPrint(LogLevel.Debug, `AES-GCM->${aesgcmDecodedStr}`)
            let md5Bytes = DynXX.cryptoHashMd5(inputBytes)
            let md5Str = DynXX.codingHexBytes2str(md5Bytes)
            DynXX.logPrint(LogLevel.Debug, `md5->${md5Str}`)
            let sha256Bytes = DynXX.cryptoHashSha256(inputBytes)
            let sha256Str = DynXX.codingHexBytes2str(sha256Bytes)
            DynXX.logPrint(LogLevel.Debug, `sha256->${sha256Str}`)
            let base64Encoded = DynXX.cryptoBase64Encode(inputBytes, true)
            let base64Decoded = DynXX.cryptoBase64Decode(base64Encoded, true)
            let base64DecodedStr = textDecoder.decodeToString(Uint8Array.from(base64Decoded))
            DynXX.logPrint(LogLevel.Debug, `base64->${base64DecodedStr}`)

            let dbConn = DynXX.sqliteOpen("test");
            if (dbConn != 0) {
              let insertSql = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT);\n" +
                "INSERT OR IGNORE INTO TestTable (platform, vendor)\n" +
                "VALUES\n" +
                "('iOS','Apple'),\n" +
                "('Android','Google'),\n" +
                "('HarmonyOS','Huawei');";
              if (DynXX.sqliteExecute(dbConn, insertSql)) {
                let querySql = "SELECT * FROM TestTable;";
                let queryResult = DynXX.sqliteQueryDo(dbConn, querySql);
                if (queryResult != 0) {
                  while(DynXX.sqliteQueryReadRow(queryResult)) {
                    let platform = DynXX.sqliteQueryReadColumnText(queryResult, "platform");
                    DynXX.logPrint(LogLevel.Debug, `platform->${platform}`);
                  }
                  DynXX.sqliteQueryDrop(queryResult);
                }
                DynXX.sqliteClose(dbConn);
              }
            }

            let kvConn = DynXX.kvOpen("test");
            if (kvConn != 0) {
              DynXX.kvWriteString(kvConn, "s", "DynXX");
              let s = DynXX.kvReadString(kvConn, "s");
              DynXX.logPrint(LogLevel.Debug, `s->${s}`);
              DynXX.kvWriteInteger(kvConn, "i", 1234567890);
              let i = DynXX.kvReadInteger(kvConn, "i");
              DynXX.logPrint(LogLevel.Debug, `i->${i}`);
              DynXX.kvWriteFloat(kvConn, "f", 0.123456789);
              let f = DynXX.kvReadFloat(kvConn, "f");
              DynXX.logPrint(LogLevel.Debug, `f->${f}`);
              DynXX.kvClose(kvConn);
            }

            let jsonParams = `{
              "url":"https://rinc.xyz",
              "params":"p0=1&p1=2&p2=3",
              "method":0,
              "header_v":[
                "Cache-Control: no-cache"
              ],
              "timeout":6666
            }`;
            /*DynXX.lLoadS(
              "function lNetHttpRequest(pReq)\n" +
                "pLog = '{\"level\":1,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "dynxx_log_printL(pLog);\n" +
                "rsp = dynxx_net_http_requestL(pReq);\n" +
                "return rsp;\n" +
                "end"
            );
            let rsp = DynXX.lCall("lNetHttpRequest", jsonParams);*/
            DynXX.jLoadS(
              "function jNetHttpRequest(params) {\n"
              + "var url = JSON.parse(params).url;\n"
              + "var pLog = `{\"level\":1,\"content\":\"Send Net Req from JS, url: ${url}\"}`;\n"
              + "dynxx_log_print(pLog);\n"
              + "return dynxx_net_http_request(params);\n"
              + "}\n"
            , "biz.js", false);
            let rsp = DynXX.jCall("jNetHttpRequest", jsonParams, false);
            /*let rsp = DynXX.netHttpRequest("https://rinc.xyz",
              HttpMethod.Get,
              "p0=1&p1=2&p2=3",
              ["Cache-Control: no-cache"]
            )*/
            DynXX.logPrint(LogLevel.Debug, `${rsp}`);

            let jsonDecoder = DynXX.jsonDecoderInit(jsonParams)
            if (jsonDecoder != 0) {
              let urlNode = DynXX.jsonDecoderReadNode(jsonDecoder, "url")
              if (urlNode != 0) {
                let url = DynXX.jsonDecoderReadString(jsonDecoder, urlNode)
                DynXX.logPrint(LogLevel.Debug, `url:${url}`)
              }
              let methodNode = DynXX.jsonDecoderReadNode(jsonDecoder, "method")
              if (methodNode != 0) {
                let method = DynXX.jsonDecoderReadInteger(jsonDecoder, methodNode)
                DynXX.logPrint(LogLevel.Debug, `method:${method}`)
              }
              let headerVNode = DynXX.jsonDecoderReadNode(jsonDecoder, "header_v")
              if (headerVNode != 0) {
                let headerNode = DynXX.jsonDecoderReadChild(jsonDecoder, headerVNode)
                while (headerNode != 0) {
                  let header = DynXX.jsonDecoderReadString(jsonDecoder, headerNode)
                  DynXX.logPrint(LogLevel.Debug, `header:${header}`)
                  headerNode = DynXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                }
              }
              DynXX.jsonDecoderRelease(jsonDecoder)
            }

            //DynXX.release();
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
