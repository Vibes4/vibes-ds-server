import { describe, expect, it } from "vitest";

import {
  parseBool,
  parseIntReply,
  parseNil,
  splitLines,
  token,
  tokens,
} from "../../src/core/encode.js";
import { VibesArgumentError, VibesError } from "../../src/index.js";

describe("token", () => {
  it("passes through simple string and number tokens", () => {
    expect(token("abc")).toBe("abc");
    expect(token(42)).toBe("42");
    expect(token(-7)).toBe("-7");
  });

  it("rejects empty tokens", () => {
    expect(() => token("")).toThrow(VibesArgumentError);
  });

  it("rejects tokens containing whitespace", () => {
    expect(() => token("a b")).toThrow(VibesArgumentError);
    expect(() => token("tab\there")).toThrow(VibesArgumentError);
  });

  it("tokens() validates every element", () => {
    expect(tokens(["a", 1, "b"])).toEqual(["a", "1", "b"]);
    expect(() => tokens(["ok", "bad value"])).toThrow(VibesArgumentError);
  });
});

describe("reply parsing", () => {
  it("parseNil maps the (nil) sentinel to null", () => {
    expect(parseNil("(nil)")).toBeNull();
    expect(parseNil("value")).toBe("value");
    expect(parseNil("")).toBe("");
  });

  it("parseIntReply parses integers and rejects junk", () => {
    expect(parseIntReply("5")).toBe(5);
    expect(parseIntReply("-2")).toBe(-2);
    expect(() => parseIntReply("nope")).toThrow(VibesError);
  });

  it("parseBool treats only '1' as true", () => {
    expect(parseBool("1")).toBe(true);
    expect(parseBool("0")).toBe(false);
    expect(parseBool("")).toBe(false);
  });

  it("splitLines splits non-empty bodies", () => {
    expect(splitLines("")).toEqual([]);
    expect(splitLines("a")).toEqual(["a"]);
    expect(splitLines("a\nb\nc")).toEqual(["a", "b", "c"]);
  });
});
