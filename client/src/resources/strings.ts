import type { Transport } from "../core/transport.js";
import { parseIntReply, parseNil, splitLines, token, tokens } from "../core/encode.js";
import { VibesArgumentError } from "../errors.js";

/** String and multi-key commands (`client.strings.*`). */
export class StringCommands {
  constructor(private readonly transport: Transport) {}

  /** SET key value. Overwrites any existing value and clears its TTL. */
  async set(key: string, value: string): Promise<void> {
    await this.transport.command(["SET", token(key), token(value)]);
  }

  /** GET key. Returns the value, or `null` if the key is absent. */
  async get(key: string): Promise<string | null> {
    return parseNil(await this.transport.command(["GET", token(key)]));
  }

  /** DEL key [key ...]. Returns the number of keys removed. */
  async del(...keys: string[]): Promise<number> {
    if (keys.length === 0) throw new VibesArgumentError("del requires at least one key");
    return parseIntReply(await this.transport.command(["DEL", ...tokens(keys)]));
  }

  /** EXISTS key [key ...]. Returns how many of the keys exist. */
  async exists(...keys: string[]): Promise<number> {
    if (keys.length === 0) throw new VibesArgumentError("exists requires at least one key");
    return parseIntReply(await this.transport.command(["EXISTS", ...tokens(keys)]));
  }

  /** MSET. Sets several key/value pairs at once. */
  async mset(pairs: Record<string, string>): Promise<void> {
    const parts = ["MSET"];
    for (const [key, value] of Object.entries(pairs)) {
      parts.push(token(key), token(value));
    }
    if (parts.length === 1) {
      throw new VibesArgumentError("mset requires at least one key/value pair");
    }
    await this.transport.command(parts);
  }

  /** MGET key [key ...]. Returns one value (or `null`) per key, in order. */
  async mget(...keys: string[]): Promise<Array<string | null>> {
    if (keys.length === 0) throw new VibesArgumentError("mget requires at least one key");
    const reply = await this.transport.command(["MGET", ...tokens(keys)]);
    return splitLines(reply).map((line) => parseNil(line));
  }

  /** APPEND key value. Returns the new string length. */
  async append(key: string, value: string): Promise<number> {
    return parseIntReply(await this.transport.command(["APPEND", token(key), token(value)]));
  }

  /** STRLEN key. Returns the length of the value (0 if absent). */
  async strlen(key: string): Promise<number> {
    return parseIntReply(await this.transport.command(["STRLEN", token(key)]));
  }

  /** GETSET key value. Returns the previous value (or `null`). */
  async getset(key: string, value: string): Promise<string | null> {
    return parseNil(await this.transport.command(["GETSET", token(key), token(value)]));
  }

  /** SETNX key value. Returns `true` if stored, `false` if the key existed. */
  async setnx(key: string, value: string): Promise<boolean> {
    return (await this.transport.command(["SETNX", token(key), token(value)])) === "1";
  }

  /** SETEX key seconds value. Stores the value with a TTL. */
  async setex(key: string, seconds: number, value: string): Promise<void> {
    await this.transport.command(["SETEX", token(key), token(seconds), token(value)]);
  }

  /** GETDEL key. Returns the value (or `null`) and removes the key. */
  async getdel(key: string): Promise<string | null> {
    return parseNil(await this.transport.command(["GETDEL", token(key)]));
  }
}
