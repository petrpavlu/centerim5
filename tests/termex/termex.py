#!/usr/bin/env python3

# Copyright (C) 2016-2017 Petr Pavlu <setup@dagobah.cz>
#
# This file is part of CenterIM.
#
# CenterIM is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# CenterIM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with CenterIM.  If not, see <http://www.gnu.org/licenses/>.

import errno
import os
import pty
import re
import sys
import threading
import time
import tkinter as tk

ROWS = 24
COLUMNS = 80
SHELL = '/bin/sh'
TERMINFO = '/path_to_terminfo'


class TermChar:
    def __init__(self):
        self.char = ' '

    def __str__(self):
        return self.char


class Term(tk.Frame):
    def __init__(self, root, command):
        super().__init__(root)
        self._root = root
        self._root.title("Termex")

        self._charbuf = b''
        self._fd_lock = threading.Lock()
        self._initialized = threading.Event()

        # Fork and connect the child's controlling terminal to a
        # pseudo-terminal.
        env = {'PATH': '/bin:/usr/bin', 'TERM': 'termex', 'TERMINFO': TERMINFO,
               'LC_ALL': 'en_US.UTF-8'}
        pid, self._fd = pty.fork()
        if pid == 0:
            os.execle(command, command, env)

        self._screen = None
        self._cur_y = 0
        self._cur_x = 0

        self._text = tk.Text(self._root, height=ROWS, width=COLUMNS)
        self._text.pack()
        self._erase_all()

        self._root.createfilehandler(self._fd, tk.READABLE, self._pty_callback)
        self._root.bind('<Key>', self._key)
        self._root.bind('<<Quit>>', lambda e: self._quit())
        self._root.protocol('WM_DELETE_WINDOW', self._quit)
        self._root.after(0, self._term_initialized)

    def run_mainloop(self):
        # Run the main loop.
        self._root.mainloop()

        self._fd_lock.acquire()
        self._root.deletefilehandler(self._fd)
        os.close(self._fd)
        self._fd = -1
        self._fd_lock.release()

        # Wait for the child to finish. It should terminate now that its input
        # was closed.
        os.wait()

    def close(self):
        # FIXME Do not send the event if the main loop already exited.
        self._root.event_generate('<<Quit>>', when='tail')

    def _quit(self):
        self._root.quit()

    def _pty_callback(self, f, mask):
        try:
            char = os.read(f, 1)
        except OSError as e:
            if e.errno == errno.EIO:
                print("Connection to terminal lost.")
                self._root.quit()
                return
            raise

        # print("Received {}.".format(char))
        self._charbuf += char

        if self._handle_sequence(self._charbuf):
            self._charbuf = b''
        else:
            print("Unmatched {}.".format(self._charbuf))
            pass

    def _handle_sequence(self, seq):
        if re.fullmatch(b'[^\x01-\x1f]+', seq):
            try:
                uchar = seq.decode('utf-8')
                self._print_char(uchar)
                return True
            except UnicodeError:
                # Continue on the assumption that it is not yet a complete
                # character. This assumption is wrong if the received text is
                # actually malformed.
                return False

        if seq == b'\x07':
            # Bell.
            self.bell()
            return True
        if seq == b'\x08':
            # Backspace non-destructively.
            self._cur_x -= 1
            return True
        if seq == b'\x0d':
            # Go to beginning of line.
            self._cur_x = 0
            return True
        if seq == b'\x0a':
            # Move cursor down one line.
            self._cursor_down()
            return True

        # Controls beginning with ESC.

        # Control sequences.
        m = re.fullmatch(b'\x1b\\[([0-9]+)@', seq)
        if m:
            # Insert blank characters.
            self._insert_blanks(int(m.group(1)))
            return True
        if seq == b'\x1b[H':
            # Set cursor position to the default (top left).
            self._cur_y = 0
            self._cur_x = 0
            return True
        m = re.fullmatch(b'\x1b\\[([0-9]+);([0-9]+)H', seq)
        if m:
            # Set cursor position to (y,x).
            self._cur_y = int(m.group(1))
            self._cur_x = int(m.group(2))
            return True
        if self._charbuf == b'\x1b[K':
            # Erase in line to right.
            for x in range(self._cur_x, COLUMNS):
                self._print_char_at(self._cur_y, x, ' ')
            return True
        if seq == b'\x1b[2J':
            # Erase display completely.
            self._erase_all()
            return True
        if seq == b'\x1b[m':
            # Normal character attribute.
            # FIXME Implement.
            return True
        if seq == b'\x1b[7m':
            # Inverse character attribute.
            # FIXME Implement.
            return True
        if seq == b'\x1b[?25l':
            # Hide cursor.
            return True

        return False

    def _cursor_down(self):
        if self._cur_y < ROWS - 1:
            self._cur_y += 1
        else:
            assert self._cur_y == ROWS - 1

            # At the last line of the terminal, scroll up the screen.
            del self._screen[0]
            self._screen.append([TermChar() for x in range(COLUMNS)])

            self._text.config(state=tk.NORMAL)
            self._text.delete('1.0', '2.0')
            self._text.insert(tk.END, '\n' + ' ' * COLUMNS)
            self._text.config(state=tk.DISABLED)

    def _erase_all(self):
        self._screen = [[TermChar() for x in range(COLUMNS)]
                        for y in range(ROWS)]
        self._text.config(state=tk.NORMAL)
        self._text.delete('1.0', tk.END)
        self._text.insert('1.0', '\n'.join([' ' * COLUMNS] * ROWS))
        self._text.config(state=tk.DISABLED)

    def _insert_blanks(self, w):
        del self._screen[self._cur_y][-w:]
        pre = self._screen[self._cur_y][:self._cur_x]
        post = self._screen[self._cur_y][self._cur_x:]
        self._screen[self._cur_y] = pre + [TermChar() for x in range(w)] + post

        self._text.config(state=tk.NORMAL)
        self._text.delete('{}.end-{}c'.format(self._cur_y + 1, w),
                          '{}.end'.format(self._cur_y + 1))
        self._text.insert('{}.{}'.format(self._cur_y + 1, self._cur_x),
                          ' ' * w)
        self._text.config(state=tk.DISABLED)

    def _print_char_at(self, y, x, char):
        # print("Print {} at ({},{})".format(char, y, x))

        # Record the character in the internal screen representation.
        self._screen[y][x].char = char

        # Add the character to the terminal text widget.
        self._text.config(state=tk.NORMAL)
        pos = '{}.{}'.format(y + 1, x)
        self._text.delete(pos)
        self._text.insert(pos, char)
        self._text.config(state=tk.DISABLED)

    def _print_char(self, char):
        self._print_char_at(self._cur_y, self._cur_x, char)

        # Advance the cursor.
        self._cur_x += 1
        if self._cur_x == COLUMNS:
            self._cur_x = 0
            self._cursor_down()

    def _key(self, event):
        if len(event.char) != 0:
            self.write(event.char)
        else:
            # A special key was pressed.
            if event.keysym == 'F1':
                self.write('\x1bOP')
            elif event.keysym == 'F2':
                self.write('\x1bOQ')
            elif event.keysym == 'F3':
                self.write('\x1bOR')
            elif event.keysym == 'F4':
                self.write('\x1bOS')
            elif event.keysym == 'F5':
                self.write('\x1b[15~')
            elif event.keysym == 'F6':
                self.write('\x1b[17~')
            elif event.keysym == 'F7':
                self.write('\x1b[18~')
            elif event.keysym == 'F8':
                self.write('\x1b[19~')
            elif event.keysym == 'F9':
                self.write('\x1b[20~')
            elif event.keysym == 'F10':
                self.write('\x1b[21~')
            elif event.keysym == 'F11':
                self.write('\x1b[23~')
            elif event.keysym == 'F12':
                self.print_screen()
            else:
                print("Unrecognized key {}.".format(event.keysym))

    def _term_initialized(self):
        self._initialized.set()

    def wait_for_init(self):
        # Wait for the main loop to start.
        self._initialized.wait()

    def write(self, string):
        self._fd_lock.acquire()
        if self._fd != -1:
            os.write(self._fd, str.encode(string))
        else:
            print("Error writing to terminal because it is closed.",
                  file=sys.stderr)
        self._fd_lock.release()

    def print_screen(self):
        # Print top border.
        print(",", end="")
        for x in range(COLUMNS):
            print("-", end="")
        print(",")

        # Print actual content of the screen.
        for y in range(ROWS):
            print("|", end="")
            for x in range(COLUMNS):
                print(self._screen[y][x], end="")
            print("|")

        # Print bottom border.
        print("'", end="")
        for x in range(COLUMNS):
            print("-", end="")
        print("'")


class Expect:
    def __init__(self, term):
        self.term = term

    def send(self, string):
        self.term.write(string)

    def expect(self, string):
        pass


def main():
    # Start the terminal GUI on the main thread and create a new thread for the
    # actual test code.
    term = Term(tk.Tk(), SHELL)
    test = threading.Thread(target=run_test, args=(term,))
    test.start()
    term.run_mainloop()


def run_test(term):
    # Wait for the terminal GUI to reach the main loop.
    term.wait_for_init()

    exp = Expect(term)
    # exp.send('ls\n')
    # time.sleep(5)

    # term.print_screen()
    # term.close()

if __name__ == '__main__':
    main()
