import { readFileSync, writeFileSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";

type AttrMap = Record<string, string>;

type CaseResult = {
    suite: string;
    name: string;
    timeMs: number;
    status: "passed" | "failed" | "skipped";
    message: string;
};

function parseAttrs(raw: string): AttrMap {
    const attrs: AttrMap = {};
    const re = /([A-Za-z0-9_:-]+)="([^"]*)"/g;
    let m: RegExpExecArray | null;
    while ((m = re.exec(raw)) !== null) {
        attrs[m[1]] = m[2];
    }
    return attrs;
}

function escapeHtml(s: string): string {
    return s
        .replaceAll("&", "&amp;")
        .replaceAll("<", "&lt;")
        .replaceAll(">", "&gt;")
        .replaceAll('"', "&quot;")
        .replaceAll("'", "&#39;");
}

function decodeXml(s: string): string {
    return s
        .replaceAll("&lt;", "<")
        .replaceAll("&gt;", ">")
        .replaceAll("&quot;", "\"")
        .replaceAll("&apos;", "'")
        .replaceAll("&amp;", "&");
}

function toMs(timeSec: string | undefined): number {
    const v = Number(timeSec ?? "0");
    if (!Number.isFinite(v)) {
        return 0;
    }
    return Math.round(v * 1000);
}

function parseJunitXml(xml: string): { total: number; failed: number; passed: number; skipped: number; cases: CaseResult[] } {
    const cases: CaseResult[] = [];
    const suiteRe = /<testsuite\b([^>]*)>([\s\S]*?)<\/testsuite>/g;
    let suiteMatch: RegExpExecArray | null;
    while ((suiteMatch = suiteRe.exec(xml)) !== null) {
        const suiteAttrs = parseAttrs(suiteMatch[1]);
        const suiteName = suiteAttrs.name ?? "UnknownSuite";
        const suiteBody = suiteMatch[2];
        const caseRe = /<testcase\b([^>]*?)(?:\/>|>([\s\S]*?)<\/testcase>)/g;
        let caseMatch: RegExpExecArray | null;
        while ((caseMatch = caseRe.exec(suiteBody)) !== null) {
            const caseAttrs = parseAttrs(caseMatch[1]);
            const body = caseMatch[2] ?? "";
            const failed = /<(failure|error)\b/.test(body);
            const skipped = /<skipped\b/.test(body) || caseAttrs.status === "notrun";
            let message = "";
            if (failed || skipped) {
                const msgMatch = /<(?:failure|error)\b([^>]*)>([\s\S]*?)<\/(?:failure|error)>/.exec(body);
                if (msgMatch) {
                    const failureAttrs = parseAttrs(msgMatch[1]);
                    const attrMessage = failureAttrs.message ?? "";
                    const bodyMessage = decodeXml(msgMatch[2].trim());
                    message = `${attrMessage}${attrMessage && bodyMessage ? "\n" : ""}${bodyMessage}`.trim();
                } else if (skipped) {
                    const skippedMatch = /<skipped\b([^>]*)\/?>/.exec(body);
                    if (skippedMatch) {
                        const skippedAttrs = parseAttrs(skippedMatch[1] ?? "");
                        message = (skippedAttrs.message ?? "").trim();
                    }
                }
            }
            let status: CaseResult["status"] = "passed";
            if (failed) {
                status = "failed";
            } else if (skipped) {
                status = "skipped";
            }
            cases.push({
                suite: suiteName,
                name: caseAttrs.name ?? "UnknownCase",
                timeMs: toMs(caseAttrs.time),
                status,
                message
            });
        }
    }
    const total = cases.length;
    const failed = cases.filter(c => c.status === "failed").length;
    const skipped = cases.filter(c => c.status === "skipped").length;
    return { total, failed, skipped, passed: total - failed - skipped, cases };
}

function buildHtml(title: string, data: { total: number; failed: number; passed: number; skipped: number; cases: CaseResult[] }): string {
    const suiteRowSpan = new Map<string, number>();
    for (const c of data.cases) {
        suiteRowSpan.set(c.suite, (suiteRowSpan.get(c.suite) ?? 0) + 1);
    }

    const renderedSuite = new Set<string>();
    const rows = data.cases.map(c => {
        let statusClass = "ok";
        if (c.status === "failed") {
            statusClass = "fail";
        } else if (c.status === "skipped") {
            statusClass = "skip";
        }
        const message = c.message ? `<pre>${escapeHtml(c.message)}</pre>` : "";
        const suiteCell = renderedSuite.has(c.suite)
            ? ""
            : `<td rowspan="${suiteRowSpan.get(c.suite) ?? 1}">${escapeHtml(c.suite)}</td>`;
        renderedSuite.add(c.suite);
        return `<tr>
${suiteCell}
<td>${escapeHtml(c.name)}</td>
<td class="${statusClass}">${c.status}</td>
<td>${c.timeMs}</td>
<td>${message}</td>
</tr>`;
    }).join("\n");

    return `<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>${escapeHtml(title)}</title>
<style>
body{font-family:Segoe UI,Arial,sans-serif;margin:24px;color:#1f2937}
h1{margin:0 0 12px}
.summary{display:flex;gap:12px;margin:0 0 18px}
.card{padding:10px 14px;border-radius:8px;background:#f3f4f6}
.ok{color:#166534;font-weight:600}
.fail{color:#991b1b;font-weight:600}
.skip{color:#92400e;font-weight:600}
table{width:100%;border-collapse:collapse}
th,td{border:1px solid #d1d5db;padding:8px;vertical-align:top}
th{background:#f9fafb;text-align:left}
pre{white-space:pre-wrap;margin:0;font-family:Consolas,monospace;font-size:12px}
</style>
</head>
<body>
<h1>${escapeHtml(title)}</h1>
<div class="summary">
<div class="card">Total: <strong>${data.total}</strong></div>
<div class="card">Passed: <strong class="ok">${data.passed}</strong></div>
<div class="card">Failed: <strong class="fail">${data.failed}</strong></div>
<div class="card">Skipped: <strong class="skip">${data.skipped}</strong></div>
</div>
<table>
<thead>
<tr><th>Suite</th><th>Case</th><th>Status</th><th>Time(ms)</th><th>Message</th></tr>
</thead>
<tbody>
${rows}
</tbody>
</table>
</body>
</html>`;
}

export function renderGtestXmlToHtml(inputXmlPath: string, outputHtmlPath: string): void {
    const xml = readFileSync(resolve(inputXmlPath), "utf8");
    const data = parseJunitXml(xml);
    mkdirSync(dirname(resolve(outputHtmlPath)), { recursive: true });
    writeFileSync(resolve(outputHtmlPath), buildHtml("DynXX GTest Report", data), "utf8");
}

if (process.argv[1]?.toLowerCase().endsWith("gen-test-report.ts")) {
    const input = process.argv[2] ?? "";
    const output = process.argv[3] ?? "";
    if (!input || !output) {
        throw new Error("Usage: tsx gen-test-report.ts <input-xml> <output-html>");
    }
    renderGtestXmlToHtml(input, output);
}
