import logging
import sys

logging.basicConfig(format='%(levelname)-8s [%(filename)s:%(lineno)d] %(message)s', level=logging.DEBUG)


class Client:
    @staticmethod
    def __send_msg(msg: str):
        message_len = len(msg)
        message = message_len.to_bytes(4, byteorder='big', signed=True)
        message += bytes(msg, encoding="utf8")
        sys.stdout.buffer.write(message)
        sys.stdout.flush()

    @staticmethod
    def __read_msg() -> str:
        read_buffer = sys.stdin.buffer
        data_len = int(read_buffer.read(8))
        data = read_buffer.read(data_len).decode()
        return data

    _my_history = []
    _enemy_history = []

    @staticmethod
    def _debug(msg: str):
        logging.debug(msg, stacklevel=3)

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
            if enemy_choice != 'ready':
                self._enemy_history.append(enemy_choice)
            else:
                enemy_choice = ''
            my_choice = self._strategy(enemy_choice)
            self._my_history.append(my_choice)
            self.__send_msg(my_choice)
