import { hilog } from '@kit.PerformanceAnalysisKit';
import util from '@ohos.util';
import NGenXX from 'libngenxx.so';

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
            let root = getContext().filesDir;
            NGenXX.init(root);

            NGenXX.logSetCallback((level, log) => {
              hilog.debug(0x0000, 'NGenXX', `${level} | ${log}`);
            });

            NGenXX.logPrint(1, `DeviceType: ${NGenXX.deviceType()}`);
            NGenXX.logPrint(1, `DeviceName: ${NGenXX.deviceName()}`);
            NGenXX.logPrint(1, `DeviceManufacturer: ${NGenXX.deviceManufacturer()}`);
            NGenXX.logPrint(1, `DeviceOsVersion: ${NGenXX.deviceOsVersion()}`);
            NGenXX.logPrint(1, `DeviceCpuArch: ${NGenXX.deviceCpuArch()}`);

            let textEncoder = new util.TextEncoder()
            let textDecoder = new util.TextDecoder()
            let inputStr = "0123456789qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM<>()[]{}@#$%^&*-=+~!/|_,;:'`";
            let inputBytes = Array.from(textEncoder.encodeInto(inputStr))
            let keyStr = "MNBVCXZLKJHGFDSA"
            let keyBytes = Array.from(textEncoder.encodeInto(keyStr))
            let aesEncodedBytes = NGenXX.cryptoAesEncrypt(inputBytes, keyBytes)
            let aesDecodedBytes = NGenXX.cryptoAesDecrypt(aesEncodedBytes, keyBytes)
            let aesDecodedStr = textDecoder.decodeToString(Uint8Array.from(aesDecodedBytes))
            NGenXX.logPrint(1, `AES->${aesDecodedStr}`)
            let aesgcmIvBytes = NGenXX.cryptoRand(12)
            let aesgcmTagBits = 15 * 8
            let aesgcmEncodedBytes = NGenXX.cryptoAesGcmEncrypt(inputBytes, keyBytes, aesgcmIvBytes, [], aesgcmTagBits)
            let aesgcmDecodedBytes = NGenXX.cryptoAesGcmDecrypt(aesgcmEncodedBytes, keyBytes, aesgcmIvBytes, [], aesgcmTagBits)
            let aesgcmDecodedStr = textDecoder.decodeToString(Uint8Array.from(aesgcmDecodedBytes))
            NGenXX.logPrint(1, `AES-GCM->${aesgcmDecodedStr}`)
            let md5Bytes = NGenXX.cryptoHashMd5(inputBytes)
            let md5Str = NGenXX.codingHexBytes2str(md5Bytes)
            NGenXX.logPrint(1, `md5->${md5Str}`)
            let sha256Bytes = NGenXX.cryptoHashSha256(inputBytes)
            let sha256Str = NGenXX.codingHexBytes2str(sha256Bytes)
            NGenXX.logPrint(1, `sha256->${sha256Str}`)
            let base64Encoded = NGenXX.cryptoBase64Encode(inputBytes)
            let base64Decoded = NGenXX.cryptoBase64Decode(base64Encoded)
            let base64DecodedStr = textDecoder.decodeToString(Uint8Array.from(base64Decoded))
            NGenXX.logPrint(1, `base64->${base64DecodedStr}`)

            let dbConn = NGenXX.storeSQLiteOpen("test");
            if (dbConn > 0) {
              let insertSql = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT);\n" +
                "INSERT OR IGNORE INTO TestTable (platform, vendor)\n" +
                "VALUES\n" +
                "('iOS','Apple'),\n" +
                "('Android','Google'),\n" +
                "('HarmonyOS','Huawei');";
              if (NGenXX.storeSQLiteExecute(dbConn, insertSql)) {
                let querySql = "SELECT * FROM TestTable;";
                let queryResult = NGenXX.storeSQLiteQueryDo(dbConn, querySql);
                if (queryResult > 0) {
                  while(NGenXX.storeSQLiteQueryReadRow(queryResult)) {
                    let platform = NGenXX.storeSQLiteQueryReadColumnText(queryResult, "platform");
                    NGenXX.logPrint(1, `platform->${platform}`);
                  }
                  NGenXX.storeSQLiteQueryDrop(queryResult);
                }
                NGenXX.storeSQLiteClose(dbConn);
              }
            }

            let kvConn = NGenXX.storeKVOpen("test");
            if (kvConn > 0) {
              NGenXX.storeKVWriteString(kvConn, "s", "NGexXX");
              let s = NGenXX.storeKVReadString(kvConn, "s");
              NGenXX.logPrint(1, `s->${s}`);
              NGenXX.storeKVWriteInteger(kvConn, "i", 1234567890);
              let i = NGenXX.storeKVReadInteger(kvConn, "i");
              NGenXX.logPrint(1, `i->${i}`);
              NGenXX.storeKVWriteFloat(kvConn, "f", 0.123456789);
              let f = NGenXX.storeKVReadFloat(kvConn, "f");
              NGenXX.logPrint(1, `f->${f}`);
              NGenXX.storeKVClose(kvConn);
            }

            NGenXX.lLoadS(
              "function lNetHttpRequest(pReq)\n" +
                "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "ngenxx_log_printL(pLog);\n" +
                "rsp = ngenxx_net_http_requestL(pReq);\n" +
                "return rsp;\n" +
                "end"
            );

            let jsonParams = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"header_v\":[\"Cache-Control: no-cache\"], \"header_c\": 1, \"timeout\":6666}";
            let rsp = NGenXX.lCall("lNetHttpRequest", jsonParams);
            /*let rsp = NGenXX.netHttpRequest("https://rinc.xyz",
              "p0=1&p1=2&p2=3",
              0,
              ["Cache-Control: no-cache"],
              5555
            )*/
            NGenXX.logPrint(1, `${rsp}`);

            let jsonDecoder = NGenXX.jsonDecoderInit(jsonParams)
            if (jsonDecoder > 0) {
              let urlNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "url")
              if (urlNode > 0) {
                let url = NGenXX.jsonDecoderReadString(jsonDecoder, urlNode)
                NGenXX.logPrint(1, `url:${url}`)
              }
              let headerCNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_c")
              if (headerCNode > 0) {
                let headersC = NGenXX.jsonDecoderReadNumber(jsonDecoder, headersCNode)
                NGenXX.logPrint(1, `headersC:${headersC}`)
              }
              let headerVNode = NGenXX.jsonDecoderReadNode(jsonDecoder, 0, "header_v")
              if (headerVNode > 0) {
                let headerNode = NGenXX.jsonDecoderReadChild(jsonDecoder, headersVNode)
                while (headerNode > 0) {
                  let header = NGenXX.jsonDecoderReadString(jsonDecoder, headerNode)
                  NGenXX.logPrint(1, `header:${header}`)
                  headerNode = NGenXX.jsonDecoderReadNext(jsonDecoder, headerNode)
                }
              }
              NGenXX.jsonDecoderRelease(jsonDecoder)
            }

            //NGenXX.release();
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
