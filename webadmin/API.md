# Webadmin REST API

The webadmin exposes a small REST API under `/api/**` so external applications
can change competitor status in the running competition. The first consumer is
the [DNS start-line app](https://github.com/mstahv/dns), which marks competitors
that did not start.

## Authentication

All `/api/**` requests require a shared API key. Provide it in the
`X-API-Key` header (or `Authorization: Bearer <key>`).

Configure the key the usual Spring Boot way (any of these):

```properties
# application.properties
tulospalvelu.api.key=your-secret-key
```

```bash
export TULOSPALVELU_API_KEY=your-secret-key      # environment variable
java -jar webadmin.jar --tulospalvelu.api.key=your-secret-key   # CLI arg
```

If no key is configured the API is **disabled** — every request gets
`503 Service Unavailable`. The key is compared in constant time.

## Endpoints

### Connectivity / key check

```
GET /api/v1/ping
```

Verifies the connection works and the api key is accepted. A `200` means the
key is correct; `401`/`503` mean it is missing/invalid or the API is disabled.
The body also tells whether webadmin currently has a live connection to the
Tulospalvelu server, so a client can distinguish "key is good but the
competition isn't reachable" from "key is wrong":

```json
{ "ok": true, "connected": false }
```

### Competitor status

Competitors are addressed by their competition number (`kilpno`).

Both endpoints validate the current state, so a client need not track it:

### Mark not started (DNS / ei lähtenyt)

```
POST /api/v1/competitors/{kilpno}/dns
```

Sets the status to `E` (ei lähtenyt). **Only allowed when the competitor is
open** (no result and no decided status):

- already DNS → `200`, `changed: false` (no-op)
- open → `200`, `changed: true` (status set)
- has a result or any other status → `409 Conflict`, nothing changed

### Reopen (avoin)

```
POST /api/v1/competitors/{kilpno}/open
```

Reverts the status to `-` (avoin). Use this when a competitor previously
marked DNS turns up and is registered at the start after all. **Only allowed
when the competitor is currently DNS**:

- already open → `200`, `changed: false` (no-op)
- DNS → `200`, `changed: true` (reverted)
- any other status → `409 Conflict`, nothing changed

### Response body

```json
{
  "kilpno": 101,
  "name": "Liisa Lähtijä",
  "status": "E",
  "changed": true,
  "message": "Merkitty ei lähteneeksi (DNS)"
}
```

`changed` tells whether this request actually altered the status. On `409` the
body carries the same shape with `changed: false` and a `message` explaining
the current state, e.g. `"Ei voitu merkitä ei lähteneeksi: kilpailijan nykyinen
tila on \"3. 1:21:39\""`.

### Errors

| Status | When |
|--------|------|
| `401 Unauthorized` | Missing or invalid api key |
| `404 Not Found` | No competitor with that `kilpno` |
| `409 Conflict` | The competitor's current state does not allow the requested change |
| `502 Bad Gateway` | The Tulospalvelu C++ server rejected the change |
| `503 Service Unavailable` | API disabled (no key) or no connection to the Tulospalvelu server |

## Example

```bash
curl -X POST http://localhost:8080/api/v1/competitors/101/dns \
     -H "X-API-Key: your-secret-key"
```
