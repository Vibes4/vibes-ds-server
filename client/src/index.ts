// Public SDK surface.

export { VibesClient } from "./client.js";

// Core building blocks (usable à la carte).
export { Transport } from "./core/transport.js";

// Resource modules.
export { StringCommands } from "./resources/strings.js";
export { NumericCommands } from "./resources/numeric.js";
export { ExpirationCommands } from "./resources/expiration.js";
export { KeyspaceCommands } from "./resources/keyspace.js";
export { ServerCommands } from "./resources/server.js";
export { PersistenceCommands } from "./resources/persistence.js";

// Errors.
export {
  VibesError,
  VibesConnectionError,
  VibesArgumentError,
  VibesCommandError,
} from "./errors.js";

// Types.
export type { VibesClientOptions, ScanResult } from "./types.js";
