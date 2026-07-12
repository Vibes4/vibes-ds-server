import { defineConfig } from "vitest/config";

export default defineConfig({
  test: {
    include: ["test/**/*.test.ts"],
    // A single worker: the tests share one Docker container started in beforeAll.
    fileParallelism: false,
    testTimeout: 30_000,
    // Building the Docker image and starting the container can be slow.
    hookTimeout: 300_000,
  },
});
