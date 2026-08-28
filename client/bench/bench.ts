import path from "node:path";
import { fileURLToPath } from "node:url";

import { GenericContainer, type StartedTestContainer, Wait } from "testcontainers";

import { VibesClient } from "../src/index.js";

// A small load generator that drives the real server (in Docker) through the
// client and reports throughput + latency, once per persistence strategy. It
// makes the snapshot-vs-AOF write trade-off visible: snapshot rewrites the whole
// file on every SET (cost grows with the key count), AOF appends one line.
//
// Tunables (env): BENCH_KEYS, BENCH_CONCURRENCY, BENCH_MODES.
const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, "..", "..");

const KEYS = Number(process.env.BENCH_KEYS ?? 2000);
const CONCURRENCY = Number(process.env.BENCH_CONCURRENCY ?? 16);
const MODES = (process.env.BENCH_MODES ?? "snapshot,aof").split(",");

interface Stats {
  ops: number;
  totalMs: number;
  qps: number;
  p50: number;
  p95: number;
  p99: number;
}

function percentile(sortedAsc: number[], p: number): number {
  if (sortedAsc.length === 0) return 0;
  const index = Math.min(sortedAsc.length - 1, Math.floor((p / 100) * sortedAsc.length));
  return sortedAsc[index] ?? 0;
}

/** Runs `op` `total` times with up to `concurrency` in flight; records latency. */
async function runWorkload(
  total: number,
  concurrency: number,
  op: (i: number) => Promise<unknown>,
): Promise<Stats> {
  const latencies = new Array<number>(total);
  let next = 0;

  const worker = async (): Promise<void> => {
    for (let i = next++; i < total; i = next++) {
      const started = performance.now();
      await op(i);
      latencies[i] = performance.now() - started;
    }
  };

  const start = performance.now();
  await Promise.all(Array.from({ length: concurrency }, worker));
  const totalMs = performance.now() - start;

  latencies.sort((a, b) => a - b);
  return {
    ops: total,
    totalMs,
    qps: (total / totalMs) * 1000,
    p50: percentile(latencies, 50),
    p95: percentile(latencies, 95),
    p99: percentile(latencies, 99),
  };
}

async function waitForServer(client: VibesClient): Promise<void> {
  for (let i = 0; i < 40; i += 1) {
    try {
      if ((await client.server.ping()) === "PONG") return;
    } catch {
      // not ready yet
    }
    await new Promise((resolve) => setTimeout(resolve, 250));
  }
  throw new Error("server did not become ready");
}

async function benchMode(mode: string): Promise<{ set: Stats; get: Stats }> {
  const image = await GenericContainer.fromDockerfile(repoRoot).build();
  const container: StartedTestContainer = await image
    .withEnvironment({ VIBES_PERSISTENCE: mode })
    .withExposedPorts(8080)
    .withWaitStrategy(Wait.forListeningPorts())
    .start();

  try {
    const client = new VibesClient({
      baseUrl: `http://${container.getHost()}:${container.getMappedPort(8080)}`,
    });
    await waitForServer(client);
    await client.server.flushall();

    const set = await runWorkload(KEYS, CONCURRENCY, (i) =>
      client.strings.set(`k:${i}`, `v${i}`),
    );
    const get = await runWorkload(KEYS, CONCURRENCY, (i) => client.strings.get(`k:${i}`));
    return { set, get };
  } finally {
    await container.stop();
  }
}

function fmt(s: Stats): string {
  return (
    `${s.qps.toFixed(0).padStart(7)} qps  ` +
    `p50=${s.p50.toFixed(2)}ms  p95=${s.p95.toFixed(2)}ms  p99=${s.p99.toFixed(2)}ms  ` +
    `(${s.totalMs.toFixed(0)}ms)`
  );
}

async function main(): Promise<void> {
  console.log(
    `\nvibes-ds-server benchmark — ${KEYS} keys, concurrency ${CONCURRENCY}, ` +
      `modes [${MODES.join(", ")}]\n`,
  );

  const results: Array<{ mode: string; set: Stats; get: Stats }> = [];
  for (const mode of MODES) {
    console.log(`building + starting server (persistence=${mode})...`);
    const { set, get } = await benchMode(mode);
    results.push({ mode, set, get });
    console.log(`  SET  ${fmt(set)}`);
    console.log(`  GET  ${fmt(get)}\n`);
  }

  console.log("=== summary ===");
  console.log(
    "mode".padEnd(10) +
      "SET qps".padStart(10) +
      "GET qps".padStart(10) +
      "SET p99".padStart(12) +
      "GET p99".padStart(12),
  );
  for (const r of results) {
    console.log(
      r.mode.padEnd(10) +
        r.set.qps.toFixed(0).padStart(10) +
        r.get.qps.toFixed(0).padStart(10) +
        `${r.set.p99.toFixed(2)}ms`.padStart(12) +
        `${r.get.p99.toFixed(2)}ms`.padStart(12),
    );
  }
  console.log("");
}

main().catch((error) => {
  console.error(error);
  process.exit(1);
});
