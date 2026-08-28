import { inject } from "vitest";

import { VibesClient } from "../../src/index.js";

/** Builds a client pointed at the shared container started by global-setup. */
export function createClient(): VibesClient {
  return new VibesClient({ baseUrl: inject("baseUrl") });
}
