import type { Transport } from "../core/transport.js";
import { EMPTY, parseNil, splitLines, token } from "../core/encode.js";
import type { ScanResult } from "../types.js";

/** Generic key-space commands (`client.keyspace.*`). */
export class KeyspaceCommands {
  constructor(private readonly transport: Transport) {}

  /** KEYS pattern (glob: `*` and `?`). Returns matching keys. */
  async keys(pattern = "*"): Promise<string[]> {
    const reply = await this.transport.command(["KEYS", token(pattern)]);
    return reply === EMPTY ? [] : splitLines(reply);
  }

  /** SCAN cursor. Returns the next cursor and a batch of keys. */
  async scan(cursor: string | number = 0): Promise<ScanResult> {
    const reply = await this.transport.command(["SCAN", token(cursor)]);
    const [next = "0", ...keys] = splitLines(reply);
    return { cursor: next, keys };
  }

  /** TYPE key. Returns "string" or "none". */
  async type(key: string): Promise<string> {
    return this.transport.command(["TYPE", token(key)]);
  }

  /** RENAME key newKey. Throws `VibesCommandError` if the key is missing. */
  async rename(key: string, newKey: string): Promise<void> {
    await this.transport.command(["RENAME", token(key), token(newKey)]);
  }

  /** RANDOMKEY. Returns a key, or `null` if the store is empty. */
  async randomKey(): Promise<string | null> {
    return parseNil(await this.transport.command(["RANDOMKEY"]));
  }
}
