import { publishObservabilitySite, renderConfigureProfilingReport, renderInstrumentationReport } from './trace/trace-utils.js';

// Entry point for the observability reports (`npm run gen:trace`), shaped like the other
// entry scripts (gen-doc.ts, build-*.ts): parse argv, call the utils module, report what
// happened.
//
//   npm run gen:trace -- <build-dir>              render next to the artifacts
//   npm run gen:trace -- <build-dir> <site-dir>   also assemble a publishable page
//
// Positional on purpose: npm parses `--flag value` pairs that follow `--` as its own cli
// config and silently drops the ones it does not know (`--site` was eaten this way), so the
// first argument is the build tree and the optional second one is the site directory.

function main(): void {
  const [buildFolder, siteDir] = process.argv.slice(2).filter((arg) => arg.length > 0);
  if (!buildFolder) {
    throw new Error('Usage: npm run gen:trace -- <build-dir> [<site-dir>]');
  }

  if (siteDir) {
    for (const path of publishObservabilitySite(buildFolder, siteDir)) {
      console.log(`[Trace] Published ${path}`);
    }
    return;
  }

  const rendered = [renderConfigureProfilingReport(buildFolder), renderInstrumentationReport(buildFolder)]
    .filter((path): path is string => path !== undefined);
  if (rendered.length === 0) {
    console.log(`[Trace] No timing artifact in ${buildFolder} (configure timing needs the flag runCMake() adds, build timing needs CMake >= 4.3 && DYNXX_ENABLE_BUILD_INSTRUMENTATION=ON)`);
  }
  for (const path of rendered) {
    console.log(`[Trace] Wrote ${path}`);
  }
}

main();
