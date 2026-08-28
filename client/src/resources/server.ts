import type { Transport } from "../core/transport.js";
import { parseIntReply, token } from "../core/encode.js";

/** Server / administrative commands (`client.server.*`). */
export class ServerCommands {
  constructor(private readonly transport: Transport) {}

  /** PING [message]. Returns "PONG", or the message if provided. */
  async ping(message?: string): Promise<string> {
    return message === undefined
      ? this.transport.command(["PING"])
      : this.transport.command(["PING", token(message)]);
  }

  /** ECHO message. Returns the message unchanged. */
  async echo(message: string): Promise<string> {
    return this.transport.command(["ECHO", token(message)]);
  }

  /** INFO. Returns the raw, section-grouped server statistics text. */
  async info(): Promise<string> {
    return this.transport.command(["INFO"]);
  }

  /** DBSIZE. Returns the number of keys currently stored. */
  async dbsize(): Promise<number> {
    return parseIntReply(await this.transport.command(["DBSIZE"]));
  }

  /** FLUSHDB. Removes every key. */
  async flushdb(): Promise<void> {
    await this.transport.command(["FLUSHDB"]);
  }

  /** FLUSHALL. Removes every key (this server has a single database). */
  async flushall(): Promise<void> {
    await this.transport.command(["FLUSHALL"]);
  }
}
