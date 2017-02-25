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

"""Termex terminal emulator and test framework."""

import argparse
import difflib
import errno
import os
import pty
import re
import selectors
import sys
import time
import tkinter
import xml.etree.ElementTree as ElementTree

# The module relies on selectors.select() to automatically retry the operation
# with a recomputed timeout when it gets interrupted by a signal. This
# behaviour was introduced in Python 3.5.
if sys.hexversion < 0x03050000:
    print("This program requires at least Python 3.5.", file=sys.stderr)
    sys.exit(1)

ROWS = 24
COLUMNS = 80
CHILD_TIMEOUT = 5

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
COLOR_REGISTER = ((COLOR_BLACK, 'black'), (COLOR_RED, 'red'),
                  (COLOR_GREEN, 'green'), (COLOR_YELLOW, 'yellow'),
                  (COLOR_BLUE, 'blue'), (COLOR_MAGENTA, 'magenta'),
                  (COLOR_CYAN, 'cyan'), (COLOR_WHITE, 'white'),
                  (COLOR_DEFAULT, 'default'))
COLOR_TO_STRING_MAP = {id_: name for id_, name in COLOR_REGISTER}
STRING_TO_COLOR_MAP = {name: id_ for id_, name in COLOR_REGISTER}
COLORS = {id_ for id_, _ in COLOR_REGISTER}
REAL_COLOR_NAMES = tuple(
    name for id_, name in COLOR_REGISTER if id_ != COLOR_DEFAULT)

COLOR_DEFAULT_FOREGROUND = COLOR_BLACK
COLOR_DEFAULT_BACKGROUND = COLOR_WHITE

CODE_ENTER = '\x0d'
CODE_FN = ('\x1bOP', '\x1bOQ', '\x1bOR', '\x1bOS', '\x1b[15~', '\x1b[17~',
           '\x1b[18~', '\x1b[19~', '\x1b[20~', '\x1b[21~', '\x1b[23~')
CODE_PAGE_UP = '\x1b[5~'
CODE_PAGE_DOWN = '\x1b[6~'


def attr_to_string(attr):
    """Get string representation of given attributes."""

    res = []
    if (attr & ATTR_REVERSE) != 0:
        res.append('reverse')
    return '|'.join(res)


def string_to_attr(string):
    """
    Convert a string to attributes. Exception ValueError is raised if some
    attribute is invalid.
    """

    res = ATTR_NORMAL
    for attr in string.split('|'):
        if attr == 'normal':
            pass
        elif attr == 'reverse':
            res |= ATTR_REVERSE
        else:
            raise ValueError("Unrecognized attribute '{}'".format(attr))
    return res


def color_to_string(color):
    """Get string representation of a given color."""

    return COLOR_TO_STRING_MAP[color]


def string_to_color(string):
    """
    Convert a string to a color. Exception ValueError is raised if the color
    name is not recognized.
    """

    try:
        return STRING_TO_COLOR_MAP[string]
    except KeyError:
        raise ValueError("Unrecognized color '{}'".format(string))


class TermChar:
    """On-screen character."""

    def __init__(self, char=" ", attr=ATTR_NORMAL, fgcolor=COLOR_DEFAULT,
                 bgcolor=COLOR_DEFAULT):
        self.char = char
        self.attr = attr
        self.fgcolor = fgcolor
        self.bgcolor = bgcolor

    def __eq__(self, other):
        return self.__dict__ == other.__dict__

    def _get_translated_fgcolor(self):
        """
        Return the foreground color. If the current color is COLOR_DEFAULT then
        COLOR_DEFAULT_FOREGROUND is returned.
        """

        if self.fgcolor == COLOR_DEFAULT:
            return COLOR_DEFAULT_FOREGROUND
        return self.fgcolor

    def _get_translated_bgcolor(self):
        """
        Return the background color. If the current color is COLOR_DEFAULT then
        COLOR_DEFAULT_BACKGROUND is returned.
        """

        if self.bgcolor == COLOR_DEFAULT:
            return COLOR_DEFAULT_BACKGROUND
        return self.bgcolor

    def get_tag_foreground(self):
        """
        Return a name of the final foreground color that should be used to
        display the character on the screen.
        """

        if self.attr & ATTR_REVERSE:
            color = self._get_translated_bgcolor()
        else:
            color = self._get_translated_fgcolor()
        return color_to_string(color)

    def get_tag_background(self):
        """
        Return a name of the final background color that should be used to
        display the character on the screen.
        """

        if self.attr & ATTR_REVERSE:
            color = self._get_translated_fgcolor()
        else:
            color = self._get_translated_bgcolor()
        return color_to_string(color)


