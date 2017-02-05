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

import argparse
import errno
import os
import pty
import re
import sys
import threading
import time
import tkinter
import xml.etree.ElementTree as ElementTree

ROWS = 24
COLUMNS = 80
TERMINFO = '/path_to_terminfo'

ATTR_NORMAL = 0
ATTR_REVERSE = 1

COLOR_BLACK = 0
COLOR_RED = 1
COLOR_GREEN = 2
COLOR_YELLOW = 3
COLOR_BLUE = 4
COLOR_MAGENTA = 5
COLOR_CYAN = 6
COLOR_WHITE = 7
COLOR_DEFAULT = 9
COLORS = (COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW, COLOR_BLUE,
          COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE, COLOR_DEFAULT)

COLOR_DEFAULT_FOREGROUND = COLOR_BLACK
COLOR_DEFAULT_BACKGROUND = COLOR_WHITE


def attr_to_string(attr):
    """Get string representation of given attributes."""

    res = []
    if (attr & ATTR_REVERSE) != 0:
        res.append("reverse")
    return "|".join(res)


def string_to_attr(string):
    """Convert a string to attributes."""

    res = ATTR_NORMAL
    for attr in string.split("|"):
        if attr == "normal":
            pass
        elif attr == "reverse":
            res |= ATTR_REVERSE
        else:
            raise ValueError("Unrecognized attribute '{}'".format(attr))
    return res


def color_to_string(color):
    """Get string representation of a given color."""

    if color == COLOR_BLACK:
        return "black"
    if color == COLOR_RED:
        return "red"
    if color == COLOR_GREEN:
        return "green"
    if color == COLOR_YELLOW:
        return "yellow"
    if color == COLOR_BLUE:
        return "blue"
    if color == COLOR_MAGENTA:
        return "magenta"
    if color == COLOR_CYAN:
        return "cyan"
    if color == COLOR_WHITE:
        return "white"
    return None


def string_to_color(string):
    """Convert a string to a color."""

    if string == "black":
        return COLOR_BLACK
    if string == "red":
        return COLOR_RED
    if string == "green":
        return COLOR_GREEN
    if string == "yellow":
        return COLOR_YELLOW
    if string == "blue":
        return COLOR_BLUE
    if string == "magenta":
        return COLOR_MAGENTA
    if string == "cyan":
        return COLOR_CYAN
    if string == "white":
        return COLOR_WHITE
    raise ValueError("Unrecognized color '{}'".format(string))


class TermChar:
    def __init__(self, char=" ", attr=ATTR_NORMAL, fgcolor=COLOR_DEFAULT,
                 bgcolor=COLOR_DEFAULT):
        self.char = char
        self.attr = attr
        self.fgcolor = fgcolor
        self.bgcolor = bgcolor

    def _get_translated_fgcolor(self):
        if self.fgcolor == COLOR_DEFAULT:
            return COLOR_DEFAULT_FOREGROUND
        return self.fgcolor

    def _get_translated_bgcolor(self):
        if self.bgcolor == COLOR_DEFAULT:
            return COLOR_DEFAULT_BACKGROUND
        return self.bgcolor

    def get_tag_foreground(self):
        if self.attr & ATTR_REVERSE:
            color = self._get_translated_bgcolor()
        else:
            color = self._get_translated_fgcolor()
        return color_to_string(color)

    def get_tag_background(self):
        if self.attr & ATTR_REVERSE:
            color = self._get_translated_fgcolor()
        else:
            color = self._get_translated_bgcolor()
        return color_to_string(color)


