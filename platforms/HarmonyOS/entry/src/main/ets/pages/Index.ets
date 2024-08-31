import { hilog } from '@kit.PerformanceAnalysisKit';
import ngenxxNapi from 'libngenxx.so';

@Entry
@Component
struct Index {
  @State message: string = 'Click to test NAPI..';
  @State lstate: number = -1;

  build() {
    Row() {
      Column() {
        Text(this.message)
          .fontSize(23)
          .fontWeight(FontWeight.Bold)
          .onClick(() => {
            ngenxxNapi.logSetCallback((level, log) => {
              hilog.debug(0x0000, 'NGenXX', `${level} | ${log}`);
            });
            if (this.lstate == -1) this.lstate = ngenxxNapi.lCreate();
            let ret: number = ngenxxNapi.lLoadS(this.lstate,
              "function lNetHttpReq(pReq)\n" +
                "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "ngenxx_log_printL(pLog);\n" +
                "rsp = ngenxx_net_http_reqL(pReq);\n" +
                "return rsp;\n" +
                "end"
            );
            if (ret == 0) {
              let params: string = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
              let rsp: number = ngenxxNapi.lCall(this.lstate, "lNetHttpReq", params);
              hilog.info(0x0000, 'NGenXX', `${rsp}`);
            }
            //ngenxxNapi.lDestroy(lstate);
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