class Term:
    """Termex terminal emulator."""

    MODE_RUN = 0
    MODE_RECORD = 1
    MODE_TEST = 2

    class _TerminalConnectionException(Exception):
        """
        Exception reported when communication with the pseudo-terminal fails.
        """
        pass

    def __init__(self, root, program, mode, terminfo=None):
        self._root = root
        self._program = program
        self._mode = mode
        self._terminfo = terminfo

        self._child_pid = None
        self._fd = None

        self._screen = None
        self._cur_y = 0
        self._cur_x = 0
        self._attr = ATTR_NORMAL
        self._fgcolor = COLOR_DEFAULT
        self._bgcolor = COLOR_DEFAULT
        self._charbuf = b''

        # Initialize the GUI if requested.
        if self._root:
            self._root.title("Termex")
            self._frame = tkinter.Frame(self._root)

            self._text = tkinter.Text(self._root, height=ROWS, width=COLUMNS)
            self._text.config(
                foreground=color_to_string(COLOR_DEFAULT_FOREGROUND),
                background=color_to_string(COLOR_DEFAULT_BACKGROUND))
            self._text.pack()

            # Configure tag values.
            for fgcolor_str in REAL_COLOR_NAMES:
                for bgcolor_str in REAL_COLOR_NAMES:
                    tag = 'tag_{}-{}'.format(fgcolor_str, bgcolor_str)
                    self._text.tag_config(tag, foreground=fgcolor_str,
                                          background=bgcolor_str)

        self._erase_all()

        if self._mode == self.MODE_RECORD:
            self._test_e = ElementTree.Element('test')

    def _start_program(self):
        """
        Fork, connect the child's controlling terminal to a pseudo-terminal and
        start the selected child program.

        Parent behaviour: Returns True when the fork was successful, False
        otherwise. Note that the returned value does not provide information
        whether the exec call in the child process was successful or not. That
        must be determined by attempting communication with the child.

        Child behaviour: Execs the selected program and does not return if the
        call was successful, returns False otherwise.
        """

        # Fork and connect the child's controlling terminal to a
        # pseudo-terminal.
        try:
            self._child_pid, self._fd = pty.fork()
        except OSError as e:
            print("Fork to run '{}' failed: {}".format(self._program, e),
                  file=sys.stderr)
            return False
        if self._child_pid == 0:
            try:
                env = {'PATH': '/bin:/usr/bin', 'TERM': 'termex',
                       'LC_ALL': 'en_US.UTF-8'}
                if self._terminfo:
                    env['TERMINFO'] = self._terminfo
                elif 'TERMINFO' in os.environ:
                    env['TERMINFO'] = os.environ['TERMINFO']
                os.execle(self._program, self._program, env)
            except OSError as e:
                print("Failed to execute '{}': {}".format(self._program, e),
                      file=sys.stderr)
                return False

        return True

    def _finalize_program(self):
        """
        Close the connection to the pseudo-terminal and wait for the child
        program to complete. Returns True when the connection was successfully
        closed and the child completed in the timeout limit, False otherwise.
        """

        res = True

        # Close the file descriptor that is connected to the child's
        # controlling terminal.
        try:
            os.close(self._fd)
        except OSError as e:
            print("Failed to close file descriptor '{}' that is connected to "
                  "the child's controlling terminal: {}.".format(self._fd, e),
                  file=sys.stderr)
            res = False

        # Wait for the child to finish. It should terminate now that its input
        # was closed.
        for _ in range(CHILD_TIMEOUT):
            try:
                pid, _status = os.waitpid(self._child_pid, os.WNOHANG)
            except OSError as e:
                print("Failed to wait on child '{}' to complete: "
                      "{}.".format(pid, e), file=sys.stderr)
                res = False
                break
            if pid != 0:
                break
            time.sleep(1)
        else:
            print("Child '{}' has not completed.".format(self._child_pid),
                  file=sys.stderr)
            res = False

        return res

    def run_gui_mainloop(self):
        """Start the selected child program and run the tkinter's main loop."""

        assert self._mode == self.MODE_RUN or self._mode == self.MODE_RECORD

        # Start the specified program.
        if not self._start_program():
            return

        try:
            # Prepare for running the main loop.
            self._root.createfilehandler(
                self._fd, tkinter.READABLE,
                lambda fd, mask: self._pty_callback())
            self._root.bind('<Key>', self._tk_key)
            self._root.bind('<<Quit>>', lambda e: self._quit_gui_mainloop())
            self._root.protocol('WM_DELETE_WINDOW', self._quit_gui_mainloop)

            # Run the main loop.
            try:
                self._root.mainloop()
            except self._TerminalConnectionException as e:
                print("{}.".format(e), file=sys.stderr)

            self._root.deletefilehandler(self._fd)
        finally:
            # Finalize the run of the child program.
            self._finalize_program()

    def _quit_gui_mainloop(self):
        """Exit the tkinter's main loop."""

        assert self._mode == self.MODE_RUN or self._mode == self.MODE_RECORD
        self._root.quit()

    def _pty_callback(self):
        """
        Process a data event from the pseudo-terminal. Returns True when the
        connection to the pseudo-terminal was closed, False otherwise.
        Exception _TerminalConnectionException is raised if the read of the new
        data from the pseudo-terminal fails.
        """

        closed = False
        try:
            char = os.read(self._fd, 1)
        except OSError as e:
            if e.errno == errno.EIO:
                closed = True
            else:
                raise self._TerminalConnectionException(
                    "Error reading from file descriptor '{}' that is "
                    "connected to the child's controlling terminal: "
                    "{}".format(self._fd, e))

        # Check whether the descriptor referring to the pseudo-terminal slave
        # has been closed or end of file was reached.
        if closed or len(char) == 0:
            if self._root:
                self._root.quit()
            return True

        self._charbuf += char

        if self._handle_sequence(self._charbuf):
            self._charbuf = b''
        else:
            # print("Unmatched {}.".format(self._charbuf), file=sys.stderr)
            pass
        return False

    def _send_key(self, chars, name):
        """
        Write the specified characters that represent one key to the
        pseudo-terminal. If the recording mode is enabled then the specified
        key name is recorded in the test playbook. Exception
        _TerminalConnectionException is raised if the write to the
        pseudo-terminal fails.
        """

        if self._mode == self.MODE_RECORD:
            # Record the key.
            action_e = ElementTree.SubElement(self._test_e, 'action')
            action_e.set('key', name)
            print("Recorded key '{}'.".format(name))

        # Send the key to the terminal.
        try:
            os.write(self._fd, str.encode(chars))
        except OSError as e:
            raise self._TerminalConnectionException(
                "Error writing characters '{}' to file descriptor '{}' that "
                "is connected to the child's controlling terminal: "
                "{}".format(chars, self._fd, e))

    def _handle_sequence(self, seq):
        """
        Process a byte sequence received from the pseudo-terminal. Returns True
        when the sequence was recognized and successfully handled, False
        otherwise.
        """

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
            if self._root:
                self._root.bell()
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
        """
        Move the screen cursor one line down. The screen is scrolled if the
        cursor points to the last line.
        """

        if self._cur_y < ROWS - 1:
            self._cur_y += 1
        else:
            assert self._cur_y == ROWS - 1

            # At the last line of the terminal, scroll up the screen.
            del self._screen[0]
            self._screen.append([TermChar() for x in range(COLUMNS)])

            if self._root:
                self._text.config(state=tkinter.NORMAL)
                self._text.delete('1.0', '2.0')
                self._text.insert(tkinter.END, "\n" + " " * COLUMNS)
                self._text.config(state=tkinter.DISABLED)

    def _erase_all(self):
        """Completely clear the terminal's screen."""

        self._screen = [[TermChar() for x in range(COLUMNS)]
                        for y in range(ROWS)]

        if self._root:
            self._text.config(state=tkinter.NORMAL)
            self._text.delete('1.0', tkinter.END)
            self._text.insert('1.0', "\n".join([" " * COLUMNS] * ROWS))
            self._text.config(state=tkinter.DISABLED)

    def _insert_blanks(self, w):
        """
        Replace the specified number of characters on the current screen line
        with blanks.
        """

        del self._screen[self._cur_y][-w:]
        pre = self._screen[self._cur_y][:self._cur_x]
        post = self._screen[self._cur_y][self._cur_x:]
        self._screen[self._cur_y] = pre + [TermChar() for x in range(w)] + post

        if self._root:
            self._text.config(state=tkinter.NORMAL)
            self._text.delete('{}.end-{}c'.format(self._cur_y + 1, w),
                              '{}.end'.format(self._cur_y + 1))
            self._text.insert('{}.{}'.format(self._cur_y + 1, self._cur_x),
                              " " * w)
            self._text.config(state=tkinter.DISABLED)

    def _print_char_at(self, y, x, char):
        """Output one character on the screen at the specified coordinates."""

        # Record the character in the internal screen representation.
        self._screen[y][x] = char

        if self._root:
            # Add the character to the terminal text widget.
            self._text.config(state=tkinter.NORMAL)
            pos = '{}.{}'.format(y + 1, x)
            self._text.delete(pos)

            tag = 'tag_{}-{}'.format(char.get_tag_foreground(),
                                     char.get_tag_background())
            self._text.insert(pos, char.char, tag)
            self._text.config(state=tkinter.DISABLED)

    def _print_char(self, char):
        """Output one character on the screen at the cursor position."""

        self._print_char_at(self._cur_y, self._cur_x, char)

        # Advance the cursor.
        self._cur_x += 1
        if self._cur_x == COLUMNS:
            self._cur_x = 0
            self._cursor_down()

    def _tk_key(self, event):
        """Process a key pressed by the user."""

        if len(event.char) != 0:
            if event.char == CODE_ENTER:
                self._send_key(event.char, 'Enter')
            else:
                self._send_key(event.char, event.char)
            return

        # A special key was pressed.
        if event.keysym == 'F12':
            self._record_expected_screen()
            return
        if event.keysym == 'Prior':
            self._send_key(CODE_PAGE_UP, 'PageUp')
            return
        if event.keysym == 'Next':
            self._send_key(CODE_PAGE_DOWN, 'PageDown')
            return
        match = re.fullmatch('F([0-9]+)', event.keysym)
        if match:
            # F1 to F11.
            fnum = int(match.group(1))
            if fnum >= 1 and fnum <= len(CODE_FN):
                self._send_key(CODE_FN[fnum - 1], event.keysym)
                return

        print("Unrecognized key {}.".format(event.keysym), file=sys.stderr)

    def _get_screen_xml(self, screen):
        """
        Return an ElementTree.Element that represents the current screen
        content.
        """

        expect_e = ElementTree.Element('expect')
        data_e = ElementTree.SubElement(expect_e, 'data')

        colors = {}
        new_key = 'a'

        # Print content of the screen.
        for y in range(ROWS):
            line_e = ElementTree.SubElement(data_e, 'line')
            line_e.text = ''

            attr = ''

            for x in range(COLUMNS):
                term_char = screen[y][x]

                line_e.text += term_char.char

                color = (term_char.attr, term_char.fgcolor, term_char.bgcolor)
                if color == (ATTR_NORMAL, COLOR_DEFAULT, COLOR_DEFAULT):
                    key = ' '
                elif color in colors:
                    key = colors[color]
                else:
                    key = new_key
                    colors[color] = key
                    assert new_key != 'z'
                    new_key = chr(ord(new_key) + 1)
                attr += key

            # Record any non-default attributes/colors.
            if attr != ' ' * COLUMNS:
                attr_e = ElementTree.SubElement(data_e, 'attr')
                attr_e.text = attr

        # Record used color schemes.
        if colors:
            scheme_e = ElementTree.SubElement(expect_e, 'scheme')
            for color, key in sorted(colors.items(), key=lambda x: x[1]):
                attr, fgcolor, bgcolor = color
                color_e = ElementTree.SubElement(scheme_e, 'color')
                color_e.set('key', key)

                attr_str = attr_to_string(attr)
                if attr_str:
                    color_e.set('attributes', attr_str)

                fgcolor_str = color_to_string(fgcolor)
                if fgcolor_str:
                    color_e.set('foreground', fgcolor_str)

                bgcolor_str = color_to_string(bgcolor)
                if bgcolor_str:
                    color_e.set('background', bgcolor_str)

        return expect_e

    def _record_expected_screen(self):
        """
        Record the current screen content as an expected screen in the test
        playbook that is being created.
        """

        assert self._mode == self.MODE_RUN or self._mode == self.MODE_RECORD

        if self._mode != self.MODE_RECORD:
            print("Recording is not enabled.", file=sys.stderr)
            return

        expect_e = self._get_screen_xml(self._screen)
        self._test_e.append(expect_e)
        print("Recorded expected screen.")

    # Method _indent_xml() is based on a code from
    # http://effbot.org/zone/element-lib.htm#prettyprint.
    def _indent_xml(self, elem, level=0):
        """
        Indent elements of a given ElementTree so it can be pretty-printed.
        """

        i = '\n' + '\t' * level
        if len(elem):
            if not elem.text or not elem.text.strip():
                elem.text = i + '\t'
            for e in elem:
                self._indent_xml(e, level+1)
            if not e.tail or not e.tail.strip():
                e.tail = i
        if not elem.tail or not elem.tail.strip():
            elem.tail = i

    def output_test(self, filename):
        """
        Output a recorded playbook to a file with the given name. Returns True
        when the writing of the test data succeeded, False otherwise.
        """

        assert self._mode == self.MODE_RECORD

        # Pretty-format the XML tree.
        self._indent_xml(self._test_e)

        # Output the test.
        tree = ElementTree.ElementTree(self._test_e)
        try:
            tree.write(filename, 'unicode', True)
        except Exception as e:
            print("Failed to write playbook file '{}': {}.".format(
                filename, e), file=sys.stderr)
            return False

        return True

    class _TestFailure(Exception):
        """Exception reported when a test failed."""
        pass

    def _playbook_key(self, cmd_e):
        """
        Parse a description of one key action and send the key to the terminal.
        Exception _TestFailure is raised if the description is malformed or
        incomplete, exception _TerminalConnectionException can be thrown when
        communication with the pseudo-terminal fails.
        """

        assert self._mode == self.MODE_TEST

        try:
            key = cmd_e.attrib['key']
        except KeyError:
            raise self._TestFailure("Element 'action' is missing required "
                                    "attribute 'key'")

        # Handle simple characters.
        if len(key) == 1:
            self._send_key(key, key)
            return

        # Handle special keys.
        if key == 'Enter':
            self._send_key(CODE_ENTER, key)
            return
        if key == 'PageUp':
            self._send_key(CODE_PAGE_UP, key)
            return
        if key == 'PageDown':
            self._send_key(CODE_PAGE_DOWN, key)
        match = re.fullmatch('F([0-9]+)', key)
        if match:
            # F1 to F11.
            fnum = int(match.group(1))
            if fnum >= 1 and fnum <= len(CODE_FN):
                self._send_key(CODE_FN[fnum - 1], key)
                return

        raise self._TestFailure(
            "Element 'action' specifies unrecognized key '{}'".format(key))

    def _parse_color_scheme(self, scheme_e):
        """
        Parse color scheme of one expected screen. Dictionary with
        {'key': (attr, fgcolor, bgcolor), ...} is returned on success,
        exception _TestFailure is raised if the description is malformed or
        incomplete.
        """

        assert self._mode == self.MODE_TEST

        colors = {}
        for color_e in scheme_e:
            try:
                key = color_e.attrib['key']
            except KeyError:
                raise self._TestFailure(
                    "Element 'color' is missing required attribute 'key'")

            attr = None
            if 'attributes' in color_e.attrib:
                try:
                    attr = color_e.attrib['attributes']
                except ValueError as e:
                    raise self._TestFailure(
                        "Value of attribute 'attributes' is invalid: "
                        "{}".format(e))

            fgcolor = None
            if 'foreground' in color_e.attrib:
                try:
                    attr = color_e.attrib['foreground']
                except ValueError as e:
                    raise self._TestFailure(
                        "Value of attribute 'foreground' is invalid: "
                        "{}".format(e))

            bgcolor = None
            if 'background' in color_e.attrib:
                try:
                    attr = color_e.attrib['background']
                except ValueError as e:
                    raise self._TestFailure(
                        "Value of attribute 'background' is invalid: "
                        "{}".format(e))

            colors[key] = (attr, fgcolor, bgcolor)

        return colors

    def _parse_screen_data(self, data_e, colors):
        """
        Parse screen lines of one expected screen. Internal screen
        representation is returned on success, exception _TestFailure is raised
        if the description is malformed or incomplete.
        """

        assert self._mode == self.MODE_TEST

        NEW_LINE = 0
        NEW_LINE_OR_ATTR = 1
        state = NEW_LINE
        line = None
        expected_screen = []

        for data_sub_e in data_e:
            # Do common processing for both states.
            if data_sub_e.tag == 'line':
                # Append the previous line.
                if line:
                    expected_screen.append(line)
                # Parse the new line.
                line = [TermChar(char) for char in data_sub_e.text]

            if state == NEW_LINE and data_sub_e.tag != 'line':
                raise self._TestFailure("Element '{}' is invalid, expected "
                                        "'line'".format(data_sub_e.tag))

            elif state == NEW_LINE_OR_ATTR:
                if data_sub_e.tag == 'attr':
                    if len(data_sub_e.text) != len(line):
                        raise self._TestFailure(
                            "Element 'attr' does not match the previous line, "
                            "expected '{}' attribute characters but got "
                            "'{}'".format(len(line), len(data_sub_e.text)))

                    for i, key in enumerate(data_sub_e.text):
                        try:
                            attr, fgcolor, bgcolor = colors[key]
                        except KeyError:
                            raise self._TestFailure("Color attribute '{}' is "
                                                    "not defined".format(key))
                        line[i].attr = attr
                        line[i].fgcolor = fgcolor
                        line[i].bgcolor = bgcolor
                elif data_sub_e.tag != 'line':
                    raise self._TestFailure(
                        "Element '{}' is invalid, expected 'line' or "
                        "'attr'".format(data_sub_e.tag))

        # Append the final line.
        if line:
            expected_screen.append(line)

        return expected_screen

    def _parse_expected_screen(self, expect_e):
        """
        Parse a description of one expected screen. Internal screen
        representation is returned on success, exception _TestFailure is raised
        if the description is malformed or incomplete.
        """

        assert self._mode == self.MODE_TEST

        data_e = None
        scheme_e = None
        for sub_e in expect_e:
            if sub_e.tag == 'data':
                if data_e:
                    raise self._TestFailure("Element 'expect' contains "
                                            "multiple 'data' sub-elements")
                data_e = sub_e
            elif sub_e.tag == 'scheme':
                if scheme_e:
                    raise self._TestFailure("Element 'expect' contains "
                                            "multiple 'scheme' sub-elements")
                scheme_e = sub_e

        if not data_e:
            raise self._TestFailure(
                "Element 'expect' is missing required sub-element 'data'")

        # Parse the color scheme.
        if scheme_e:
            colors = self._parse_color_scheme(scheme_e)
        else:
            colors = {}

        # Parse the screen data.
        return self._parse_screen_data(data_e, colors)

    def _report_failed_expectation(self, expected_screen):
        """
        Report that the expected screen state has not been reached. The output
        consists of the expected screen, the current screen content, followed
        by differences between the two screens.
        """

        assert self._mode == self.MODE_TEST

        # Print the expected screen. The output is not verbatim as it was
        # specified in the input file, but instead the screen is printed in the
        # same way the current screen gets output. This allows to properly show
        # differences between the two screens.

        expected_screen_e = self._get_screen_xml(expected_screen)
        self._indent_xml(expected_screen_e)
        expected_screen_str = ElementTree.tostring(
            expected_screen_e, 'unicode')
        print("Expected (normalized) screen:", file=sys.stderr)
        print(expected_screen_str, file=sys.stderr)

        # Print the current screen.
        current_screen_e = self._get_screen_xml(self._screen)
        self._indent_xml(current_screen_e)
        current_screen_str = ElementTree.tostring(current_screen_e, 'unicode')
        print("Current screen:", file=sys.stderr)
        print(current_screen_str, file=sys.stderr)

        # Print the delta.
        print("Differences:", file=sys.stderr)
        sys.stderr.writelines(difflib.unified_diff(
            expected_screen_str.splitlines(keepends=True),
            current_screen_str.splitlines(keepends=True),
            fromfile="Expected screen", tofile="Current screen"))

    def _execute_playbook(self, test_e):
        """
        Run the main loop and execute the given test playbook. Normal return
        from the method indicates that the test succeeded. Exception
        _TestFailure is raised when the test fails and exception
        _TerminalConnectionException can be thrown when communication with the
        pseudo-terminal fails.
        """

        assert self._mode == self.MODE_TEST

        if test_e.tag != 'test':
            raise self._TestFailure("Root element '{}' is invalid, expected "
                                    "'test'".format(test_e.tag))
        cmd_iter = iter(test_e)

        # Start the main loop.
        with selectors.DefaultSelector() as sel:
            sel.register(self._fd, selectors.EVENT_READ)

            expected_screen = None
            more_commands = True
            while True:
                # Process any actions and find an expected screen.
                while not expected_screen and more_commands:
                    try:
                        cmd_e = next(cmd_iter)
                        if cmd_e.tag == 'action':
                            self._playbook_key(cmd_e)
                        elif cmd_e.tag == 'expect':
                            expected_screen = self._parse_expected_screen(
                                cmd_e)
                            # Stop processing more commands for now and wait
                            # for the expected screen to appear.
                            break
                        else:
                            raise self._TestFailure(
                                "Element '{}' is invalid, expected 'action' "
                                "or 'expect'".format(cmd_e.tag))
                    except StopIteration:
                        # No more commands.
                        more_commands = False

                # Wait for the expected screen.
                events = sel.select(CHILD_TIMEOUT)
                if not events:
                    if expected_screen:
                        self._report_failed_expectation(expected_screen)
                    raise self._TestFailure(
                        "Timeout reached. No event received in the last {} "
                        "second(s)".format(CHILD_TIMEOUT))

                # Expect only an event on self._fd.
                assert len(events) == 1
                event = events[0]
                key, _mask = event
                assert key.fd == self._fd

                closed = self._pty_callback()
                if closed:
                    if more_commands:
                        raise self._TestFailure(
                            "Connection to the terminal was closed but the "
                            "playbook contains more commands")
                    break

                # Check if the expected screen is present.
                if self._screen == expected_screen:
                    expected_screen = None

    def execute_test(self, filename):
        """
        Load test data from a given file, start the program under the test and
        execute the test playbook. Returns True when the test succeeded, False
        otherwise.
        """

        assert self._mode == self.MODE_TEST

        # Read the test data.
        try:
            tree = ElementTree.ElementTree(file=filename)
        except Exception as e:
            print("Failed to read playbook file '{}': {}.".format(filename, e),
                  file=sys.stderr)
            return False

        # Start the specified program.
        if not self._start_program():
            return False

        # Execute the test playbook.
        res = True
        try:
            self._execute_playbook(tree.getroot())
        except (self._TerminalConnectionException, self._TestFailure) as e:
            print("{}.".format(e), file=sys.stderr)
            res = False
        finally:
            # Finalize the run of the child program.
            if not self._finalize_program():
                res = False

        # Return whether the test passed.
        return res


