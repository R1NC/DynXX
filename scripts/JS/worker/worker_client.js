function worker_client_main()
{
    const worker = new os.Worker("./worker_server.js");

    worker.onmessage = function (msg) {
        const data = msg.data;
        print("WorkerClient recv:", JSON.stringify(data));
        if (data.type === "report") {
            DynXXLogPrint(3, `Main Thread received report request: ${data.payload}`);
            jTestNetHttpReqPro('https://qq.com')
                .then(function (res) {
                    worker.postMessage({ type: "reportResult", payload: res });
                }, function (err) {
                    DynXXLogPrint(6, `${err}`);
                });
        }
    };
}
