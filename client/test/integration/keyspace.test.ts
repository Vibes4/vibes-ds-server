import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import { VibesCommandError, type VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("keyspace", () => {
  it("keys supports glob patterns", async () => {
    await client.strings.mset({ "user:1": "a", "user:2": "b", other: "c" });
    expect((await client.keyspace.keys("user:*")).sort()).toEqual(["user:1", "user:2"]);
  });

  it("keys returns an empty array when nothing matches", async () => {
    expect(await client.keyspace.keys("none:*")).toEqual([]);
  });

  it("type reports string or none", async () => {
    await client.strings.set("k", "v");
    expect(await client.keyspace.type("k")).toBe("string");
    expect(await client.keyspace.type("ghost")).toBe("none");
  });

  it("rename moves a key", async () => {
    await client.strings.set("a", "1");
    await client.keyspace.rename("a", "b");
    expect(await client.strings.get("b")).toBe("1");
    expect(await client.strings.get("a")).toBeNull();
  });

  it("rename throws for a missing source key", async () => {
    await expect(client.keyspace.rename("ghost", "x")).rejects.toBeInstanceOf(VibesCommandError);
  });

  it("scan returns a cursor and keys", async () => {
    await client.strings.mset({ a: "1", b: "2" });
    const { cursor, keys } = await client.keyspace.scan(0);
    expect(cursor).toBe("0");
    expect(keys.sort()).toEqual(["a", "b"]);
  });

  it("randomKey returns null on an empty store", async () => {
    expect(await client.keyspace.randomKey()).toBeNull();
  });
});
