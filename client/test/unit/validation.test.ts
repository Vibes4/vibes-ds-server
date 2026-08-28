import { describe, expect, it } from "vitest";

import { VibesArgumentError, VibesClient } from "../../src/index.js";

// These assertions exercise client-side validation, which runs before any
// request is made -- so no server (and no mocking) is required. The baseUrl is
// never contacted.
const client = new VibesClient({ baseUrl: "http://localhost:1" });

describe("client-side validation", () => {
  it("rejects values containing whitespace", async () => {
    await expect(client.strings.set("k", "has space")).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("rejects empty values", async () => {
    await expect(client.strings.set("k", "")).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("del requires at least one key", async () => {
    await expect(client.strings.del()).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("mset requires at least one pair", async () => {
    await expect(client.strings.mset({})).rejects.toBeInstanceOf(VibesArgumentError);
  });

  it("raw command validates its arguments", async () => {
    await expect(client.command("SET", "k", "bad value")).rejects.toBeInstanceOf(
      VibesArgumentError,
    );
  });
});
