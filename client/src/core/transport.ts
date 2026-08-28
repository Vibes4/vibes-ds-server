import { VibesCommandError, VibesConnectionError, VibesError } from "../errors.js";
import type { VibesClientOptions } from "../types.js";

/**
 * The HTTP transport: the single place that talks to the server.
 *
 * It takes an already-validated list of command tokens, sends them to
 * `GET {baseUrl}/redis?cmd=<command line>`, and maps the response to either the
 * reply text (200) or a typed error (400 / other). It performs no argument
 * validation or reply parsing -- that lives in `core/encode` and the resources.
 */
export class Transport {
  private readonly baseUrl: string;
  private readonly timeoutMs: number;
  private readonly fetchImpl: typeof globalThis.fetch;

  constructor(options: VibesClientOptions = {}) {
    const {
      baseUrl,
      host = "localhost",
      port = 8080,
      timeoutMs = 5000,
      fetch: fetchImpl,
    } = options;

    this.baseUrl = (baseUrl ?? `http://${host}:${port}`).replace(/\/+$/, "");
    this.timeoutMs = timeoutMs;

    const resolved = fetchImpl ?? globalThis.fetch;
    if (typeof resolved !== "function") {
      throw new VibesError("No fetch implementation found. Use Node 18+ or pass options.fetch.");
    }
    this.fetchImpl = resolved;
  }

  /** Sends a command (list of tokens) and returns the raw reply text. */
  async command(parts: string[]): Promise<string> {
    const commandLine = parts.join(" ");
    const url = `${this.baseUrl}/redis?cmd=${encodeURIComponent(commandLine)}`;

    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), this.timeoutMs);

    let response: Response;
    try {
      response = await this.fetchImpl(url, { method: "GET", signal: controller.signal });
    } catch (error) {
      const reason = error instanceof Error ? error.message : String(error);
      throw new VibesConnectionError(`Request to ${this.baseUrl} failed: ${reason}`);
    } finally {
      clearTimeout(timer);
    }

    const body = await response.text();
    if (response.status === 200) {
      return body;
    }
    if (response.status === 400) {
      throw new VibesCommandError(parts[0] ?? "", body);
    }
    throw new VibesError(`Unexpected response ${response.status}: ${body}`);
  }
}
