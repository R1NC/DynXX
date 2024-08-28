import { hilog } from '@kit.PerformanceAnalysisKit';
import enginexxNapi from 'libenginexx.so';

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
            enginexxNapi.logSetCallback((level, log) => {
              hilog.debug(0x0000, 'EngineXX', `${level} | ${log}`);
            });
            let lstate: number = enginexxNapi.lCreate();
            let ret: number = enginexxNapi.lLoadS(lstate,
              "function lNetHttpReq(pReq)\n" +
                "pLog = '{\"level\":3,\"content\":\"Send Net Req from Lua..\"}';\n" +
                "enginexx_log_printL(pLog);\n" +
                "rsp = enginexx_net_http_reqL(pReq);\n" +
                "return rsp;\n" +
                "end"
            );
            if (ret == 0) {
              let params: string = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
              let rsp: number = enginexxNapi.lCall(lstate, "lNetHttpReq", params);
              hilog.info(0x0000, 'EngineXX', `${rsp}`);
            }
            enginexxNapi.lDestroy(lstate);
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
