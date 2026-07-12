import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import type { VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("server", () => {
  it("ping returns PONG or the message", async () => {
    expect(await client.server.ping()).toBe("PONG");
    expect(await client.server.ping("hello")).toBe("hello");
  });

  it("echo returns the message", async () => {
    expect(await client.server.echo("world")).toBe("world");
  });

  it("dbsize counts keys", async () => {
    await client.strings.mset({ a: "1", b: "2" });
    expect(await client.server.dbsize()).toBe(2);
  });

  it("info includes the expected sections", async () => {
    const info = await client.server.info();
    expect(info).toContain("# Server");
    expect(info).toContain("# Memory");
    expect(info).toContain("# Stats");
    expect(info).toContain("# Persistence");
    expect(info).toContain("# Keyspace");
  });
});