class Term(tkinter.Frame):
    def __init__(self, root, command, record=False):
        super().__init__(root)
        self._root = root
        self._root.title("Termex")

        self._record = record
        if self._record:
            self._test_e = ElementTree.Element("test")

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
        self._attr = ATTR_NORMAL
        self._fgcolor = COLOR_DEFAULT
        self._bgcolor = COLOR_DEFAULT

        self._text = tkinter.Text(self._root, height=ROWS, width=COLUMNS)
        self._text.config(foreground=color_to_string(COLOR_DEFAULT_FOREGROUND),
                          background=color_to_string(COLOR_DEFAULT_BACKGROUND))
        self._text.pack()
        self._erase_all()

        self._root.createfilehandler(self._fd, tkinter.READABLE,
                                     self._pty_callback)
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
                self._print_char(
                    TermChar(uchar, self._attr, self._fgcolor, self._bgcolor))
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
        match = re.fullmatch(b'\x1b\\[([0-9]+)@', seq)
        if match:
            # Insert blank characters.
            self._insert_blanks(int(match.group(1)))
            return True
        if seq == b'\x1b[H':
            # Set cursor position to the default (top left).
            self._cur_y = 0
            self._cur_x = 0
            return True
        match = re.fullmatch(b'\x1b\\[([0-9]+);([0-9]+)H', seq)
        if match:
            # Set cursor position to (y,x).
            self._cur_y = int(match.group(1))
            self._cur_x = int(match.group(2))
            return True
        if self._charbuf == b'\x1b[K':
            # Erase in line to right.
            for x in range(self._cur_x, COLUMNS):
                self._print_char_at(self._cur_y, x, TermChar())
            return True
        if seq == b'\x1b[2J':
            # Erase display completely.
            self._erase_all()
            return True
        if seq == b'\x1b[m':
            # Normal character attribute (all attributes off).
            self._attr = ATTR_NORMAL
            return True
        if seq == b'\x1b[7m':
            # Inverse character attribute.
            self._attr |= ATTR_REVERSE
            return True
        match = re.fullmatch(b'\x1b\\[3([0-9]+)m', seq)
        if match:
            # Set foreground color.
            color = int(match.group(1))
            if color in COLORS:
                self._fgcolor = color
                return True
            return False
        match = re.fullmatch(b'\x1b\\[4([0-9]+)m', seq)
        if match:
            # Set background color.
            color = int(match.group(1))
            if color in COLORS:
                self._bgcolor = color
                return True
            return False
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

            self._text.config(state=tkinter.NORMAL)
            self._text.delete('1.0', '2.0')
            self._text.insert(tkinter.END, "\n" + " " * COLUMNS)
            self._text.config(state=tkinter.DISABLED)

    def _erase_all(self):
        self._screen = [[TermChar() for x in range(COLUMNS)]
                        for y in range(ROWS)]
        self._text.config(state=tkinter.NORMAL)
        self._text.delete('1.0', tkinter.END)
        self._text.insert('1.0', "\n".join([" " * COLUMNS] * ROWS))
        self._text.config(state=tkinter.DISABLED)

    def _insert_blanks(self, w):
        del self._screen[self._cur_y][-w:]
        pre = self._screen[self._cur_y][:self._cur_x]
        post = self._screen[self._cur_y][self._cur_x:]
        self._screen[self._cur_y] = pre + [TermChar() for x in range(w)] + post

        self._text.config(state=tkinter.NORMAL)
        self._text.delete('{}.end-{}c'.format(self._cur_y + 1, w),
                          '{}.end'.format(self._cur_y + 1))
        self._text.insert('{}.{}'.format(self._cur_y + 1, self._cur_x),
                          " " * w)
        self._text.config(state=tkinter.DISABLED)

    def _print_char_at(self, y, x, char):
        # Record the character in the internal screen representation.
        self._screen[y][x] = char

        # Add the character to the terminal text widget.
        self._text.config(state=tkinter.NORMAL)
        pos = '{}.{}'.format(y + 1, x)
        self._text.delete(pos)

        # Configure tag. There is one tag for each used combination of
        # foreground+background color. The tags are currently never deleted.
        # Their maximum count is colors# * colors#, which is 256.
        foreground = char.get_tag_foreground()
        background = char.get_tag_background()
        tag = "tag_{}-{}".format(foreground, background)
        self._text.tag_config(tag, foreground=foreground,
                              background=background)

        self._text.insert(pos, char.char, tag)
        self._text.config(state=tkinter.DISABLED)

    def _print_char(self, char):
        self._print_char_at(self._cur_y, self._cur_x, char)

        # Advance the cursor.
        self._cur_x += 1
        if self._cur_x == COLUMNS:
            self._cur_x = 0
            self._cursor_down()

    def _key(self, event):
        if len(event.char) != 0:
            if event.char == '\x0d':
                self._send_key('Enter', event.char)
            else:
                self._send_key(event.char, event.char)
        else:
            # A special key was pressed.
            if event.keysym == 'F1':
                self._send_key(event.keysym, '\x1bOP')
            elif event.keysym == 'F2':
                self._send_key(event.keysym, '\x1bOQ')
            elif event.keysym == 'F3':
                self._send_key(event.keysym, '\x1bOR')
            elif event.keysym == 'F4':
                self._send_key(event.keysym, '\x1bOS')
            elif event.keysym == 'F5':
                self._send_key(event.keysym, '\x1b[15~')
            elif event.keysym == 'F6':
                self._send_key(event.keysym, '\x1b[17~')
            elif event.keysym == 'F7':
                self._send_key(event.keysym, '\x1b[18~')
            elif event.keysym == 'F8':
                self._send_key(event.keysym, '\x1b[19~')
            elif event.keysym == 'F9':
                self._send_key(event.keysym, '\x1b[20~')
            elif event.keysym == 'F10':
                self._send_key(event.keysym, '\x1b[21~')
            elif event.keysym == 'F11':
                self._send_key(event.keysym, '\x1b[23~')
            elif event.keysym == 'F12':
                self.record_expected_screen()
            elif event.keysym == 'Prior':
                self._send_key('PageUp', '\x1b[5~')
            elif event.keysym == 'Next':
                self._send_key('PageDown', '\x1b[6~')
            else:
                print("Unrecognized key {}.".format(event.keysym))

    def _term_initialized(self):
        self._initialized.set()

    def wait_for_init(self):
        # Wait for the main loop to start.
        self._initialized.wait()

    def _send_key(self, name, chars):
        if self._record:
            # Record the key.
            action_e = ElementTree.SubElement(self._test_e, "action")
            action_e.set("key", name)
            print("Recorded key '{}'.".format(name))

        # Send the key to the terminal.
        self._fd_lock.acquire()
        if self._fd != -1:
            os.write(self._fd, str.encode(chars))
        else:
            print("Error writing to terminal because it is closed.",
                  file=sys.stderr)
        self._fd_lock.release()

    def record_expected_screen(self):
        if not self._record:
            print("Recording is not enabled.", file=sys.stderr)
            return

        expect_e = ElementTree.SubElement(self._test_e, "expect")
        data_e = ElementTree.SubElement(expect_e, "data")

        colors = {}
        new_key = "a"

        # Print content of the screen.
        for y in range(ROWS):
            line_e = ElementTree.SubElement(data_e, "line")
            line_e.text = ""

            attr = ""

            for x in range(COLUMNS):
                term_char = self._screen[y][x]

                line_e.text += term_char.char

                color = (term_char.attr, term_char.fgcolor, term_char.bgcolor)
                if color == (ATTR_NORMAL, COLOR_DEFAULT, COLOR_DEFAULT):
                    key = " "
                elif color in colors:
                    key = colors[color]
                else:
                    key = new_key
                    colors[color] = key
                    assert new_key != "z"
                    new_key = chr(ord(new_key) + 1)
                attr += key

            # Record any non-default attributes/colors.
            if attr != " " * COLUMNS:
                attr_e = ElementTree.SubElement(data_e, "attr")
                attr_e.text = attr

        # Record used color schemes.
        if colors:
            scheme_e = ElementTree.SubElement(expect_e, "scheme")
            for color, key in sorted(colors.items(), key=lambda x: x[1]):
                attr, fgcolor, bgcolor = color
                color_e = ElementTree.SubElement(scheme_e, "color")
                color_e.set("key", key)

                attr_str = attr_to_string(attr)
                if attr_str:
                    color_e.set("attributes", attr_str)

                fgcolor_str = color_to_string(fgcolor)
                if fgcolor_str:
                    color_e.set("foreground", fgcolor_str)

                bgcolor_str = color_to_string(bgcolor)
                if bgcolor_str:
                    color_e.set("background", bgcolor_str)

        print("Recorded expected screen.")

    def output_test(self, filename):
        assert self._record

        # Function indent_xml() is based on a code from
        # http://effbot.org/zone/element-lib.htm#prettyprint.
        def indent_xml(elem, level=0):
            """Prepare a given ElementTree element for pretty-printing."""

            i = "\n" + "\t" * level
            if len(elem):
                if not elem.text or not elem.text.strip():
                    elem.text = i + "\t"
                for e in elem:
                    indent_xml(e, level+1)
                if not e.tail or not e.tail.strip():
                    e.tail = i
            if not elem.tail or not elem.tail.strip():
                elem.tail = i

        # Pretty-format the XML tree.
        indent_xml(self._test_e)

        # Output the test.
        tree = ElementTree.ElementTree(self._test_e)
        tree.write(filename, 'unicode', True)


