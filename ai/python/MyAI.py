from sdk.Client import Client


class MyAI(Client):
    def _strategy(self, enemy_choice: str) -> str:
        # Copy what the enemy chose
        return enemy_choice
