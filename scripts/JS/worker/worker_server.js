var server = os.Worker.parent;

function handle_msg(msg) {
    var data = msg.data;
    print("WorkerServer recv:", JSON.stringify(data));
    switch(data.type) {
    case "reportResult":
        print(`WorkerServer received report result: ${data.payload}`);
        break;
    }
}

function worker_server_main() {
    server.onmessage = handle_msg;
    var i = 0;
    os.setInterval(()=>{
        print(`WorkerServer Timer triggered report -> ${i++}`);
        server.postMessage({ type: "report", payload: i });
    }, 3000);
}

worker_server_main();
