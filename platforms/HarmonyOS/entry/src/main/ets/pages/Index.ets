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
            let lstate: number = enginexxNapi.lCreate();
            let ret: number = enginexxNapi.lLoadS(lstate,
              "function lNetHttpReq(p) " +
              "return enginexx_net_http_reqL(p) " +
              "end"
            );
            if (ret == 0) {
              let params: string = "{\"url\":\"https://rinc.xyz\", \"params\":\"\"}";
              let rsp: number = enginexxNapi.lCall(lstate, "lNetHttpReq", params);
              hilog.info(0x0000, 'EngineXX', 'Http rsp: %{public}d', rsp);
            }
            enginexxNapi.lDestroy(lstate);
          })
      }
      .width('100%')
    }
    .height('100%')
  }
}
