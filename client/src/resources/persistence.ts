import type { Transport } from "../core/transport.js";
import { parseIntReply } from "../core/encode.js";

/** Persistence commands (`client.persistence.*`). */
export class PersistenceCommands {
  constructor(private readonly transport: Transport) {}

  /** SAVE. Forces a synchronous snapshot to disk. */
  async save(): Promise<void> {
    await this.transport.command(["SAVE"]);
  }

  /** BGSAVE. Triggers a background save; returns the server's acknowledgement. */
  async bgsave(): Promise<string> {
    return this.transport.command(["BGSAVE"]);
  }

  /** LASTSAVE. Returns the unix timestamp (seconds) of the last save. */
  async lastsave(): Promise<number> {
    return parseIntReply(await this.transport.command(["LASTSAVE"]));
  }
}
