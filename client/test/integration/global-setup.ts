import path from "node:path";
import { fileURLToPath } from "node:url";

import { GenericContainer, type StartedTestContainer, Wait } from "testcontainers";
import type { GlobalSetupContext } from "vitest/node";

const here = path.dirname(fileURLToPath(import.meta.url));
const repoRoot = path.resolve(here, "..", "..", ".."); // client/test/integration -> repo root

let container: StartedTestContainer | undefined;

/**
 * Builds the server image from the repository Dockerfile, starts it once, and
 * shares its URL with every integration test file via `provide` / `inject`.
 */
export default async function setup({ provide }: GlobalSetupContext) {
  const image = await GenericContainer.fromDockerfile(repoRoot).build();
  container = await image
    .withExposedPorts(8080)
    .withWaitStrategy(Wait.forListeningPorts())
    .start();

  const baseUrl = `http://${container.getHost()}:${container.getMappedPort(8080)}`;

  // Belt-and-braces readiness check before handing the URL to the tests.
  for (let attempt = 0; attempt < 40; attempt += 1) {
    try {
      const response = await fetch(`${baseUrl}/redis?cmd=PING`);
      if ((await response.text()) === "PONG") break;
    } catch {
      // not ready yet
    }
    await new Promise((resolve) => setTimeout(resolve, 500));
  }

  provide("baseUrl", baseUrl);

  return async () => {
    await container?.stop();
  };
}

declare module "vitest" {
  interface ProvidedContext {
    baseUrl: string;
  }
}
