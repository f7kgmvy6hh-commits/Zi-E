import hmac


class BearerAuth:
    def __init__(self, token: str):
        self._token = token

    def valid(self, authorization: str | None) -> bool:
        if not authorization or not authorization.startswith("Bearer "):
            return False
        return hmac.compare_digest(authorization[7:], self._token)
