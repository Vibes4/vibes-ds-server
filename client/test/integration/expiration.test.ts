import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import type { VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("expiration", () => {
  it("expire, ttl, and persist", async () => {
    await client.strings.set("k", "v");
    expect(await client.expiration.expire("k", 100)).toBe(true);
    expect(await client.expiration.ttl("k")).toBeGreaterThan(90);
    expect(await client.expiration.persist("k")).toBe(true);
    expect(await client.expiration.ttl("k")).toBe(-1);
  });

  it("ttl is -2 for a missing key", async () => {
    expect(await client.expiration.ttl("ghost")).toBe(-2);
  });

  it("setex actually expires the key (real timing)", async () => {
    await client.strings.setex("temp", 1, "x");
    expect(await client.strings.get("temp")).toBe("x");
    await new Promise((resolve) => setTimeout(resolve, 1500));
    expect(await client.strings.get("temp")).toBeNull();
  });

  it("incr preserves an existing ttl", async () => {
    await client.strings.set("c", "1");
    await client.expiration.expire("c", 100);
    await client.numeric.incr("c");
    expect(await client.expiration.ttl("c")).toBeGreaterThan(90);
  });
});
