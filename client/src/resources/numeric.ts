import type { Transport } from "../core/transport.js";
import { parseIntReply, token } from "../core/encode.js";

/** Integer commands (`client.numeric.*`). A missing key counts as 0. */
export class NumericCommands {
  constructor(private readonly transport: Transport) {}

  /** INCR key. Returns the new value. */
  async incr(key: string): Promise<number> {
    return parseIntReply(await this.transport.command(["INCR", token(key)]));
  }

  /** INCRBY key amount. Returns the new value. */
  async incrby(key: string, amount: number): Promise<number> {
    return parseIntReply(await this.transport.command(["INCRBY", token(key), token(amount)]));
  }

  /** DECR key. Returns the new value. */
  async decr(key: string): Promise<number> {
    return parseIntReply(await this.transport.command(["DECR", token(key)]));
  }

  /** DECRBY key amount. Returns the new value. */
  async decrby(key: string, amount: number): Promise<number> {
    return parseIntReply(await this.transport.command(["DECRBY", token(key), token(amount)]));
  }
}
