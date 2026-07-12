/** Options for constructing a {@link VibesClient}. */
export interface VibesClientOptions {
  /**
   * Full base URL of the server, e.g. "http://localhost:8080".
   * When set, `host` and `port` are ignored.
   */
  baseUrl?: string;
  /** Server host. Default: "localhost". Ignored when `baseUrl` is set. */
  host?: string;
  /** Server port. Default: 8080. Ignored when `baseUrl` is set. */
  port?: number;
  /** Per-request timeout in milliseconds. Default: 5000. */
  timeoutMs?: number;
  /**
   * Custom `fetch` implementation. Defaults to the global `fetch`
   * (available in Node 18+ and browsers). Provide one for older runtimes.
   */
  fetch?: typeof globalThis.fetch;
}

/** Result of a {@link VibesClient.scan} call. */
export interface ScanResult {
  /** The next cursor. "0" means iteration is complete. */
  cursor: string;
  /** Keys returned in this batch. */
  keys: string[];
}
