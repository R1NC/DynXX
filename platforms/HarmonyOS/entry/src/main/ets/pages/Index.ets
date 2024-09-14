import { hilog } from '@kit.PerformanceAnalysisKit';
import ngenxxNapi from 'libngenxx.so';

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
            ngenxxNapi.init(root);

            ngenxxNapi.logSetCallback((level, log) => {
              hilog.debug(0x0000, 'NGenXX', `${level} | ${log}`);
            });

            let dbConn = ngenxxNapi.storeSQLiteOpen("test");
            if (dbConn > 0) {
              let insertSql = "CREATE TABLE IF NOT EXISTS TestTable (_id INTEGER PRIMARY KEY AUTOINCREMENT, platform TEXT, vendor TEXT);\n" +
                "INSERT OR IGNORE INTO TestTable (platform, vendor)\n" +
                "VALUES\n" +
                "('iOS','Apple'),\n" +
                "('Android','Google'),\n" +
                "('HarmonyOS','Huawei');";
              if (ngenxxNapi.storeSQLiteExecute(dbConn, insertSql)) {
                let querySql = "SELECT * FROM TestTable;";
                let queryResult = ngenxxNapi.storeSQLiteQueryDo(dbConn, querySql);
                if (queryResult > 0) {
                  while(ngenxxNapi.storeSQLiteQueryReadRow(queryResult)) {
                    let platform = ngenxxNapi.storeSQLiteQueryReadColumnText(queryResult, "platform");
                    ngenxxNapi.logPrint(1, `platform->${platform}`);
                  }
                  ngenxxNapi.storeSQLiteQueryDrop(queryResult);
                }
                ngenxxNapi.storeSQLiteClose(dbConn);
              }
            }

            let kvConn = ngenxxNapi.storeKVOpen("test");
            if (kvConn > 0) {
              ngenxxNapi.storeKVWriteString(kvConn, "s", "NGexXX");
              let s = ngenxxNapi.storeKVReadString(kvConn, "s");
              ngenxxNapi.logPrint(1, `s->${s}`);
              ngenxxNapi.storeKVWriteInteger(kvConn, "i", 1234567890);
              let i = ngenxxNapi.storeKVReadInteger(kvConn, "i");
              ngenxxNapi.logPrint(1, `i->${i}`);
              ngenxxNapi.storeKVWriteFloat(kvConn, "f", 0.123456789);
              let f = ngenxxNapi.storeKVReadFloat(kvConn, "f");
              ngenxxNapi.logPrint(1, `f->${f}`);
              ngenxxNapi.storeKVClose(kvConn);
            }

            ngenxxNapi.lLoadS(
              "function lNetHttpRequest(pReq)\n" +
                "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "ngenxx_log_printL(pLog);\n" +
                "rsp = ngenxx_net_http_requestL(pReq);\n" +
                "return rsp;\n" +
                "end"
            );

            let params: string = "{\"url\":\"https://rinc.xyz\", \"params\":\"p0=1&p1=2&p2=3\", \"method\":0, \"headers_v\":[\"Cache-Control: no-cache\"], \"headers_c\": 1, \"timeout\":6666}";
            let rsp = ngenxxNapi.lCall("lNetHttpRequest", params);
            /*let rsp = ngenxxNapi.netHttpRequest("https://rinc.xyz",
              "p0=1&p1=2&p2=3",
              0,
              ["Cache-Control: no-cache"],
              5555
            )*/
            ngenxxNapi.logPrint(1, `${rsp}`);

            //ngenxxNapi.release();
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
