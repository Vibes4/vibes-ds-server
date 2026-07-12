import path from "node:path";
import { fileURLToPath } from "node:url";

import { GenericContainer, type StartedTestContainer, Wait } from "testcontainers";
import { afterAll, beforeAll, beforeEach, describe, expect, it } from "vitest";

import { VibesArgumentError, VibesClient, VibesCommandError } from "../src/index.js";

const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, "..", ".."); // client/test -> repo root

let container: StartedTestContainer;
let client: VibesClient;

/** Polls PING until the server answers, so tests never race a cold start. */
async function waitForServer(c: VibesClient, attempts = 40): Promise<void> {
  for (let i = 0; i < attempts; i += 1) {
    try {
      if ((await c.ping()) === "PONG") return;
    } catch {
      // not ready yet
    }
    await new Promise((resolve) => setTimeout(resolve, 500));
  }
  throw new Error("server did not become ready in time");
}

beforeAll(async () => {
  // Build the real server image from the repo Dockerfile and run it. No mocks.
  const image = await GenericContainer.fromDockerfile(repoRoot).build();
  container = await image
    .withExposedPorts(8080)
    .withWaitStrategy(Wait.forListeningPorts())
    .start();

  const baseUrl = `http://${container.getHost()}:${container.getMappedPort(8080)}`;
  client = new VibesClient({ baseUrl });
  await waitForServer(client);
}, 300_000);

afterAll(async () => {
  await container?.stop();
});

beforeEach(async () => {
  await client.flushall(); // isolate each test
});

describe("strings", () => {
  it("sets and gets a value", async () => {
    await client.set("name", "Vaibu");
    expect(await client.get("name")).toBe("Vaibu");
  });

  it("returns null for a missing key", async () => {
    expect(await client.get("missing")).toBeNull();
  });

  it("del returns the number of keys removed", async () => {
    await client.set("a", "1");
    await client.set("b", "2");
    expect(await client.del("a", "b", "ghost")).toBe(2);
  });

  it("exists counts existing keys", async () => {
    await client.set("a", "1");
    expect(await client.exists("a", "ghost")).toBe(1);
  });

  it("mset and mget round-trip, with nulls for misses", async () => {
    await client.mset({ a: "1", b: "2" });
    expect(await client.mget("a", "b", "ghost")).toEqual(["1", "2", null]);
  });

  it("append returns the new length", async () => {
    expect(await client.append("s", "ab")).toBe(2);
    expect(await client.append("s", "cd")).toBe(4);
    expect(await client.strlen("s")).toBe(4);
  });

  it("getset returns the previous value", async () => {
    await client.set("k", "old");
    expect(await client.getset("k", "new")).toBe("old");
    expect(await client.get("k")).toBe("new");
  });

  it("setnx only sets when absent", async () => {
    expect(await client.setnx("k", "1")).toBe(true);
    expect(await client.setnx("k", "2")).toBe(false);
    expect(await client.get("k")).toBe("1");
  });

  it("getdel returns and removes", async () => {
    await client.set("k", "v");
    expect(await client.getdel("k")).toBe("v");
    expect(await client.get("k")).toBeNull();
  });
});

describe("numeric", () => {
  it("increments and decrements", async () => {
    expect(await client.incr("c")).toBe(1);
    expect(await client.incrby("c", 5)).toBe(6);
    expect(await client.decr("c")).toBe(5);
    expect(await client.decrby("c", 2)).toBe(3);
  });

  it("throws on a non-integer value", async () => {
    await client.set("c", "abc");
    await expect(client.incr("c")).rejects.toBeInstanceOf(VibesCommandError);
  });
});

describe("expiration", () => {
  it("expire, ttl, and persist", async () => {
    await client.set("k", "v");
    expect(await client.expire("k", 100)).toBe(true);
    expect(await client.ttl("k")).toBeGreaterThan(90);
    expect(await client.persist("k")).toBe(true);
    expect(await client.ttl("k")).toBe(-1);
  });

  it("ttl is -2 for a missing key", async () => {
    expect(await client.ttl("ghost")).toBe(-2);
  });

  it("setex actually expires the key (real timing)", async () => {
    await client.setex("temp", 1, "x");
    expect(await client.get("temp")).toBe("x");
    await new Promise((resolve) => setTimeout(resolve, 1500));
    expect(await client.get("temp")).toBeNull();
  });

  it("incr preserves an existing ttl", async () => {
    await client.set("c", "1");
    await client.expire("c", 100);
    await client.incr("c");
    expect(await client.ttl("c")).toBeGreaterThan(90);
  });
});

describe("keyspace", () => {
  it("keys supports glob patterns", async () => {
    await client.mset({ "user:1": "a", "user:2": "b", other: "c" });
    expect((await client.keys("user:*")).sort()).toEqual(["user:1", "user:2"]);
  });

  it("keys returns an empty array when nothing matches", async () => {
    expect(await client.keys("none:*")).toEqual([]);
  });

  it("type reports string or none", async () => {
    await client.set("k", "v");
    expect(await client.type("k")).toBe("string");
    expect(await client.type("ghost")).toBe("none");
  });

  it("rename moves a key", async () => {
    await client.set("a", "1");
    await client.rename("a", "b");
    expect(await client.get("b")).toBe("1");
    expect(await client.get("a")).toBeNull();
  });

  it("rename throws for a missing source key", async () => {
    await expect(client.rename("ghost", "x")).rejects.toBeInstanceOf(VibesCommandError);
  });

  it("dbsize counts keys", async () => {
    await client.mset({ a: "1", b: "2" });
    expect(await client.dbsize()).toBe(2);
  });

  it("scan returns a cursor and keys", async () => {
    await client.mset({ a: "1", b: "2" });
    const { cursor, keys } = await client.scan(0);
    expect(cursor).toBe("0");
    expect(keys.sort()).toEqual(["a", "b"]);
  });

  it("randomKey returns null on an empty store", async () => {
    expect(await client.randomKey()).toBeNull();
  });
});

describe("server", () => {
  it("ping returns PONG or the message", async () => {
    expect(await client.ping()).toBe("PONG");
    expect(await client.ping("hello")).toBe("hello");
  });

  it("echo returns the message", async () => {
    expect(await client.echo("world")).toBe("world");
  });

  it("info includes the expected sections", async () => {
    const info = await client.info();
    expect(info).toContain("# Server");
    expect(info).toContain("# Memory");
    expect(info).toContain("# Stats");
    expect(info).toContain("# Persistence");
    expect(info).toContain("# Keyspace");
  });
});

describe("persistence", () => {
  it("save records a last-save timestamp", async () => {
    await client.set("k", "v");
    await client.save();
    expect(await client.lastsave()).toBeGreaterThan(0);
  });
});

describe("validation and raw commands", () => {
  it("rejects whitespace in values before sending", async () => {
    await expect(client.set("k", "has space")).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("rejects empty tokens before sending", async () => {
    await expect(client.set("k", "")).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("supports raw commands", async () => {
    expect(await client.command("SET", "k", "v")).toBe("OK");
    expect(await client.command("GET", "k")).toBe("v");
  });

  it("surfaces unknown commands as VibesCommandError", async () => {
    await expect(client.command("NOPE", "x")).rejects.toBeInstanceOf(VibesCommandError);
  });
});