def main():
    """
    Parse command line arguments and execute the operation that the user
    selected. Returns 0 if the operation was successful and a non-zero value
    otherwise.
    """

    # Parse command line arguments.
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-t', '--terminfo', metavar='PATH', help="path to terminfo directory")

    subparsers = parser.add_subparsers(dest='command')
    subparsers.required = True

    program_parser = argparse.ArgumentParser(add_help=False)
    program_parser.add_argument('program')

    # Create the parser for the 'run' command.
    parser_run = subparsers.add_parser(
        'run', parents=[program_parser], help="run a command")
    parser_run.set_defaults(mode=Term.MODE_RUN)

    # Create the parser for the 'record' command.
    parser_record = subparsers.add_parser(
        'record', parents=[program_parser], help="record a test")
    parser_record.set_defaults(mode=Term.MODE_RECORD)
    parser_record.add_argument(
        '-p', '--playbook', metavar='FILE', required=True,
        help="output playbook file")

    # Create the parser for the 'test' command.
    parser_test = subparsers.add_parser(
        'test', parents=[program_parser], help="run a test")
    parser_test.set_defaults(mode=Term.MODE_TEST)
    parser_test.add_argument(
        '-p', '--playbook', metavar='FILE', required=True,
        help="input playbook file")

    args = parser.parse_args()

    tk_root = None
    if args.mode in (Term.MODE_RUN, Term.MODE_RECORD):
        # Start the terminal GUI.
        try:
            tk_root = tkinter.Tk()
        except tkinter.TclError as e:
            print("Failed to initialize GUI: {}.".format(e), file=sys.stderr)
            return 1

    term = Term(tk_root, args.program, args.mode, args.terminfo)
    if tk_root:
        # Start the GUI main loop.
        term.run_gui_mainloop()
    else:
        # Execute and check the playbook, without running GUI.
        ok = term.execute_test(args.playbook)
        if ok:
            msg = "succeeded"
            res = 0
        else:
            msg = "failed"
            res = 1

        print("Run of '{}' using playbook '{}' {}.".format(
            args.program, args.playbook, msg))
        return res

    if args.mode == Term.MODE_RECORD:
        # Get the recorded test data and write them to a file.
        if not term.output_test(args.playbook):
            return 1

    return 0


if __name__ == '__main__':
    sys.exit(main())
