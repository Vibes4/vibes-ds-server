import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import { VibesCommandError, type VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("raw command", () => {
  it("runs arbitrary commands", async () => {
    expect(await client.command("SET", "k", "v")).toBe("OK");
    expect(await client.command("GET", "k")).toBe("v");
  });

  it("surfaces unknown commands as VibesCommandError", async () => {
    await expect(client.command("NOPE", "x")).rejects.toBeInstanceOf(VibesCommandError);
  });
});
