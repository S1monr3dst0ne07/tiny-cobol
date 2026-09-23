#!/usr/bin/python3

import sys
from dataclasses import dataclass as dc

old_print = print
emit  = lambda x: old_print(x)
print = lambda x: old_print(x, file=sys.stderr)

def lex(path):
    with open(path, 'r') as f:
        src = f.read()

    def get(char):
        match char:
            case x if x.isalnum(): return 'word'
            case '-':              return 'word'
            case '(' | ')':        return 'word'
            case '.':              return 'dot'
            case '"':              return 'quote'
            case ' ' | '\n':       return 'format'
            case _:                return 'symb'

    @dc
    class Token:
        content : str
        line_no : int
        path    : str

        def __str__(self):
            return f"{self.content}"

    @dc
    class Streamer:
        toks : list[Token]
        index : int = 0

        def __str__(self):
            return '\n'.join(str(x) for x in self.toks)

        def peek(self):
            return self.toks[self.index].content

        def pop(self):
            tok = self.peek()
            self.index += 1
            return tok

        def has(self):
            return self.index < len(self.toks)

        def expect(self, word):
            token = self.toks[self.index]
            self.index += 1

            if token.content != word:
                print(f"Error on line {token.line_no} in file {token.path}: Expected `{word}` but got `{token.content}`")
                sys.exit(1)

        def expects(self, *args):
            for arg in args:
                self.expect(arg)

    buffer = ''
    stream = []

    line_no = 1
    state   = 'format'

    in_string   = False

    for index, char in enumerate(src):
        kind = get(char)
        if char == '\n': line_no += 1
        if state == 'quote': in_string = not in_string

        if kind != state and not in_string:
            if state != 'format':
                stream.append(Token(
                    buffer, line_no, path
                ))

            buffer = ''

        state = kind
        if state != 'quote':
            buffer += char

    return Streamer(stream)



WORKING_FIELD_LEVEL = 77

# global symbol table
st = []

# zero means "not present"
@dc
class FieldEntry:
    # parser data
    level : int = 0
    name  : str = ""

    picture   : str = "" # for atomic item
    value     : str = ""

    occurance : int = 1

    # entry linkage
    parent_idx : int = 0
    child_idx  : int = 0

    prev_idx   : int = 0
    next_idx   : int = 0

    #data layout
    base_address : int = 0
    inner_size   : int = 0 # inside of group field
    outer_size   : int = 0 # scaled by occurances

        # accumulated size (outer_size + next.accum_size).
        # needed to compute inner_size of parent.
    accum_size   : int = 0 





def add_sym_table_entry():
    index = len(st)
    st.append(FieldEntry())
    return index


def parse_picture(stream, curr):
    content = stream.pop()

    buffer = ''
    count = ''
    bracket = False
    for char in content:
        if   char == '(': bracket = True
        elif char == ')': 
            char = buffer[-1]
            buffer += char * (int(count) - 1)
            count = ''
            bracket = False

        elif bracket: count  += char
        else:         buffer += char



    st[curr].picture = buffer


def parse_data(stream):
    # base entry
    curr = add_sym_table_entry()
    st[curr].level = 0
     
    while stream.has() and stream.peek().isdigit():
        level = int(stream.peek())

        # new child
        if level > st[curr].level:
            parent = curr
            curr = add_sym_table_entry()

            st[curr].level = level

            st[parent].child_idx = curr
            st[curr].parent_idx = parent

        # new sibiling
        elif level == st[curr].level:
            prev = curr
            curr = add_sym_table_entry()

            st[curr].level = level
            st[curr].parent_idx = st[prev].parent_idx

            st[prev].next_idx = curr
            st[curr].prev_idx = prev

        # done
        elif level < st[curr].level:
            curr = st[curr].parent_idx
            continue

        stream.pop() #level
        st[curr].name = stream.pop() #name

        while stream.peek() != '.':
            match stream.pop():
                case 'occurs':
                    st[curr].occurance = int(stream.pop())
                    stream.expect('times')
                case 'pic':   parse_picture(stream, curr)
                case 'value': st[curr].value = stream.pop()
        stream.expect('.')


def compute_data_layout():
    # compute content sizes
    for entry in st[::-1]:
        entry.inner_size = 0
        if entry.picture   != '': entry.inner_size += len(entry.picture)
        if entry.child_idx != 0:  entry.inner_size += st[entry.child_idx].accum_size

        entry.outer_size = entry.inner_size * entry.occurance
        entry.accum_size = entry.outer_size

        if entry.next_idx  != 0:  entry.accum_size += st[entry.next_idx ].accum_size

    # compute base addresses
    st[0].base_address = 0
    for entry in st[1:]:
        # if this is in a chain, compute chain based
        if entry.prev_idx != 0:
            prev_base       = st[entry.prev_idx].base_address
            prev_outer_size = st[entry.prev_idx].outer_size
            base_address    = prev_base + prev_outer_size

        # otherwise, the base_address is shared with the parent
        else:
            base_address = st[entry.parent_idx].base_address

        entry.base_address = base_address


def sym_lookup(token):
    for s in st: 
        if s.name == token:
            return s
    return None

load_ptr = 0
load_size = 0
def compile_load(stream):
    global load_ptr, load_size
    token = stream.pop()

    res = sym_lookup(token)
    
    if res is not None:
        load_size = res.outer_size
        load_ptr  = res.base_address

    else:
        for index, char in enumerate(token):
            emit(f"mov [buffer + {index}], '{char}'")
        return index + 1


def compile_perform(stream):
    stream.expect('perform')
    target_name = stream.pop()

    match stream.peek():
        case 'exactly':
            stream.pop()
            compile_load(stream)
            stream.expect('times')

    stream.expect(".")

def compile_display(stream):
    stream.expect('display')
    size = compile_load(stream)
    emit("mov rax, 1")
    emit("mov rdi, 1")
    emit("mov rsi, buffer")
    emit(f"mov rdx, {size}")
    emit(f"syscall")



def compile_procs(stream):
    global in_func
    while stream.has():
        match stream.peek():
            case 'perform': compile_perform(stream)
            case 'display': compile_display(stream)
            case 'move':    compile_move(stream)
            case 'add':     compile_add(stream)
            case label:
                stream.pop()
                stream.expect('.')

                emit(f"{label}:")



def main():
    stream = lex(sys.argv[1])

    stream.expects('data', 'division', '.')
    stream.expects('working-storage', 'section', '.')
    parse_data(stream)
    compute_data_layout()

    stream.expects('procedure', 'division', '.')
    compile_procs(stream)

    for i, x in enumerate(st):
        print(i, x)


if __name__ == "__main__":
    main()
