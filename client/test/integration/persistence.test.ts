import { beforeAll, beforeEach, describe, expect, it } from "vitest";

import type { VibesClient } from "../../src/index.js";
import { createClient } from "./helpers.js";

let client: VibesClient;
beforeAll(() => {
  client = createClient();
});
beforeEach(() => client.server.flushall());

describe("persistence", () => {
  it("save records a last-save timestamp", async () => {
    await client.strings.set("k", "v");
    await client.persistence.save();
    expect(await client.persistence.lastsave()).toBeGreaterThan(0);
  });

  it("bgsave is acknowledged", async () => {
    expect(await client.persistence.bgsave()).toContain("saving");
  });
});
