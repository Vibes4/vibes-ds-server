import {
  VibesArgumentError,
  VibesCommandError,
  VibesConnectionError,
  VibesError,
} from "./errors.js";
import type { ScanResult, VibesClientOptions } from "./types.js";

/** Sentinel the server returns for a missing value. */
const NIL = "(nil)";
/** Sentinel the server returns for an empty KEYS result. */
const EMPTY = "(empty)";

/**
 * A typed client for vibes-ds-server.
 *
 * Commands are sent to `GET {baseUrl}/redis?cmd=<command line>` and the plain
 * text reply is parsed into a convenient type. A raw {@link command} escape
 * hatch is provided for anything without a dedicated method.
 *
 * Limitation: the server tokenizes commands on whitespace and has no quoting,
 * so keys and values must be non-empty and contain no whitespace. The client
 * validates this up front and throws {@link VibesArgumentError}.
 */
export class VibesClient {
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
      throw new VibesArgumentError(
        "No fetch implementation found. Use Node 18+ or pass options.fetch.",
      );
    }
    this.fetchImpl = resolved;
  }

  // -------------------------------------------------------------------------
  // Raw command
  // -------------------------------------------------------------------------

  /** Sends an arbitrary command and returns the raw text reply. */
  async command(...args: Array<string | number>): Promise<string> {
    return this.send(args.map((arg, index) => this.encodeToken(arg, index)));
  }

  // -------------------------------------------------------------------------
  // Strings
  // -------------------------------------------------------------------------

  /** SET key value. Overwrites any existing value and clears its TTL. */
  async set(key: string, value: string): Promise<void> {
    await this.send(["SET", this.token(key), this.token(value)]);
  }

  /** GET key. Returns the value, or `null` if the key is absent. */
  async get(key: string): Promise<string | null> {
    return this.nilable(await this.send(["GET", this.token(key)]));
  }

  /** DEL key [key ...]. Returns the number of keys removed. */
  async del(...keys: string[]): Promise<number> {
    this.requireArgs(keys, "del");
    return this.toInt(await this.send(["DEL", ...this.tokens(keys)]));
  }

  /** EXISTS key [key ...]. Returns how many of the keys exist. */
  async exists(...keys: string[]): Promise<number> {
    this.requireArgs(keys, "exists");
    return this.toInt(await this.send(["EXISTS", ...this.tokens(keys)]));
  }

  /** MSET. Sets several key/value pairs at once. */
  async mset(pairs: Record<string, string>): Promise<void> {
    const parts = ["MSET"];
    for (const [key, value] of Object.entries(pairs)) {
      parts.push(this.token(key), this.token(value));
    }
    if (parts.length === 1) {
      throw new VibesArgumentError("mset requires at least one key/value pair");
    }
    await this.send(parts);
  }

  /** MGET key [key ...]. Returns one value (or `null`) per key, in order. */
  async mget(...keys: string[]): Promise<Array<string | null>> {
    this.requireArgs(keys, "mget");
    const reply = await this.send(["MGET", ...this.tokens(keys)]);
    return this.splitLines(reply).map((line) => this.nilable(line));
  }

  /** APPEND key value. Returns the new string length. */
  async append(key: string, value: string): Promise<number> {
    return this.toInt(await this.send(["APPEND", this.token(key), this.token(value)]));
  }

  /** STRLEN key. Returns the length of the value (0 if absent). */
  async strlen(key: string): Promise<number> {
    return this.toInt(await this.send(["STRLEN", this.token(key)]));
  }

  /** GETSET key value. Returns the previous value (or `null`). */
  async getset(key: string, value: string): Promise<string | null> {
    return this.nilable(await this.send(["GETSET", this.token(key), this.token(value)]));
  }

  /** SETNX key value. Returns `true` if stored, `false` if the key existed. */
  async setnx(key: string, value: string): Promise<boolean> {
    return (await this.send(["SETNX", this.token(key), this.token(value)])) === "1";
  }

  /** SETEX key seconds value. Stores the value with a TTL. */
  async setex(key: string, seconds: number, value: string): Promise<void> {
    await this.send(["SETEX", this.token(key), this.token(seconds), this.token(value)]);
  }

  /** GETDEL key. Returns the value (or `null`) and removes the key. */
  async getdel(key: string): Promise<string | null> {
    return this.nilable(await this.send(["GETDEL", this.token(key)]));
  }

  // -------------------------------------------------------------------------
  // Numeric
  // -------------------------------------------------------------------------

  /** INCR key. Returns the new value. */
  async incr(key: string): Promise<number> {
    return this.toInt(await this.send(["INCR", this.token(key)]));
  }

  /** INCRBY key amount. Returns the new value. */
  async incrby(key: string, amount: number): Promise<number> {
    return this.toInt(await this.send(["INCRBY", this.token(key), this.token(amount)]));
  }

  /** DECR key. Returns the new value. */
  async decr(key: string): Promise<number> {
    return this.toInt(await this.send(["DECR", this.token(key)]));
  }

  /** DECRBY key amount. Returns the new value. */
  async decrby(key: string, amount: number): Promise<number> {
    return this.toInt(await this.send(["DECRBY", this.token(key), this.token(amount)]));
  }

  // -------------------------------------------------------------------------
  // Expiration
  // -------------------------------------------------------------------------

  /** EXPIRE key seconds. Returns `true` if the TTL was set. */
  async expire(key: string, seconds: number): Promise<boolean> {
    return (await this.send(["EXPIRE", this.token(key), this.token(seconds)])) === "1";
  }

  /** PEXPIRE key milliseconds. Returns `true` if the TTL was set. */
  async pexpire(key: string, milliseconds: number): Promise<boolean> {
    return (await this.send(["PEXPIRE", this.token(key), this.token(milliseconds)])) === "1";
  }

  /** TTL key. Returns remaining seconds, `-1` if no TTL, `-2` if missing. */
  async ttl(key: string): Promise<number> {
    return this.toInt(await this.send(["TTL", this.token(key)]));
  }

  /** PTTL key. Returns remaining milliseconds, `-1` if no TTL, `-2` if missing. */
  async pttl(key: string): Promise<number> {
    return this.toInt(await this.send(["PTTL", this.token(key)]));
  }

  /** PERSIST key. Returns `true` if an existing TTL was removed. */
  async persist(key: string): Promise<boolean> {
    return (await this.send(["PERSIST", this.token(key)])) === "1";
  }

  // -------------------------------------------------------------------------
  // Keyspace
  // -------------------------------------------------------------------------

  /** KEYS pattern (glob: `*` and `?`). Returns matching keys. */
  async keys(pattern = "*"): Promise<string[]> {
    const reply = await this.send(["KEYS", this.token(pattern)]);
    return reply === EMPTY ? [] : this.splitLines(reply);
  }

  /** SCAN cursor. Returns the next cursor and a batch of keys. */
  async scan(cursor: string | number = 0): Promise<ScanResult> {
    const reply = await this.send(["SCAN", this.token(cursor)]);
    const [next = "0", ...keys] = this.splitLines(reply);
    return { cursor: next, keys };
  }

  /** TYPE key. Returns "string" or "none". */
  async type(key: string): Promise<string> {
    return this.send(["TYPE", this.token(key)]);
  }

  /** RENAME key newKey. Throws {@link VibesCommandError} if the key is missing. */
  async rename(key: string, newKey: string): Promise<void> {
    await this.send(["RENAME", this.token(key), this.token(newKey)]);
  }

  /** RANDOMKEY. Returns a key, or `null` if the store is empty. */
  async randomKey(): Promise<string | null> {
    return this.nilable(await this.send(["RANDOMKEY"]));
  }

  // -------------------------------------------------------------------------
  // Server
  // -------------------------------------------------------------------------

  /** PING [message]. Returns "PONG", or the message if provided. */
  async ping(message?: string): Promise<string> {
    return message === undefined
      ? this.send(["PING"])
      : this.send(["PING", this.token(message)]);
  }

  /** ECHO message. Returns the message unchanged. */
  async echo(message: string): Promise<string> {
    return this.send(["ECHO", this.token(message)]);
  }

  /** INFO. Returns the raw, section-grouped server statistics text. */
  async info(): Promise<string> {
    return this.send(["INFO"]);
  }

  /** DBSIZE. Returns the number of keys currently stored. */
  async dbsize(): Promise<number> {
    return this.toInt(await this.send(["DBSIZE"]));
  }

  /** FLUSHDB. Removes every key. */
  async flushdb(): Promise<void> {
    await this.send(["FLUSHDB"]);
  }

  /** FLUSHALL. Removes every key (this server has a single database). */
  async flushall(): Promise<void> {
    await this.send(["FLUSHALL"]);
  }

  // -------------------------------------------------------------------------
  // Persistence
  // -------------------------------------------------------------------------

  /** SAVE. Forces a synchronous snapshot to disk. */
  async save(): Promise<void> {
    await this.send(["SAVE"]);
  }

  /** BGSAVE. Triggers a background save; returns the server's acknowledgement. */
  async bgsave(): Promise<string> {
    return this.send(["BGSAVE"]);
  }

  /** LASTSAVE. Returns the unix timestamp (seconds) of the last save. */
  async lastsave(): Promise<number> {
    return this.toInt(await this.send(["LASTSAVE"]));
  }

  // -------------------------------------------------------------------------
  // Internals
  // -------------------------------------------------------------------------

  private tokens(values: Array<string | number>): string[] {
    return values.map((value) => this.token(value));
  }

  private token(value: string | number): string {
    return this.encodeToken(value, 0);
  }

  /** Validates that an argument is a single, non-empty, whitespace-free token. */
  private encodeToken(value: string | number, index: number): string {
    const text = typeof value === "number" ? String(value) : value;
    if (text.length === 0) {
      throw new VibesArgumentError(
        `Argument at position ${index} is empty; the server rejects empty tokens.`,
      );
    }
    if (/\s/.test(text)) {
      throw new VibesArgumentError(
        `Argument "${text}" contains whitespace. The server tokenizes on whitespace ` +
          "and has no quoting, so keys and values must be single tokens.",
      );
    }
    return text;
  }

  private requireArgs(list: unknown[], command: string): void {
    if (list.length === 0) {
      throw new VibesArgumentError(`${command} requires at least one key`);
    }
  }

  private nilable(text: string): string | null {
    return text === NIL ? null : text;
  }

  private toInt(text: string): number {
    const value = Number.parseInt(text, 10);
    if (Number.isNaN(value)) {
      throw new VibesError(`Expected an integer reply but got "${text}"`);
    }
    return value;
  }

  private splitLines(text: string): string[] {
    return text.length === 0 ? [] : text.split("\n");
  }

  private async send(parts: string[]): Promise<string> {
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
