import { defineConfig } from "vitest/config";

// Unit tests: pure functions and client-side validation. No server, no Docker,
// no mocking (validation throws before any request is made).
export default defineConfig({
  test: {
    include: ["test/unit/**/*.test.ts"],
  },
});
