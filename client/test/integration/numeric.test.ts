import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import { VibesCommandError, type VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("numeric", () => {
  it("increments and decrements", async () => {
    expect(await client.numeric.incr("c")).toBe(1);
    expect(await client.numeric.incrby("c", 5)).toBe(6);
    expect(await client.numeric.decr("c")).toBe(5);
    expect(await client.numeric.decrby("c", 2)).toBe(3);
  });

  it("throws on a non-integer value", async () => {
    await client.strings.set("c", "abc");
    await expect(client.numeric.incr("c")).rejects.toBeInstanceOf(VibesCommandError);
  });
});