def command_run(args):
    """Handle the 'run' command."""

    # Start the terminal GUI.
    term = Term(tkinter.Tk(), args.prog)
    term.run_mainloop()


def command_record(args):
    """Handle the 'record' command."""

    # Start the terminal in the record mode.
    term = Term(tkinter.Tk(), args.prog, True)
    term.run_mainloop()

    # Get the recorded test data and write them to a file.
    term.output_test(args.playbook)


def command_test(args):
    """Handle the 'test' command."""

    pass


def main():
    # Parse command line arguments.
    parser = argparse.ArgumentParser()
    parser.set_defaults(func=None)
    subparsers = parser.add_subparsers(dest='command')
    subparsers.required = True

    prog_parser = argparse.ArgumentParser(add_help=False)
    prog_parser.add_argument("prog")

    # Create the parser for the "run" command.
    parser_run = subparsers.add_parser(
        "run", parents=[prog_parser], help="run a command")
    parser_run.set_defaults(func=command_run)

    # Create the parser for the "record" command.
    parser_record = subparsers.add_parser(
        "record", parents=[prog_parser], help="record a test")
    parser_record.set_defaults(func=command_record)
    parser_record.add_argument(
        "-p", "--playbook", metavar="FILE", required=True,
        help="output playbook file")

    # Create the parser for the "test" command.
    parser_test = subparsers.add_parser(
        "test", parents=[prog_parser], help="run a test")
    parser_test.set_defaults(func=command_test)
    parser_test.add_argument(
        "-p", "--playbook", metavar="FILE", required=True,
        help="input playbook file")

    args = parser.parse_args()
    args.func(args)


if __name__ == '__main__':
    main()
