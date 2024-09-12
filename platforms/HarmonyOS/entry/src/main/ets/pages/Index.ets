import { hilog } from '@kit.PerformanceAnalysisKit';
import ngenxxNapi from 'libngenxx.so';

@Entry
@Component
struct Index {
  @State message: string = 'Click to test NAPI..';
  @State handle: number = -1;

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(23)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            if (this.handle == -1) {
              this.handle = ngenxxNapi.init();

              ngenxxNapi.logSetCallback((level, log) => {
                hilog.debug(0x0000, 'NGenXX', `${level} | ${log}`);
              });

              ngenxxNapi.lLoadS(this.handle,
                "function lNetHttpRequest(pReq)\n" +
                "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "ngenxx_log_printL(pLog);\n" +
                "rsp = ngenxx_net_http_requestL(pReq);\n" +
                "return rsp;\n" +
                "end"
              );
            }

            //let params: string = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
            //let rsp: number = ngenxxNapi.lCall(this.handle, "lNetHttpRequest", params);
            let rsp = ngenxxNapi.netHttpRequest("https://rinc.xyz",
              "p0=1&p1=2&p2=3",
              0,
              ["Cache-Control: no-cache"],
              1,
              5555
            )

            ngenxxNapi.logPrint(1, `${rsp}`);

            //ngenxxNapi.release(this.handle);
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
