import { defineConfig } from "vitest/config";

// Integration tests: exercise the client against a real server running in
// Docker. One container is started once by the global setup and shared by all
// integration test files, which run sequentially so each can flush safely.
export default defineConfig({
  test: {
    include: ["test/integration/**/*.test.ts"],
    globalSetup: ["./test/integration/global-setup.ts"],
    fileParallelism: false,
    testTimeout: 30_000,
    hookTimeout: 300_000, // building the image + starting the container is slow
  },
});
