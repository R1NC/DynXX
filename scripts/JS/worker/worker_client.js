function worker_client_main()
{
    var worker = new os.Worker("./worker_server.js");

    worker.onmessage = function (msg) {
        var data = msg.data;
        print("WorkerClient recv:", JSON.stringify(data));
        switch(data.type) {
        case "report":
            DynXXLogPrint(3, `Main Thread received report request: ${data.payload}`);
            jTestNetHttpReqPro('https://qq.com')
                .then(function (res) {
                    worker.postMessage({ type: "reportResult", payload: res });
                }, function (err) {
                    DynXXLogPrint(6, `${err}`);
                });
            break;
        }
    };
}
