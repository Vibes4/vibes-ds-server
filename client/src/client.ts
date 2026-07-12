import { Transport } from "./core/transport.js";
import { token } from "./core/encode.js";
import { ExpirationCommands } from "./resources/expiration.js";
import { KeyspaceCommands } from "./resources/keyspace.js";
import { NumericCommands } from "./resources/numeric.js";
import { PersistenceCommands } from "./resources/persistence.js";
import { ServerCommands } from "./resources/server.js";
import { StringCommands } from "./resources/strings.js";
import type { VibesClientOptions } from "./types.js";

/**
 * The vibes-ds-server SDK.
 *
 * Commands are organized into resource groups that mirror the server's command
 * categories:
 *
 * ```ts
 * const client = new VibesClient({ host: "localhost", port: 8080 });
 * await client.strings.set("name", "Vaibu");
 * await client.numeric.incr("counter");
 * await client.expiration.expire("counter", 60);
 * await client.keyspace.keys("user:*");
 * await client.server.info();
 * await client.persistence.save();
 * ```
 *
 * Use {@link command} for anything without a dedicated method.
 */
export class VibesClient {
  readonly strings: StringCommands;
  readonly numeric: NumericCommands;
  readonly expiration: ExpirationCommands;
  readonly keyspace: KeyspaceCommands;
  readonly server: ServerCommands;
  readonly persistence: PersistenceCommands;

  private readonly transport: Transport;

  constructor(options: VibesClientOptions = {}) {
    this.transport = new Transport(options);
    this.strings = new StringCommands(this.transport);
    this.numeric = new NumericCommands(this.transport);
    this.expiration = new ExpirationCommands(this.transport);
    this.keyspace = new KeyspaceCommands(this.transport);
    this.server = new ServerCommands(this.transport);
    this.persistence = new PersistenceCommands(this.transport);
  }

  /** Sends an arbitrary command and returns the raw text reply. */
  async command(...args: Array<string | number>): Promise<string> {
    return this.transport.command(args.map((arg, index) => token(arg, index)));
  }
}
