import { copyFileSync, existsSync, mkdirSync, readFileSync, readdirSync, statSync, writeFileSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';

// Renders the two observability artifacts into self-contained HTML reports:
//
//   <build>/profiling-configure.json      --profiling-format=google-trace output of a
//                                        Debug configure (B/E events of every CMake
//                                        script command, each with file:line + args)
//                                        -> <build>/profiling-configure.html
//   <build>/.cmake/instrumentation/v1/data/trace/*.json
//                                        CMake Instrumentation API output of a Debug
//                                        build (one event per compile/link/... command)
//                                        -> <build>/instrumentation.html
//
// The JSON stays authoritative (grep it, open it in Perfetto); these reports answer the
// two questions that are painful to read out of 5 MB of JSON by hand: which CMake code
// costs the configure time, and which files cost the build time.
//
// Reached either as a library (the build scripts call the functions below through
// tools/build/build-utils.ts) or as the `npm run gen:trace` entry through
// tools/gen-trace.ts - this module has no CLI of its own.

type TraceEvent = {
  cat?: string;
  name?: string;
  ph?: string;
  pid?: number;
  tid?: number;
  ts?: number;
  dur?: number;
  args?: { location?: string; functionArgs?: string };
};

type Span = {
  name: string;
  cat: string;
  ts: number;
  dur: number;
  self: number;
  location: string;
  args: string;
};

function escapeHtml(text: string): string {
  return text
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;');
}

function formatDuration(us: number): string {
  if (us >= 1_000_000) {
    return `${(us / 1_000_000).toFixed(2)}s`;
  }
  if (us >= 1_000) {
    return `${(us / 1_000).toFixed(1)}ms`;
  }
  return `${Math.round(us)}us`;
}

function readTraceJson(path: string): TraceEvent[] {
  const raw = JSON.parse(readFileSync(resolve(path), 'utf8')) as TraceEvent[] | { traceEvents?: TraceEvent[] };
  return Array.isArray(raw) ? raw : (raw.traceEvents ?? []);
}

// Both artifacts carry absolute paths of one machine; strip what every entry shares so the
// tables show `src/core/log/Log.cxx` instead of `/workspace/build.Linux/Debug/src/...`.
function commonDirectoryPrefix(paths: string[]): string {
  const usable = paths.filter((p) => p.includes('/'));
  if (usable.length === 0) {
    return '';
  }
  let prefix = usable[0].slice(0, usable[0].lastIndexOf('/') + 1);
  for (const path of usable) {
    while (prefix.length > 0 && !path.startsWith(prefix)) {
      prefix = prefix.slice(0, Math.max(0, prefix.lastIndexOf('/', prefix.length - 2) + 1));
    }
    if (prefix.length === 0) {
      return '';
    }
  }
  return prefix.length > 1 ? prefix : '';
}

function shorten(path: string, prefix: string): string {
  return prefix.length > 0 && path.startsWith(prefix) ? path.slice(prefix.length) : path;
}

// A cell is either bare content (table() wraps it in <td>) or content plus a class for the
// numeric/bar columns. Call sites used to return finished `<td>` elements and forgot the
// wrapper on the leading cells, which pushed them out of the table entirely - the wrapper
// lives here so that cannot happen again.
type Cell = string | { html: string; className?: string };

function cellHtml(cell: Cell): string {
  return typeof cell === 'string'
    ? `<td>${cell}</td>`
    : `<td${cell.className ? ` class="${cell.className}"` : ''}>${cell.html}</td>`;
}

function table(headers: string[], rows: Cell[][]): string {
  const head = headers.map((h) => `<th>${escapeHtml(h)}</th>`).join('');
  const body = rows
    .map((row) => {
      // These tables use no rowspan, so a row with the wrong cell count would shift every
      // following column (the bug this helper replaced produced cells outside the table
      // entirely, which browsers silently render as run-on text). Fail loudly instead.
      if (row.length !== headers.length) {
        throw new Error(`trace report: row has ${row.length} cells, table has ${headers.length} columns`);
      }
      return `<tr>${row.map(cellHtml).join('')}</tr>`;
    })
    .join('\n');
  return `<table>\n<thead><tr>${head}</tr></thead>\n<tbody>\n${body}\n</tbody>\n</table>`;
}

// Bar content only - table() supplies the surrounding `<td class="bar">`.
function bar(value: number, max: number): string {
  const ratio = max > 0 ? Math.max(1, Math.round((value / max) * 100)) : 0;
  return `<span style="width:${ratio}%"></span>`;
}

function page(title: string, cards: Array<[string, string]>, sections: string[]): string {
  const cardsHtml = cards
    .map(([label, value]) => `<div class="card">${escapeHtml(label)}: <strong>${escapeHtml(value)}</strong></div>`)
    .join('\n');
  return `<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>${escapeHtml(title)}</title>
<style>
body{font-family:Segoe UI,Arial,sans-serif;margin:24px;color:#1f2937}
h1{margin:0 0 12px}
h2{margin:26px 0 8px;font-size:18px}
.summary{display:flex;flex-wrap:wrap;gap:12px;margin:0 0 18px}
.card{padding:10px 14px;border-radius:8px;background:#f3f4f6}
table{width:100%;border-collapse:collapse}
th,td{border:1px solid #d1d5db;padding:6px 8px;vertical-align:top;font-size:13px}
th{background:#f9fafb;text-align:left}
td.num{text-align:right;white-space:nowrap;font-variant-numeric:tabular-nums}
td.bar{padding:0;background:#e5e7eb;min-width:80px}
td.bar>span{display:block;height:20px;background:#3b82f6}
code{font-family:Consolas,monospace;font-size:12px}
.muted{color:#6b7280}
</style>
</head>
<body>
<h1>${escapeHtml(title)}</h1>
<div class="summary">
${cardsHtml}
</div>
${sections.join('\n')}
</body>
</html>`;
}

// ---------------------------------------------------------------- configure profiling

// The profiling format pairs `ph:"B"` and `ph:"E"` events per thread, so durations have to
// be reconstructed; `self` subtracts the children, which is what makes the aggregation by
// location meaningful (the root `configure` span would otherwise swallow everything).
function pairProfilingSpans(events: TraceEvent[]): Span[] {
  const stacks = new Map<string, Span[]>();
  const spans: Span[] = [];
  for (const event of events) {
    const key = `${event.pid ?? 0}:${event.tid ?? 0}`;
    const stack = stacks.get(key) ?? [];
    stacks.set(key, stack);
    if (event.ph === 'B') {
      stack.push({
        name: event.name ?? '(unnamed)',
        cat: event.cat ?? '',
        ts: event.ts ?? 0,
        dur: 0,
        self: 0,
        location: event.args?.location ?? '',
        args: event.args?.functionArgs ?? ''
      });
    } else if (event.ph === 'E') {
      const open = stack.pop();
      if (!open) {
        continue;
      }
      open.dur = (event.ts ?? 0) - open.ts;
      open.self = open.dur;
      const parent = stack[stack.length - 1];
      if (parent) {
        parent.self -= open.dur;
      }
      spans.push(open);
    }
  }
  return spans;
}

export function buildConfigureProfilingHtml(tracePath: string): string {
  const spans = pairProfilingSpans(readTraceJson(tracePath));
  const prefix = commonDirectoryPrefix(spans.map((s) => s.location));
  const start = Math.min(...spans.map((s) => s.ts));
  const end = Math.max(...spans.map((s) => s.ts + s.dur));
  const located = spans.filter((s) => s.location.length > 0);

  const byLocation = new Map<string, { self: number; incl: number; count: number }>();
  for (const span of located) {
    const entry = byLocation.get(span.location) ?? { self: 0, incl: 0, count: 0 };
    entry.self += span.self;
    entry.incl += span.dur;
    entry.count += 1;
    byLocation.set(span.location, entry);
  }
  const locationRows = [...byLocation.entries()].sort((a, b) => b[1].self - a[1].self).slice(0, 40);
  const locationMax = locationRows.length > 0 ? locationRows[0][1].self : 0;

  const byName = new Map<string, { self: number; incl: number; count: number }>();
  for (const span of spans) {
    const entry = byName.get(span.name) ?? { self: 0, incl: 0, count: 0 };
    entry.self += span.self;
    entry.incl += span.dur;
    entry.count += 1;
    byName.set(span.name, entry);
  }
  const nameRows = [...byName.entries()].sort((a, b) => b[1].self - a[1].self).slice(0, 25);

  const slowest = [...spans].sort((a, b) => b.self - a.self).slice(0, 25);

  const sections = [
    `<h2>Aggregated by location (self time)</h2>
${table(['Location', 'Self', '', 'Inclusive', 'Calls'], locationRows.map(([location, entry]) => [
      `<code>${escapeHtml(shorten(location, prefix))}</code>`,
      { html: formatDuration(entry.self), className: 'num' },
      { html: bar(entry.self, locationMax), className: 'bar' },
      { html: formatDuration(entry.incl), className: 'num muted' },
      { html: String(entry.count), className: 'num' }
    ]))}`,
    `<h2>Slowest single invocations (self time)</h2>
${table(['Command', 'Location', 'Arguments', 'Self'], slowest.map((span) => [
      `<code>${escapeHtml(span.name)}</code>`,
      `<code class="muted">${escapeHtml(shorten(span.location, prefix))}</code>`,
      `<span class="muted">${escapeHtml(span.args.slice(0, 90))}</span>`,
      { html: formatDuration(span.self), className: 'num' }
    ]))}`,
    `<h2>Aggregated by command name</h2>
${table(['Command', 'Self', 'Inclusive', 'Calls'], nameRows.map(([name, entry]) => [
      `<code>${escapeHtml(name)}</code>`,
      { html: formatDuration(entry.self), className: 'num' },
      { html: formatDuration(entry.incl), className: 'num muted' },
      { html: String(entry.count), className: 'num' }
    ]))}`
  ];

  return page('DynXX Configure Timings', [
    ['Source', tracePath.split(/[\\/]/).pop() ?? tracePath],
    ['Events', String(spans.length)],
    ['Elapsed', formatDuration(end - start)],
    ['Files', String(new Set(located.map((s) => s.location.split(':')[0])).size)]
  ], sections);
}

export function renderConfigureProfilingReport(buildFolder: string): string | undefined {
  const tracePath = join(buildFolder, 'profiling-configure.json');
  if (!existsSync(resolve(tracePath))) {
    return undefined;
  }
  const htmlPath = join(buildFolder, 'profiling-configure.html');
  mkdirSync(dirname(resolve(htmlPath)), { recursive: true });
  writeFileSync(resolve(htmlPath), buildConfigureProfilingHtml(tracePath), 'utf8');
  return htmlPath;
}

// ------------------------------------------------------------- build instrumentation

export function findInstrumentationTrace(buildFolder: string): string | undefined {
  const traceDir = join(buildFolder, '.cmake', 'instrumentation', 'v1', 'data', 'trace');
  if (!existsSync(resolve(traceDir))) {
    return undefined;
  }
  const traces = readdirSync(resolve(traceDir))
    .filter((name) => name.endsWith('.json'))
    .map((name) => join(traceDir, name))
    .sort((a, b) => statSync(resolve(b)).mtimeMs - statSync(resolve(a)).mtimeMs);
  return traces[0];
}

export function buildInstrumentationHtml(tracePath: string): string {
  const events = readTraceJson(tracePath).filter((event) => (event.dur ?? 0) > 0);
  const start = Math.min(...events.map((e) => e.ts ?? 0));
  const end = Math.max(...events.map((e) => (e.ts ?? 0) + (e.dur ?? 0)));

  // Event names look like `compile: /abs/path/file.cxx`; keep the detail only when present.
  const detailOf = (name: string): string => name.replace(/^[a-z]+:\s*/, '');
  const prefix = commonDirectoryPrefix(events.map((e) => detailOf(e.name ?? '')));

  const byCategory = new Map<string, number>();
  for (const event of events) {
    byCategory.set(event.cat ?? '(none)', (byCategory.get(event.cat ?? '(none)') ?? 0) + (event.dur ?? 0));
  }
  const categoryRows = [...byCategory.entries()].sort((a, b) => b[1] - a[1]);
  const categoryMax = categoryRows.length > 0 ? categoryRows[0][1] : 0;
  // Command durations are summed, so the total exceeds the elapsed wall time whenever jobs
  // run in parallel; the share is therefore relative to the sum, not to the elapsed time.
  const categorySum = categoryRows.reduce((sum, [, total]) => sum + total, 0);

  const byDirectory = new Map<string, { total: number; count: number }>();
  for (const event of events) {
    const detail = detailOf(event.name ?? '');
    if (!detail.includes('/')) {
      // Phase spans (configure/cmakeBuild/generate/link) carry no source file.
      continue;
    }
    const cut = detail.lastIndexOf('/');
    const dir = cut > 0 ? shorten(detail.slice(0, cut), prefix) : shorten(detail, prefix);
    const entry = byDirectory.get(dir) ?? { total: 0, count: 0 };
    entry.total += event.dur ?? 0;
    entry.count += 1;
    byDirectory.set(dir, entry);
  }
  const directoryRows = [...byDirectory.entries()].sort((a, b) => b[1].total - a[1].total).slice(0, 30);
  const directoryMax = directoryRows.length > 0 ? directoryRows[0][1].total : 0;

  const slowest = [...events].sort((a, b) => (b.dur ?? 0) - (a.dur ?? 0)).slice(0, 30);

  const sections = [
    `<h2>By category</h2>
<p class="muted">Durations are summed per command; parallel jobs overlap, so the total can exceed Elapsed. Share is relative to the sum of all categories.</p>
${table(['Category', 'Total', '', 'Share'], categoryRows.map(([cat, total]) => [
      `<code>${escapeHtml(cat)}</code>`,
      { html: formatDuration(total), className: 'num' },
      { html: bar(total, categoryMax), className: 'bar' },
      { html: `${categorySum > 0 ? ((total / categorySum) * 100).toFixed(0) : '0'}%`, className: 'num muted' }
    ]))}`,
    `<h2>Slowest commands</h2>
${table(['Category', 'Command', '', 'Duration'], slowest.map((event) => [
      `<code>${escapeHtml(event.cat ?? '')}</code>`,
      `<code>${escapeHtml(shorten(detailOf(event.name ?? ''), prefix))}</code>`,
      { html: bar(event.dur ?? 0, slowest[0] ? (slowest[0].dur ?? 0) : 0), className: 'bar' },
      { html: formatDuration(event.dur ?? 0), className: 'num' }
    ]))}`,
    `<h2>Aggregated by directory</h2>
${table(['Directory', 'Total', '', 'Commands'], directoryRows.map(([dir, entry]) => [
      `<code>${escapeHtml(dir)}</code>`,
      { html: formatDuration(entry.total), className: 'num' },
      { html: bar(entry.total, directoryMax), className: 'bar' },
      { html: String(entry.count), className: 'num' }
    ]))}`
  ];

  return page('DynXX Build Timings', [
    ['Source', tracePath.split(/[\\/]/).pop() ?? tracePath],
    ['Commands', String(events.length)],
    ['Elapsed', formatDuration(end - start)]
  ], sections);
}

export function renderInstrumentationReport(buildFolder: string): string | undefined {
  const tracePath = findInstrumentationTrace(buildFolder);
  if (!tracePath) {
    return undefined;
  }
  const htmlPath = join(buildFolder, 'instrumentation.html');
  writeFileSync(resolve(htmlPath), buildInstrumentationHtml(tracePath), 'utf8');
  return htmlPath;
}

// Why a report is missing, read out of the build tree instead of guessed: CMakeCache.txt
// records the CMake that wrote it (CMAKE_CACHE_*_VERSION), the generator and the option, so
// the placeholder can name the actual blocker ("CMake 3.28.3 configured this tree, the API
// was added in 4.3") rather than only restating the requirement.
type BuildFacts = {
  configured: boolean;
  cmakeVersion: string;
  generator: string;
  instrumentationOption: string;
};

function cacheValue(cache: string, key: string): string {
  return new RegExp(`^${key}:[^=\\r\\n]*=([^\\r\\n]*)`, 'm').exec(cache)?.[1]?.trim() ?? '';
}

function readBuildFacts(buildFolder: string): BuildFacts {
  const cachePath = join(buildFolder, 'CMakeCache.txt');
  if (!existsSync(resolve(cachePath))) {
    return { configured: false, cmakeVersion: '', generator: '', instrumentationOption: '' };
  }
  const cache = readFileSync(resolve(cachePath), 'utf8');
  const major = cacheValue(cache, 'CMAKE_CACHE_MAJOR_VERSION');
  const minor = cacheValue(cache, 'CMAKE_CACHE_MINOR_VERSION');
  const patch = cacheValue(cache, 'CMAKE_CACHE_PATCH_VERSION');
  return {
    configured: true,
    cmakeVersion: major && minor ? `${major}.${minor}.${patch || '0'}` : '',
    generator: cacheValue(cache, 'CMAKE_GENERATOR'),
    instrumentationOption: cacheValue(cache, 'DYNXX_ENABLE_BUILD_INSTRUMENTATION')
  };
}

function versionAtLeast(version: string, major: number, minor: number): boolean {
  const parts = version.split('.').map((part) => Number.parseInt(part, 10));
  if (parts.length < 2 || parts.some(Number.isNaN)) {
    return false;
  }
  return parts[0] > major || (parts[0] === major && parts[1] >= minor);
}

function explainMissingConfigure(facts: BuildFacts): string[] {
  if (!facts.configured) {
    return ['This directory has no CMakeCache.txt, so nothing was configured here.'];
  }
  return [
    'No profiling-configure.json was written. runCMake() passes --profiling-format=google-trace unless DYNXX_ENABLE_CONFIGURE_PROFILING=0, so this tree was either configured outside the build scripts or that variable was set.',
    `Configure profiling is available since CMake 3.18${facts.cmakeVersion ? `; this tree was configured with ${facts.cmakeVersion}` : ''}.`
  ];
}

function explainMissingInstrumentation(facts: BuildFacts): string[] {
  if (!facts.configured) {
    return ['This directory has no CMakeCache.txt, so nothing was configured here.'];
  }
  const reasons: string[] = [];
  if (facts.cmakeVersion && !versionAtLeast(facts.cmakeVersion, 4, 3)) {
    reasons.push(`CMake ${facts.cmakeVersion} configured this build tree, and the CMake Instrumentation API was added in 4.3.`);
  }
  if (facts.instrumentationOption.toUpperCase() === 'OFF') {
    reasons.push('DYNXX_ENABLE_BUILD_INSTRUMENTATION is OFF in this build tree.');
  }
  if (facts.generator && !/(Ninja|Makefiles|FASTBuild)/.test(facts.generator)) {
    reasons.push(`The generator is "${facts.generator}", and only Makefiles/Ninja/FASTBuild builds can be instrumented.`);
  }
  if (reasons.length === 0) {
    reasons.push(`CMake ${facts.cmakeVersion || '(unknown)'} with generator "${facts.generator || '(unknown)'}" supports instrumentation, but this build produced no data.`);
  }
  return reasons;
}

// Writes the timing reports a build produced into <siteDir>/<name>/index.html. A report the
// runner could not produce (its CMake cannot instrument the build) is replaced by a page
// that says why, so the portal can link both entries unconditionally.
export function publishTimingReports(buildFolder: string, siteDir: string): string[] {
  const reports = [
    {
      name: 'configure',
      title: 'DynXX Configure Timings',
      render: renderConfigureProfilingReport,
      about: 'Produced by runCMake() with --profiling-format=google-trace (CMake >= 3.18).',
      explain: explainMissingConfigure
    },
    {
      name: 'instruments',
      title: 'DynXX Build Timings',
      render: renderInstrumentationReport,
      about: 'Produced by the CMake Instrumentation API (CMake >= 4.3 with a Makefiles/Ninja/FASTBuild generator and DYNXX_ENABLE_BUILD_INSTRUMENTATION=ON).',
      explain: explainMissingInstrumentation
    }
  ];

  const written: string[] = [];
  for (const report of reports) {
    const reportDir = join(siteDir, report.name);
    mkdirSync(resolve(reportDir), { recursive: true });
    const indexPath = join(reportDir, 'index.html');
    const reportPath = report.render(buildFolder);
    if (reportPath) {
      copyFileSync(resolve(reportPath), resolve(indexPath));
    } else {
      const facts = readBuildFacts(buildFolder);
      const reasons = report.explain(facts)
        .map((reason) => `<li>${escapeHtml(reason)}</li>`)
        .join('\n');
      writeFileSync(
        resolve(indexPath),
        page(
          report.title,
          [
            ['Status', 'not produced'],
            ['CMake', facts.cmakeVersion || '(unknown)'],
            ['Generator', facts.generator || '(unknown)'],
            ['Build tree', buildFolder]
          ],
          [`<p class="muted">${escapeHtml(report.about)}</p>`, `<ul>\n${reasons}\n</ul>`]
        ),
        'utf8'
      );
    }
    written.push(indexPath);
  }
  return written;
}
