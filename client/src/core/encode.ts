import { VibesArgumentError, VibesError } from "../errors.js";

/** Sentinel the server returns for a missing value. */
export const NIL = "(nil)";
/** Sentinel the server returns for an empty KEYS result. */
export const EMPTY = "(empty)";

/**
 * Validates an argument and returns it as a single command token.
 *
 * The server tokenizes commands on whitespace and has no quoting, so every key
 * and value must be a non-empty, whitespace-free token.
 */
export function token(value: string | number, index = 0): string {
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

/** Validates and returns a list of tokens. */
export function tokens(values: Array<string | number>): string[] {
  return values.map((value, index) => token(value, index));
}

/** Maps the server's "(nil)" sentinel to `null`, otherwise returns the text. */
export function parseNil(text: string): string | null {
  return text === NIL ? null : text;
}

/** Parses an integer reply, throwing if the body is not a valid integer. */
export function parseIntReply(text: string): number {
  const value = Number.parseInt(text, 10);
  if (Number.isNaN(value)) {
    throw new VibesError(`Expected an integer reply but got "${text}"`);
  }
  return value;
}

/** Parses a boolean reply ("1" is true, anything else false). */
export function parseBool(text: string): boolean {
  return text === "1";
}

/** Splits a multi-line reply into lines ([] for an empty body). */
export function splitLines(text: string): string[] {
  return text.length === 0 ? [] : text.split("\n");
}
