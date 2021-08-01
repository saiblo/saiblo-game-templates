import sys


class Client:
    def __send_msg(self, msg: str):
        message_len = len(msg)
        message = message_len.to_bytes(4, byteorder='big', signed=True)
        message += bytes(msg, encoding="utf8")
        sys.stdout.buffer.write(message)
        sys.stdout.flush()
        self._my_history.append(msg)

    def __read_msg(self) -> str:
        msg = sys.stdin.readline()
        if msg != 'ready':
            self._enemy_history.append(msg)
        else:
            msg = ''
        return msg

    _my_history = []
    _enemy_history = []

    @staticmethod
    def _greater(left: str, right: str) -> bool:
        return left == "rock" and right == "scissors" or \
               left == "scissors" and right == "paper" or \
               left == "paper" and right == "rock"

    def _strategy(self, enemy_choice: str) -> str:
        raise NotImplementedError()

    def run(self):
        while True:
            enemy_choice = self.__read_msg()
            my_choice = self._strategy(enemy_choice)
            self.__send_msg(my_choice)
