import json
import logging
import sys
from enum import Enum
from typing import Optional

logging.basicConfig(format='%(levelname)-8s [%(filename)s:%(lineno)d] %(message)s', level=logging.DEBUG)


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

    @staticmethod
    def __add_ai_message_head(message: str):
        head = str(len(message))
        return '0' * (8 - len(head)) + head + message

    def __any_send(self, target: int, content: str):
        self.__send(-1, json.dumps({
            'state': self.__state,
            'listen': [target],
            'player': [target],
            'content': [
                content if self._player_status[target] == PlayerStatus.HUMAN else self.__add_ai_message_head(content)],
        }))

    def __get_target_message(self) -> (str, ErrorType, int):
        while True:
            v = self.__listen()
            if v['player'] >= 0:
                return v['content'], ErrorType.NONE, -1
            else:
                error_content = json.loads(v['content'])
                if error_content['state'] != self.__state:  # Special case for TLE
                    continue
                return error_content['error_log'], ErrorType(error_content['error']), error_content['player']

    @staticmethod
    def _debug(msg: str):
        logging.debug(msg, stacklevel=3)

    def _get_state(self) -> int:
        """
        Get current state of the logic.

        :return: value of `self.__state`
        """
        return self.__state

    def _write_text_to_replay(self, text: str):
        """
        Write text value to the replay file.

        Note that the replay file is opened with flag `w` rather than `a`.

        :param text:  the text value to write
        """
        with open(self.__replay_location, 'w') as f:
            f.write(text)

    def _write_json_to_replay(self, o: object):
        """
        Write JSON object to the replay file.

        Note that the replay file is opened with flag `w` rather than `a`.

        :param o:  the JSON object to write
        """
        with open(self.__replay_location, 'w') as f:
            f.write(json.dumps(o))

    def _single_send(self, target: int, msg: str):
        """
        Send message to a target player.

        :param target:  the target player ID
        :param msg:     the message to send
        """
        if target >= 0:
            self.__send(target,
                        msg if self._player_status[target] == PlayerStatus.HUMAN else self.__add_ai_message_head(msg))

    def _send_game_over_message(self, scores: [int]):
        """
        Send game-over message to judger.

        As logic will terminate as soon as the message is sent,
        please make sure that replay data has been written to the replay file before this method is called.

        :param scores:  players' scores, ordered by their indices
        """
        end_info = {str(i): score for i, score in enumerate(scores)}
        self.__send(-1, json.dumps({
            'state': -1,
            'end_info': json.dumps(end_info),
        }))
        self.__game_over = True
        sys.exit(0)

    def _prepare(self):
        """
        Executed after receiving metadata and before entering the major loop.
        """
        raise NotImplementedError()

    def _send_msg_to_player(self) -> (int, str, Optional[int], Optional[int]):
        """
        Executed before `_handle_response()`.

        This determines the player you will send message to and listen to in current round.

        You can also update the timeLimit and lengthLimit if you like.

        Note that timing of the target player starts IMMEDIATELY AFTER this method is called.

        :return:  a tuple of four elements. The first element is required, representing the
                  player ID that you will be listening to. The second element refers to the
                  content of the message you will be sending to the target player. The third
                  element represents the time limit of the current round, or None if the time
                  limit should remain unchanged. The fourth element represents the length
                  limit of the current round, or None if the length limit should remain unchanged.
        """
        raise NotImplementedError()

    def _handle_response(self, response: str, error_type: ErrorType, error_player: int):
        """
        Handle game logic as well as sending and receiving messages from players.

        :param response:      content of the response
        :param error_type:    type of error if any occurs, or `NONE` if none
        :param error_player:  the player that caused the error if any occurs, or -1 if none
        :return:
        """
        raise NotImplementedError()

    def run(self):
        self.__receive_metadata()
        self._prepare()
        while not self.__game_over:
            listen_target, content, time_limit, length_limit = self._send_msg_to_player()

            if time_limit is not None or length_limit is not None:
                if time_limit is not None:
                    self.__time_limit = time_limit
                if length_limit is not None:
                    self.__length_limit = length_limit
                self.__update_limits()

            self.__any_send(listen_target, content)

            response, error_type, error_player = self.__get_target_message()
            self._handle_response(response, error_type, error_player)
            self.__state += 1
