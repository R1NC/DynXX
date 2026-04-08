const server = os.Worker.parent;

function handle_msg(msg) {
    const data = msg.data;
    print("WorkerServer recv:", JSON.stringify(data));
    if (data.type === "reportResult") {
        print(`WorkerServer received report result: ${data.payload}`);
    }
}

function worker_server_main() {
    server.onmessage = handle_msg;
    let i = 0;
    os.setInterval(()=>{
        print(`WorkerServer Timer triggered report -> ${i++}`);
        server.postMessage({ type: "report", payload: i });
    }, 3000);
}

worker_server_main();
