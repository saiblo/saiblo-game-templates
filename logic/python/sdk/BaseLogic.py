import json
import sys
from enum import Enum
from typing import Optional


class PlayerStatus(Enum):
    FAIL = 0
    AI = 1
    HUMAN = 2


class ErrorType(Enum):
    NONE = -1
    RE = 0
    TLE = 1
    OLE = 2


class BaseLogic:
    __replay_location = ''

    __state = 1

    __listen_target = -1

    __time_limit = 3
    __length_limit = 1024

    __game_over = False

    _player_status: [PlayerStatus] = []

    @staticmethod
    def __listen():
        read_buffer = sys.stdin.buffer
        data_len = int.from_bytes(read_buffer.read(4), byteorder='big', signed=True)
        data = json.loads(read_buffer.read(data_len).decode())
        return data

    def __receive_metadata(self):
        v = self.__listen()
        self._player_status = [PlayerStatus(x) for x in v['player_list']]
        self.__replay_location = v['replay']

    @staticmethod
    def __send(target: int, msg: str):
        message_len = len(msg)
        message = message_len.to_bytes(4, byteorder='big', signed=True)
        message += target.to_bytes(4, byteorder='big', signed=True)
        message += bytes(msg, encoding="utf8")
        sys.stdout.buffer.write(message)
        sys.stdout.flush()

    def __update_limits(self):
        self.__send(-1, json.dumps({
            'state': 0,
            'time': self.__time_limit,
            'length': self.__length_limit,
        }))

    def _get_state(self) -> int:
        return self.__state

    def _write_text_to_replay(self, text: str):
        with open(self.__replay_location, 'w') as f:
            f.write(text)

    def _write_json_to_replay(self, o: object):
        with open(self.__replay_location, 'w') as f:
            f.write(json.dumps(o))

    def _single_send(self, target: int, msg: str):
        if target >= 0:
            self.__send(target, msg)

    def _any_send(self, messages: [(int, str)]):
        self.__send(-1, json.dumps({
            'state': self.__state,
            'listen': [self.__listen_target] if self.__listen_target >= 0 else [],
            'player': [x for (x, y) in messages],
            'content': [y for (x, y) in messages],
        }))

    def _get_target_message(self) -> (str, ErrorType, int):
        while True:
            v = self.__listen()
            if v['player'] >= 0:
                return v['content'], ErrorType.NONE, -1
            else:
                error_content = json.loads(v['content'])
                if error_content['state'] != self.__state:  # Special case for TLE
                    continue
                return error_content['error_log'], ErrorType(error_content['error']), error_content['player']

    def _send_game_over_message(self, scores: [int]):
        end_info = {str(i): score for i, score in enumerate(scores)}
        self.__send(-1, json.dumps({
            'state': -1,
            'end_info': json.dumps(end_info),
        }))
        self.__game_over = True
        sys.exit(0)

    def _prepare(self):
        raise NotImplementedError()

    def _set_listen_target(self) -> (int, Optional[int], Optional[int]):
        raise NotImplementedError()

    def _handle_logic(self):
        raise NotImplementedError()

    def run(self):
        self.__receive_metadata()
        self._prepare()
        while not self.__game_over:
            listen_target, time_limit, length_limit = self._set_listen_target()

            if time_limit is not None or length_limit is not None:
                if time_limit is not None:
                    self.__time_limit = time_limit
                if length_limit is not None:
                    self.__length_limit = length_limit
                self.__update_limits()

            if listen_target >= 0:
                self._any_send([])

            self._handle_logic()
            self.__state += 1
