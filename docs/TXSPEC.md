Message Specification
-------------------------

### Format

Messages passed to the Ledger device will be in the following format. The Ledger device MUST accept any transaction (valid as below) in this format.

```diff
+ WARNING: This is work in progress, the format is expected to change
```


```json
{
  "height": number,
  "other": string,
  "round": number
}
```

- round: int8 
- height: int64
- other: dummy field

all fields apart from round and height will be ignored

### Validation

The Ledger device MUST validate that supplied JSON is valid. Our JSON specification is a subset of [RFC 7159](https://tools.ietf.org/html/rfc7159) - invalid RFC 7159 JSON is invalid Ledger JSON, but not all valid RFC 7159 JSON is valid Ledger JSON.

Comments are **not** supported.

We add the following two rules:
- No spaces or newlines can be included, other than escaped in strings
- All dictionaries must be serialized in lexicographical key order

This serves to prevent signature compatibility issues among different client libraries.

This is equivalent to the following Python snippet:

```python
import json

def ledger_validate(json_str):
  obj = json.loads(json_str)
  canonical = json.dumps(obj, sort_keys = True, separators = (',', ':'))
  return canonical == json_str

assert ledger_validate('{"a":2,"b":3}')
assert ledger_validate('{"a ":2,"b":3}')
assert not ledger_validate('{"a":2,\n"b":3}')
assert not ledger_validate('{"b":2,"a":3}')
assert not ledger_validate('{"a" : 2 }')
```
