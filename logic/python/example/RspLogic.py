from typing import Optional

from sdk.BaseLogic import BaseLogic, ErrorType


class RspLogic(BaseLogic):
    __turn = 0

    __history = []

    __win = [0, 0]

    def _prepare(self):
        pass  # Do nothing...

    def _set_listen_target(self) -> (int, Optional[int], Optional[int]):
        # Time and length limits remain default and do not need updating.
        return self.__turn, None, None

    def __write_replay_and_game_over(self):
        self._write_text_to_replay('\n'.join(self.__history))
        self._send_game_over_message(self.__win)

    def __send_msg_to_player(self):
        if self._get_state() <= 2:
            send_content = "ready"
        elif self.__turn == 0:
            send_content = self.__history[self._get_state() - 2]
        else:
            send_content = self.__history[self._get_state() - 4]
        self._single_send(self.__turn, send_content)

    def __listen_to_player_response(self):
        recv_content, error_type, error_player = self._get_target_message()
        if error_type == ErrorType.NONE:
            # Error handling is VERY IMPORTANT!!!
            # Even if no error occurs in the process of listening,
            # there can still be errors in the `recvContent` itself!!!
            if recv_content not in ['rock', 'scissors', 'paper']:
                error_type = ErrorType.RE
                error_player = self.__turn
            else:
                self.__history.append(recv_content)

        # Terminate game if any error occurs.
        if error_type != ErrorType.NONE:
            self.__history.append(f'Player {error_player} error: {error_type} {recv_content}')
            self.__win[1 - error_player] = 1
            self.__win[error_player] = 0
            self.__write_replay_and_game_over()

    def __reverse_turn(self):
        self.__turn = 1 - self.__turn  # Change turn.
        if self.__turn == 0:
            # Both player 0 and player 1 have made their choices.
            first, second = self.__history[self._get_state() - 2], self.__history[self._get_state() - 1]
            if first == "rock" and second == "scissors" or \
                    first == "scissors" and second == "paper" or \
                    first == "paper" and second == "rock":
                self.__win[0] += 1
            elif first != second:
                self.__win[1] += 1

            if self.__win[0] >= 3 or self.__win[1] >= 3:
                self.__history.append(f'{self.__win[0]}:{self.__win[1]}')
                self.__write_replay_and_game_over()

    def _handle_logic(self):
        # `handleLogic()` does not necessarily have to be composed of these three phases.
        # You can create your own way of logic handling.
        self.__send_msg_to_player()
        self.__listen_to_player_response()
        self.__reverse_turn()
