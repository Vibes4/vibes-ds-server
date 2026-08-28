/** Base class for every error thrown by this client. */
export class VibesError extends Error {
  constructor(message: string) {
    super(message);
    this.name = new.target.name;
  }
}

/** The server could not be reached (network failure, timeout, DNS, ...). */
export class VibesConnectionError extends VibesError {}

/** An argument failed client-side validation before any request was sent. */
export class VibesArgumentError extends VibesError {}

/**
 * The server rejected the command with HTTP 400. `command` is the command name
 * (e.g. "SET") and `message` is the server's error text.
 */
export class VibesCommandError extends VibesError {
  constructor(
    public readonly command: string,
    message: string,
  ) {
    super(message);
  }
}
