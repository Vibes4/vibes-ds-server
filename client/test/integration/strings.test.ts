import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import type { VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("strings", () => {
  it("sets and gets a value", async () => {
    await client.strings.set("name", "Vaibu");
    expect(await client.strings.get("name")).toBe("Vaibu");
  });

  it("returns null for a missing key", async () => {
    expect(await client.strings.get("missing")).toBeNull();
  });

  it("del returns the number of keys removed", async () => {
    await client.strings.set("a", "1");
    await client.strings.set("b", "2");
    expect(await client.strings.del("a", "b", "ghost")).toBe(2);
  });

  it("exists counts existing keys", async () => {
    await client.strings.set("a", "1");
    expect(await client.strings.exists("a", "ghost")).toBe(1);
  });

  it("mset and mget round-trip, with nulls for misses", async () => {
    await client.strings.mset({ a: "1", b: "2" });
    expect(await client.strings.mget("a", "b", "ghost")).toEqual(["1", "2", null]);
  });

  it("append returns the new length", async () => {
    expect(await client.strings.append("s", "ab")).toBe(2);
    expect(await client.strings.append("s", "cd")).toBe(4);
    expect(await client.strings.strlen("s")).toBe(4);
  });

  it("getset returns the previous value", async () => {
    await client.strings.set("k", "old");
    expect(await client.strings.getset("k", "new")).toBe("old");
    expect(await client.strings.get("k")).toBe("new");
  });

  it("setnx only sets when absent", async () => {
    expect(await client.strings.setnx("k", "1")).toBe(true);
    expect(await client.strings.setnx("k", "2")).toBe(false);
    expect(await client.strings.get("k")).toBe("1");
  });

  it("getdel returns and removes", async () => {
    await client.strings.set("k", "v");
    expect(await client.strings.getdel("k")).toBe("v");
    expect(await client.strings.get("k")).toBeNull();
  });
});
