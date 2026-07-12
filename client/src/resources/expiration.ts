import type { Transport } from "../core/transport.js";
import { parseBool, parseIntReply, token } from "../core/encode.js";

/** TTL commands (`client.expiration.*`). */
export class ExpirationCommands {
  constructor(private readonly transport: Transport) {}

  /** EXPIRE key seconds. Returns `true` if the TTL was set. */
  async expire(key: string, seconds: number): Promise<boolean> {
    return parseBool(await this.transport.command(["EXPIRE", token(key), token(seconds)]));
  }

  /** PEXPIRE key milliseconds. Returns `true` if the TTL was set. */
  async pexpire(key: string, milliseconds: number): Promise<boolean> {
    return parseBool(await this.transport.command(["PEXPIRE", token(key), token(milliseconds)]));
  }

  /** TTL key. Returns remaining seconds, `-1` if no TTL, `-2` if missing. */
  async ttl(key: string): Promise<number> {
    return parseIntReply(await this.transport.command(["TTL", token(key)]));
  }

  /** PTTL key. Returns remaining milliseconds, `-1` if no TTL, `-2` if missing. */
  async pttl(key: string): Promise<number> {
    return parseIntReply(await this.transport.command(["PTTL", token(key)]));
  }

  /** PERSIST key. Returns `true` if an existing TTL was removed. */
  async persist(key: string): Promise<boolean> {
    return parseBool(await this.transport.command(["PERSIST", token(key)]));
  }
}
