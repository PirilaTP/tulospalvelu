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

Competitors are addressed by their competition number (`kilpno`).

### Mark not started (DNS / ei lähtenyt)

```
POST /api/v1/competitors/{kilpno}/dns
```

Sets the competitor's status to `E` (ei lähtenyt).

### Reopen (avoin)

```
POST /api/v1/competitors/{kilpno}/open
```

Reverts the status to `-` (avoin). Use this when a competitor previously
marked DNS turns up and is registered at the start after all.

### Success response (`200 OK`)

```json
{
  "kilpno": 101,
  "name": "Liisa Lähtijä",
  "status": "E",
  "message": "Merkitty ei lähteneeksi (DNS)"
}
```

### Errors

| Status | When |
|--------|------|
| `401 Unauthorized` | Missing or invalid api key |
| `404 Not Found` | No competitor with that `kilpno` |
| `502 Bad Gateway` | The Tulospalvelu C++ server rejected the change |
| `503 Service Unavailable` | API disabled (no key) or no connection to the Tulospalvelu server |

## Example

```bash
curl -X POST http://localhost:8080/api/v1/competitors/101/dns \
     -H "X-API-Key: your-secret-key"
```
